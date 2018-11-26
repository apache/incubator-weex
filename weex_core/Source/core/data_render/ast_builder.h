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
// Created by pentao.pt on 2018/7/25.
//

#ifndef DATA_RENDER_RAX_AST_BUILDER_
#define DATA_RENDER_RAX_AST_BUILDER_

#include "core/data_render/ast_factory.h"
#include "core/data_render/rax_source_locator.h"
#include "core/data_render/rax_parser_context.h"
#include "core/data_render/ast.h"
#include "core/data_render/rax_parser_scope.h"
#include <list>

namespace weex {
namespace core {
namespace data_render {

class ASTBuilder {
public:
    ASTBuilder(ParserContext *ctx, ASTFactory *factory,
               SourceLocator *locator, ScopeManager *manager)
    : factory_{ factory }, locator_{ locator }, ctx_{ ctx }, manager_{manager}
    { }
    // creates a heap allocated expression list and return a pointer to it.
    // Helpful for expression which requires list of expression
    Handle<ExpressionList> NewExpressionList();
    Handle<ExpressionList> NewChunkStatement(Handle<ExpressionList> list);
    // create a new node representing undefined
    Handle<Expression> NewUndefinedConstant();
    // create a new node representing JavaScript label statement
    Handle<Expression> NewLabelledStatement(std::string label, Handle<Expression> expr);
    // create a new node representing JavaScript ternary expression
    Handle<Expression> NewTernaryExpression(Handle<Expression> first, Handle<Expression> second, Handle<Expression> third);
    // create a new node representing JavaScript binary expression
    Handle<Expression> NewBinaryExpression(BinaryOperation op, Handle<Expression> lhs, Handle<Expression> rhs);
    // create a new node representing JavaScript assign expression
    Handle<Expression> NewAssignExpression(Handle<Expression> lhs, Handle<Expression> rhs);
    Handle<Expression> NewDeclaration(std::string name, Handle<Expression> init = nullptr);
    Handle<Expression> NewDeclaration(Handle<Expression> expr, Handle<Expression> init = nullptr);
    Handle<Expression> NewIntegralConstant(int value);
    Handle<Expression> NewDoubleConstant(double value);
    // create a new node representing JavaScript prefix operation
    Handle<Expression> NewPrefixExpression(PrefixOperation op, Handle<Expression> expr);
    // create a new node representing JavaScript postfix operation
    Handle<Expression> NewPostfixExpression(PostfixOperation op, Handle<Expression> expr);
    // create a new node representing JavaScript `new` expression
    Handle<Expression> NewNewExpression(Handle<Expression> expr, Handle<ExpressionList> args = nullptr);
    // create a new node representing JavaScript Identifier
    Handle<Expression> NewIdentifier(std::string name);
    // create a new node representing JavaScript argument list
    Handle<Expression> NewArgumentList(Handle<ExpressionList> arg);
    // create a new node representing JavaScript member call
    Handle<Expression> NewCallExpression(MemberAccessKind kind, Handle<Expression> func, Handle<Expression> args);
    // create a new node representing JavaScript member call
    Handle<Expression> NewCallExpression(Handle<Expression> callee, Handle<Expression> args_expr);
    // create a new node representing JavaScript member expression
    Handle<Expression> NewMemberExpression(MemberAccessKind kind, Handle<Expression> expr, Handle<Expression> args);
    // create a new node representing JavaScript Comma Expression
    Handle<Expression> NewCommaExpression(Handle<ExpressionList> list);
    // create a new node representing JavaScript 'null'
    Handle<Expression> NewNullConstant();
    // create a new node representing JavaScript string
    Handle<Expression> NewStringConstant(const std::string &str);

    Handle<Expression> NewRegexConstant(const std::string &str);
    // create a new node representing JavaScript boolean
    Handle<Expression> NewBooleanConstant(bool value);
    // create a new node representing JavaScript 'this'
    Handle<Expression> NewThisExpression();
    // create a new node representing JavaScript array
    // after passing `arr` to this function, your arr becomes unusable
    Handle<Expression> NewArrayConstant(ProxyArray arr);
    // create a new node representing JavaScript object
    Handle<Expression> NewObjectConstant(ProxyObject obj);
    // create a new node representing JavaScript FunctionPrototype
    Handle<Expression> NewFunctionPrototype(std::string name, std::vector<std::string> args);
    // create a new node representing JavaScript function statement
    // and fuunction expression
    Handle<Expression> NewFunctionStatement(Handle<FunctionPrototype> proto, Handle<Expression> body);
    // create a new node representing JavaScript block statement
    Handle<Expression> NewBlockStatement(Handle<ExpressionList> stmts);
    // create a new node representing JavaScript return statement
    Handle<Expression> NewReturnStatement(Handle<Expression> ret);
    // create a new node representing JavaScript if statement
    Handle<Expression> NewIfStatement(Handle<Expression> condition, Handle<Expression> then);
    // create a new node representing JavaScript if else statement
    Handle<Expression> NewIfElseStatement(Handle<Expression> cond, Handle<Expression> then, Handle<Expression> els);
    // create a new node representing JavaScript for loop
    Handle<Expression> NewForStatement(ForKind kind, Handle<Expression> init, Handle<Expression> cond, Handle<Expression> update, Handle<Expression> body);
    // create a new node representing JavaScript break statement
    Handle<Expression> NewCaseStatement(Handle<Expression> test_case,Handle<ExpressionList> expr);
    Handle<Expression> NewSwitchStatement(Handle<Expression> test_value,std::vector<Handle<Expression>> cases);
    Handle<Expression> NewTryCatchStatement(Handle<Expression> try_block,
                                            Handle<Expression> catch_expr, Handle<Expression> catch_block, Handle<Expression> finally);
    Handle<Expression> NewBreakStatement(Handle<Expression> label = nullptr);

    // create a new node representing JavaScript continue statement
    Handle<Expression> NewContinueStatement(Handle<Expression> label = nullptr);
    Handle<Expression> NewArrowFunctionStatement(Handle<Expression> body, std::vector<Handle<Expression>> args);
    Handle<Expression> NewJSXNodeExpression(Handle<Expression> identifier, Handle<Expression> props, Handle<Expression> parent, std::vector<Handle<Expression>> childrens);
    Handle<Expression> NewClassStatement(Handle<Expression> identifier, Handle<Expression> superClass, Handle<Expression> body);
    Handle<Expression> NewClassProperty(std::string name, Handle<Expression> init = nullptr);

    Handle<ClassBody> NewClassBody();

    ASTFactory *factory() { return factory_; }
    SourceLocator *locator() { return locator_; }
    ScopeManager *manager() { return manager_; }

    template <typename T>
    inline Handle<Expression> save(Handle<T> handle) {
        exprs_.push_back(handle);
        return handle;
    }
private:
    ASTFactory *factory_;
    SourceLocator *locator_;
    ParserContext *ctx_;
    ScopeManager *manager_;
    std::list<Handle<Expression>> exprs_;
};

}
}
}

#endif
