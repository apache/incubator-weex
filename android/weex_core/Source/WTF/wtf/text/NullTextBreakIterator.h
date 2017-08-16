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

namespace WTF {

class NullTextBreakIterator {
public:
    NullTextBreakIterator() = default;
    NullTextBreakIterator(const NullTextBreakIterator&) = delete;
    NullTextBreakIterator(NullTextBreakIterator&&) = default;
    NullTextBreakIterator& operator=(const NullTextBreakIterator&) = delete;
    NullTextBreakIterator& operator=(NullTextBreakIterator&&) = default;

    std::optional<unsigned> preceding(unsigned) const
    {
        ASSERT_NOT_REACHED();
        return { };
    }

    std::optional<unsigned> following(unsigned) const
    {
        ASSERT_NOT_REACHED();
        return { };
    }

    bool isBoundary(unsigned) const
    {
        ASSERT_NOT_REACHED();
        return false;
    }

    void setText(StringView)
    {
        ASSERT_NOT_REACHED();
    }
};

}

