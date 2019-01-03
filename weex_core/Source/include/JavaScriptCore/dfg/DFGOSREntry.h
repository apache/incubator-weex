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

#include "DFGAbstractValue.h"
#include "Operands.h"
#include <wtf/BitVector.h>

namespace JSC {

class ExecState;
class CodeBlock;

namespace DFG {

#if ENABLE(DFG_JIT)
struct OSREntryReshuffling {
    OSREntryReshuffling() { }
    
    OSREntryReshuffling(int fromOffset, int toOffset)
        : fromOffset(fromOffset)
        , toOffset(toOffset)
    {
    }
    
    int fromOffset;
    int toOffset;
};

struct OSREntryData {
    unsigned m_bytecodeIndex;
    unsigned m_machineCodeOffset;
    Operands<AbstractValue> m_expectedValues;
    // Use bitvectors here because they tend to only require one word.
    BitVector m_localsForcedDouble;
    BitVector m_localsForcedAnyInt;
    Vector<OSREntryReshuffling> m_reshufflings;
    BitVector m_machineStackUsed;
    
    void dumpInContext(PrintStream&, DumpContext*) const;
    void dump(PrintStream&) const;
};

inline unsigned getOSREntryDataBytecodeIndex(OSREntryData* osrEntryData)
{
    return osrEntryData->m_bytecodeIndex;
}

// Returns a pointer to a data buffer that the OSR entry thunk will recognize and
// parse. If this returns null, it means 
void* prepareOSREntry(ExecState*, CodeBlock*, unsigned bytecodeIndex);
#else
inline void* prepareOSREntry(ExecState*, CodeBlock*, unsigned) { return 0; }
#endif

} } // namespace JSC::DFG
