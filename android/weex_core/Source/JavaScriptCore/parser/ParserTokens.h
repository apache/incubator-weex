/*
 * Copyright (C) 2010, 2013 Apple Inc. All rights reserved.
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

#include "ParserModes.h"
#include <limits.h>
#include <stdint.h>

namespace JSC {

class Identifier;

enum {
    // Token Bitfield: 0b000000000RTE000IIIIPPPPKUXXXXXXX
    // R = right-associative bit
    // T = unterminated error flag
    // E = error flag
    // I = binary operator allows 'in'
    // P = binary operator precedence
    // K = keyword flag
    // U = unary operator flag
    UnaryOpTokenFlag = 128,
    KeywordTokenFlag = 256,
    BinaryOpTokenPrecedenceShift = 9,
    BinaryOpTokenAllowsInPrecedenceAdditionalShift = 4,
    BinaryOpTokenPrecedenceMask = 15 << BinaryOpTokenPrecedenceShift,
    ErrorTokenFlag = 1 << (BinaryOpTokenAllowsInPrecedenceAdditionalShift + BinaryOpTokenPrecedenceShift + 7),
    UnterminatedErrorTokenFlag = ErrorTokenFlag << 1,
    RightAssociativeBinaryOpTokenFlag = UnterminatedErrorTokenFlag << 1
};

#define BINARY_OP_PRECEDENCE(prec) (((prec) << BinaryOpTokenPrecedenceShift) | ((prec) << (BinaryOpTokenPrecedenceShift + BinaryOpTokenAllowsInPrecedenceAdditionalShift)))
#define IN_OP_PRECEDENCE(prec) ((prec) << (BinaryOpTokenPrecedenceShift + BinaryOpTokenAllowsInPrecedenceAdditionalShift))

enum JSTokenType {
    NULLTOKEN = KeywordTokenFlag,
    TRUETOKEN,
    FALSETOKEN,
    BREAK,
    CASE,
    DEFAULT,
    FOR,
    NEW,
    VAR,
    CONSTTOKEN,
    CONTINUE,
    FUNCTION,
    RETURN,
    IF,
    THISTOKEN,
    DO,
    WHILE,
    SWITCH,
    WITH,
    RESERVED,
    RESERVED_IF_STRICT,
    THROW,
    TRY,
    CATCH,
    FINALLY,
    DEBUGGER,
    ELSE,
    IMPORT,
    EXPORT,
    CLASSTOKEN,
    EXTENDS,
    SUPER,

    // Contextual keywords
    
    LET,
    YIELD,
    AWAIT,
    ASYNC,

    FirstContextualKeywordToken = LET,
    LastContextualKeywordToken = ASYNC,
    FirstSafeContextualKeywordToken = AWAIT,
    LastSafeContextualKeywordToken = LastContextualKeywordToken,

    OPENBRACE = 0,
    CLOSEBRACE,
    OPENPAREN,
    CLOSEPAREN,
    OPENBRACKET,
    CLOSEBRACKET,
    COMMA,
    QUESTION,
    BACKQUOTE,
    INTEGER,
    DOUBLE,
    IDENT,
    STRING,
    TEMPLATE,
    REGEXP,
    SEMICOLON,
    COLON,
    DOT,
    EOFTOK,
    EQUAL,
    PLUSEQUAL,
    MINUSEQUAL,
    MULTEQUAL,
    DIVEQUAL,
    LSHIFTEQUAL,
    RSHIFTEQUAL,
    URSHIFTEQUAL,
    ANDEQUAL,
    MODEQUAL,
    POWEQUAL,
    XOREQUAL,
    OREQUAL,
    DOTDOTDOT,
    ARROWFUNCTION,
    LastUntaggedToken,

    // Begin tagged tokens
    PLUSPLUS = 0 | UnaryOpTokenFlag,
    MINUSMINUS = 1 | UnaryOpTokenFlag,
    AUTOPLUSPLUS = 2 | UnaryOpTokenFlag,
    AUTOMINUSMINUS = 3 | UnaryOpTokenFlag,
    EXCLAMATION = 4 | UnaryOpTokenFlag,
    TILDE = 5 | UnaryOpTokenFlag,
    TYPEOF = 6 | UnaryOpTokenFlag | KeywordTokenFlag,
    VOIDTOKEN = 7 | UnaryOpTokenFlag | KeywordTokenFlag,
    DELETETOKEN = 8 | UnaryOpTokenFlag | KeywordTokenFlag,
    OR = 0 | BINARY_OP_PRECEDENCE(1),
    AND = 1 | BINARY_OP_PRECEDENCE(2),
    BITOR = 2 | BINARY_OP_PRECEDENCE(3),
    BITXOR = 3 | BINARY_OP_PRECEDENCE(4),
    BITAND = 4 | BINARY_OP_PRECEDENCE(5),
    EQEQ = 5 | BINARY_OP_PRECEDENCE(6),
    NE = 6 | BINARY_OP_PRECEDENCE(6),
    STREQ = 7 | BINARY_OP_PRECEDENCE(6),
    STRNEQ = 8 | BINARY_OP_PRECEDENCE(6),
    LT = 9 | BINARY_OP_PRECEDENCE(7),
    GT = 10 | BINARY_OP_PRECEDENCE(7),
    LE = 11 | BINARY_OP_PRECEDENCE(7),
    GE = 12 | BINARY_OP_PRECEDENCE(7),
    INSTANCEOF = 13 | BINARY_OP_PRECEDENCE(7) | KeywordTokenFlag,
    INTOKEN = 14 | IN_OP_PRECEDENCE(7) | KeywordTokenFlag,
    LSHIFT = 15 | BINARY_OP_PRECEDENCE(8),
    RSHIFT = 16 | BINARY_OP_PRECEDENCE(8),
    URSHIFT = 17 | BINARY_OP_PRECEDENCE(8),
    PLUS = 18 | BINARY_OP_PRECEDENCE(9) | UnaryOpTokenFlag,
    MINUS = 19 | BINARY_OP_PRECEDENCE(9) | UnaryOpTokenFlag,
    TIMES = 20 | BINARY_OP_PRECEDENCE(10),
    DIVIDE = 21 | BINARY_OP_PRECEDENCE(10),
    MOD = 22 | BINARY_OP_PRECEDENCE(10),
    POW = 23 | BINARY_OP_PRECEDENCE(11) | RightAssociativeBinaryOpTokenFlag, // Make sure that POW has the highest operator precedence.
    ERRORTOK = 0 | ErrorTokenFlag,
    UNTERMINATED_IDENTIFIER_ESCAPE_ERRORTOK = 0 | ErrorTokenFlag | UnterminatedErrorTokenFlag,
    INVALID_IDENTIFIER_ESCAPE_ERRORTOK = 1 | ErrorTokenFlag,
    UNTERMINATED_IDENTIFIER_UNICODE_ESCAPE_ERRORTOK = 2 | ErrorTokenFlag | UnterminatedErrorTokenFlag,
    INVALID_IDENTIFIER_UNICODE_ESCAPE_ERRORTOK = 3 | ErrorTokenFlag,
    UNTERMINATED_MULTILINE_COMMENT_ERRORTOK = 4 | ErrorTokenFlag | UnterminatedErrorTokenFlag,
    UNTERMINATED_NUMERIC_LITERAL_ERRORTOK = 5 | ErrorTokenFlag | UnterminatedErrorTokenFlag,
    UNTERMINATED_OCTAL_NUMBER_ERRORTOK = 6 | ErrorTokenFlag | UnterminatedErrorTokenFlag,
    INVALID_NUMERIC_LITERAL_ERRORTOK = 7 | ErrorTokenFlag,
    UNTERMINATED_STRING_LITERAL_ERRORTOK = 8 | ErrorTokenFlag | UnterminatedErrorTokenFlag,
    INVALID_STRING_LITERAL_ERRORTOK = 9 | ErrorTokenFlag,
    INVALID_PRIVATE_NAME_ERRORTOK = 10 | ErrorTokenFlag,
    UNTERMINATED_HEX_NUMBER_ERRORTOK = 11 | ErrorTokenFlag | UnterminatedErrorTokenFlag,
    UNTERMINATED_BINARY_NUMBER_ERRORTOK = 12 | ErrorTokenFlag | UnterminatedErrorTokenFlag,
    UNTERMINATED_TEMPLATE_LITERAL_ERRORTOK = 13 | ErrorTokenFlag | UnterminatedErrorTokenFlag,
    UNTERMINATED_REGEXP_LITERAL_ERRORTOK = 14 | ErrorTokenFlag | UnterminatedErrorTokenFlag,
    INVALID_TEMPLATE_LITERAL_ERRORTOK = 15 | ErrorTokenFlag,
};

struct JSTextPosition {
    JSTextPosition() : line(0), offset(0), lineStartOffset(0) { }
    JSTextPosition(int _line, int _offset, int _lineStartOffset) : line(_line), offset(_offset), lineStartOffset(_lineStartOffset) { }
    JSTextPosition(const JSTextPosition& other) : line(other.line), offset(other.offset), lineStartOffset(other.lineStartOffset) { }

    JSTextPosition operator+(int adjustment) const { return JSTextPosition(line, offset + adjustment, lineStartOffset); }
    JSTextPosition operator+(unsigned adjustment) const { return *this + static_cast<int>(adjustment); }
    JSTextPosition operator-(int adjustment) const { return *this + (- adjustment); }
    JSTextPosition operator-(unsigned adjustment) const { return *this + (- static_cast<int>(adjustment)); }

    operator int() const { return offset; }

    int line;
    int offset;
    int lineStartOffset;
};

union JSTokenData {
    struct {
        uint32_t line;
        uint32_t offset;
        uint32_t lineStartOffset;
    };
    double doubleValue;
    const Identifier* ident;
    struct {
        const Identifier* cooked;
        const Identifier* raw;
        bool isTail;
    };
    struct {
        const Identifier* pattern;
        const Identifier* flags;
    };
};

struct JSTokenLocation {
    JSTokenLocation() : line(0), lineStartOffset(0), startOffset(0) { }
    JSTokenLocation(const JSTokenLocation& location)
    {
        line = location.line;
        lineStartOffset = location.lineStartOffset;
        startOffset = location.startOffset;
        endOffset = location.endOffset;
    }

    int line;
    unsigned lineStartOffset;
    unsigned startOffset;
    unsigned endOffset;
};

struct JSToken {
    JSTokenType m_type;
    JSTokenData m_data;
    JSTokenLocation m_location;
    JSTextPosition m_startPosition;
    JSTextPosition m_endPosition;
};

ALWAYS_INLINE bool isUpdateOp(JSTokenType token)
{
    return token >= PLUSPLUS && token <= AUTOMINUSMINUS;
}

ALWAYS_INLINE bool isUnaryOp(JSTokenType token)
{
    return token & UnaryOpTokenFlag;
}

} // namespace JSC
