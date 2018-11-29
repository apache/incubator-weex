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
// Created by pengtao.pt on 2018/7/25.
//

#include "core/data_render/ast_builder.h"

namespace weex {
namespace core {
namespace data_render {

#ifndef DISABLE_COUNTERS
#define COUNT() ctx_->Counters().Expression()++
#else
#define COUNT()
#endif

Handle<ExpressionList> ASTBuilder::NewExpressionList()
{
    return (factory()->NewExpressionList());
}
    
Handle<ClassBody> ASTBuilder::NewClassBody() {
    return (factory()->NewClassBody());
}
    
Handle<ExpressionList> ASTBuilder::NewChunkStatement(Handle<ExpressionList> list) {
    COUNT();
    return save(factory()->NewChunkStatement(locator()->location(), manager()->current(), list));
}
    
Handle<Expression> ASTBuilder::NewUndefinedConstant()
{
    COUNT();
    return save(factory()->NewUndefinedConstant(locator()->location(), manager()->current()));
}
    
Handle<Expression> ASTBuilder::NewLabelledStatement(std::string label, Handle<Expression> expr)
{
    COUNT();
    return save(factory()->NewLabelledStatement(locator()->location(), manager()->current(), label, expr));
}

Handle<Expression> ASTBuilder::NewTernaryExpression(Handle<Expression> first,
                                                    Handle<Expression> second, Handle<Expression> third)
{
    COUNT();
    return save(factory()->NewTernaryExpression(locator()->location(), manager()->current(), first, second, third));
}
    
Handle<Expression> ASTBuilder::NewDeclaration(std::string name, Handle<Expression> init) {
    COUNT();
    return save(factory()->NewDeclaration(locator()->location(), manager()->current(), name, init));
}
    
Handle<Expression> ASTBuilder::NewDeclaration(Handle<Expression> expr, Handle<Expression> init) {
    COUNT();
    return save(factory()->NewDeclaration(locator()->location(), manager()->current(), expr, init));
}
    
Handle<Expression> ASTBuilder::NewBinaryExpression(BinaryOperation op,
                                                   Handle<Expression> lhs, Handle<Expression> rhs)
{
    COUNT();
    return save(factory()->NewBinaryExpression(locator()->location(), manager()->current(), op, lhs, rhs));
}

Handle<Expression> ASTBuilder::NewAssignExpression(Handle<Expression> lhs, Handle<Expression> rhs)
{
    COUNT();
    return save(factory()->NewAssignExpression(locator()->location(), manager()->current(), lhs, rhs));
}
    
Handle<Expression> ASTBuilder::NewIntegralConstant(int value)
{
    COUNT();
    return save(factory()->NewIntegralConstant(locator()->location(), manager()->current(), value));
}

Handle<Expression> ASTBuilder::NewDoubleConstant(double value)
{
    COUNT();
    return save(factory()->NewDoubleConstant(locator()->location(), manager()->current(), value));
}

Handle<Expression> ASTBuilder::NewPrefixExpression(PrefixOperation op, Handle<Expression> expr)
{
    COUNT();
    return save(factory()->NewPrefixExpression(locator()->location(), manager()->current(), op, expr));
}
    
Handle<Expression> ASTBuilder::NewNewExpression(Handle<Expression> expr, Handle<ExpressionList> args)
{
    COUNT();
    return save(factory()->NewNewExpression(locator()->location(), manager()->current(), expr, args));
}
    
Handle<Expression> ASTBuilder::NewPostfixExpression(PostfixOperation op,
                                                    Handle<Expression> expr)
{
    COUNT();
    return save(factory()->NewPostfixExpression(locator()->location(), manager()->current(), op, expr));
}
    
Handle<Expression> ASTBuilder::NewIdentifier(std::string name)
{
    COUNT();
    return save(factory()->NewIdentifier(locator()->location(), manager()->current(), name));
}
    
Handle<Expression> ASTBuilder::NewArgumentList(Handle<ExpressionList> args)
{
    COUNT();
    return save(factory()->NewArgumentList(locator()->location(), manager()->current(), args));
}

Handle<Expression> ASTBuilder::NewCallExpression(MemberAccessKind kind, Handle<Expression> func, Handle<Expression> args)
{
    COUNT();
    return save(factory()->NewCallExpression(locator()->location(), manager()->current(), kind, func, args));
}
    
Handle<Expression> ASTBuilder::NewCallExpression(Handle<Expression> callee, Handle<Expression> args_expr) {
    COUNT();
    return save(factory()->NewCallExpression(locator()->location(), manager()->current(), callee, args_expr));
}
    
Handle<Expression> ASTBuilder::NewMemberExpression(MemberAccessKind kind,
                                                   Handle<Expression> func, Handle<Expression> args)
{
    COUNT();
    return save(factory()->NewMemberExpression(locator()->location(), manager()->current(), kind, func, args));
}
    
Handle<Expression> ASTBuilder::NewCommaExpression(Handle<ExpressionList> list)
{
    COUNT();
    return save(factory()->NewCommaExpression(locator()->location(), manager()->current(), list));
}

Handle<Expression> ASTBuilder::NewNullConstant()
{
    COUNT();
    return save(factory()->NewNullConstant(locator()->location(), manager()->current()));
}

Handle<Expression> ASTBuilder::NewStringConstant(const std::string &str)
{
    COUNT();
    return save(factory()->NewStringConstant(locator()->location(), manager()->current(), str));
}

Handle<Expression> ASTBuilder::NewRegexConstant(const std::string &str)
{
    COUNT();
    return save(factory()->NewRegexConstant(locator()->location(), manager()->current(), str));
}
    
Handle<Expression> ASTBuilder::NewThisExpression()
{
    COUNT();
    return save(factory()->NewThisExpression(locator()->location(), manager()->current()));
}
    
Handle<Expression> ASTBuilder::NewBooleanConstant(bool value)
{
    COUNT();
    return save(factory()->NewBooleanConstant(locator()->location(), manager()->current(), value));
}
    
Handle<Expression> ASTBuilder::NewArrayConstant(ProxyArray arr)
{
    COUNT();
    return save(factory()->NewArrayConstant(locator()->location(), manager()->current(), std::move(arr)));
}
    
Handle<Expression> ASTBuilder::NewObjectConstant(ProxyObject obj)
{
    COUNT();
    return save(factory()->NewObjectConstant(locator()->location(), manager()->current(), std::move(obj)));
}
    
Handle<Expression> ASTBuilder::NewFunctionPrototype(std::string name, std::vector<std::string> args)
{
    COUNT();
    return save(factory()->NewFunctionPrototype(locator()->location(), manager()->current(), name, std::move(args)));
}
    
Handle<Expression> ASTBuilder::NewFunctionStatement(Handle<FunctionPrototype> proto, Handle<Expression> body)
{
    COUNT();
    return save(factory()->NewFunctionStatement(locator()->location(), manager()->current(), proto, body));
}
    
Handle<Expression> ASTBuilder::NewBlockStatement(Handle<ExpressionList> stmts)
{
    COUNT();
    return save(factory()->NewBlockStatement(locator()->location(), manager()->current(), stmts));
}

Handle<Expression> ASTBuilder::NewForStatement(ForKind kind, Handle<Expression> init, Handle<Expression> cond, Handle<Expression> update, Handle<Expression> body)
{
    COUNT();
    return save(factory()->NewForStatement(locator()->location(), manager()->current(), kind, init, cond, update, body));
}
    
Handle<Expression> ASTBuilder::NewBreakStatement(Handle<Expression> label)
{
    COUNT();
    return save(factory()->NewBreakStatement(locator()->location(), manager()->current(), label));
}
    
Handle<Expression> ASTBuilder::NewContinueStatement(Handle<Expression> label)
{
    COUNT();
    return save(factory()->NewContinueStatement(locator()->location(), manager()->current(), label));
}

Handle<Expression> ASTBuilder::NewIfStatement(Handle<Expression> condition, Handle<Expression> then)
{
    COUNT();
    return save(factory()->NewIfStatement(locator()->location(), manager()->current(), condition, then));
}

Handle<Expression> ASTBuilder::NewIfElseStatement(Handle<Expression> condition, Handle<Expression> then, Handle<Expression> els)
{
    COUNT();
    return save(factory()->NewIfElseStatement(locator()->location(), manager()->current(), condition, then, els));
}

Handle<Expression> ASTBuilder::NewReturnStatement(Handle<Expression> expr)
{
    COUNT();
    return save(factory()->NewReturnStatement(locator()->location(), manager()->current(), expr));
}

Handle<Expression> ASTBuilder::NewArrowFunctionStatement(Handle<Expression> body, std::vector<Handle<Expression>> args)
{
    COUNT();
    return save(factory()->NewArrowFunctionStatement(locator()->location(), manager()->current(), body, args));
}

Handle<Expression> ASTBuilder::NewJSXNodeExpression(Handle<Expression> identifier, Handle<Expression> props, Handle<Expression> parent, std::vector<Handle<Expression>> childrens) {
    COUNT();
    return save(factory()->NewJSXNodeExpression(locator()->location(), manager()->current(), identifier, props, parent, childrens));
}

Handle<Expression> ASTBuilder::NewClassStatement(Handle<Expression> identifier, Handle<Expression> superClass, Handle<Expression> body) {
    COUNT();
    return save(factory()->NewClassStatement(locator()->location(), manager()->current(), identifier, superClass, body));
}
    
Handle<Expression> ASTBuilder::NewClassProperty(std::string name, Handle<Expression> init) {
    COUNT();
    return save(factory()->NewClassProperty(locator()->location(), manager()->current(), name, init));
}

Handle<Expression> ASTBuilder::NewCaseStatement(Handle<Expression> test_case,Handle<ExpressionList> expr) {
    COUNT();
    return save(factory()->NewCaseStatement(locator()->location(), manager()->current(), test_case, expr));
}

Handle<Expression> ASTBuilder::NewSwitchStatement(Handle<Expression> test_value,
                                                  std::vector<Handle<weex::core::data_render::Expression>> cases) {
    COUNT();
    return save(factory()->NewSwitchStatement(locator()->location(), manager()->current(), test_value, cases));
}
Handle<Expression> ASTBuilder::NewTryCatchStatement(Handle<Expression> try_block,
                                                    Handle<weex::core::data_render::Expression> catch_expr,
                                                    Handle<weex::core::data_render::Expression> catch_block,
                                                    Handle<weex::core::data_render::Expression> finally) {
    COUNT();
    return save(factory()->NewTryCatchStatement(locator()->location(), manager()->current(),try_block,catch_expr,catch_block,finally));
}

}
}
}
