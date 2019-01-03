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

#include <wtf/text/UniquedStringImpl.h>

namespace WTF {

// SymbolImpl is used to represent the symbol string impl.
// It is uniqued string impl, but is not registered in Atomic String tables, so it's not atomic.
class SymbolImpl : public UniquedStringImpl {
private:
    static constexpr const unsigned s_flagIsNullSymbol = 1u;

public:
    unsigned hashForSymbol() const { return m_hashForSymbol; }
    SymbolRegistry* const& symbolRegistry() const { return m_symbolRegistry; }
    SymbolRegistry*& symbolRegistry() { return m_symbolRegistry; }
    bool isNullSymbol() const { return m_flags & s_flagIsNullSymbol; }

    WTF_EXPORT_STRING_API static Ref<SymbolImpl> createNullSymbol();
    WTF_EXPORT_STRING_API static Ref<SymbolImpl> create(StringImpl& rep);

    Ref<StringImpl> extractFoldedString()
    {
        ASSERT(substringBuffer());
        ASSERT(substringBuffer() == m_owner);
        ASSERT(!substringBuffer()->isSymbol());
        return createSubstringSharingImpl(*this, 0, length());
    }

private:
    WTF_EXPORT_PRIVATE static unsigned nextHashForSymbol();

    friend class StringImpl;

    SymbolImpl(const LChar* characters, unsigned length, Ref<StringImpl>&& base)
        : UniquedStringImpl(CreateSymbol, characters, length)
        , m_owner(&base.leakRef())
        , m_hashForSymbol(nextHashForSymbol())
    {
        ASSERT(StringImpl::tailOffset<StringImpl*>() == OBJECT_OFFSETOF(SymbolImpl, m_owner));
    }

    SymbolImpl(const UChar* characters, unsigned length, Ref<StringImpl>&& base)
        : UniquedStringImpl(CreateSymbol, characters, length)
        , m_owner(&base.leakRef())
        , m_hashForSymbol(nextHashForSymbol())
    {
        ASSERT(StringImpl::tailOffset<StringImpl*>() == OBJECT_OFFSETOF(SymbolImpl, m_owner));
    }

    SymbolImpl()
        : UniquedStringImpl(CreateSymbol)
        , m_owner(StringImpl::empty())
        , m_hashForSymbol(nextHashForSymbol())
        , m_flags(s_flagIsNullSymbol)
    {
        ASSERT(StringImpl::tailOffset<StringImpl*>() == OBJECT_OFFSETOF(SymbolImpl, m_owner));
    }

    // The pointer to the owner string should be immediately following after the StringImpl layout,
    // since we would like to align the layout of SymbolImpl to the one of BufferSubstring StringImpl.
    StringImpl* m_owner;
    SymbolRegistry* m_symbolRegistry { nullptr };
    unsigned m_hashForSymbol;
    unsigned m_flags { 0 };
};

inline unsigned StringImpl::symbolAwareHash() const
{
    if (isSymbol())
        return static_cast<const SymbolImpl*>(this)->hashForSymbol();
    return hash();
}

inline unsigned StringImpl::existingSymbolAwareHash() const
{
    if (isSymbol())
        return static_cast<const SymbolImpl*>(this)->hashForSymbol();
    return existingHash();
}

#if !ASSERT_DISABLED
// SymbolImpls created from StaticStringImpl will ASSERT
// in the generic ValueCheck<T>::checkConsistency
// as they are not allocated by fastMalloc.
// We don't currently have any way to detect that case
// so we ignore the consistency check for all SymbolImpls*.
template<> struct
ValueCheck<SymbolImpl*> {
    static void checkConsistency(const SymbolImpl*) { }
};

template<> struct
ValueCheck<const SymbolImpl*> {
    static void checkConsistency(const SymbolImpl*) { }
};
#endif

} // namespace WTF

using WTF::SymbolImpl;
