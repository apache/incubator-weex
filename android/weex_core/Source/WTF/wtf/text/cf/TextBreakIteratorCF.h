/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include <wtf/Optional.h>
#include <wtf/spi/cf/CFStringSPI.h>

namespace WTF {

class TextBreakIteratorCF {
public:
    enum class Mode {
        Caret,
        Delete
    };

    TextBreakIteratorCF(StringView string, Mode mode)
        : m_string(string.createCFStringWithoutCopying())
    {
        switch (mode) {
        case Mode::Caret:
            m_type = kCFStringComposedCharacterCluster;
            break;
        case Mode::Delete:
            m_type = kCFStringBackwardDeletionCluster;
            break;
        }
    }

    TextBreakIteratorCF() = delete;
    TextBreakIteratorCF(const TextBreakIteratorCF&) = delete;
    TextBreakIteratorCF(TextBreakIteratorCF&&) = default;
    TextBreakIteratorCF& operator=(const TextBreakIteratorCF&) = delete;
    TextBreakIteratorCF& operator=(TextBreakIteratorCF&&) = default;

    void setText(StringView string)
    {
        m_string = string.createCFStringWithoutCopying();
    }

    std::optional<unsigned> preceding(unsigned location) const
    {
        if (!location)
            return { };
        auto length = static_cast<unsigned long>(CFStringGetLength(m_string.get()));
        if (location > length)
            return length;
        auto range = CFStringGetRangeOfCharacterClusterAtIndex(m_string.get(), location - 1, m_type);
        return range.location;
    }

    std::optional<unsigned> following(unsigned location) const
    {
        if (location >= static_cast<unsigned long>(CFStringGetLength(m_string.get())))
            return { };
        auto range = CFStringGetRangeOfCharacterClusterAtIndex(m_string.get(), location, m_type);
        return range.location + range.length;
    }

    bool isBoundary(unsigned location) const
    {
        if (location == static_cast<unsigned long>(CFStringGetLength(m_string.get())))
            return true;
        auto range = CFStringGetRangeOfCharacterClusterAtIndex(m_string.get(), location, m_type);
        return static_cast<unsigned long>(range.location) == location;
    }

private:
    RetainPtr<CFStringRef> m_string;
    CFStringCharacterClusterType m_type;
};

}
