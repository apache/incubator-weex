/*
 * Copyright (C) 2010, 2013, 2016 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#include "config.h"
#include "StringBuilder.h"

#include "IntegerToStringConversion.h"
#include "MathExtras.h"
#include "WTFString.h"
#include <wtf/dtoa.h>

namespace WTF {

static unsigned expandedCapacity(unsigned capacity, unsigned requiredLength)
{
    static const unsigned minimumCapacity = 16;
    return std::max(requiredLength, std::max(minimumCapacity, capacity * 2));
}

void StringBuilder::reifyString() const
{
    // Check if the string already exists.
    if (!m_string.isNull()) {
        ASSERT(m_string.length() == m_length);
        return;
    }

    // Check for empty.
    if (!m_length) {
        m_string = StringImpl::empty();
        return;
    }

    // Must be valid in the buffer, take a substring (unless string fills the buffer).
    ASSERT(m_buffer && m_length <= m_buffer->length());
    if (m_length == m_buffer->length())
        m_string = m_buffer.get();
    else
        m_string = StringImpl::createSubstringSharingImpl(*m_buffer, 0, m_length);
}

void StringBuilder::resize(unsigned newSize)
{
    // Check newSize < m_length, hence m_length > 0.
    ASSERT(newSize <= m_length);
    if (newSize == m_length)
        return;
    ASSERT(m_length);

    // If there is a buffer, we only need to duplicate it if it has more than one ref.
    if (m_buffer) {
        m_string = String(); // Clear the string to remove the reference to m_buffer if any before checking the reference count of m_buffer.
        if (!m_buffer->hasOneRef()) {
            if (m_buffer->is8Bit())
                allocateBuffer(m_buffer->characters8(), m_buffer->length());
            else
                allocateBuffer(m_buffer->characters16(), m_buffer->length());
        }
        m_length = newSize;
        ASSERT(m_buffer->length() >= m_length);
        return;
    }

    // Since m_length && !m_buffer, the string must be valid in m_string, and m_string.length() > 0.
    ASSERT(!m_string.isEmpty());
    ASSERT(m_length == m_string.length());
    ASSERT(newSize < m_string.length());
    m_length = newSize;
    m_string = StringImpl::createSubstringSharingImpl(*m_string.impl(), 0, newSize);
}

// Allocate a new 8 bit buffer, copying in currentCharacters (these may come from either m_string
// or m_buffer, neither will be reassigned until the copy has completed).
void StringBuilder::allocateBuffer(const LChar* currentCharacters, unsigned requiredLength)
{
    ASSERT(m_is8Bit);
    // Copy the existing data into a new buffer, set result to point to the end of the existing data.
    auto buffer = StringImpl::createUninitialized(requiredLength, m_bufferCharacters8);
    memcpy(m_bufferCharacters8, currentCharacters, static_cast<size_t>(m_length) * sizeof(LChar)); // This can't overflow.
    
    // Update the builder state.
    m_buffer = WTFMove(buffer);
    m_string = String();
    ASSERT(m_buffer->length() == requiredLength);
}

// Allocate a new 16 bit buffer, copying in currentCharacters (these may come from either m_string
// or m_buffer,  neither will be reassigned until the copy has completed).
void StringBuilder::allocateBuffer(const UChar* currentCharacters, unsigned requiredLength)
{
    ASSERT(!m_is8Bit);
    // Copy the existing data into a new buffer, set result to point to the end of the existing data.
    auto buffer = StringImpl::createUninitialized(requiredLength, m_bufferCharacters16);
    memcpy(m_bufferCharacters16, currentCharacters, static_cast<size_t>(m_length) * sizeof(UChar)); // This can't overflow.
    
    // Update the builder state.
    m_buffer = WTFMove(buffer);
    m_string = String();
    ASSERT(m_buffer->length() == requiredLength);
}

// Allocate a new 16 bit buffer, copying in currentCharacters (which is 8 bit and may come
// from either m_string or m_buffer, neither will be reassigned until the copy has completed).
void StringBuilder::allocateBufferUpConvert(const LChar* currentCharacters, unsigned requiredLength)
{
    ASSERT(m_is8Bit);
    ASSERT(requiredLength >= m_length);
    // Copy the existing data into a new buffer, set result to point to the end of the existing data.
    auto buffer = StringImpl::createUninitialized(requiredLength, m_bufferCharacters16);
    for (unsigned i = 0; i < m_length; ++i)
        m_bufferCharacters16[i] = currentCharacters[i];
    
    m_is8Bit = false;
    
    // Update the builder state.
    m_buffer = WTFMove(buffer);
    m_string = String();
    ASSERT(m_buffer->length() == requiredLength);
}

template <>
void StringBuilder::reallocateBuffer<LChar>(unsigned requiredLength)
{
    // If the buffer has only one ref (by this StringBuilder), reallocate it,
    // otherwise fall back to "allocate and copy" method.
    m_string = String();
    
    ASSERT(m_is8Bit);
    ASSERT(m_buffer->is8Bit());
    
    if (m_buffer->hasOneRef())
        m_buffer = StringImpl::reallocate(m_buffer.releaseNonNull(), requiredLength, m_bufferCharacters8);
    else
        allocateBuffer(m_buffer->characters8(), requiredLength);
    ASSERT(m_buffer->length() == requiredLength);
}

template <>
void StringBuilder::reallocateBuffer<UChar>(unsigned requiredLength)
{
    // If the buffer has only one ref (by this StringBuilder), reallocate it,
    // otherwise fall back to "allocate and copy" method.
    m_string = String();
    
    if (m_buffer->is8Bit())
        allocateBufferUpConvert(m_buffer->characters8(), requiredLength);
    else if (m_buffer->hasOneRef())
        m_buffer = StringImpl::reallocate(m_buffer.releaseNonNull(), requiredLength, m_bufferCharacters16);
    else
        allocateBuffer(m_buffer->characters16(), requiredLength);
    ASSERT(m_buffer->length() == requiredLength);
}

void StringBuilder::reserveCapacity(unsigned newCapacity)
{
    if (m_buffer) {
        // If there is already a buffer, then grow if necessary.
        if (newCapacity > m_buffer->length()) {
            if (m_buffer->is8Bit())
                reallocateBuffer<LChar>(newCapacity);
            else
                reallocateBuffer<UChar>(newCapacity);
        }
    } else {
        // Grow the string, if necessary.
        if (newCapacity > m_length) {
            if (!m_length) {
                LChar* nullPlaceholder = 0;
                allocateBuffer(nullPlaceholder, newCapacity);
            } else if (m_string.is8Bit())
                allocateBuffer(m_string.characters8(), newCapacity);
            else
                allocateBuffer(m_string.characters16(), newCapacity);
        }
    }
    ASSERT(!newCapacity || m_buffer->length() >= newCapacity);
}

// Make 'length' additional capacity be available in m_buffer, update m_string & m_length,
// return a pointer to the newly allocated storage.
template <typename CharType>
ALWAYS_INLINE CharType* StringBuilder::appendUninitialized(unsigned length)
{
    ASSERT(length);

    // Calculate the new size of the builder after appending.
    unsigned requiredLength = length + m_length;
    if (requiredLength < length)
        CRASH();

    if ((m_buffer) && (requiredLength <= m_buffer->length())) {
        // If the buffer is valid it must be at least as long as the current builder contents!
        ASSERT(m_buffer->length() >= m_length);
        unsigned currentLength = m_length;
        m_string = String();
        m_length = requiredLength;
        return getBufferCharacters<CharType>() + currentLength;
    }
    
    return appendUninitializedSlow<CharType>(requiredLength);
}

// Make 'length' additional capacity be available in m_buffer, update m_string & m_length,
// return a pointer to the newly allocated storage.
template <typename CharType>
CharType* StringBuilder::appendUninitializedSlow(unsigned requiredLength)
{
    ASSERT(requiredLength);

    if (m_buffer) {
        // If the buffer is valid it must be at least as long as the current builder contents!
        ASSERT(m_buffer->length() >= m_length);
        
        reallocateBuffer<CharType>(expandedCapacity(capacity(), requiredLength));
    } else {
        ASSERT(m_string.length() == m_length);
        allocateBuffer(m_length ? m_string.characters<CharType>() : 0, expandedCapacity(capacity(), requiredLength));
    }
    
    CharType* result = getBufferCharacters<CharType>() + m_length;
    m_length = requiredLength;
    ASSERT(m_buffer->length() >= m_length);
    return result;
}

void StringBuilder::append(const UChar* characters, unsigned length)
{
    if (!length)
        return;

    ASSERT(characters);

    if (m_is8Bit) {
        if (length == 1 && !(*characters & ~0xff)) {
            // Append as 8 bit character
            LChar lChar = static_cast<LChar>(*characters);
            append(&lChar, 1);
            return;
        }

        // Calculate the new size of the builder after appending.
        unsigned requiredLength = length + m_length;
        if (requiredLength < length)
            CRASH();
        
        if (m_buffer) {
            // If the buffer is valid it must be at least as long as the current builder contents!
            ASSERT(m_buffer->length() >= m_length);
            
            allocateBufferUpConvert(m_buffer->characters8(), expandedCapacity(capacity(), requiredLength));
        } else {
            ASSERT(m_string.length() == m_length);
            allocateBufferUpConvert(m_string.isNull() ? 0 : m_string.characters8(), expandedCapacity(capacity(), requiredLength));
        }

        memcpy(m_bufferCharacters16 + m_length, characters, static_cast<size_t>(length) * sizeof(UChar));
        m_length = requiredLength;
    } else
        memcpy(appendUninitialized<UChar>(length), characters, static_cast<size_t>(length) * sizeof(UChar));
    ASSERT(m_buffer->length() >= m_length);
}

void StringBuilder::append(const LChar* characters, unsigned length)
{
    if (!length)
        return;
    ASSERT(characters);

    if (m_is8Bit) {
        LChar* dest = appendUninitialized<LChar>(length);
        if (length > 8)
            memcpy(dest, characters, static_cast<size_t>(length) * sizeof(LChar));
        else {
            const LChar* end = characters + length;
            while (characters < end)
                *(dest++) = *(characters++);
        }
    } else {
        UChar* dest = appendUninitialized<UChar>(length);
        const LChar* end = characters + length;
        while (characters < end)
            *(dest++) = *(characters++);
    }
}

#if USE(CF)

void StringBuilder::append(CFStringRef string)
{
    // Fast path: avoid constructing a temporary String when possible.
    if (auto* characters = CFStringGetCStringPtr(string, kCFStringEncodingISOLatin1)) {
        append(reinterpret_cast<const LChar*>(characters), CFStringGetLength(string));
        return;
    }
    append(String(string));
}

#endif

void StringBuilder::appendNumber(int number)
{
    numberToStringSigned<StringBuilder>(number, this);
}

void StringBuilder::appendNumber(unsigned int number)
{
    numberToStringUnsigned<StringBuilder>(number, this);
}

void StringBuilder::appendNumber(long number)
{
    numberToStringSigned<StringBuilder>(number, this);
}

void StringBuilder::appendNumber(unsigned long number)
{
    numberToStringUnsigned<StringBuilder>(number, this);
}

void StringBuilder::appendNumber(long long number)
{
    numberToStringSigned<StringBuilder>(number, this);
}

void StringBuilder::appendNumber(unsigned long long number)
{
    numberToStringUnsigned<StringBuilder>(number, this);
}

void StringBuilder::appendNumber(double number, unsigned precision, TrailingZerosTruncatingPolicy trailingZerosTruncatingPolicy)
{
    NumberToStringBuffer buffer;
    append(numberToFixedPrecisionString(number, precision, buffer, trailingZerosTruncatingPolicy == TruncateTrailingZeros));
}

void StringBuilder::appendECMAScriptNumber(double number)
{
    NumberToStringBuffer buffer;
    append(numberToString(number, buffer));
}

void StringBuilder::appendFixedWidthNumber(double number, unsigned decimalPlaces)
{
    NumberToStringBuffer buffer;
    append(numberToFixedWidthString(number, decimalPlaces, buffer));
}

bool StringBuilder::canShrink() const
{
    // Only shrink the buffer if it's less than 80% full. Need to tune this heuristic!
    return m_buffer && m_buffer->length() > (m_length + (m_length >> 2));
}

void StringBuilder::shrinkToFit()
{
    if (canShrink()) {
        if (m_is8Bit)
            reallocateBuffer<LChar>(m_length);
        else
            reallocateBuffer<UChar>(m_length);
        m_string = WTFMove(m_buffer);
    }
}

template <typename OutputCharacterType, typename InputCharacterType>
static void appendQuotedJSONStringInternalSlow(OutputCharacterType*& output, const InputCharacterType character)
{
    switch (character) {
    case '\t':
        *output++ = '\\';
        *output++ = 't';
        break;
    case '\r':
        *output++ = '\\';
        *output++ = 'r';
        break;
    case '\n':
        *output++ = '\\';
        *output++ = 'n';
        break;
    case '\f':
        *output++ = '\\';
        *output++ = 'f';
        break;
    case '\b':
        *output++ = '\\';
        *output++ = 'b';
        break;
    default:
        ASSERT(!(character & 0xFF00));
        *output++ = '\\';
        *output++ = 'u';
        *output++ = '0';
        *output++ = '0';
        *output++ = upperNibbleToLowercaseASCIIHexDigit(character);
        *output++ = lowerNibbleToLowercaseASCIIHexDigit(character);
        break;
    }
}

template <typename OutputCharacterType, typename InputCharacterType>
static void appendQuotedJSONStringInternal(OutputCharacterType*& output, const InputCharacterType* input, unsigned length)
{
    for (const InputCharacterType* end = input + length; input != end; ++input) {
        const InputCharacterType character = *input;
        if (LIKELY(character != '"' && character != '\\' && character > 0x1F)) {
            *output++ = character;
            continue;
        }

        if (character == '"' || character == '\\') {
            *output++ = '\\';
            *output++ = character;
            continue;
        }

        appendQuotedJSONStringInternalSlow(output, character);
    }
}

void StringBuilder::appendQuotedJSONString(const String& string)
{
    // Make sure we have enough buffer space to append this string without having
    // to worry about reallocating in the middle.
    // The 2 is for the '"' quotes on each end.
    // The 6 is for characters that need to be \uNNNN encoded.
    Checked<unsigned> stringLength = string.length();
    Checked<unsigned> maximumCapacityRequired = length();
    maximumCapacityRequired += 2 + stringLength * 6;
    unsigned allocationSize = maximumCapacityRequired.unsafeGet();
    // This max() is here to allow us to allocate sizes between the range [2^31, 2^32 - 2] because roundUpToPowerOfTwo(1<<31 + some int smaller than 1<<31) == 0.
    allocationSize = std::max(allocationSize, roundUpToPowerOfTwo(allocationSize));

    if (is8Bit() && !string.is8Bit())
        allocateBufferUpConvert(m_bufferCharacters8, allocationSize);
    else
        reserveCapacity(allocationSize);
    ASSERT(m_buffer->length() >= allocationSize);

    if (is8Bit()) {
        ASSERT(string.is8Bit());
        LChar* output = m_bufferCharacters8 + m_length;
        *output++ = '"';
        appendQuotedJSONStringInternal(output, string.characters8(), string.length());
        *output++ = '"';
        m_length = output - m_bufferCharacters8;
    } else {
        UChar* output = m_bufferCharacters16 + m_length;
        *output++ = '"';
        if (string.is8Bit())
            appendQuotedJSONStringInternal(output, string.characters8(), string.length());
        else
            appendQuotedJSONStringInternal(output, string.characters16(), string.length());
        *output++ = '"';
        m_length = output - m_bufferCharacters16;
    }
    ASSERT(m_buffer->length() >= m_length);
}

} // namespace WTF
