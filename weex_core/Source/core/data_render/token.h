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

#ifndef WEEX_TOKEN_H_
#define WEEX_TOKEN_H_

#include <assert.h>
#include <cstdint>
#include <string>

namespace weex {
namespace core {
namespace data_render {

//   T: Non-keyword tokens
//   K: Keyword tokens
#define TOKEN_TYPE_LIST(T, K)                                      \
                                                                   \
  /* Punctuators (ECMA-262, section 7.7, page 15). */              \
  T(LPAREN, "(", 0)                                                \
  T(RPAREN, ")", 0)                                                \
  T(LBRACK, "[", 0)                                                \
  T(RBRACK, "]", 0)                                                \
  T(LBRACE, "{", 0)                                                \
  T(RBRACE, "}", 0)                                                \
  T(COLON, ":", 0)                                                 \
  T(SEMICOLON, ";", 0)                                             \
  T(PERIOD, ".", 0)                                                \
  T(CONDITIONAL, "?", 2)                                           \
  T(INC, "++", 0)                                                  \
  T(DEC, "--", 0)                                                  \
  T(UNFOLD, "...", 0)                                              \
                                                                   \
  /* Assignment operators. */                                      \
  /* IsAssignmentOp() relies on this block of enum values being */ \
  /* contiguous and sorted in the same order! */                   \
  T(ASSIGN, "=", 2)                                                \
  T(ASSIGN_BIT_OR, "|=", 2)                                        \
  T(ASSIGN_BIT_XOR, "^=", 2)                                       \
  T(ASSIGN_BIT_AND, "&=", 2)                                       \
  T(ASSIGN_SHL, "<<=", 2)                                          \
  T(ASSIGN_SAR, ">>=", 2)                                          \
  T(ASSIGN_SHR, ">>>=", 2)                                         \
  T(ASSIGN_ADD, "+=", 2)                                           \
  T(ASSIGN_SUB, "-=", 2)                                           \
  T(ASSIGN_MUL, "*=", 2)                                           \
  T(ASSIGN_DIV, "/=", 2)                                           \
  T(ASSIGN_MOD, "%=", 2)                                           \
  T(ASSIGN_EXP, "**=", 2)                                          \
                                                                   \
  /* Binary operators sorted by precedence. */                     \
  /* IsBinaryOp() relies on this block of enum values */           \
  /* being contiguous and sorted in the same order! */             \
  T(COMMA, ",", 1)                                                 \
  T(OR, "||", 4)                                                   \
  T(AND, "&&", 5)                                                  \
  T(BIT_OR, "|", 6)                                                \
  T(BIT_XOR, "^", 7)                                               \
  T(BIT_AND, "&", 8)                                               \
  T(SHL, "<<", 11)                                                 \
  T(SAR, ">>", 11)                                                 \
  T(SHR, ">>>", 11)                                                \
  T(ADD, "+", 12)                                                  \
  T(SUB, "-", 12)                                                  \
  T(MUL, "*", 13)                                                  \
  T(DIV, "/", 13)                                                  \
  T(MOD, "%", 13)                                                  \
  T(EXP, "**", 14)                                                 \
                                                                   \
  /* Compare operators sorted by precedence. */                    \
  /* IsCompareOp() relies on this block of enum values */          \
  /* being contiguous and sorted in the same order! */             \
  T(EQ, "==", 9)                                                   \
  T(NE, "!=", 9)                                                   \
  T(EQ_STRICT, "===", 9)                                           \
  T(NE_STRICT, "!==", 9)                                           \
  T(LT, "<", 10)                                                   \
  T(GT, ">", 10)                                                   \
  T(LTE, "<=", 10)                                                 \
  T(GTE, ">=", 10)                                                 \
  T(ARROW_FUNCTION, "=>", 10)                                      \
                                                                   \
  /*jsx */                                                         \
  T(JSX_TAG_CLOSE,"</",0)                                          \
  T(JSX_TAG_END,"/>",0)                                            \
                                                                   \
  /* Unary operators. */                                           \
  /* IsUnaryOp() relies on this block of enum values */            \
  /* being contiguous and sorted in the same order! */             \
  T(NOT, "!", 0)                                                   \
  T(BIT_NOT, "~", 0)                                               \
  K(VOID, "void", 0)                                               \
                                                                   \
  /* Keywords (ECMA-262, section 7.5.2, page 13). */               \
  K(BREAK, "break", 0)                                             \
  K(CASE, "case", 0)                                               \
  K(DEFAULT, "default", 0)                                         \
  K(CONTINUE, "continue", 0)                                       \
  K(ELSE, "else", 0)                                               \
  K(FOR, "for", 0)                                                 \
  K(FUNCTION, "function", 0)                                       \
  K(IF, "if", 0)                                                   \
  K(RETURN, "return", 0)                                           \
  K(SWITCH, "switch", 0)                                           \
  K(TRY, "try", 0)                                                 \
  K(CATCH, "catch", 0)                                             \
  K(FINALLY, "finally", 0)                                         \
  K(VAR, "var", 0)                                                 \
                                                                   \
  /* Literals (ECMA-262, section 7.8, page 16). */                 \
  K(NULL_LITERAL, "null", 0)                                       \
  K(TRUE_LITERAL, "true", 0)                                       \
  K(FALSE_LITERAL, "false", 0)                                     \
  K(UNDEFINED, "undefined", 0)                                     \
  K(IN, "in", 10)                                                  \
  K(THIS, "this", 0)                                               \
  K(TYPEOF, "typeof", 0)                                           \
  K(DELETE, "delete", 0)                                           \
  K(NEW, "new", 0)                                                 \
  K(CLASS, "class", 0)                                             \
  K(CONST, "const", 0)                                             \
  K(EXTENDS, "extends", 0)                                         \
  K(SUPER, "super", 0)                                             \
  K(LET, "let", 0)                                                 \
  K(STATIC, "static", 0)                                           \
                                                                   \
  T(EOS, "EOS", 0)                                                 \
  T(INTEGER, "INTEGER", 0)                                         \
  T(NUMBER, "NUMBER", 0)                                           \
  T(STRING, "STRING", 0)                                           \
  T(TEMPLATE, "TEMPLATE", 0)                                       \
  T(IDENTIFIER, "IDENTIFIER", 0)                                   \
  T(WHITESPACE, "WHITESPACE", 0)                                   \
  T(UNINITIALIZED, "UNINITIALIZED", 0)                             \
  T(REGEXP_LITERAL, "REGEXP_LITERAL", 0)                           \
  T(ERROR, "ERROR", 0)                                             \
  T(INVALID, "INVALID", 0)


class Position {
 public:
  using size_type = std::string::size_type;

