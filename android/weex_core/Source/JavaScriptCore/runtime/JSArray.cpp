/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003-2017 Apple Inc. All rights reserved.
 *  Copyright (C) 2003 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2006 Alexey Proskuryakov (ap@nypop.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "config.h"
#include "JSArray.h"

#include "ArrayPrototype.h"
#include "ButterflyInlines.h"
#include "CodeBlock.h"
#include "Error.h"
#include "GetterSetter.h"
#include "IndexingHeaderInlines.h"
#include "JSArrayInlines.h"
#include "JSCInlines.h"
#include "PropertyNameArray.h"
#include "TypeError.h"
#include <wtf/Assertions.h>

using namespace std;
using namespace WTF;

namespace JSC {

static const char* const LengthExceededTheMaximumArrayLengthError = "Length exceeded the maximum array length";

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(JSArray);

const ClassInfo JSArray::s_info = {"Array", &JSNonFinalObject::s_info, 0, CREATE_METHOD_TABLE(JSArray)};

Butterfly* createArrayButterflyInDictionaryIndexingMode(
    VM& vm, JSCell* intendedOwner, unsigned initialLength)
{
    Butterfly* butterfly = Butterfly::create(
        vm, intendedOwner, 0, 0, true, IndexingHeader(), ArrayStorage::sizeFor(0));
    ArrayStorage* storage = butterfly->arrayStorage();
    storage->setLength(initialLength);
    storage->setVectorLength(0);
    storage->m_indexBias = 0;
    storage->m_sparseMap.clear();
    storage->m_numValuesInVector = 0;
    return butterfly;
}

JSArray* JSArray::tryCreateForInitializationPrivate(VM& vm, GCDeferralContext* deferralContext, Structure* structure, unsigned initialLength)
{
    if (initialLength > MAX_STORAGE_VECTOR_LENGTH)
        return 0;

    unsigned outOfLineStorage = structure->outOfLineCapacity();

    Butterfly* butterfly;
    IndexingType indexingType = structure->indexingType();
    if (LIKELY(!hasAnyArrayStorage(indexingType))) {
        ASSERT(
            hasUndecided(indexingType)
            || hasInt32(indexingType)
            || hasDouble(indexingType)
            || hasContiguous(indexingType));

        unsigned vectorLength = Butterfly::optimalContiguousVectorLength(structure, initialLength);
        void* temp = vm.auxiliarySpace.tryAllocate(deferralContext, Butterfly::totalSize(0, outOfLineStorage, true, vectorLength * sizeof(EncodedJSValue)));
        if (!temp)
            return nullptr;
        butterfly = Butterfly::fromBase(temp, 0, outOfLineStorage);
        butterfly->setVectorLength(vectorLength);
        butterfly->setPublicLength(initialLength);
        if (hasDouble(indexingType)) {
            for (unsigned i = initialLength; i < vectorLength; ++i)
                butterfly->contiguousDouble()[i] = PNaN;
        } else {
            for (unsigned i = initialLength; i < vectorLength; ++i)
                butterfly->contiguous()[i].clear();
        }
    } else {
        unsigned vectorLength = ArrayStorage::optimalVectorLength(0, structure, initialLength);
        void* temp = vm.auxiliarySpace.tryAllocate(deferralContext, Butterfly::totalSize(0, outOfLineStorage, true, ArrayStorage::sizeFor(vectorLength)));
        if (!temp)
            return nullptr;
        butterfly = Butterfly::fromBase(temp, 0, outOfLineStorage);
        *butterfly->indexingHeader() = indexingHeaderForArrayStorage(initialLength, vectorLength);
        ArrayStorage* storage = butterfly->arrayStorage();
        storage->m_indexBias = 0;
        storage->m_sparseMap.clear();
        storage->m_numValuesInVector = initialLength;
        for (unsigned i = initialLength; i < vectorLength; ++i)
            storage->m_vector[i].clear();
    }

    return createWithButterfly(vm, deferralContext, structure, butterfly);
}

void JSArray::setLengthWritable(ExecState* exec, bool writable)
{
    ASSERT(isLengthWritable() || !writable);
    if (!isLengthWritable() || writable)
        return;

    enterDictionaryIndexingMode(exec->vm());

    SparseArrayValueMap* map = arrayStorage()->m_sparseMap.get();
    ASSERT(map);
    map->setLengthIsReadOnly();
}

// Defined in ES5.1 15.4.5.1
bool JSArray::defineOwnProperty(JSObject* object, ExecState* exec, PropertyName propertyName, const PropertyDescriptor& descriptor, bool throwException)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSArray* array = jsCast<JSArray*>(object);

    // 3. If P is "length", then
    if (propertyName == vm.propertyNames->length) {
        // All paths through length definition call the default [[DefineOwnProperty]], hence:
        // from ES5.1 8.12.9 7.a.
        if (descriptor.configurablePresent() && descriptor.configurable())
            return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeConfigurabilityError));
        // from ES5.1 8.12.9 7.b.
        if (descriptor.enumerablePresent() && descriptor.enumerable())
            return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeEnumerabilityError));

        // a. If the [[Value]] field of Desc is absent, then
        // a.i. Return the result of calling the default [[DefineOwnProperty]] internal method (8.12.9) on A passing "length", Desc, and Throw as arguments.
        if (descriptor.isAccessorDescriptor())
            return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeAccessMechanismError));
        // from ES5.1 8.12.9 10.a.
        if (!array->isLengthWritable() && descriptor.writablePresent() && descriptor.writable())
            return typeError(exec, scope, throwException, ASCIILiteral(UnconfigurablePropertyChangeWritabilityError));
        // This descriptor is either just making length read-only, or changing nothing!
        if (!descriptor.value()) {
            if (descriptor.writablePresent())
                array->setLengthWritable(exec, descriptor.writable());
            return true;
        }
        
        // b. Let newLenDesc be a copy of Desc.
        // c. Let newLen be ToUint32(Desc.[[Value]]).
        unsigned newLen = descriptor.value().toUInt32(exec);
        // d. If newLen is not equal to ToNumber( Desc.[[Value]]), throw a RangeError exception.
        if (newLen != descriptor.value().toNumber(exec)) {
            JSC::throwException(exec, scope, createRangeError(exec, ASCIILiteral("Invalid array length")));
            return false;
        }

        // Based on SameValue check in 8.12.9, this is always okay.
        // FIXME: Nothing prevents this from being called on a RuntimeArray, and the length function will always return 0 in that case.
        if (newLen == array->length()) {
            if (descriptor.writablePresent())
                array->setLengthWritable(exec, descriptor.writable());
            return true;
        }

        // e. Set newLenDesc.[[Value] to newLen.
        // f. If newLen >= oldLen, then
        // f.i. Return the result of calling the default [[DefineOwnProperty]] internal method (8.12.9) on A passing "length", newLenDesc, and Throw as arguments.
        // g. Reject if oldLenDesc.[[Writable]] is false.
        if (!array->isLengthWritable())
            return typeError(exec, scope, throwException, ASCIILiteral(ReadonlyPropertyChangeError));
        
        // h. If newLenDesc.[[Writable]] is absent or has the value true, let newWritable be true.
        // i. Else,
        // i.i. Need to defer setting the [[Writable]] attribute to false in case any elements cannot be deleted.
        // i.ii. Let newWritable be false.
        // i.iii. Set newLenDesc.[[Writable] to true.
        // j. Let succeeded be the result of calling the default [[DefineOwnProperty]] internal method (8.12.9) on A passing "length", newLenDesc, and Throw as arguments.
        // k. If succeeded is false, return false.
        // l. While newLen < oldLen repeat,
        // l.i. Set oldLen to oldLen – 1.
        // l.ii. Let deleteSucceeded be the result of calling the [[Delete]] internal method of A passing ToString(oldLen) and false as arguments.
        // l.iii. If deleteSucceeded is false, then
        bool success = array->setLength(exec, newLen, throwException);
        ASSERT(!scope.exception() || !success);
        if (!success) {
            // 1. Set newLenDesc.[[Value] to oldLen+1.
            // 2. If newWritable is false, set newLenDesc.[[Writable] to false.
            // 3. Call the default [[DefineOwnProperty]] internal method (8.12.9) on A passing "length", newLenDesc, and false as arguments.
            // 4. Reject.
            if (descriptor.writablePresent())
                array->setLengthWritable(exec, descriptor.writable());
            return false;
        }

        // m. If newWritable is false, then
        // i. Call the default [[DefineOwnProperty]] internal method (8.12.9) on A passing "length",
        //    Property Descriptor{[[Writable]]: false}, and false as arguments. This call will always
        //    return true.
        if (descriptor.writablePresent())
            array->setLengthWritable(exec, descriptor.writable());
        // n. Return true.
        return true;
    }

    // 4. Else if P is an array index (15.4), then
    // a. Let index be ToUint32(P).
    if (std::optional<uint32_t> optionalIndex = parseIndex(propertyName)) {
        // b. Reject if index >= oldLen and oldLenDesc.[[Writable]] is false.
        uint32_t index = optionalIndex.value();
        // FIXME: Nothing prevents this from being called on a RuntimeArray, and the length function will always return 0 in that case.
        if (index >= array->length() && !array->isLengthWritable())
            return typeError(exec, scope, throwException, ASCIILiteral("Attempting to define numeric property on array with non-writable length property."));
        // c. Let succeeded be the result of calling the default [[DefineOwnProperty]] internal method (8.12.9) on A passing P, Desc, and false as arguments.
        // d. Reject if succeeded is false.
        // e. If index >= oldLen
        // e.i. Set oldLenDesc.[[Value]] to index + 1.
        // e.ii. Call the default [[DefineOwnProperty]] internal method (8.12.9) on A passing "length", oldLenDesc, and false as arguments. This call will always return true.
        // f. Return true.
        scope.release();
        return array->defineOwnIndexedProperty(exec, index, descriptor, throwException);
    }

    scope.release();
    return array->JSObject::defineOwnNonIndexProperty(exec, propertyName, descriptor, throwException);
}

