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

#include "DFGCommon.h"
#include "MacroAssembler.h"

namespace JSC { namespace DFG {

class JITCompiler;
class SpeculativeJIT;

class OSRExitJumpPlaceholder {
public:
    OSRExitJumpPlaceholder()
        : m_index(std::numeric_limits<unsigned>::max())
    {
    }

private:
    friend class SpeculativeJIT;
    
    OSRExitJumpPlaceholder(unsigned index)
        : m_index(index)
    {
    }

public:
    bool operator!() const { return m_index == std::numeric_limits<unsigned>::max(); }
    
    void fill(JITCompiler&, const MacroAssembler::JumpList&);
    void fill(SpeculativeJIT*, const MacroAssembler::JumpList&);
    
private:
    unsigned m_index;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
