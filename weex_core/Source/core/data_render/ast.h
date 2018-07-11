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

#ifndef WEEX_AST_H_
#define WEEX_AST_H_

#include <memory>
#include <vector>
#include <assert.h>
#include "core/data_render/json/json11.hpp"
#include "core/data_render/handle.h"

using namespace json11;

namespace weex {
namespace core {
namespace data_render {

#ifdef NO_EMIT_FUNCTION
#   define EMIT_FUNCTION
#else
#   define EMIT_FUNCTION
#endif
    
#define DEFINE_NODE_TYPE(Type) \
public: \
friend class ASTVisitor; \
friend class Expression; \
friend class ASTFactory; \
Type(Json &json) \
: Expression(json) \
{ } \
virtual ~Type() = default; \
bool Is##Type() const override { return true; }   \
Handle<Type> As##Type() override \
{ return Handle<Type>(dynamic_cast<Type*>(this)); }  \
ASTNodeType type() const override { return ASTNodeType::k##Type; }  \
void Accept(ASTVisitor *visitor) override; \
protected: \
static Handle<Type> Create(Json json) \
{ return MakeHandle<Type>(json); }
    
#define AST_NODE_LIST(M)    \
M(ArgumentList)         \
M(IfStatement)          \
M(IfElseStatement)      \
M(ForStatement)         \
M(BlockStatement)       \
M(FunctionPrototype)    \
M(FunctionStatement)    \
M(StringLiteral)        \
M(BinaryExpression)     \
M(ChildBlockStatement)  \
M(Declaration)          \
M(DeclarationList)      \
M(IntegralLiteral)      \
M(CallExpression)       \
M(MemberExpression)     \
M(Identifier)           \
M(PrefixExpression)     \
M(ObjectLiteral)        \
M(ArrayLiteral)         \
M(ChunkStatement)       \
M(ReturnStatement)


    class ASTVisitor;
    
#define FORWARD_DECLARE(Type)   \
class Type;
    AST_NODE_LIST(FORWARD_DECLARE)
#undef FORWARD_DECLARE
    
    enum class ASTNodeType {
        kUnknownType,
#define AST_NODE_TYPE(type) k##type,
        AST_NODE_LIST(AST_NODE_TYPE)
#undef AST_NODE_TYPE
        kNrType
    };
    
    extern const char *type_as_string[(int)ASTNodeType::kNrType];
    
    class Expression : public RefCountObject {
    protected:
        Expression(Json &json) : json_(json) {
        };
    public:
        virtual ~Expression() { }
        virtual void Pasing(Json &json) { };
        virtual void Accept(ASTVisitor *visitor) = 0;
        virtual bool ProduceRValue() { return true; }
        virtual ASTNodeType type() const = 0;
        virtual void SetContent(Json &json) { json_ = json; }
        
        // helper conversion functions
#define AS_EXPRESSION_FUNCTION(Type)    \
virtual Handle<Type> As##Type() { assert(0 && "Expression is not " #Type); }
        AST_NODE_LIST(AS_EXPRESSION_FUNCTION)
#undef AS_EXPRESSION_FUNCTION
        
        // helpful for constant folding
#define IS_EXPRESSION_FUNCTION(Type)    \
virtual bool Is##Type() const { return false; }
        AST_NODE_LIST(IS_EXPRESSION_FUNCTION)
#undef IS_EXPRESSION_FUNCTION
        
    private:
        Json json_;
    };
    
    using ProxyArray = std::vector<Handle<Expression>>;
    using ProxyObject = std::map<std::string, Handle<Expression>>;
    
    // ExpressionList ::= helper class representing a list of expressions
    class ExpressionList : public RefCountObject {
    public:
        using iterator = std::vector<Handle<Expression>>::iterator;
        
        ExpressionList()
        : exprs_{ }
        { }
        
        void Insert(Handle<Expression> expr)
        {
            exprs_.push_back(expr);
        }
        
        size_t Size()
        {
            return exprs_.size();
        }
        
        std::vector<Handle<Expression>> &raw_list() { return exprs_; }
        
        ~ExpressionList()
        {
            exprs_.clear();
        }
        
        iterator begin() { return exprs_.begin(); }
        iterator end() { return exprs_.end(); }
    private:
        std::vector<Handle<Expression>> exprs_;
    };
    class ArgumentList : public Expression {
    public:
        ArgumentList(Json &json, Handle<ExpressionList> args)
        : Expression(json), args_{ std::move(args) }
        { }
        
        Handle<ExpressionList> args() { return args_; }
        size_t length() { return args()->Size(); }
        
        DEFINE_NODE_TYPE(ArgumentList);
    private:
        Handle<ExpressionList> args_;
    };
    class Identifier : public Expression {
    private:
        std::string name_;
    public:
        Identifier(Json &json, const std::string &name)
        : Expression(json), name_(name)
        { }
        
        const std::string &GetName() const { return name_; }
        bool ProduceRValue() override { return false; }
        DEFINE_NODE_TYPE(Identifier);
    };
    class StringLiteral : public Expression {
    private:
        std::string str_;
    public:
        StringLiteral(Json &json, const std::string &str)
        : Expression(json), str_(str)
        { }        
        std::string &string() { return str_; }
        DEFINE_NODE_TYPE(StringLiteral);
    };
    enum class BinaryOperation {
        kAddition,
        kSubtraction,
        kMultiplication,
        kDivision,
        kMod,
        kShiftRight,
        kShiftLeft,
        kShiftZeroRight,
        kLessThan,
        kGreaterThan,
        kLessThanEqual,
        kGreaterThanEqual,
        kEqual,
        kNotEqual,
        kStrictEqual,
        kStrictNotEqual,
        kAnd,
        kOr,
        kBitAnd,
        kBitOr,
        kBitXor,
        kInstanceOf,
        kIn
    };
    
    class BinaryExpression : public Expression {
        DEFINE_NODE_TYPE(BinaryExpression);
    public:
        
        BinaryOperation op() { return op_; }
        Handle<Expression> lhs() { return lhs_; }
        Handle<Expression> rhs() { return rhs_; }
    private:
        BinaryOperation op_;
        Handle<Expression> lhs_;
        Handle<Expression> rhs_;
    public:
        BinaryExpression(Json &json, BinaryOperation op,
                         Handle<Expression> lhs, Handle<Expression> rhs)
        : Expression(json), op_(op), lhs_(lhs), rhs_(rhs) { }
    };
    class Declaration : public Expression {
    public:
        Declaration(Json &json, std::string name, Handle<Expression> init)
        : Expression(json), name_{ name }, init_{ init }
        { }        
        std::string &name() { return name_; }
        
        Handle<Expression> expr() { return init_; }
        DEFINE_NODE_TYPE(Declaration);
    private:
        std::string name_;
        Handle<Expression> init_;
    };
    class DeclarationList : public Expression {
    public:
        DeclarationList(Json &json,
                        std::vector<Handle<Declaration>> exprs)
        : Expression(json), exprs_{ std::move(exprs) }
        { }
        
        std::vector<Handle<Declaration>> &exprs() { return exprs_; }
        
        void Append(Handle<Declaration> decl)
        {
            exprs_.push_back(Handle<Declaration>(decl));
        }
        DEFINE_NODE_TYPE(DeclarationList);
    private:
        std::vector<Handle<Declaration>> exprs_;
    };
    class IntegralLiteral : public Expression {
    private:
        int value_;
    public:
        IntegralLiteral(Json &json, int value)
        : Expression(json), value_(value)
        { }
        
        int value() { return value_; }
        DEFINE_NODE_TYPE(IntegralLiteral);
    };
    enum class MemberAccessKind {
        kCall,
        kDot,
        kIndex,
    };
    class MemberExpression : public Expression {
    public:
        MemberExpression(Json &json, MemberAccessKind kind,
                         Handle<Expression> expr, Handle<Expression> member)
        : Expression(json), kind_{ kind }, expr_(expr), member_(member)
        { }
        
        Handle<Expression> member() { return member_; }
        MemberAccessKind kind() { return kind_; }
        
        Handle<Expression> expr() { return expr_; }
        bool ProduceRValue() override { return false; }
        DEFINE_NODE_TYPE(MemberExpression);
    private:
        MemberAccessKind kind_;
        Handle<Expression> expr_;
        Handle<Expression> member_;
    };
    class CallExpression : public Expression {
    public:
        CallExpression(Json &json, MemberAccessKind kind,
                       Handle<Expression> expr, Handle<Expression> member)
        : Expression(json), kind_{ kind }, expr_(expr), member_(member)
        { }
        CallExpression(Json &json, Handle<Expression> callee, std::vector<Handle<Expression>> args)
        : Expression(json), kind_{ MemberAccessKind::kCall }, callee_(callee), args_{ std::move(args) }
        { }
        Handle<Expression> member() { return member_; }
        MemberAccessKind kind() { return kind_; }
        
        Handle<Expression> expr() { return expr_; }
        bool ProduceRValue() override { return false; }
        void InsertArgument(Handle<Expression> arg)
        {
            args_.push_back(arg);
        }
        DEFINE_NODE_TYPE(CallExpression);
    private:
        MemberAccessKind kind_;
        Handle<Expression> expr_;
        Handle<Expression> member_;
        Handle<Expression> callee_;
        std::vector<Handle<Expression>> args_;
    };
    enum class PrefixOperation {
        kIncrement,
        kDecrement,
    };
    
    class PrefixExpression : public Expression {
    public:
        PrefixExpression(Json &json, PrefixOperation op, Handle<Expression> expr)
        : Expression(json), op_{ op }, expr_{ expr }
        { }
        PrefixOperation op() { return op_; }
        Handle<Expression> expr() { return expr_; }
        DEFINE_NODE_TYPE(PrefixExpression);
    private:
        PrefixOperation op_;
        Handle<Expression> expr_;
    };
    class ObjectLiteral : public Expression {
    public:
        ObjectLiteral(Json &json, ProxyObject props)
        : Expression(json), Props{ std::move(props) }
        { }
        
        ProxyObject &proxy() { return Props; }
        bool IsEmpty() { return Props.empty(); }
        ProxyObject::size_type GetPropertyCount() { return Props.size(); }
        
        DEFINE_NODE_TYPE(ObjectLiteral);
    private:
        ProxyObject Props;
    };
    class ArrayLiteral : public Expression {
    public:
        ArrayLiteral(Json &json, ProxyArray exprs)
        : Expression(json), exprs_{ std::move(exprs) }
        { }
        
        ProxyArray &exprs() { return exprs_; }
        
        typename ProxyArray::size_type length() { return exprs_.size(); }
        
        DEFINE_NODE_TYPE(ArrayLiteral);
    private:
        ProxyArray exprs_;
    };

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif
