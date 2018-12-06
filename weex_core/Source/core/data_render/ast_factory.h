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
#include "core/data_render/rax_jsx_ast.h"

namespace weex {
namespace core {
namespace data_render {

class ASTFactory {
    ASTFactory() { }
public:
    // GetFactoryInstance ::= returns singleton instance of ASTFactory
    static ASTFactory *GetFactoryInstance();
    virtual Handle<ExpressionList> NewExpressionList();
    virtual Handle<ExpressionList> NewExpressionList(const std::vector<Handle<Expression>>& list);
    virtual Handle<ClassBody> NewClassBody();
    virtual Handle<Expression> NewThisExpression();
    virtual Handle<Expression> NewNewExpression(Handle<Expression> expr, Handle<ExpressionList> args = nullptr);
    virtual Handle<Expression> NewIdentifier(std::string name);
    virtual Handle<ChunkStatement> NewChunkStatement(Handle<ExpressionList> list);
    virtual Handle<Expression> NewBlockStatement(Handle<ExpressionList> list);
    virtual Handle<Expression> NewReturnStatement(Handle<Expression> expr);
    virtual Handle<Expression> NewFunctionStatement(Handle<FunctionPrototype> proto, Handle<Expression> body);
    virtual Handle<Expression> NewFunctionPrototype(std::string name, std::vector<std::string> args);
    virtual Handle<Expression> NewArgumentList(Handle<ExpressionList>);
    virtual Handle<Expression> NewStringConstant(std::string str);
    virtual Handle<Expression> NewBinaryExpression(BinaryOperation op, Handle<Expression> lhs, Handle<Expression> rhs);
    virtual Handle<Expression> NewDeclaration(std::string name, Handle<Expression> init = nullptr);
    virtual Handle<Expression> NewDeclarationList(std::vector<Handle<Declaration>> decls);
    virtual Handle<Expression> NewDeclarationList();
    virtual Handle<Expression> NewIntegralConstant(int value);
    virtual Handle<Expression> NewDoubleConstant(double value);
    virtual Handle<Expression> NewBooleanConstant(bool value);
    virtual Handle<Expression> NewNullConstant();
    virtual Handle<Expression> NewCallExpression(MemberAccessKind kind, Handle<Expression> expr, Handle<Expression> member, std::vector<Handle<Expression>> args);
    virtual Handle<Expression> NewCommaExpression(const std::vector<Handle<Expression>>& list);
    virtual Handle<Expression> NewCallExpression(Handle<Expression>func, std::vector<Handle<Expression>> args);
    virtual Handle<Expression> NewMemberExpression(MemberAccessKind kind, Handle<Expression> expr, Handle<Expression> mem);
    virtual Handle<Expression> NewPrefixExpression(PrefixOperation op, Handle<Expression> expr);
    virtual Handle<Expression> NewPostfixExpression(PostfixOperation op, Handle<Expression> expr);
    virtual Handle<Expression> NewForStatement(ForKind kind, Handle<Expression> init, Handle<Expression> condition, Handle<Expression> update, Handle<Expression> body);
    virtual Handle<Expression> NewObjectConstant(ProxyObject obj);
    virtual Handle<Expression> NewAssignExpression(Handle<Expression> lhs, Handle<Expression> rhs);
    virtual Handle<Expression> NewIfStatement(Handle<Expression> condition, Handle<Expression> then);
    virtual Handle<Expression> NewIfElseStatement(Handle<Expression> condition, Handle<Expression> then, Handle<Expression> els);
    virtual Handle<ChunkStatement> NewChunkStatement(Position &loc, Scope *scope, Handle<ExpressionList> list);
    virtual Handle<Expression> NewLabelledStatement(Position &loc, Scope *scope, std::string label, Handle<Expression> expr);
    // create a new node representing undefined
    virtual Handle<Expression> NewUndefinedConstant(Position &loc, Scope *scope);
    virtual Handle<Expression> NewTernaryExpression(Position &loc, Scope *scope, Handle<Expression> first, Handle<Expression> second, Handle<Expression> third);
    virtual Handle<Expression> NewBinaryExpression(Position &loc, Scope *scope, BinaryOperation op, Handle<Expression> lhs, Handle<Expression> rhs);
    virtual Handle<Expression> NewAssignExpression(Position &loc, Scope *scope, Handle<Expression> lhs, Handle<Expression> rhs);
    virtual Handle<Expression> NewPrefixExpression(Position &loc, Scope *scope, PrefixOperation op, Handle<Expression> expr);
    
