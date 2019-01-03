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

#include "DFGGraph.h"
#include <wtf/Vector.h>

namespace JSC { namespace DFG {

// === ScoreBoard ===
//
// This class is used in performing a virtual register allocation over the graph.
// VirtualRegisters are allocated to nodes, with a used count for each virtual
// register tracking the lifespan of the value; after the final use of a node
// the VirtualRegister associated is freed such that it can be reused for
// another node.
class ScoreBoard {
public:
    ScoreBoard(unsigned nextMachineLocal)
        : m_highWatermark(nextMachineLocal + 1)
    {
        m_used.fill(max(), nextMachineLocal);
        m_free.reserveCapacity(nextMachineLocal);
    }

    ~ScoreBoard()
    {
        assertClear();
    }
    
    void sortFree()
    {
        std::sort(m_free.begin(), m_free.end());
    }
    
    void assertClear()
    {
        if (ASSERT_DISABLED)
            return;
        
        // For every entry in the used list the use count of the virtual register should be zero, or max, due to it being a preserved local.
        for (size_t i = 0; i < m_used.size(); ++i)
            RELEASE_ASSERT(!m_used[i] || m_used[i] == max());
        // For every entry in the free list, the use count should be zero.
        for (size_t i = 0; i < m_free.size(); ++i)
            RELEASE_ASSERT(!m_used[m_free[i]]);
        // There must not be duplicates in the free list.
        for (size_t i = 0; i < m_free.size(); ++i) {
            for (size_t j = i + 1; j < m_free.size(); ++j)
                RELEASE_ASSERT(m_free[i] != m_free[j]);
        }
    }

    VirtualRegister allocate()
    {
        // Do we have any VirtualRegsiters in the free list, that were used by
        // prior nodes, but are now available?
        if (!m_free.isEmpty()) {
            uint32_t index = m_free.last();
            m_free.removeLast();
            // Use count must have hit zero for it to have been added to the free list!
            ASSERT(!m_used[index]);
            m_highWatermark = std::max(m_highWatermark, static_cast<unsigned>(index) + 1);
            return virtualRegisterForLocal(index);
        }

        // Allocate a new VirtualRegister, and add a corresponding entry to m_used.
        size_t next = m_used.size();
        m_used.append(0);
        m_highWatermark = std::max(m_highWatermark, static_cast<unsigned>(next) + 1);
        return virtualRegisterForLocal(next);
    }

    // Increment the usecount for the VirtualRegister associated with 'child',
    // if it reaches the node's refcount, free the VirtualRegister.
    void use(Node* child)
    {
        if (!child)
            return;

        // Find the virtual register number for this child, increment its use count.
        uint32_t index = child->virtualRegister().toLocal();
        ASSERT(m_used[index] != max());
        if (child->refCount() == ++m_used[index]) {
            // If the use count in the scoreboard reaches the use count for the node,
            // then this was its last use; the virtual register is now free.
            // Clear the use count & add to the free list.
            m_used[index] = 0;
            m_free.append(index);
        }
    }
    void use(Edge child)
    {
        use(child.node());
    }
    
    void useIfHasResult(Edge child)
    {
        if (!child)
            return;
        if (!child->hasResult())
            return;
        use(child);
    }

    unsigned highWatermark()
    {
        return m_highWatermark;
    }
    
#ifndef NDEBUG
    void dump()
    {
        dataLogF("    USED: [ ");
        for (unsigned i = 0; i < m_used.size(); ++i) {
            if (!m_free.contains(i)) {
                dataLogF("%d:", i);
                if (m_used[i] == max())
                    dataLogF("local ");
                else
                    dataLogF("%d ", m_used[i]);
            }
        }
        dataLogF("]\n");

        dataLogF("    FREE: [ ");
        for (unsigned i = 0; i < m_used.size(); ++i) {
            if (m_free.contains(i) && m_used[i] != max()) {
                ASSERT(!m_used[i]);
                dataLogF("%d ", i);
            }
        }
        dataLogF("]\n");
    }

#endif

private:
    static uint32_t max() { return std::numeric_limits<uint32_t>::max(); }
    
    // The size of the span of virtual registers that this code block will use.
    unsigned m_highWatermark;
    
    // For every virtual register that has been allocated (either currently alive, or in
    // the free list), we keep a count of the number of remaining uses until it is dead
    // (0, in the case of entries in the free list). Since there is an entry for every
    // allocated VirtualRegister, the length of this array conveniently provides the
    // next available VirtualRegister number.
    Vector<uint32_t, 64> m_used;
    // A free list of VirtualRegsiters no longer alive.
    Vector<uint32_t, 64> m_free;
};

} } // namespace JSC::DFG

#endif
