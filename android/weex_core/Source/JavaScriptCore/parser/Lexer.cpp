/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2006-2009, 2011-2013, 2016 Apple Inc. All Rights Reserved.
 *  Copyright (C) 2007 Cameron Zwarich (cwzwarich@uwaterloo.ca)
 *  Copyright (C) 2010 Zoltan Herczeg (zherczeg@inf.u-szeged.hu)
 *  Copyright (C) 2012 Mathias Bynens (mathias@qiwi.be)
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

#include "config.h"
#include "Lexer.h"

#include "BuiltinNames.h"
#include "Identifier.h"
#include "JSCInlines.h"
#include "JSFunctionInlines.h"
#include "KeywordLookup.h"
#include "Lexer.lut.h"
#include "Nodes.h"
#include "ParseInt.h"
#include "Parser.h"
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <wtf/Assertions.h>
#include <wtf/dtoa.h>

namespace JSC {

bool isLexerKeyword(const Identifier& identifier)
{
    return JSC::mainTable.entry(identifier);
}

enum CharacterType {
    // Types for the main switch

    // The first three types are fixed, and also used for identifying
    // ASCII alpha and alphanumeric characters (see isIdentStart and isIdentPart).
    CharacterIdentifierStart,
    CharacterZero,
    CharacterNumber,

    CharacterInvalid,
    CharacterLineTerminator,
    CharacterExclamationMark,
    CharacterOpenParen,
    CharacterCloseParen,
    CharacterOpenBracket,
    CharacterCloseBracket,
    CharacterComma,
    CharacterColon,
    CharacterQuestion,
    CharacterTilde,
    CharacterQuote,
    CharacterBackQuote,
    CharacterDot,
    CharacterSlash,
    CharacterBackSlash,
    CharacterSemicolon,
    CharacterOpenBrace,
    CharacterCloseBrace,

    CharacterAdd,
    CharacterSub,
    CharacterMultiply,
    CharacterModulo,
    CharacterAnd,
    CharacterXor,
    CharacterOr,
    CharacterLess,
    CharacterGreater,
    CharacterEqual,

