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

#pragma once

#include "JSCJSValue.h"
#include <wtf/dtoa.h>

namespace JSC {

static const double mantissaOverflowLowerBound = 9007199254740992.0;

ALWAYS_INLINE static int parseDigit(unsigned short c, int radix)
{
    int digit = -1;

    if (isASCIIDigit(c))
        digit = c - '0';
    else if (isASCIIUpper(c))
        digit = c - 'A' + 10;
    else if (isASCIILower(c))
        digit = c - 'a' + 10;

    if (digit >= radix)
        return -1;
    return digit;
}

static double parseIntOverflow(const LChar* s, unsigned length, int radix)
{
    double number = 0.0;
    double radixMultiplier = 1.0;

    for (const LChar* p = s + length - 1; p >= s; p--) {
        if (radixMultiplier == std::numeric_limits<double>::infinity()) {
            if (*p != '0') {
                number = std::numeric_limits<double>::infinity();
                break;
            }
        } else {
            int digit = parseDigit(*p, radix);
            number += digit * radixMultiplier;
        }

        radixMultiplier *= radix;
    }

    return number;
}

static double parseIntOverflow(const UChar* s, unsigned length, int radix)
{
    double number = 0.0;
    double radixMultiplier = 1.0;

    for (const UChar* p = s + length - 1; p >= s; p--) {
        if (radixMultiplier == std::numeric_limits<double>::infinity()) {
            if (*p != '0') {
                number = std::numeric_limits<double>::infinity();
                break;
            }
        } else {
            int digit = parseDigit(*p, radix);
            number += digit * radixMultiplier;
        }

        radixMultiplier *= radix;
    }

    return number;
}

static double parseIntOverflow(StringView string, int radix)
{
    if (string.is8Bit())
        return parseIntOverflow(string.characters8(), string.length(), radix);
    return parseIntOverflow(string.characters16(), string.length(), radix);
}

ALWAYS_INLINE static bool isStrWhiteSpace(UChar c)
{
    switch (c) {
    // ECMA-262-5th 7.2 & 7.3
    case 0x0009:
    case 0x000A:
    case 0x000B:
    case 0x000C:
    case 0x000D:
    case 0x0020:
    case 0x00A0:
    case 0x180E: // This character used to be in Zs category before Unicode 6.3, and EcmaScript says that we should keep treating it as such.
    case 0x2028:
    case 0x2029:
    case 0xFEFF:
        return true;
    default:
        return c > 0xFF && u_charType(c) == U_SPACE_SEPARATOR;
    }
}

// ES5.1 15.1.2.2
template <typename CharType>
ALWAYS_INLINE
static double parseInt(StringView s, const CharType* data, int radix)
{
    // 1. Let inputString be ToString(string).
    // 2. Let S be a newly created substring of inputString consisting of the first character that is not a
    //    StrWhiteSpaceChar and all characters following that character. (In other words, remove leading white
    //    space.) If inputString does not contain any such characters, let S be the empty string.
    int length = s.length();
    int p = 0;
    while (p < length && isStrWhiteSpace(data[p]))
        ++p;

    // 3. Let sign be 1.
    // 4. If S is not empty and the first character of S is a minus sign -, let sign be -1.
    // 5. If S is not empty and the first character of S is a plus sign + or a minus sign -, then remove the first character from S.
    double sign = 1;
    if (p < length) {
        if (data[p] == '+')
            ++p;
        else if (data[p] == '-') {
            sign = -1;
            ++p;
        }
    }

    // 6. Let R = ToInt32(radix).
    // 7. Let stripPrefix be true.
    // 8. If R != 0,then
    //   b. If R != 16, let stripPrefix be false.
    // 9. Else, R == 0
    //   a. LetR = 10.
    // 10. If stripPrefix is true, then
    //   a. If the length of S is at least 2 and the first two characters of S are either ―0x or ―0X,
    //      then remove the first two characters from S and let R = 16.
    // 11. If S contains any character that is not a radix-R digit, then let Z be the substring of S
    //     consisting of all characters before the first such character; otherwise, let Z be S.
    if ((radix == 0 || radix == 16) && length - p >= 2 && data[p] == '0' && (data[p + 1] == 'x' || data[p + 1] == 'X')) {
        radix = 16;
        p += 2;
    } else if (radix == 0)
        radix = 10;

    // 8.a If R < 2 or R > 36, then return NaN.
    if (radix < 2 || radix > 36)
        return PNaN;

    // 13. Let mathInt be the mathematical integer value that is represented by Z in radix-R notation, using the letters
    //     A-Z and a-z for digits with values 10 through 35. (However, if R is 10 and Z contains more than 20 significant
    //     digits, every significant digit after the 20th may be replaced by a 0 digit, at the option of the implementation;
    //     and if R is not 2, 4, 8, 10, 16, or 32, then mathInt may be an implementation-dependent approximation to the
    //     mathematical integer value that is represented by Z in radix-R notation.)
    // 14. Let number be the Number value for mathInt.
    int firstDigitPosition = p;
    bool sawDigit = false;
    double number = 0;
    while (p < length) {
        int digit = parseDigit(data[p], radix);
        if (digit == -1)
            break;
        sawDigit = true;
        number *= radix;
        number += digit;
        ++p;
    }

    // 12. If Z is empty, return NaN.
    if (!sawDigit)
        return PNaN;

    // Alternate code path for certain large numbers.
    if (number >= mantissaOverflowLowerBound) {
        if (radix == 10) {
            size_t parsedLength;
            number = parseDouble(s.substring(firstDigitPosition, p - firstDigitPosition), parsedLength);
        } else if (radix == 2 || radix == 4 || radix == 8 || radix == 16 || radix == 32)
            number = parseIntOverflow(s.substring(firstDigitPosition, p - firstDigitPosition), radix);
    }

    // 15. Return sign x number.
    return sign * number;
}

ALWAYS_INLINE static double parseInt(StringView s, int radix)
{
    if (s.is8Bit())
        return parseInt(s, s.characters8(), radix);
    return parseInt(s, s.characters16(), radix);
}

template<typename CallbackWhenNoException>
static ALWAYS_INLINE typename std::result_of<CallbackWhenNoException(StringView)>::type toStringView(ExecState* exec, JSValue value, CallbackWhenNoException callback)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSString* string = value.toStringOrNull(exec);
    if (UNLIKELY(!string))
        return { };
    auto viewWithString = string->viewWithUnderlyingString(*exec);
    RETURN_IF_EXCEPTION(scope, { });
    return callback(viewWithString.view);
}

} // namespace JSC
