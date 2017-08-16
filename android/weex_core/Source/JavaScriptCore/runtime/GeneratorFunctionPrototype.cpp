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
#include "GeneratorFunctionPrototype.h"

#include "BuiltinExecutables.h"
#include "BuiltinNames.h"
#include "Error.h"
#include "JSArray.h"
#include "JSCInlines.h"
#include "JSFunction.h"
#include "JSString.h"
#include "JSStringBuilder.h"
#include "Lexer.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(GeneratorFunctionPrototype);

const ClassInfo GeneratorFunctionPrototype::s_info = { "GeneratorFunction", &Base::s_info, nullptr, CREATE_METHOD_TABLE(GeneratorFunctionPrototype) };

GeneratorFunctionPrototype::GeneratorFunctionPrototype(VM& vm, Structure* structure)
    : JSNonFinalObject(vm, structure)
{
}

void GeneratorFunctionPrototype::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(0), ReadOnly | DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->toStringTagSymbol, jsString(&vm, "GeneratorFunction"), DontEnum | ReadOnly);
    vm.prototypeMap.addPrototype(this);
}

} // namespace JSC
