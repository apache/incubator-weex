/*
 * Copyright (C) 2007-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WTF_ASCIICType_h
#define WTF_ASCIICType_h

#include <wtf/Assertions.h>
#include <wtf/text/LChar.h>

// The behavior of many of the functions in the <ctype.h> header is dependent
// on the current locale. But in the WebKit project, all uses of those functions
// are in code processing something that's not locale-specific. These equivalents
// for some of the <ctype.h> functions are named more explicitly, not dependent
// on the C library locale, and we should also optimize them as needed.

// All functions return false or leave the character unchanged if passed a character
// that is outside the range 0-7F. So they can be used on Unicode strings or
// characters if the intent is to do processing only if the character is ASCII.

namespace WTF {

template<typename CharacterType> bool isASCII(CharacterType);
template<typename CharacterType> bool isASCIIAlpha(CharacterType);
template<typename CharacterType> bool isASCIIAlphanumeric(CharacterType);
template<typename CharacterType> bool isASCIIBinaryDigit(CharacterType);
template<typename CharacterType> bool isASCIIDigit(CharacterType);
template<typename CharacterType> bool isASCIIHexDigit(CharacterType);
template<typename CharacterType> bool isASCIILower(CharacterType);
template<typename CharacterType> bool isASCIIOctalDigit(CharacterType);
template<typename CharacterType> bool isASCIIPrintable(CharacterType);
template<typename CharacterType> bool isASCIISpace(CharacterType);
template<typename CharacterType> bool isASCIIUpper(CharacterType);

template<typename CharacterType> CharacterType toASCIILower(CharacterType);
template<typename CharacterType> CharacterType toASCIIUpper(CharacterType);

template<typename CharacterType> uint8_t toASCIIHexValue(CharacterType);
template<typename CharacterType> uint8_t toASCIIHexValue(CharacterType firstCharacter, CharacterType secondCharacter);

char lowerNibbleToASCIIHexDigit(uint8_t);
char upperNibbleToASCIIHexDigit(uint8_t);
char lowerNibbleToLowercaseASCIIHexDigit(uint8_t);
char upperNibbleToLowercaseASCIIHexDigit(uint8_t);

template<typename CharacterType> bool isASCIIAlphaCaselessEqual(CharacterType, char expectedASCIILowercaseLetter);

// The toASCIILowerUnchecked function can be used for comparing any input character
// to a lowercase English character. The isASCIIAlphaCaselessEqual function should
// be used for regular comparison of ASCII alpha characters, but switch statements
// in the CSS tokenizer, for example, instead make direct use toASCIILowerUnchecked.
template<typename CharacterType> CharacterType toASCIILowerUnchecked(CharacterType);

const unsigned char asciiCaseFoldTable[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

template<typename CharacterType> inline bool isASCII(CharacterType character)
{
    return !(character & ~0x7F);
}

template<typename CharacterType> inline bool isASCIILower(CharacterType character)
{
    return character >= 'a' && character <= 'z';
}

template<typename CharacterType> inline CharacterType toASCIILowerUnchecked(CharacterType character)
{
    // This function can be used for comparing any input character
    // to a lowercase English character. The isASCIIAlphaCaselessEqual
    // below should be used for regular comparison of ASCII alpha
    // characters, but switch statements in CSS tokenizer instead make
    // direct use of this function.
    return character | 0x20;
}

template<typename CharacterType> inline bool isASCIIAlpha(CharacterType character)
{
    return isASCIILower(toASCIILowerUnchecked(character));
}

template<typename CharacterType> inline bool isASCIIDigit(CharacterType character)
{
    return character >= '0' && character <= '9';
}

template<typename CharacterType> inline bool isASCIIAlphanumeric(CharacterType character)
{
    return isASCIIDigit(character) || isASCIIAlpha(character);
}

template<typename CharacterType> inline bool isASCIIHexDigit(CharacterType character)
{
    return isASCIIDigit(character) || (toASCIILowerUnchecked(character) >= 'a' && toASCIILowerUnchecked(character) <= 'f');
}

template<typename CharacterType> inline bool isASCIIBinaryDigit(CharacterType character)
{
    return character == '0' || character == '1';
}

template<typename CharacterType> inline bool isASCIIOctalDigit(CharacterType character)
{
    return character >= '0' && character <= '7';
}

template<typename CharacterType> inline bool isASCIIPrintable(CharacterType character)
{
    return character >= ' ' && character <= '~';
}

/*
    Statistics from a run of Apple's page load test for callers of isASCIISpace:

    character          count
    ---------          -----
    non-spaces         689383
    20  space          294720
    0A  \n             89059
    09  \t             28320
    0D  \r             0
    0C  \f             0
    0B  \v             0

    Because of those, we first check to quickly return false for non-control characters,
    then check for space itself to quickly return true for that case, then do the rest.
*/
template<typename CharacterType> inline bool isASCIISpace(CharacterType character)
{
    return character <= ' ' && (character == ' ' || (character <= 0xD && character >= 0x9));
}

