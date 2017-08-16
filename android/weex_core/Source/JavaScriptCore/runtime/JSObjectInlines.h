/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003-2017 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Eric Seidel (eric@webkit.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "AuxiliaryBarrierInlines.h"
#include "Error.h"
#include "JSObject.h"
#include "Lookup.h"

namespace JSC {

// Section 7.3.17 of the spec.
template <typename AddFunction> // Add function should have a type like: (JSValue, RuntimeType) -> bool
void createListFromArrayLike(ExecState* exec, JSValue arrayLikeValue, RuntimeTypeMask legalTypesFilter, const String& errorMessage, AddFunction addFunction)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    
    Vector<JSValue> result;
    JSValue lengthProperty = arrayLikeValue.get(exec, vm.propertyNames->length);
    RETURN_IF_EXCEPTION(scope, void());
    double lengthAsDouble = lengthProperty.toLength(exec);
    RETURN_IF_EXCEPTION(scope, void());
    RELEASE_ASSERT(lengthAsDouble >= 0.0 && lengthAsDouble == std::trunc(lengthAsDouble));
    uint64_t length = static_cast<uint64_t>(lengthAsDouble);
    for (uint64_t index = 0; index < length; index++) {
        JSValue next = arrayLikeValue.get(exec, index);
        RETURN_IF_EXCEPTION(scope, void());
        
        RuntimeType type = runtimeTypeForValue(next);
        if (!(type & legalTypesFilter)) {
            throwTypeError(exec, scope, errorMessage);
            return;
        }
        
        bool exitEarly = addFunction(next, type);
        if (exitEarly)
            return;
    }
}

ALWAYS_INLINE bool JSObject::canPerformFastPutInline(ExecState* exec, VM& vm, PropertyName propertyName)
{
    if (UNLIKELY(propertyName == exec->propertyNames().underscoreProto))
        return false;

    // Check if there are any setters or getters in the prototype chain
    JSValue prototype;
    JSObject* obj = this;
    while (true) {
        if (obj->structure(vm)->hasReadOnlyOrGetterSetterPropertiesExcludingProto() || obj->type() == ProxyObjectType)
            return false;

        prototype = obj->getPrototypeDirect();
        if (prototype.isNull())
            return true;

        obj = asObject(prototype);
    }

    ASSERT_NOT_REACHED();
}

template<typename CallbackWhenNoException>
ALWAYS_INLINE typename std::result_of<CallbackWhenNoException(bool, PropertySlot&)>::type JSObject::getPropertySlot(ExecState* exec, PropertyName propertyName, CallbackWhenNoException callback) const
{
    PropertySlot slot(this, PropertySlot::InternalMethodType::Get);
    return getPropertySlot(exec, propertyName, slot, callback);
}

template<typename CallbackWhenNoException>
ALWAYS_INLINE typename std::result_of<CallbackWhenNoException(bool, PropertySlot&)>::type JSObject::getPropertySlot(ExecState* exec, PropertyName propertyName, PropertySlot& slot, CallbackWhenNoException callback) const
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    bool found = const_cast<JSObject*>(this)->getPropertySlot(exec, propertyName, slot);
    RETURN_IF_EXCEPTION(scope, { });
    return callback(found, slot);
}

ALWAYS_INLINE bool JSObject::getPropertySlot(ExecState* exec, unsigned propertyName, PropertySlot& slot)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    auto& structureIDTable = vm.heap.structureIDTable();
    JSObject* object = this;
    MethodTable::GetPrototypeFunctionPtr defaultGetPrototype = JSObject::getPrototype;
    while (true) {
        Structure* structure = structureIDTable.get(object->structureID());
        if (structure->classInfo()->methodTable.getOwnPropertySlotByIndex(object, exec, propertyName, slot))
            return true;
        RETURN_IF_EXCEPTION(scope, false);
        JSValue prototype;
        if (LIKELY(structure->classInfo()->methodTable.getPrototype == defaultGetPrototype || slot.internalMethodType() == PropertySlot::InternalMethodType::VMInquiry))
            prototype = structure->storedPrototype();
        else {
            prototype = object->getPrototype(vm, exec);
            RETURN_IF_EXCEPTION(scope, false);
        }
        if (!prototype.isObject())
            return false;
        object = asObject(prototype);
    }
}

ALWAYS_INLINE bool JSObject::getNonIndexPropertySlot(ExecState* exec, PropertyName propertyName, PropertySlot& slot)
{
    // This method only supports non-index PropertyNames.
    ASSERT(!parseIndex(propertyName));

    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    auto& structureIDTable = vm.heap.structureIDTable();
    JSObject* object = this;
    MethodTable::GetPrototypeFunctionPtr defaultGetPrototype = JSObject::getPrototype;
    while (true) {
        Structure* structure = structureIDTable.get(object->structureID());
        if (LIKELY(!TypeInfo::overridesGetOwnPropertySlot(object->inlineTypeFlags()))) {
            if (object->getOwnNonIndexPropertySlot(vm, structure, propertyName, slot))
                return true;
        } else {
            if (structure->classInfo()->methodTable.getOwnPropertySlot(object, exec, propertyName, slot))
                return true;
            RETURN_IF_EXCEPTION(scope, false);
        }
        JSValue prototype;
        if (LIKELY(structure->classInfo()->methodTable.getPrototype == defaultGetPrototype || slot.internalMethodType() == PropertySlot::InternalMethodType::VMInquiry))
            prototype = structure->storedPrototype();
        else {
            prototype = object->getPrototype(vm, exec);
            RETURN_IF_EXCEPTION(scope, false);
        }
        if (!prototype.isObject())
            return false;
        object = asObject(prototype);
    }
}

