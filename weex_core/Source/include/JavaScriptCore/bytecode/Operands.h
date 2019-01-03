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

#include "CallFrame.h"
#include "JSObject.h"
#include "VirtualRegister.h"

#include <wtf/PrintStream.h>
#include <wtf/Vector.h>

namespace JSC {

template<typename T> struct OperandValueTraits;

enum OperandKind { ArgumentOperand, LocalOperand };

enum OperandsLikeTag { OperandsLike };

template<typename T>
class Operands {
public:
    Operands() { }
    
    explicit Operands(size_t numArguments, size_t numLocals)
    {
        if (WTF::VectorTraits<T>::needsInitialization) {
            m_arguments.resize(numArguments);
            m_locals.resize(numLocals);
        } else {
            m_arguments.fill(T(), numArguments);
            m_locals.fill(T(), numLocals);
        }
    }

    explicit Operands(size_t numArguments, size_t numLocals, const T& initialValue)
    {
        m_arguments.fill(initialValue, numArguments);
        m_locals.fill(initialValue, numLocals);
    }
    
    template<typename U>
    explicit Operands(OperandsLikeTag, const Operands<U>& other)
    {
        m_arguments.fill(T(), other.numberOfArguments());
        m_locals.fill(T(), other.numberOfLocals());
    }
    
    size_t numberOfArguments() const { return m_arguments.size(); }
    size_t numberOfLocals() const { return m_locals.size(); }
    
    T& argument(size_t idx) { return m_arguments[idx]; }
    const T& argument(size_t idx) const { return m_arguments[idx]; }
    
    T& local(size_t idx) { return m_locals[idx]; }
    const T& local(size_t idx) const { return m_locals[idx]; }
    
    template<OperandKind operandKind>
    size_t sizeFor() const
    {
        if (operandKind == ArgumentOperand)
            return numberOfArguments();
        return numberOfLocals();
    }
    template<OperandKind operandKind>
    T& atFor(size_t idx)
    {
        if (operandKind == ArgumentOperand)
            return argument(idx);
        return local(idx);
    }
    template<OperandKind operandKind>
    const T& atFor(size_t idx) const
    {
        if (operandKind == ArgumentOperand)
            return argument(idx);
        return local(idx);
    }
    
    void ensureLocals(size_t size)
    {
        if (size <= m_locals.size())
            return;

        size_t oldSize = m_locals.size();
        m_locals.resize(size);
        if (!WTF::VectorTraits<T>::needsInitialization) {
            for (size_t i = oldSize; i < m_locals.size(); ++i)
                m_locals[i] = T();
        }
    }

    void ensureLocals(size_t size, const T& ensuredValue)
    {
        if (size <= m_locals.size())
            return;

        size_t oldSize = m_locals.size();
        m_locals.resize(size);
        for (size_t i = oldSize; i < m_locals.size(); ++i)
            m_locals[i] = ensuredValue;
    }
    
    void setLocal(size_t idx, const T& value)
    {
        ensureLocals(idx + 1);
        
        m_locals[idx] = value;
    }
    
    T getLocal(size_t idx)
    {
        if (idx >= m_locals.size())
            return T();
        return m_locals[idx];
    }
    
    void setArgumentFirstTime(size_t idx, const T& value)
    {
        ASSERT(m_arguments[idx] == T());
        argument(idx) = value;
    }
    
    void setLocalFirstTime(size_t idx, const T& value)
    {
        ASSERT(idx >= m_locals.size() || m_locals[idx] == T());
        setLocal(idx, value);
    }
    
    T& operand(int operand)
    {
        if (operandIsArgument(operand)) {
            int argument = VirtualRegister(operand).toArgument();
            return m_arguments[argument];
        }

        return m_locals[VirtualRegister(operand).toLocal()];
    }

    T& operand(VirtualRegister virtualRegister)
    {
        return operand(virtualRegister.offset());
    }

    const T& operand(int operand) const { return const_cast<const T&>(const_cast<Operands*>(this)->operand(operand)); }
    const T& operand(VirtualRegister operand) const { return const_cast<const T&>(const_cast<Operands*>(this)->operand(operand)); }
    
    bool hasOperand(int operand) const
    {
        if (operandIsArgument(operand))
            return true;
        return static_cast<size_t>(VirtualRegister(operand).toLocal()) < numberOfLocals();
    }
    bool hasOperand(VirtualRegister reg) const
    {
        return hasOperand(reg.offset());
    }
    
    void setOperand(int operand, const T& value)
    {
        if (operandIsArgument(operand)) {
            int argument = VirtualRegister(operand).toArgument();
            m_arguments[argument] = value;
            return;
        }
        
        setLocal(VirtualRegister(operand).toLocal(), value);
    }
    
    void setOperand(VirtualRegister virtualRegister, const T& value)
    {
        setOperand(virtualRegister.offset(), value);
    }

    size_t size() const { return numberOfArguments() + numberOfLocals(); }
    const T& at(size_t index) const
    {
        if (index < numberOfArguments())
            return m_arguments[index];
        return m_locals[index - numberOfArguments()];
    }
    T& at(size_t index)
    {
        if (index < numberOfArguments())
            return m_arguments[index];
        return m_locals[index - numberOfArguments()];
    }
    const T& operator[](size_t index) const { return at(index); }
    T& operator[](size_t index) { return at(index); }

    bool isArgument(size_t index) const { return index < numberOfArguments(); }
    bool isVariable(size_t index) const { return !isArgument(index); }
    int argumentForIndex(size_t index) const
    {
        return index;
    }
    int variableForIndex(size_t index) const
    {
        return index - m_arguments.size();
    }
    int operandForIndex(size_t index) const
    {
        if (index < numberOfArguments())
            return virtualRegisterForArgument(index).offset();
        return virtualRegisterForLocal(index - numberOfArguments()).offset();
    }
    VirtualRegister virtualRegisterForIndex(size_t index) const
    {
        return VirtualRegister(operandForIndex(index));
    }
    size_t indexForOperand(int operand) const
    {
        if (operandIsArgument(operand))
            return static_cast<size_t>(VirtualRegister(operand).toArgument());
        return static_cast<size_t>(VirtualRegister(operand).toLocal()) + numberOfArguments();
    }
    size_t indexForOperand(VirtualRegister reg) const
    {
        return indexForOperand(reg.offset());
    }
    
    void setOperandFirstTime(int operand, const T& value)
    {
        if (operandIsArgument(operand)) {
            setArgumentFirstTime(VirtualRegister(operand).toArgument(), value);
            return;
        }
        
        setLocalFirstTime(VirtualRegister(operand).toLocal(), value);
    }
    
    void fill(T value)
    {
        for (size_t i = 0; i < m_arguments.size(); ++i)
            m_arguments[i] = value;
        for (size_t i = 0; i < m_locals.size(); ++i)
            m_locals[i] = value;
    }
    
    void clear()
    {
        fill(T());
    }
    
    bool operator==(const Operands& other) const
    {
        ASSERT(numberOfArguments() == other.numberOfArguments());
        ASSERT(numberOfLocals() == other.numberOfLocals());
        
        return m_arguments == other.m_arguments && m_locals == other.m_locals;
    }
    
    void dumpInContext(PrintStream& out, DumpContext* context) const;
    void dump(PrintStream& out) const;
    
private:
    Vector<T, 8> m_arguments;
    Vector<T, 16> m_locals;
};

} // namespace JSC
