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

#if ENABLE(DFG_JIT)

#include "AssemblyHelpers.h"
#include "CCallHelpers.h"
#include "DFGOSRExit.h"
#include "DFGOperations.h"

namespace JSC {

class ExecState;

namespace DFG {

class OSRExitCompiler {
public:
    OSRExitCompiler(CCallHelpers& jit)
        : m_jit(jit)
    {
    }
    
    void compileExit(const OSRExit&, const Operands<ValueRecovery>&, SpeculationRecovery*);

private:
    void emitRestoreArguments(const Operands<ValueRecovery>&);
    
    CCallHelpers& m_jit;
};

extern "C" {
void JIT_OPERATION compileOSRExit(ExecState*) WTF_INTERNAL;
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
