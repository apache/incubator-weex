/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003-2017 Apple Inc. All rights reserved.
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

#pragma once

#include "ArrayConventions.h"
#include "ButterflyInlines.h"
#include "JSCellInlines.h"
#include "JSObject.h"

namespace JSC {

class JSArray;
class LLIntOffsetsExtractor;

class JSArray : public JSNonFinalObject {
    friend class LLIntOffsetsExtractor;
    friend class Walker;
    friend class JIT;

public:
    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | OverridesGetPropertyNames;

    static size_t allocationSize(size_t inlineCapacity)
    {
        ASSERT_UNUSED(inlineCapacity, !inlineCapacity);
        return sizeof(JSArray);
    }
        
protected:
    explicit JSArray(VM& vm, Structure* structure, Butterfly* butterfly)
        : JSNonFinalObject(vm, structure, butterfly)
    {
    }

public:
    static JSArray* tryCreate(VM&, Structure*, unsigned initialLength = 0);
    static JSArray* create(VM&, Structure*, unsigned initialLength = 0);
    static JSArray* createWithButterfly(VM&, GCDeferralContext*, Structure*, Butterfly*);

    // tryCreateForInitializationPrivate is used for fast construction of arrays whose size and
    // contents are known at time of creation. This should be considered a private API.
    // Clients of this interface must:
    //   - null-check the result (indicating out of memory, or otherwise unable to allocate vector).
    //   - call 'initializeIndex' for all properties in sequence, for 0 <= i < initialLength.
    //   - Provide a valid GCDefferalContext* if they might garbage collect when initializing properties,
    //     otherwise the caller can provide a null GCDefferalContext*.
    //
    JS_EXPORT_PRIVATE static JSArray* tryCreateForInitializationPrivate(VM&, GCDeferralContext*, Structure*, unsigned initialLength);
    static JSArray* tryCreateForInitializationPrivate(VM& vm, Structure* structure, unsigned initialLength)
    {
        return tryCreateForInitializationPrivate(vm, nullptr, structure, initialLength);
    }

    JS_EXPORT_PRIVATE static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool throwException);

    JS_EXPORT_PRIVATE static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);

    DECLARE_EXPORT_INFO;

    // OK if we know this is a JSArray, but not if it could be an object of a derived class; for RuntimeArray this always returns 0.
    unsigned length() const { return getArrayLength(); }

    // OK to use on new arrays, but not if it might be a RegExpMatchArray or RuntimeArray.
    JS_EXPORT_PRIVATE bool setLength(ExecState*, unsigned, bool throwException = false);

    JS_EXPORT_PRIVATE void push(ExecState*, JSValue);
    JS_EXPORT_PRIVATE JSValue pop(ExecState*);

    JSArray* fastSlice(ExecState&, unsigned startIndex, unsigned count);

    bool canFastCopy(VM&, JSArray* otherArray);
    // This function returns NonArray if the indexing types are not compatable for copying.
    IndexingType mergeIndexingTypeForCopying(IndexingType other);
    bool appendMemcpy(ExecState*, VM&, unsigned startIndex, JSArray* otherArray);

    enum ShiftCountMode {
        // This form of shift hints that we're doing queueing. With this assumption in hand,
        // we convert to ArrayStorage, which has queue optimizations.
        ShiftCountForShift,
            
        // This form of shift hints that we're just doing care and feeding on an array that
        // is probably typically used for ordinary accesses. With this assumption in hand,
        // we try to preserve whatever indexing type it has already.
        ShiftCountForSplice
    };

    bool shiftCountForShift(ExecState* exec, unsigned startIndex, unsigned count)
    {
        return shiftCountWithArrayStorage(exec->vm(), startIndex, count, ensureArrayStorage(exec->vm()));
    }
    bool shiftCountForSplice(ExecState* exec, unsigned& startIndex, unsigned count)
    {
        return shiftCountWithAnyIndexingType(exec, startIndex, count);
    }
    template<ShiftCountMode shiftCountMode>
    bool shiftCount(ExecState* exec, unsigned& startIndex, unsigned count)
    {
        switch (shiftCountMode) {
        case ShiftCountForShift:
            return shiftCountForShift(exec, startIndex, count);
        case ShiftCountForSplice:
            return shiftCountForSplice(exec, startIndex, count);
        default:
            CRASH();
            return false;
        }
    }
        
    bool unshiftCountForShift(ExecState* exec, unsigned startIndex, unsigned count)
    {
        return unshiftCountWithArrayStorage(exec, startIndex, count, ensureArrayStorage(exec->vm()));
    }
    bool unshiftCountForSplice(ExecState* exec, unsigned startIndex, unsigned count)
    {
        return unshiftCountWithAnyIndexingType(exec, startIndex, count);
    }
    template<ShiftCountMode shiftCountMode>
    bool unshiftCount(ExecState* exec, unsigned startIndex, unsigned count)
    {
        switch (shiftCountMode) {
        case ShiftCountForShift:
            return unshiftCountForShift(exec, startIndex, count);
        case ShiftCountForSplice:
            return unshiftCountForSplice(exec, startIndex, count);
        default:
            CRASH();
            return false;
        }
    }

    JS_EXPORT_PRIVATE void fillArgList(ExecState*, MarkedArgumentBuffer&);
    JS_EXPORT_PRIVATE void copyToArguments(ExecState*, VirtualRegister firstElementDest, unsigned offset, unsigned length);

    bool isIteratorProtocolFastAndNonObservable();

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype, IndexingType indexingType)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ArrayType, StructureFlags), info(), indexingType);
    }
        
