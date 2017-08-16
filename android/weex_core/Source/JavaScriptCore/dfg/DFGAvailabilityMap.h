/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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
