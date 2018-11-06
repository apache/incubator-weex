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
#include "core/data_render/rax_jsx_ast.h"

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

Handle<ExpressionList> ASTFactory::NewExpressionList(const std::vector<Handle<Expression>>& list) {
    return MakeHandle<ExpressionList>(list);
}

Handle<ClassBody> ASTFactory::NewClassBody() {
  return MakeHandle<ClassBody>();
}

Handle<Expression> ASTFactory::NewThisExpression() {
  return MakeHandle<ThisExpression>();
}

Handle<Expression> ASTFactory::NewNewExpression(Handle<Expression> expr, Handle<ExpressionList> args) {
    return MakeHandle<NewExpression>(expr, args);
}

Handle<Expression> ASTFactory::NewIdentifier(std::string name) {
  return MakeHandle<Identifier>(name);
}

Handle<Expression> ASTFactory::NewStringConstant(std::string str) {
  return MakeHandle<StringConstant>(str);
}

Handle<Expression> ASTFactory::NewArgumentList(Handle<ExpressionList> args) {
  return MakeHandle<ArgumentList>(std::move(args));
}

Handle<Expression> ASTFactory::NewBlockStatement(Handle<ExpressionList> list) {
  Handle<Expression> expr = MakeHandle<BlockStatement>(list);
  return expr;
}
Handle<ChunkStatement> ASTFactory::NewChunkStatement(Handle<ExpressionList> list) {
  Handle<ChunkStatement> expr = MakeHandle<ChunkStatement>(list);
  return expr;
}

Handle<ChunkStatement> ASTFactory::NewChunkStatement(Position &loc, Scope *scope, Handle<ExpressionList> list) {
    Handle<ChunkStatement> expr = MakeHandle<ChunkStatement>(loc, scope, list);
    return expr;
}

Handle<Expression> ASTFactory::NewFunctionPrototype(std::string name, std::vector<std::string> args) {
  return MakeHandle<FunctionPrototype>(name, std::move(args));
}

Handle<Expression> ASTFactory::NewFunctionStatement(Handle<FunctionPrototype> proto, Handle<Expression> body) {
  return MakeHandle<FunctionStatement>(proto, body);
}

Handle<Expression> ASTFactory::NewReturnStatement(Handle<Expression> expr) {
  return MakeHandle<ReturnStatement>(expr);
}

Handle<Expression> ASTFactory::NewDeclaration(std::string name, Handle<Expression> init) {
  return MakeHandle<Declaration>(name, init);
}

Handle<Expression> ASTFactory::NewDeclarationList() {
    return MakeHandle<DeclarationList>();
}

Handle<Expression> ASTFactory::NewDeclarationList(std::vector<Handle<Declaration>> decls) {
  return MakeHandle<DeclarationList>(std::move(decls));
}

Handle<Expression> ASTFactory::NewCommaExpression(const std::vector<Handle<Expression>>& list) {
    return MakeHandle<CommaExpression>(NewExpressionList(list));
}

Handle<Expression> ASTFactory::NewBinaryExpression(BinaryOperation op, Handle<Expression> lhs, Handle<Expression> rhs) {
  return MakeHandle<BinaryExpression>(op, lhs, rhs);
}

Handle<Expression> ASTFactory::NewIntegralConstant(int value) {
  return MakeHandle<IntegralConstant>(value);
}

Handle<Expression> ASTFactory::NewDoubleConstant(double value) {
  return MakeHandle<DoubleConstant>(value);
}

Handle<Expression> ASTFactory::NewBooleanConstant(bool value) {
  return MakeHandle<BooleanConstant>(value);
}

Handle<Expression> ASTFactory::NewNullConstant() {
  return MakeHandle<NullConstant>();
}

Handle<Expression> ASTFactory::NewCallExpression(MemberAccessKind kind, Handle<Expression> expr, Handle<Expression> member, std::vector<Handle<Expression>> args) {
    return MakeHandle<CallExpression>(kind, expr, member, args);
}

