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
#ifndef WTF_SymbolRegistry_h
#define WTF_SymbolRegistry_h

#include <wtf/HashSet.h>
#include <wtf/text/StringHash.h>
#include <wtf/text/SymbolImpl.h>
#include <wtf/text/WTFString.h>

namespace WTF {

// Since StringImpl* used for Symbol uid doesn't have a hash value reflecting the string content,
// to compare with an external string in string contents, introduce SymbolRegistryKey.
// SymbolRegistryKey holds a hash value reflecting the string content additionally.
class SymbolRegistryKey {
public:
    SymbolRegistryKey() = default;
    explicit SymbolRegistryKey(StringImpl* uid);
    SymbolRegistryKey(WTF::HashTableDeletedValueType);

    unsigned hash() const { return m_hash; }
    StringImpl* impl() const { return m_impl; }

    bool isHashTableDeletedValue() const { return m_impl == hashTableDeletedValue(); }

private:
    static StringImpl* hashTableDeletedValue() { return reinterpret_cast<StringImpl*>(-1); }

    StringImpl* m_impl { nullptr };
    unsigned m_hash { 0 };
};

template<typename T> struct DefaultHash;
template<> struct DefaultHash<SymbolRegistryKey> {
    struct Hash : StringHash {
        static unsigned hash(const SymbolRegistryKey& key)
        {
            return key.hash();
        }
        static bool equal(const SymbolRegistryKey& a, const SymbolRegistryKey& b)
        {
            return StringHash::equal(a.impl(), b.impl());
        }
    };
};

template<> struct HashTraits<SymbolRegistryKey> : SimpleClassHashTraits<SymbolRegistryKey> {
    static const bool hasIsEmptyValueFunction = true;
    static bool isEmptyValue(const SymbolRegistryKey& key)
    {
        return key.impl() == nullptr;
    }
};

class SymbolRegistry {
    WTF_MAKE_NONCOPYABLE(SymbolRegistry);
public:
    SymbolRegistry() = default;
    WTF_EXPORT_PRIVATE ~SymbolRegistry();

    WTF_EXPORT_PRIVATE Ref<SymbolImpl> symbolForKey(const String&);
    WTF_EXPORT_PRIVATE String keyForSymbol(SymbolImpl&);

    void remove(SymbolImpl&);

private:
    HashSet<SymbolRegistryKey> m_table;
};

inline SymbolRegistryKey::SymbolRegistryKey(StringImpl* uid)
    : m_impl(uid)
{
    if (uid->isSymbol()) {
        if (uid->is8Bit())
            m_hash = StringHasher::computeHashAndMaskTop8Bits(uid->characters8(), uid->length());
        else
            m_hash = StringHasher::computeHashAndMaskTop8Bits(uid->characters16(), uid->length());
    } else
        m_hash = uid->hash();
}

inline SymbolRegistryKey::SymbolRegistryKey(WTF::HashTableDeletedValueType)
    : m_impl(hashTableDeletedValue())
{
}

}

#endif
