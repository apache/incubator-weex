#ifndef WEEX_STATEMENT_H_
#define WEEX_STATEMENT_H_

#include "ast.h"
#include <iostream>

namespace weex {
namespace core {
namespace data_render {

///  BlockStatment ::= class representing block statments
class BlockStatement : public Expression {
public:
    BlockStatement(Json &json, Handle<ExpressionList> stmts) : Expression(json), stmts_{ stmts } { }
    void Pasing(Json &json) override;
    Handle<ExpressionList> statements() { return stmts_; }
    void PushExpression(Handle<Expression> expr);
    DEFINE_NODE_TYPE(BlockStatement);
private:
    Handle<ExpressionList> stmts_;
};

class ChunkStatement : public Expression {
public:
    ChunkStatement(Json &json, Handle<ExpressionList> stmts) : Expression(json), stmts_{ stmts } { }
    void Pasing(Json &json) override;
    Handle<ExpressionList> statements() { return stmts_; }
    void PushExpression(Handle<Expression> expr);
    DEFINE_NODE_TYPE(ChunkStatement);
private:
    Handle<ExpressionList> stmts_;
};

class ChildBlockStatement : public Expression {
public:
    ChildBlockStatement(Json &json, Handle<ExpressionList> stmts, std::string parent) : Expression(json), stmts_{ stmts }, parent_{ parent } { };
    void Pasing(Json &json) override;
    Handle<ExpressionList> statements() { return stmts_; }
    void PushExpression(Handle<Expression> expr);
    DEFINE_NODE_TYPE(ChildBlockStatement);
private:
    Handle<ExpressionList> stmts_;
    std::string parent_;
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

    DEFINE_NODE_TYPE(ForStatement);
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
    DEFINE_NODE_TYPE(FunctionPrototype);
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
    DEFINE_NODE_TYPE(FunctionStatement);
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
    DEFINE_NODE_TYPE(IfStatement);
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
    DEFINE_NODE_TYPE(IfElseStatement);
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

    Handle<Expression> expr() { return expr_; }

    DEFINE_NODE_TYPE(ReturnStatement);
private:
    Handle<Expression> expr_;
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif
