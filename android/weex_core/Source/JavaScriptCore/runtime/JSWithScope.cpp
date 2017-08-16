/*
 * Copyright (C) 2012, 2016 Apple Inc. All Rights Reserved.
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
#include "JSWithScope.h"

#include "JSCInlines.h"

namespace JSC {

const ClassInfo JSWithScope::s_info = { "WithScope", &Base::s_info, 0, CREATE_METHOD_TABLE(JSWithScope) };

JSWithScope* JSWithScope::create(
    VM& vm, JSGlobalObject* globalObject, JSObject* object, JSScope* next)
{
    Structure* structure = globalObject->withScopeStructure();
    JSWithScope* withScope = new (NotNull, allocateCell<JSWithScope>(vm.heap)) JSWithScope(vm, structure, object, next);
    withScope->finishCreation(vm);
    return withScope;
}

void JSWithScope::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSWithScope* thisObject = jsCast<JSWithScope*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_object);
}

Structure* JSWithScope::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
{
    return Structure::create(vm, globalObject, proto, TypeInfo(WithScopeType, StructureFlags), info());
}

JSWithScope::JSWithScope(VM& vm, Structure* structure, JSObject* object, JSScope* next)
    : Base(vm, structure, next)
    , m_object(vm, this, object)
{
}

} // namespace JSC
