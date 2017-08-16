/*
 * Copyright (C) 2016 Caitlin Potter <caitp@igalia.com>.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "JSAsyncFunction.h"

#include "Error.h"
#include "JSCInlines.h"
#include "JSCJSValue.h"
#include "JSFunction.h"
#include "JSFunctionInlines.h"
#include "JSObject.h"
#include "PropertySlot.h"
#include "VM.h"

namespace JSC {

const ClassInfo JSAsyncFunction::s_info = { "AsyncFunction",  &Base::s_info, nullptr, CREATE_METHOD_TABLE(JSAsyncFunction) };

JSAsyncFunction::JSAsyncFunction(VM& vm, FunctionExecutable* executable, JSScope* scope, Structure* structure)
    : Base(vm, executable, scope, structure)
{
}

JSAsyncFunction* JSAsyncFunction::createImpl(VM& vm, FunctionExecutable* executable, JSScope* scope, Structure* structure)
{
    JSAsyncFunction* asyncFunction = new (NotNull, allocateCell<JSAsyncFunction>(vm.heap)) JSAsyncFunction(vm, executable, scope, structure);
    ASSERT(asyncFunction->structure()->globalObject());
    asyncFunction->finishCreation(vm);
    return asyncFunction;
}

JSAsyncFunction* JSAsyncFunction::create(VM& vm, FunctionExecutable* executable, JSScope* scope)
{
    JSAsyncFunction* asyncFunction = createImpl(vm, executable, scope, scope->globalObject()->asyncFunctionStructure());
    executable->singletonFunction()->notifyWrite(vm, asyncFunction, "Allocating an async function");
    return asyncFunction;
}

JSAsyncFunction* JSAsyncFunction::create(VM& vm, FunctionExecutable* executable, JSScope* scope, Structure* structure)
{
    JSAsyncFunction* asyncFunction = createImpl(vm, executable, scope, structure);
    executable->singletonFunction()->notifyWrite(vm, asyncFunction, "Allocating an async function");
    return asyncFunction;
}

JSAsyncFunction* JSAsyncFunction::createWithInvalidatedReallocationWatchpoint(VM& vm, FunctionExecutable* executable, JSScope* scope)
{
    return createImpl(vm, executable, scope, scope->globalObject()->asyncFunctionStructure());
}

}