protected:
    void finishCreation(VM& vm)
    {
        Base::finishCreation(vm);
        ASSERT_WITH_MESSAGE(type() == ArrayType || type() == DerivedArrayType, "Instance inheriting JSArray should have either ArrayType or DerivedArrayType");
    }

    static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);

    static bool deleteProperty(JSCell*, ExecState*, PropertyName);
    JS_EXPORT_PRIVATE static void getOwnNonIndexPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);

private:
    bool isLengthWritable()
    {
        ArrayStorage* storage = arrayStorageOrNull();
        if (!storage)
            return true;
        SparseArrayValueMap* map = storage->m_sparseMap.get();
        return !map || !map->lengthIsReadOnly();
    }
        
    bool shiftCountWithAnyIndexingType(ExecState*, unsigned& startIndex, unsigned count);
    JS_EXPORT_PRIVATE bool shiftCountWithArrayStorage(VM&, unsigned startIndex, unsigned count, ArrayStorage*);

    bool unshiftCountWithAnyIndexingType(ExecState*, unsigned startIndex, unsigned count);
    bool unshiftCountWithArrayStorage(ExecState*, unsigned startIndex, unsigned count, ArrayStorage*);
    bool unshiftCountSlowCase(const AbstractLocker&, VM&, DeferGC&, bool, unsigned);

    bool setLengthWithArrayStorage(ExecState*, unsigned newLength, bool throwException, ArrayStorage*);
    void setLengthWritable(ExecState*, bool writable);
};

inline Butterfly* tryCreateArrayButterfly(VM& vm, JSCell* intendedOwner, unsigned initialLength)
{
    Butterfly* butterfly = Butterfly::tryCreate(
        vm, intendedOwner, 0, 0, true, baseIndexingHeaderForArrayStorage(initialLength),
        ArrayStorage::sizeFor(BASE_ARRAY_STORAGE_VECTOR_LEN));
    if (!butterfly)
        return nullptr;
    ArrayStorage* storage = butterfly->arrayStorage();
    storage->m_sparseMap.clear();
    storage->m_indexBias = 0;
    storage->m_numValuesInVector = 0;
    return butterfly;
}

inline Butterfly* createArrayButterfly(VM& vm, JSCell* intendedOwner, unsigned initialLength)
{
    Butterfly* result = tryCreateArrayButterfly(vm, intendedOwner, initialLength);
    RELEASE_ASSERT(result);
    return result;
}

Butterfly* createArrayButterflyInDictionaryIndexingMode(
    VM&, JSCell* intendedOwner, unsigned initialLength);