inline void JSObject::putDirectWithoutTransition(VM& vm, PropertyName propertyName, JSValue value, unsigned attributes)
{
    ASSERT(!value.isGetterSetter() && !(attributes & Accessor));
    ASSERT(!value.isCustomGetterSetter());
    StructureID structureID = this->structureID();
    Structure* structure = vm.heap.structureIDTable().get(structureID);
    PropertyOffset offset = prepareToPutDirectWithoutTransition(vm, propertyName, attributes, structureID, structure);
    bool shouldOptimize = false;
    structure->willStoreValueForNewTransition(vm, propertyName, value, shouldOptimize);
    putDirect(vm, offset, value);
    if (attributes & ReadOnly)
        structure->setContainsReadOnlyProperties();
}

ALWAYS_INLINE PropertyOffset JSObject::prepareToPutDirectWithoutTransition(VM& vm, PropertyName propertyName, unsigned attributes, StructureID structureID, Structure* structure)
{
    unsigned oldOutOfLineCapacity = structure->outOfLineCapacity();
    PropertyOffset result;
    structure->addPropertyWithoutTransition(
        vm, propertyName, attributes,
        [&] (const GCSafeConcurrentJSLocker&, PropertyOffset offset, PropertyOffset newLastOffset) {
            unsigned newOutOfLineCapacity = Structure::outOfLineCapacity(newLastOffset);
            if (newOutOfLineCapacity != oldOutOfLineCapacity) {
                Butterfly* butterfly = allocateMoreOutOfLineStorage(vm, oldOutOfLineCapacity, newOutOfLineCapacity);
                nukeStructureAndSetButterfly(vm, structureID, butterfly);
                structure->setLastOffset(newLastOffset);
                WTF::storeStoreFence();
                setStructureIDDirectly(structureID);
            } else
                structure->setLastOffset(newLastOffset);
            result = offset;
        });
    return result;
}

// ECMA 8.6.2.2
ALWAYS_INLINE bool JSObject::putInline(JSCell* cell, ExecState* exec, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSObject* thisObject = jsCast<JSObject*>(cell);
    ASSERT(value);
    ASSERT(!Heap::heap(value) || Heap::heap(value) == Heap::heap(thisObject));

    if (UNLIKELY(isThisValueAltered(slot, thisObject)))
        return ordinarySetSlow(exec, thisObject, propertyName, value, slot.thisValue(), slot.isStrictMode());

    // Try indexed put first. This is required for correctness, since loads on property names that appear like
    // valid indices will never look in the named property storage.
    if (std::optional<uint32_t> index = parseIndex(propertyName))
        return putByIndex(thisObject, exec, index.value(), value, slot.isStrictMode());

    if (thisObject->canPerformFastPutInline(exec, vm, propertyName)) {
        ASSERT(!thisObject->structure(vm)->prototypeChainMayInterceptStoreTo(vm, propertyName));
        if (!thisObject->putDirectInternal<PutModePut>(vm, propertyName, value, 0, slot))
            return typeError(exec, scope, slot.isStrictMode(), ASCIILiteral(ReadonlyPropertyWriteError));
        return true;
    }

    return thisObject->putInlineSlow(exec, propertyName, value, slot);
}

// HasOwnProperty(O, P) from section 7.3.11 in the spec.
// http://www.ecma-international.org/ecma-262/6.0/index.html#sec-hasownproperty
ALWAYS_INLINE bool JSObject::hasOwnProperty(ExecState* exec, PropertyName propertyName, PropertySlot& slot) const
{
    ASSERT(slot.internalMethodType() == PropertySlot::InternalMethodType::GetOwnProperty);
    if (LIKELY(const_cast<JSObject*>(this)->methodTable(exec->vm())->getOwnPropertySlot == JSObject::getOwnPropertySlot))
        return JSObject::getOwnPropertySlot(const_cast<JSObject*>(this), exec, propertyName, slot);
    return const_cast<JSObject*>(this)->methodTable(exec->vm())->getOwnPropertySlot(const_cast<JSObject*>(this), exec, propertyName, slot);
}

ALWAYS_INLINE bool JSObject::hasOwnProperty(ExecState* exec, PropertyName propertyName) const
{
    PropertySlot slot(this, PropertySlot::InternalMethodType::GetOwnProperty);
    return hasOwnProperty(exec, propertyName, slot);
}

ALWAYS_INLINE bool JSObject::hasOwnProperty(ExecState* exec, unsigned propertyName) const
{
    PropertySlot slot(this, PropertySlot::InternalMethodType::GetOwnProperty);
    return const_cast<JSObject*>(this)->methodTable(exec->vm())->getOwnPropertySlotByIndex(const_cast<JSObject*>(this), exec, propertyName, slot);
}

