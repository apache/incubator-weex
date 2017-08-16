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
#include "DirectEvalExecutable.h"

#include "CodeCache.h"
#include "Debugger.h"
#include "Error.h"
#include "HeapInlines.h"
#include "JSCJSValueInlines.h"
#include "ParserError.h"

namespace JSC {

DirectEvalExecutable* DirectEvalExecutable::create(ExecState* exec, const SourceCode& source, bool isInStrictContext, DerivedContextType derivedContextType, bool isArrowFunctionContext, EvalContextType evalContextType, const VariableEnvironment* variablesUnderTDZ)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    if (!globalObject->evalEnabled()) {
        throwException(exec, scope, createEvalError(exec, globalObject->evalDisabledErrorMessage()));
        return 0;
    }

    auto* executable = new (NotNull, allocateCell<DirectEvalExecutable>(*exec->heap())) DirectEvalExecutable(exec, source, isInStrictContext, derivedContextType, isArrowFunctionContext, evalContextType);
    executable->finishCreation(vm);

    ParserError error;
    JSParserStrictMode strictMode = executable->isStrictMode() ? JSParserStrictMode::Strict : JSParserStrictMode::NotStrict;
    DebuggerMode debuggerMode = globalObject->hasInteractiveDebugger() ? DebuggerOn : DebuggerOff;

    // We don't bother with CodeCache here because direct eval uses a specialized DirectEvalCodeCache.
    UnlinkedEvalCodeBlock* unlinkedEvalCode = generateUnlinkedCodeBlock<UnlinkedEvalCodeBlock>(
        vm, executable, executable->source(), strictMode, JSParserScriptMode::Classic, debuggerMode, error, evalContextType, variablesUnderTDZ);

    if (globalObject->hasDebugger())
        globalObject->debugger()->sourceParsed(exec, executable->source().provider(), error.line(), error.message());

    if (error.isValid()) {
        throwVMError(exec, scope, error.toErrorObject(globalObject, executable->source()));
        return nullptr;
    }

    executable->m_unlinkedEvalCodeBlock.set(vm, executable, unlinkedEvalCode);

    return executable;
}

DirectEvalExecutable::DirectEvalExecutable(ExecState* exec, const SourceCode& source, bool inStrictContext, DerivedContextType derivedContextType, bool isArrowFunctionContext, EvalContextType evalContextType)
    : EvalExecutable(exec, source, inStrictContext, derivedContextType, isArrowFunctionContext, evalContextType)
{
}

} // namespace JSC