template<typename CharacterType> inline bool isASCIIUpper(CharacterType character)
{
    return character >= 'A' && character <= 'Z';
}

template<typename CharacterType> inline CharacterType toASCIILower(CharacterType character)
{
    return character | (isASCIIUpper(character) << 5);
}

template<> inline char toASCIILower(char character)
{
    return static_cast<char>(asciiCaseFoldTable[static_cast<uint8_t>(character)]);
}

template<> inline LChar toASCIILower(LChar character)
{
    return asciiCaseFoldTable[character];
}

template<typename CharacterType> inline CharacterType toASCIIUpper(CharacterType character)
{
    return character & ~(isASCIILower(character) << 5);
}

template<typename CharacterType> inline uint8_t toASCIIHexValue(CharacterType character)
{
    ASSERT(isASCIIHexDigit(character));
    return character < 'A' ? character - '0' : (character - 'A' + 10) & 0xF;
}

template<typename CharacterType> inline uint8_t toASCIIHexValue(CharacterType firstCharacter, CharacterType secondCharacter)
{
    return toASCIIHexValue(firstCharacter) << 4 | toASCIIHexValue(secondCharacter);
}

inline char lowerNibbleToASCIIHexDigit(uint8_t value)
{
    uint8_t nibble = value & 0xF;
    return nibble + (nibble < 10 ? '0' : 'A' - 10);
}

inline char upperNibbleToASCIIHexDigit(uint8_t value)
{
    uint8_t nibble = value >> 4;
    return nibble + (nibble < 10 ? '0' : 'A' - 10);
}

inline char lowerNibbleToLowercaseASCIIHexDigit(uint8_t value)
{
    uint8_t nibble = value & 0xF;
    return nibble + (nibble < 10 ? '0' : 'a' - 10);
}

inline char upperNibbleToLowercaseASCIIHexDigit(uint8_t value)
{
    uint8_t nibble = value >> 4;
    return nibble + (nibble < 10 ? '0' : 'a' - 10);
}

template<typename CharacterType> inline bool isASCIIAlphaCaselessEqual(CharacterType inputCharacter, char expectedASCIILowercaseLetter)
{
    // Name of this argument says this must be a lowercase letter, but it can actually be:
    //   - a lowercase letter
    //   - a numeric digit
    //   - a space
    //   - punctuation in the range 0x21-0x3F, including "-", "/", and "+"
    // It cannot be:
    //   - an uppercase letter
    //   - a non-ASCII character
    //   - other punctuation, such as underscore and backslash
    //   - a control character such as "\n"
    // FIXME: Would be nice to make both the function name and expectedASCIILowercaseLetter argument name clearer.
    ASSERT(toASCIILowerUnchecked(expectedASCIILowercaseLetter) == expectedASCIILowercaseLetter);
    return LIKELY(toASCIILowerUnchecked(inputCharacter) == expectedASCIILowercaseLetter);
}

}

using WTF::isASCII;
using WTF::isASCIIAlpha;
using WTF::isASCIIAlphaCaselessEqual;
using WTF::isASCIIAlphanumeric;
using WTF::isASCIIBinaryDigit;
using WTF::isASCIIDigit;
using WTF::isASCIIHexDigit;
using WTF::isASCIILower;
using WTF::isASCIIOctalDigit;
using WTF::isASCIIPrintable;
using WTF::isASCIISpace;
using WTF::isASCIIUpper;
using WTF::lowerNibbleToASCIIHexDigit;
using WTF::lowerNibbleToLowercaseASCIIHexDigit;
using WTF::toASCIIHexValue;
using WTF::toASCIILower;
using WTF::toASCIILowerUnchecked;
using WTF::toASCIIUpper;
using WTF::upperNibbleToASCIIHexDigit;
using WTF::upperNibbleToLowercaseASCIIHexDigit;

#endif