bool JSArray::getOwnPropertySlot(JSObject* object, ExecState* exec, PropertyName propertyName, PropertySlot& slot)
{
    JSArray* thisObject = jsCast<JSArray*>(object);
    if (propertyName == exec->propertyNames().length) {
        unsigned attributes = thisObject->isLengthWritable() ? DontDelete | DontEnum : DontDelete | DontEnum | ReadOnly;
        slot.setValue(thisObject, attributes, jsNumber(thisObject->length()));
        return true;
    }

    return JSObject::getOwnPropertySlot(thisObject, exec, propertyName, slot);
}

// ECMA 15.4.5.1
bool JSArray::put(JSCell* cell, ExecState* exec, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSArray* thisObject = jsCast<JSArray*>(cell);

    if (UNLIKELY(isThisValueAltered(slot, thisObject))) {
        scope.release();
        return ordinarySetSlow(exec, thisObject, propertyName, value, slot.thisValue(), slot.isStrictMode());
    }

    if (propertyName == exec->propertyNames().length) {
        unsigned newLength = value.toUInt32(exec);
        RETURN_IF_EXCEPTION(scope, false);
        if (value.toNumber(exec) != static_cast<double>(newLength)) {
            throwException(exec, scope, createRangeError(exec, ASCIILiteral("Invalid array length")));
            return false;
        }
        scope.release();
        return thisObject->setLength(exec, newLength, slot.isStrictMode());
    }

    scope.release();
    return JSObject::put(thisObject, exec, propertyName, value, slot);
}

bool JSArray::deleteProperty(JSCell* cell, ExecState* exec, PropertyName propertyName)
{
    JSArray* thisObject = jsCast<JSArray*>(cell);

    if (propertyName == exec->propertyNames().length)
        return false;

    return JSObject::deleteProperty(thisObject, exec, propertyName);
}

static int compareKeysForQSort(const void* a, const void* b)
{
    unsigned da = *static_cast<const unsigned*>(a);
    unsigned db = *static_cast<const unsigned*>(b);
    return (da > db) - (da < db);
}

void JSArray::getOwnNonIndexPropertyNames(JSObject* object, ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    JSArray* thisObject = jsCast<JSArray*>(object);

    if (mode.includeDontEnumProperties())
        propertyNames.add(exec->propertyNames().length);

    JSObject::getOwnNonIndexPropertyNames(thisObject, exec, propertyNames, mode);
}

