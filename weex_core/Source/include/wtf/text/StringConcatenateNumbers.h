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