Handle<Expression> ASTFactory::NewCallExpression(Handle<Expression> func, std::vector<Handle<Expression>> args) {
  return MakeHandle<CallExpression>(func, args);
}

Handle<Expression> ASTFactory::NewMemberExpression(MemberAccessKind kind, Handle<Expression> expr, Handle<Expression> mem) {
  return MakeHandle<MemberExpression>(kind, expr, mem);
}

Handle<Expression> ASTFactory::NewPrefixExpression(PrefixOperation op, Handle<Expression> expr) {
  return MakeHandle<PrefixExpression>(op, expr);
}

Handle<Expression> ASTFactory::NewPostfixExpression(PostfixOperation op, Handle<Expression> expr) {
  return MakeHandle<PostfixExpression>(op, expr);
}

Handle<Expression> ASTFactory::NewForStatement(ForKind kind, Handle<Expression> init, Handle<Expression> condition,
                                               Handle<Expression> update, Handle<Expression> body) {
  return MakeHandle<ForStatement>(kind, init, condition, update, body);
}

Handle<Expression> ASTFactory::NewObjectConstant(ProxyObject obj) {
  return MakeHandle<ObjectConstant>(std::move(obj));
}

Handle<Expression> ASTFactory::NewAssignExpression(Handle<Expression> lhs, Handle<Expression> rhs) {
  return MakeHandle<AssignExpression>(lhs, rhs);
}

Handle<Expression> ASTFactory::NewIfStatement(Handle<Expression> condition, Handle<Expression> then) {
  return MakeHandle<IfStatement>(condition, then);
}

Handle<Expression> ASTFactory::NewIfElseStatement(Handle<Expression> condition, Handle<Expression> then, Handle<Expression> els)
{
    return MakeHandle<IfElseStatement>(condition, then, els);
}

Handle<Expression> ASTFactory::NewUndefinedConstant(Position &loc, Scope *scope)
{
    return MakeHandle<UndefinedConstant>(loc, scope);
}

Handle<Expression> ASTFactory::NewLabelledStatement(Position &loc, Scope *scope,
                                                    std::string label, Handle<Expression> stmt)
{
    return MakeHandle<LabelledStatement>(loc, scope, label, stmt);
}

Handle<Expression> ASTFactory::NewTernaryExpression(Position &loc, Scope *scope,
                                                    Handle<Expression> first, Handle<Expression> second, Handle<Expression> third)
{
    return MakeHandle<TernaryExpression>(loc, scope, first, second, third);
}

Handle<Expression> ASTFactory::NewAssignExpression(Position &loc, Scope *scope,
                                                   Handle<Expression> lhs, Handle<Expression> rhs)
{
    return MakeHandle<AssignExpression>(loc, scope, lhs, rhs);
}

Handle<Expression> ASTFactory::NewPrefixExpression(Position &loc, Scope *scope,
                                                   PrefixOperation op, Handle<Expression> expr)
{
    return MakeHandle<PrefixExpression>(loc, scope, op, expr);
}

Handle<Expression> ASTFactory::NewPostfixExpression(Position &loc, Scope *scope,
                                                    PostfixOperation op, Handle<Expression> expr)
{
    return MakeHandle<PostfixExpression>(loc, scope, op, expr);
}

Handle<Expression> ASTFactory::NewIntegralConstant(Position &loc, Scope *scope, int value)
{
    return MakeHandle<IntegralConstant>(loc, scope, value);
}
Handle<Expression> ASTFactory::NewDoubleConstant(Position &loc, Scope *scope, double value)
{
    return MakeHandle<DoubleConstant>(loc, scope, value);
}

Handle<Expression> ASTFactory::NewNewExpression(Position &loc, Scope *scope, Handle<Expression> expr, Handle<ExpressionList> args)
{
    return MakeHandle<NewExpression>(loc, scope, expr, args);
}

