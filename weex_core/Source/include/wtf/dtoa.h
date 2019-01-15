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
#ifndef WTF_dtoa_h
#define WTF_dtoa_h

#include <unicode/utypes.h>
#include <wtf/ASCIICType.h>
#include <wtf/dtoa/double-conversion.h>
#include <wtf/text/StringView.h>

namespace WTF {

typedef char DtoaBuffer[80];

WTF_EXPORT_PRIVATE void dtoa(DtoaBuffer result, double dd, bool& sign, int& exponent, unsigned& precision);
WTF_EXPORT_PRIVATE void dtoaRoundSF(DtoaBuffer result, double dd, int ndigits, bool& sign, int& exponent, unsigned& precision);
WTF_EXPORT_PRIVATE void dtoaRoundDP(DtoaBuffer result, double dd, int ndigits, bool& sign, int& exponent, unsigned& precision);

// Size = 80 for sizeof(DtoaBuffer) + some sign bits, decimal point, 'e', exponent digits.
const unsigned NumberToStringBufferLength = 96;
typedef char NumberToStringBuffer[NumberToStringBufferLength];
typedef LChar NumberToLStringBuffer[NumberToStringBufferLength];

WTF_EXPORT_PRIVATE const char* numberToString(double, NumberToStringBuffer);
WTF_EXPORT_PRIVATE const char* numberToFixedPrecisionString(double, unsigned significantFigures, NumberToStringBuffer, bool truncateTrailingZeros = false);
WTF_EXPORT_PRIVATE const char* numberToFixedWidthString(double, unsigned decimalPlaces, NumberToStringBuffer);

double parseDouble(const LChar* string, size_t length, size_t& parsedLength);
double parseDouble(const UChar* string, size_t length, size_t& parsedLength);
double parseDouble(StringView, size_t& parsedLength);

namespace Internal {
    WTF_EXPORT_PRIVATE double parseDoubleFromLongString(const UChar* string, size_t length, size_t& parsedLength);
}

inline double parseDouble(const LChar* string, size_t length, size_t& parsedLength)
{
    return double_conversion::StringToDoubleConverter::StringToDouble(reinterpret_cast<const char*>(string), length, &parsedLength);
}

inline double parseDouble(const UChar* string, size_t length, size_t& parsedLength)
{
    const size_t conversionBufferSize = 64;
    if (length > conversionBufferSize)
        return Internal::parseDoubleFromLongString(string, length, parsedLength);
    LChar conversionBuffer[conversionBufferSize];
    for (int i = 0; i < static_cast<int>(length); ++i)
        conversionBuffer[i] = isASCII(string[i]) ? string[i] : 0;
    return parseDouble(conversionBuffer, length, parsedLength);
}

inline double parseDouble(StringView string, size_t& parsedLength)
{
    if (string.is8Bit())
        return parseDouble(string.characters8(), string.length(), parsedLength);
    return parseDouble(string.characters16(), string.length(), parsedLength);
}
    
} // namespace WTF

using WTF::NumberToStringBuffer;
using WTF::NumberToLStringBuffer;
using WTF::numberToString;
using WTF::numberToFixedPrecisionString;
using WTF::numberToFixedWidthString;
using WTF::parseDouble;

#endif // WTF_dtoa_h