// This method makes room in the vector, but leaves the new space for count slots uncleared.
bool JSArray::unshiftCountSlowCase(const AbstractLocker&, VM& vm, DeferGC&, bool addToFront, unsigned count)
{
    ArrayStorage* storage = ensureArrayStorage(vm);
    Butterfly* butterfly = storage->butterfly();
    Structure* structure = this->structure(vm);
    unsigned propertyCapacity = structure->outOfLineCapacity();
    unsigned propertySize = structure->outOfLineSize();
    
    // If not, we should have handled this on the fast path.
    ASSERT(!addToFront || count > storage->m_indexBias);

    // Step 1:
    // Gather 4 key metrics:
    //  * usedVectorLength - how many entries are currently in the vector (conservative estimate - fewer may be in use in sparse vectors).
    //  * requiredVectorLength - how many entries are will there be in the vector, after allocating space for 'count' more.
    //  * currentCapacity - what is the current size of the vector, including any pre-capacity.
    //  * desiredCapacity - how large should we like to grow the vector to - based on 2x requiredVectorLength.

    unsigned length = storage->length();
    unsigned oldVectorLength = storage->vectorLength();
    unsigned usedVectorLength = min(oldVectorLength, length);
    ASSERT(usedVectorLength <= MAX_STORAGE_VECTOR_LENGTH);
    // Check that required vector length is possible, in an overflow-safe fashion.
    if (count > MAX_STORAGE_VECTOR_LENGTH - usedVectorLength)
        return false;
    unsigned requiredVectorLength = usedVectorLength + count;
    ASSERT(requiredVectorLength <= MAX_STORAGE_VECTOR_LENGTH);
    // The sum of m_vectorLength and m_indexBias will never exceed MAX_STORAGE_VECTOR_LENGTH.
    ASSERT(storage->vectorLength() <= MAX_STORAGE_VECTOR_LENGTH && (MAX_STORAGE_VECTOR_LENGTH - storage->vectorLength()) >= storage->m_indexBias);
    unsigned currentCapacity = storage->vectorLength() + storage->m_indexBias;
    // The calculation of desiredCapacity won't overflow, due to the range of MAX_STORAGE_VECTOR_LENGTH.
    // FIXME: This code should be fixed to avoid internal fragmentation. It's not super high
    // priority since increaseVectorLength() will "fix" any mistakes we make, but it would be cool
    // to get this right eventually.
    unsigned desiredCapacity = min(MAX_STORAGE_VECTOR_LENGTH, max(BASE_ARRAY_STORAGE_VECTOR_LEN, requiredVectorLength) << 1);

    // Step 2:
    // We're either going to choose to allocate a new ArrayStorage, or we're going to reuse the existing one.

    void* newAllocBase = 0;
    unsigned newStorageCapacity;
    bool allocatedNewStorage;
    // If the current storage array is sufficiently large (but not too large!) then just keep using it.
    if (currentCapacity > desiredCapacity && isDenseEnoughForVector(currentCapacity, requiredVectorLength)) {
        newAllocBase = butterfly->base(structure);
        newStorageCapacity = currentCapacity;
        allocatedNewStorage = false;
    } else {
        size_t newSize = Butterfly::totalSize(0, propertyCapacity, true, ArrayStorage::sizeFor(desiredCapacity));
        newAllocBase = vm.auxiliarySpace.tryAllocate(newSize);
        if (!newAllocBase)
            return false;
        newStorageCapacity = desiredCapacity;
        allocatedNewStorage = true;
    }

    // Step 3:
    // Work out where we're going to move things to.

    // Determine how much of the vector to use as pre-capacity, and how much as post-capacity.
    // If we're adding to the end, we'll add all the new space to the end.
    // If the vector had no free post-capacity (length >= m_vectorLength), don't give it any.
    // If it did, we calculate the amount that will remain based on an atomic decay - leave the
    // vector with half the post-capacity it had previously.
    unsigned postCapacity = 0;
    if (!addToFront)
        postCapacity = max(newStorageCapacity - requiredVectorLength, count);
    else if (length < storage->vectorLength()) {
        // Atomic decay, + the post-capacity cannot be greater than what is available.
        postCapacity = min((storage->vectorLength() - length) >> 1, newStorageCapacity - requiredVectorLength);
        // If we're moving contents within the same allocation, the post-capacity is being reduced.
        ASSERT(newAllocBase != butterfly->base(structure) || postCapacity < storage->vectorLength() - length);
    }

    unsigned newVectorLength = requiredVectorLength + postCapacity;
    unsigned newIndexBias = newStorageCapacity - newVectorLength;

    Butterfly* newButterfly = Butterfly::fromBase(newAllocBase, newIndexBias, propertyCapacity);

    if (addToFront) {
        ASSERT(count + usedVectorLength <= newVectorLength);
        memmove(newButterfly->arrayStorage()->m_vector + count, storage->m_vector, sizeof(JSValue) * usedVectorLength);
        memmove(newButterfly->propertyStorage() - propertySize, butterfly->propertyStorage() - propertySize, sizeof(JSValue) * propertySize + sizeof(IndexingHeader) + ArrayStorage::sizeFor(0));
        
        if (allocatedNewStorage) {
            // We will set the vectorLength to newVectorLength. We populated requiredVectorLength
            // (usedVectorLength + count), which is less. Clear the difference.
            for (unsigned i = requiredVectorLength; i < newVectorLength; ++i)
                newButterfly->arrayStorage()->m_vector[i].clear();
        }
    } else if ((newAllocBase != butterfly->base(structure)) || (newIndexBias != storage->m_indexBias)) {
        memmove(newButterfly->propertyStorage() - propertySize, butterfly->propertyStorage() - propertySize, sizeof(JSValue) * propertySize + sizeof(IndexingHeader) + ArrayStorage::sizeFor(0));
        memmove(newButterfly->arrayStorage()->m_vector, storage->m_vector, sizeof(JSValue) * usedVectorLength);
        
        for (unsigned i = requiredVectorLength; i < newVectorLength; i++)
            newButterfly->arrayStorage()->m_vector[i].clear();
    }

    newButterfly->arrayStorage()->setVectorLength(newVectorLength);
    newButterfly->arrayStorage()->m_indexBias = newIndexBias;
    
    setButterfly(vm, newButterfly);

    return true;
}

