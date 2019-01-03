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

#include "DFGAvailability.h"
#include "DFGPromotedHeapLocation.h"

namespace JSC { namespace DFG {

struct AvailabilityMap {
    void pruneHeap();
    void pruneByLiveness(Graph&, CodeOrigin);
    void clear();
    
    void dump(PrintStream& out) const;
    
    bool operator==(const AvailabilityMap& other) const;
    
    void merge(const AvailabilityMap& other);
    
    template<typename Functor>
    void forEachAvailability(const Functor& functor)
    {
        for (unsigned i = m_locals.size(); i--;)
            functor(m_locals[i]);
        for (auto pair : m_heap)
            functor(pair.value);
    }
    
    template<typename HasFunctor, typename AddFunctor>
    void closeOverNodes(const HasFunctor& has, const AddFunctor& add)
    {
        bool changed;
        do {
            changed = false;
            for (auto pair : m_heap) {
                if (pair.value.hasNode() && has(pair.key.base()))
                    changed |= add(pair.value.node());
            }
        } while (changed);
    }
    
    template<typename HasFunctor, typename AddFunctor>
    void closeStartingWithLocal(VirtualRegister reg, const HasFunctor& has, const AddFunctor& add)
    {
        Availability availability = m_locals.operand(reg);
        if (!availability.hasNode())
            return;
        
        if (!add(availability.node()))
            return;
        
        closeOverNodes(has, add);
    }
    
    Operands<Availability> m_locals;
    HashMap<PromotedHeapLocation, Availability> m_heap;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
