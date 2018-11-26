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
    BlockStatement(const Position &pos, Scope *scope, Handle<ExpressionList> stmts) : Expression(pos, scope), stmts_{ stmts } { }
    BlockStatement(Handle<ExpressionList> stmts) : Expression(), stmts_{ stmts } { }
    Handle<ExpressionList> statements() { return stmts_; }
    void PushExpression(Handle<Expression> expr);
    DEFINE_NODE_TYPE(BlockStatement, Expression);
private:
    Handle<ExpressionList> stmts_;
};

class ChunkStatement : public BlockStatement {
public:
    ChunkStatement(const Position &pos, Scope *scope, Handle<ExpressionList> stmts) : BlockStatement(pos, scope, stmts) { }
    ChunkStatement(Handle<ExpressionList> stmts) : BlockStatement(stmts) { }
    DEFINE_NODE_TYPE(ChunkStatement, BlockStatement);
private:
};

enum class ForKind {
    kForOf,
    kForIn
};

class ForStatement : public Expression {
public:
    ForStatement(Position &loc, Scope *scope, ForKind kind, Handle<Expression> init,
                 Handle<Expression> condition, Handle<Expression> update, Handle<Expression> body)
    : Expression(loc, scope), kind_{ kind }, init_{ init }, condition_{ condition },
    update_{ update }, body_{ body } { }
    ForStatement(ForKind kind, Handle<Expression> init,
        Handle<Expression> condition, Handle<Expression> update, Handle<Expression> body)
        : Expression(), kind_{ kind }, init_{ init }, condition_{ condition },
          update_{ update }, body_{ body } { }
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

class BreakStatement : public Expression {
public:
    BreakStatement(Position &loc, Scope *scope, Handle<Expression> label = nullptr)
        : Expression(loc, scope), label_{ label } { }
    Handle<Expression> label() { return label_; }
    DEFINE_NODE_TYPE(BreakStatement, Expression);
private:
    Handle<Expression> label_;
};

class ContinueStatement : public Expression {
public:
    ContinueStatement(Position &loc, Scope *scope, Handle<Expression> label = nullptr)
        : Expression(loc, scope), label_{ label } { }
    Handle<Expression> label() { return label_; }
    DEFINE_NODE_TYPE(ContinueStatement, Expression);
private:
    Handle<Expression> label_;
};

// FunctionPrototype - captures the prototype of the function
// which includes the name of the function and
class FunctionPrototype : public Expression {
    DEFINE_NODE_TYPE(FunctionPrototype, Expression);
public:
    FunctionPrototype(Position &loc, Scope *scope, std::string name,
                      std::vector<std::string> args)
    : Expression(loc, scope), name_{ name }, args_{ std::move(args) } { }
    FunctionPrototype(std::string name,
        std::vector<std::string> args)
        : Expression(), name_{ name }, args_{ std::move(args) }
    { }
    void SetClassName(std::string &class_name) { class_name_ = class_name; };
    const std::string &GetName() const;
    const std::string &GetClassName() const;
    const std::vector<std::string> &GetArgs() const;
private:
    std::string class_name_;
    std::string name_;
    std::vector<std::string> args_;
};

class MethodStatement : public Expression {
    DEFINE_NODE_TYPE(MethodStatement, Expression);
public:
    MethodStatement(Position &loc, Scope *scope, Handle<Expression> identifier, Handle<Expression> function)
    : Expression(loc, scope), identifier_{ identifier }, function_{ function } { }
    Handle<Expression> identifier() { return identifier_; }
    Handle<Expression> function() { return function_; }
    void SetStatic(bool is_static) { static_ = is_static; }
    bool Static() { return static_; }
private:
    Handle<Expression> identifier_;
    Handle<Expression> function_;
    bool static_;
};

// FunctionStatement - captures the function statement
class FunctionStatement : public Expression {
    DEFINE_NODE_TYPE(FunctionStatement, Expression);
public:
    FunctionStatement(Position &loc, Scope *scope, Handle<Expression> args, Handle<Expression> body)
    : Expression(loc, scope), args_{ (args) }, body_{ body } { }
    FunctionStatement(Position &loc, Scope *scope, Handle<FunctionPrototype> proto, Handle<Expression> body)
    : Expression(loc, scope), proto_{ (proto) }, body_{ body } { }
    FunctionStatement(Handle<FunctionPrototype> proto, Handle<Expression> body)
        : Expression(), proto_{ (proto) }, body_{ body } { }

