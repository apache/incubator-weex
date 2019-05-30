/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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

#include "Identifier.h"
#include "PrivateName.h"
#include <wtf/Optional.h>

namespace JSC {

class PropertyName {
public:
    PropertyName(UniquedStringImpl* propertyName)
        : m_impl(propertyName)
    {
    }

    PropertyName(const Identifier& propertyName)
        : PropertyName(propertyName.impl())
    {
    }

    PropertyName(const PrivateName& propertyName)
        : m_impl(&propertyName.uid())
    {
        ASSERT(m_impl);
        ASSERT(m_impl->isSymbol());
    }

    bool isNull() const { return !m_impl; }

    bool isSymbol()
    {
        return m_impl && m_impl->isSymbol();
    }

    UniquedStringImpl* uid() const
    {
        return m_impl;
    }

    AtomicStringImpl* publicName() const
    {
        return (!m_impl || m_impl->isSymbol()) ? nullptr : static_cast<AtomicStringImpl*>(m_impl);
    }

    void dump(PrintStream& out) const
    {
        if (m_impl)
            out.print(m_impl);
        else
            out.print("<null property name>");
    }

private:
    UniquedStringImpl* m_impl;
};

inline bool operator==(PropertyName a, const Identifier& b)
{
    return a.uid() == b.impl();
}

inline bool operator==(const Identifier& a, PropertyName b)
{
    return a.impl() == b.uid();
}

inline bool operator==(PropertyName a, PropertyName b)
{
    return a.uid() == b.uid();
}

inline bool operator==(PropertyName a, const char* b)
{
    return equal(a.uid(), b);
}

inline bool operator!=(PropertyName a, const Identifier& b)
{
    return a.uid() != b.impl();
}

inline bool operator!=(const Identifier& a, PropertyName b)
{
    return a.impl() != b.uid();
}

inline bool operator!=(PropertyName a, PropertyName b)
{
    return a.uid() != b.uid();
}

ALWAYS_INLINE std::optional<uint32_t> parseIndex(PropertyName propertyName)
{
    auto uid = propertyName.uid();
    if (!uid)
        return std::nullopt;
    if (uid->isSymbol())
        return std::nullopt;
    return parseIndex(*uid);
}

} // namespace JSC
