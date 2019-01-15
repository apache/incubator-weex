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

#include "CCallHelpers.h"
#include "RegisterSet.h"

namespace JSC { namespace DOMJIT {

class PatchpointParams;

typedef CCallHelpers::JumpList PatchpointGeneratorFunction(CCallHelpers&, PatchpointParams&);
typedef SharedTask<PatchpointGeneratorFunction> PatchpointGenerator;

// DOMJIT patchpoint is the way to inject an opaque code generator into DFG and FTL.
// While B3::Patchpoint is self-contained about its compilation information,
// DOMJIT::Patchpoint depends on which DFG Node invokes. For example, CheckDOM will
// link returned failureCases to BadType OSRExit, but this information is offered
// from CheckDOM DFG Node, not from this patchpoint. This patchpoint mainly focuses
// on injecting a snippet generator that can tell register usage and can be used
// in both DFG and FTL.
class Patchpoint : public ThreadSafeRefCounted<Patchpoint> {
public:
    static Ref<Patchpoint> create()
    {
        return adoptRef(*new Patchpoint());
    }

    template<typename Functor>
    void setGenerator(const Functor& functor)
    {
        m_generator = createSharedTask<PatchpointGeneratorFunction>(functor);
    }

    RefPtr<PatchpointGenerator> generator() const { return m_generator; }

    uint8_t numGPScratchRegisters { 0 };
    uint8_t numFPScratchRegisters { 0 };

protected:
    Patchpoint() = default;

private:
    RefPtr<PatchpointGenerator> m_generator;
};

} }

#endif
