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
#include "SimpleTypedArrayController.h"

#include "ArrayBuffer.h"
#include "JSArrayBuffer.h"
#include "JSCInlines.h"

namespace JSC {

SimpleTypedArrayController::SimpleTypedArrayController() { }
SimpleTypedArrayController::~SimpleTypedArrayController() { }

JSArrayBuffer* SimpleTypedArrayController::toJS(
    ExecState* exec, JSGlobalObject* globalObject, ArrayBuffer* native)
{
    if (JSArrayBuffer* buffer = native->m_wrapper.get())
        return buffer;

    // The JSArrayBuffer::create function will register the wrapper in finishCreation.
    JSArrayBuffer* result = JSArrayBuffer::create(
        exec->vm(), globalObject->arrayBufferStructure(native->sharingMode()), native);
    return result;
}

void SimpleTypedArrayController::registerWrapper(JSGlobalObject*, ArrayBuffer* native, JSArrayBuffer* wrapper)
{
    ASSERT(!native->m_wrapper);
    native->m_wrapper = Weak<JSArrayBuffer>(wrapper, &m_owner);
}

bool SimpleTypedArrayController::isAtomicsWaitAllowedOnCurrentThread()
{
    return true;
}

bool SimpleTypedArrayController::JSArrayBufferOwner::isReachableFromOpaqueRoots(JSC::Handle<JSC::Unknown> handle, void*, JSC::SlotVisitor& visitor)
{
    auto& wrapper = *JSC::jsCast<JSC::JSArrayBuffer*>(handle.slot()->asCell());
    return visitor.containsOpaqueRoot(wrapper.impl());
}

void SimpleTypedArrayController::JSArrayBufferOwner::finalize(JSC::Handle<JSC::Unknown>, void*) { }

} // namespace JSC

