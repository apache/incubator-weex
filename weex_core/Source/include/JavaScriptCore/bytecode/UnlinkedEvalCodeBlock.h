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

class UnlinkedEvalCodeBlock final : public UnlinkedGlobalCodeBlock {
public:
    typedef UnlinkedGlobalCodeBlock Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static UnlinkedEvalCodeBlock* create(VM* vm, const ExecutableInfo& info, DebuggerMode debuggerMode)
    {
        UnlinkedEvalCodeBlock* instance = new (NotNull, allocateCell<UnlinkedEvalCodeBlock>(vm->heap)) UnlinkedEvalCodeBlock(vm, vm->unlinkedEvalCodeBlockStructure.get(), info, debuggerMode);
        instance->finishCreation(*vm);
        return instance;
    }

    static void destroy(JSCell*);

    const Identifier& variable(unsigned index) { return m_variables[index]; }
    unsigned numVariables() { return m_variables.size(); }
    void adoptVariables(Vector<Identifier, 0, UnsafeVectorOverflow>& variables)
    {
        ASSERT(m_variables.isEmpty());
        m_variables.swap(variables);
    }

private:
    UnlinkedEvalCodeBlock(VM* vm, Structure* structure, const ExecutableInfo& info, DebuggerMode debuggerMode)
        : Base(vm, structure, EvalCode, info, debuggerMode)
    {
    }

    Vector<Identifier, 0, UnsafeVectorOverflow> m_variables;

public:
    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
    {
        return Structure::create(vm, globalObject, proto, TypeInfo(UnlinkedEvalCodeBlockType, StructureFlags), info());
    }

    DECLARE_INFO;
};

}
