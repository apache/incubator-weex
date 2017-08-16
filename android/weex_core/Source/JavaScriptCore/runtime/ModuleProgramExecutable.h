/*
 * Copyright (C) 2009, 2010, 2013-2016 Apple Inc. All rights reserved.
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

#pragma once

#include "ScriptExecutable.h"

namespace JSC {

class ModuleProgramExecutable final : public ScriptExecutable {
    friend class LLIntOffsetsExtractor;
public:
    typedef ScriptExecutable Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static ModuleProgramExecutable* create(ExecState*, const SourceCode&);

    static void destroy(JSCell*);

    ModuleProgramCodeBlock* codeBlock()
    {
        return m_moduleProgramCodeBlock.get();
    }

    Ref<JITCode> generatedJITCode()
    {
        return generatedJITCodeForCall();
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
    {
        return Structure::create(vm, globalObject, proto, TypeInfo(ModuleProgramExecutableType, StructureFlags), info());
    }

    DECLARE_INFO;

    ExecutableInfo executableInfo() const { return ExecutableInfo(usesEval(), isStrictMode(), false, false, ConstructorKind::None, JSParserScriptMode::Module, SuperBinding::NotNeeded, SourceParseMode::ModuleEvaluateMode, derivedContextType(), isArrowFunctionContext(), false, EvalContextType::None); }

    UnlinkedModuleProgramCodeBlock* unlinkedModuleProgramCodeBlock() { return m_unlinkedModuleProgramCodeBlock.get(); }

    SymbolTable* moduleEnvironmentSymbolTable() { return m_moduleEnvironmentSymbolTable.get(); }

private:
    friend class ExecutableBase;
    friend class ScriptExecutable;

    ModuleProgramExecutable(ExecState*, const SourceCode&);

    static void visitChildren(JSCell*, SlotVisitor&);

    WriteBarrier<UnlinkedModuleProgramCodeBlock> m_unlinkedModuleProgramCodeBlock;
    WriteBarrier<SymbolTable> m_moduleEnvironmentSymbolTable;
    WriteBarrier<ModuleProgramCodeBlock> m_moduleProgramCodeBlock;
};

} // namespace JSC
