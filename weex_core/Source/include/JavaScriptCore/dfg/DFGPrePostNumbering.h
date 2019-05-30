/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

#include "DFGBasicBlock.h"
#include "DFGBlockMap.h"
#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>

namespace JSC { namespace DFG {

enum EdgeKind {
    ForwardEdge,
    CrossEdge,
    BackEdge
};

class PrePostNumbering {
    WTF_MAKE_NONCOPYABLE(PrePostNumbering);
    WTF_MAKE_FAST_ALLOCATED;
public:
    PrePostNumbering(Graph&);
    ~PrePostNumbering();

    unsigned preNumber(BasicBlock* block) const { return m_map[block].m_preNumber; }
    unsigned postNumber(BasicBlock* block) const { return m_map[block].m_postNumber; }
    
    // Is from a strict ancestor of to?
    bool isStrictAncestorOf(BasicBlock* from, BasicBlock* to) const
    {
        return preNumber(from) < preNumber(to)
            && postNumber(from) > postNumber(to);
    }
    
    bool isAncestorOf(BasicBlock* from, BasicBlock* to) const
    {
        return from == to || isStrictAncestorOf(from, to);
    }
    
    bool isStrictDescendantOf(BasicBlock* from, BasicBlock* to) const
    {
        return isStrictAncestorOf(to, from);
    }
    
    bool isDescendantOf(BasicBlock* from, BasicBlock* to) const
    {
        return isAncestorOf(to, from);
    }
    
    // This will give a bogus answer if there is actually no such edge. If you want to determine
    // if there is any such edge, you have to do it yourself.
    EdgeKind edgeKind(BasicBlock* from, BasicBlock* to) const
    {
        if (isStrictDescendantOf(to, from))
            return ForwardEdge;
        
        if (isAncestorOf(to, from))
            return BackEdge;
        
        return CrossEdge;
    }
    
private:
    struct Numbering {
        unsigned m_preNumber;
        unsigned m_postNumber;
    };

    BlockMap<Numbering> m_map;
};

} } // namespace JSC::DFG

namespace WTF {

void printInternal(PrintStream&, JSC::DFG::EdgeKind);

} // namespace WTF

#endif // ENABLE(DFG_JIT)
