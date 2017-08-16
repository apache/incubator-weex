/*
 *  Copyright (C) 2003, 2008, 2012 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
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
