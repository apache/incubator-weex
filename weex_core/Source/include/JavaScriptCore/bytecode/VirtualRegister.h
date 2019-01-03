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

#include "BytecodeConventions.h"
#include "CallFrame.h"
#include <wtf/PrintStream.h>

namespace JSC {

inline bool operandIsLocal(int operand)
{
    return operand < 0;
}

inline bool operandIsArgument(int operand)
{
    return operand >= 0;
}


class VirtualRegister {
public:
    friend VirtualRegister virtualRegisterForLocal(int);
    friend VirtualRegister virtualRegisterForArgument(int, int);

    VirtualRegister()
        : m_virtualRegister(s_invalidVirtualRegister)
    { }

    explicit VirtualRegister(int virtualRegister)
        : m_virtualRegister(virtualRegister)
    { }

    bool isValid() const { return (m_virtualRegister != s_invalidVirtualRegister); }
    bool isLocal() const { return operandIsLocal(m_virtualRegister); }
    bool isArgument() const { return operandIsArgument(m_virtualRegister); }
    bool isHeader() const { return m_virtualRegister >= 0 && m_virtualRegister < CallFrameSlot::thisArgument; }
    bool isConstant() const { return m_virtualRegister >= s_firstConstantRegisterIndex; }
    int toLocal() const { ASSERT(isLocal()); return operandToLocal(m_virtualRegister); }
    int toArgument() const { ASSERT(isArgument()); return operandToArgument(m_virtualRegister); }
    int toConstantIndex() const { ASSERT(isConstant()); return m_virtualRegister - s_firstConstantRegisterIndex; }
    int offset() const { return m_virtualRegister; }
    int offsetInBytes() const { return m_virtualRegister * sizeof(Register); }

    bool operator==(VirtualRegister other) const { return m_virtualRegister == other.m_virtualRegister; }
    bool operator!=(VirtualRegister other) const { return m_virtualRegister != other.m_virtualRegister; }
    bool operator<(VirtualRegister other) const { return m_virtualRegister < other.m_virtualRegister; }
    bool operator>(VirtualRegister other) const { return m_virtualRegister > other.m_virtualRegister; }
    bool operator<=(VirtualRegister other) const { return m_virtualRegister <= other.m_virtualRegister; }
    bool operator>=(VirtualRegister other) const { return m_virtualRegister >= other.m_virtualRegister; }
    
    VirtualRegister operator+(int value) const
    {
        return VirtualRegister(offset() + value);
    }
    VirtualRegister operator-(int value) const
    {
        return VirtualRegister(offset() - value);
    }
    VirtualRegister operator+(VirtualRegister value) const
    {
        return VirtualRegister(offset() + value.offset());
    }
    VirtualRegister operator-(VirtualRegister value) const
    {
        return VirtualRegister(offset() - value.offset());
    }
    VirtualRegister& operator+=(int value)
    {
        return *this = *this + value;
    }
    VirtualRegister& operator-=(int value)
    {
        return *this = *this - value;
    }
    
    void dump(PrintStream& out) const;

private:
    static const int s_invalidVirtualRegister = 0x3fffffff;
    static const int s_firstConstantRegisterIndex = FirstConstantRegisterIndex;

    static int localToOperand(int local) { return -1 - local; }
    static int operandToLocal(int operand) { return -1 - operand; }
    static int operandToArgument(int operand) { return operand - CallFrame::thisArgumentOffset(); }
    static int argumentToOperand(int argument) { return argument + CallFrame::thisArgumentOffset(); }

    int m_virtualRegister;
};

COMPILE_ASSERT(sizeof(VirtualRegister) == sizeof(int), VirtualRegister_is_32bit);

inline VirtualRegister virtualRegisterForLocal(int local)
{
    return VirtualRegister(VirtualRegister::localToOperand(local));
}

inline VirtualRegister virtualRegisterForArgument(int argument, int offset = 0)
{
    return VirtualRegister(VirtualRegister::argumentToOperand(argument) + offset);
}

} // namespace JSC
