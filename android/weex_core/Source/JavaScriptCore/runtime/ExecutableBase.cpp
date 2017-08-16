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
#include "Debugger.h"
#include "EvalCodeBlock.h"
#include "FunctionCodeBlock.h"
#include "JIT.h"
#include "JSCInlines.h"
#include "LLIntEntrypoint.h"
#include "ModuleProgramCodeBlock.h"
#include "Parser.h"
#include "ProgramCodeBlock.h"
#include "TypeProfiler.h"
#include "VMInlines.h"
#include <wtf/CommaPrinter.h>

namespace JSC {

const ClassInfo ExecutableBase::s_info = { "Executable", 0, 0, CREATE_METHOD_TABLE(ExecutableBase) };

void ExecutableBase::destroy(JSCell* cell)
{
    static_cast<ExecutableBase*>(cell)->ExecutableBase::~ExecutableBase();
}

void ExecutableBase::clearCode()
{
#if ENABLE(JIT)
    m_jitCodeForCall = nullptr;
    m_jitCodeForConstruct = nullptr;
    m_jitCodeForCallWithArityCheck = MacroAssemblerCodePtr();
    m_jitCodeForConstructWithArityCheck = MacroAssemblerCodePtr();
#endif
    m_numParametersForCall = NUM_PARAMETERS_NOT_COMPILED;
    m_numParametersForConstruct = NUM_PARAMETERS_NOT_COMPILED;
    VM& vm = *this->vm();


    if (structure(vm)->classInfo() == FunctionExecutable::info()) {
        FunctionExecutable* executable = static_cast<FunctionExecutable*>(this);
        executable->m_codeBlockForCall.clear();
        executable->m_codeBlockForConstruct.clear();
        return;
    }

    if (structure(vm)->classInfo() == EvalExecutable::info()) {
        EvalExecutable* executable = static_cast<EvalExecutable*>(this);
        executable->m_evalCodeBlock.clear();
        executable->m_unlinkedEvalCodeBlock.clear();
        return;
    }
    
    if (structure(vm)->classInfo() == ProgramExecutable::info()) {
        ProgramExecutable* executable = static_cast<ProgramExecutable*>(this);
        executable->m_programCodeBlock.clear();
        executable->m_unlinkedProgramCodeBlock.clear();
        return;
    }

    if (structure(vm)->classInfo() == ModuleProgramExecutable::info()) {
        ModuleProgramExecutable* executable = static_cast<ModuleProgramExecutable*>(this);
        executable->m_moduleProgramCodeBlock.clear();
        executable->m_unlinkedModuleProgramCodeBlock.clear();
        executable->m_moduleEnvironmentSymbolTable.clear();
        return;
    }
    
    ASSERT(structure(vm)->classInfo() == NativeExecutable::info());
}

void ExecutableBase::dump(PrintStream& out) const
{
    VM& vm = *this->vm();
    ExecutableBase* realThis = const_cast<ExecutableBase*>(this);

    if (classInfo(vm) == NativeExecutable::info()) {
        NativeExecutable* native = jsCast<NativeExecutable*>(realThis);
        out.print("NativeExecutable:", RawPointer(bitwise_cast<void*>(native->function())), "/", RawPointer(bitwise_cast<void*>(native->constructor())));
        return;
    }
    
    if (classInfo(vm) == EvalExecutable::info()) {
        EvalExecutable* eval = jsCast<EvalExecutable*>(realThis);
        if (CodeBlock* codeBlock = eval->codeBlock())
            out.print(*codeBlock);
        else
            out.print("EvalExecutable w/o CodeBlock");
        return;
    }
    
    if (classInfo(vm) == ProgramExecutable::info()) {
        ProgramExecutable* eval = jsCast<ProgramExecutable*>(realThis);
        if (CodeBlock* codeBlock = eval->codeBlock())
            out.print(*codeBlock);
        else
            out.print("ProgramExecutable w/o CodeBlock");
        return;
    }

    if (classInfo(vm) == ModuleProgramExecutable::info()) {
        ModuleProgramExecutable* executable = jsCast<ModuleProgramExecutable*>(realThis);
        if (CodeBlock* codeBlock = executable->codeBlock())
            out.print(*codeBlock);
        else
            out.print("ModuleProgramExecutable w/o CodeBlock");
        return;
    }
    
    FunctionExecutable* function = jsCast<FunctionExecutable*>(realThis);
    if (!function->eitherCodeBlock())
        out.print("FunctionExecutable w/o CodeBlock");
    else {
        CommaPrinter comma("/");
        if (function->codeBlockForCall())
            out.print(comma, *function->codeBlockForCall());
        if (function->codeBlockForConstruct())
            out.print(comma, *function->codeBlockForConstruct());
    }
}

CodeBlockHash ExecutableBase::hashFor(CodeSpecializationKind kind) const
{
    if (this->classInfo(*this->vm()) == NativeExecutable::info())
        return jsCast<const NativeExecutable*>(this)->hashFor(kind);
    
    return jsCast<const ScriptExecutable*>(this)->hashFor(kind);
}

} // namespace JSC
