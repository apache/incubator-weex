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
//
// Created by Xu Jiacheng on 2018/7/17.
//

#include "core/data_render/expression_parser.h"
#include <sstream>
#include <string>
#include <cstdlib>
#include <android/base/log_utils.h>

namespace weex {
namespace core {
namespace data_render {
#define EXPECT(tok)     \
    do {    \
        if (peek() != tok)  \
            LOGE("exprect %s, got %s",Token::str(tok).c_str(),lex()->currentToken().view().c_str());\
        advance();  \
    } while (0)

ExpressionParser::ExpressionParser(ParserContext* context, ASTFactory* builder, Tokenizer* lex)
    : ctx_{context}, builder_{builder}, lex_{lex} {
}

ExpressionParser::~ExpressionParser() {
}

const std::string& ExpressionParser::GetStringLiteral() {
  return lex()->currentToken().view();
}

double ExpressionParser::ParseNumber(const Token& token) {
  //todo error handle
//  try {
  double num = std::strtod(token.view().c_str(), nullptr);
  return num;
//  } catch(std::exception &e) {
//    throw SyntaxError(token, "invalid number (" + token.view() + ")");

//  }
}
int ExpressionParser::ParseInteger(const Token& token) {
  //todo error handle
//  try {
  int num = std::atoi(token.view().c_str());
  return num;
//  } catch(std::exception &e) {
//    throw SyntaxError(token, "invalid number (" + token.view() + ")");

//  }
}

const std::string& ExpressionParser::GetIdentifierName() {
  return lex()->currentToken().view();
}

Token::Type ExpressionParser::peek() {
  return lex()->peek();
}

void ExpressionParser::advance(bool not_regex) {
  lex()->advance(not_regex);
}


Handle<Expression> ExpressionParser::ParsePrimary() {
  auto tok = peek();
  Handle<Expression> result;

  if (tok == Token::NULL_LITERAL) {
    result = builder()->NewNullConstant();
  } else if (tok == Token::NUMBER) {
    result = builder()->NewDoubleConstant(
        ParseNumber(lex()->currentToken()));
  } else if (tok == Token::INTEGER) {
    result = builder()->NewIntegralConstant(
        ParseInteger(lex()->currentToken()));
  } else if (tok == Token::TEMPLATE) {//not support
    result = builder()->NewStringConstant(lex()->currentToken().view());
  } else if (tok == Token::Token::REGEXP_LITERAL) {//not support
    result = builder()->NewStringConstant(lex()->currentToken().view());
  } else if (tok == Token::STRING) {
    result = builder()->NewStringConstant(lex()->currentToken().view());
  } else if (tok == Token::IDENTIFIER) {
    result = builder()->NewIdentifier(lex()->currentToken().view());
  } else if (tok == Token::TRUE_LITERAL) {
    result = builder()->NewBooleanConstant(true);
  } else if (tok == Token::FALSE_LITERAL) {
    result = builder()->NewBooleanConstant(false);
  } else if (tok == Token::LPAREN) {
    advance();    // eat '('
    result = ParseCommaExpression();
    tok = peek();

    if (tok != Token::RPAREN)
//      throw SyntaxError(lex()->currentToken(), "expected a ')'");
      LOGE("expected a ')', got: %s", lex()->currentToken().view().c_str());
  } /*else if (tok == Token::LBRACK) {
    result = ParseArrayLiteral();
  } else if (tok == Token::LBRACE) {
    result = ParseObjectLiteral();
  } */else {
//    throw SyntaxError(lex()->currentToken(), "expected a primary expression");
    LOGE("expected a primary expression %s", lex()->currentToken().view().c_str());
  }

  advance(true);
  return result;
}


Handle<Expression> ExpressionParser::ParseDotExpression() {
  // eat the '.'
  advance();

  auto tok = peek();

  // this token should be a valid identifier
  if (tok != Token::IDENTIFIER && !Token::IsKeyword(tok))
//    throw SyntaxError(lex()->currentToken(), "expected a valid identifier");
    LOGE("expected a valid identifier %s", lex()->currentToken().view().c_str());
  auto name = GetIdentifierName();

  auto ident = builder()->NewIdentifier(name);
  advance(true);
  return ident;
}

Handle<Expression> ExpressionParser::ParseIndexExpression() {
  // eat the '['
  advance();
  auto expr = ParseAssignExpression();
  if (lex()->peek() != Token::RBRACK)
//    throw SyntaxError(lex()->currentToken(), "expected a ']'");
    LOGE("expected a ']' got:%s", lex()->currentToken().view().c_str());

  advance(true); // consumex ']'
  return expr;
}
// MemberExpression :
//    PrimaryExpression
//    FunctionExpression
//    MemberExpression [ Expression ]
//    MemberExpression . IdentifierName
Handle<Expression> ExpressionParser::ParseMemberAndCallExpression() {
  Handle<Expression> primary = nullptr;
  primary = ParsePrimary();


  // if next token is neither '[' or '.' or '('
  Token::Type tok = peek();

  if (tok != Token::LBRACK && tok != Token::PERIOD && tok != Token::LPAREN)
    return primary;

  Handle<Expression> temp;
  Handle<Expression> member = primary;
  MemberAccessKind kind;
  while (true) {
    tok = peek();
    if (tok == Token::PERIOD) {
      temp = ParseDotExpression();
      kind = MemberAccessKind::kDot;
    } else if (tok == Token::LBRACK) {
      temp = ParseIndexExpression();
      kind = MemberAccessKind::kIndex;
    } else if (tok == Token::LPAREN) {
      auto args = ParseArgumentList();
      temp = builder()->NewArgumentList(args);
      member = builder()->NewCallExpression(member, args->raw_list());
      continue;
    } else {
      break;
    }
    member = builder()->NewMemberExpression(kind, member, temp);
  }

  return member;
}

Handle<ExpressionList> ExpressionParser::ParseArgumentList() {
  Handle<ExpressionList> exprs = builder()->NewExpressionList();

  auto tok = peek();
  EXPECT(Token::LPAREN);

  tok = peek();
  if (tok == Token::RPAREN) {
    advance(true);
    return {};
  }

  while (true) {
    auto one = ParseAssignExpression();
    exprs->Insert(one);

    tok = peek();
    if (tok == Token::RPAREN)
      break;
    if (tok != Token::COMMA)
//      throw SyntaxError(lex()->currentToken(), "expected a ',' or ')'");
      LOGE("expect a ',' or ')'");
    advance();
  }

  advance(true); // eat the last ')'
  return exprs;
}

PrefixOperation MapTokenWithPrefixOperator(Token& tok) {
  switch (tok.type()) {
    case Token::INC:
      return PrefixOperation::kIncrement;
    case Token::DEC:
      return PrefixOperation::kDecrement;
    case Token::NOT:
      return PrefixOperation::kNot;
    default:
      LOGE("error prefix opration: %s", tok.view().c_str());
  }
}


Handle<Expression> ExpressionParser::ParseUnaryExpression() {
  // UnaryExpression :
  //     PostfixExpression
  //     delete UnaryExpression
  //     void UnaryExpression
  //     typeof UnaryExpression
  //     ++ UnaryExpression
  //     -- UnaryExpression
  //     + UnaryExpression
  //     - UnaryExpression
  //     ~ UnaryExpression
  //     ! UnaryExpression
  auto tok = peek();

  if (tok == Token::ADD) {
    advance();
    // convert + (Expr) to Expr * 1
    return builder()->NewBinaryExpression(BinaryOperation::kMultiplication,
                                          ParseUnaryExpression(),
                                          builder()->NewIntegralConstant(1));
  } else if (tok == Token::SUB) {
    advance();

    // similarly for `-Expr` to `Expr * -1`
    return builder()->NewBinaryExpression(BinaryOperation::kMultiplication,
                                          ParseUnaryExpression(),
                                          builder()->NewIntegralConstant(1));
  } else if (tok == Token::INC || tok == Token::DEC || tok == Token::NOT) {
    auto token = lex()->currentToken();
    lex()->advance();

    return builder()->NewPrefixExpression(MapTokenWithPrefixOperator(token),
                                          ParseUnaryExpression());
  }

  // PostfixExpression :
  //      LeftHandSideExpression
  //      LeftHandSideExpression [no LineTerminator here] ++
  //      LeftHandSideExpression [no LineTerminator here] --
  auto left = ParseMemberAndCallExpression();

  tok = peek();
  if (tok == Token::INC) {
    advance();
    return builder()->NewPostfixExpression(PostfixOperation::kIncrement,
                                           left);
  } else if (tok == Token::DEC) {
    advance();
    return builder()->NewPostfixExpression(PostfixOperation::kDecrement,
                                           left);
  } else {
    return left;
  }
}

BinaryOperation MapBinaryOperator(Token& tok) {
  switch (tok.type()) {
    case Token::ADD:
      return BinaryOperation::kAddition;
    case Token::SUB:
      return BinaryOperation::kSubtraction;
    case Token::MUL:
      return BinaryOperation::kMultiplication;
    case Token::DIV:
      return BinaryOperation::kDivision;
    case Token::MOD:
      return BinaryOperation::kMod;
    case Token::SHL:
      return BinaryOperation::kShiftLeft;
    case Token::SAR:
      return BinaryOperation::kShiftRight;
    case Token::SHR:
      return BinaryOperation::kShiftZeroRight;
    case Token::LT:
      return BinaryOperation::kLessThan;
    case Token::GT:
      return BinaryOperation::kGreaterThan;
    case Token::LTE:
      return BinaryOperation::kLessThanEqual;
    case Token::GTE:
      return BinaryOperation::kGreaterThanEqual;
    case Token::EQ:
      return BinaryOperation::kEqual;
    case Token::NE:
      return BinaryOperation::kNotEqual;
    case Token::EQ_STRICT:
      return BinaryOperation::kStrictEqual;
    case Token::NE_STRICT:
      return BinaryOperation::kStrictNotEqual;
    case Token::AND:
      return BinaryOperation::kAnd;
    case Token::OR:
      return BinaryOperation::kOr;
    case Token::BIT_AND:
      return BinaryOperation::kBitAnd;
    case Token::BIT_OR:
      return BinaryOperation::kBitOr;
    case Token::BIT_XOR:
      return BinaryOperation::kBitXor;
    default:
//      throw SyntaxError(tok, "unexpected token as binary operator");
      LOGE("unexpected token as binary operator %s", tok.view().c_str());
  }
}

Handle<Expression> ExpressionParser::ParseBinaryExpressionRhs(int prec, Handle<Expression> lhs) {
  while (true) {
    int tokprec = Token::precedence(peek());

    if (tokprec < prec) {
      return lhs;
    }

    // now we definitely have a binary operator
    auto op = MapBinaryOperator(lex()->currentToken());
    advance();

    auto rhs = ParseUnaryExpression();

    auto nextprec = Token::precedence(peek());
    if (tokprec < nextprec) {
      rhs = ParseBinaryExpressionRhs(tokprec + 1, rhs);
    }


    // merge lhs and rhs
    lhs = builder()->NewBinaryExpression(op, lhs, rhs);
  }
}

Handle<Expression> ExpressionParser::ParseBinaryExpression() {
  auto lhs = ParseUnaryExpression();

  // parse the rhs, if any
  return ParseBinaryExpressionRhs(3, lhs);
}
bool IsAssign(Token::Type tok) {
  return tok >= Token::ASSIGN && tok <= Token::ASSIGN_MOD;
}

Handle<Expression> ExpressionParser::ParseAssignExpression() {
  auto lhs = ParseTernaryExpression();
  auto tok = peek();

  if (!IsAssign(tok))
    return lhs;

  // TODO: information about what kind of assignment is done here should
  //  be stored here. (in the AST?)
  advance();
  auto rhs = ParseAssignExpression();
  return builder()->NewAssignExpression((lhs), (rhs));
}

Handle<Expression> ExpressionParser::ParseTernaryExpression() {
  auto first = ParseBinaryExpression();

  auto tok = peek();
  if (tok != Token::CONDITIONAL) {
    return first;
  }

  // now we're parsing conditional expression
  // eat '?'
  advance();
  auto second = ParseAssignExpression();

  tok = peek();
  if (tok != Token::COLON) {
    LOGE("expected a ':', got: %s", lex()->currentToken().view().c_str());
  }

  // eat ':'
  advance();
  auto third = ParseAssignExpression();

  return builder()->NewTernaryExpression(first, second, third);
}

Handle<Expression> ExpressionParser::ParseCommaExpression() {
  auto one = ParseAssignExpression();
  auto tok = lex()->peek();
  return one;
}

Handle<Expression> ExpressionParser::ParseExpression() {
  return ParseCommaExpression();
}
Handle<Expression> ExpressionParser::ParseExpressionByString(const std::string& str) {
  auto factory = weex::core::data_render::ASTFactory::GetFactoryInstance();
  std::istringstream ss(str);
  StandardCharacterStream input(ss);
  ParserContext* context = new weex::core::data_render::ParserContext();
  Tokenizer tokenizer(&input, context);
  ExpressionParser parser(context, factory, &tokenizer);
  Handle<Expression> result = parser.ParseExpression();
  delete context;
  return result;
}

}
}
}