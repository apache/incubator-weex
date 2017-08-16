/*

Copyright (C) 2014-2017 Apple Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1.  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "config.h"
#include "StringView.h"

#include <mutex>
#include <unicode/ubrk.h>
#include <wtf/HashMap.h>
#include <wtf/Lock.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/Optional.h>
#include <wtf/text/TextBreakIterator.h>
#include <wtf/unicode/WTFUTF8.h>

namespace WTF {

using namespace Unicode;

bool StringView::containsIgnoringASCIICase(const StringView& matchString) const
{
    return findIgnoringASCIICase(matchString) != notFound;
}

bool StringView::containsIgnoringASCIICase(const StringView& matchString, unsigned startOffset) const
{
    return findIgnoringASCIICase(matchString, startOffset) != notFound;
}

size_t StringView::findIgnoringASCIICase(const StringView& matchString) const
{
    return ::WTF::findIgnoringASCIICase(*this, matchString, 0);
}

size_t StringView::findIgnoringASCIICase(const StringView& matchString, unsigned startOffset) const
{
    return ::WTF::findIgnoringASCIICase(*this, matchString, startOffset);
}

bool StringView::startsWith(const StringView& prefix) const
{
    return ::WTF::startsWith(*this, prefix);
}

bool StringView::startsWithIgnoringASCIICase(const StringView& prefix) const
{
    return ::WTF::startsWithIgnoringASCIICase(*this, prefix);
}

bool StringView::endsWith(const StringView& suffix) const
{
    return ::WTF::endsWith(*this, suffix);
}

bool StringView::endsWithIgnoringASCIICase(const StringView& suffix) const
{
    return ::WTF::endsWithIgnoringASCIICase(*this, suffix);
}

CString StringView::utf8(ConversionMode mode) const
{
    if (isNull())
        return CString("", 0);
    if (is8Bit())
        return StringImpl::utf8ForCharacters(characters8(), length());
    return StringImpl::utf8ForCharacters(characters16(), length(), mode);
}

size_t StringView::find(StringView matchString, unsigned start) const
{
    return findCommon(*this, matchString, start);
}

void StringView::SplitResult::Iterator::findNextSubstring()
{
    for (size_t separatorPosition; (separatorPosition = m_result.m_string.find(m_result.m_separator, m_position)) != notFound; ++m_position) {
        if (separatorPosition > m_position) {
            m_length = separatorPosition - m_position;
            return;
        }
    }
    m_length = m_result.m_string.length() - m_position;
}

auto StringView::SplitResult::Iterator::operator++() -> Iterator&
{
    ASSERT(m_position < m_result.m_string.length());
    m_position += m_length;
    if (m_position < m_result.m_string.length()) {
        ++m_position;
        findNextSubstring();
    }
    return *this;
}

class StringView::GraphemeClusters::Iterator::Impl {
public:
    Impl(const StringView& stringView, std::optional<NonSharedCharacterBreakIterator>&& iterator, unsigned index)
        : m_stringView(stringView)
        , m_iterator(WTFMove(iterator))
        , m_index(index)
        , m_indexEnd(computeIndexEnd())
    {
    }

    void operator++()
    {
        ASSERT(m_indexEnd > m_index);
        m_index = m_indexEnd;
        m_indexEnd = computeIndexEnd();
    }

    StringView operator*() const
    {
        if (m_stringView.is8Bit())
            return StringView(m_stringView.characters8() + m_index, m_indexEnd - m_index);
        return StringView(m_stringView.characters16() + m_index, m_indexEnd - m_index);
    }

    bool operator==(const Impl& other) const
    {
        ASSERT(&m_stringView == &other.m_stringView);
        auto result = m_index == other.m_index;
        ASSERT(!result || m_indexEnd == other.m_indexEnd);
        return result;
    }

    unsigned computeIndexEnd()
    {
        if (!m_iterator)
            return 0;
        if (m_index == m_stringView.length())
            return m_index;
        return ubrk_following(m_iterator.value(), m_index);
    }

private:
    const StringView& m_stringView;
    std::optional<NonSharedCharacterBreakIterator> m_iterator;
    unsigned m_index;
    unsigned m_indexEnd;
};

StringView::GraphemeClusters::Iterator::Iterator(const StringView& stringView, unsigned index)
    : m_impl(std::make_unique<Impl>(stringView, stringView.isNull() ? std::nullopt : std::optional<NonSharedCharacterBreakIterator>(NonSharedCharacterBreakIterator(stringView)), index))
{
}

StringView::GraphemeClusters::Iterator::~Iterator()
{
}

StringView::GraphemeClusters::Iterator::Iterator(Iterator&& other)
    : m_impl(WTFMove(other.m_impl))
{
}

auto StringView::GraphemeClusters::Iterator::operator++() -> Iterator&
{
    ++(*m_impl);
    return *this;
}

StringView StringView::GraphemeClusters::Iterator::operator*() const
{
    return **m_impl;
}

bool StringView::GraphemeClusters::Iterator::operator==(const Iterator& other) const
{
    return *m_impl == *(other.m_impl);
}

bool StringView::GraphemeClusters::Iterator::operator!=(const Iterator& other) const
{
    return !(*this == other);
}

#if CHECK_STRINGVIEW_LIFETIME

// Manage reference count manually so UnderlyingString does not need to be defined in the header.

struct StringView::UnderlyingString {
    std::atomic_uint refCount { 1u };
    bool isValid { true };
    const StringImpl& string;
    explicit UnderlyingString(const StringImpl&);
};

StringView::UnderlyingString::UnderlyingString(const StringImpl& string)
    : string(string)
{
}

static StaticLock underlyingStringsMutex;

static HashMap<const StringImpl*, StringView::UnderlyingString*>& underlyingStrings()
{
    static NeverDestroyed<HashMap<const StringImpl*, StringView::UnderlyingString*>> map;
    return map;
}

void StringView::invalidate(const StringImpl& stringToBeDestroyed)
{
    UnderlyingString* underlyingString;
    {
        std::lock_guard<StaticLock> lock(underlyingStringsMutex);
        underlyingString = underlyingStrings().take(&stringToBeDestroyed);
        if (!underlyingString)
            return;
    }
    ASSERT(underlyingString->isValid);
    underlyingString->isValid = false;
}

bool StringView::underlyingStringIsValid() const
{
    return !m_underlyingString || m_underlyingString->isValid;
}

void StringView::adoptUnderlyingString(UnderlyingString* underlyingString)
{
    if (m_underlyingString) {
        std::lock_guard<StaticLock> lock(underlyingStringsMutex);
        if (!--m_underlyingString->refCount) {
            if (m_underlyingString->isValid) {
                underlyingStrings().remove(&m_underlyingString->string);
            }
            delete m_underlyingString;
        }
    }
    m_underlyingString = underlyingString;
}

void StringView::setUnderlyingString(const StringImpl* string)
{
    UnderlyingString* underlyingString;
    if (!string)
        underlyingString = nullptr;
    else {
        std::lock_guard<StaticLock> lock(underlyingStringsMutex);
        auto result = underlyingStrings().add(string, nullptr);
        if (result.isNewEntry)
            result.iterator->value = new UnderlyingString(*string);
        else
            ++result.iterator->value->refCount;
        underlyingString = result.iterator->value;
    }
    adoptUnderlyingString(underlyingString);
}

void StringView::setUnderlyingString(const StringView& otherString)
{
    UnderlyingString* underlyingString = otherString.m_underlyingString;
    if (underlyingString)
        ++underlyingString->refCount;
    adoptUnderlyingString(underlyingString);
}

#endif // CHECK_STRINGVIEW_LIFETIME

} // namespace WTF