Handle<Expression> ASTFactory::NewIdentifier(Position &loc, Scope *scope, std::string name)
{
    return MakeHandle<Identifier>(loc, scope, name);
}

Handle<Expression> ASTFactory::NewArgumentList(Position &loc, Scope *scope, Handle<ExpressionList> args)
{
    return MakeHandle<ArgumentList>(loc, scope, std::move(args));
}

Handle<Expression> ASTFactory::NewCallExpression(Position &loc, Scope *scope, MemberAccessKind kind, Handle<Expression> func, Handle<Expression> args)
{
    return MakeHandle<CallExpression>(loc, scope, kind, func, args);
}

Handle<Expression> ASTFactory::NewCallExpression(Position &loc, Scope *scope, Handle<Expression>func, Handle<Expression> args) {
    return MakeHandle<CallExpression>(loc, scope, func, args);
}

Handle<Expression> ASTFactory::NewNullConstant(Position &loc, Scope *scope)
{
    return MakeHandle<NullConstant>(loc, scope);
}

Handle<Expression> ASTFactory::NewStringConstant(Position &loc, Scope *scope, std::string str)
{
    return MakeHandle<StringConstant>(loc, scope, str);
}

Handle<Expression> ASTFactory::NewRegexConstant(Position &loc, Scope *scope, std::string str)
{
    return MakeHandle<RegexConstant>(loc, scope, str);
}

Handle<Expression> ASTFactory::NewBooleanConstant(Position &loc, Scope *scope, bool val)
{
    return MakeHandle<BooleanConstant>(loc, scope, val);
}

Handle<Expression> ASTFactory::NewThisExpression(Position &loc, Scope *scope)
{
    return MakeHandle<ThisExpression>(loc, scope);
}

Handle<Expression> ASTFactory::NewArrayConstant(Position &loc, Scope *scope, ProxyArray arr)
{
    return MakeHandle<ArrayConstant>(loc, scope, std::move(arr));
}

Handle<Expression> ASTFactory::NewObjectConstant(Position &loc, Scope *scope,
                                                ProxyObject obj)
{
    return MakeHandle<ObjectConstant>(loc, scope, std::move(obj));
}

Handle<Expression> ASTFactory::NewFunctionPrototype(Position &loc, Scope *scope, std::string name, std::vector<std::string> args)
{
    return MakeHandle<FunctionPrototype>(loc, scope, name, std::move(args));
}

Handle<Expression> ASTFactory::NewFunctionStatement(Position &loc, Scope *scope, Handle<FunctionPrototype> proto, Handle<Expression> body)
{
    return MakeHandle<FunctionStatement>(loc, scope, proto, body);
}

Handle<Expression> ASTFactory::NewBlockStatement(Position &loc, Scope *scope, Handle<ExpressionList> list)
{
    return MakeHandle<BlockStatement>(loc, scope, list);
}

Handle<Expression> ASTFactory::NewForStatement(Position &loc, Scope *scope, ForKind kind, Handle<Expression> init, Handle<Expression> condition, Handle<Expression> update, Handle<Expression> body)
{
    return MakeHandle<ForStatement>(loc, scope, kind, init, condition, update, body);
}

Handle<Expression> ASTFactory::NewBreakStatement(Position &loc, Scope *scope, Handle<Expression> label)
{
    return MakeHandle<BreakStatement>(loc, scope, label);
}

Handle<Expression> ASTFactory::NewContinueStatement(Position &loc, Scope *scope, Handle<Expression> label)
{
    return MakeHandle<ContinueStatement>(loc, scope, label);
}

Handle<Expression> ASTFactory::NewIfStatement(Position &loc, Scope *scope, Handle<Expression> condition, Handle<Expression> then)
{
    return MakeHandle<IfStatement>(loc, scope, condition, then);
}

