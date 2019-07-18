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

#include "Structure.h"
#include <wtf/HashMap.h>

namespace JSC { namespace DFG {

// This object is a key for finding a property's type. It's a tuple of Structure* and UniquedStringImpl*.

class PropertyTypeKey {
public:
    PropertyTypeKey()
        : m_structure(nullptr)
        , m_uid(nullptr)
    {
    }

    PropertyTypeKey(Structure* structure, UniquedStringImpl* uid)
        : m_structure(structure)
        , m_uid(uid)
    {
    }

    PropertyTypeKey(WTF::HashTableDeletedValueType)
        : m_structure(nullptr)
        , m_uid(deletedUID())
    {
    }

    explicit operator bool() const { return m_structure && m_uid; }

    Structure* structure() const { return m_structure; }
    UniquedStringImpl* uid() const { return m_uid; }

    bool operator==(const PropertyTypeKey& other) const
    {
        return m_structure == other.m_structure
            && m_uid == other.m_uid;
    }

    bool operator!=(const PropertyTypeKey& other) const
    {
        return !(*this == other);
    }

    unsigned hash() const
    {
        return WTF::PtrHash<Structure*>::hash(m_structure) + WTF::PtrHash<UniquedStringImpl*>::hash(m_uid);
    }

    bool isHashTableDeletedValue() const
    {
        return !m_structure && m_uid == deletedUID();
    }

    void dumpInContext(PrintStream& out, DumpContext* context) const
    {
        out.print(pointerDumpInContext(m_structure, context), "+", m_uid);
    }

    void dump(PrintStream& out) const
    {
        dumpInContext(out, nullptr);
    }

private:
    static UniquedStringImpl* deletedUID()
    {
        return bitwise_cast<UniquedStringImpl*>(static_cast<intptr_t>(1));
    }
    
    Structure* m_structure;
    UniquedStringImpl* m_uid;
};

struct PropertyTypeKeyHash {
    static unsigned hash(const PropertyTypeKey& key) { return key.hash(); }
    static bool equal(const PropertyTypeKey& a, const PropertyTypeKey& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::DFG

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::DFG::PropertyTypeKey> {
    typedef JSC::DFG::PropertyTypeKeyHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::DFG::PropertyTypeKey> : SimpleClassHashTraits<JSC::DFG::PropertyTypeKey> {
    static const bool emptyValueIsZero = false;
};

} // namespace WTF

#endif // ENABLE(DFG_JIT)
