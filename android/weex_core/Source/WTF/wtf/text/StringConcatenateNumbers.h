/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include <wtf/dtoa.h>
#include <wtf/text/IntegerToStringConversion.h>
#include <wtf/text/StringConcatenate.h>

namespace WTF {

template<>
class StringTypeAdapter<int> {
public:
    StringTypeAdapter<int>(int number)
        : m_number(number)
    {
    }

    unsigned length() const { return lengthOfNumberAsStringSigned(m_number); }
    bool is8Bit() const { return true; }

    void writeTo(LChar* destination) const { writeNumberToBufferSigned(m_number, destination); }
    void writeTo(UChar* destination) const { writeNumberToBufferSigned(m_number, destination); }

    String toString() const { return String::number(m_number); }

private:
    int m_number;
};

template<>
class StringTypeAdapter<unsigned> {
public:
    StringTypeAdapter<unsigned>(unsigned number)
        : m_number(number)
    {
    }

    unsigned length() const { return lengthOfNumberAsStringUnsigned(m_number); }
    bool is8Bit() const { return true; }

    void writeTo(LChar* destination) const { writeNumberToBufferUnsigned(m_number, destination); }
    void writeTo(UChar* destination) const { writeNumberToBufferUnsigned(m_number, destination); }

    String toString() const { return String::number(m_number); }

private:
    unsigned m_number;
};

template<>
class StringTypeAdapter<double> {
public:
    StringTypeAdapter<double>(double number)
    {
        numberToString(number, m_buffer);
        m_length = strlen(m_buffer);
    }

    unsigned length() const { return m_length; }
    bool is8Bit() const { return true; }

    void writeTo(LChar* destination) const
    {
        for (unsigned i = 0; i < m_length; ++i)
            destination[i] = m_buffer[i];
    }

    void writeTo(UChar* destination) const
    {
        for (unsigned i = 0; i < m_length; ++i)
            destination[i] = m_buffer[i];
    }

    String toString() const { return { m_buffer, m_length }; }

private:
    NumberToStringBuffer m_buffer;
    unsigned m_length;
};

template<>
class StringTypeAdapter<float> : public StringTypeAdapter<double> {
public:
    StringTypeAdapter<float>(float number)
        : StringTypeAdapter<double>(number)
    {
    }
};

class FormattedNumber {
public:
    static FormattedNumber fixedPrecision(double number, unsigned significantFigures = 6, bool truncateTrailingZeros = false)
    {
        FormattedNumber numberFormatter;
        numberToFixedPrecisionString(number, significantFigures, numberFormatter.m_buffer, truncateTrailingZeros);
        numberFormatter.m_length = strlen(numberFormatter.m_buffer);
        return numberFormatter;
    }

    static FormattedNumber fixedWidth(double number, unsigned decimalPlaces)
    {
        FormattedNumber numberFormatter;
        numberToFixedWidthString(number, decimalPlaces, numberFormatter.m_buffer);
        numberFormatter.m_length = strlen(numberFormatter.m_buffer);
        return numberFormatter;
    }

    unsigned length() const { return m_length; }
    const LChar* buffer() const { return reinterpret_cast<const LChar*>(m_buffer); }

private:
    NumberToStringBuffer m_buffer;
    unsigned m_length;
};

template<>
class StringTypeAdapter<FormattedNumber> {
public:
    StringTypeAdapter<FormattedNumber>(const FormattedNumber& numberFormatter)
        : m_numberFormatter(numberFormatter)
    {
    }

    unsigned length() const { return m_numberFormatter.length(); }
    bool is8Bit() const { return true; }

    void writeTo(LChar* destination) const
    {
        auto buffer = m_numberFormatter.buffer();
        auto length = m_numberFormatter.length();
        for (unsigned i = 0; i < length; ++i)
            destination[i] = buffer[i];
    }

    void writeTo(UChar* destination) const
    {
        auto buffer = m_numberFormatter.buffer();
        auto length = m_numberFormatter.length();
        for (unsigned i = 0; i < length; ++i)
            destination[i] = buffer[i];
    }

    String toString() const { return { m_numberFormatter.buffer(), m_numberFormatter.length() }; }

private:
    const FormattedNumber& m_numberFormatter;
};

}

using WTF::FormattedNumber;
