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
