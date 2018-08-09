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

#ifndef CORE_DATA_RENDER_AST_FACTORY_H_
#define CORE_DATA_RENDER_AST_FACTORY_H_

#include <memory>
#include <vector>
#include "core/data_render/ast.h"
#include "core/data_render/handle.h"
#include "core/data_render/statement.h"

namespace weex {
namespace core {
namespace data_render {

class ASTFactory {
  ASTFactory() {}

 public:
  // GetFactoryInstance ::= returns singleton instance of ASTFactory
  static ASTFactory *GetFactoryInstance();
  virtual Handle<ExpressionList> NewExpressionList();
  virtual Handle<Expression> NewIdentifier(Json &json, std::string name);
  virtual Handle<Expression> NewIdentifier(std::string name);
  virtual Handle<ChunkStatement> NewChunkStatement(Json &json,
                                                   Handle<ExpressionList> list);
  virtual Handle<Expression> NewBlockStatement(Json &json,
                                               Handle<ExpressionList> list);
  virtual Handle<Expression> NewFunctionPrototype(
      Json &json, std::string name, std::vector<std::string> args);
  virtual Handle<Expression> NewFunctionStatement(
      Json &json, Handle<FunctionPrototype> proto, Handle<Expression> body);
  virtual Handle<Expression> NewReturnStatement(Handle<Expression> expr);
  virtual Handle<Expression> NewArgumentList(Json &json,
                                             Handle<ExpressionList>);
  virtual Handle<Expression> NewArgumentList(Handle<ExpressionList>);
  virtual Handle<Expression> NewStringConstant(Json &json, std::string str);
  virtual Handle<Expression> NewStringConstant(std::string str);
  virtual Handle<Expression> NewBinaryExpression(Json &json, BinaryOperation op,
                                                 Handle<Expression> lhs,
                                                 Handle<Expression> rhs);
  virtual Handle<Expression> NewBinaryExpression(BinaryOperation op,
                                                 Handle<Expression> lhs,
                                                 Handle<Expression> rhs);
  virtual Handle<Expression> NewTernaryExpression(Handle<Expression> first,
                                                  Handle<Expression> second,
                                                  Handle<Expression> third);
  virtual Handle<Expression> NewCommaExpression(
      const std::vector<Handle<Expression>> &list);
  virtual Handle<Expression> NewDeclaration(Json &json, std::string name,
                                            Handle<Expression> init = nullptr);
  virtual Handle<Expression> NewDeclaration(std::string name,
                                            Handle<Expression> init = nullptr);
  virtual Handle<Expression> NewDeclarationList(
      Json &json, std::vector<Handle<Declaration>> decls);
  virtual Handle<Expression> NewDeclarationList(Json &json);
  virtual Handle<Expression> NewIntegralConstant(Json &json, int value);
  virtual Handle<Expression> NewIntegralConstant(int value);
  virtual Handle<Expression> NewDoubleConstant(Json &json, double value);
  virtual Handle<Expression> NewDoubleConstant(double value);
  virtual Handle<Expression> NewBooleanConstant(Json &json, bool value);
  virtual Handle<Expression> NewBooleanConstant(bool value);
  virtual Handle<Expression> NewNullConstant(Json &json);
  virtual Handle<Expression> NewNullConstant();
  virtual Handle<Expression> NewCallExpression(Json &json,
                                               MemberAccessKind kind,
                                               Handle<Expression> func,
                                               Handle<Expression> args);
  virtual Handle<Expression> NewCallExpression(
      Json &json, Handle<Expression> func,
      std::vector<Handle<Expression>> args);
  virtual Handle<Expression> NewCallExpression(
      Handle<Expression> func, std::vector<Handle<Expression>> args);
  virtual Handle<Expression> NewCallExpression(Handle<Expression> func,
                                               Handle<Expression> arg);
  virtual Handle<Expression> NewMemberExpression(Json &json,
                                                 MemberAccessKind kind,
                                                 Handle<Expression> expr,
                                                 Handle<Expression> mem);
  virtual Handle<Expression> NewMemberExpression(MemberAccessKind kind,
                                                 Handle<Expression> expr,
                                                 Handle<Expression> mem);
  virtual Handle<Expression> NewPrefixExpression(Json &json, PrefixOperation op,
                                                 Handle<Expression> expr);
  virtual Handle<Expression> NewPrefixExpression(PrefixOperation op,
                                                 Handle<Expression> expr);
  virtual Handle<Expression> NewPostfixExpression(Json &json,
                                                  PostfixOperation op,
                                                  Handle<Expression> expr);
  virtual Handle<Expression> NewPostfixExpression(PostfixOperation op,
                                                  Handle<Expression> expr);
  virtual Handle<Expression> NewForStatement(Json &json, ForKind kind,
                                             Handle<Expression> init,
                                             Handle<Expression> condition,
                                             Handle<Expression> update,
                                             Handle<Expression> body);
  virtual Handle<Expression> NewObjectConstant(Json &json, ProxyObject obj);
  virtual Handle<Expression> NewAssignExpression(Json &json,
                                                 Handle<Expression> lhs,
                                                 Handle<Expression> rhs);
  virtual Handle<Expression> NewAssignExpression(Handle<Expression> lhs,
                                                 Handle<Expression> rhs);
  virtual Handle<Expression> NewIfStatement(Json &json,
                                            Handle<Expression> condition,
                                            Handle<Expression> then);
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_AST_FACTORY_H_