bool JSArray::setLengthWithArrayStorage(ExecState* exec, unsigned newLength, bool throwException, ArrayStorage* storage)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    unsigned length = storage->length();
    
    // If the length is read only then we enter sparse mode, so should enter the following 'if'.
    ASSERT(isLengthWritable() || storage->m_sparseMap);

    if (SparseArrayValueMap* map = storage->m_sparseMap.get()) {
        // Fail if the length is not writable.
        if (map->lengthIsReadOnly())
            return typeError(exec, scope, throwException, ASCIILiteral(ReadonlyPropertyWriteError));

        if (newLength < length) {
            // Copy any keys we might be interested in into a vector.
            Vector<unsigned, 0, UnsafeVectorOverflow> keys;
            keys.reserveInitialCapacity(min(map->size(), static_cast<size_t>(length - newLength)));
            SparseArrayValueMap::const_iterator end = map->end();
            for (SparseArrayValueMap::const_iterator it = map->begin(); it != end; ++it) {
                unsigned index = static_cast<unsigned>(it->key);
                if (index < length && index >= newLength)
                    keys.append(index);
            }

            // Check if the array is in sparse mode. If so there may be non-configurable
            // properties, so we have to perform deletion with caution, if not we can
            // delete values in any order.
            if (map->sparseMode()) {
                qsort(keys.begin(), keys.size(), sizeof(unsigned), compareKeysForQSort);
                unsigned i = keys.size();
                while (i) {
                    unsigned index = keys[--i];
                    SparseArrayValueMap::iterator it = map->find(index);
                    ASSERT(it != map->notFound());
                    if (it->value.attributes & DontDelete) {
                        storage->setLength(index + 1);
                        return typeError(exec, scope, throwException, ASCIILiteral(UnableToDeletePropertyError));
                    }
                    map->remove(it);
                }
            } else {
                for (unsigned i = 0; i < keys.size(); ++i)
                    map->remove(keys[i]);
                if (map->isEmpty())
                    deallocateSparseIndexMap();
            }
        }
    }

    if (newLength < length) {
        // Delete properties from the vector.
        unsigned usedVectorLength = min(length, storage->vectorLength());
        for (unsigned i = newLength; i < usedVectorLength; ++i) {
            WriteBarrier<Unknown>& valueSlot = storage->m_vector[i];
            bool hadValue = !!valueSlot;
            valueSlot.clear();
            storage->m_numValuesInVector -= hadValue;
        }
    }

    storage->setLength(newLength);

    return true;
}

bool JSArray::appendMemcpy(ExecState* exec, VM& vm, unsigned startIndex, JSC::JSArray* otherArray)
{
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!canFastCopy(vm, otherArray))
        return false;

    IndexingType type = indexingType();
    IndexingType copyType = mergeIndexingTypeForCopying(otherArray->indexingType());
    if (type == ArrayWithUndecided && copyType != NonArray) {
        if (copyType == ArrayWithInt32)
            convertUndecidedToInt32(vm);
        else if (copyType == ArrayWithDouble)
            convertUndecidedToDouble(vm);
        else if (copyType == ArrayWithContiguous)
            convertUndecidedToContiguous(vm);
        else {
            ASSERT(copyType == ArrayWithUndecided);
            return true;
        }
    } else if (type != copyType)
        return false;

    unsigned otherLength = otherArray->length();
    Checked<unsigned, RecordOverflow> checkedNewLength = startIndex;
    checkedNewLength += otherLength;

    unsigned newLength;
    if (checkedNewLength.safeGet(newLength) == CheckedState::DidOverflow) {
        throwException(exec, scope, createRangeError(exec, ASCIILiteral(LengthExceededTheMaximumArrayLengthError)));
        return false;
    }

    if (newLength >= MIN_SPARSE_ARRAY_INDEX)
        return false;

    if (!ensureLength(vm, newLength)) {
        throwOutOfMemoryError(exec, scope);
        return false;
    }
    ASSERT(copyType == indexingType());

    if (type == ArrayWithDouble)
        memcpy(butterfly()->contiguousDouble().data() + startIndex, otherArray->butterfly()->contiguousDouble().data(), sizeof(JSValue) * otherLength);
    else
        memcpy(butterfly()->contiguous().data() + startIndex, otherArray->butterfly()->contiguous().data(), sizeof(JSValue) * otherLength);

    return true;
}

bool JSArray::setLength(ExecState* exec, unsigned newLength, bool throwException)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    Butterfly* butterfly = m_butterfly.get();
    switch (indexingType()) {
    case ArrayClass:
        if (!newLength)
            return true;
        if (newLength >= MIN_SPARSE_ARRAY_INDEX) {
            scope.release();
            return setLengthWithArrayStorage(
                exec, newLength, throwException,
                ensureArrayStorage(vm));
        }
        createInitialUndecided(vm, newLength);
        return true;
        
    case ArrayWithUndecided:
    case ArrayWithInt32:
    case ArrayWithDouble:
    case ArrayWithContiguous: {
        if (newLength == butterfly->publicLength())
            return true;
        if (newLength >= MAX_ARRAY_INDEX // This case ensures that we can do fast push.
            || (newLength >= MIN_SPARSE_ARRAY_INDEX
                && !isDenseEnoughForVector(newLength, countElements()))) {
            scope.release();
            return setLengthWithArrayStorage(
                exec, newLength, throwException,
                ensureArrayStorage(vm));
        }
        if (newLength > butterfly->publicLength()) {
            if (!ensureLength(vm, newLength)) {
                throwOutOfMemoryError(exec, scope);
                return false;
            }
            return true;
        }

        unsigned lengthToClear = butterfly->publicLength() - newLength;
        unsigned costToAllocateNewButterfly = 64; // a heuristic.
        if (lengthToClear > newLength && lengthToClear > costToAllocateNewButterfly) {
            reallocateAndShrinkButterfly(vm, newLength);
            return true;
        }

        if (indexingType() == ArrayWithDouble) {
            for (unsigned i = butterfly->publicLength(); i-- > newLength;)
                butterfly->contiguousDouble()[i] = PNaN;
        } else {
            for (unsigned i = butterfly->publicLength(); i-- > newLength;)
                butterfly->contiguous()[i].clear();
        }
        butterfly->setPublicLength(newLength);
        return true;
    }
        
    case ArrayWithArrayStorage:
    case ArrayWithSlowPutArrayStorage:
        scope.release();
        return setLengthWithArrayStorage(exec, newLength, throwException, arrayStorage());
        
    default:
        CRASH();
        return false;
    }
}

