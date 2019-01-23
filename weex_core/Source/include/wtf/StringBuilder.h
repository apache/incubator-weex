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
#ifndef StringBuilder_h
#define StringBuilder_h

#include <wtf/text/AtomicString.h>
#include <wtf/text/IntegerToStringConversion.h>
#include <wtf/text/StringView.h>
#include <wtf/text/WTFString.h>

namespace WTF {

class StringBuilder {
    // Disallow copying since it's expensive and we don't want code to do it by accident.
    WTF_MAKE_NONCOPYABLE(StringBuilder);

public:
    StringBuilder()
        : m_length(0)
        , m_is8Bit(true)
        , m_bufferCharacters8(0)
    {
    }

    WTF_EXPORT_PRIVATE void append(const UChar*, unsigned);
    WTF_EXPORT_PRIVATE void append(const LChar*, unsigned);

    ALWAYS_INLINE void append(const char* characters, unsigned length) { append(reinterpret_cast<const LChar*>(characters), length); }

    void append(const AtomicString& atomicString)
    {
        append(atomicString.string());
    }

    void append(const String& string)
    {
        if (!string.length())
            return;

        // If we're appending to an empty string, and there is not a buffer (reserveCapacity has not been called)
        // then just retain the string.
        if (!m_length && !m_buffer) {
            m_string = string;
            m_length = string.length();
            m_is8Bit = m_string.is8Bit();
            return;
        }

        if (string.is8Bit())
            append(string.characters8(), string.length());
        else
            append(string.characters16(), string.length());
    }

    void append(const StringBuilder& other)
    {
        if (!other.m_length)
            return;

        // If we're appending to an empty string, and there is not a buffer (reserveCapacity has not been called)
        // then just retain the string.
        if (!m_length && !m_buffer && !other.m_string.isNull()) {
            m_string = other.m_string;
            m_length = other.m_length;
            return;
        }

        if (other.is8Bit())
            append(other.characters8(), other.m_length);
        else
            append(other.characters16(), other.m_length);
    }

    void append(StringView stringView)
    {
        if (stringView.is8Bit())
            append(stringView.characters8(), stringView.length());
        else
            append(stringView.characters16(), stringView.length());
    }

#if USE(CF)
    WTF_EXPORT_PRIVATE void append(CFStringRef);
#endif
#if USE(CF) && defined(__OBJC__)
    void append(NSString *string) { append((__bridge CFStringRef)string); }
#endif
    
    void append(const String& string, unsigned offset, unsigned length)
    {
        if (!string.length())
            return;

        if ((offset + length) > string.length())
            return;

        if (string.is8Bit())
            append(string.characters8() + offset, length);
        else
            append(string.characters16() + offset, length);
    }

    void append(const char* characters)
    {
        if (characters)
            append(characters, strlen(characters));
    }

    void append(UChar c)
    {
        if (m_buffer && m_length < m_buffer->length() && m_string.isNull()) {
            if (!m_is8Bit) {
                m_bufferCharacters16[m_length++] = c;
                return;
            }

            if (!(c & ~0xff)) {
                m_bufferCharacters8[m_length++] = static_cast<LChar>(c);
                return;
            }
        }
        append(&c, 1);
    }

    void append(LChar c)
    {
        if (m_buffer && m_length < m_buffer->length() && m_string.isNull()) {
            if (m_is8Bit)
                m_bufferCharacters8[m_length++] = c;
            else
                m_bufferCharacters16[m_length++] = c;
        } else
            append(&c, 1);
    }

    void append(char c)
    {
        append(static_cast<LChar>(c));
    }

    void append(UChar32 c)
    {
        if (U_IS_BMP(c)) {
            append(static_cast<UChar>(c));
            return;
        }
        append(U16_LEAD(c));
        append(U16_TRAIL(c));
    }

    WTF_EXPORT_PRIVATE void appendQuotedJSONString(const String&);

    template<unsigned charactersCount>
    ALWAYS_INLINE void appendLiteral(const char (&characters)[charactersCount]) { append(characters, charactersCount - 1); }

    WTF_EXPORT_PRIVATE void appendNumber(int);
    WTF_EXPORT_PRIVATE void appendNumber(unsigned int);
    WTF_EXPORT_PRIVATE void appendNumber(long);
    WTF_EXPORT_PRIVATE void appendNumber(unsigned long);
    WTF_EXPORT_PRIVATE void appendNumber(long long);
    WTF_EXPORT_PRIVATE void appendNumber(unsigned long long);
    WTF_EXPORT_PRIVATE void appendNumber(double, unsigned precision = 6, TrailingZerosTruncatingPolicy = TruncateTrailingZeros);
    WTF_EXPORT_PRIVATE void appendECMAScriptNumber(double);
    WTF_EXPORT_PRIVATE void appendFixedWidthNumber(double, unsigned decimalPlaces);

    String toString()
    {
        shrinkToFit();
        if (m_string.isNull())
            reifyString();
        return m_string;
    }

    const String& toStringPreserveCapacity() const
    {
        if (m_string.isNull())
            reifyString();
        return m_string;
    }