    // Other types (only one so far)
    CharacterWhiteSpace,
    CharacterPrivateIdentifierStart
};

// 256 Latin-1 codes
static const unsigned short typesOfLatin1Characters[256] = {
/*   0 - Null               */ CharacterInvalid,
/*   1 - Start of Heading   */ CharacterInvalid,
/*   2 - Start of Text      */ CharacterInvalid,
/*   3 - End of Text        */ CharacterInvalid,
/*   4 - End of Transm.     */ CharacterInvalid,
/*   5 - Enquiry            */ CharacterInvalid,
/*   6 - Acknowledgment     */ CharacterInvalid,
/*   7 - Bell               */ CharacterInvalid,
/*   8 - Back Space         */ CharacterInvalid,
/*   9 - Horizontal Tab     */ CharacterWhiteSpace,
/*  10 - Line Feed          */ CharacterLineTerminator,
/*  11 - Vertical Tab       */ CharacterWhiteSpace,
/*  12 - Form Feed          */ CharacterWhiteSpace,
/*  13 - Carriage Return    */ CharacterLineTerminator,
/*  14 - Shift Out          */ CharacterInvalid,
/*  15 - Shift In           */ CharacterInvalid,
/*  16 - Data Line Escape   */ CharacterInvalid,
/*  17 - Device Control 1   */ CharacterInvalid,
/*  18 - Device Control 2   */ CharacterInvalid,
/*  19 - Device Control 3   */ CharacterInvalid,
/*  20 - Device Control 4   */ CharacterInvalid,
/*  21 - Negative Ack.      */ CharacterInvalid,
/*  22 - Synchronous Idle   */ CharacterInvalid,
/*  23 - End of Transmit    */ CharacterInvalid,
/*  24 - Cancel             */ CharacterInvalid,
/*  25 - End of Medium      */ CharacterInvalid,
/*  26 - Substitute         */ CharacterInvalid,
/*  27 - Escape             */ CharacterInvalid,
/*  28 - File Separator     */ CharacterInvalid,
/*  29 - Group Separator    */ CharacterInvalid,
/*  30 - Record Separator   */ CharacterInvalid,
/*  31 - Unit Separator     */ CharacterInvalid,
/*  32 - Space              */ CharacterWhiteSpace,
/*  33 - !                  */ CharacterExclamationMark,
/*  34 - "                  */ CharacterQuote,
/*  35 - #                  */ CharacterInvalid,
/*  36 - $                  */ CharacterIdentifierStart,
/*  37 - %                  */ CharacterModulo,
/*  38 - &                  */ CharacterAnd,
/*  39 - '                  */ CharacterQuote,
/*  40 - (                  */ CharacterOpenParen,
/*  41 - )                  */ CharacterCloseParen,
/*  42 - *                  */ CharacterMultiply,
/*  43 - +                  */ CharacterAdd,
/*  44 - ,                  */ CharacterComma,
/*  45 - -                  */ CharacterSub,
/*  46 - .                  */ CharacterDot,
/*  47 - /                  */ CharacterSlash,
/*  48 - 0                  */ CharacterZero,
/*  49 - 1                  */ CharacterNumber,
/*  50 - 2                  */ CharacterNumber,
/*  51 - 3                  */ CharacterNumber,
/*  52 - 4                  */ CharacterNumber,
/*  53 - 5                  */ CharacterNumber,
/*  54 - 6                  */ CharacterNumber,
/*  55 - 7                  */ CharacterNumber,
/*  56 - 8                  */ CharacterNumber,
/*  57 - 9                  */ CharacterNumber,
/*  58 - :                  */ CharacterColon,
/*  59 - ;                  */ CharacterSemicolon,
/*  60 - <                  */ CharacterLess,
/*  61 - =                  */ CharacterEqual,
/*  62 - >                  */ CharacterGreater,
/*  63 - ?                  */ CharacterQuestion,
/*  64 - @                  */ CharacterPrivateIdentifierStart,
/*  65 - A                  */ CharacterIdentifierStart,
/*  66 - B                  */ CharacterIdentifierStart,
/*  67 - C                  */ CharacterIdentifierStart,
/*  68 - D                  */ CharacterIdentifierStart,
/*  69 - E                  */ CharacterIdentifierStart,
/*  70 - F                  */ CharacterIdentifierStart,
/*  71 - G                  */ CharacterIdentifierStart,
/*  72 - H                  */ CharacterIdentifierStart,
/*  73 - I                  */ CharacterIdentifierStart,
/*  74 - J                  */ CharacterIdentifierStart,
/*  75 - K                  */ CharacterIdentifierStart,
/*  76 - L                  */ CharacterIdentifierStart,
/*  77 - M                  */ CharacterIdentifierStart,
/*  78 - N                  */ CharacterIdentifierStart,
/*  79 - O                  */ CharacterIdentifierStart,
/*  80 - P                  */ CharacterIdentifierStart,
/*  81 - Q                  */ CharacterIdentifierStart,
/*  82 - R                  */ CharacterIdentifierStart,
/*  83 - S                  */ CharacterIdentifierStart,
/*  84 - T                  */ CharacterIdentifierStart,
/*  85 - U                  */ CharacterIdentifierStart,
/*  86 - V                  */ CharacterIdentifierStart,
/*  87 - W                  */ CharacterIdentifierStart,
/*  88 - X                  */ CharacterIdentifierStart,
/*  89 - Y                  */ CharacterIdentifierStart,
/*  90 - Z                  */ CharacterIdentifierStart,
/*  91 - [                  */ CharacterOpenBracket,
/*  92 - \                  */ CharacterBackSlash,
/*  93 - ]                  */ CharacterCloseBracket,
/*  94 - ^                  */ CharacterXor,
/*  95 - _                  */ CharacterIdentifierStart,
/*  96 - `                  */ CharacterBackQuote,
/*  97 - a                  */ CharacterIdentifierStart,
/*  98 - b                  */ CharacterIdentifierStart,
/*  99 - c                  */ CharacterIdentifierStart,
/* 100 - d                  */ CharacterIdentifierStart,
/* 101 - e                  */ CharacterIdentifierStart,
/* 102 - f                  */ CharacterIdentifierStart,
/* 103 - g                  */ CharacterIdentifierStart,
/* 104 - h                  */ CharacterIdentifierStart,
/* 105 - i                  */ CharacterIdentifierStart,
/* 106 - j                  */ CharacterIdentifierStart,
/* 107 - k                  */ CharacterIdentifierStart,
/* 108 - l                  */ CharacterIdentifierStart,
/* 109 - m                  */ CharacterIdentifierStart,
/* 110 - n                  */ CharacterIdentifierStart,
/* 111 - o                  */ CharacterIdentifierStart,
/* 112 - p                  */ CharacterIdentifierStart,
/* 113 - q                  */ CharacterIdentifierStart,
/* 114 - r                  */ CharacterIdentifierStart,
/* 115 - s                  */ CharacterIdentifierStart,
/* 116 - t                  */ CharacterIdentifierStart,
/* 117 - u                  */ CharacterIdentifierStart,
/* 118 - v                  */ CharacterIdentifierStart,
/* 119 - w                  */ CharacterIdentifierStart,
/* 120 - x                  */ CharacterIdentifierStart,
/* 121 - y                  */ CharacterIdentifierStart,
/* 122 - z                  */ CharacterIdentifierStart,
/* 123 - {                  */ CharacterOpenBrace,
/* 124 - |                  */ CharacterOr,
/* 125 - }                  */ CharacterCloseBrace,
/* 126 - ~                  */ CharacterTilde,
/* 127 - Delete             */ CharacterInvalid,
/* 128 - Cc category        */ CharacterInvalid,
/* 129 - Cc category        */ CharacterInvalid,
/* 130 - Cc category        */ CharacterInvalid,
/* 131 - Cc category        */ CharacterInvalid,
/* 132 - Cc category        */ CharacterInvalid,
/* 133 - Cc category        */ CharacterInvalid,
/* 134 - Cc category        */ CharacterInvalid,
/* 135 - Cc category        */ CharacterInvalid,
/* 136 - Cc category        */ CharacterInvalid,
/* 137 - Cc category        */ CharacterInvalid,
/* 138 - Cc category        */ CharacterInvalid,
/* 139 - Cc category        */ CharacterInvalid,
/* 140 - Cc category        */ CharacterInvalid,
/* 141 - Cc category        */ CharacterInvalid,
/* 142 - Cc category        */ CharacterInvalid,
/* 143 - Cc category        */ CharacterInvalid,
/* 144 - Cc category        */ CharacterInvalid,
/* 145 - Cc category        */ CharacterInvalid,
/* 146 - Cc category        */ CharacterInvalid,
/* 147 - Cc category        */ CharacterInvalid,
/* 148 - Cc category        */ CharacterInvalid,
/* 149 - Cc category        */ CharacterInvalid,
/* 150 - Cc category        */ CharacterInvalid,
/* 151 - Cc category        */ CharacterInvalid,
/* 152 - Cc category        */ CharacterInvalid,
/* 153 - Cc category        */ CharacterInvalid,
/* 154 - Cc category        */ CharacterInvalid,
/* 155 - Cc category        */ CharacterInvalid,
/* 156 - Cc category        */ CharacterInvalid,
/* 157 - Cc category        */ CharacterInvalid,
/* 158 - Cc category        */ CharacterInvalid,
/* 159 - Cc category        */ CharacterInvalid,
/* 160 - Zs category (nbsp) */ CharacterWhiteSpace,
/* 161 - Po category        */ CharacterInvalid,
/* 162 - Sc category        */ CharacterInvalid,
/* 163 - Sc category        */ CharacterInvalid,
/* 164 - Sc category        */ CharacterInvalid,
/* 165 - Sc category        */ CharacterInvalid,
/* 166 - So category        */ CharacterInvalid,
/* 167 - So category        */ CharacterInvalid,
/* 168 - Sk category        */ CharacterInvalid,
/* 169 - So category        */ CharacterInvalid,
/* 170 - Ll category        */ CharacterIdentifierStart,
/* 171 - Pi category        */ CharacterInvalid,
/* 172 - Sm category        */ CharacterInvalid,
/* 173 - Cf category        */ CharacterInvalid,
/* 174 - So category        */ CharacterInvalid,
/* 175 - Sk category        */ CharacterInvalid,
/* 176 - So category        */ CharacterInvalid,
/* 177 - Sm category        */ CharacterInvalid,
/* 178 - No category        */ CharacterInvalid,
/* 179 - No category        */ CharacterInvalid,
/* 180 - Sk category        */ CharacterInvalid,
/* 181 - Ll category        */ CharacterIdentifierStart,
/* 182 - So category        */ CharacterInvalid,
/* 183 - Po category        */ CharacterInvalid,
/* 184 - Sk category        */ CharacterInvalid,
/* 185 - No category        */ CharacterInvalid,
/* 186 - Ll category        */ CharacterIdentifierStart,
/* 187 - Pf category        */ CharacterInvalid,
/* 188 - No category        */ CharacterInvalid,
/* 189 - No category        */ CharacterInvalid,
/* 190 - No category        */ CharacterInvalid,
/* 191 - Po category        */ CharacterInvalid,
/* 192 - Lu category        */ CharacterIdentifierStart,
/* 193 - Lu category        */ CharacterIdentifierStart,
/* 194 - Lu category        */ CharacterIdentifierStart,
/* 195 - Lu category        */ CharacterIdentifierStart,
/* 196 - Lu category        */ CharacterIdentifierStart,
/* 197 - Lu category        */ CharacterIdentifierStart,
/* 198 - Lu category        */ CharacterIdentifierStart,
/* 199 - Lu category        */ CharacterIdentifierStart,
/* 200 - Lu category        */ CharacterIdentifierStart,
/* 201 - Lu category        */ CharacterIdentifierStart,
/* 202 - Lu category        */ CharacterIdentifierStart,
/* 203 - Lu category        */ CharacterIdentifierStart,
/* 204 - Lu category        */ CharacterIdentifierStart,
/* 205 - Lu category        */ CharacterIdentifierStart,
/* 206 - Lu category        */ CharacterIdentifierStart,
/* 207 - Lu category        */ CharacterIdentifierStart,
/* 208 - Lu category        */ CharacterIdentifierStart,
/* 209 - Lu category        */ CharacterIdentifierStart,
/* 210 - Lu category        */ CharacterIdentifierStart,
/* 211 - Lu category        */ CharacterIdentifierStart,
/* 212 - Lu category        */ CharacterIdentifierStart,
/* 213 - Lu category        */ CharacterIdentifierStart,
/* 214 - Lu category        */ CharacterIdentifierStart,
/* 215 - Sm category        */ CharacterInvalid,
/* 216 - Lu category        */ CharacterIdentifierStart,
/* 217 - Lu category        */ CharacterIdentifierStart,
/* 218 - Lu category        */ CharacterIdentifierStart,
/* 219 - Lu category        */ CharacterIdentifierStart,
/* 220 - Lu category        */ CharacterIdentifierStart,
/* 221 - Lu category        */ CharacterIdentifierStart,
/* 222 - Lu category        */ CharacterIdentifierStart,
/* 223 - Ll category        */ CharacterIdentifierStart,
/* 224 - Ll category        */ CharacterIdentifierStart,
/* 225 - Ll category        */ CharacterIdentifierStart,
/* 226 - Ll category        */ CharacterIdentifierStart,
/* 227 - Ll category        */ CharacterIdentifierStart,
/* 228 - Ll category        */ CharacterIdentifierStart,
/* 229 - Ll category        */ CharacterIdentifierStart,
/* 230 - Ll category        */ CharacterIdentifierStart,
/* 231 - Ll category        */ CharacterIdentifierStart,
/* 232 - Ll category        */ CharacterIdentifierStart,
/* 233 - Ll category        */ CharacterIdentifierStart,
/* 234 - Ll category        */ CharacterIdentifierStart,
/* 235 - Ll category        */ CharacterIdentifierStart,
/* 236 - Ll category        */ CharacterIdentifierStart,
/* 237 - Ll category        */ CharacterIdentifierStart,
/* 238 - Ll category        */ CharacterIdentifierStart,
/* 239 - Ll category        */ CharacterIdentifierStart,
/* 240 - Ll category        */ CharacterIdentifierStart,
/* 241 - Ll category        */ CharacterIdentifierStart,
/* 242 - Ll category        */ CharacterIdentifierStart,
/* 243 - Ll category        */ CharacterIdentifierStart,
/* 244 - Ll category        */ CharacterIdentifierStart,
/* 245 - Ll category        */ CharacterIdentifierStart,
/* 246 - Ll category        */ CharacterIdentifierStart,
/* 247 - Sm category        */ CharacterInvalid,
/* 248 - Ll category        */ CharacterIdentifierStart,
/* 249 - Ll category        */ CharacterIdentifierStart,
/* 250 - Ll category        */ CharacterIdentifierStart,
/* 251 - Ll category        */ CharacterIdentifierStart,
/* 252 - Ll category        */ CharacterIdentifierStart,
/* 253 - Ll category        */ CharacterIdentifierStart,
/* 254 - Ll category        */ CharacterIdentifierStart,
/* 255 - Ll category        */ CharacterIdentifierStart
};

// This table provides the character that results from \X where X is the index in the table beginning
// with SPACE. A table value of 0 means that more processing needs to be done.
static const LChar singleCharacterEscapeValuesForASCII[128] = {
/*   0 - Null               */ 0,
/*   1 - Start of Heading   */ 0,
/*   2 - Start of Text      */ 0,
/*   3 - End of Text        */ 0,
/*   4 - End of Transm.     */ 0,
/*   5 - Enquiry            */ 0,
/*   6 - Acknowledgment     */ 0,
/*   7 - Bell               */ 0,
/*   8 - Back Space         */ 0,
/*   9 - Horizontal Tab     */ 0,
/*  10 - Line Feed          */ 0,
/*  11 - Vertical Tab       */ 0,
/*  12 - Form Feed          */ 0,
/*  13 - Carriage Return    */ 0,
/*  14 - Shift Out          */ 0,
/*  15 - Shift In           */ 0,
/*  16 - Data Line Escape   */ 0,
/*  17 - Device Control 1   */ 0,
/*  18 - Device Control 2   */ 0,
/*  19 - Device Control 3   */ 0,
/*  20 - Device Control 4   */ 0,
/*  21 - Negative Ack.      */ 0,
/*  22 - Synchronous Idle   */ 0,
/*  23 - End of Transmit    */ 0,
/*  24 - Cancel             */ 0,
/*  25 - End of Medium      */ 0,
/*  26 - Substitute         */ 0,
/*  27 - Escape             */ 0,
/*  28 - File Separator     */ 0,
/*  29 - Group Separator    */ 0,
/*  30 - Record Separator   */ 0,
/*  31 - Unit Separator     */ 0,
/*  32 - Space              */ ' ',
/*  33 - !                  */ '!',
/*  34 - "                  */ '"',
/*  35 - #                  */ '#',
/*  36 - $                  */ '$',
/*  37 - %                  */ '%',
/*  38 - &                  */ '&',
/*  39 - '                  */ '\'',
/*  40 - (                  */ '(',
/*  41 - )                  */ ')',
/*  42 - *                  */ '*',
/*  43 - +                  */ '+',
/*  44 - ,                  */ ',',
/*  45 - -                  */ '-',
/*  46 - .                  */ '.',
/*  47 - /                  */ '/',
/*  48 - 0                  */ 0,
/*  49 - 1                  */ 0,
/*  50 - 2                  */ 0,
/*  51 - 3                  */ 0,
/*  52 - 4                  */ 0,
/*  53 - 5                  */ 0,
/*  54 - 6                  */ 0,
/*  55 - 7                  */ 0,
/*  56 - 8                  */ 0,
/*  57 - 9                  */ 0,
/*  58 - :                  */ ':',
/*  59 - ;                  */ ';',
/*  60 - <                  */ '<',
/*  61 - =                  */ '=',
/*  62 - >                  */ '>',
/*  63 - ?                  */ '?',
/*  64 - @                  */ '@',
/*  65 - A                  */ 'A',
/*  66 - B                  */ 'B',
/*  67 - C                  */ 'C',
/*  68 - D                  */ 'D',
/*  69 - E                  */ 'E',
/*  70 - F                  */ 'F',
/*  71 - G                  */ 'G',
/*  72 - H                  */ 'H',
/*  73 - I                  */ 'I',
/*  74 - J                  */ 'J',
/*  75 - K                  */ 'K',
/*  76 - L                  */ 'L',
/*  77 - M                  */ 'M',
/*  78 - N                  */ 'N',
/*  79 - O                  */ 'O',
/*  80 - P                  */ 'P',
/*  81 - Q                  */ 'Q',
/*  82 - R                  */ 'R',
/*  83 - S                  */ 'S',
/*  84 - T                  */ 'T',
/*  85 - U                  */ 'U',
/*  86 - V                  */ 'V',
/*  87 - W                  */ 'W',
/*  88 - X                  */ 'X',
/*  89 - Y                  */ 'Y',
/*  90 - Z                  */ 'Z',
/*  91 - [                  */ '[',
/*  92 - \                  */ '\\',
/*  93 - ]                  */ ']',
/*  94 - ^                  */ '^',
/*  95 - _                  */ '_',
/*  96 - `                  */ '`',
/*  97 - a                  */ 'a',
/*  98 - b                  */ 0x08,
/*  99 - c                  */ 'c',
/* 100 - d                  */ 'd',
/* 101 - e                  */ 'e',
/* 102 - f                  */ 0x0C,
/* 103 - g                  */ 'g',
/* 104 - h                  */ 'h',
/* 105 - i                  */ 'i',
/* 106 - j                  */ 'j',
/* 107 - k                  */ 'k',
/* 108 - l                  */ 'l',
/* 109 - m                  */ 'm',
/* 110 - n                  */ 0x0A,
/* 111 - o                  */ 'o',
/* 112 - p                  */ 'p',
/* 113 - q                  */ 'q',
/* 114 - r                  */ 0x0D,
/* 115 - s                  */ 's',
/* 116 - t                  */ 0x09,
/* 117 - u                  */ 0,
/* 118 - v                  */ 0x0B,
/* 119 - w                  */ 'w',
/* 120 - x                  */ 0,
/* 121 - y                  */ 'y',
/* 122 - z                  */ 'z',
/* 123 - {                  */ '{',
/* 124 - |                  */ '|',
/* 125 - }                  */ '}',
/* 126 - ~                  */ '~',
/* 127 - Delete             */ 0
};

template <typename T>
Lexer<T>::Lexer(VM* vm, JSParserBuiltinMode builtinMode, JSParserScriptMode scriptMode)
    : m_isReparsingFunction(false)
    , m_vm(vm)
    , m_parsingBuiltinFunction(builtinMode == JSParserBuiltinMode::Builtin)
    , m_scriptMode(scriptMode)
{
}

static inline JSTokenType tokenTypeForIntegerLikeToken(double doubleValue)
{
    if ((doubleValue || !std::signbit(doubleValue)) && static_cast<int64_t>(doubleValue) == doubleValue)
        return INTEGER;
    return DOUBLE;
}

template <typename T>
Lexer<T>::~Lexer()
{
}

template <typename T>
String Lexer<T>::invalidCharacterMessage() const
{
    switch (m_current) {
    case 0:
        return ASCIILiteral("Invalid character: '\\0'");
    case 10:
        return ASCIILiteral("Invalid character: '\\n'");
    case 11:
        return ASCIILiteral("Invalid character: '\\v'");
    case 13:
        return ASCIILiteral("Invalid character: '\\r'");
    case 35:
        return ASCIILiteral("Invalid character: '#'");
    case 64:
        return ASCIILiteral("Invalid character: '@'");
    case 96:
        return ASCIILiteral("Invalid character: '`'");
    default:
        return String::format("Invalid character '\\u%04u'", static_cast<unsigned>(m_current));
    }
}

template <typename T>
ALWAYS_INLINE const T* Lexer<T>::currentSourcePtr() const
{
    ASSERT(m_code <= m_codeEnd);
    return m_code;
}

template <typename T>
void Lexer<T>::setCode(const SourceCode& source, ParserArena* arena)
{
    m_arena = &arena->identifierArena();
    
    m_lineNumber = source.firstLine().oneBasedInt();
    m_lastToken = -1;
    
    StringView sourceString = source.provider()->source();

    if (!sourceString.isNull())
        setCodeStart(sourceString);
    else
        m_codeStart = 0;

    m_source = &source;
    m_sourceOffset = source.startOffset();
    m_codeStartPlusOffset = m_codeStart + source.startOffset();
    m_code = m_codeStartPlusOffset;
    m_codeEnd = m_codeStart + source.endOffset();
    m_error = false;
    m_atLineStart = true;
    m_lineStart = m_code;
    m_lexErrorMessage = String();
    m_sourceURLDirective = String();
    m_sourceMappingURLDirective = String();
    
    m_buffer8.reserveInitialCapacity(initialReadBufferCapacity);
    m_buffer16.reserveInitialCapacity((m_codeEnd - m_code) / 2);
    m_bufferForRawTemplateString16.reserveInitialCapacity(initialReadBufferCapacity);
    
    if (LIKELY(m_code < m_codeEnd))
        m_current = *m_code;
    else
        m_current = 0;
    ASSERT(currentOffset() == source.startOffset());
}

template <typename T>
template <int shiftAmount> ALWAYS_INLINE void Lexer<T>::internalShift()
{
    m_code += shiftAmount;
    ASSERT(currentOffset() >= currentLineStartOffset());
    m_current = *m_code;
}

template <typename T>
ALWAYS_INLINE void Lexer<T>::shift()
{
    // At one point timing showed that setting m_current to 0 unconditionally was faster than an if-else sequence.
    m_current = 0;
    ++m_code;
    if (LIKELY(m_code < m_codeEnd))
        m_current = *m_code;
}

template <typename T>
ALWAYS_INLINE bool Lexer<T>::atEnd() const
{
    ASSERT(!m_current || m_code < m_codeEnd);
    return UNLIKELY(UNLIKELY(!m_current) && m_code == m_codeEnd);
}

template <typename T>
ALWAYS_INLINE T Lexer<T>::peek(int offset) const
{
    ASSERT(offset > 0 && offset < 5);
    const T* code = m_code + offset;
    return (code < m_codeEnd) ? *code : 0;
}

struct ParsedUnicodeEscapeValue {
    ParsedUnicodeEscapeValue(UChar32 value)
        : m_value(value)
    {
        ASSERT(isValid());
    }