JSValue JSArray::pop(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    Butterfly* butterfly = m_butterfly.get();
    
    switch (indexingType()) {
    case ArrayClass:
        return jsUndefined();
        
    case ArrayWithUndecided:
        if (!butterfly->publicLength())
            return jsUndefined();
        // We have nothing but holes. So, drop down to the slow version.
        break;
        
    case ArrayWithInt32:
    case ArrayWithContiguous: {
        unsigned length = butterfly->publicLength();
        
        if (!length--)
            return jsUndefined();
        
        RELEASE_ASSERT(length < butterfly->vectorLength());
        JSValue value = butterfly->contiguous()[length].get();
        if (value) {
            butterfly->contiguous()[length].clear();
            butterfly->setPublicLength(length);
            return value;
        }
        break;
    }
        
    case ArrayWithDouble: {
        unsigned length = butterfly->publicLength();
        
        if (!length--)
            return jsUndefined();
        
        RELEASE_ASSERT(length < butterfly->vectorLength());
        double value = butterfly->contiguousDouble()[length];
        if (value == value) {
            butterfly->contiguousDouble()[length] = PNaN;
            butterfly->setPublicLength(length);
            return JSValue(JSValue::EncodeAsDouble, value);
        }
        break;
    }
        
    case ARRAY_WITH_ARRAY_STORAGE_INDEXING_TYPES: {
        ArrayStorage* storage = butterfly->arrayStorage();
    
        unsigned length = storage->length();
        if (!length) {
            if (!isLengthWritable())
                throwTypeError(exec, scope, ASCIILiteral(ReadonlyPropertyWriteError));
            return jsUndefined();
        }

        unsigned index = length - 1;
        if (index < storage->vectorLength()) {
            WriteBarrier<Unknown>& valueSlot = storage->m_vector[index];
            if (valueSlot) {
                --storage->m_numValuesInVector;
                JSValue element = valueSlot.get();
                valueSlot.clear();
            
                RELEASE_ASSERT(isLengthWritable());
                storage->setLength(index);
                return element;
            }
        }
        break;
    }
        
    default:
        CRASH();
        return JSValue();
    }
    
    unsigned index = getArrayLength() - 1;
    // Let element be the result of calling the [[Get]] internal method of O with argument indx.
    JSValue element = get(exec, index);
    RETURN_IF_EXCEPTION(scope, JSValue());
    // Call the [[Delete]] internal method of O with arguments indx and true.
    bool success = deletePropertyByIndex(this, exec, index);
    RETURN_IF_EXCEPTION(scope, JSValue());
    if (!success) {
        throwTypeError(exec, scope, ASCIILiteral(UnableToDeletePropertyError));
        return jsUndefined();
    }
    // Call the [[Put]] internal method of O with arguments "length", indx, and true.
    scope.release();
    setLength(exec, index, true);
    // Return element.
    return element;
}

// Push & putIndex are almost identical, with two small differences.
//  - we always are writing beyond the current array bounds, so it is always necessary to update m_length & m_numValuesInVector.
//  - pushing to an array of length 2^32-1 stores the property, but throws a range error.
void JSArray::push(ExecState* exec, JSValue value)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    Butterfly* butterfly = m_butterfly.get();
    
    switch (indexingType()) {
    case ArrayClass: {
        createInitialUndecided(vm, 0);
        FALLTHROUGH;
    }
        
    case ArrayWithUndecided: {
        convertUndecidedForValue(vm, value);
        scope.release();
        push(exec, value);
        return;
    }
        
    case ArrayWithInt32: {
        if (!value.isInt32()) {
            convertInt32ForValue(vm, value);
            scope.release();
            push(exec, value);
            return;
        }

        unsigned length = butterfly->publicLength();
        ASSERT(length <= butterfly->vectorLength());
        if (length < butterfly->vectorLength()) {
            butterfly->contiguousInt32()[length].setWithoutWriteBarrier(value);
            butterfly->setPublicLength(length + 1);
            return;
        }
        
        if (UNLIKELY(length > MAX_ARRAY_INDEX)) {
            methodTable(vm)->putByIndex(this, exec, length, value, true);
            if (!scope.exception())
                throwException(exec, scope, createRangeError(exec, ASCIILiteral(LengthExceededTheMaximumArrayLengthError)));
            return;
        }

        scope.release();
        putByIndexBeyondVectorLengthWithoutAttributes<Int32Shape>(exec, length, value);
        return;
    }

    case ArrayWithContiguous: {
        unsigned length = butterfly->publicLength();
        ASSERT(length <= butterfly->vectorLength());
        if (length < butterfly->vectorLength()) {
            butterfly->contiguous()[length].set(vm, this, value);
            butterfly->setPublicLength(length + 1);
            return;
        }
        
        if (UNLIKELY(length > MAX_ARRAY_INDEX)) {
            methodTable(vm)->putByIndex(this, exec, length, value, true);
            if (!scope.exception())
                throwException(exec, scope, createRangeError(exec, ASCIILiteral(LengthExceededTheMaximumArrayLengthError)));
            return;
        }

        scope.release();
        putByIndexBeyondVectorLengthWithoutAttributes<ContiguousShape>(exec, length, value);
        return;
    }
        
    case ArrayWithDouble: {
        if (!value.isNumber()) {
            convertDoubleToContiguous(vm);
            scope.release();
            push(exec, value);
            return;
        }
        double valueAsDouble = value.asNumber();
        if (valueAsDouble != valueAsDouble) {
            convertDoubleToContiguous(vm);
            scope.release();
            push(exec, value);
            return;
        }

        unsigned length = butterfly->publicLength();
        ASSERT(length <= butterfly->vectorLength());
        if (length < butterfly->vectorLength()) {
            butterfly->contiguousDouble()[length] = valueAsDouble;
            butterfly->setPublicLength(length + 1);
            return;
        }
        
        if (UNLIKELY(length > MAX_ARRAY_INDEX)) {
            methodTable(vm)->putByIndex(this, exec, length, value, true);
            if (!scope.exception())
                throwException(exec, scope, createRangeError(exec, ASCIILiteral(LengthExceededTheMaximumArrayLengthError)));
            return;
        }

        scope.release();
        putByIndexBeyondVectorLengthWithoutAttributes<DoubleShape>(exec, length, value);
        return;
    }
        
    case ArrayWithSlowPutArrayStorage: {
        unsigned oldLength = length();
        bool putResult = false;
        if (attemptToInterceptPutByIndexOnHole(exec, oldLength, value, true, putResult)) {
            if (!scope.exception() && oldLength < 0xFFFFFFFFu) {
                scope.release();
                setLength(exec, oldLength + 1, true);
            }
            return;
        }
        FALLTHROUGH;
    }
        
    case ArrayWithArrayStorage: {
        ArrayStorage* storage = butterfly->arrayStorage();

        // Fast case - push within vector, always update m_length & m_numValuesInVector.
        unsigned length = storage->length();
        if (length < storage->vectorLength()) {
            storage->m_vector[length].set(vm, this, value);
            storage->setLength(length + 1);
            ++storage->m_numValuesInVector;
            return;
        }

        // Pushing to an array of invalid length (2^31-1) stores the property, but throws a range error.
        if (UNLIKELY(storage->length() > MAX_ARRAY_INDEX)) {
            methodTable(vm)->putByIndex(this, exec, storage->length(), value, true);
            // Per ES5.1 15.4.4.7 step 6 & 15.4.5.1 step 3.d.
            if (!scope.exception())
                throwException(exec, scope, createRangeError(exec, ASCIILiteral(LengthExceededTheMaximumArrayLengthError)));
            return;
        }

        // Handled the same as putIndex.
        scope.release();
        putByIndexBeyondVectorLengthWithArrayStorage(exec, storage->length(), value, true, storage);
        return;
    }
        
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }
}

