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

#include "BatchedTransitionOptimizer.h"
#include "CodeBlock.h"
#include "CodeCache.h"
#include "Debugger.h"
#include "JIT.h"
#include "JSCInlines.h"
#include "LLIntEntrypoint.h"
#include "ModuleProgramCodeBlock.h"
#include "Parser.h"
#include "TypeProfiler.h"
#include "VMInlines.h"
#include <wtf/CommaPrinter.h>

namespace JSC {

const ClassInfo ModuleProgramExecutable::s_info = { "ModuleProgramExecutable", &ScriptExecutable::s_info, 0, CREATE_METHOD_TABLE(ModuleProgramExecutable) };

ModuleProgramExecutable::ModuleProgramExecutable(ExecState* exec, const SourceCode& source)
    : ScriptExecutable(exec->vm().moduleProgramExecutableStructure.get(), exec->vm(), source, false, DerivedContextType::None, false, EvalContextType::None, NoIntrinsic)
{
    ASSERT(source.provider()->sourceType() == SourceProviderSourceType::Module);
    m_typeProfilingStartOffset = 0;
    m_typeProfilingEndOffset = source.length() - 1;
    if (exec->vm().typeProfiler() || exec->vm().controlFlowProfiler())
        exec->vm().functionHasExecutedCache()->insertUnexecutedRange(sourceID(), m_typeProfilingStartOffset, m_typeProfilingEndOffset);
}

ModuleProgramExecutable* ModuleProgramExecutable::create(ExecState* exec, const SourceCode& source)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    ModuleProgramExecutable* executable = new (NotNull, allocateCell<ModuleProgramExecutable>(*exec->heap())) ModuleProgramExecutable(exec, source);
    executable->finishCreation(exec->vm());

    ParserError error;
    DebuggerMode debuggerMode = globalObject->hasInteractiveDebugger() ? DebuggerOn : DebuggerOff;
    UnlinkedModuleProgramCodeBlock* unlinkedModuleProgramCode = vm.codeCache()->getUnlinkedModuleProgramCodeBlock(
        vm, executable, executable->source(), debuggerMode, error);

    if (globalObject->hasDebugger())
        globalObject->debugger()->sourceParsed(exec, executable->source().provider(), error.line(), error.message());

    if (error.isValid()) {
        throwVMError(exec, scope, error.toErrorObject(globalObject, executable->source()));
        return nullptr;
    }

    executable->m_unlinkedModuleProgramCodeBlock.set(exec->vm(), executable, unlinkedModuleProgramCode);

    executable->m_moduleEnvironmentSymbolTable.set(exec->vm(), executable, jsCast<SymbolTable*>(unlinkedModuleProgramCode->constantRegister(unlinkedModuleProgramCode->moduleEnvironmentSymbolTableConstantRegisterOffset()).get())->cloneScopePart(exec->vm()));

    return executable;
}

void ModuleProgramExecutable::destroy(JSCell* cell)
{
    static_cast<ModuleProgramExecutable*>(cell)->ModuleProgramExecutable::~ModuleProgramExecutable();
}

void ModuleProgramExecutable::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    ModuleProgramExecutable* thisObject = jsCast<ModuleProgramExecutable*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    ScriptExecutable::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_unlinkedModuleProgramCodeBlock);
    visitor.append(thisObject->m_moduleEnvironmentSymbolTable);
    if (ModuleProgramCodeBlock* moduleProgramCodeBlock = thisObject->m_moduleProgramCodeBlock.get())
        moduleProgramCodeBlock->visitWeakly(visitor);
}

} // namespace JSC
