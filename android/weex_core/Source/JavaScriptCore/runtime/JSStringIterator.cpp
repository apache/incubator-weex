/*
 * Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>.
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "JSStringIterator.h"

#include "BuiltinNames.h"
#include "JSCInlines.h"

namespace JSC {

const ClassInfo JSStringIterator::s_info = { "String Iterator", &Base::s_info, 0, CREATE_METHOD_TABLE(JSStringIterator) };

void JSStringIterator::finishCreation(VM& vm, JSGlobalObject*, JSString* iteratedString)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    putDirect(vm, vm.propertyNames->builtinNames().iteratedStringPrivateName(), iteratedString);
    putDirect(vm, vm.propertyNames->builtinNames().stringIteratorNextIndexPrivateName(), jsNumber(0));
}

JSValue JSStringIterator::iteratedValue(ExecState* exec) const
{
    return getDirect(exec->vm(), exec->vm().propertyNames->builtinNames().iteratedStringPrivateName());
}

JSStringIterator* JSStringIterator::clone(ExecState* exec)
{
    VM& vm = exec->vm();
    JSValue iteratedString = getDirect(vm, vm.propertyNames->builtinNames().iteratedStringPrivateName());
    JSValue nextIndex = getDirect(vm, vm.propertyNames->builtinNames().stringIteratorNextIndexPrivateName());

    auto clone = JSStringIterator::create(exec, exec->jsCallee()->globalObject()->stringIteratorStructure(), asString(iteratedString));
    clone->putDirect(vm, vm.propertyNames->builtinNames().stringIteratorNextIndexPrivateName(), nextIndex);
    return clone;
}

} // namespace JSC
