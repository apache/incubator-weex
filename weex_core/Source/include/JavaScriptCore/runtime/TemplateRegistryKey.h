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

#pragma once

#include <limits>
#include <wtf/Vector.h>
#include <wtf/text/StringHash.h>
#include <wtf/text/WTFString.h>

namespace JSC {

class TemplateRegistryKeyTable;

class TemplateRegistryKey : public RefCounted<TemplateRegistryKey> {
public:
    friend class TemplateRegistryKeyTable;
    typedef Vector<String, 4> StringVector;
    typedef Vector<std::optional<String>, 4> OptionalStringVector;

    enum DeletedValueTag { DeletedValue };
    TemplateRegistryKey(DeletedValueTag);
    enum EmptyValueTag { EmptyValue };
    TemplateRegistryKey(EmptyValueTag);

    bool isDeletedValue() const { return m_rawStrings.isEmpty() && m_hash == std::numeric_limits<unsigned>::max(); }

    bool isEmptyValue() const { return m_rawStrings.isEmpty() && !m_hash; }

    unsigned hash() const { return m_hash; }

    const StringVector& rawStrings() const { return m_rawStrings; }
    const OptionalStringVector& cookedStrings() const { return m_cookedStrings; }

    bool operator==(const TemplateRegistryKey& other) const { return m_hash == other.m_hash && m_rawStrings == other.m_rawStrings; }
    bool operator!=(const TemplateRegistryKey& other) const { return m_hash != other.m_hash || m_rawStrings != other.m_rawStrings; }

    struct Hasher {
        static unsigned hash(const TemplateRegistryKey& key) { return key.hash(); }
        static bool equal(const TemplateRegistryKey& a, const TemplateRegistryKey& b) { return a == b; }
        static const bool safeToCompareToEmptyOrDeleted = false;
    };

    static unsigned calculateHash(const StringVector& rawStrings);
    ~TemplateRegistryKey();

private:
    static Ref<TemplateRegistryKey> create(StringVector&& rawStrings, OptionalStringVector&& cookedStrings)
    {
        return adoptRef(*new TemplateRegistryKey(WTFMove(rawStrings), WTFMove(cookedStrings)));
    }

    TemplateRegistryKey(StringVector&& rawStrings, OptionalStringVector&& cookedStrings);

    TemplateRegistryKeyTable* m_table { nullptr };
    StringVector m_rawStrings;
    OptionalStringVector m_cookedStrings;
    unsigned m_hash { 0 };
};

inline TemplateRegistryKey::TemplateRegistryKey(StringVector&& rawStrings, OptionalStringVector&& cookedStrings)
    : m_rawStrings(WTFMove(rawStrings))
    , m_cookedStrings(WTFMove(cookedStrings))
    , m_hash(calculateHash(rawStrings))
{
}

inline TemplateRegistryKey::TemplateRegistryKey(DeletedValueTag)
    : m_hash(std::numeric_limits<unsigned>::max())
{
}

inline TemplateRegistryKey::TemplateRegistryKey(EmptyValueTag)
    : m_hash(0)
{
}

inline unsigned TemplateRegistryKey::calculateHash(const StringVector& rawStrings)
{
    StringHasher hasher;
    for (const String& string : rawStrings) {
        if (string.is8Bit())
            hasher.addCharacters(string.characters8(), string.length());
        else
            hasher.addCharacters(string.characters16(), string.length());
    }
    return hasher.hash();
}

} // namespace JSC

namespace WTF {
template<typename T> struct DefaultHash;

template<> struct DefaultHash<JSC::TemplateRegistryKey> {
    typedef JSC::TemplateRegistryKey::Hasher Hash;
};

template<> struct HashTraits<JSC::TemplateRegistryKey> : CustomHashTraits<JSC::TemplateRegistryKey> {
};

} // namespace WTF
