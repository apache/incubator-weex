/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "JSCJSValue.h"
#include <wtf/HashMap.h>
#include <wtf/PrintStream.h>

namespace JSC { namespace Profiler {

class UID {
public:
    UID()
        : m_uid(0)
    {
    }
    
    static UID fromInt(uint64_t value)
    {
        UID result;
        result.m_uid = value;
        return result;
    }
    
    UID(WTF::HashTableDeletedValueType)
        : m_uid(std::numeric_limits<uint64_t>::max())
    {
    }
    
    static UID create();
    
    uint64_t toInt() const
    {
        return m_uid;
    }
    
    bool operator==(const UID& other) const
    {
        return m_uid == other.m_uid;
    }
    
    bool operator!=(const UID& other) const
    {
        return !(*this == other);
    }
    
    explicit operator bool() const
    {
        return *this != UID();
    }
    
    bool isHashTableDeletedValue() const
    {
        return *this != UID(WTF::HashTableDeletedValue);
    }
    
    unsigned hash() const
    {
        return IntHash<uint64_t>::hash(m_uid);
    }
    
    void dump(PrintStream&) const;
    JSValue toJS(ExecState*) const;
    
private:
    uint64_t m_uid;
};

struct UIDHash {
    static unsigned hash(const UID& key) { return key.hash(); }
    static bool equal(const UID& a, const UID& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::Profiler

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::Profiler::UID> {
    typedef JSC::Profiler::UIDHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::Profiler::UID> : SimpleClassHashTraits<JSC::Profiler::UID> { };

} // namespace WTF