    Handle<FunctionPrototype> proto() { return proto_; }
    Handle<Expression> body() { return body_; }
    Handle<Expression> args() { return args_; };
private:
    Handle<Expression> args_{nullptr};
    Handle<FunctionPrototype> proto_{nullptr};
    Handle<Expression> body_;
};

// ArrowFunctionStatement
class ArrowFunctionStatement : public Expression {
    DEFINE_NODE_TYPE(ArrowFunctionStatement, Expression);
public:
    ArrowFunctionStatement(Position &loc, Scope *scope, Handle<Expression> body, std::vector<Handle<Expression>> args)
    : Expression(loc, scope), body_{ body }, args_{std::move(args) } { }
    Handle<Expression> body() { return body_; }
    std::vector<Handle<Expression>>& args() { return args_; }
    std::string &name() { return name_; }
private:
    std::string name_;
    Handle<Expression> body_;
    std::vector<Handle<Expression>> args_;
};

class IfStatement : public Expression {
    DEFINE_NODE_TYPE(IfStatement, Expression);
    using ExprPtr = Handle<Expression>;
public:
    IfStatement(Position &loc, Scope *scope, ExprPtr cond, ExprPtr body)
    : Expression(loc, scope), condition_{ (cond) }, body_{ (body) } { }
    IfStatement(ExprPtr cond, ExprPtr body)
        : Expression(), condition_{ (cond) }, body_{ (body) } { }
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
    IfElseStatement(Position &loc, Scope *scope, ExprPtr cond, ExprPtr body, ExprPtr el)
    : Expression(loc, scope), condition_{ (cond) },
    body_{ (body) },
    else_{ (el) } { }
    IfElseStatement(ExprPtr cond, ExprPtr body, ExprPtr el)
    : Expression(), condition_{ (cond) },
      body_{ (body) },
      else_{ (el) } { }

    Handle<Expression> condition() { return condition_; }
    Handle<Expression> body() { return body_; }
    Handle<Expression> els() { return else_; }
private:
    Handle<Expression> condition_;
    Handle<Expression> body_;
    Handle<Expression> else_;
};

class LabelledStatement : public Expression {
public:
    LabelledStatement(Position &loc, Scope *scope, std::string &label, Handle<Expression> expr)
    : Expression(loc, scope), label_{ label }, expr_{ expr }
    { }

    Handle<Expression> expr() { return expr_; }
    std::string &label() { return label_; }

    DEFINE_NODE_TYPE(LabelledStatement, Expression);
private:
    std::string label_;
    Handle<Expression> expr_;
};

class ReturnStatement : public Expression {
public:
    ReturnStatement(Position &loc, Scope *scope, Handle<Expression> expr)
    : Expression(loc, scope), expr_{ (expr) } { }
    ReturnStatement(Handle<Expression> expr)
        : Expression(), expr_{ (expr) } { }

    Handle<Expression> expr() { return expr_; }

    DEFINE_NODE_TYPE(ReturnStatement, Expression);
private:
    Handle<Expression> expr_;
};

class CaseStatement : public Expression {
 public:
  CaseStatement(Handle<Expression> text_case, Handle<ExpressionList> statements)
      : Expression(), text_case_{(text_case)}, expr_(statements) {
      default_ = false;
  }

  Handle<Expression> test_case() { return text_case_; }
  Handle<ExpressionList> statements() { return expr_; }
  void set_default(bool flag) { default_ = flag; }
  bool is_default() { return default_; }

 DEFINE_NODE_TYPE(CaseStatement, Expression);
 private:
  Handle<Expression> text_case_;
  Handle<ExpressionList> expr_;
  bool default_;
};

class SwitchStatement : public Expression {
 public:
  SwitchStatement(Handle<Expression> test_value,std::vector<Handle<Expression>> cases)
      : Expression(), test_value_{ test_value },cases_ { cases}{}

  Handle<Expression> test_case() { return test_value_; }
  const std::vector<Handle<Expression>>& get_cases(){ return cases_;}

 DEFINE_NODE_TYPE(SwitchStatement, Expression);
 private:
  Handle<Expression> test_value_;
  std::vector<Handle<Expression>> cases_;
};

class ClassStatement : public Expression {
public:
    ClassStatement(Position &loc, Scope *scope, Handle<Expression> identifier, Handle<Expression> super_class, Handle<Expression> body)
    : Expression(loc, scope), identifier_{ identifier }, super_class_{super_class}, body_{body}  { }

    Handle<Expression> &Identifier() { return identifier_; }
    Handle<Expression> &Body () { return body_; }
    Handle<Expression> &Super() { return super_class_; }

    DEFINE_NODE_TYPE(ClassStatement, Expression);
private:
    Handle<Expression> identifier_;
    Handle<Expression> super_class_;
    Handle<Expression> body_;
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif // CORE_DATA_RENDER_STATEMENT_H_