    AtomicString toAtomicString() const
    {
        if (!m_length)
            return emptyAtom;

        // If the buffer is sufficiently over-allocated, make a new AtomicString from a copy so its buffer is not so large.
        if (canShrink()) {
            if (is8Bit())
                return AtomicString(characters8(), length());
            return AtomicString(characters16(), length());            
        }

        if (!m_string.isNull())
            return AtomicString(m_string);

        ASSERT(m_buffer);
        return AtomicString(m_buffer.get(), 0, m_length);
    }

    unsigned length() const
    {
        return m_length;
    }

    bool isEmpty() const { return !m_length; }

    WTF_EXPORT_PRIVATE void reserveCapacity(unsigned newCapacity);

    unsigned capacity() const
    {
        return m_buffer ? m_buffer->length() : m_length;
    }

    WTF_EXPORT_PRIVATE void resize(unsigned newSize);

    WTF_EXPORT_PRIVATE bool canShrink() const;

    WTF_EXPORT_PRIVATE void shrinkToFit();

    UChar operator[](unsigned i) const
    {
        ASSERT_WITH_SECURITY_IMPLICATION(i < m_length);
        if (m_is8Bit)
            return characters8()[i];
        return characters16()[i];
    }

    const LChar* characters8() const
    {
        ASSERT(m_is8Bit);
        if (!m_length)
            return 0;
        if (!m_string.isNull())
            return m_string.characters8();
        ASSERT(m_buffer);
        return m_buffer->characters8();
    }

    const UChar* characters16() const
    {
        ASSERT(!m_is8Bit);
        if (!m_length)
            return 0;
        if (!m_string.isNull())
            return m_string.characters16();
        ASSERT(m_buffer);
        return m_buffer->characters16();
    }
    
    bool is8Bit() const { return m_is8Bit; }

    void clear()
    {
        m_length = 0;
        m_string = String();
        m_buffer = nullptr;
        m_bufferCharacters8 = 0;
        m_is8Bit = true;
    }

    void swap(StringBuilder& stringBuilder)
    {
        std::swap(m_length, stringBuilder.m_length);
        m_string.swap(stringBuilder.m_string);
        m_buffer.swap(stringBuilder.m_buffer);
        std::swap(m_is8Bit, stringBuilder.m_is8Bit);
        std::swap(m_bufferCharacters8, stringBuilder.m_bufferCharacters8);
        ASSERT(!m_buffer || m_buffer->length() >= m_length);
    }

private:
    void allocateBuffer(const LChar* currentCharacters, unsigned requiredLength);
    void allocateBuffer(const UChar* currentCharacters, unsigned requiredLength);
    void allocateBufferUpConvert(const LChar* currentCharacters, unsigned requiredLength);
    template <typename CharType>
    void reallocateBuffer(unsigned requiredLength);
    template <typename CharType>
    ALWAYS_INLINE CharType* appendUninitialized(unsigned length);
    template <typename CharType>
    CharType* appendUninitializedSlow(unsigned length);
    template <typename CharType>
    ALWAYS_INLINE CharType * getBufferCharacters();
    WTF_EXPORT_PRIVATE void reifyString() const;

    unsigned m_length;
    mutable String m_string;
    RefPtr<StringImpl> m_buffer;
    bool m_is8Bit;
    union {
        LChar* m_bufferCharacters8;
        UChar* m_bufferCharacters16;
    };
};

template <>
ALWAYS_INLINE LChar* StringBuilder::getBufferCharacters<LChar>()
{
    ASSERT(m_is8Bit);
    return m_bufferCharacters8;
}

template <>
ALWAYS_INLINE UChar* StringBuilder::getBufferCharacters<UChar>()
{
    ASSERT(!m_is8Bit);
    return m_bufferCharacters16;
}    

template <typename CharType>
bool equal(const StringBuilder& s, const CharType* buffer, unsigned length)
{
    if (s.length() != length)
        return false;

    if (s.is8Bit())
        return equal(s.characters8(), buffer, length);

    return equal(s.characters16(), buffer, length);
}

template <typename StringType>
bool equal(const StringBuilder& a, const StringType& b)
{
    if (a.length() != b.length())
        return false;

    if (!a.length())
        return true;

    if (a.is8Bit()) {
        if (b.is8Bit())
            return equal(a.characters8(), b.characters8(), a.length());
        return equal(a.characters8(), b.characters16(), a.length());
    }

    if (b.is8Bit())
        return equal(a.characters16(), b.characters8(), a.length());
    return equal(a.characters16(), b.characters16(), a.length());
}

inline bool operator==(const StringBuilder& a, const StringBuilder& b) { return equal(a, b); }
inline bool operator!=(const StringBuilder& a, const StringBuilder& b) { return !equal(a, b); }
inline bool operator==(const StringBuilder& a, const String& b) { return equal(a, b); }
inline bool operator!=(const StringBuilder& a, const String& b) { return !equal(a, b); }
inline bool operator==(const String& a, const StringBuilder& b) { return equal(b, a); }
inline bool operator!=(const String& a, const StringBuilder& b) { return !equal(b, a); }

template<> struct IntegerToStringConversionTrait<StringBuilder> {
    using ReturnType = void;
    using AdditionalArgumentType = StringBuilder;
    static void flush(LChar* characters, unsigned length, StringBuilder* stringBuilder) { stringBuilder->append(characters, length); }
};

} // namespace WTF

using WTF::StringBuilder;

#endif // StringBuilder_h
