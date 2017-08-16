/*
 * Copyright (C) 2013-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "JSDataView.h"

#include "ArrayBufferView.h"
#include "DataView.h"
#include "Error.h"
#include "JSCInlines.h"
#include "TypeError.h"

namespace JSC {

const ClassInfo JSDataView::s_info = {
    "DataView", &Base::s_info, 0, CREATE_METHOD_TABLE(JSDataView)};

JSDataView::JSDataView(VM& vm, ConstructionContext& context, ArrayBuffer* buffer)
    : Base(vm, context)
    , m_buffer(buffer)
{
}

JSDataView* JSDataView::create(
    ExecState* exec, Structure* structure, RefPtr<ArrayBuffer>&& buffer,
    unsigned byteOffset, unsigned byteLength)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    ASSERT(buffer);
    if (!ArrayBufferView::verifySubRangeLength(*buffer, byteOffset, byteLength, sizeof(uint8_t))) {
        throwVMError(exec, scope, createRangeError(exec, ASCIILiteral("Length out of range of buffer")));
        return nullptr;
    }
    if (!ArrayBufferView::verifyByteOffsetAlignment(byteOffset, sizeof(uint8_t))) {
        throwException(exec, scope, createRangeError(exec, ASCIILiteral("Byte offset is not aligned")));
        return nullptr;
    }
    ConstructionContext context(
        structure, buffer.copyRef(), byteOffset, byteLength, ConstructionContext::DataView);
    ASSERT(context);
    JSDataView* result =
        new (NotNull, allocateCell<JSDataView>(vm.heap)) JSDataView(vm, context, buffer.get());
    result->finishCreation(vm);
    return result;
}

JSDataView* JSDataView::createUninitialized(ExecState*, Structure*, unsigned)
{
    UNREACHABLE_FOR_PLATFORM();
    return 0;
}

JSDataView* JSDataView::create(ExecState*, Structure*, unsigned)
{
    UNREACHABLE_FOR_PLATFORM();
    return 0;
}

bool JSDataView::set(ExecState*, unsigned, JSObject*, unsigned, unsigned)
{
    UNREACHABLE_FOR_PLATFORM();
    return false;
}

bool JSDataView::setIndex(ExecState*, unsigned, JSValue)
{
    UNREACHABLE_FOR_PLATFORM();
    return false;
}

RefPtr<DataView> JSDataView::possiblySharedTypedImpl()
{
    return DataView::create(possiblySharedBuffer(), byteOffset(), length());
}

RefPtr<DataView> JSDataView::unsharedTypedImpl()
{
    return DataView::create(unsharedBuffer(), byteOffset(), length());
}

bool JSDataView::getOwnPropertySlot(
    JSObject* object, ExecState* exec, PropertyName propertyName, PropertySlot& slot)
{
    JSDataView* thisObject = jsCast<JSDataView*>(object);
    if (propertyName == exec->propertyNames().byteLength) {
        slot.setValue(thisObject, DontEnum | ReadOnly, jsNumber(thisObject->m_length));
        return true;
    }
    if (propertyName == exec->propertyNames().byteOffset) {
        slot.setValue(thisObject, DontEnum | ReadOnly, jsNumber(thisObject->byteOffset()));
        return true;
    }

    return Base::getOwnPropertySlot(thisObject, exec, propertyName, slot);
}

bool JSDataView::put(
    JSCell* cell, ExecState* exec, PropertyName propertyName, JSValue value,
    PutPropertySlot& slot)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSDataView* thisObject = jsCast<JSDataView*>(cell);

    if (UNLIKELY(isThisValueAltered(slot, thisObject))) {
        scope.release();
        return ordinarySetSlow(exec, thisObject, propertyName, value, slot.thisValue(), slot.isStrictMode());
    }

    if (propertyName == vm.propertyNames->byteLength
        || propertyName == vm.propertyNames->byteOffset)
        return typeError(exec, scope, slot.isStrictMode(), ASCIILiteral("Attempting to write to read-only typed array property."));

    scope.release();
    return Base::put(thisObject, exec, propertyName, value, slot);
}

bool JSDataView::defineOwnProperty(
    JSObject* object, ExecState* exec, PropertyName propertyName,
    const PropertyDescriptor& descriptor, bool shouldThrow)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSDataView* thisObject = jsCast<JSDataView*>(object);
    if (propertyName == vm.propertyNames->byteLength
        || propertyName == vm.propertyNames->byteOffset)
        return typeError(exec, scope, shouldThrow, ASCIILiteral("Attempting to define read-only typed array property."));

    return Base::defineOwnProperty(thisObject, exec, propertyName, descriptor, shouldThrow);
}

bool JSDataView::deleteProperty(
    JSCell* cell, ExecState* exec, PropertyName propertyName)
{
    JSDataView* thisObject = jsCast<JSDataView*>(cell);
    if (propertyName == exec->propertyNames().byteLength
        || propertyName == exec->propertyNames().byteOffset)
        return false;

    return Base::deleteProperty(thisObject, exec, propertyName);
}

void JSDataView::getOwnNonIndexPropertyNames(
    JSObject* object, ExecState* exec, PropertyNameArray& array, EnumerationMode mode)
{
    JSDataView* thisObject = jsCast<JSDataView*>(object);
    
    if (mode.includeDontEnumProperties()) {
        array.add(exec->propertyNames().byteOffset);
        array.add(exec->propertyNames().byteLength);
    }
    
    Base::getOwnNonIndexPropertyNames(thisObject, exec, array, mode);
}

ArrayBuffer* JSDataView::slowDownAndWasteMemory(JSArrayBufferView*)
{
    UNREACHABLE_FOR_PLATFORM();
    return 0;
}

RefPtr<ArrayBufferView> JSDataView::getTypedArrayImpl(JSArrayBufferView* object)
{
    JSDataView* thisObject = jsCast<JSDataView*>(object);
    return thisObject->possiblySharedTypedImpl();
}

Structure* JSDataView::createStructure(
    VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(
        vm, globalObject, prototype, TypeInfo(DataViewType, StructureFlags), info(),
        NonArray);
}

} // namespace JSC