JSArray* JSArray::fastSlice(ExecState& exec, unsigned startIndex, unsigned count)
{
    auto arrayType = indexingType();
    switch (arrayType) {
    case ArrayWithDouble:
    case ArrayWithInt32:
    case ArrayWithContiguous: {
        VM& vm = exec.vm();
        if (count >= MIN_SPARSE_ARRAY_INDEX || structure(vm)->holesMustForwardToPrototype(vm))
            return nullptr;

        Structure* resultStructure = exec.lexicalGlobalObject()->arrayStructureForIndexingTypeDuringAllocation(arrayType);
        JSArray* resultArray = JSArray::tryCreateForInitializationPrivate(vm, resultStructure, count);
        if (!resultArray)
            return nullptr;

        auto& resultButterfly = *resultArray->butterfly();
        if (arrayType == ArrayWithDouble)
            memcpy(resultButterfly.contiguousDouble().data(), m_butterfly.get()->contiguousDouble().data() + startIndex, sizeof(JSValue) * count);
        else
            memcpy(resultButterfly.contiguous().data(), m_butterfly.get()->contiguous().data() + startIndex, sizeof(JSValue) * count);
        resultButterfly.setPublicLength(count);

        return resultArray;
    }
    default:
        return nullptr;
    }
}

bool JSArray::shiftCountWithArrayStorage(VM& vm, unsigned startIndex, unsigned count, ArrayStorage* storage)
{
    unsigned oldLength = storage->length();
    RELEASE_ASSERT(count <= oldLength);
    
    // If the array contains holes or is otherwise in an abnormal state,
    // use the generic algorithm in ArrayPrototype.
    if ((storage->hasHoles() && this->structure(vm)->holesMustForwardToPrototype(vm)) 
        || hasSparseMap() 
        || shouldUseSlowPut(indexingType())) {
        return false;
    }

    if (!oldLength)
        return true;
    
    unsigned length = oldLength - count;
    
    storage->m_numValuesInVector -= count;
    storage->setLength(length);
    
    unsigned vectorLength = storage->vectorLength();
    if (!vectorLength)
        return true;
    
    if (startIndex >= vectorLength)
        return true;
    
    DisallowGC disallowGC;
    auto locker = holdLock(*this);
    
    if (startIndex + count > vectorLength)
        count = vectorLength - startIndex;
    
    unsigned usedVectorLength = min(vectorLength, oldLength);
    
    unsigned numElementsBeforeShiftRegion = startIndex;
    unsigned firstIndexAfterShiftRegion = startIndex + count;
    unsigned numElementsAfterShiftRegion = usedVectorLength - firstIndexAfterShiftRegion;
    ASSERT(numElementsBeforeShiftRegion + count + numElementsAfterShiftRegion == usedVectorLength);

    // The point of this comparison seems to be to minimize the amount of elements that have to 
    // be moved during a shift operation.
    if (numElementsBeforeShiftRegion < numElementsAfterShiftRegion) {
        // The number of elements before the shift region is less than the number of elements
        // after the shift region, so we move the elements before to the right.
        if (numElementsBeforeShiftRegion) {
            RELEASE_ASSERT(count + startIndex <= vectorLength);
            if (storage->hasHoles()) {
                for (unsigned i = startIndex; i-- > 0;) {
                    unsigned destinationIndex = count + i;
                    JSValue source = storage->m_vector[i].get();
                    JSValue dest = storage->m_vector[destinationIndex].get();
                    // Any time we overwrite a hole we know we overcounted the number of values we removed 
                    // when we subtracted count from m_numValuesInVector above.
                    if (!dest && destinationIndex >= firstIndexAfterShiftRegion)
                        storage->m_numValuesInVector++;
                    storage->m_vector[count + i].setWithoutWriteBarrier(source);
                }
            } else {
                memmove(storage->m_vector + count,
                    storage->m_vector,
                    sizeof(JSValue) * startIndex);
            }
        }
        // Adjust the Butterfly and the index bias. We only need to do this here because we're changing
        // the start of the Butterfly, which needs to point at the first indexed property in the used
        // portion of the vector.
        Butterfly* butterfly = m_butterfly.get()->shift(structure(), count);
        setButterfly(vm, butterfly);
        storage = butterfly->arrayStorage();
        storage->m_indexBias += count;

        // Since we're consuming part of the vector by moving its beginning to the left,
        // we need to modify the vector length appropriately.
        storage->setVectorLength(vectorLength - count);
    } else {
        // The number of elements before the shift region is greater than or equal to the number 
        // of elements after the shift region, so we move the elements after the shift region to the left.
        if (storage->hasHoles()) {
            for (unsigned i = 0; i < numElementsAfterShiftRegion; ++i) {
                unsigned destinationIndex = startIndex + i;
                JSValue source = storage->m_vector[firstIndexAfterShiftRegion + i].get();
                JSValue dest = storage->m_vector[destinationIndex].get();
                // Any time we overwrite a hole we know we overcounted the number of values we removed 
                // when we subtracted count from m_numValuesInVector above.
                if (!dest && destinationIndex < firstIndexAfterShiftRegion)
                    storage->m_numValuesInVector++;
                storage->m_vector[startIndex + i].setWithoutWriteBarrier(source);
            }
        } else {
            memmove(storage->m_vector + startIndex,
                storage->m_vector + firstIndexAfterShiftRegion,
                sizeof(JSValue) * numElementsAfterShiftRegion);
        }
        // Clear the slots of the elements we just moved.
        unsigned startOfEmptyVectorTail = usedVectorLength - count;
        for (unsigned i = startOfEmptyVectorTail; i < usedVectorLength; ++i)
            storage->m_vector[i].clear();
        // We don't modify the index bias or the Butterfly pointer in this case because we're not changing 
        // the start of the Butterfly, which needs to point at the first indexed property in the used 
        // portion of the vector. We also don't modify the vector length because we're not actually changing
        // its length; we're just using less of it.
    }
    
    return true;
}

