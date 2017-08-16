/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "JSNativeStdFunction.h"

#include "JSCInlines.h"
#include "JSFunction.h"
#include "JSFunctionInlines.h"
#include "JSObject.h"
#include "NativeStdFunctionCell.h"
#include "VM.h"

namespace JSC {

const ClassInfo JSNativeStdFunction::s_info = { "Function", &Base::s_info, nullptr, CREATE_METHOD_TABLE(JSNativeStdFunction) };

JSNativeStdFunction::JSNativeStdFunction(VM& vm, JSGlobalObject* globalObject, Structure* structure)
    : Base(vm, globalObject, structure)
{
}

void JSNativeStdFunction::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSNativeStdFunction* thisObject = jsCast<JSNativeStdFunction*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_functionCell);
}

void JSNativeStdFunction::finishCreation(VM& vm, NativeExecutable* executable, int length, const String& name, NativeStdFunctionCell* functionCell)
{
    Base::finishCreation(vm, executable, length, name);
    ASSERT(inherits(vm, info()));
    m_functionCell.set(vm, this, functionCell);
}

static EncodedJSValue JSC_HOST_CALL runStdFunction(ExecState* state)
{
    JSNativeStdFunction* function = jsCast<JSNativeStdFunction*>(state->jsCallee());
    ASSERT(function);
    return function->nativeStdFunctionCell()->function()(state);
}

JSNativeStdFunction* JSNativeStdFunction::create(VM& vm, JSGlobalObject* globalObject, int length, const String& name, NativeStdFunction&& nativeStdFunction, Intrinsic intrinsic, NativeFunction nativeConstructor)
{
    NativeExecutable* executable = vm.getHostFunction(runStdFunction, intrinsic, nativeConstructor, nullptr, name);
    NativeStdFunctionCell* functionCell = NativeStdFunctionCell::create(vm, WTFMove(nativeStdFunction));
    Structure* structure = globalObject->nativeStdFunctionStructure();
    JSNativeStdFunction* function = new (NotNull, allocateCell<JSNativeStdFunction>(vm.heap)) JSNativeStdFunction(vm, globalObject, structure);
    function->finishCreation(vm, executable, length, name, functionCell);
    return function;
}

} // namespace JSC
