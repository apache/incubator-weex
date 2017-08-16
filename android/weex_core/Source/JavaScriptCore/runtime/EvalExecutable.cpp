/*
 * Copyright (C) 2009, 2010, 2013, 2015-2016 Apple Inc. All rights reserved.
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
#include "EvalExecutable.h"

#include "EvalCodeBlock.h"
#include "JSCInlines.h"

namespace JSC {

const ClassInfo EvalExecutable::s_info = { "EvalExecutable", &ScriptExecutable::s_info, 0, CREATE_METHOD_TABLE(EvalExecutable) };

EvalExecutable::EvalExecutable(ExecState* exec, const SourceCode& source, bool inStrictContext, DerivedContextType derivedContextType, bool isArrowFunctionContext, EvalContextType evalContextType)
    : ScriptExecutable(exec->vm().evalExecutableStructure.get(), exec->vm(), source, inStrictContext, derivedContextType, isArrowFunctionContext, evalContextType, NoIntrinsic)
{
    ASSERT(source.provider()->sourceType() == SourceProviderSourceType::Program);
}

void EvalExecutable::destroy(JSCell* cell)
{
    static_cast<EvalExecutable*>(cell)->EvalExecutable::~EvalExecutable();
}

void EvalExecutable::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    EvalExecutable* thisObject = jsCast<EvalExecutable*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    ScriptExecutable::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_unlinkedEvalCodeBlock);
    if (EvalCodeBlock* evalCodeBlock = thisObject->m_evalCodeBlock.get())
        evalCodeBlock->visitWeakly(visitor);
}

} // namespace JSC
