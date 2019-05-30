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

#if ENABLE(DFG_JIT)

#include <wtf/PrintStream.h>

namespace JSC { namespace DFG {

// Utility class for epoch-based analyses.

class Epoch {
public:
    Epoch()
        : m_epoch(s_none)
    {
    }
    
    static Epoch fromUnsigned(unsigned value)
    {
        Epoch result;
        result.m_epoch = value;
        return result;
    }
    
    unsigned toUnsigned() const
    {
        return m_epoch;
    }
    
    static Epoch first()
    {
        Epoch result;
        result.m_epoch = s_first;
        return result;
    }
    
    bool operator!() const
    {
        return m_epoch == s_none;
    }
    
    Epoch next() const
    {
        Epoch result;
        result.m_epoch = m_epoch + 1;
        return result;
    }
    
    void bump()
    {
        *this = next();
    }
    
    bool operator==(const Epoch& other) const
    {
        return m_epoch == other.m_epoch;
    }
    
    bool operator!=(const Epoch& other) const
    {
        return !(*this == other);
    }
    
    bool operator<(const Epoch& other) const
    {
        return m_epoch < other.m_epoch;
    }
    
    bool operator>(const Epoch& other) const
    {
        return other < *this;
    }
    
    bool operator<=(const Epoch& other) const
    {
        return !(*this > other);
    }
    
    bool operator>=(const Epoch& other) const
    {
        return !(*this < other);
    }
    
    void dump(PrintStream&) const;
    
private:
    static const unsigned s_none = 0;
    static const unsigned s_first = 1;
    
    unsigned m_epoch;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
