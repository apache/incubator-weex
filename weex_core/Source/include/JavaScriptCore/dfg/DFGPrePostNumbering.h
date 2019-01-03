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
