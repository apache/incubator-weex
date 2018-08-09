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
#ifndef CORE_DATA_RENDER_STATEMENT_H_
#define CORE_DATA_RENDER_STATEMENT_H_

#include "ast.h"
#include <iostream>

namespace weex {
namespace core {
namespace data_render {

///  BlockStatment ::= class representing block statments
class BlockStatement : public Expression {
public:
    BlockStatement(Json &json, Handle<ExpressionList> stmts) : Expression(json), stmts_{ stmts } { }
    Handle<ExpressionList> statements() { return stmts_; }
    void PushExpression(Handle<Expression> expr);
    DEFINE_NODE_TYPE(BlockStatement, Expression);
private:
    Handle<ExpressionList> stmts_;
};

class ChunkStatement : public BlockStatement {
public:
    ChunkStatement(Json &json, Handle<ExpressionList> stmts) : BlockStatement(json, stmts) { }
    DEFINE_NODE_TYPE(ChunkStatement, BlockStatement);
private:
};

enum class ForKind {
    kForOf,
    kForIn
};

class ForStatement : public Expression {
public:
    ForStatement(Json &json, ForKind kind, Handle<Expression> init,
        Handle<Expression> condition, Handle<Expression> update, Handle<Expression> body)
        : Expression(json), kind_{ kind }, init_{ init }, condition_{ condition },
          update_{ update }, body_{ body }
    { }
    Handle<Expression> init() { return init_; }
    Handle<Expression> condition() { return condition_; }
    Handle<Expression> update() { return update_; }
    Handle<Expression> body() { return body_; }

    ForKind kind() { return kind_; }

    DEFINE_NODE_TYPE(ForStatement, Expression);
private:
    ForKind kind_;
    Handle<Expression> init_;
    Handle<Expression> condition_;
    Handle<Expression> update_;
    Handle<Expression> body_;
};

//class BreakStatement : public Expression {
//public:
//    BreakStatement(Json &json, Handle<Expression> label = nullptr)
//        : Expression(json), label_{ label }
//    { }
//
//
//    Handle<Expression> label() { return label_; }
//    DEFINE_NODE_TYPE(BreakStatement);
//private:
//    Handle<Expression> label_;
//};
//
//class ContinueStatement : public Expression {
//public:
//    ContinueStatement(Json &json, Handle<Expression> label = nullptr)
//        : Expression(json), label_{ label }
//    { }
//
//
//    Handle<Expression> label() { return label_; }
//
//    DEFINE_NODE_TYPE(ContinueStatement);
//private:
//    Handle<Expression> label_;
//};
            
// FunctionPrototype - captures the prototype of the function
// which includes the name of the function and
class FunctionPrototype : public Expression {
    DEFINE_NODE_TYPE(FunctionPrototype, Expression);
public:
    FunctionPrototype(Json &json, std::string name,
        std::vector<std::string> args)
        : Expression(json), name_{ name }, args_{ std::move(args) }
    { }
    const std::string &GetName() const;
    const std::vector<std::string> &GetArgs() const;
private:
    std::string name_;
    std::vector<std::string> args_;
};

// FunctionStatement - captures the function statement
class FunctionStatement : public Expression {
    DEFINE_NODE_TYPE(FunctionStatement, Expression);
public:
    FunctionStatement(Json &json,
        Handle<FunctionPrototype> proto, Handle<Expression> body)
        : Expression(json), proto_{ (proto) }, body_{ body }
    { }


    Handle<FunctionPrototype> proto() { return proto_; }
    Handle<Expression> body() { return body_; }
private:
    Handle<FunctionPrototype> proto_;
    Handle<Expression> body_;
};

class IfStatement : public Expression {
    DEFINE_NODE_TYPE(IfStatement, Expression);
    using ExprPtr = Handle<Expression>;
public:
    IfStatement(Json &json, ExprPtr cond, ExprPtr body)
        : Expression(json), condition_{ (cond) }, body_{ (body) }
    { }


    Handle<Expression> condition() { return condition_; }
    Handle<Expression> body() { return body_; }
private:
    Handle<Expression> condition_;
    Handle<Expression> body_;
};

class IfElseStatement : public Expression {
    DEFINE_NODE_TYPE(IfElseStatement, Expression);
    using ExprPtr = Handle<Expression>;
public:
    IfElseStatement(Json &json, ExprPtr cond, ExprPtr body, ExprPtr el)
    : Expression(json), condition_{ (cond) },
      body_{ (body) },
      else_{ (el) }
    { }


    Handle<Expression> condition() { return condition_; }
    Handle<Expression> body() { return body_; }
    Handle<Expression> els() { return else_; }
private:
    Handle<Expression> condition_;
    Handle<Expression> body_;
    Handle<Expression> else_;
};

class ReturnStatement : public Expression {
public:
    ReturnStatement(Json &json, Handle<Expression> expr)
        : Expression(json), expr_{ (expr) }
    { }

    ReturnStatement(Handle<Expression> expr)
        : Expression(), expr_{ (expr) }
    { }

    Handle<Expression> expr() { return expr_; }

    DEFINE_NODE_TYPE(ReturnStatement, Expression);
private:
    Handle<Expression> expr_;
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif // CORE_DATA_RENDER_STATEMENT_H_
