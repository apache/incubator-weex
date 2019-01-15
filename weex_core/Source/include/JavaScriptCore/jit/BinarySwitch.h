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

#include "GPRInfo.h"
#include "MacroAssembler.h"
#include <wtf/WeakRandom.h>

namespace JSC {

// The BinarySwitch class makes it easy to emit a switch statement over either
// 32-bit integers or pointers, where the switch uses a tree of branches
// rather than a jump table. This makes it particularly useful if the case
// values are too far apart to make a jump table practical, or if there are
// sufficiently few cases that the total cost of log(numCases) branches is
// less than the cost of an indirected jump.
//
// In an effort to simplify the logic of emitting code for each case, this
// uses an iterator style, rather than a functor callback style. This makes
// sense because even the iterator implementation found herein is relatively
// simple, whereas the code it's used from is usually quite complex - one
// example being the trie-of-trees string switch implementation, where the
// code emitted for each case involves recursing to emit code for a sub-trie.
//
// Use this like so:
//
// BinarySwitch switch(valueReg, casesVector, BinarySwitch::Int32);
// while (switch.advance(jit)) {
//     int value = switch.caseValue();
//     unsigned index = switch.caseIndex(); // index into casesVector, above
//     ... // generate code for this case
//     ... = jit.jump(); // you have to jump out yourself; falling through causes undefined behavior
// }
// switch.fallThrough().link(&jit);

class BinarySwitch {
public:
    enum Type {
        Int32,
        IntPtr
    };
    
    BinarySwitch(GPRReg value, const Vector<int64_t>& cases, Type);
    ~BinarySwitch();
    
    unsigned caseIndex() const { return m_cases[m_caseIndex].index; }
    int64_t caseValue() const { return m_cases[m_caseIndex].value; }
    
    bool advance(MacroAssembler&);
    
    MacroAssembler::JumpList& fallThrough() { return m_fallThrough; }
    
private:
    void build(unsigned start, bool hardStart, unsigned end);
    
    GPRReg m_value;
    
    struct Case {
        Case() { }

        Case(int64_t value, unsigned index)
            : value(value)
            , index(index)
        {
        }
        
        bool operator<(const Case& other) const
        {
            return value < other.value;
        }

        void dump(PrintStream& out) const;
        
        int64_t value;
        unsigned index;
    };
    
    Vector<Case> m_cases;
    
    enum BranchKind {
        NotEqualToFallThrough,
        NotEqualToPush,
        LessThanToPush,
        Pop,
        ExecuteCase
    };
        
    struct BranchCode {
        BranchCode() { }
        
        BranchCode(BranchKind kind, unsigned index = UINT_MAX)
            : kind(kind)
            , index(index)
        {
        }

        void dump(PrintStream& out) const;
        
        BranchKind kind;
        unsigned index;
    };
    
    WeakRandom m_weakRandom;
    
    Vector<BranchCode> m_branches;

    unsigned m_index;
    unsigned m_caseIndex;
    Vector<MacroAssembler::Jump> m_jumpStack;
    
    MacroAssembler::JumpList m_fallThrough;
    
    Type m_type;
};

} // namespace JSC

#endif // ENABLE(JIT)
