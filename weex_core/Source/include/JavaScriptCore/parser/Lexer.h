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

#include "Lookup.h"
#include "ParserArena.h"
#include "ParserTokens.h"
#include "SourceCode.h"
#include <wtf/ASCIICType.h>
#include <wtf/Vector.h>

namespace JSC {

enum LexerFlags {
    LexerFlagsIgnoreReservedWords = 1, 
    LexerFlagsDontBuildStrings = 2,
    LexexFlagsDontBuildKeywords = 4
};

enum class LexerEscapeParseMode { Template, String };

struct ParsedUnicodeEscapeValue;

bool isLexerKeyword(const Identifier&);

template <typename T>
class Lexer {
    WTF_MAKE_NONCOPYABLE(Lexer);
    WTF_MAKE_FAST_ALLOCATED;

public:
    Lexer(VM*, JSParserBuiltinMode, JSParserScriptMode);
    ~Lexer();

    // Character manipulation functions.
    static bool isWhiteSpace(T character);
    static bool isLineTerminator(T character);
    static unsigned char convertHex(int c1, int c2);
    static UChar convertUnicode(int c1, int c2, int c3, int c4);

    // Functions to set up parsing.
    void setCode(const SourceCode&, ParserArena*);
    void setIsReparsingFunction() { m_isReparsingFunction = true; }
    bool isReparsingFunction() const { return m_isReparsingFunction; }

    JSTokenType lex(JSToken*, unsigned, bool strictMode);
    bool nextTokenIsColon();
    int lineNumber() const { return m_lineNumber; }
    ALWAYS_INLINE int currentOffset() const { return offsetFromSourcePtr(m_code); }
    ALWAYS_INLINE int currentLineStartOffset() const { return offsetFromSourcePtr(m_lineStart); }
    ALWAYS_INLINE JSTextPosition currentPosition() const
    {
        return JSTextPosition(m_lineNumber, currentOffset(), currentLineStartOffset());
    }
    JSTextPosition positionBeforeLastNewline() const { return m_positionBeforeLastNewline; }
    JSTokenLocation lastTokenLocation() const { return m_lastTokenLocation; }
    void setLastLineNumber(int lastLineNumber) { m_lastLineNumber = lastLineNumber; }
    int lastLineNumber() const { return m_lastLineNumber; }
    bool prevTerminator() const { return m_terminator; }
    JSTokenType scanRegExp(JSToken*, UChar patternPrefix = 0);
    enum class RawStringsBuildMode { BuildRawStrings, DontBuildRawStrings };
    JSTokenType scanTemplateString(JSToken*, RawStringsBuildMode);

    // Functions for use after parsing.
    bool sawError() const { return m_error; }
    void setSawError(bool sawError) { m_error = sawError; }
    String getErrorMessage() const { return m_lexErrorMessage; }
    void setErrorMessage(const String& errorMessage) { m_lexErrorMessage = errorMessage; }
    String sourceURL() const { return m_sourceURLDirective; }
    String sourceMappingURL() const { return m_sourceMappingURLDirective; }
    void clear();
    void setOffset(int offset, int lineStartOffset)
    {
        m_error = 0;
        m_lexErrorMessage = String();

        m_code = sourcePtrFromOffset(offset);
        m_lineStart = sourcePtrFromOffset(lineStartOffset);
        ASSERT(currentOffset() >= currentLineStartOffset());

        m_buffer8.resize(0);
        m_buffer16.resize(0);
        if (LIKELY(m_code < m_codeEnd))
            m_current = *m_code;
        else
            m_current = 0;
    }
    void setLineNumber(int line)
    {
        m_lineNumber = line;
    }
    void setTerminator(bool terminator)
    {
        m_terminator = terminator;
    }

    JSTokenType lexExpectIdentifier(JSToken*, unsigned, bool strictMode);

