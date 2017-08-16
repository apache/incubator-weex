/*
 * Copyright (C) 2013, 2016 Apple, Inc. All rights reserved.
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
#include "JSSetIterator.h"

#include "JSCInlines.h"
#include "JSSet.h"

namespace JSC {

const ClassInfo JSSetIterator::s_info = { "Set Iterator", &Base::s_info, 0, CREATE_METHOD_TABLE(JSSetIterator) };

void JSSetIterator::finishCreation(VM& vm, JSSet* iteratedObject)
{
    Base::finishCreation(vm);
    m_set.set(vm, this, iteratedObject);
    setIterator(vm, m_set->impl()->head());
}

void JSSetIterator::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSSetIterator* thisObject = jsCast<JSSetIterator*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_set);
    visitor.append(thisObject->m_iter);
}

JSValue JSSetIterator::createPair(CallFrame* callFrame, JSValue key, JSValue value)
{
    MarkedArgumentBuffer args;
    args.append(key);
    args.append(value);
    JSGlobalObject* globalObject = callFrame->jsCallee()->globalObject();
    return constructArray(callFrame, 0, globalObject, args);
}

JSSetIterator* JSSetIterator::clone(ExecState* exec)
{
    VM& vm = exec->vm();
    auto clone = JSSetIterator::create(vm, exec->jsCallee()->globalObject()->setIteratorStructure(), m_set.get(), m_kind);
    clone->setIterator(vm, m_iter.get());
    return clone;
}

}
