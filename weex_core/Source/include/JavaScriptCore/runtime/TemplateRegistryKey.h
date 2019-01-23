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