inline JSArray* JSArray::tryCreate(VM& vm, Structure* structure, unsigned initialLength)
{
    unsigned outOfLineStorage = structure->outOfLineCapacity();

    Butterfly* butterfly;
    IndexingType indexingType = structure->indexingType();
    if (LIKELY(!hasAnyArrayStorage(indexingType))) {
        ASSERT(
            hasUndecided(indexingType)
            || hasInt32(indexingType)
            || hasDouble(indexingType)
            || hasContiguous(indexingType));

        if (initialLength > MAX_STORAGE_VECTOR_LENGTH)
            return 0;

        unsigned vectorLength = Butterfly::optimalContiguousVectorLength(structure, initialLength);
        void* temp = vm.auxiliarySpace.tryAllocate(nullptr, Butterfly::totalSize(0, outOfLineStorage, true, vectorLength * sizeof(EncodedJSValue)));
        if (!temp)
            return nullptr;
        butterfly = Butterfly::fromBase(temp, 0, outOfLineStorage);
        butterfly->setVectorLength(vectorLength);
        butterfly->setPublicLength(initialLength);
        if (hasDouble(indexingType))
            clearArray(butterfly->contiguousDouble().data(), vectorLength);
        else
            clearArray(butterfly->contiguous().data(), vectorLength);
    } else {
        ASSERT(
            indexingType == ArrayWithSlowPutArrayStorage
            || indexingType == ArrayWithArrayStorage);
        butterfly = tryCreateArrayButterfly(vm, 0, initialLength);
        if (!butterfly)
            return nullptr;
        for (unsigned i = 0; i < BASE_ARRAY_STORAGE_VECTOR_LEN; ++i)
            butterfly->arrayStorage()->m_vector[i].clear();
    }

    return createWithButterfly(vm, nullptr, structure, butterfly);
}

inline JSArray* JSArray::create(VM& vm, Structure* structure, unsigned initialLength)
{
    JSArray* result = JSArray::tryCreate(vm, structure, initialLength);
    RELEASE_ASSERT(result);

    return result;
}

inline JSArray* JSArray::createWithButterfly(VM& vm, GCDeferralContext* deferralContext, Structure* structure, Butterfly* butterfly)
{
    JSArray* array = new (NotNull, allocateCell<JSArray>(vm.heap, deferralContext)) JSArray(vm, structure, butterfly);
    array->finishCreation(vm);
    return array;
}

JSArray* asArray(JSValue);

inline JSArray* asArray(JSCell* cell)
{
    ASSERT(cell->inherits(*cell->vm(), JSArray::info()));
    return jsCast<JSArray*>(cell);
}

inline JSArray* asArray(JSValue value)
{
    return asArray(value.asCell());
}

inline bool isJSArray(JSCell* cell)
{
    ASSERT((cell->classInfo(*cell->vm()) == JSArray::info()) == (cell->type() == ArrayType));
    return cell->type() == ArrayType;
}

inline bool isJSArray(JSValue v) { return v.isCell() && isJSArray(v.asCell()); }

inline JSArray* constructArray(ExecState* exec, Structure* arrayStructure, const ArgList& values)
{
    VM& vm = exec->vm();
    unsigned length = values.size();
    JSArray* array = JSArray::tryCreateForInitializationPrivate(vm, arrayStructure, length);

    // FIXME: we should probably throw an out of memory error here, but
    // when making this change we should check that all clients of this
    // function will correctly handle an exception being thrown from here.
    RELEASE_ASSERT(array);

    for (unsigned i = 0; i < length; ++i)
        array->initializeIndex(vm, i, values.at(i));
    return array;
}
    
inline JSArray* constructArray(ExecState* exec, Structure* arrayStructure, const JSValue* values, unsigned length)
{
    VM& vm = exec->vm();
    JSArray* array = JSArray::tryCreateForInitializationPrivate(vm, arrayStructure, length);

    // FIXME: we should probably throw an out of memory error here, but
    // when making this change we should check that all clients of this
    // function will correctly handle an exception being thrown from here.
    RELEASE_ASSERT(array);

    for (unsigned i = 0; i < length; ++i)
        array->initializeIndex(vm, i, values[i]);
    return array;
}

inline JSArray* constructArrayNegativeIndexed(ExecState* exec, Structure* arrayStructure, const JSValue* values, unsigned length)
{
    VM& vm = exec->vm();
    JSArray* array = JSArray::tryCreateForInitializationPrivate(vm, arrayStructure, length);

    // FIXME: we should probably throw an out of memory error here, but
    // when making this change we should check that all clients of this
    // function will correctly handle an exception being thrown from here.
    RELEASE_ASSERT(array);

    for (int i = 0; i < static_cast<int>(length); ++i)
        array->initializeIndex(vm, i, values[-i]);
    return array;
}

} // namespace JSC
