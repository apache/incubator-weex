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

class ProgramExecutable final : public ScriptExecutable {
    friend class LLIntOffsetsExtractor;
public:
    typedef ScriptExecutable Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static ProgramExecutable* create(ExecState* exec, const SourceCode& source)
    {
        ProgramExecutable* executable = new (NotNull, allocateCell<ProgramExecutable>(*exec->heap())) ProgramExecutable(exec, source);
        executable->finishCreation(exec->vm());
        return executable;
    }

    JSObject* initializeGlobalProperties(VM&, CallFrame*, JSScope*);

    static void destroy(JSCell*);

    ProgramCodeBlock* codeBlock()
    {
        return m_programCodeBlock.get();
    }

    JSObject* checkSyntax(ExecState*);

    Ref<JITCode> generatedJITCode()
    {
        return generatedJITCodeForCall();
    }
        
    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
    {
        return Structure::create(vm, globalObject, proto, TypeInfo(ProgramExecutableType, StructureFlags), info());
    }
        
    DECLARE_INFO;

    ExecutableInfo executableInfo() const { return ExecutableInfo(usesEval(), isStrictMode(), false, false, ConstructorKind::None, JSParserScriptMode::Classic, SuperBinding::NotNeeded, SourceParseMode::ProgramMode, derivedContextType(), isArrowFunctionContext(), false, EvalContextType::None); }

private:
    friend class ExecutableBase;
    friend class ScriptExecutable;

    ProgramExecutable(ExecState*, const SourceCode&);

    static void visitChildren(JSCell*, SlotVisitor&);

    WriteBarrier<UnlinkedProgramCodeBlock> m_unlinkedProgramCodeBlock;
    WriteBarrier<ProgramCodeBlock> m_programCodeBlock;
};

} // namespace JSC
