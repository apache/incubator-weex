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

#include "core/data_render/ast.h"
#include "core/data_render/ast_factory.h"
#include "core/data_render/statement.h"

namespace weex {
namespace core {
namespace data_render {

ASTFactory* ASTFactory::GetFactoryInstance() {
  static std::unique_ptr<ASTFactory> factory_instance;

  if (!factory_instance) {
    factory_instance.reset(new ASTFactory());
  }
  return factory_instance.get();
}

Handle<ExpressionList> ASTFactory::NewExpressionList() {
  return MakeHandle<ExpressionList>();
}
Handle<Expression> ASTFactory::NewIdentifier(Json& json, std::string name) {
  return MakeHandle<Identifier>(json, name);
}
Handle<Expression> ASTFactory::NewIdentifier(std::string name) {
  return MakeHandle<Identifier>(name);
}
Handle<Expression> ASTFactory::NewStringConstant(Json& json, std::string str) {
  return MakeHandle<StringConstant>(json, str);
}
Handle<Expression> ASTFactory::NewStringConstant(std::string str) {
  return MakeHandle<StringConstant>(str);
}
Handle<Expression> ASTFactory::NewArgumentList(Json& json, Handle<ExpressionList> args) {
  return MakeHandle<ArgumentList>(json, std::move(args));
}
Handle<Expression> ASTFactory::NewArgumentList(Handle<ExpressionList> args) {
  return MakeHandle<ArgumentList>(std::move(args));
}
Handle<Expression> ASTFactory::NewBlockStatement(Json& json, Handle<ExpressionList> list) {
  Handle<Expression> expr = MakeHandle<BlockStatement>(json, list);
  return expr;
}
Handle<ChunkStatement> ASTFactory::NewChunkStatement(Json& json, Handle<ExpressionList> list) {
  Handle<ChunkStatement> expr = MakeHandle<ChunkStatement>(json, list);
  return expr;
}
Handle<Expression> ASTFactory::NewFunctionPrototype(
    Json& json, std::string name, std::vector<std::string> args) {
  return MakeHandle<FunctionPrototype>(json, name, std::move(args));
}
Handle<Expression> ASTFactory::NewFunctionStatement(
    Json& json, Handle<FunctionPrototype> proto, Handle<Expression> body) {
  return MakeHandle<FunctionStatement>(json, proto, body);
}
Handle<Expression> ASTFactory::NewReturnStatement(Handle<Expression> expr) {
  return MakeHandle<ReturnStatement>(expr);
}
Handle<Expression> ASTFactory::NewDeclaration(
    Json& json, std::string name, Handle<Expression> init) {
  return MakeHandle<Declaration>(json, name, init);
}
Handle<Expression> ASTFactory::NewDeclaration(
    std::string name, Handle<Expression> init) {
  return MakeHandle<Declaration>(name, init);
}
Handle<Expression> ASTFactory::NewDeclarationList(Json& json) {
  return MakeHandle<DeclarationList>(json);
}
Handle<Expression> ASTFactory::NewDeclarationList(
    Json& json, std::vector<Handle<Declaration>> decls) {
  return MakeHandle<DeclarationList>(json, std::move(decls));
}
Handle<Expression> ASTFactory::NewTernaryExpression(
    Handle<Expression> first, Handle<Expression> second, Handle<Expression> third) {
  return MakeHandle<TernaryExpression>(first, second, third);
}
Handle<Expression>
ASTFactory::NewCommaExpression(const std::vector<Handle<Expression>>& list) {
  return MakeHandle<CommaExpression>(list);
}
Handle<Expression> ASTFactory::NewBinaryExpression(
    Json& json, BinaryOperation op, Handle<Expression> lhs,
    Handle<Expression> rhs) {
  return MakeHandle<BinaryExpression>(json, op, lhs, rhs);
}
Handle<Expression> ASTFactory::NewBinaryExpression(BinaryOperation op, Handle<Expression> lhs,
                                                   Handle<Expression> rhs) {
  return MakeHandle<BinaryExpression>(op, lhs, rhs);
}
Handle<Expression> ASTFactory::NewIntegralConstant(Json& json, int value) {
  return MakeHandle<IntegralConstant>(json, value);
}
Handle<Expression> ASTFactory::NewIntegralConstant(int value) {
  return MakeHandle<IntegralConstant>(value);
}
Handle<Expression> ASTFactory::NewDoubleConstant(Json& json, double value) {
  return MakeHandle<DoubleConstant>(json, value);
}
Handle<Expression> ASTFactory::NewDoubleConstant(double value) {
  return MakeHandle<DoubleConstant>(value);
}
Handle<Expression> ASTFactory::NewBooleanConstant(Json& json, bool value) {
  return MakeHandle<BooleanConstant>(json, value);
}
Handle<Expression> ASTFactory::NewBooleanConstant(bool value) {
  return MakeHandle<BooleanConstant>(value);
}
Handle<Expression> ASTFactory::NewNullConstant(Json& json) {
  return MakeHandle<NullConstant>(json);
}
Handle<Expression> ASTFactory::NewNullConstant() {
  return MakeHandle<NullConstant>();
}
Handle<Expression> ASTFactory::NewCallExpression(
    Json& json, MemberAccessKind kind, Handle<Expression> func,
    Handle<Expression> args) {
  return MakeHandle<CallExpression>(json, kind, func, args);
}
Handle<Expression> ASTFactory::NewCallExpression(Json& json, Handle<Expression> func,
                                                 std::vector<Handle<Expression>> args) {
  return MakeHandle<CallExpression>(json, func, args);
}
Handle<Expression> ASTFactory::NewCallExpression(Handle<Expression> func,
                                                 std::vector<Handle<Expression>> args) {
  return MakeHandle<CallExpression>(func, args);
}
Handle<Expression> ASTFactory::NewCallExpression(Handle<Expression> func,
                                                 Handle<Expression> arg) {
  std::vector<Handle<Expression>> args;
  args.push_back(arg);
  return MakeHandle<CallExpression>(func, args);
}
Handle<Expression> ASTFactory::NewMemberExpression(
    Json& json, MemberAccessKind kind, Handle<Expression> expr,
    Handle<Expression> mem) {
  return MakeHandle<MemberExpression>(json, kind, expr, mem);
}
Handle<Expression> ASTFactory::NewMemberExpression(
    MemberAccessKind kind, Handle<Expression> expr,
    Handle<Expression> mem) {
  return MakeHandle<MemberExpression>(kind, expr, mem);
}
Handle<Expression> ASTFactory::NewPrefixExpression(
    Json& json, PrefixOperation op, Handle<Expression> expr) {
  return MakeHandle<PrefixExpression>(json, op, expr);
}
Handle<Expression> ASTFactory::NewPrefixExpression(
    PrefixOperation op, Handle<Expression> expr) {
  return MakeHandle<PrefixExpression>(op, expr);
}
Handle<Expression> ASTFactory::NewPostfixExpression(
    Json& json, PostfixOperation op, Handle<Expression> expr) {
  return MakeHandle<PostfixExpression>(json, op, expr);
}
Handle<Expression> ASTFactory::NewPostfixExpression(
    PostfixOperation op, Handle<Expression> expr) {
  return MakeHandle<PostfixExpression>(op, expr);
}
Handle<Expression> ASTFactory::NewForStatement(Json& json, ForKind kind, Handle<Expression> init,
                                               Handle<Expression> condition,
                                               Handle<Expression> update, Handle<Expression> body) {
  return MakeHandle<ForStatement>(json, kind, init, condition, update, body);
}
Handle<Expression> ASTFactory::NewObjectConstant(Json& json, ProxyObject obj) {
  return MakeHandle<ObjectConstant>(json, std::move(obj));
}
Handle<Expression> ASTFactory::NewAssignExpression(
    Json& json, Handle<Expression> lhs, Handle<Expression> rhs) {
  return MakeHandle<AssignExpression>(json, lhs, rhs);
}
Handle<Expression> ASTFactory::NewAssignExpression(
    Handle<Expression> lhs, Handle<Expression> rhs) {
  return MakeHandle<AssignExpression>(lhs, rhs);
}
Handle<Expression> ASTFactory::NewIfStatement(
    Json& json, Handle<Expression> condition, Handle<Expression> then) {
  return MakeHandle<IfStatement>(json, condition, then);
}
}  // namespace data_render
}  // namespace core
}  // namespace weex