template<JSObject::PutMode mode>
ALWAYS_INLINE bool JSObject::putDirectInternal(VM& vm, PropertyName propertyName, JSValue value, unsigned attributes, PutPropertySlot& slot)
{
    ASSERT(value);
    ASSERT(value.isGetterSetter() == !!(attributes & Accessor));
    ASSERT(!Heap::heap(value) || Heap::heap(value) == Heap::heap(this));
    ASSERT(!parseIndex(propertyName));

    StructureID structureID = this->structureID();
    Structure* structure = vm.heap.structureIDTable().get(structureID);
    if (structure->isDictionary()) {
        ASSERT(!structure->hasInferredTypes());
        
        unsigned currentAttributes;
        PropertyOffset offset = structure->get(vm, propertyName, currentAttributes);
        if (offset != invalidOffset) {
            if ((mode == PutModePut) && currentAttributes & ReadOnly)
                return false;

            putDirect(vm, offset, value);
            structure->didReplaceProperty(offset);
            slot.setExistingProperty(this, offset);

            if ((attributes & Accessor) != (currentAttributes & Accessor) || (attributes & CustomAccessor) != (currentAttributes & CustomAccessor)) {
                ASSERT(!(attributes & ReadOnly));
                setStructure(vm, Structure::attributeChangeTransition(vm, structure, propertyName, attributes));
            }
            return true;
        }

        if ((mode == PutModePut) && !isStructureExtensible())
            return false;

        offset = prepareToPutDirectWithoutTransition(vm, propertyName, attributes, structureID, structure);
        validateOffset(offset);
        putDirect(vm, offset, value);
        slot.setNewProperty(this, offset);
        if (attributes & ReadOnly)
            this->structure()->setContainsReadOnlyProperties();
        return true;
    }

    PropertyOffset offset;
    size_t currentCapacity = this->structure()->outOfLineCapacity();
    Structure* newStructure = Structure::addPropertyTransitionToExistingStructure(
        structure, propertyName, attributes, offset);
    if (newStructure) {
        newStructure->willStoreValueForExistingTransition(
            vm, propertyName, value, slot.context() == PutPropertySlot::PutById);
        
        Butterfly* newButterfly = butterfly();
        if (currentCapacity != newStructure->outOfLineCapacity()) {
            ASSERT(newStructure != this->structure());
            newButterfly = allocateMoreOutOfLineStorage(vm, currentCapacity, newStructure->outOfLineCapacity());
            nukeStructureAndSetButterfly(vm, structureID, newButterfly);
        }

        validateOffset(offset);
        ASSERT(newStructure->isValidOffset(offset));
        putDirect(vm, offset, value);
        setStructure(vm, newStructure);
        slot.setNewProperty(this, offset);
        return true;
    }

    unsigned currentAttributes;
    bool hasInferredType;
    offset = structure->get(vm, propertyName, currentAttributes, hasInferredType);
    if (offset != invalidOffset) {
        if ((mode == PutModePut) && currentAttributes & ReadOnly)
            return false;

        structure->didReplaceProperty(offset);
        if (UNLIKELY(hasInferredType)) {
            structure->willStoreValueForReplace(
                vm, propertyName, value, slot.context() == PutPropertySlot::PutById);
        }

        slot.setExistingProperty(this, offset);
        putDirect(vm, offset, value);

        if ((attributes & Accessor) != (currentAttributes & Accessor) || (attributes & CustomAccessor) != (currentAttributes & CustomAccessor)) {
            ASSERT(!(attributes & ReadOnly));
            setStructure(vm, Structure::attributeChangeTransition(vm, structure, propertyName, attributes));
        }
        return true;
    }

    if ((mode == PutModePut) && !isStructureExtensible())
        return false;

    // We want the structure transition watchpoint to fire after this object has switched
    // structure. This allows adaptive watchpoints to observe if the new structure is the one
    // we want.
    DeferredStructureTransitionWatchpointFire deferredWatchpointFire;
    
    newStructure = Structure::addNewPropertyTransition(
        vm, structure, propertyName, attributes, offset, slot.context(), &deferredWatchpointFire);
    newStructure->willStoreValueForNewTransition(
        vm, propertyName, value, slot.context() == PutPropertySlot::PutById);
    
    validateOffset(offset);
    ASSERT(newStructure->isValidOffset(offset));
    size_t oldCapacity = structure->outOfLineCapacity();
    size_t newCapacity = newStructure->outOfLineCapacity();
    ASSERT(oldCapacity <= newCapacity);
    if (oldCapacity != newCapacity) {
        Butterfly* newButterfly = allocateMoreOutOfLineStorage(vm, oldCapacity, newCapacity);
        nukeStructureAndSetButterfly(vm, structureID, newButterfly);
    }
    putDirect(vm, offset, value);
    setStructure(vm, newStructure);
    slot.setNewProperty(this, offset);
    if (attributes & ReadOnly)
        newStructure->setContainsReadOnlyProperties();
    return true;
}

} // namespace JSC