Handle<Expression> ASTFactory::NewIfElseStatement(Position &loc, Scope *scope, Handle<Expression> condition, Handle<Expression> then, Handle<Expression> els)
{
    return MakeHandle<IfElseStatement>(loc, scope, condition, then, els);
}

Handle<Expression> ASTFactory::NewReturnStatement(Position &loc, Scope *scope, Handle<Expression> expr)
{
    return MakeHandle<ReturnStatement>(loc, scope, expr);
}

Handle<Expression> ASTFactory::NewMemberExpression(Position &loc, Scope *scope, MemberAccessKind kind, Handle<Expression> expr, Handle<Expression> mem)
{
    return MakeHandle<MemberExpression>(loc, scope, kind, expr, mem);
}

Handle<Expression> ASTFactory::NewCommaExpression(Position &loc, Scope *scope, Handle<ExpressionList> l)
{
    return MakeHandle<CommaExpression>(loc, scope, l);
}

Handle<Expression> ASTFactory::NewBinaryExpression(Position &loc, Scope *scope, BinaryOperation op, Handle<Expression> lhs, Handle<Expression> rhs)
{
    return MakeHandle<BinaryExpression>(loc, scope, op, lhs, rhs);
}

Handle<Declaration> ASTFactory::NewDeclaration(Position &loc, Scope *scope, std::string name, Handle<Expression> init)
{
    return MakeHandle<Declaration>(loc, scope, name, init);
}

Handle<Declaration> ASTFactory::NewDeclaration(Position &loc, Scope *scope, Handle<Expression> expr, Handle<Expression> init)
{
    return MakeHandle<Declaration>(loc, scope, expr, init);
}

Handle<Expression> ASTFactory::NewDeclarationList(Position &loc, Scope *scope, std::vector<Handle<Declaration>> decls)
{
    return MakeHandle<DeclarationList>(loc, scope, std::move(decls));
}

Handle<Expression> ASTFactory::NewArrowFunctionStatement(Position &loc, Scope *scope, Handle<Expression> body, std::vector<Handle<Expression>> args)
{
    return MakeHandle<ArrowFunctionStatement>(loc, scope, body, std::move(args));
}

Handle<Expression> ASTFactory::NewJSXNodeExpression(Position &loc, Scope *scope, Handle<Expression> identifier, Handle<Expression> props, Handle<Expression> parent, std::vector<Handle<Expression>> childrens) {
    return MakeHandle<JSXNodeExpression>(loc, scope, identifier, props, parent, std::move(childrens));
}

Handle<Expression> ASTFactory::NewClassStatement(Position &loc, Scope *scope, Handle<Expression> identifier, Handle<Expression> superClass, Handle<Expression> body) {
    return MakeHandle<ClassStatement>(loc, scope, identifier, superClass, body);
}

Handle<Expression> ASTFactory::NewCaseStatement(Position& loc,
                                                Scope* scope,
                                                Handle<Expression> test_value,Handle<ExpressionList> expr) {
  return MakeHandle<CaseStatement>(test_value,expr);
}

Handle<Expression> ASTFactory::NewSwitchStatement(Position& loc,
                                                  Scope* scope,
                                                  Handle<Expression> test_case,
                                                  std::vector<Handle<weex::core::data_render::Expression>> cases) {
  return MakeHandle<SwitchStatement>(test_case,cases);
}

Handle<Expression> ASTFactory::NewClassProperty(Position &loc, Scope *scope, std::string name, Handle<Expression> init)
{
    return MakeHandle<ClassProperty>(loc, scope, name, init);
}

Handle<Expression> ASTFactory::NewTryCatchStatement(Position& loc,
                                                    Scope* scope,
                                                    Handle<Expression> try_block,
                                                    Handle<Expression> catch_expr,
                                                    Handle<Expression> catch_block,
                                                    Handle<Expression> finally) {
  return MakeHandle<TryCatchStatement>(try_block,catch_expr,catch_block,finally);
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
