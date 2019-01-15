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

#include "CodeBlock.h"
#include <wtf/FastBitVector.h>

namespace JSC {

class BytecodeLivenessAnalysis;

class BytecodeKills {
    WTF_MAKE_FAST_ALLOCATED;
public:
    BytecodeKills()
        : m_codeBlock(nullptr)
    {
    }
    
    // By convention, we say that non-local operands are never killed.
    bool operandIsKilled(unsigned bytecodeIndex, int operand) const
    {
        ASSERT_WITH_SECURITY_IMPLICATION(bytecodeIndex < m_codeBlock->instructions().size());
        VirtualRegister reg(operand);
        if (reg.isLocal())
            return m_killSets[bytecodeIndex].contains(operand);
        return false;
    }
    
    bool operandIsKilled(Instruction* instruction, int operand) const
    {
        return operandIsKilled(instruction - m_codeBlock->instructions().begin(), operand);
    }
    
    template<typename Functor>
    void forEachOperandKilledAt(unsigned bytecodeIndex, const Functor& functor) const
    {
        ASSERT_WITH_SECURITY_IMPLICATION(bytecodeIndex < m_codeBlock->instructions().size());
        m_killSets[bytecodeIndex].forEachLocal(
            [&] (unsigned local) {
                functor(virtualRegisterForLocal(local));
            });
    }
    
    template<typename Functor>
    void forEachOperandKilledAt(Instruction* pc, const Functor& functor) const
    {
        forEachOperandKilledAt(pc - m_codeBlock->instructions().begin(), functor);
    }
    
private:
    friend class BytecodeLivenessAnalysis;

    class KillSet {
    public:
        KillSet()
            : m_word(0)
        {
        }

        ~KillSet()
        {
            if (hasVector())
                delete vector();
        }
        
        void add(unsigned local)
        {
            if (isEmpty()) {
                setOneItem(local);
                return;
            }
            if (hasOneItem()) {
                ASSERT(oneItem() != local);
                Vector<unsigned>* vector = new Vector<unsigned>();
                vector->append(oneItem());
                vector->append(local);
                setVector(vector);
                return;
            }
            ASSERT(!vector()->contains(local));
            vector()->append(local);
        }
        
        template<typename Functor>
        void forEachLocal(const Functor& functor)
        {
            if (isEmpty())
                return;
            if (hasOneItem()) {
                functor(oneItem());
                return;
            }
            for (unsigned local : *vector())
                functor(local);
        }
        
        bool contains(unsigned expectedLocal)
        {
            if (isEmpty())
                return false;
            if (hasOneItem())
                return oneItem() == expectedLocal;
            for (unsigned local : *vector()) {
                if (local == expectedLocal)
                    return true;
            }
            return false;
        }
        
    private:
        bool isEmpty() const
        {
            return !m_word;
        }
        
        bool hasOneItem() const
        {
            return m_word & 1;
        }
        
        unsigned oneItem() const
        {
            return m_word >> 1;
        }
        
        void setOneItem(unsigned value)
        {
            m_word = (value << 1) | 1;
        }
        
        bool hasVector() const
        {
            return !isEmpty() && !hasOneItem();
        }
        
        Vector<unsigned>* vector()
        {
            return bitwise_cast<Vector<unsigned>*>(m_word);
        }
        
        void setVector(Vector<unsigned>* value)
        {
            m_word = bitwise_cast<uintptr_t>(value);
        }
        
        uintptr_t m_word;
    };
    
    CodeBlock* m_codeBlock;
    std::unique_ptr<KillSet[]> m_killSets;
};

} // namespace JSC
