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

#if ENABLE(FTL_JIT)

#include "RegisterSet.h"

namespace JSC { namespace FTL {

// This is used for creating some sanity in slow-path calls out of the FTL's inline
// caches. The idea is that we don't want all of the register save/restore stuff to
// be generated at each IC site. Instead, the IC slow path call site will just save
// the registers needed for the arguments. It will arrange for there to be enough
// space on top of stack to save the remaining registers and the return PC. Then it
// will call a shared thunk that will save the remaining registers. That thunk needs
// to know the stack offset at which things get saved along with the call target.

// Note that the offset is *not including* the return PC that would be pushed on X86.

class SlowPathCallKey {
public:
    SlowPathCallKey()
        : m_callTarget(0)
        , m_offset(0)
    {
    }
    
    SlowPathCallKey(
        const RegisterSet& set, void* callTarget, const RegisterSet& argumentRegisters,
        ptrdiff_t offset)
        : m_usedRegisters(set)
        , m_callTarget(callTarget)
        , m_argumentRegisters(argumentRegisters)
        , m_offset(offset)
    {
    }
    
    const RegisterSet& usedRegisters() const { return m_usedRegisters; }
    void* callTarget() const { return m_callTarget; }
    const RegisterSet& argumentRegisters() const { return m_argumentRegisters; }
    ptrdiff_t offset() const { return m_offset; }
    
    SlowPathCallKey withCallTarget(void* callTarget)
    {
        return SlowPathCallKey(usedRegisters(), callTarget, argumentRegisters(), offset());
    }
    
    void dump(PrintStream&) const;
    
    enum EmptyValueTag { EmptyValue };
    enum DeletedValueTag { DeletedValue };
    
    SlowPathCallKey(EmptyValueTag)
        : m_usedRegisters(RegisterSet::EmptyValue)
        , m_callTarget(0)
        , m_offset(0)
    {
    }
    
    SlowPathCallKey(DeletedValueTag)
        : m_usedRegisters(RegisterSet::DeletedValue)
        , m_callTarget(0)
        , m_offset(0)
    {
    }
    
    bool isEmptyValue() const { return m_usedRegisters.isEmptyValue(); }
    bool isDeletedValue() const { return m_usedRegisters.isDeletedValue(); }
    
    bool operator==(const SlowPathCallKey& other) const
    {
        return m_usedRegisters == other.m_usedRegisters
            && m_callTarget == other.m_callTarget
            && m_offset == other.m_offset;
    }
    unsigned hash() const
    {
        return m_usedRegisters.hash() + PtrHash<void*>::hash(m_callTarget) + m_offset;
    }

private:
    RegisterSet m_usedRegisters;
    void* m_callTarget;
    RegisterSet m_argumentRegisters;
    ptrdiff_t m_offset;
};

struct SlowPathCallKeyHash {
    static unsigned hash(const SlowPathCallKey& key) { return key.hash(); }
    static bool equal(const SlowPathCallKey& a, const SlowPathCallKey& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = false;
};

} } // namespace JSC::FTL

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::FTL::SlowPathCallKey> {
    typedef JSC::FTL::SlowPathCallKeyHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::FTL::SlowPathCallKey> : public CustomHashTraits<JSC::FTL::SlowPathCallKey> { };

} // namespace WTF

#endif // ENABLE(FTL_JIT)