    virtual Handle<Expression> NewPostfixExpression(Position &loc, Scope *scope, PostfixOperation op, Handle<Expression> expr);
    virtual Handle<Expression> NewIntegralConstant(Position &loc, Scope *scope, int value);
    virtual Handle<Expression> NewDoubleConstant(Position &loc, Scope *scope, double value);
    virtual Handle<Expression> NewNewExpression(Position &loc, Scope *scope, Handle<Expression> expr, Handle<ExpressionList> args = nullptr);
    virtual Handle<Expression> NewIdentifier(Position &loc, Scope *scope, std::string name);
    virtual Handle<Expression> NewArgumentList(Position &loc, Scope *scope, Handle<ExpressionList>);
    virtual Handle<Expression> NewCallExpression(Position &loc, Scope *scope, MemberAccessKind kind, Handle<Expression> func, Handle<Expression> args);
    virtual Handle<Expression> NewCallExpression(Position &loc, Scope *scope, Handle<Expression>func, Handle<Expression> args);
    virtual Handle<Expression> NewMemberExpression(Position &loc, Scope *scope, MemberAccessKind kind, Handle<Expression> expr, Handle<Expression> mem);
    virtual Handle<Expression> NewNullConstant(Position &loc, Scope *scope);
    virtual Handle<Expression> NewStringConstant(Position &loc, Scope *scope, std::string str);
    virtual Handle<Expression> NewRegexConstant(Position &loc, Scope *scope, std::string str);
    virtual Handle<Expression> NewThisExpression(Position &loc, Scope *scope);
    virtual Handle<Expression> NewBooleanConstant(Position &loc, Scope *scope, bool val);
    virtual Handle<Expression> NewArrayConstant(Position &loc, Scope *scope, ProxyArray arr);
    virtual Handle<Expression> NewObjectConstant(Position &loc, Scope *scope, ProxyObject obj);
    virtual Handle<Expression> NewFunctionPrototype(Position &loc, Scope *scope, std::string name, std::vector<std::string> args);
    virtual Handle<Expression> NewFunctionStatement(Position &loc, Scope *scope, Handle<FunctionPrototype> proto, Handle<Expression> body);
    virtual Handle<Expression> NewBlockStatement(Position &loc, Scope *scope, Handle<ExpressionList> list);
    virtual Handle<Expression> NewForStatement(Position &loc, Scope *scope, ForKind kind, Handle<Expression> init, Handle<Expression> condition, Handle<Expression> update, Handle<Expression> body);
    virtual Handle<Expression> NewBreakStatement(Position &loc, Scope *scope, Handle<Expression> label = nullptr);
    virtual Handle<Expression> NewCaseStatement(Position &loc, Scope *scope, Handle<Expression> test_value,Handle<ExpressionList> expr);
    virtual Handle<Expression> NewSwitchStatement(Position &loc, Scope *scope, Handle<Expression> test_case, std::vector<Handle<Expression>> cases);
    virtual Handle<Expression> NewTryCatchStatement(Position &loc, Scope *scope,
                                                    Handle<Expression> try_block,
                                                    Handle<Expression> catch_expr,
                                                    Handle<Expression> catch_block,
                                                    Handle<Expression> finally );
    virtual Handle<Expression> NewContinueStatement(Position &loc, Scope *scope, Handle<Expression> label = nullptr);
    virtual Handle<Expression> NewIfStatement(Position &loc, Scope *scope, Handle<Expression> condition, Handle<Expression> then);
    virtual Handle<Expression> NewIfElseStatement(Position &loc, Scope *scope, Handle<Expression> condition, Handle<Expression> then, Handle<Expression> els);
    virtual Handle<Expression> NewReturnStatement(Position &loc, Scope *scope, Handle<Expression> expr);
    virtual Handle<Expression> NewCommaExpression(Position &loc, Scope *scope, Handle<ExpressionList> l);
    virtual Handle<Declaration> NewDeclaration(Position &loc, Scope *scope, std::string name, Handle<Expression> init = nullptr);
    virtual Handle<Declaration> NewDeclaration(Position &loc, Scope *scope, Handle<Expression> expr, Handle<Expression> init = nullptr);
    virtual Handle<Expression> NewDeclarationList(Position &loc, Scope *scope, std::vector<Handle<Declaration>> decls);
    virtual Handle<Expression> NewArrowFunctionStatement(Position &loc, Scope *scope, Handle<Expression> body, std::vector<Handle<Expression>> args);
    virtual Handle<Expression> NewJSXNodeExpression(Position &loc, Scope *scope, Handle<Expression> identifier, Handle<Expression> props, Handle<Expression> parent, std::vector<Handle<Expression>> childrens);
    virtual Handle<Expression> NewClassStatement(Position &loc, Scope *scope, Handle<Expression> identifier, Handle<Expression> superClass, Handle<Expression> body);
    virtual Handle<Expression> NewClassProperty(Position &loc, Scope *scope, std::string name, Handle<Expression> init = nullptr);

};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_AST_FACTORY_H_
