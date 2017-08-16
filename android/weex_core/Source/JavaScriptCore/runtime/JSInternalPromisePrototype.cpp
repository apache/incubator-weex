/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "JSInternalPromisePrototype.h"

#include "Error.h"
#include "JSCBuiltins.h"
#include "JSCInlines.h"
#include "JSGlobalObject.h"
#include "JSInternalPromise.h"
#include "Microtask.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(JSInternalPromisePrototype);

const ClassInfo JSInternalPromisePrototype::s_info = { "InternalPromisePrototype", &Base::s_info, nullptr, CREATE_METHOD_TABLE(JSInternalPromisePrototype) };

JSInternalPromisePrototype* JSInternalPromisePrototype::create(VM& vm, JSGlobalObject*, Structure* structure)
{
    JSInternalPromisePrototype* object = new (NotNull, allocateCell<JSInternalPromisePrototype>(vm.heap)) JSInternalPromisePrototype(vm, structure);
    object->finishCreation(vm, structure);
    return object;
}

Structure* JSInternalPromisePrototype::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

JSInternalPromisePrototype::JSInternalPromisePrototype(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

} // namespace JSC
