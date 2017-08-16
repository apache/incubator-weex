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
#include "Exception.h"

#include "Interpreter.h"
#include "JSCInlines.h"

namespace JSC {

const ClassInfo Exception::s_info = { "Exception", &Base::s_info, 0, CREATE_METHOD_TABLE(Exception) };

Exception* Exception::create(VM& vm, JSValue thrownValue, StackCaptureAction action)
{
    Exception* result = new (NotNull, allocateCell<Exception>(vm.heap)) Exception(vm);
    result->finishCreation(vm, thrownValue, action);
    return result;
}

void Exception::destroy(JSCell* cell)
{
    Exception* exception = static_cast<Exception*>(cell);
    exception->~Exception();
}

Structure* Exception::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void Exception::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    Exception* thisObject = jsCast<Exception*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);

    visitor.append(thisObject->m_value);
}

Exception::Exception(VM& vm)
    : Base(vm, vm.exceptionStructure.get())
{
}

Exception::~Exception()
{
}

void Exception::finishCreation(VM& vm, JSValue thrownValue, StackCaptureAction action)
{
    Base::finishCreation(vm);

    m_value.set(vm, this, thrownValue);

    Vector<StackFrame> stackTrace;
    if (action == StackCaptureAction::CaptureStack)
        vm.interpreter->getStackTrace(stackTrace);
    m_stack = WTFMove(stackTrace);
}

} // namespace JSC
