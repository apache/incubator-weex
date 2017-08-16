/*
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
#include "WebAssemblyRuntimeErrorPrototype.h"

#if ENABLE(WEBASSEMBLY)

#include "FunctionPrototype.h"
#include "JSCInlines.h"

#include "WebAssemblyRuntimeErrorPrototype.lut.h"

namespace JSC {

const ClassInfo WebAssemblyRuntimeErrorPrototype::s_info = { "WebAssembly.RuntimeError.prototype", &Base::s_info, &prototypeTableWebAssemblyRuntimeError, CREATE_METHOD_TABLE(WebAssemblyRuntimeErrorPrototype) };

/* Source for WebAssemblyRuntimeErrorPrototype.lut.h
 @begin prototypeTableWebAssemblyRuntimeError
 @end
 */

WebAssemblyRuntimeErrorPrototype* WebAssemblyRuntimeErrorPrototype::create(VM& vm, JSGlobalObject*, Structure* structure)
{
    auto* object = new (NotNull, allocateCell<WebAssemblyRuntimeErrorPrototype>(vm.heap)) WebAssemblyRuntimeErrorPrototype(vm, structure);
    object->finishCreation(vm);
    return object;
}

Structure* WebAssemblyRuntimeErrorPrototype::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void WebAssemblyRuntimeErrorPrototype::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
}

WebAssemblyRuntimeErrorPrototype::WebAssemblyRuntimeErrorPrototype(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
