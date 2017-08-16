/*
 * Copyright (C) 2015-2016 Yusuke Suzuki <utatane.tea@gmail.com>.
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