    ALWAYS_INLINE StringView getToken(const JSToken& token)
    {
        SourceProvider* sourceProvider = m_source->provider();
        ASSERT_WITH_MESSAGE(token.m_location.startOffset <= token.m_location.endOffset, "Calling this function with the baked token.");
        return sourceProvider->getRange(token.m_location.startOffset, token.m_location.endOffset);
    }

private:
    void record8(int);
    void append8(const T*, size_t);
    void record16(int);
    void record16(T);
    void recordUnicodeCodePoint(UChar32);
    void append16(const LChar*, size_t);
    void append16(const UChar* characters, size_t length) { m_buffer16.append(characters, length); }

    ALWAYS_INLINE void shift();
    ALWAYS_INLINE bool atEnd() const;
    ALWAYS_INLINE T peek(int offset) const;

    ParsedUnicodeEscapeValue parseUnicodeEscape();
    void shiftLineTerminator();

    ALWAYS_INLINE int offsetFromSourcePtr(const T* ptr) const { return ptr - m_codeStart; }
    ALWAYS_INLINE const T* sourcePtrFromOffset(int offset) const { return m_codeStart + offset; }

    String invalidCharacterMessage() const;
    ALWAYS_INLINE const T* currentSourcePtr() const;
    ALWAYS_INLINE void setOffsetFromSourcePtr(const T* sourcePtr, unsigned lineStartOffset) { setOffset(offsetFromSourcePtr(sourcePtr), lineStartOffset); }

    ALWAYS_INLINE void setCodeStart(const StringView&);

    ALWAYS_INLINE const Identifier* makeIdentifier(const LChar* characters, size_t length);
    ALWAYS_INLINE const Identifier* makeIdentifier(const UChar* characters, size_t length);
    ALWAYS_INLINE const Identifier* makeLCharIdentifier(const LChar* characters, size_t length);
    ALWAYS_INLINE const Identifier* makeLCharIdentifier(const UChar* characters, size_t length);
    ALWAYS_INLINE const Identifier* makeRightSizedIdentifier(const UChar* characters, size_t length, UChar orAllChars);
    ALWAYS_INLINE const Identifier* makeIdentifierLCharFromUChar(const UChar* characters, size_t length);
    ALWAYS_INLINE const Identifier* makeEmptyIdentifier();

    ALWAYS_INLINE bool lastTokenWasRestrKeyword() const;
    
    ALWAYS_INLINE void skipWhitespace();

    template <int shiftAmount> void internalShift();
    template <bool shouldCreateIdentifier> ALWAYS_INLINE JSTokenType parseKeyword(JSTokenData*);
    template <bool shouldBuildIdentifiers> ALWAYS_INLINE JSTokenType parseIdentifier(JSTokenData*, unsigned lexerFlags, bool strictMode);
    template <bool shouldBuildIdentifiers> NEVER_INLINE JSTokenType parseIdentifierSlowCase(JSTokenData*, unsigned lexerFlags, bool strictMode);
    enum StringParseResult {
        StringParsedSuccessfully,
        StringUnterminated,
        StringCannotBeParsed
    };
    template <bool shouldBuildStrings> ALWAYS_INLINE StringParseResult parseString(JSTokenData*, bool strictMode);
    template <bool shouldBuildStrings> NEVER_INLINE StringParseResult parseStringSlowCase(JSTokenData*, bool strictMode);

    template <bool shouldBuildStrings, LexerEscapeParseMode escapeParseMode> ALWAYS_INLINE StringParseResult parseComplexEscape(bool strictMode, T stringQuoteCharacter);
    ALWAYS_INLINE StringParseResult parseTemplateLiteral(JSTokenData*, RawStringsBuildMode);
    ALWAYS_INLINE void parseHex(double& returnValue);
    ALWAYS_INLINE bool parseBinary(double& returnValue);
    ALWAYS_INLINE bool parseOctal(double& returnValue);
    ALWAYS_INLINE bool parseDecimal(double& returnValue);
    ALWAYS_INLINE void parseNumberAfterDecimalPoint();
    ALWAYS_INLINE bool parseNumberAfterExponentIndicator();
    ALWAYS_INLINE bool parseMultilineComment();