bool JSArray::shiftCountWithAnyIndexingType(ExecState* exec, unsigned& startIndex, unsigned count)
{
    VM& vm = exec->vm();
    RELEASE_ASSERT(count > 0);

    Butterfly* butterfly = m_butterfly.get();
    
    switch (indexingType()) {
    case ArrayClass:
        return true;
        
    case ArrayWithUndecided:
        // Don't handle this because it's confusing and it shouldn't come up.
        return false;
        
    case ArrayWithInt32:
    case ArrayWithContiguous: {
        unsigned oldLength = butterfly->publicLength();
        RELEASE_ASSERT(count <= oldLength);
        
        // We may have to walk the entire array to do the shift. We're willing to do
        // so only if it's not horribly slow.
        if (oldLength - (startIndex + count) >= MIN_SPARSE_ARRAY_INDEX)
            return shiftCountWithArrayStorage(vm, startIndex, count, ensureArrayStorage(vm));

        // Storing to a hole is fine since we're still having a good time. But reading from a hole 
        // is totally not fine, since we might have to read from the proto chain.
        // We have to check for holes before we start moving things around so that we don't get halfway 
        // through shifting and then realize we should have been in ArrayStorage mode.
        unsigned end = oldLength - count;
        if (this->structure(vm)->holesMustForwardToPrototype(vm)) {
            for (unsigned i = startIndex; i < end; ++i) {
                JSValue v = butterfly->contiguous()[i + count].get();
                if (UNLIKELY(!v)) {
                    startIndex = i;
                    return shiftCountWithArrayStorage(vm, startIndex, count, ensureArrayStorage(vm));
                }
                butterfly->contiguous()[i].setWithoutWriteBarrier(v);
            }
        } else {
            memmove(butterfly->contiguous().data() + startIndex, 
                butterfly->contiguous().data() + startIndex + count, 
                sizeof(JSValue) * (end - startIndex));
        }

        for (unsigned i = end; i < oldLength; ++i)
            butterfly->contiguous()[i].clear();
        
        butterfly->setPublicLength(oldLength - count);

        // Our memmoving of values around in the array could have concealed some of them from
        // the collector. Let's make sure that the collector scans this object again.
        vm.heap.writeBarrier(this);
        
        return true;
    }
        
    case ArrayWithDouble: {
        unsigned oldLength = butterfly->publicLength();
        RELEASE_ASSERT(count <= oldLength);
        
        // We may have to walk the entire array to do the shift. We're willing to do
        // so only if it's not horribly slow.
        if (oldLength - (startIndex + count) >= MIN_SPARSE_ARRAY_INDEX)
            return shiftCountWithArrayStorage(vm, startIndex, count, ensureArrayStorage(vm));

        // Storing to a hole is fine since we're still having a good time. But reading from a hole 
        // is totally not fine, since we might have to read from the proto chain.
        // We have to check for holes before we start moving things around so that we don't get halfway 
        // through shifting and then realize we should have been in ArrayStorage mode.
        unsigned end = oldLength - count;
        if (this->structure(vm)->holesMustForwardToPrototype(vm)) {
            for (unsigned i = startIndex; i < end; ++i) {
                double v = butterfly->contiguousDouble()[i + count];
                if (UNLIKELY(v != v)) {
                    startIndex = i;
                    return shiftCountWithArrayStorage(vm, startIndex, count, ensureArrayStorage(vm));
                }
                butterfly->contiguousDouble()[i] = v;
            }
        } else {
            memmove(butterfly->contiguousDouble().data() + startIndex,
                butterfly->contiguousDouble().data() + startIndex + count,
                sizeof(JSValue) * (end - startIndex));
        }
        for (unsigned i = end; i < oldLength; ++i)
            butterfly->contiguousDouble()[i] = PNaN;
        
        butterfly->setPublicLength(oldLength - count);
        return true;
    }
        
    case ArrayWithArrayStorage:
    case ArrayWithSlowPutArrayStorage:
        return shiftCountWithArrayStorage(vm, startIndex, count, arrayStorage());
        
    default:
        CRASH();
        return false;
    }
}

// Returns true if the unshift can be handled, false to fallback.    
bool JSArray::unshiftCountWithArrayStorage(ExecState* exec, unsigned startIndex, unsigned count, ArrayStorage* storage)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    unsigned length = storage->length();

    RELEASE_ASSERT(startIndex <= length);

    // If the array contains holes or is otherwise in an abnormal state,
    // use the generic algorithm in ArrayPrototype.
    if (storage->hasHoles() || storage->inSparseMode() || shouldUseSlowPut(indexingType()))
        return false;

    bool moveFront = !startIndex || startIndex < length / 2;

    unsigned vectorLength = storage->vectorLength();

    // Need to have GC deferred around the unshiftCountSlowCase(), since that leaves the butterfly in
    // a weird state: some parts of it will be left uninitialized, which we will fill in here.
    DeferGC deferGC(vm.heap);
    auto locker = holdLock(*this);
    
    if (moveFront && storage->m_indexBias >= count) {
        Butterfly* newButterfly = storage->butterfly()->unshift(structure(), count);
        storage = newButterfly->arrayStorage();
        storage->m_indexBias -= count;
        storage->setVectorLength(vectorLength + count);
        setButterfly(vm, newButterfly);
    } else if (!moveFront && vectorLength - length >= count)
        storage = storage->butterfly()->arrayStorage();
    else if (unshiftCountSlowCase(locker, vm, deferGC, moveFront, count))
        storage = arrayStorage();
    else {
        throwOutOfMemoryError(exec, scope);
        return true;
    }

    WriteBarrier<Unknown>* vector = storage->m_vector;

    if (startIndex) {
        if (moveFront)
            memmove(vector, vector + count, startIndex * sizeof(JSValue));
        else if (length - startIndex)
            memmove(vector + startIndex + count, vector + startIndex, (length - startIndex) * sizeof(JSValue));
    }

    for (unsigned i = 0; i < count; i++)
        vector[i + startIndex].clear();
    
    return true;
}

