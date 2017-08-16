/*
 * Copyright (C) 2013, 2016 Apple Inc. All rights reserved.
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
#include "JSArrayBuffer.h"

#include "JSCInlines.h"
#include "TypeError.h"
#include "TypedArrayController.h"

namespace JSC {

const ClassInfo JSArrayBuffer::s_info = {
    "ArrayBuffer", &Base::s_info, 0, CREATE_METHOD_TABLE(JSArrayBuffer)};

JSArrayBuffer::JSArrayBuffer(VM& vm, Structure* structure, RefPtr<ArrayBuffer>&& arrayBuffer)
    : Base(vm, structure)
    , m_impl(arrayBuffer.get())
{
}

void JSArrayBuffer::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    // This probably causes GCs in the various VMs to overcount the impact of the array buffer.
    vm.heap.addReference(this, m_impl);
    vm.m_typedArrayController->registerWrapper(globalObject, m_impl, this);
}

JSArrayBuffer* JSArrayBuffer::create(
    VM& vm, Structure* structure, RefPtr<ArrayBuffer>&& buffer)
{
    JSArrayBuffer* result =
        new (NotNull, allocateCell<JSArrayBuffer>(vm.heap))
        JSArrayBuffer(vm, structure, WTFMove(buffer));
    result->finishCreation(vm, structure->globalObject());
    return result;
}

Structure* JSArrayBuffer::createStructure(
    VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(
        vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info(),
        NonArray);
}

bool JSArrayBuffer::isShared() const
{
    return m_impl->isShared();
}

ArrayBufferSharingMode JSArrayBuffer::sharingMode() const
{
    return m_impl->sharingMode();
}

size_t JSArrayBuffer::estimatedSize(JSCell* cell)
{
    JSArrayBuffer* thisObject = jsCast<JSArrayBuffer*>(cell);
    size_t bufferEstimatedSize = thisObject->impl()->gcSizeEstimateInBytes();
    return Base::estimatedSize(cell) + bufferEstimatedSize;
}

} // namespace JSC

