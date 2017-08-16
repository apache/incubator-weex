/*
 * Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>.
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
#include "GeneratorFunctionConstructor.h"

#include "FunctionConstructor.h"
#include "GeneratorFunctionPrototype.h"
#include "JSCInlines.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(GeneratorFunctionConstructor);

const ClassInfo GeneratorFunctionConstructor::s_info = { "GeneratorFunction", &Base::s_info, nullptr, CREATE_METHOD_TABLE(GeneratorFunctionConstructor) };

GeneratorFunctionConstructor::GeneratorFunctionConstructor(VM& vm, Structure* structure)
    : InternalFunction(vm, structure)
{
}

void GeneratorFunctionConstructor::finishCreation(VM& vm, GeneratorFunctionPrototype* generatorFunctionPrototype)
{
    Base::finishCreation(vm, "GeneratorFunction");
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, generatorFunctionPrototype, DontEnum | DontDelete | ReadOnly);

    // Number of arguments for constructor
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(1), ReadOnly | DontEnum);
}

static EncodedJSValue JSC_HOST_CALL callGeneratorFunctionConstructor(ExecState* exec)
{
    ArgList args(exec);
    return JSValue::encode(constructFunction(exec, asInternalFunction(exec->jsCallee())->globalObject(), args, FunctionConstructionMode::Generator));
}

static EncodedJSValue JSC_HOST_CALL constructGeneratorFunctionConstructor(ExecState* exec)
{
    ArgList args(exec);
    return JSValue::encode(constructFunction(exec, asInternalFunction(exec->jsCallee())->globalObject(), args, FunctionConstructionMode::Generator, exec->newTarget()));
}

CallType GeneratorFunctionConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callGeneratorFunctionConstructor;
    return CallType::Host;
}

ConstructType GeneratorFunctionConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructGeneratorFunctionConstructor;
    return ConstructType::Host;
}

} // namespace JSC
