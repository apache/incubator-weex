/*
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

#include "core/data_render/tokenizer.h"
#include "core/data_render/token.h"
#include "core/data_render/scanner.h"
#include "core/data_render/class_string.h"

#include <cstring>
#include <cassert>
#include <cctype>
#include <iostream>
#include <unordered_map>

namespace weex {
namespace core {
namespace data_render {


// TokenizerState implementation
// -------------------------------

#ifdef __ANDROID__
#undef EOF
    static const char EOF = -1;
#endif

class TokenizerState {
 public:
  static std::unordered_map<std::string, Token::Type> keywords;

  friend class Tokenizer;

  using size_type = size_t;
  using seek_type = size_t;
  using char_type = char;

  TokenizerState(CharacterStream* scanner)
      : seek_{0}, scanner_{scanner} {}


  inline void Reset(CharacterStream* scanner) {
    scanner_ = scanner;
    seek_ = 0;
    last_col_length_ = 0;
    position_ = Position();
  }

  inline size_type& seek() {
    return seek_;
  }

  inline Position& position() {
    return position_;
  }

  inline Token& token() {
    return token_;
  }

  inline Token& last_token() {
    return last_token_;
  }

  inline void set_token(Token token) {
    last_token_ = token_;
    token_ = token;
  }

  inline void set_position(int col, int row) {
    position_.col() = col;
    position_.row() = row;
  }

  inline const Position& position() const {
    return position_;
  }

  inline char_type ReadChar() {
    char ch = scanner_->ReadChar();
    if (ch == '\n') {
      position_.row()++;

      // save the last column position
      last_col_length_ = position_.col();
      position_.col() = 0;
    } else if (ch == EOF) {
      // in case of EOF we don't want to go outside the limit
      // of our source code
      seek_++;
      return EOF;
    } else {
      position_.col()++;
    }

    seek_++;
    return ch;
  }

  inline void PutBack(char_type ch) {
    seek_--;
    if (ch == EOF)
      return;
    if (ch == '\n') {
      assert(position_.row() != 0);
      position_.col() = last_col_length_;
      position_.row()--;
    } else {
      assert(position_.col() != 0);
      position_.col()--;
    }
    scanner_->PutBack(ch);
  }

 private:
  seek_type seek_;

  Token token_;
  Token last_token_;

  Position position_;
  size_t last_col_length_;
  CharacterStream* scanner_;
};

std::unordered_map<std::string, Token::Type> TokenizerState::keywords = {
#define K(t, k, p) { k, Token::t },
#define T(t, k, p)
    TOKEN_TYPE_LIST(T, K)
#undef K
#undef T
};

// small utility functions
bool IsValidIdentifierStart(char ch) {
  return std::isalpha(ch) || ch == '_' || ch == '$';
}

bool IsValidIdentifierChar(char ch) {
  return std::isalnum(ch) || ch == '_' || ch == '$';
}

Token::Type IsKeyword(const std::string& str) {
  auto it = TokenizerState::keywords.find(str);
  if (it == TokenizerState::keywords.end())
    return Token::IDENTIFIER;
  return it->second;
}

Token::Type IsOneCharacterSymbol(char ch) {
  // returns the type of token from the given symbol
  switch (ch) {
    case '(':
      return Token::LPAREN;
    case ')':
      return Token::RPAREN;
    case '{':
      return Token::LBRACE;
    case '}':
      return Token::RBRACE;
    case '[':
      return Token::LBRACK;
    case ']':
      return Token::RBRACK;
    case ':':
      return Token::COLON;
    case ';':
      return Token::SEMICOLON;
    case '.':
      return Token::PERIOD;
    case ',':
      return Token::COMMA;
    case '?':
      return Token::CONDITIONAL;
    case '+':
      return Token::ADD;
    case '-':
      return Token::SUB;
    case '/':
      return Token::DIV;
    case '*':
      return Token::MUL;
    case '%':
      return Token::MOD;
    case '>':
      return Token::GT;
    case '<':
      return Token::LT;
    case '=':
      return Token::ASSIGN;
    case '^':
      return Token::BIT_XOR;
    case '|':
      return Token::BIT_OR;
    case '&':
      return Token::BIT_AND;
    case '!':
      return Token::NOT;
    case '~':
      return Token::BIT_NOT;
    default:
      return Token::INVALID;
  }
}

Token::Type IsTwoCharacterSymbol(char ch1, char ch2) {
// returns the type of symbol of two characters
    switch (ch1) {
        case '=':
            switch (ch2) {
                case '>':
                    return Token::ARROW_FUNCTION;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
  switch (ch2) {
    case '=':
      switch (ch1) {
        case '+':
          return Token::ASSIGN_ADD;
        case '-':
          return Token::ASSIGN_SUB;
        case '*':
          return Token::ASSIGN_MUL;
        case '/':
          return Token::ASSIGN_DIV;
        case '%':
          return Token::ASSIGN_MOD;
        case '^':
          return Token::ASSIGN_BIT_XOR;
        case '&':
          return Token::ASSIGN_BIT_AND;
        case '|':
          return Token::ASSIGN_BIT_OR;
        case '>':
          return Token::GTE;
        case '<':
          return Token::LTE;
        case '=':
          return Token::EQ;
        case '!':
          return Token::NE;
      }
      return Token::INVALID;
    case '|':
      if (ch1 == '|')
        return Token::OR;
      return Token::INVALID;
    case '&':
      if (ch1 == '&')
        return Token::AND;
      return Token::INVALID;
    case '+':
      if (ch1 == '+')
        return Token::INC;
      return Token::INVALID;
    case '-':
      if (ch1 == '-')
        return Token::DEC;
      return Token::INVALID;
    case '<':
      if (ch1 == '<')
        return Token::SHL;
      return Token::INVALID;
    case '>':
      if (ch1 == '>')
        return Token::SAR;
    case '/':
      if (ch1 == '<')
        return Token::JSX_TAG_CLOSE;
    default:
      return Token::INVALID;
  }
}

Token::Type IsThreeCharacterSymbol(char ch1, char ch2, char ch3) {
  if (ch1 == '=' && ch2 == '=' && ch3 == '=')
    return Token::EQ_STRICT;
  else if (ch1 == '!' && ch2 == '=' && ch3 == '=')
    return Token::NE_STRICT;
  else if (ch1 == '>' && ch2 == '>' && ch3 == '>')
    return Token::SHR;
  else if (ch1 == '>' && ch2 == '>' && ch3 == '=')
    return Token::ASSIGN_SAR;
  else if (ch1 == '<' && ch2 == '<' && ch3 == '=')
    return Token::ASSIGN_SHL;
  else if (ch1 == '.' && ch2 == '.' && ch3 == '.')
      return Token::UNFOLD;

  return Token::INVALID;
}

bool IsSpace(char ch) {
  return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}


// Tokenizer implementation
// --------------------------

Tokenizer::Tokenizer(CharacterStream *stream, ParserContext *context)
    : state_{new TokenizerState(stream)}, context_{ context } {}

Tokenizer::Tokenizer(CharacterStream *stream)
: state_{new TokenizerState(stream)} {}

Tokenizer::~Tokenizer() {
  delete state_;
}

#define _ state_->

void Tokenizer::Reset(CharacterStream* stream) {
  state_->Reset(stream);
}

void Tokenizer::Advance(bool divide_expected) {
  _ set_token(AdvanceInternal(divide_expected));
}

Token::Type Tokenizer::Peek() {
  if (_ seek() == 0)
    Advance();
  return _ token().type();
}

Token& Tokenizer::CurrentToken() {
  return _ token();
}

// The heart of the lexer
// -----------------------
Token Tokenizer::AdvanceInternal(bool not_regex) {
  char ch = _ ReadChar();

  do {
    // skip all non printable characters
    while (ch != EOF && IsSpace(ch))
      ch = _ ReadChar();


    if (ch == '/') {
      char next = _ ReadChar();

      if (next == EOF) {
        return Token(std::string(1, ch), Token::DIV, _ position(),
                     _ seek());
      } else if (next == '/') {
        // single line comment skip whole line
        while (ch != '\n') {
          ch = _ ReadChar();
        }
      } else if (next == '*') {
        // block comment
        ch = _ ReadChar();
        char last = ch;

        while (ch != '/' || last != '*') {
          if (ch == EOF)
            break;
          last = ch;
          ch = _ ReadChar();
        }

        ch = _ ReadChar();
        if (ch == EOF) {
          return Token(std::string("ERROR"), Token::ERROR,
                       _ position(), _ seek());
        }
      } else if(next == '>'){
        return Token("/>",Token::JSX_TAG_END,_ position(), _ seek());
      } else {
        bool ok = true;
        _ PutBack(next);

        // do not parse this as regular expression
        if (not_regex) break;
        Token t = ParseRegex(&ok);
        if (!ok) {
          break;
        } else {
          return t;
        }
      }
    } else if (!IsSpace(ch)) {
      break;
    } else if (ch == EOF || ch == '\0') {
      return Token(std::string("EOF"), Token::EOS,
                   _ position(), _ seek());
    }
  } while (true);
  auto seek = _ seek();
  auto position = _ position();

  // now check the various possibilities of tokens
  // Possibility 1:
  //  token is a valid identifier, or keyword

  if (IsValidIdentifierStart(ch)) {
    std::string identifier;
    identifier += ch;

    ch = _ ReadChar();
    while (IsValidIdentifierChar(ch)) {
      identifier += ch;
      ch = _ ReadChar();
    }

    _ PutBack(ch);

    return Token(identifier, IsKeyword(identifier), position, seek);
  }

  if (ch == EOF || ch == '\0') {
    return Token(std::string("EOF"), Token::EOS,
                 position, seek);
  }

  char first = ch;
  char second = _ ReadChar();
  char third = _ ReadChar();

  Token::Type type;
  std::string view;

  if (first != EOF && second != EOF) {
    type = IsThreeCharacterSymbol(first, second, third);
    if (type == Token::SHR) {
      char fourth = _ ReadChar();

      if (fourth == '=') {
        // >>>=
        view = view + first + second + third + fourth;
        return Token(view, Token::ASSIGN_SHR, _ position(), _ seek());
      } else {
        _ PutBack(fourth);
        view = view + first + second + third;
        return Token(view, type, position, seek);
      }
    } else if (type != Token::INVALID) {
      view = view + first + second + third;
      return Token(view, type, position, seek);
    }
    _ PutBack(third);
  }


  if (second != EOF) {
    type = IsTwoCharacterSymbol(first, second);

    if (type != Token::INVALID) {
      view = view + first + second;
      return Token(view, type, position, seek);
    }
    _ PutBack(second);
  }


  type = IsOneCharacterSymbol(first);

  if (type == Token::PERIOD) {
    if (isdigit(second)) {
      return ParseNumber(first);
    }
  }
  if (type != Token::INVALID) {
    view += first;
    return Token(view, type, position, seek);
  }

  if (ch == '"' || ch == '\'' || ch == '`') {
    return ParseString(ch);
  }

  if (isdigit(ch)) {
    return ParseNumber(ch);
  } else if (ch == EOF || ch == '\0') {
    return Token(std::string("EOF"), Token::EOS, position, seek);
  }

  return Token(std::string("ILLEGAL"), Token::INVALID, position, seek);
}

Token Tokenizer::ParseRegex(bool* ok) {
  std::string buffer;
  auto seek = _ seek();
  auto position = _ position();

  char ch = _ ReadChar();

  while (ch != '/') {
    if (ch == EOF || ch == '\n') {
      *ok = false;
      if (ch == '\n') {
        _ PutBack('\n');
      }
      for (long i = buffer.length() - 1; i >= 0; i--) {
        _ PutBack(buffer[i]);
      }
      return Token(std::string("ERROR"), Token::ERROR, position, seek);
    }

    if (ch == '[') {
      buffer.push_back(ch);
      ch = _ ReadChar();
      while (ch != ']') {

        if (ch == '\\') {
          buffer.push_back(ch);
          ch = _ ReadChar();
        }

        if (ch == EOF) {
          return Token(std::string("ERROR"), Token::ERROR, position, seek);
        }

        buffer.push_back(ch);
        ch = _ ReadChar();
      }
    }

    if (ch == '\\') {
      buffer.push_back(ch);
      ch = _ ReadChar();
    }

    buffer.push_back(ch);
    ch = _ ReadChar();

  }

  // adding end marker
  buffer.push_back('$');

  // parse regex flags g, i, m, u, y
  // Including special case n when there is no flag present
  ch = _ ReadChar();
  bool flag_present = false;
  while (ch == 'g' || ch == 'i' || ch == 'm' || ch == 'u' || ch == 'y') {
    flag_present = true;
    buffer.push_back(ch);
    ch = _ ReadChar();
  }

  _ PutBack(ch);

  if (!flag_present) {
    buffer.push_back('n');
  }

  return Token(buffer, Token::REGEXP_LITERAL, position, seek);
}

Token Tokenizer::ParseString(char delim) {
  std::string buffer;

  auto seek = _ seek();
  auto position = _ position();
  char ch = _ ReadChar();
  bool utf8 = false;
  while (ch != EOF && ch != delim) {
    // escape characters
    if (ch == '\\') {
      buffer.push_back(ch);
      ch = _ ReadChar();
      if (tolower(ch) == 'u') {
          utf8 = true;
      }
      if (ch == EOF) {
        break;
      }
    }
    buffer.push_back(ch);
    ch = _ ReadChar();
  }
  if (ch == EOF) {
    return Token(std::string("EOF"), Token::ERROR, position, seek);
  }

  Token::Type type = delim == '`' ? Token::TEMPLATE : Token::STRING;

  if (utf8) {
      buffer = utf8_decode(buffer);
  }
  return Token(buffer, type, position, seek);
}

Token Tokenizer::ParseNumber(char start) {
  std::string buffer;
  auto seek = _ seek();
  bool had_exp = false;
  bool seen_dot = false;
  bool isdouble = false;
  auto position = _ position();
  char ch = _ ReadChar();

  buffer.push_back(start);

  if (tolower(ch) == 'x') {
    // parsing hex number
    buffer.push_back(ch);
    ch = _ ReadChar();
    while (ch != EOF && (tolower(ch) == 'a' || tolower(ch) == 'b'
                         || tolower(ch) == 'c' || tolower(ch) == 'd'
                         || tolower(ch) == 'e' || tolower(ch) == 'f'
                         || isdigit(ch))) {
      buffer.push_back(ch);
      ch = _ ReadChar();
    }

  } else if (tolower(ch) == 'b') {
    // parsing a bin number
    buffer.push_back(ch);
    ch = _ ReadChar();
    while (ch != EOF && (ch == '0' || ch == '1')) {
      buffer.push_back(ch);
      ch = _ ReadChar();
    }

  } else if (tolower(ch) == 'o') {
    // parsing oct number
    while (ch != EOF && ch < '8' && ch >= '0') {
      buffer.push_back(ch);
      ch = _ ReadChar();
    }

  } else {
    if (start == '.' || ch == '.') {
      seen_dot = true;
      isdouble = true;
    }

    //todo fix 2-a got 2- problem
    if (ch != EOF)
      buffer.push_back(ch);

    while (isdigit(ch) || (tolower(ch) == 'e') || ch == '.') {
      ch = _ ReadChar();

      if (ch == '.') {
        if (!seen_dot) {
          seen_dot = true;
          isdouble = true;
          buffer.push_back(ch);
        } else {
          // not the character that we should care about
          break;
        }
      } else if (tolower(ch) == 'e') {
        if (!had_exp) {
          had_exp = true;
          buffer.push_back(ch);
          isdouble = true;
        } else {
          break;
        }
      } else if (isdigit(ch)) {
        buffer.push_back(ch);
      } else if (ch == EOF) {
        if (start == '.' && buffer.length() == 1) {
          return Token(std::string("."), Token::PERIOD, position,
                       seek);
        }
        break;
      } else {
        break;
      }
    }
  }


  _ PutBack(ch);

  if (!isdouble) {
    return Token(buffer, Token::INTEGER, position, seek);
  } else {
    return Token(buffer, Token::NUMBER, position, seek);
  }
}

}
}
}

