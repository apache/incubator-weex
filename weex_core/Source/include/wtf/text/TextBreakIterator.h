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

#include "StringView.h"
#include <wtf/NeverDestroyed.h>
#include <wtf/Variant.h>
#include <wtf/text/icu/TextBreakIteratorICU.h>

#if PLATFORM(MAC) || PLATFORM(IOS)
#include <wtf/text/cf/TextBreakIteratorCF.h>
#else
#include <wtf/text/NullTextBreakIterator.h>
#endif

namespace WTF {

#if PLATFORM(MAC) || PLATFORM(IOS)
typedef TextBreakIteratorCF TextBreakIteratorPlatform;
#else
typedef NullTextBreakIterator TextBreakIteratorPlatform;
#endif

class TextBreakIteratorCache;

class TextBreakIterator {
public:
    enum class Mode {
        Line,
        Caret,
        Delete
    };

    TextBreakIterator() = delete;
    TextBreakIterator(const TextBreakIterator&) = delete;
    TextBreakIterator(TextBreakIterator&&) = default;
    TextBreakIterator& operator=(const TextBreakIterator&) = delete;
    TextBreakIterator& operator=(TextBreakIterator&&) = default;

    std::optional<unsigned> preceding(unsigned location) const
    {
        return switchOn(m_backing, [&](const auto& iterator) {
            return iterator.preceding(location);
        });
    }

    std::optional<unsigned> following(unsigned location) const
    {
        return switchOn(m_backing, [&](const auto& iterator) {
            return iterator.following(location);
        });
    }

    bool isBoundary(unsigned location) const
    {
        return switchOn(m_backing, [&](const auto& iterator) {
            return iterator.isBoundary(location);
        });
    }

private:
    friend class TextBreakIteratorCache;

    // Use CachedTextBreakIterator instead of constructing one of these directly.
    WTF_EXPORT TextBreakIterator(StringView, Mode, const AtomicString& locale);

    void setText(StringView string)
    {
        return switchOn(m_backing, [&](auto& iterator) {
            return iterator.setText(string);
        });
    }

    Mode mode() const
    {
        return m_mode;
    }

    const AtomicString& locale() const
    {
        return m_locale;
    }

    Variant<TextBreakIteratorICU, TextBreakIteratorPlatform> m_backing;
    Mode m_mode;
    AtomicString m_locale;
};

class CachedTextBreakIterator;

class TextBreakIteratorCache {
// Use CachedTextBreakIterator instead of dealing with the cache directly.
private:
    friend class NeverDestroyed<TextBreakIteratorCache>;
    friend class CachedTextBreakIterator;

    static TextBreakIteratorCache& singleton()
    {
        static NeverDestroyed<TextBreakIteratorCache> cache;
        return cache.get();
    }

    TextBreakIteratorCache(const TextBreakIteratorCache&) = delete;
    TextBreakIteratorCache(TextBreakIteratorCache&&) = delete;
    TextBreakIteratorCache& operator=(const TextBreakIteratorCache&) = delete;
    TextBreakIteratorCache& operator=(TextBreakIteratorCache&&) = delete;

    TextBreakIterator take(StringView string, TextBreakIterator::Mode mode, const AtomicString& locale)
    {
        auto iter = std::find_if(m_unused.begin(), m_unused.end(), [&](TextBreakIterator& candidate) {
            return candidate.mode() == mode && candidate.locale() == locale;
        });
        if (iter == m_unused.end())
            return TextBreakIterator(string, mode, locale);
        auto result = WTFMove(*iter);
        m_unused.remove(iter - m_unused.begin());
        result.setText(string);
        return result;
        
    }

    void put(TextBreakIterator&& iterator)
    {
        m_unused.append(WTFMove(iterator));
        if (m_unused.size() > capacity)
            m_unused.remove(0);
    }

    TextBreakIteratorCache()
    {
    }

    static constexpr int capacity = 2;
    // FIXME: Break this up into different Vectors per mode.
    Vector<TextBreakIterator, capacity> m_unused;
};

// RAII for TextBreakIterator and TextBreakIteratorCache.
class CachedTextBreakIterator {
public:
    CachedTextBreakIterator(StringView string, TextBreakIterator::Mode mode, const AtomicString& locale)
        : m_backing(TextBreakIteratorCache::singleton().take(string, mode, locale))
    {
    }

    ~CachedTextBreakIterator()
    {
        TextBreakIteratorCache::singleton().put(WTFMove(m_backing));
    }

    CachedTextBreakIterator() = delete;
    CachedTextBreakIterator(const CachedTextBreakIterator&) = delete;
    CachedTextBreakIterator(CachedTextBreakIterator&&) = default;
    CachedTextBreakIterator& operator=(const CachedTextBreakIterator&) = delete;
    CachedTextBreakIterator& operator=(CachedTextBreakIterator&&) = default;

    std::optional<unsigned> preceding(unsigned location) const
    {
        return m_backing.preceding(location);
    }

    std::optional<unsigned> following(unsigned location) const
    {
        return m_backing.following(location);
    }

    bool isBoundary(unsigned location) const
    {
        return m_backing.isBoundary(location);
    }

private:
    TextBreakIterator m_backing;
};

// Note: The returned iterator is good only until you get another iterator, with the exception of acquireLineBreakIterator.

enum class LineBreakIteratorMode { Default, Loose, Normal, Strict };

WTF_EXPORT_PRIVATE UBreakIterator* wordBreakIterator(StringView);
WTF_EXPORT_PRIVATE UBreakIterator* sentenceBreakIterator(StringView);

WTF_EXPORT_PRIVATE UBreakIterator* acquireLineBreakIterator(StringView, const AtomicString& locale, const UChar* priorContext, unsigned priorContextLength, LineBreakIteratorMode);
WTF_EXPORT_PRIVATE void releaseLineBreakIterator(UBreakIterator*);
UBreakIterator* openLineBreakIterator(const AtomicString& locale);
void closeLineBreakIterator(UBreakIterator*&);

WTF_EXPORT_PRIVATE bool isWordTextBreak(UBreakIterator*);

class LazyLineBreakIterator {
public:
    LazyLineBreakIterator()
    {
        resetPriorContext();
    }

    explicit LazyLineBreakIterator(StringView stringView, const AtomicString& locale = AtomicString(), LineBreakIteratorMode mode = LineBreakIteratorMode::Default)
        : m_stringView(stringView)
        , m_locale(locale)
        , m_mode(mode)
    {
        resetPriorContext();
    }

    ~LazyLineBreakIterator()
    {
        if (m_iterator)
            releaseLineBreakIterator(m_iterator);
    }

    StringView stringView() const { return m_stringView; }
    LineBreakIteratorMode mode() const { return m_mode; }

    UChar lastCharacter() const
    {
        static_assert(WTF_ARRAY_LENGTH(m_priorContext) == 2, "UBreakIterator unexpected prior context length");
        return m_priorContext[1];
    }

    UChar secondToLastCharacter() const
    {
        static_assert(WTF_ARRAY_LENGTH(m_priorContext) == 2, "UBreakIterator unexpected prior context length");
        return m_priorContext[0];
    }

    void setPriorContext(UChar last, UChar secondToLast)
    {
        static_assert(WTF_ARRAY_LENGTH(m_priorContext) == 2, "UBreakIterator unexpected prior context length");
        m_priorContext[0] = secondToLast;
        m_priorContext[1] = last;
    }

    void updatePriorContext(UChar last)
    {
        static_assert(WTF_ARRAY_LENGTH(m_priorContext) == 2, "UBreakIterator unexpected prior context length");
        m_priorContext[0] = m_priorContext[1];
        m_priorContext[1] = last;
    }

