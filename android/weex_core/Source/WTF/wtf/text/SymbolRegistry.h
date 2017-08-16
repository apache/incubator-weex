/*
 * Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
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
