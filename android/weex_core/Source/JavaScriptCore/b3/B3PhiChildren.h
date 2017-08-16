/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#if ENABLE(B3_JIT)

#include "B3Procedure.h"
#include "B3UpsilonValue.h"
#include <wtf/GraphNodeWorklist.h>
#include <wtf/IndexMap.h>

namespace JSC { namespace B3 {

class PhiChildren {
public:
    PhiChildren(Procedure&);
    ~PhiChildren();

    class ValueCollection {
    public:
        ValueCollection(Vector<UpsilonValue*>* values = nullptr)
            : m_values(values)
        {
        }

        unsigned size() const { return m_values->size(); }
        Value* at(unsigned index) const { return m_values->at(index)->child(0); }
        Value* operator[](unsigned index) const { return at(index); }

        bool contains(Value* value) const
        {
            for (unsigned i = size(); i--;) {
                if (at(i) == value)
                    return true;
            }
            return false;
        }

        class iterator {
        public:
            iterator(Vector<UpsilonValue*>* values = nullptr, unsigned index = 0)
                : m_values(values)
                , m_index(index)
            {
            }

            Value* operator*() const
            {
                return m_values->at(m_index)->child(0);
            }

            iterator& operator++()
            {
                m_index++;
                return *this;
            }

            bool operator==(const iterator& other) const
            {
                ASSERT(m_values == other.m_values);
                return m_index == other.m_index;
            }

            bool operator!=(const iterator& other) const
            {
                return !(*this == other);
            }

        private:
            Vector<UpsilonValue*>* m_values;
            unsigned m_index;
        };

        iterator begin() const { return iterator(m_values); }
        iterator end() const { return iterator(m_values, m_values->size()); }

    private:
        Vector<UpsilonValue*>* m_values;
    };
    
    class UpsilonCollection {
    public:
        UpsilonCollection()
        {
        }
        
        UpsilonCollection(PhiChildren* phiChildren, Value* value, Vector<UpsilonValue*>* values)
            : m_phiChildren(phiChildren)
            , m_value(value)
            , m_values(values)
        {
        }

        unsigned size() const { return m_values->size(); }
        Value* at(unsigned index) const { return m_values->at(index); }
        Value* operator[](unsigned index) const { return at(index); }

        bool contains(Value* value) const { return m_values->contains(value); }

        typedef Vector<UpsilonValue*>::const_iterator iterator;
        Vector<UpsilonValue*>::const_iterator begin() const { return m_values->begin(); }
        Vector<UpsilonValue*>::const_iterator end() const { return m_values->end(); }

        ValueCollection values() { return ValueCollection(m_values); }
        
        template<typename Functor>
        void forAllTransitiveIncomingValues(const Functor& functor)
        {
            if (m_value->opcode() != Phi) {
                functor(m_value);
                return;
            }
            
            GraphNodeWorklist<Value*> worklist;
            worklist.push(m_value);
            while (Value* phi = worklist.pop()) {
                for (Value* child : m_phiChildren->at(phi).values()) {
                    if (child->opcode() == Phi)
                        worklist.push(child);
                    else
                        functor(child);
                }
            }
        }

        bool transitivelyUses(Value* candidate)
        {
            bool result = false;
            forAllTransitiveIncomingValues(
                [&] (Value* child) {
                    result |= child == candidate;
                });
            return result;
        }

    private:
        PhiChildren* m_phiChildren { nullptr };
        Value* m_value { nullptr };
        Vector<UpsilonValue*>* m_values { nullptr };
    };

    UpsilonCollection at(Value* value) { return UpsilonCollection(this, value, &m_upsilons[value]); }
    UpsilonCollection operator[](Value* value) { return at(value); }

    const Vector<Value*, 8>& phis() const { return m_phis; }

private:
    IndexMap<Value, Vector<UpsilonValue*>> m_upsilons;
    Vector<Value*, 8> m_phis;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
