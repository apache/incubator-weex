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

#if ENABLE(JIT)

#include "DOMJITPatchpointParams.h"

namespace JSC {

struct AccessGenerationState;

class DOMJITAccessCasePatchpointParams : public DOMJIT::PatchpointParams {
public:
    DOMJITAccessCasePatchpointParams(Vector<DOMJIT::Value>&& regs, Vector<GPRReg>&& gpScratch, Vector<FPRReg>&& fpScratch)
        : DOMJIT::PatchpointParams(WTFMove(regs), WTFMove(gpScratch), WTFMove(fpScratch))
    {
    }

    class SlowPathCallGenerator {
    public:
        virtual ~SlowPathCallGenerator() { }
        virtual CCallHelpers::JumpList generate(AccessGenerationState&, const RegisterSet& usedRegistersByPatchpoint, CCallHelpers&) = 0;
    };

    CCallHelpers::JumpList emitSlowPathCalls(AccessGenerationState&, const RegisterSet& usedRegistersByPatchpoint, CCallHelpers&);

private:
#define JSC_DEFINE_CALL_OPERATIONS(OperationType, ResultType, ...) void addSlowPathCallImpl(CCallHelpers::JumpList, CCallHelpers&, OperationType, ResultType, std::tuple<__VA_ARGS__> args) override;
    DOMJIT_SLOW_PATH_CALLS(JSC_DEFINE_CALL_OPERATIONS)
#undef JSC_DEFINE_CALL_OPERATIONS
    Vector<std::unique_ptr<SlowPathCallGenerator>> m_generators;
};

}

#endif