    enum SpecialValueType { Incomplete = -2, Invalid = -1 };
    ParsedUnicodeEscapeValue(SpecialValueType type)
        : m_value(type)
    {
    }

    bool isValid() const { return m_value >= 0; }
    bool isIncomplete() const { return m_value == Incomplete; }

    UChar32 value() const
    {
        ASSERT(isValid());
        return m_value;
    }

private:
    UChar32 m_value;
};

template<typename CharacterType>
ParsedUnicodeEscapeValue Lexer<CharacterType>::parseUnicodeEscape()
{
    if (m_current == '{') {
        shift();
        UChar32 codePoint = 0;
        do {
            if (!isASCIIHexDigit(m_current))
                return m_current ? ParsedUnicodeEscapeValue::Invalid : ParsedUnicodeEscapeValue::Incomplete;
            codePoint = (codePoint << 4) | toASCIIHexValue(m_current);
            if (codePoint > UCHAR_MAX_VALUE) {
                // For raw template literal syntax, we consume `NotEscapeSequence`.
                // Here, we consume NotCodePoint's HexDigits.
                //
                // NotEscapeSequence ::
                //     u { [lookahread not one of HexDigit]
                //     u { NotCodePoint
                //     u { CodePoint [lookahead != }]
                //
                // NotCodePoint ::
                //     HexDigits but not if MV of HexDigits <= 0x10FFFF
                //
                // CodePoint ::
                //     HexDigits but not if MV of HexDigits > 0x10FFFF
                shift();
                while (isASCIIHexDigit(m_current))
                    shift();

                return atEnd() ? ParsedUnicodeEscapeValue::Incomplete : ParsedUnicodeEscapeValue::Invalid;
            }
            shift();
        } while (m_current != '}');
        shift();
        return codePoint;
    }

    auto character2 = peek(1);
    auto character3 = peek(2);
    auto character4 = peek(3);
    if (UNLIKELY(!isASCIIHexDigit(m_current) || !isASCIIHexDigit(character2) || !isASCIIHexDigit(character3) || !isASCIIHexDigit(character4))) {
        auto result = (m_code + 4) >= m_codeEnd ? ParsedUnicodeEscapeValue::Incomplete : ParsedUnicodeEscapeValue::Invalid;

        // For raw template literal syntax, we consume `NotEscapeSequence`.
        //
        // NotEscapeSequence ::
        //     u [lookahead not one of HexDigit][lookahead != {]
        //     u HexDigit [lookahead not one of HexDigit]
        //     u HexDigit HexDigit [lookahead not one of HexDigit]
        //     u HexDigit HexDigit HexDigit [lookahead not one of HexDigit]
        while (isASCIIHexDigit(m_current))
            shift();

        return result;
    }

    auto result = convertUnicode(m_current, character2, character3, character4);
    shift();
    shift();
    shift();
    shift();
    return result;
}

template <typename T>
void Lexer<T>::shiftLineTerminator()
{
    ASSERT(isLineTerminator(m_current));

    m_positionBeforeLastNewline = currentPosition();
    T prev = m_current;
    shift();

    // Allow both CRLF and LFCR.
    if (prev + m_current == '\n' + '\r')
        shift();

    ++m_lineNumber;
}

template <typename T>
ALWAYS_INLINE bool Lexer<T>::lastTokenWasRestrKeyword() const
{
    return m_lastToken == CONTINUE || m_lastToken == BREAK || m_lastToken == RETURN || m_lastToken == THROW;
}

template <typename T>
ALWAYS_INLINE void Lexer<T>::skipWhitespace()
{
    while (isWhiteSpace(m_current))
        shift();
}

static NEVER_INLINE bool isNonLatin1IdentStart(UChar c)
{
    return U_GET_GC_MASK(c) & U_GC_L_MASK;
}

static ALWAYS_INLINE bool isLatin1(LChar)
{
    return true;
}

static ALWAYS_INLINE bool isLatin1(UChar c)
{
    return c < 256;
}

static ALWAYS_INLINE bool isLatin1(UChar32 c)
{
    return !(c & ~0xFF);
}

static inline bool isIdentStart(LChar c)
{
    return typesOfLatin1Characters[c] == CharacterIdentifierStart;
}

static inline bool isIdentStart(UChar32 c)
{
    return isLatin1(c) ? isIdentStart(static_cast<LChar>(c)) : isNonLatin1IdentStart(c);
}

static NEVER_INLINE bool isNonLatin1IdentPart(UChar32 c)
{
    // FIXME: ES6 says this should be based on the Unicode property ID_Continue now instead.
    return (U_GET_GC_MASK(c) & (U_GC_L_MASK | U_GC_MN_MASK | U_GC_MC_MASK | U_GC_ND_MASK | U_GC_PC_MASK)) || c == 0x200C || c == 0x200D;
}

static ALWAYS_INLINE bool isIdentPart(LChar c)
{
    // Character types are divided into two groups depending on whether they can be part of an
    // identifier or not. Those whose type value is less or equal than CharacterNumber can be
    // part of an identifier. (See the CharacterType definition for more details.)
    return typesOfLatin1Characters[c] <= CharacterNumber;
}

static ALWAYS_INLINE bool isIdentPart(UChar32 c)
{
    return isLatin1(c) ? isIdentPart(static_cast<LChar>(c)) : isNonLatin1IdentPart(c);
}

static ALWAYS_INLINE bool isIdentPart(UChar c)
{
    return isIdentPart(static_cast<UChar32>(c));
}

template<typename CharacterType> ALWAYS_INLINE bool isIdentPartIncludingEscapeTemplate(const CharacterType* code, const CharacterType* codeEnd)
{
    if (isIdentPart(code[0]))
        return true;

    // Shortest sequence handled below is \u{0}, which is 5 characters.
    if (!(code[0] == '\\' && codeEnd - code >= 5 && code[1] == 'u'))
        return false;

    if (code[2] == '{') {
        UChar32 codePoint = 0;
        const CharacterType* pointer;
        for (pointer = &code[3]; pointer < codeEnd; ++pointer) {
            auto digit = *pointer;
            if (!isASCIIHexDigit(digit))
                break;
            codePoint = (codePoint << 4) | toASCIIHexValue(digit);
            if (codePoint > UCHAR_MAX_VALUE)
                return false;
        }
        return isIdentPart(codePoint) && pointer < codeEnd && *pointer == '}';
    }

    // Shortest sequence handled below is \uXXXX, which is 6 characters.
    if (codeEnd - code < 6)
        return false;

    auto character1 = code[2];
    auto character2 = code[3];
    auto character3 = code[4];
    auto character4 = code[5];
    return isASCIIHexDigit(character1) && isASCIIHexDigit(character2) && isASCIIHexDigit(character3) && isASCIIHexDigit(character4)
        && isIdentPart(Lexer<LChar>::convertUnicode(character1, character2, character3, character4));
}

static ALWAYS_INLINE bool isIdentPartIncludingEscape(const LChar* code, const LChar* codeEnd)
{
    return isIdentPartIncludingEscapeTemplate(code, codeEnd);
}

static ALWAYS_INLINE bool isIdentPartIncludingEscape(const UChar* code, const UChar* codeEnd)
{
    return isIdentPartIncludingEscapeTemplate(code, codeEnd);
}

static inline LChar singleEscape(int c)
{
    if (c < 128) {
        ASSERT(static_cast<size_t>(c) < ARRAY_SIZE(singleCharacterEscapeValuesForASCII));
        return singleCharacterEscapeValuesForASCII[c];
    }
    return 0;
}

template <typename T>
inline void Lexer<T>::record8(int c)
{
    ASSERT(c >= 0);
    ASSERT(c <= 0xFF);
    m_buffer8.append(static_cast<LChar>(c));
}

template <typename T>
inline void assertCharIsIn8BitRange(T c)
{
    UNUSED_PARAM(c);
    ASSERT(c >= 0);
    ASSERT(c <= 0xFF);
}

template <>
inline void assertCharIsIn8BitRange(UChar c)
{
    UNUSED_PARAM(c);
    ASSERT(c <= 0xFF);
}

template <>
inline void assertCharIsIn8BitRange(LChar)
{
}

template <typename T>
inline void Lexer<T>::append8(const T* p, size_t length)
{
    size_t currentSize = m_buffer8.size();
    m_buffer8.grow(currentSize + length);
    LChar* rawBuffer = m_buffer8.data() + currentSize;

    for (size_t i = 0; i < length; i++) {
        T c = p[i];
        assertCharIsIn8BitRange(c);
        rawBuffer[i] = c;
    }
}

template <typename T>
inline void Lexer<T>::append16(const LChar* p, size_t length)
{
    size_t currentSize = m_buffer16.size();
    m_buffer16.grow(currentSize + length);
    UChar* rawBuffer = m_buffer16.data() + currentSize;

    for (size_t i = 0; i < length; i++)
        rawBuffer[i] = p[i];
}

template <typename T>
inline void Lexer<T>::record16(T c)
{
    m_buffer16.append(c);
}

template <typename T>
inline void Lexer<T>::record16(int c)
{
    ASSERT(c >= 0);
    ASSERT(c <= static_cast<int>(USHRT_MAX));
    m_buffer16.append(static_cast<UChar>(c));
}
    
template<typename CharacterType> inline void Lexer<CharacterType>::recordUnicodeCodePoint(UChar32 codePoint)
{
    ASSERT(codePoint >= 0);
    ASSERT(codePoint <= UCHAR_MAX_VALUE);
    if (U_IS_BMP(codePoint))
        record16(codePoint);
    else {
        UChar codeUnits[2] = { U16_LEAD(codePoint), U16_TRAIL(codePoint) };
        append16(codeUnits, 2);
    }
}

#if !ASSERT_DISABLED
bool isSafeBuiltinIdentifier(VM& vm, const Identifier* ident)
{
    if (!ident)
        return true;
    /* Just block any use of suspicious identifiers.  This is intended to
     * be used as a safety net while implementing builtins.
     */
    // FIXME: How can a debug-only assertion be a safety net?
    if (*ident == vm.propertyNames->builtinNames().callPublicName())
        return false;
    if (*ident == vm.propertyNames->builtinNames().applyPublicName())
        return false;
    if (*ident == vm.propertyNames->eval)
        return false;
    if (*ident == vm.propertyNames->Function)
        return false;
    return true;
}
#endif
    
template <>
template <bool shouldCreateIdentifier> ALWAYS_INLINE JSTokenType Lexer<LChar>::parseIdentifier(JSTokenData* tokenData, unsigned lexerFlags, bool strictMode)
{
    const ptrdiff_t remaining = m_codeEnd - m_code;
    if ((remaining >= maxTokenLength) && !(lexerFlags & LexerFlagsIgnoreReservedWords)) {
        JSTokenType keyword = parseKeyword<shouldCreateIdentifier>(tokenData);
        if (keyword != IDENT) {
            ASSERT((!shouldCreateIdentifier) || tokenData->ident);
            return keyword == RESERVED_IF_STRICT && !strictMode ? IDENT : keyword;
        }
    }
    
    bool isPrivateName = m_current == '@' && m_parsingBuiltinFunction;
    if (isPrivateName)
        shift();
    
    const LChar* identifierStart = currentSourcePtr();
    unsigned identifierLineStart = currentLineStartOffset();
    
    while (isIdentPart(m_current))
        shift();
    
    if (UNLIKELY(m_current == '\\')) {
        setOffsetFromSourcePtr(identifierStart, identifierLineStart);
        return parseIdentifierSlowCase<shouldCreateIdentifier>(tokenData, lexerFlags, strictMode);
    }

    const Identifier* ident = 0;
    
    if (shouldCreateIdentifier || m_parsingBuiltinFunction) {
        int identifierLength = currentSourcePtr() - identifierStart;
        ident = makeIdentifier(identifierStart, identifierLength);
        if (m_parsingBuiltinFunction) {
            if (!isSafeBuiltinIdentifier(*m_vm, ident) && !isPrivateName) {
                m_lexErrorMessage = makeString("The use of '", ident->string(), "' is disallowed in builtin functions.");
                return ERRORTOK;
            }
            if (isPrivateName)
                ident = m_vm->propertyNames->lookUpPrivateName(*ident);
            else if (*ident == m_vm->propertyNames->undefinedKeyword)
                tokenData->ident = &m_vm->propertyNames->builtinNames().undefinedPrivateName();
            if (!ident)
                return INVALID_PRIVATE_NAME_ERRORTOK;
        }
        tokenData->ident = ident;
    } else
        tokenData->ident = 0;

    if (UNLIKELY((remaining < maxTokenLength) && !(lexerFlags & LexerFlagsIgnoreReservedWords)) && !isPrivateName) {
        ASSERT(shouldCreateIdentifier);
        if (remaining < maxTokenLength) {
            const HashTableValue* entry = JSC::mainTable.entry(*ident);
            ASSERT((remaining < maxTokenLength) || !entry);
            if (!entry)
                return IDENT;
            JSTokenType token = static_cast<JSTokenType>(entry->lexerValue());
            return (token != RESERVED_IF_STRICT) || strictMode ? token : IDENT;
        }
        return IDENT;
    }

    return IDENT;
}

template <>
template <bool shouldCreateIdentifier> ALWAYS_INLINE JSTokenType Lexer<UChar>::parseIdentifier(JSTokenData* tokenData, unsigned lexerFlags, bool strictMode)
{
    const ptrdiff_t remaining = m_codeEnd - m_code;
    if ((remaining >= maxTokenLength) && !(lexerFlags & LexerFlagsIgnoreReservedWords)) {
        JSTokenType keyword = parseKeyword<shouldCreateIdentifier>(tokenData);
        if (keyword != IDENT) {
            ASSERT((!shouldCreateIdentifier) || tokenData->ident);
            return keyword == RESERVED_IF_STRICT && !strictMode ? IDENT : keyword;
        }
    }
    
    bool isPrivateName = m_current == '@' && m_parsingBuiltinFunction;
    if (isPrivateName)
        shift();

    const UChar* identifierStart = currentSourcePtr();
    int identifierLineStart = currentLineStartOffset();

    UChar orAllChars = 0;
    
    while (isIdentPart(m_current)) {
        orAllChars |= m_current;
        shift();
    }
    
    if (UNLIKELY(m_current == '\\')) {
        ASSERT(!isPrivateName);
        setOffsetFromSourcePtr(identifierStart, identifierLineStart);
        return parseIdentifierSlowCase<shouldCreateIdentifier>(tokenData, lexerFlags, strictMode);
    }

    bool isAll8Bit = false;

    if (!(orAllChars & ~0xff))
        isAll8Bit = true;

    const Identifier* ident = 0;
    
    if (shouldCreateIdentifier || m_parsingBuiltinFunction) {
        int identifierLength = currentSourcePtr() - identifierStart;
        if (isAll8Bit)
            ident = makeIdentifierLCharFromUChar(identifierStart, identifierLength);
        else
            ident = makeIdentifier(identifierStart, identifierLength);
        if (m_parsingBuiltinFunction) {
            if (!isSafeBuiltinIdentifier(*m_vm, ident) && !isPrivateName) {
                m_lexErrorMessage = makeString("The use of '", ident->string(), "' is disallowed in builtin functions.");
                return ERRORTOK;
            }
            if (isPrivateName)
                ident = m_vm->propertyNames->lookUpPrivateName(*ident);
            else if (*ident == m_vm->propertyNames->undefinedKeyword)
                tokenData->ident = &m_vm->propertyNames->builtinNames().undefinedPrivateName();
            if (!ident)
                return INVALID_PRIVATE_NAME_ERRORTOK;
        }
        tokenData->ident = ident;
    } else
        tokenData->ident = 0;
    
    if (UNLIKELY((remaining < maxTokenLength) && !(lexerFlags & LexerFlagsIgnoreReservedWords)) && !isPrivateName) {
        ASSERT(shouldCreateIdentifier);
        if (remaining < maxTokenLength) {
            const HashTableValue* entry = JSC::mainTable.entry(*ident);
            ASSERT((remaining < maxTokenLength) || !entry);
            if (!entry)
                return IDENT;
            JSTokenType token = static_cast<JSTokenType>(entry->lexerValue());
            return (token != RESERVED_IF_STRICT) || strictMode ? token : IDENT;
        }
        return IDENT;
    }

    return IDENT;
}

template<typename CharacterType> template<bool shouldCreateIdentifier> JSTokenType Lexer<CharacterType>::parseIdentifierSlowCase(JSTokenData* tokenData, unsigned lexerFlags, bool strictMode)
{
    auto identifierStart = currentSourcePtr();
    bool bufferRequired = false;

    while (true) {
        if (LIKELY(isIdentPart(m_current))) {
            shift();
            continue;
        }
        if (LIKELY(m_current != '\\'))
            break;

        // \uXXXX unicode characters.
        bufferRequired = true;
        if (identifierStart != currentSourcePtr())
            m_buffer16.append(identifierStart, currentSourcePtr() - identifierStart);
        shift();
        if (UNLIKELY(m_current != 'u'))
            return atEnd() ? UNTERMINATED_IDENTIFIER_ESCAPE_ERRORTOK : INVALID_IDENTIFIER_ESCAPE_ERRORTOK;
        shift();
        auto character = parseUnicodeEscape();
        if (UNLIKELY(!character.isValid()))
            return character.isIncomplete() ? UNTERMINATED_IDENTIFIER_UNICODE_ESCAPE_ERRORTOK : INVALID_IDENTIFIER_UNICODE_ESCAPE_ERRORTOK;
        if (UNLIKELY(m_buffer16.size() ? !isIdentPart(character.value()) : !isIdentStart(character.value())))
            return INVALID_IDENTIFIER_UNICODE_ESCAPE_ERRORTOK;
        if (shouldCreateIdentifier)
            recordUnicodeCodePoint(character.value());
        identifierStart = currentSourcePtr();
    }

    int identifierLength;
    const Identifier* ident = nullptr;
    if (shouldCreateIdentifier) {
        if (!bufferRequired) {
            identifierLength = currentSourcePtr() - identifierStart;
            ident = makeIdentifier(identifierStart, identifierLength);
        } else {
            if (identifierStart != currentSourcePtr())
                m_buffer16.append(identifierStart, currentSourcePtr() - identifierStart);
            ident = makeIdentifier(m_buffer16.data(), m_buffer16.size());
        }

        tokenData->ident = ident;
    } else
        tokenData->ident = nullptr;

    m_buffer16.shrink(0);

    if (LIKELY(!(lexerFlags & LexerFlagsIgnoreReservedWords))) {
        ASSERT(shouldCreateIdentifier);
        const HashTableValue* entry = JSC::mainTable.entry(*ident);
        if (!entry)
            return IDENT;
        JSTokenType token = static_cast<JSTokenType>(entry->lexerValue());
        return (token != RESERVED_IF_STRICT) || strictMode ? token : IDENT;
    }

    return IDENT;
}

static ALWAYS_INLINE bool characterRequiresParseStringSlowCase(LChar character)
{
    return character < 0xE;
}

static ALWAYS_INLINE bool characterRequiresParseStringSlowCase(UChar character)
{
    return character < 0xE || character > 0xFF;
}

template <typename T>
template <bool shouldBuildStrings> ALWAYS_INLINE typename Lexer<T>::StringParseResult Lexer<T>::parseString(JSTokenData* tokenData, bool strictMode)
{
    int startingOffset = currentOffset();
    int startingLineStartOffset = currentLineStartOffset();
    int startingLineNumber = lineNumber();
    T stringQuoteCharacter = m_current;
    shift();

    const T* stringStart = currentSourcePtr();

    while (m_current != stringQuoteCharacter) {
        if (UNLIKELY(m_current == '\\')) {
            if (stringStart != currentSourcePtr() && shouldBuildStrings)
                append8(stringStart, currentSourcePtr() - stringStart);
            shift();

            LChar escape = singleEscape(m_current);

            // Most common escape sequences first.
            if (escape) {
                if (shouldBuildStrings)
                    record8(escape);
                shift();
            } else if (UNLIKELY(isLineTerminator(m_current)))
                shiftLineTerminator();
            else if (m_current == 'x') {
                shift();
                if (!isASCIIHexDigit(m_current) || !isASCIIHexDigit(peek(1))) {
                    m_lexErrorMessage = ASCIILiteral("\\x can only be followed by a hex character sequence");
                    return (atEnd() || (isASCIIHexDigit(m_current) && (m_code + 1 == m_codeEnd))) ? StringUnterminated : StringCannotBeParsed;
                }
                T prev = m_current;
                shift();
                if (shouldBuildStrings)
                    record8(convertHex(prev, m_current));
                shift();
            } else {
                setOffset(startingOffset, startingLineStartOffset);
                setLineNumber(startingLineNumber);
                m_buffer8.shrink(0);
                return parseStringSlowCase<shouldBuildStrings>(tokenData, strictMode);
            }
            stringStart = currentSourcePtr();
            continue;
        }

        if (UNLIKELY(characterRequiresParseStringSlowCase(m_current))) {
            setOffset(startingOffset, startingLineStartOffset);
            setLineNumber(startingLineNumber);
            m_buffer8.shrink(0);
            return parseStringSlowCase<shouldBuildStrings>(tokenData, strictMode);
        }

        shift();
    }

    if (currentSourcePtr() != stringStart && shouldBuildStrings)
        append8(stringStart, currentSourcePtr() - stringStart);
    if (shouldBuildStrings) {
        tokenData->ident = makeIdentifier(m_buffer8.data(), m_buffer8.size());
        m_buffer8.shrink(0);
    } else
        tokenData->ident = 0;

    return StringParsedSuccessfully;
}

template <typename T>
template <bool shouldBuildStrings, LexerEscapeParseMode escapeParseMode> ALWAYS_INLINE auto Lexer<T>::parseComplexEscape(bool strictMode, T stringQuoteCharacter) -> StringParseResult
{
    if (m_current == 'x') {
        shift();
        if (!isASCIIHexDigit(m_current) || !isASCIIHexDigit(peek(1))) {
            // For raw template literal syntax, we consume `NotEscapeSequence`.
            //
            // NotEscapeSequence ::
            //     x [lookahread not one of HexDigit]
            //     x HexDigit [lookahread not one of HexDigit]
            if (isASCIIHexDigit(m_current))
                shift();
            ASSERT(!isASCIIHexDigit(m_current));

            m_lexErrorMessage = ASCIILiteral("\\x can only be followed by a hex character sequence");
            return atEnd() ? StringUnterminated : StringCannotBeParsed;
        }

        T prev = m_current;
        shift();
        if (shouldBuildStrings)
            record16(convertHex(prev, m_current));
        shift();

        return StringParsedSuccessfully;
    }

    if (m_current == 'u') {
        shift();

        if (escapeParseMode == LexerEscapeParseMode::String && m_current == stringQuoteCharacter) {
            if (shouldBuildStrings)
                record16('u');
            return StringParsedSuccessfully;
        }

        auto character = parseUnicodeEscape();
        if (character.isValid()) {
            if (shouldBuildStrings)
                recordUnicodeCodePoint(character.value());
            return StringParsedSuccessfully;
        }

        m_lexErrorMessage = ASCIILiteral("\\u can only be followed by a Unicode character sequence");
        return atEnd() ? StringUnterminated : StringCannotBeParsed;
    }

    if (strictMode) {
        if (isASCIIDigit(m_current)) {
            // The only valid numeric escape in strict mode is '\0', and this must not be followed by a decimal digit.
            int character1 = m_current;
            shift();
            if (character1 != '0' || isASCIIDigit(m_current)) {
                // For raw template literal syntax, we consume `NotEscapeSequence`.
                //
                // NotEscapeSequence ::
                //     0 DecimalDigit
                //     DecimalDigit but not 0
                if (character1 == '0')
                    shift();

                m_lexErrorMessage = ASCIILiteral("The only valid numeric escape in strict mode is '\\0'");
                return atEnd() ? StringUnterminated : StringCannotBeParsed;
            }
            if (shouldBuildStrings)
                record16(0);
            return StringParsedSuccessfully;
        }
    } else {
        if (isASCIIOctalDigit(m_current)) {
            // Octal character sequences
            T character1 = m_current;
            shift();
            if (isASCIIOctalDigit(m_current)) {
                // Two octal characters
                T character2 = m_current;
                shift();
                if (character1 >= '0' && character1 <= '3' && isASCIIOctalDigit(m_current)) {
                    if (shouldBuildStrings)
                        record16((character1 - '0') * 64 + (character2 - '0') * 8 + m_current - '0');
                    shift();
                } else {
                    if (shouldBuildStrings)
                        record16((character1 - '0') * 8 + character2 - '0');
                }
            } else {
                if (shouldBuildStrings)
                    record16(character1 - '0');
            }
            return StringParsedSuccessfully;
        }
    }

    if (!atEnd()) {
        if (shouldBuildStrings)
            record16(m_current);
        shift();
        return StringParsedSuccessfully;
    }

    m_lexErrorMessage = ASCIILiteral("Unterminated string constant");
    return StringUnterminated;
}

template <typename T>
template <bool shouldBuildStrings> auto Lexer<T>::parseStringSlowCase(JSTokenData* tokenData, bool strictMode) -> StringParseResult
{
    T stringQuoteCharacter = m_current;
    shift();

    const T* stringStart = currentSourcePtr();

    while (m_current != stringQuoteCharacter) {
        if (UNLIKELY(m_current == '\\')) {
            if (stringStart != currentSourcePtr() && shouldBuildStrings)
                append16(stringStart, currentSourcePtr() - stringStart);
            shift();

            LChar escape = singleEscape(m_current);

            // Most common escape sequences first
            if (escape) {
                if (shouldBuildStrings)
                    record16(escape);
                shift();
            } else if (UNLIKELY(isLineTerminator(m_current)))
                shiftLineTerminator();
            else {
                StringParseResult result = parseComplexEscape<shouldBuildStrings, LexerEscapeParseMode::String>(strictMode, stringQuoteCharacter);
                if (result != StringParsedSuccessfully)
                    return result;
            }

            stringStart = currentSourcePtr();
            continue;
        }
        // Fast check for characters that require special handling.
        // Catches 0, \n, \r, 0x2028, and 0x2029 as efficiently
        // as possible, and lets through all common ASCII characters.
        if (UNLIKELY(((static_cast<unsigned>(m_current) - 0xE) & 0x2000))) {
            // New-line or end of input is not allowed
            if (atEnd() || isLineTerminator(m_current)) {
                m_lexErrorMessage = ASCIILiteral("Unexpected EOF");
                return atEnd() ? StringUnterminated : StringCannotBeParsed;
            }
            // Anything else is just a normal character
        }
        shift();
    }

    if (currentSourcePtr() != stringStart && shouldBuildStrings)
        append16(stringStart, currentSourcePtr() - stringStart);
    if (shouldBuildStrings)
        tokenData->ident = makeIdentifier(m_buffer16.data(), m_buffer16.size());
    else
        tokenData->ident = 0;

    m_buffer16.shrink(0);
    return StringParsedSuccessfully;
}

// While the lexer accepts <LF><CR> (not <CR><LF>) sequence
// as one line terminator and increments one line number,
// TemplateLiteral considers it as two line terminators <LF> and <CR>.
//
// TemplateLiteral normalizes line terminators as follows.
//
// <LF> => <LF>
// <CR> => <LF>
// <CR><LF> => <LF>
// <\u2028> => <\u2028>
// <\u2029> => <\u2029>
//
// So, <LF><CR> should be normalized to <LF><LF>.
// However, the lexer should increment the line number only once for <LF><CR>.
//
// To achieve this, LineNumberAdder holds the current status of line terminator sequence.
// When TemplateLiteral lexer encounters a line terminator, it notifies to LineNumberAdder.
// LineNumberAdder maintains the status and increments the line number when it's necessary.
// For example, LineNumberAdder increments the line number only once for <LF><CR> and <CR><LF>.
template<typename CharacterType>
class LineNumberAdder {
public:
    LineNumberAdder(int& lineNumber)
        : m_lineNumber(lineNumber)
    {
    }

