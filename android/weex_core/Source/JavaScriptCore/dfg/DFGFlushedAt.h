/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#include "DFGFlushFormat.h"
#include "VirtualRegister.h"

namespace JSC { namespace DFG {

class FlushedAt {
public:
    FlushedAt()
        : m_format(DeadFlush)
    {
    }
    
    explicit FlushedAt(FlushFormat format)
        : m_format(format)
    {
        ASSERT(format == DeadFlush || format == ConflictingFlush);
    }
    
    FlushedAt(FlushFormat format, VirtualRegister virtualRegister)
        : m_format(format)
        , m_virtualRegister(virtualRegister)
    {
        if (format == DeadFlush)
            ASSERT(!virtualRegister.isValid());
    }
    
    bool operator!() const { return m_format == DeadFlush; }
    
    FlushFormat format() const { return m_format; }
    VirtualRegister virtualRegister() const { return m_virtualRegister; }
    
    bool operator==(const FlushedAt& other) const
    {
        return m_format == other.m_format
            && m_virtualRegister == other.m_virtualRegister;
    }
    
    bool operator!=(const FlushedAt& other) const { return !(*this == other); }
    
    FlushedAt merge(const FlushedAt& other) const
    {
        if (!*this)
            return other;
        if (!other)
            return *this;
        if (*this == other)
            return *this;
        return FlushedAt(ConflictingFlush);
    }
    
    void dump(PrintStream&) const;
    void dumpInContext(PrintStream&, DumpContext*) const;
    
private:
    FlushFormat m_format;
    VirtualRegister m_virtualRegister;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
