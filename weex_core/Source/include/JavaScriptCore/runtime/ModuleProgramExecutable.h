/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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
