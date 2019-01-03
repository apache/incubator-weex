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

#include "Identifier.h"
#include "JSCJSValue.h"
#include "JSGlobalObjectFunctions.h"
#include <array>
#include <wtf/text/StringBuilder.h>
#include <wtf/text/WTFString.h>

namespace JSC {

typedef enum { StrictJSON, NonStrictJSON, JSONP } ParserMode;

enum JSONPPathEntryType {
    JSONPPathEntryTypeDeclare, // var pathEntryName = JSON
    JSONPPathEntryTypeDot, // <prior entries>.pathEntryName = JSON
    JSONPPathEntryTypeLookup, // <prior entries>[pathIndex] = JSON
    JSONPPathEntryTypeCall // <prior entries>(JSON)
};

enum ParserState { StartParseObject, StartParseArray, StartParseExpression, 
                   StartParseStatement, StartParseStatementEndStatement, 
                   DoParseObjectStartExpression, DoParseObjectEndExpression,
                   DoParseArrayStartExpression, DoParseArrayEndExpression };
enum TokenType { TokLBracket, TokRBracket, TokLBrace, TokRBrace, 
                 TokString, TokIdentifier, TokNumber, TokColon, 
                 TokLParen, TokRParen, TokComma, TokTrue, TokFalse,
                 TokNull, TokEnd, TokDot, TokAssign, TokSemi, TokError };
    
struct JSONPPathEntry {
    JSONPPathEntryType m_type;
    Identifier m_pathEntryName;
    int m_pathIndex;
};

struct JSONPData {
    Vector<JSONPPathEntry> m_path;
    Strong<Unknown> m_value;
};

template <typename CharType>
struct LiteralParserToken {
private:
WTF_MAKE_NONCOPYABLE(LiteralParserToken<CharType>);

public:
    LiteralParserToken() = default;

    TokenType type;
    const CharType* start;
    const CharType* end;
    union {
        double numberToken;
        struct {
            union {
                const LChar* stringToken8;
                const UChar* stringToken16;
            };
            unsigned stringIs8Bit : 1;
            unsigned stringLength : 31;
        };
    };
};

template <typename CharType>
ALWAYS_INLINE void setParserTokenString(LiteralParserToken<CharType>&, const CharType* string);

template <typename CharType>
class LiteralParser {
public:
    LiteralParser(ExecState* exec, const CharType* characters, unsigned length, ParserMode mode)
        : m_exec(exec)
        , m_lexer(characters, length, mode)
        , m_mode(mode)
    {
    }
    
    String getErrorMessage()
    { 
        if (!m_lexer.getErrorMessage().isEmpty())
            return String::format("JSON Parse error: %s", m_lexer.getErrorMessage().ascii().data());
        if (!m_parseErrorMessage.isEmpty())
            return String::format("JSON Parse error: %s", m_parseErrorMessage.ascii().data());
        return ASCIILiteral("JSON Parse error: Unable to parse JSON string");
    }
    
    JSValue tryLiteralParse()
    {
        m_lexer.next();
        JSValue result = parse(m_mode == StrictJSON ? StartParseExpression : StartParseStatement);
        if (m_lexer.currentToken()->type == TokSemi)
            m_lexer.next();
        if (m_lexer.currentToken()->type != TokEnd)
            return JSValue();
        return result;
    }
    
    bool tryJSONPParse(Vector<JSONPData>&, bool needsFullSourceInfo);

private:
    class Lexer {
    public:
        Lexer(const CharType* characters, unsigned length, ParserMode mode)
            : m_mode(mode)
            , m_ptr(characters)
            , m_end(characters + length)
        {
        }
        
        TokenType next();
        
#if ASSERT_DISABLED
        typedef const LiteralParserToken<CharType>* LiteralParserTokenPtr;

        LiteralParserTokenPtr currentToken()
        {
            return &m_currentToken;
        }
#else
        class LiteralParserTokenPtr;
        friend class LiteralParserTokenPtr;
        class LiteralParserTokenPtr {
        public:
            LiteralParserTokenPtr(Lexer& lexer)
                : m_lexer(lexer)
                , m_tokenID(lexer.m_currentTokenID)
            {
            }

            ALWAYS_INLINE const LiteralParserToken<CharType>* operator->() const
            {
                ASSERT(m_tokenID == m_lexer.m_currentTokenID);
                return &m_lexer.m_currentToken;
            }

        private:
            Lexer& m_lexer;
            unsigned m_tokenID;
        };

        LiteralParserTokenPtr currentToken()
        {
            return LiteralParserTokenPtr(*this);
        }
#endif
        
        String getErrorMessage() { return m_lexErrorMessage; }
        
    private:
        String m_lexErrorMessage;
        template <ParserMode mode> TokenType lex(LiteralParserToken<CharType>&);
        ALWAYS_INLINE TokenType lexIdentifier(LiteralParserToken<CharType>&);
        template <ParserMode mode, char terminator> ALWAYS_INLINE TokenType lexString(LiteralParserToken<CharType>&);
        template <ParserMode mode, char terminator> TokenType lexStringSlow(LiteralParserToken<CharType>&, const CharType* runStart);
        ALWAYS_INLINE TokenType lexNumber(LiteralParserToken<CharType>&);
        LiteralParserToken<CharType> m_currentToken;
        ParserMode m_mode;
        const CharType* m_ptr;
        const CharType* m_end;
        StringBuilder m_builder;
#if !ASSERT_DISABLED
        unsigned m_currentTokenID { 0 };
#endif
    };
    
    class StackGuard;
    JSValue parse(ParserState);

    ExecState* m_exec;
    typename LiteralParser<CharType>::Lexer m_lexer;
    ParserMode m_mode;
    String m_parseErrorMessage;
    static unsigned const MaximumCachableCharacter = 128;
    std::array<Identifier, MaximumCachableCharacter> m_shortIdentifiers;
    std::array<Identifier, MaximumCachableCharacter> m_recentIdentifiers;
    ALWAYS_INLINE const Identifier makeIdentifier(const LChar* characters, size_t length);
    ALWAYS_INLINE const Identifier makeIdentifier(const UChar* characters, size_t length);
};

} // namespace JSC