bool JSArray::unshiftCountWithAnyIndexingType(ExecState* exec, unsigned startIndex, unsigned count)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    Butterfly* butterfly = m_butterfly.get();
    
    switch (indexingType()) {
    case ArrayClass:
    case ArrayWithUndecided:
        // We could handle this. But it shouldn't ever come up, so we won't.
        return false;

    case ArrayWithInt32:
    case ArrayWithContiguous: {
        unsigned oldLength = butterfly->publicLength();
        
        // We may have to walk the entire array to do the unshift. We're willing to do so
        // only if it's not horribly slow.
        if (oldLength - startIndex >= MIN_SPARSE_ARRAY_INDEX) {
            scope.release();
            return unshiftCountWithArrayStorage(exec, startIndex, count, ensureArrayStorage(vm));
        }
        
        if (!ensureLength(vm, oldLength + count)) {
            throwOutOfMemoryError(exec, scope);
            return false;
        }
        butterfly = m_butterfly.get();

        // We have to check for holes before we start moving things around so that we don't get halfway 
        // through shifting and then realize we should have been in ArrayStorage mode.
        for (unsigned i = oldLength; i-- > startIndex;) {
            JSValue v = butterfly->contiguous()[i].get();
            if (UNLIKELY(!v)) {
                scope.release();
                return unshiftCountWithArrayStorage(exec, startIndex, count, ensureArrayStorage(vm));
            }
        }

        for (unsigned i = oldLength; i-- > startIndex;) {
            JSValue v = butterfly->contiguous()[i].get();
            ASSERT(v);
            butterfly->contiguous()[i + count].setWithoutWriteBarrier(v);
        }
        
        // Our memmoving of values around in the array could have concealed some of them from
        // the collector. Let's make sure that the collector scans this object again.
        vm.heap.writeBarrier(this);
        
        // NOTE: we're leaving being garbage in the part of the array that we shifted out
        // of. This is fine because the caller is required to store over that area, and
        // in contiguous mode storing into a hole is guaranteed to behave exactly the same
        // as storing over an existing element.
        
        return true;
    }
        
    case ArrayWithDouble: {
        unsigned oldLength = butterfly->publicLength();
        
        // We may have to walk the entire array to do the unshift. We're willing to do so
        // only if it's not horribly slow.
        if (oldLength - startIndex >= MIN_SPARSE_ARRAY_INDEX) {
            scope.release();
            return unshiftCountWithArrayStorage(exec, startIndex, count, ensureArrayStorage(vm));
        }
        
        if (!ensureLength(vm, oldLength + count)) {
            throwOutOfMemoryError(exec, scope);
            return false;
        }
        butterfly = m_butterfly.get();
        
        // We have to check for holes before we start moving things around so that we don't get halfway 
        // through shifting and then realize we should have been in ArrayStorage mode.
        for (unsigned i = oldLength; i-- > startIndex;) {
            double v = butterfly->contiguousDouble()[i];
            if (UNLIKELY(v != v)) {
                scope.release();
                return unshiftCountWithArrayStorage(exec, startIndex, count, ensureArrayStorage(vm));
            }
        }

        for (unsigned i = oldLength; i-- > startIndex;) {
            double v = butterfly->contiguousDouble()[i];
            ASSERT(v == v);
            butterfly->contiguousDouble()[i + count] = v;
        }
        
        // NOTE: we're leaving being garbage in the part of the array that we shifted out
        // of. This is fine because the caller is required to store over that area, and
        // in contiguous mode storing into a hole is guaranteed to behave exactly the same
        // as storing over an existing element.
        
        return true;
    }
        
    case ArrayWithArrayStorage:
    case ArrayWithSlowPutArrayStorage:
        scope.release();
        return unshiftCountWithArrayStorage(exec, startIndex, count, arrayStorage());
        
    default:
        CRASH();
        return false;
    }
}

void JSArray::fillArgList(ExecState* exec, MarkedArgumentBuffer& args)
{
    unsigned i = 0;
    unsigned vectorEnd;
    WriteBarrier<Unknown>* vector;

    Butterfly* butterfly = m_butterfly.get();
    
    switch (indexingType()) {
    case ArrayClass:
        return;
        
    case ArrayWithUndecided: {
        vector = 0;
        vectorEnd = 0;
        break;
    }
        
    case ArrayWithInt32:
    case ArrayWithContiguous: {
        vectorEnd = butterfly->publicLength();
        vector = butterfly->contiguous().data();
        break;
    }
        
    case ArrayWithDouble: {
        vector = 0;
        vectorEnd = 0;
        for (; i < butterfly->publicLength(); ++i) {
            double v = butterfly->contiguousDouble()[i];
            if (v != v)
                break;
            args.append(JSValue(JSValue::EncodeAsDouble, v));
        }
        break;
    }
    
    case ARRAY_WITH_ARRAY_STORAGE_INDEXING_TYPES: {
        ArrayStorage* storage = butterfly->arrayStorage();
        
        vector = storage->m_vector;
        vectorEnd = min(storage->length(), storage->vectorLength());
        break;
    }
        
    default:
        CRASH();
#if COMPILER_QUIRK(CONSIDERS_UNREACHABLE_CODE)
        vector = 0;
        vectorEnd = 0;
        break;
#endif
    }
    
    for (; i < vectorEnd; ++i) {
        WriteBarrier<Unknown>& v = vector[i];
        if (!v)
            break;
        args.append(v.get());
    }

    // FIXME: What prevents this from being called with a RuntimeArray? The length function will always return 0 in that case.
    for (; i < length(); ++i)
        args.append(get(exec, i));
}

void JSArray::copyToArguments(ExecState* exec, VirtualRegister firstElementDest, unsigned offset, unsigned length)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    unsigned i = offset;
    WriteBarrier<Unknown>* vector;
    unsigned vectorEnd;
    length += offset; // We like to think of the length as being our length, rather than the output length.

    // FIXME: What prevents this from being called with a RuntimeArray? The length function will always return 0 in that case.
    ASSERT(length == this->length());

    Butterfly* butterfly = m_butterfly.get();
    switch (indexingType()) {
    case ArrayClass:
        return;
        
    case ArrayWithUndecided: {
        vector = 0;
        vectorEnd = 0;
        break;
    }
        
    case ArrayWithInt32:
    case ArrayWithContiguous: {
        vector = butterfly->contiguous().data();
        vectorEnd = butterfly->publicLength();
        break;
    }
        
    case ArrayWithDouble: {
        vector = 0;
        vectorEnd = 0;
        for (; i < butterfly->publicLength(); ++i) {
            ASSERT(i < butterfly->vectorLength());
            double v = butterfly->contiguousDouble()[i];
            if (v != v)
                break;
            exec->r(firstElementDest + i - offset) = JSValue(JSValue::EncodeAsDouble, v);
        }
        break;
    }
        
    case ARRAY_WITH_ARRAY_STORAGE_INDEXING_TYPES: {
        ArrayStorage* storage = butterfly->arrayStorage();
        vector = storage->m_vector;
        vectorEnd = min(length, storage->vectorLength());
        break;
    }
        
    default:
        CRASH();
#if COMPILER_QUIRK(CONSIDERS_UNREACHABLE_CODE)
        vector = 0;
        vectorEnd = 0;
        break;
#endif
    }
    
    for (; i < vectorEnd; ++i) {
        WriteBarrier<Unknown>& v = vector[i];
        if (!v)
            break;
        exec->r(firstElementDest + i - offset) = v.get();
    }
    
    for (; i < length; ++i) {
        exec->r(firstElementDest + i - offset) = get(exec, i);
        RETURN_IF_EXCEPTION(scope, void());
    }
}

} // namespace JSC
