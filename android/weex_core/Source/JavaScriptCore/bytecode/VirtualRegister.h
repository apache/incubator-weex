/*
 * Copyright (C) 2011, 2015-2016 Apple Inc. All rights reserved.
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