    void clear()
    {
        m_previous = 0;
    }

    void add(CharacterType character)
    {
        ASSERT(Lexer<CharacterType>::isLineTerminator(character));
        if ((character + m_previous) == ('\n' + '\r'))
            m_previous = 0;
        else {
            ++m_lineNumber;
            m_previous = character;
        }
    }

private:
    int& m_lineNumber;
    CharacterType m_previous { 0 };
};

template <typename T>
typename Lexer<T>::StringParseResult Lexer<T>::parseTemplateLiteral(JSTokenData* tokenData, RawStringsBuildMode rawStringsBuildMode)
{
    bool parseCookedFailed = false;
    const T* stringStart = currentSourcePtr();
    const T* rawStringStart = currentSourcePtr();

    LineNumberAdder<T> lineNumberAdder(m_lineNumber);

    while (m_current != '`') {
        if (UNLIKELY(m_current == '\\')) {
            lineNumberAdder.clear();
            if (stringStart != currentSourcePtr())
                append16(stringStart, currentSourcePtr() - stringStart);
            shift();

            LChar escape = singleEscape(m_current);

            // Most common escape sequences first.
            if (escape) {
                record16(escape);
                shift();
            } else if (UNLIKELY(isLineTerminator(m_current))) {
                // Normalize <CR>, <CR><LF> to <LF>.
                if (m_current == '\r') {
                    ASSERT_WITH_MESSAGE(rawStringStart != currentSourcePtr(), "We should have at least shifted the escape.");

                    if (rawStringsBuildMode == RawStringsBuildMode::BuildRawStrings) {
                        m_bufferForRawTemplateString16.append(rawStringStart, currentSourcePtr() - rawStringStart);
                        m_bufferForRawTemplateString16.append('\n');
                    }

                    lineNumberAdder.add(m_current);
                    shift();
                    if (m_current == '\n') {
                        lineNumberAdder.add(m_current);
                        shift();
                    }

                    rawStringStart = currentSourcePtr();
                } else {
                    lineNumberAdder.add(m_current);
                    shift();
                }
            } else {
                bool strictMode = true;
                StringParseResult result = parseComplexEscape<true, LexerEscapeParseMode::Template>(strictMode, '`');
                if (result != StringParsedSuccessfully) {
                    if (rawStringsBuildMode == RawStringsBuildMode::BuildRawStrings && result == StringCannotBeParsed)
                        parseCookedFailed = true;
                    else
                        return result;
                }
            }

            stringStart = currentSourcePtr();
            continue;
        }

        if (m_current == '$' && peek(1) == '{')
            break;

        // Fast check for characters that require special handling.
        // Catches 0, \n, \r, 0x2028, and 0x2029 as efficiently
        // as possible, and lets through all common ASCII characters.
        if (UNLIKELY(((static_cast<unsigned>(m_current) - 0xE) & 0x2000))) {
            // End of input is not allowed.
            // Unlike String, line terminator is allowed.
            if (atEnd()) {
                m_lexErrorMessage = ASCIILiteral("Unexpected EOF");
                return atEnd() ? StringUnterminated : StringCannotBeParsed;
            }

            if (isLineTerminator(m_current)) {
                if (m_current == '\r') {
                    // Normalize <CR>, <CR><LF> to <LF>.
                    if (stringStart != currentSourcePtr())
                        append16(stringStart, currentSourcePtr() - stringStart);
                    if (rawStringStart != currentSourcePtr() && rawStringsBuildMode == RawStringsBuildMode::BuildRawStrings)
                        m_bufferForRawTemplateString16.append(rawStringStart, currentSourcePtr() - rawStringStart);

                    record16('\n');
                    if (rawStringsBuildMode == RawStringsBuildMode::BuildRawStrings)
                        m_bufferForRawTemplateString16.append('\n');
                    lineNumberAdder.add(m_current);
                    shift();
                    if (m_current == '\n') {
                        lineNumberAdder.add(m_current);
                        shift();
                    }
                    stringStart = currentSourcePtr();
                    rawStringStart = currentSourcePtr();
                } else {
                    lineNumberAdder.add(m_current);
                    shift();
                }
                continue;
            }
            // Anything else is just a normal character
        }

        lineNumberAdder.clear();
        shift();
    }

    bool isTail = m_current == '`';

    if (currentSourcePtr() != stringStart)
        append16(stringStart, currentSourcePtr() - stringStart);
    if (rawStringStart != currentSourcePtr() && rawStringsBuildMode == RawStringsBuildMode::BuildRawStrings)
        m_bufferForRawTemplateString16.append(rawStringStart, currentSourcePtr() - rawStringStart);

    if (!parseCookedFailed)
        tokenData->cooked = makeIdentifier(m_buffer16.data(), m_buffer16.size());
    else
        tokenData->cooked = nullptr;

    // Line terminator normalization (e.g. <CR> => <LF>) should be applied to both the raw and cooked representations.
    if (rawStringsBuildMode == RawStringsBuildMode::BuildRawStrings)
        tokenData->raw = makeIdentifier(m_bufferForRawTemplateString16.data(), m_bufferForRawTemplateString16.size());
    else
        tokenData->raw = nullptr;

    tokenData->isTail = isTail;

    m_buffer16.shrink(0);
    m_bufferForRawTemplateString16.shrink(0);

    if (isTail) {
        // Skip `
        shift();
    } else {
        // Skip $ and {
        shift();
        shift();
    }

    return StringParsedSuccessfully;
}

template <typename T>
ALWAYS_INLINE void Lexer<T>::parseHex(double& returnValue)
{
    // Optimization: most hexadecimal values fit into 4 bytes.
    uint32_t hexValue = 0;
    int maximumDigits = 7;

    do {
        hexValue = (hexValue << 4) + toASCIIHexValue(m_current);
        shift();
        --maximumDigits;
    } while (isASCIIHexDigit(m_current) && maximumDigits >= 0);

    if (maximumDigits >= 0) {
        returnValue = hexValue;
        return;
    }

    // No more place in the hexValue buffer.
    // The values are shifted out and placed into the m_buffer8 vector.
    for (int i = 0; i < 8; ++i) {
         int digit = hexValue >> 28;
         if (digit < 10)
             record8(digit + '0');
         else
             record8(digit - 10 + 'a');
         hexValue <<= 4;
    }

    while (isASCIIHexDigit(m_current)) {
        record8(m_current);
        shift();
    }

    returnValue = parseIntOverflow(m_buffer8.data(), m_buffer8.size(), 16);
}

template <typename T>
ALWAYS_INLINE bool Lexer<T>::parseBinary(double& returnValue)
{
    // Optimization: most binary values fit into 4 bytes.
    uint32_t binaryValue = 0;
    const unsigned maximumDigits = 32;
    int digit = maximumDigits - 1;
    // Temporary buffer for the digits. Makes easier
    // to reconstruct the input characters when needed.
    LChar digits[maximumDigits];

    do {
        binaryValue = (binaryValue << 1) + (m_current - '0');
        digits[digit] = m_current;
        shift();
        --digit;
    } while (isASCIIBinaryDigit(m_current) && digit >= 0);

    if (!isASCIIDigit(m_current) && digit >= 0) {
        returnValue = binaryValue;
        return true;
    }

    for (int i = maximumDigits - 1; i > digit; --i)
        record8(digits[i]);

    while (isASCIIBinaryDigit(m_current)) {
        record8(m_current);
        shift();
    }

    if (isASCIIDigit(m_current)) {
        returnValue = 0;
        return false;
    }

    returnValue = parseIntOverflow(m_buffer8.data(), m_buffer8.size(), 2);
    return true;
}

template <typename T>
ALWAYS_INLINE bool Lexer<T>::parseOctal(double& returnValue)
{
    // Optimization: most octal values fit into 4 bytes.
    uint32_t octalValue = 0;
    const unsigned maximumDigits = 10;
    int digit = maximumDigits - 1;
    // Temporary buffer for the digits. Makes easier
    // to reconstruct the input characters when needed.
    LChar digits[maximumDigits];

    do {
        octalValue = octalValue * 8 + (m_current - '0');
        digits[digit] = m_current;
        shift();
        --digit;
    } while (isASCIIOctalDigit(m_current) && digit >= 0);

    if (!isASCIIDigit(m_current) && digit >= 0) {
        returnValue = octalValue;
        return true;
    }

    for (int i = maximumDigits - 1; i > digit; --i)
         record8(digits[i]);

    while (isASCIIOctalDigit(m_current)) {
        record8(m_current);
        shift();
    }

    if (isASCIIDigit(m_current)) {
        returnValue = 0;
        return false;
    }

    returnValue = parseIntOverflow(m_buffer8.data(), m_buffer8.size(), 8);
    return true;
}

template <typename T>
ALWAYS_INLINE bool Lexer<T>::parseDecimal(double& returnValue)
{
    // Optimization: most decimal values fit into 4 bytes.
    uint32_t decimalValue = 0;

    // Since parseOctal may be executed before parseDecimal,
    // the m_buffer8 may hold ascii digits.
    if (!m_buffer8.size()) {
        const unsigned maximumDigits = 10;
        int digit = maximumDigits - 1;
        // Temporary buffer for the digits. Makes easier
        // to reconstruct the input characters when needed.
        LChar digits[maximumDigits];

        do {
            decimalValue = decimalValue * 10 + (m_current - '0');
            digits[digit] = m_current;
            shift();
            --digit;
        } while (isASCIIDigit(m_current) && digit >= 0);

        if (digit >= 0 && m_current != '.' && (m_current | 0x20) != 'e') {
            returnValue = decimalValue;
            return true;
        }

        for (int i = maximumDigits - 1; i > digit; --i)
            record8(digits[i]);
    }

    while (isASCIIDigit(m_current)) {
        record8(m_current);
        shift();
    }

    return false;
}

template <typename T>
ALWAYS_INLINE void Lexer<T>::parseNumberAfterDecimalPoint()
{
    record8('.');
    while (isASCIIDigit(m_current)) {
        record8(m_current);
        shift();
    }
}

template <typename T>
ALWAYS_INLINE bool Lexer<T>::parseNumberAfterExponentIndicator()
{
    record8('e');
    shift();
    if (m_current == '+' || m_current == '-') {
        record8(m_current);
        shift();
    }

    if (!isASCIIDigit(m_current))
        return false;

    do {
        record8(m_current);
        shift();
    } while (isASCIIDigit(m_current));
    return true;
}

template <typename T>
ALWAYS_INLINE bool Lexer<T>::parseMultilineComment()
{
    while (true) {
        while (UNLIKELY(m_current == '*')) {
            shift();
            if (m_current == '/') {
                shift();
                return true;
            }
        }

        if (atEnd())
            return false;

        if (isLineTerminator(m_current)) {
            shiftLineTerminator();
            m_terminator = true;
        } else
            shift();
    }
}

template <typename T>
ALWAYS_INLINE void Lexer<T>::parseCommentDirective()
{
    // sourceURL and sourceMappingURL directives.
    if (!consume("source"))
        return;

    if (consume("URL=")) {
        m_sourceURLDirective = parseCommentDirectiveValue();
        return;
    }

    if (consume("MappingURL=")) {
        m_sourceMappingURLDirective = parseCommentDirectiveValue();
        return;
    }
}

template <typename T>
ALWAYS_INLINE String Lexer<T>::parseCommentDirectiveValue()
{
    skipWhitespace();
    const T* stringStart = currentSourcePtr();
    while (!isWhiteSpace(m_current) && !isLineTerminator(m_current) && m_current != '"' && m_current != '\'' && !atEnd())
        shift();
    const T* stringEnd = currentSourcePtr();
    skipWhitespace();

    if (!isLineTerminator(m_current) && !atEnd())
        return String();

    append8(stringStart, stringEnd - stringStart);
    String result = String(m_buffer8.data(), m_buffer8.size());
    m_buffer8.shrink(0);
    return result;
}

template <typename T>
template <unsigned length>
ALWAYS_INLINE bool Lexer<T>::consume(const char (&input)[length])
{
    unsigned lengthToCheck = length - 1; // Ignore the ending NULL byte in the string literal.

    unsigned i = 0;
    for (; i < lengthToCheck && m_current == input[i]; i++)
        shift();

    return i == lengthToCheck;
}

template <typename T>
bool Lexer<T>::nextTokenIsColon()
{
    const T* code = m_code;
    while (code < m_codeEnd && (isWhiteSpace(*code) || isLineTerminator(*code)))
        code++;
    
    return code < m_codeEnd && *code == ':';
}

template <typename T>
void Lexer<T>::fillTokenInfo(JSToken* tokenRecord, JSTokenType token, int lineNumber, int endOffset, int lineStartOffset, JSTextPosition endPosition)
{
    JSTokenLocation* tokenLocation = &tokenRecord->m_location;
    tokenLocation->line = lineNumber;
    tokenLocation->endOffset = endOffset;
    tokenLocation->lineStartOffset = lineStartOffset;
    ASSERT(tokenLocation->endOffset >= tokenLocation->lineStartOffset);
    tokenRecord->m_endPosition = endPosition;
    m_lastToken = token;
}

template <typename T>
JSTokenType Lexer<T>::lex(JSToken* tokenRecord, unsigned lexerFlags, bool strictMode)
{
    JSTokenData* tokenData = &tokenRecord->m_data;
    JSTokenLocation* tokenLocation = &tokenRecord->m_location;
    m_lastTokenLocation = JSTokenLocation(tokenRecord->m_location);
    
    ASSERT(!m_error);
    ASSERT(m_buffer8.isEmpty());
    ASSERT(m_buffer16.isEmpty());

    JSTokenType token = ERRORTOK;
    m_terminator = false;

start:
    skipWhitespace();

    if (atEnd())
        return EOFTOK;
    
    tokenLocation->startOffset = currentOffset();
    ASSERT(currentOffset() >= currentLineStartOffset());
    tokenRecord->m_startPosition = currentPosition();

    CharacterType type;
    if (LIKELY(isLatin1(m_current)))
        type = static_cast<CharacterType>(typesOfLatin1Characters[m_current]);
    else if (isNonLatin1IdentStart(m_current))
        type = CharacterIdentifierStart;
    else if (isLineTerminator(m_current))
        type = CharacterLineTerminator;
    else
        type = CharacterInvalid;

    switch (type) {
    case CharacterGreater:
        shift();
        if (m_current == '>') {
            shift();
            if (m_current == '>') {
                shift();
                if (m_current == '=') {
                    shift();
                    token = URSHIFTEQUAL;
                    break;
                }
                token = URSHIFT;
                break;
            }
            if (m_current == '=') {
                shift();
                token = RSHIFTEQUAL;
                break;
            }
            token = RSHIFT;
            break;
        }
        if (m_current == '=') {
            shift();
            token = GE;
            break;
        }
        token = GT;
        break;
    case CharacterEqual: {
        if (peek(1) == '>') {
            token = ARROWFUNCTION;
            tokenData->line = lineNumber();
            tokenData->offset = currentOffset();
            tokenData->lineStartOffset = currentLineStartOffset();
            ASSERT(tokenData->offset >= tokenData->lineStartOffset);
            shift();
            shift();
            break;
        }

        shift();
        if (m_current == '=') {
            shift();
            if (m_current == '=') {
                shift();
                token = STREQ;
                break;
            }
            token = EQEQ;
            break;
        }
        token = EQUAL;
        break;
    }
    case CharacterLess:
        shift();
        if (m_current == '!' && peek(1) == '-' && peek(2) == '-') {
            if (m_scriptMode == JSParserScriptMode::Classic) {
                // <!-- marks the beginning of a line comment (for www usage)
                goto inSingleLineComment;
            }
        }
        if (m_current == '<') {
            shift();
            if (m_current == '=') {
                shift();
                token = LSHIFTEQUAL;
                break;
            }
            token = LSHIFT;
            break;
        }
        if (m_current == '=') {
            shift();
            token = LE;
            break;
        }
        token = LT;
        break;
    case CharacterExclamationMark:
        shift();
        if (m_current == '=') {
            shift();
            if (m_current == '=') {
                shift();
                token = STRNEQ;
                break;
            }
            token = NE;
            break;
        }
        token = EXCLAMATION;
        break;
    case CharacterAdd:
        shift();
        if (m_current == '+') {
            shift();
            token = (!m_terminator) ? PLUSPLUS : AUTOPLUSPLUS;
            break;
        }
        if (m_current == '=') {
            shift();
            token = PLUSEQUAL;
            break;
        }
        token = PLUS;
        break;
    case CharacterSub:
        shift();
        if (m_current == '-') {
            shift();
            if (m_atLineStart && m_current == '>') {
                if (m_scriptMode == JSParserScriptMode::Classic) {
                    shift();
                    goto inSingleLineComment;
                }
            }
            token = (!m_terminator) ? MINUSMINUS : AUTOMINUSMINUS;
            break;
        }
        if (m_current == '=') {
            shift();
            token = MINUSEQUAL;
            break;
        }
        token = MINUS;
        break;
    case CharacterMultiply:
        shift();
        if (m_current == '=') {
            shift();
            token = MULTEQUAL;
            break;
        }
        if (m_current == '*') {
            shift();
            if (m_current == '=') {
                shift();
                token = POWEQUAL;
                break;
            }
            token = POW;
            break;
        }
        token = TIMES;
        break;
    case CharacterSlash:
        shift();
        if (m_current == '/') {
            shift();
            goto inSingleLineCommentCheckForDirectives;
        }
        if (m_current == '*') {
            shift();
            if (parseMultilineComment())
                goto start;
            m_lexErrorMessage = ASCIILiteral("Multiline comment was not closed properly");
            token = UNTERMINATED_MULTILINE_COMMENT_ERRORTOK;
            goto returnError;
        }
        if (m_current == '=') {
            shift();
            token = DIVEQUAL;
            break;
        }
        token = DIVIDE;
        break;
    case CharacterAnd:
        shift();
        if (m_current == '&') {
            shift();
            token = AND;
            break;
        }
        if (m_current == '=') {
            shift();
            token = ANDEQUAL;
            break;
        }
        token = BITAND;
        break;
    case CharacterXor:
        shift();
        if (m_current == '=') {
            shift();
            token = XOREQUAL;
            break;
        }
        token = BITXOR;
        break;
    case CharacterModulo:
        shift();
        if (m_current == '=') {
            shift();
            token = MODEQUAL;
            break;
        }
        token = MOD;
        break;
    case CharacterOr:
        shift();
        if (m_current == '=') {
            shift();
            token = OREQUAL;
            break;
        }
        if (m_current == '|') {
            shift();
            token = OR;
            break;
        }
        token = BITOR;
        break;
    case CharacterOpenParen:
        token = OPENPAREN;
        tokenData->line = lineNumber();
        tokenData->offset = currentOffset();
        tokenData->lineStartOffset = currentLineStartOffset();
        shift();
        break;
    case CharacterCloseParen:
        token = CLOSEPAREN;
        shift();
        break;
    case CharacterOpenBracket:
        token = OPENBRACKET;
        shift();
        break;
    case CharacterCloseBracket:
        token = CLOSEBRACKET;
        shift();
        break;
    case CharacterComma:
        token = COMMA;
        shift();
        break;
    case CharacterColon:
        token = COLON;
        shift();
        break;
    case CharacterQuestion:
        token = QUESTION;
        shift();
        break;
    case CharacterTilde:
        token = TILDE;
        shift();
        break;
    case CharacterSemicolon:
        shift();
        token = SEMICOLON;
        break;
    case CharacterBackQuote:
        shift();
        token = BACKQUOTE;
        break;
    case CharacterOpenBrace:
        tokenData->line = lineNumber();
        tokenData->offset = currentOffset();
        tokenData->lineStartOffset = currentLineStartOffset();
        ASSERT(tokenData->offset >= tokenData->lineStartOffset);
        shift();
        token = OPENBRACE;
        break;
    case CharacterCloseBrace:
        tokenData->line = lineNumber();
        tokenData->offset = currentOffset();
        tokenData->lineStartOffset = currentLineStartOffset();
        ASSERT(tokenData->offset >= tokenData->lineStartOffset);
        shift();
        token = CLOSEBRACE;
        break;
    case CharacterDot:
        shift();
        if (!isASCIIDigit(m_current)) {
            if (UNLIKELY((m_current == '.') && (peek(1) == '.'))) {
                shift();
                shift();
                token = DOTDOTDOT;
                break;
            }
            token = DOT;
            break;
        }
        goto inNumberAfterDecimalPoint;
    case CharacterZero:
        shift();
        if ((m_current | 0x20) == 'x') {
            if (!isASCIIHexDigit(peek(1))) {
                m_lexErrorMessage = ASCIILiteral("No hexadecimal digits after '0x'");
                token = UNTERMINATED_HEX_NUMBER_ERRORTOK;
                goto returnError;
            }

            // Shift out the 'x' prefix.
            shift();

            parseHex(tokenData->doubleValue);
            if (isIdentStart(m_current)) {
                m_lexErrorMessage = ASCIILiteral("No space between hexadecimal literal and identifier");
                token = UNTERMINATED_HEX_NUMBER_ERRORTOK;
                goto returnError;
            }
            token = tokenTypeForIntegerLikeToken(tokenData->doubleValue);
            m_buffer8.shrink(0);
            break;
        }
        if ((m_current | 0x20) == 'b') {
            if (!isASCIIBinaryDigit(peek(1))) {
                m_lexErrorMessage = ASCIILiteral("No binary digits after '0b'");
                token = UNTERMINATED_BINARY_NUMBER_ERRORTOK;
                goto returnError;
            }

            // Shift out the 'b' prefix.
            shift();

            parseBinary(tokenData->doubleValue);
            if (isIdentStart(m_current)) {
                m_lexErrorMessage = ASCIILiteral("No space between binary literal and identifier");
                token = UNTERMINATED_BINARY_NUMBER_ERRORTOK;
                goto returnError;
            }
            token = tokenTypeForIntegerLikeToken(tokenData->doubleValue);
            m_buffer8.shrink(0);
            break;
        }

        if ((m_current | 0x20) == 'o') {
            if (!isASCIIOctalDigit(peek(1))) {
                m_lexErrorMessage = ASCIILiteral("No octal digits after '0o'");
                token = UNTERMINATED_OCTAL_NUMBER_ERRORTOK;
                goto returnError;
            }

            // Shift out the 'o' prefix.
            shift();

            parseOctal(tokenData->doubleValue);
            if (isIdentStart(m_current)) {
                m_lexErrorMessage = ASCIILiteral("No space between octal literal and identifier");
                token = UNTERMINATED_OCTAL_NUMBER_ERRORTOK;
                goto returnError;
            }
            token = tokenTypeForIntegerLikeToken(tokenData->doubleValue);
            m_buffer8.shrink(0);
            break;
        }

        record8('0');
        if (strictMode && isASCIIDigit(m_current)) {
            m_lexErrorMessage = ASCIILiteral("Decimal integer literals with a leading zero are forbidden in strict mode");
            token = UNTERMINATED_OCTAL_NUMBER_ERRORTOK;
            goto returnError;
        }
        if (isASCIIOctalDigit(m_current)) {
            if (parseOctal(tokenData->doubleValue)) {
                token = tokenTypeForIntegerLikeToken(tokenData->doubleValue);
            }
        }
        FALLTHROUGH;
    case CharacterNumber:
        if (LIKELY(token != INTEGER && token != DOUBLE)) {
            if (!parseDecimal(tokenData->doubleValue)) {
                token = INTEGER;
                if (m_current == '.') {
                    shift();
inNumberAfterDecimalPoint:
                    parseNumberAfterDecimalPoint();
                    token = DOUBLE;
                }
                if ((m_current | 0x20) == 'e') {
                    if (!parseNumberAfterExponentIndicator()) {
                        m_lexErrorMessage = ASCIILiteral("Non-number found after exponent indicator");
                        token = atEnd() ? UNTERMINATED_NUMERIC_LITERAL_ERRORTOK : INVALID_NUMERIC_LITERAL_ERRORTOK;
                        goto returnError;
                    }
                }
                size_t parsedLength;
                tokenData->doubleValue = parseDouble(m_buffer8.data(), m_buffer8.size(), parsedLength);
                if (token == INTEGER)
                    token = tokenTypeForIntegerLikeToken(tokenData->doubleValue);
            } else
                token = tokenTypeForIntegerLikeToken(tokenData->doubleValue);
        }

        if (UNLIKELY(isIdentStart(m_current))) {
            m_lexErrorMessage = ASCIILiteral("No identifiers allowed directly after numeric literal");
            token = atEnd() ? UNTERMINATED_NUMERIC_LITERAL_ERRORTOK : INVALID_NUMERIC_LITERAL_ERRORTOK;
            goto returnError;
        }
        m_buffer8.shrink(0);
        break;
    case CharacterQuote: {
        StringParseResult result = StringCannotBeParsed;
        if (lexerFlags & LexerFlagsDontBuildStrings)
            result = parseString<false>(tokenData, strictMode);
        else
            result = parseString<true>(tokenData, strictMode);

        if (UNLIKELY(result != StringParsedSuccessfully)) {
            token = result == StringUnterminated ? UNTERMINATED_STRING_LITERAL_ERRORTOK : INVALID_STRING_LITERAL_ERRORTOK;
            goto returnError;
        }
        shift();
        token = STRING;
        break;
        }
    case CharacterIdentifierStart:
        ASSERT(isIdentStart(m_current));
        FALLTHROUGH;
    case CharacterBackSlash:
        parseIdent:
        if (lexerFlags & LexexFlagsDontBuildKeywords)
            token = parseIdentifier<false>(tokenData, lexerFlags, strictMode);
        else
            token = parseIdentifier<true>(tokenData, lexerFlags, strictMode);
        break;
    case CharacterLineTerminator:
        ASSERT(isLineTerminator(m_current));
        shiftLineTerminator();
        m_atLineStart = true;
        m_terminator = true;
        m_lineStart = m_code;
        goto start;
    case CharacterPrivateIdentifierStart:
        if (m_parsingBuiltinFunction)
            goto parseIdent;

        FALLTHROUGH;
    case CharacterInvalid:
        m_lexErrorMessage = invalidCharacterMessage();
        token = ERRORTOK;
        goto returnError;
    default:
        RELEASE_ASSERT_NOT_REACHED();
        m_lexErrorMessage = ASCIILiteral("Internal Error");
        token = ERRORTOK;
        goto returnError;
    }

    m_atLineStart = false;
    goto returnToken;

inSingleLineCommentCheckForDirectives:
    // Script comment directives like "//# sourceURL=test.js".
    if (UNLIKELY((m_current == '#' || m_current == '@') && isWhiteSpace(peek(1)))) {
        shift();
        shift();
        parseCommentDirective();
    }
    // Fall through to complete single line comment parsing.

inSingleLineComment:
    {
        auto lineNumber = m_lineNumber;
        auto endOffset = currentOffset();
        auto lineStartOffset = currentLineStartOffset();
        auto endPosition = currentPosition();

        while (!isLineTerminator(m_current)) {
            if (atEnd())
                return EOFTOK;
            shift();
        }
        shiftLineTerminator();
        m_atLineStart = true;
        m_terminator = true;
        m_lineStart = m_code;
        if (!lastTokenWasRestrKeyword())
            goto start;

        token = SEMICOLON;
        fillTokenInfo(tokenRecord, token, lineNumber, endOffset, lineStartOffset, endPosition);
        return token;
    }

returnToken:
    fillTokenInfo(tokenRecord, token, m_lineNumber, currentOffset(), currentLineStartOffset(), currentPosition());
    return token;

returnError:
    m_error = true;
    fillTokenInfo(tokenRecord, token, m_lineNumber, currentOffset(), currentLineStartOffset(), currentPosition());
    RELEASE_ASSERT(token & ErrorTokenFlag);
    return token;
}

template <typename T>
static inline void orCharacter(UChar&, UChar);

template <>
inline void orCharacter<LChar>(UChar&, UChar) { }

template <>
inline void orCharacter<UChar>(UChar& orAccumulator, UChar character)
{
    orAccumulator |= character;
}

template <typename T>
JSTokenType Lexer<T>::scanRegExp(JSToken* tokenRecord, UChar patternPrefix)
{
    JSTokenData* tokenData = &tokenRecord->m_data;
    ASSERT(m_buffer16.isEmpty());

    bool lastWasEscape = false;
    bool inBrackets = false;
    UChar charactersOredTogether = 0;

    if (patternPrefix) {
        ASSERT(!isLineTerminator(patternPrefix));
        ASSERT(patternPrefix != '/');
        ASSERT(patternPrefix != '[');
        record16(patternPrefix);
    }

    while (true) {
        if (isLineTerminator(m_current) || atEnd()) {
            m_buffer16.shrink(0);
            JSTokenType token = UNTERMINATED_REGEXP_LITERAL_ERRORTOK;
            fillTokenInfo(tokenRecord, token, m_lineNumber, currentOffset(), currentLineStartOffset(), currentPosition());
            m_error = true;
            m_lexErrorMessage = makeString("Unterminated regular expression literal '", getToken(*tokenRecord), "'");
            return token;
        }

        T prev = m_current;
        
        shift();

        if (prev == '/' && !lastWasEscape && !inBrackets)
            break;

        record16(prev);
        orCharacter<T>(charactersOredTogether, prev);

        if (lastWasEscape) {
            lastWasEscape = false;
            continue;
        }

        switch (prev) {
        case '[':
            inBrackets = true;
            break;
        case ']':
            inBrackets = false;
            break;
        case '\\':
            lastWasEscape = true;
            break;
        }
    }

    tokenData->pattern = makeRightSizedIdentifier(m_buffer16.data(), m_buffer16.size(), charactersOredTogether);

    m_buffer16.shrink(0);
    charactersOredTogether = 0;

    while (isIdentPart(m_current)) {
        record16(m_current);
        orCharacter<T>(charactersOredTogether, m_current);
        shift();
    }

    tokenData->flags = makeRightSizedIdentifier(m_buffer16.data(), m_buffer16.size(), charactersOredTogether);
    m_buffer16.shrink(0);

    // Since RegExp always ends with /, m_atLineStart always becomes false.
    m_atLineStart = false;

    JSTokenType token = REGEXP;
    fillTokenInfo(tokenRecord, token, m_lineNumber, currentOffset(), currentLineStartOffset(), currentPosition());
    return token;
}

template <typename T>
JSTokenType Lexer<T>::scanTemplateString(JSToken* tokenRecord, RawStringsBuildMode rawStringsBuildMode)
{
    JSTokenData* tokenData = &tokenRecord->m_data;
    ASSERT(!m_error);
    ASSERT(m_buffer16.isEmpty());

    // Leading backquote ` (for template head) or closing brace } (for template trailing) are already shifted in the previous token scan.
    // So in this re-scan phase, shift() is not needed here.
    StringParseResult result = parseTemplateLiteral(tokenData, rawStringsBuildMode);
    JSTokenType token = ERRORTOK;
    if (UNLIKELY(result != StringParsedSuccessfully)) {
        token = result == StringUnterminated ? UNTERMINATED_TEMPLATE_LITERAL_ERRORTOK : INVALID_TEMPLATE_LITERAL_ERRORTOK;
        m_error = true;
    } else
        token = TEMPLATE;

    // Since TemplateString always ends with ` or }, m_atLineStart always becomes false.
    m_atLineStart = false;
    fillTokenInfo(tokenRecord, token, m_lineNumber, currentOffset(), currentLineStartOffset(), currentPosition());
    return token;
}

template <typename T>
void Lexer<T>::clear()
{
    m_arena = 0;

    Vector<LChar> newBuffer8;
    m_buffer8.swap(newBuffer8);

    Vector<UChar> newBuffer16;
    m_buffer16.swap(newBuffer16);

    Vector<UChar> newBufferForRawTemplateString16;
    m_bufferForRawTemplateString16.swap(newBufferForRawTemplateString16);

    m_isReparsingFunction = false;
}

// Instantiate the two flavors of Lexer we need instead of putting most of this file in Lexer.h
template class Lexer<LChar>;
template class Lexer<UChar>;

} // namespace JSC
