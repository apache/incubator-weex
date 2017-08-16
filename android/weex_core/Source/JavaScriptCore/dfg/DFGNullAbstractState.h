/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#pragma once

#if ENABLE(DFG_JIT)

namespace JSC { namespace DFG {

class Edge;
struct AbstractValue;
struct Node;

// Use this as a stub for things that can optionally take some kind of abstract state but you wish
// to not pass any abstract state. This works if the templatized code also does a check (using the
// operator bool) to see if the state is valid.
class NullAbstractState {
    WTF_MAKE_FAST_ALLOCATED;
public:
    NullAbstractState() { }
    
    explicit operator bool() const { return false; }
    
    AbstractValue& forNode(Node*)
    {
        RELEASE_ASSERT_NOT_REACHED();
        return *bitwise_cast<AbstractValue*>(static_cast<intptr_t>(0x1234));
    }
    
    AbstractValue& forNode(Edge)
    {
        return forNode(nullptr);
    }
    
    // It's valid to add more stub methods here as needed.
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