    ALWAYS_INLINE void parseCommentDirective();
    ALWAYS_INLINE String parseCommentDirectiveValue();

    template <unsigned length>
    ALWAYS_INLINE bool consume(const char (&input)[length]);

    void fillTokenInfo(JSToken*, JSTokenType, int lineNumber, int endOffset, int lineStartOffset, JSTextPosition endPosition);

    static const size_t initialReadBufferCapacity = 32;

    int m_lineNumber;
    int m_lastLineNumber;

    Vector<LChar> m_buffer8;
    Vector<UChar> m_buffer16;
    Vector<UChar> m_bufferForRawTemplateString16;
    bool m_terminator;
    int m_lastToken;

    const SourceCode* m_source;
    unsigned m_sourceOffset;
    const T* m_code;
    const T* m_codeStart;
    const T* m_codeEnd;
    const T* m_codeStartPlusOffset;
    const T* m_lineStart;
    JSTextPosition m_positionBeforeLastNewline;
    JSTokenLocation m_lastTokenLocation;
    bool m_isReparsingFunction;
    bool m_atLineStart;
    bool m_error;
    String m_lexErrorMessage;

    String m_sourceURLDirective;
    String m_sourceMappingURLDirective;

    T m_current;

    IdentifierArena* m_arena;

    VM* m_vm;
    bool m_parsingBuiltinFunction;
    JSParserScriptMode m_scriptMode;
};

template <>
ALWAYS_INLINE bool Lexer<LChar>::isWhiteSpace(LChar ch)
{
    return ch == ' ' || ch == '\t' || ch == 0xB || ch == 0xC || ch == 0xA0;
}

template <>
ALWAYS_INLINE bool Lexer<UChar>::isWhiteSpace(UChar ch)
{
    // 0x180E used to be in Zs category before Unicode 6.3, and EcmaScript says that we should keep treating it as such.
    return (ch < 256) ? Lexer<LChar>::isWhiteSpace(static_cast<LChar>(ch)) : (u_charType(ch) == U_SPACE_SEPARATOR || ch == 0x180E || ch == 0xFEFF);
}

template <>
ALWAYS_INLINE bool Lexer<LChar>::isLineTerminator(LChar ch)
{
    return ch == '\r' || ch == '\n';
}

template <>
ALWAYS_INLINE bool Lexer<UChar>::isLineTerminator(UChar ch)
{
    return ch == '\r' || ch == '\n' || (ch & ~1) == 0x2028;
}

template <typename T>
inline unsigned char Lexer<T>::convertHex(int c1, int c2)
{
    return (toASCIIHexValue(c1) << 4) | toASCIIHexValue(c2);
}

template <typename T>
inline UChar Lexer<T>::convertUnicode(int c1, int c2, int c3, int c4)
{
    return (convertHex(c1, c2) << 8) | convertHex(c3, c4);
}

template <typename T>
ALWAYS_INLINE const Identifier* Lexer<T>::makeIdentifier(const LChar* characters, size_t length)
{
    return &m_arena->makeIdentifier(m_vm, characters, length);
}

template <typename T>
ALWAYS_INLINE const Identifier* Lexer<T>::makeIdentifier(const UChar* characters, size_t length)
{
    return &m_arena->makeIdentifier(m_vm, characters, length);
}

template <>
ALWAYS_INLINE const Identifier* Lexer<LChar>::makeRightSizedIdentifier(const UChar* characters, size_t length, UChar)
{
    return &m_arena->makeIdentifierLCharFromUChar(m_vm, characters, length);
}

template <>
ALWAYS_INLINE const Identifier* Lexer<UChar>::makeRightSizedIdentifier(const UChar* characters, size_t length, UChar orAllChars)
{
    if (!(orAllChars & ~0xff))
        return &m_arena->makeIdentifierLCharFromUChar(m_vm, characters, length);

    return &m_arena->makeIdentifier(m_vm, characters, length);
}

template <typename T>
ALWAYS_INLINE const Identifier* Lexer<T>::makeEmptyIdentifier()
{
    return &m_arena->makeEmptyIdentifier(m_vm);
}

template <>
ALWAYS_INLINE void Lexer<LChar>::setCodeStart(const StringView& sourceString)
{
    ASSERT(sourceString.is8Bit());
    m_codeStart = sourceString.characters8();
}

template <>
ALWAYS_INLINE void Lexer<UChar>::setCodeStart(const StringView& sourceString)
{
    ASSERT(!sourceString.is8Bit());
    m_codeStart = sourceString.characters16();
}

template <typename T>
ALWAYS_INLINE const Identifier* Lexer<T>::makeIdentifierLCharFromUChar(const UChar* characters, size_t length)
{
    return &m_arena->makeIdentifierLCharFromUChar(m_vm, characters, length);
}

template <typename T>
ALWAYS_INLINE const Identifier* Lexer<T>::makeLCharIdentifier(const LChar* characters, size_t length)
{
    return &m_arena->makeIdentifier(m_vm, characters, length);
}

template <typename T>
ALWAYS_INLINE const Identifier* Lexer<T>::makeLCharIdentifier(const UChar* characters, size_t length)
{
    return &m_arena->makeIdentifierLCharFromUChar(m_vm, characters, length);
}

#if ASSERT_DISABLED
ALWAYS_INLINE bool isSafeBuiltinIdentifier(VM&, const Identifier*) { return true; }
#else
bool isSafeBuiltinIdentifier(VM&, const Identifier*);
#endif

template <typename T>
ALWAYS_INLINE JSTokenType Lexer<T>::lexExpectIdentifier(JSToken* tokenRecord, unsigned lexerFlags, bool strictMode)
{
    JSTokenData* tokenData = &tokenRecord->m_data;
    JSTokenLocation* tokenLocation = &tokenRecord->m_location;
    ASSERT((lexerFlags & LexerFlagsIgnoreReservedWords));
    const T* start = m_code;
    const T* ptr = start;
    const T* end = m_codeEnd;
    JSTextPosition startPosition = currentPosition();
    if (ptr >= end) {
        ASSERT(ptr == end);
        goto slowCase;
    }
    if (!WTF::isASCIIAlpha(*ptr))
        goto slowCase;
    ++ptr;
    while (ptr < end) {
        if (!WTF::isASCIIAlphanumeric(*ptr))
            break;
        ++ptr;
    }

    // Here's the shift
    if (ptr < end) {
        if ((!WTF::isASCII(*ptr)) || (*ptr == '\\') || (*ptr == '_') || (*ptr == '$'))
            goto slowCase;
        m_current = *ptr;
    } else
        m_current = 0;

    m_code = ptr;
    ASSERT(currentOffset() >= currentLineStartOffset());

    // Create the identifier if needed
    if (lexerFlags & LexexFlagsDontBuildKeywords
#if !ASSERT_DISABLED
        && !m_parsingBuiltinFunction
#endif
        )
        tokenData->ident = 0;
    else
        tokenData->ident = makeLCharIdentifier(start, ptr - start);

    tokenLocation->line = m_lineNumber;
    tokenLocation->lineStartOffset = currentLineStartOffset();
    tokenLocation->startOffset = offsetFromSourcePtr(start);
    tokenLocation->endOffset = currentOffset();
    ASSERT(tokenLocation->startOffset >= tokenLocation->lineStartOffset);
    tokenRecord->m_startPosition = startPosition;
    tokenRecord->m_endPosition = currentPosition();
#if !ASSERT_DISABLED
    if (m_parsingBuiltinFunction) {
        if (!isSafeBuiltinIdentifier(*m_vm, tokenData->ident))
            return ERRORTOK;
    }
#endif

    m_lastToken = IDENT;
    return IDENT;
    
slowCase:
    return lex(tokenRecord, lexerFlags, strictMode);
}

} // namespace JSC
