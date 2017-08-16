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

#include <wtf/Optional.h>
#include <wtf/TriState.h>

namespace JSC {

class DefinePropertyAttributes {
public:
    static_assert(FalseTriState == 0, "FalseTriState is 0.");
    static_assert(TrueTriState == 1, "TrueTriState is 1.");
    static_assert(MixedTriState == 2, "MixedTriState is 2.");

    static const unsigned ConfigurableShift = 0;
    static const unsigned EnumerableShift = 2;
    static const unsigned WritableShift = 4;
    static const unsigned ValueShift = 6;
    static const unsigned GetShift = 7;
    static const unsigned SetShift = 8;

    DefinePropertyAttributes()
        : m_attributes(
            (MixedTriState << ConfigurableShift)
            | (MixedTriState << EnumerableShift)
            | (MixedTriState << WritableShift)
            | (0 << ValueShift)
            | (0 << GetShift)
            | (0 << SetShift))
    {
    }

    explicit DefinePropertyAttributes(unsigned attributes)
        : m_attributes(attributes)
    {
    }

    unsigned rawRepresentation() const
    {
        return m_attributes;
    }

    bool hasValue() const
    {
        return m_attributes & (0b1 << ValueShift);
    }

    void setValue()
    {
        m_attributes = m_attributes | (0b1 << ValueShift);
    }

    bool hasGet() const
    {
        return m_attributes & (0b1 << GetShift);
    }

    void setGet()
    {
        m_attributes = m_attributes | (0b1 << GetShift);
    }

    bool hasSet() const
    {
        return m_attributes & (0b1 << SetShift);
    }

    void setSet()
    {
        m_attributes = m_attributes | (0b1 << SetShift);
    }

    bool hasWritable() const
    {
        return extractTriState(WritableShift) != MixedTriState;
    }

    std::optional<bool> writable() const
    {
        if (!hasWritable())
            return std::nullopt;
        return extractTriState(WritableShift) == TrueTriState;
    }

    bool hasConfigurable() const
    {
        return extractTriState(ConfigurableShift) != MixedTriState;
    }

    std::optional<bool> configurable() const
    {
        if (!hasConfigurable())
            return std::nullopt;
        return extractTriState(ConfigurableShift) == TrueTriState;
    }

    bool hasEnumerable() const
    {
        return extractTriState(EnumerableShift) != MixedTriState;
    }

    std::optional<bool> enumerable() const
    {
        if (!hasEnumerable())
            return std::nullopt;
        return extractTriState(EnumerableShift) == TrueTriState;
    }

    void setWritable(bool value)
    {
        fillWithTriState(value ? TrueTriState : FalseTriState, WritableShift);
    }

    void setConfigurable(bool value)
    {
        fillWithTriState(value ? TrueTriState : FalseTriState, ConfigurableShift);
    }

    void setEnumerable(bool value)
    {
        fillWithTriState(value ? TrueTriState : FalseTriState, EnumerableShift);
    }

private:
    void fillWithTriState(TriState state, unsigned shift)
    {
        unsigned mask = 0b11 << shift;
        m_attributes = (m_attributes & ~mask) | (state << shift);
    }

    TriState extractTriState(unsigned shift) const
    {
        return static_cast<TriState>((m_attributes >> shift) & 0b11);
    }

    unsigned m_attributes;
};


} // namespace JSC
