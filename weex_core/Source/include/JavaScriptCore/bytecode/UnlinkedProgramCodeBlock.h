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

#include "UnlinkedGlobalCodeBlock.h"

namespace JSC {

class UnlinkedProgramCodeBlock final : public UnlinkedGlobalCodeBlock {
public:
    typedef UnlinkedGlobalCodeBlock Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static UnlinkedProgramCodeBlock* create(VM* vm, const ExecutableInfo& info, DebuggerMode debuggerMode)
    {
        UnlinkedProgramCodeBlock* instance = new (NotNull, allocateCell<UnlinkedProgramCodeBlock>(vm->heap)) UnlinkedProgramCodeBlock(vm, vm->unlinkedProgramCodeBlockStructure.get(), info, debuggerMode);
        instance->finishCreation(*vm);
        return instance;
    }

    static void destroy(JSCell*);

    void setVariableDeclarations(const VariableEnvironment& environment) { m_varDeclarations = environment; }
    const VariableEnvironment& variableDeclarations() const { return m_varDeclarations; }

    void setLexicalDeclarations(const VariableEnvironment& environment) { m_lexicalDeclarations = environment; }
    const VariableEnvironment& lexicalDeclarations() const { return m_lexicalDeclarations; }

    static void visitChildren(JSCell*, SlotVisitor&);

private:
    UnlinkedProgramCodeBlock(VM* vm, Structure* structure, const ExecutableInfo& info, DebuggerMode debuggerMode)
        : Base(vm, structure, GlobalCode, info, debuggerMode)
    {
    }

    VariableEnvironment m_varDeclarations;
    VariableEnvironment m_lexicalDeclarations;

public:
    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
    {
        return Structure::create(vm, globalObject, proto, TypeInfo(UnlinkedProgramCodeBlockType, StructureFlags), info());
    }

    DECLARE_INFO;
};

}