    void resetPriorContext()
    {
        static_assert(WTF_ARRAY_LENGTH(m_priorContext) == 2, "UBreakIterator unexpected prior context length");
        m_priorContext[0] = 0;
        m_priorContext[1] = 0;
    }

    unsigned priorContextLength() const
    {
        unsigned priorContextLength = 0;
        static_assert(WTF_ARRAY_LENGTH(m_priorContext) == 2, "UBreakIterator unexpected prior context length");
        if (m_priorContext[1]) {
            ++priorContextLength;
            if (m_priorContext[0])
                ++priorContextLength;
        }
        return priorContextLength;
    }

    // Obtain text break iterator, possibly previously cached, where this iterator is (or has been)
    // initialized to use the previously stored string as the primary breaking context and using
    // previously stored prior context if non-empty.
    UBreakIterator* get(unsigned priorContextLength)
    {
        ASSERT(priorContextLength <= priorContextCapacity);
        const UChar* priorContext = priorContextLength ? &m_priorContext[priorContextCapacity - priorContextLength] : 0;
        if (!m_iterator) {
            m_iterator = acquireLineBreakIterator(m_stringView, m_locale, priorContext, priorContextLength, m_mode);
            m_cachedPriorContext = priorContext;
            m_cachedPriorContextLength = priorContextLength;
        } else if (priorContext != m_cachedPriorContext || priorContextLength != m_cachedPriorContextLength) {
            resetStringAndReleaseIterator(m_stringView, m_locale, m_mode);
            return this->get(priorContextLength);
        }
        return m_iterator;
    }

    void resetStringAndReleaseIterator(StringView stringView, const AtomicString& locale, LineBreakIteratorMode mode)
    {
        if (m_iterator)
            releaseLineBreakIterator(m_iterator);
        m_stringView = stringView;
        m_locale = locale;
        m_iterator = nullptr;
        m_cachedPriorContext = nullptr;
        m_mode = mode;
        m_cachedPriorContextLength = 0;
    }

private:
    static constexpr unsigned priorContextCapacity = 2;
    StringView m_stringView;
    AtomicString m_locale;
    UBreakIterator* m_iterator { nullptr };
    const UChar* m_cachedPriorContext { nullptr };
    LineBreakIteratorMode m_mode { LineBreakIteratorMode::Default };
    unsigned m_cachedPriorContextLength { 0 };
    UChar m_priorContext[priorContextCapacity];
};

// Iterates over "extended grapheme clusters", as defined in UAX #29.
// Note that platform implementations may be less sophisticated - e.g. ICU prior to
// version 4.0 only supports "legacy grapheme clusters".
// Use this for general text processing, e.g. string truncation.

class NonSharedCharacterBreakIterator {
    WTF_MAKE_NONCOPYABLE(NonSharedCharacterBreakIterator);
public:
    WTF_EXPORT_PRIVATE NonSharedCharacterBreakIterator(StringView);
    WTF_EXPORT_PRIVATE ~NonSharedCharacterBreakIterator();

    NonSharedCharacterBreakIterator(NonSharedCharacterBreakIterator&&);

    operator UBreakIterator*() const { return m_iterator; }

private:
    UBreakIterator* m_iterator;
};

// Counts the number of grapheme clusters. A surrogate pair or a sequence
// of a non-combining character and following combining characters is
// counted as 1 grapheme cluster.
WTF_EXPORT_PRIVATE unsigned numGraphemeClusters(StringView);

// Returns the number of characters which will be less than or equal to
// the specified grapheme cluster length.
WTF_EXPORT_PRIVATE unsigned numCharactersInGraphemeClusters(StringView, unsigned);

}

using WTF::CachedTextBreakIterator;
using WTF::LazyLineBreakIterator;
using WTF::LineBreakIteratorMode;
using WTF::NonSharedCharacterBreakIterator;
using WTF::TextBreakIterator;
using WTF::TextBreakIteratorCache;
using WTF::isWordTextBreak;
