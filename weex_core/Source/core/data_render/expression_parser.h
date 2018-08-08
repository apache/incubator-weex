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

#ifndef CORE_DATA_RENDER_EXPRESSION_PARSER_
#define CORE_DATA_RENDER_EXPRESSION_PARSER_

#include "core/data_render/ast_factory.h"
#include "core/data_render/token.h"
#include "core/data_render/tokenizer.h"

namespace weex {
namespace core {
namespace data_render {
class ExpressionParser {
 public:
  ExpressionParser(ASTFactory* builder, Tokenizer* lex);

  ~ExpressionParser();

  Handle<Expression> ParsePrimary();

  //  Handle<Expression> ParseArrayLiteral();
  //  Handle<Expression> ParseObjectLiteral();

  Handle<Expression> ParseDotExpression();
  Handle<Expression> ParseIndexExpression();
  Handle<Expression> ParseMemberAndCallExpression();
  Handle<ExpressionList> ParseArgumentList();

  Handle<Expression> ParseUnaryExpression();

  Handle<Expression> ParseBinaryExpression();
  Handle<Expression> ParseBinaryExpressionRhs(int precedance,
                                              Handle<Expression> lhs);
  Handle<Expression> ParseTernaryExpression();
  Handle<Expression> ParseAssignExpression();
  Handle<Expression> ParseCommaExpression();

  Handle<Expression> ParseExpression();

  static Handle<Expression> ParseExpressionByString(const std::string& str);

 private:
  const std::string& GetStringLiteral();
  const std::string& GetIdentifierName();
  double ParseNumber(const Token& token);
  int ParseInteger(const Token& token);

  Token::Type Peek();
  void Advance(bool not_regex = false);

  ASTFactory* builder() { return builder_; }
  Tokenizer* lex() { return lex_; }
  //  ScopeManager *scope_manager() { return manager_; }

 private:
  ASTFactory* builder_;
  Tokenizer* lex_;
  //  ScopeManager *manager_;
  //  ParserFlags flags_;
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_EXPRESSION_PARSER_