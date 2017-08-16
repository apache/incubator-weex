/*
 * Copyright (C) 2014, 2016 Apple Inc. All rights reserved.
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
#include "JSCallee.h"

#include "GetterSetter.h"
#include "JSCInlines.h"
#include "JSGlobalObject.h"
#include "StackVisitor.h"

namespace JSC {

const ClassInfo JSCallee::s_info = { "Callee", &Base::s_info, 0, CREATE_METHOD_TABLE(JSCallee) };

JSCallee::JSCallee(VM& vm, JSGlobalObject* globalObject, Structure* structure)
    : Base(vm, structure)
    , m_scope(vm, this, globalObject)
{
    RELEASE_ASSERT(!isLargeAllocation());
}

JSCallee::JSCallee(VM& vm, JSScope* scope, Structure* structure)
    : Base(vm, structure)
{
    setScope(vm, scope);
}

void JSCallee::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
}

void JSCallee::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSCallee* thisObject = jsCast<JSCallee*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);

    visitor.append(thisObject->m_scope);
}

} // namespace JSC
