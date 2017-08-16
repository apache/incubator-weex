/*
 * Copyright (C) 2012, 2014 Apple Inc. All rights reserved.
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

enum BranchDirection {
    // This is not a branch and so there is no branch direction, or
    // the branch direction has yet to be set.
    InvalidBranchDirection,
        
    // The branch takes the true case.
    TakeTrue,
        
    // The branch takes the false case.
    TakeFalse,
        
    // For all we know, the branch could go either direction, so we
    // have to assume the worst.
    TakeBoth
};
    
static inline const char* branchDirectionToString(BranchDirection branchDirection)
{
    switch (branchDirection) {
    case InvalidBranchDirection:
        return "InvalidBranchDirection";
    case TakeTrue:
        return "TakeTrue";
    case TakeFalse:
        return "TakeFalse";
    case TakeBoth:
        return "TakeBoth";
    }

    RELEASE_ASSERT_NOT_REACHED();
    return "InvalidBranchDirection";
}
    
static inline bool isKnownDirection(BranchDirection branchDirection)
{
    switch (branchDirection) {
    case TakeTrue:
    case TakeFalse:
        return true;
    default:
        return false;
    }
}

static inline bool branchCondition(BranchDirection branchDirection)
{
    if (branchDirection == TakeTrue)
        return true;
    ASSERT(branchDirection == TakeFalse);
    return false;
}

} } // namespace JSC::DFG

namespace WTF {

inline void printInternal(PrintStream& out, JSC::DFG::BranchDirection direction)
{
    out.print(JSC::DFG::branchDirectionToString(direction));
}

} // namespace WTF

#endif // ENABLE(DFG_JIT)