  Position()
      : col_ { 0 }, row_{ 0 }
  { }

  Position(size_type col, size_type row)
      : col_{ col }, row_ { row }
  { }

  size_type &col() { return col_; }
  size_type &row() { return row_; }

  const size_type &col() const { return col_; }
  const size_type &row() const { return row_; }
 private:
  size_type col_;
  size_type row_;
};

class Token {
 public:
#define T(name, string, precedence) name,
  enum Type {
    TOKEN_TYPE_LIST(T, T) NUM_TOKENS
  };
#undef T
  static const char* Name(Type token) {
    assert(token < NUM_TOKENS);
    return s_name_[token];
  }

  static bool IsKeyword(Type token) { return s_token_type_[token] == 'K'; }

  static bool IsAssignmentOp(Type token) {
    return ASSIGN <= token && token <= ASSIGN_EXP;
  }

  static bool IsBinaryOp(Type op) { return COMMA <= op && op <= EXP; }

  static bool IsEqualityOp(Type op) { return op == EQ || op == EQ_STRICT; }

  static const char* String(Type token) {
    assert(token < NUM_TOKENS);  // token is unsigned.
    return s_string_[token];
  }

  static std::string Str(Type type);

  static int Precedence(Type token) {
    assert(token < NUM_TOKENS);  // token is unsigned.
    return s_precedence_[token];
  }

  Token(std::string str, Type type, Position pos, size_t index)
      : str_{ str }, type_{ type }, pos_{pos}, index_{ index }
  { }

  Token(double num, Type type, Position pos, size_t index)
      : num_{ num }, type_{ type }, pos_{pos}, index_{ index }
  { }

  Token(){ };

  inline int line() { return line_; }

  inline int column() { return column_; }

  inline Type type() const { return type_; }

  Position &position() { return pos_; }
    
  inline const std::string& view() const { return str_; }

 private:
  int line_;
  int column_;
  std::string str_;
  double num_;
  Type type_;
  std::size_t index_;
  Position pos_;

  static const char* const s_name_[NUM_TOKENS];
  static const char* const s_string_[NUM_TOKENS];
  static const int8_t s_precedence_[NUM_TOKENS];
  static const char s_token_type_[NUM_TOKENS];
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif
