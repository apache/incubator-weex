/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
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

#include <wtf/PrintStream.h>
#include <wtf/StdLibExtras.h>

namespace JSC { namespace DFG {

// Entries in the NodeType enum (below) are composed of an id, a result type (possibly none)
// and some additional informative flags (must generate, is constant, etc).
#define NodeResultMask                   0x0007
#define NodeResultJS                     0x0001
#define NodeResultNumber                 0x0002
#define NodeResultDouble                 0x0003
#define NodeResultInt32                  0x0004
#define NodeResultInt52                  0x0005
#define NodeResultBoolean                0x0006
#define NodeResultStorage                0x0007
                                
#define NodeMustGenerate                 0x0008 // set on nodes that have side effects, and may not trivially be removed by DCE.
#define NodeHasVarArgs                   0x0010
    
#define NodeBehaviorMask                 0x07e0
#define NodeMayHaveDoubleResult          0x0020
#define NodeMayOverflowInt52             0x0040
#define NodeMayOverflowInt32InBaseline   0x0080
#define NodeMayOverflowInt32InDFG        0x0100
#define NodeMayNegZeroInBaseline         0x0200
#define NodeMayNegZeroInDFG              0x0400
#define NodeMayHaveNonNumberResult       0x0800
#define NodeMayHaveNonIntResult          (NodeMayHaveDoubleResult | NodeMayHaveNonNumberResult)
                                
#define NodeBytecodeBackPropMask        0x1f000
#define NodeBytecodeUseBottom           0x00000
#define NodeBytecodeUsesAsNumber        0x01000 // The result of this computation may be used in a context that observes fractional, or bigger-than-int32, results.
#define NodeBytecodeNeedsNegZero        0x02000 // The result of this computation may be used in a context that observes -0.
#define NodeBytecodeUsesAsOther         0x04000 // The result of this computation may be used in a context that distinguishes between NaN and other things (like undefined).
#define NodeBytecodeUsesAsValue         (NodeBytecodeUsesAsNumber | NodeBytecodeNeedsNegZero | NodeBytecodeUsesAsOther)
#define NodeBytecodeUsesAsInt           0x08000 // The result of this computation is known to be used in a context that prefers, but does not require, integer values.
#define NodeBytecodeUsesAsArrayIndex    0x10000 // The result of this computation is known to be used in a context that strongly prefers integer values, to the point that we should avoid using doubles if at all possible.

#define NodeArithFlagsMask               (NodeBehaviorMask | NodeBytecodeBackPropMask)

#define NodeIsFlushed                   0x20000 // Computed by CPSRethreadingPhase, will tell you which local nodes are backwards-reachable from a Flush.

#define NodeMiscFlag1                   0x40000
#define NodeMiscFlag2                   0x80000

typedef uint32_t NodeFlags;

static inline bool bytecodeUsesAsNumber(NodeFlags flags)
{
    return !!(flags & NodeBytecodeUsesAsNumber);
}

static inline bool bytecodeCanTruncateInteger(NodeFlags flags)
{
    return !bytecodeUsesAsNumber(flags);
}

static inline bool bytecodeCanIgnoreNegativeZero(NodeFlags flags)
{
    return !(flags & NodeBytecodeNeedsNegZero);
}

enum RareCaseProfilingSource {
    BaselineRareCase, // Comes from slow case counting in the baseline JIT.
    DFGRareCase, // Comes from OSR exit profiles.
    AllRareCases
};

static inline bool nodeMayOverflowInt52(NodeFlags flags, RareCaseProfilingSource)
{
    return !!(flags & NodeMayOverflowInt52);
}

static inline bool nodeMayOverflowInt32(NodeFlags flags, RareCaseProfilingSource source)
{
    NodeFlags mask = 0;
    switch (source) {
    case BaselineRareCase:
        mask = NodeMayOverflowInt32InBaseline;
        break;
    case DFGRareCase:
        mask = NodeMayOverflowInt32InDFG;
        break;
    case AllRareCases:
        mask = NodeMayOverflowInt32InBaseline | NodeMayOverflowInt32InDFG;
        break;
    }
    return !!(flags & mask);
}

static inline bool nodeMayNegZero(NodeFlags flags, RareCaseProfilingSource source)
{
    NodeFlags mask = 0;
    switch (source) {
    case BaselineRareCase:
        mask = NodeMayNegZeroInBaseline;
        break;
    case DFGRareCase:
        mask = NodeMayNegZeroInDFG;
        break;
    case AllRareCases:
        mask = NodeMayNegZeroInBaseline | NodeMayNegZeroInDFG;
        break;
    }
    return !!(flags & mask);
}

static inline bool nodeCanSpeculateInt32(NodeFlags flags, RareCaseProfilingSource source)
{
    if (nodeMayOverflowInt32(flags, source))
        return !bytecodeUsesAsNumber(flags);
    
    if (nodeMayNegZero(flags, source))
        return bytecodeCanIgnoreNegativeZero(flags);
    
    return true;
}

static inline bool nodeCanSpeculateInt52(NodeFlags flags, RareCaseProfilingSource source)
{
    if (nodeMayOverflowInt52(flags, source))
        return false;

    if (nodeMayNegZero(flags, source))
        return bytecodeCanIgnoreNegativeZero(flags);
    
    return true;
}

// FIXME: Get rid of this.
// https://bugs.webkit.org/show_bug.cgi?id=131689
static inline NodeFlags canonicalResultRepresentation(NodeFlags flags)
{
    switch (flags) {
    case NodeResultDouble:
    case NodeResultInt52:
    case NodeResultStorage:
        return flags;
    default:
        return NodeResultJS;
    }
}

void dumpNodeFlags(PrintStream&, NodeFlags);
MAKE_PRINT_ADAPTOR(NodeFlagsDump, NodeFlags, dumpNodeFlags);

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
