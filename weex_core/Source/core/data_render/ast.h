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

#ifndef CORE_DATA_RENDER_AST_H_
#define CORE_DATA_RENDER_AST_H_

#include <assert.h>
#include <memory>
#include <vector>
#include "core/data_render/handle.h"
#include "core/data_render/rax_parser_scope.h"
#include "core/data_render/token.h"
#include "third_party/json11/json11.hpp"

using namespace json11;

namespace weex {
namespace core {
namespace data_render {

#ifdef NO_EMIT_FUNCTION
#define EMIT_FUNCTION
#else
#define EMIT_FUNCTION
#endif

#define DEFINE_NODE_TYPE(Type, Inheritor)                            \
 public:                                                             \
  friend class ASTVisitor;                                           \
  friend class Expression;                                           \
  friend class ASTFactory;                                           \
  Type(const Position &pos, Scope *scope) : Inheritor(pos, scope) {} \
  Type() : Inheritor() {}                                            \
  virtual ~Type() = default;                                         \
  bool Is##Type() const override { return true; }                    \
  Handle<Type> As##Type() override {                                 \
    return Handle<Type>(dynamic_cast<Type*>(this));                  \
  }                                                                  \
  ASTNodeType type() const override { return ASTNodeType::k##Type; } \
  void Accept(ASTVisitor* visitor, void* data) override;             \
                                                                     \
 protected:                                                          \
  static Handle<Type> Create() { return MakeHandle<Type>(); }        \
  static Handle<Type> Create(const Position &pos, Scope *scope)      \
    { return MakeHandle<Type>(pos, scope); }

#define AST_NODE_LIST(M) \
  M(ArgumentList)        \
  M(IfStatement)         \
  M(IfElseStatement)     \
  M(ForStatement)        \
  M(ContinueStatement)   \
  M(BreakStatement)      \
  M(CaseStatement)       \
  M(SwitchStatement)     \
  M(TryCatchStatement)   \
  M(BlockStatement)      \
  M(FunctionPrototype)   \
  M(FunctionStatement)   \
  M(StringConstant)      \
  M(RegexConstant)       \
  M(BinaryExpression)    \
  M(TernaryExpression)   \
  M(AssignExpression)    \
  M(Declaration)         \
  M(DeclarationList)     \
  M(IntegralConstant)    \
  M(DoubleConstant)      \
  M(BooleanConstant)     \
  M(NullConstant)        \
  M(CallExpression)      \
  M(MemberExpression)    \
  M(Identifier)          \
  M(PrefixExpression)    \
  M(PostfixExpression)   \
  M(ObjectConstant)      \
  M(ArrayConstant)       \
  M(UndefinedConstant)   \
  M(ChunkStatement)      \
  M(ReturnStatement)     \
  M(LabelledStatement)   \
  M(ClassStatement)      \
  M(MethodStatement)     \
  M(ThisExpression)      \
  M(CommaExpression)     \
  M(JSXNodeExpression)    \
  M(NewExpression)        \
  M(ArrowFunctionStatement) \
  M(ClassBody)              \
  M(ClassProperty)          \
  M(ExpressionList)

class ASTVisitor;

#define FORWARD_DECLARE(Type) class Type;

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
  Expression(const Position &loc, Scope *scope) : locator_{ loc }, scope_{ scope }
    { }
  Expression() {};

 public:
  virtual ~Expression() {}
  virtual void Accept(ASTVisitor* visitor, void* data) = 0;
  virtual bool ProduceRValue() { return true; }
  virtual ASTNodeType type() const = 0;
  virtual void SetScope(Scope *scope) { scope_ = scope; }
  virtual Scope *GetScope() { return scope_; }

  // helper conversion functions
#define AS_EXPRESSION_FUNCTION(Type) \
  virtual Handle<Type> As##Type() { assert(0 && "Expression is not " #Type); }
  AST_NODE_LIST(AS_EXPRESSION_FUNCTION)
#undef AS_EXPRESSION_FUNCTION

  // helpful for constant folding
#define IS_EXPRESSION_FUNCTION(Type) \
  virtual bool Is##Type() const { return false; }
  AST_NODE_LIST(IS_EXPRESSION_FUNCTION)
#undef IS_EXPRESSION_FUNCTION

 private:
  Scope *scope_;
  Position locator_;
};

using ProxyArray = std::vector<Handle<Expression>>;
using ProxyObject = std::map<std::string, Handle<Expression>>;

// ExpressionList ::= helper class representing a list of expressions
class ExpressionList : public Expression {
 public:
    ExpressionList(const std::vector<Handle<Expression>> &exprs)
    : Expression(), exprs_{std::move(exprs)} {}
  using iterator = std::vector<Handle<Expression>>::iterator;

  void Insert(Handle<Expression> expr) { exprs_.push_back(expr); }

  size_t Size() { return exprs_.size(); }

  std::vector<Handle<Expression>>& raw_list() { return exprs_; }

  iterator begin() { return exprs_.begin(); }
  iterator end() { return exprs_.end(); }

  DEFINE_NODE_TYPE(ExpressionList, Expression);

 private:
  std::vector<Handle<Expression>> exprs_;
};

class ClassBody : public Expression {
public:
    using iterator = std::vector<Handle<Expression>>::iterator;

    void Insert(Handle<Expression> expr) { body_.push_back(expr); }

    size_t Size() { return body_.size(); }

    std::vector<Handle<Expression>>& raw_list() { return body_; }

    iterator begin() { return body_.begin(); }
    iterator end() { return body_.end(); }
    DEFINE_NODE_TYPE(ClassBody, Expression);
private:
    std::vector<Handle<Expression>> body_;
};

class ArgumentList : public Expression {
 public:
  ArgumentList(Handle<ExpressionList> args)
    : Expression(), args_{std::move(args)} {}
  ArgumentList(Position &loc, Scope *scope, Handle<ExpressionList> args)
    : Expression(loc, scope), args_{ std::move(args) } { }
  Handle<ExpressionList> args() { return args_; }
    size_t length() { return args() ? args()->Size() : 0; }

  DEFINE_NODE_TYPE(ArgumentList, Expression);

 private:
  Handle<ExpressionList> args_;
};

class Identifier : public Expression {
 private:
  std::string name_;

 public:
  Identifier(const std::string& name)
      : Expression(), name_(name) {}
  Identifier(Position &loc, Scope *scope, const std::string &name)
    : Expression(loc, scope), name_(name) { }
  const std::string& GetName() const { return name_; }
  bool ProduceRValue() override { return false; }
  DEFINE_NODE_TYPE(Identifier, Expression);
};

class StringConstant : public Expression {
 private:
  std::string str_;

 public:
  StringConstant(Position &loc, Scope *scope, const std::string &str)
    : Expression(loc, scope), str_(str)
    { }
  StringConstant(const std::string& str)
      : Expression(), str_(str) {}
  std::string& string() { return str_; }
  DEFINE_NODE_TYPE(StringConstant, Expression);
};

class RegexConstant : public Expression {
 private:
  std::string str_;
  std::string str_flag_;

 public:
  RegexConstant(Position &loc, Scope *scope, const std::string &str)
    : Expression(loc, scope)
    {
      auto p = str.find_last_of('$');
      if (p==std::string::npos){
        str_ = str;
      } else {
        str_ = str.substr(0,p);
        str_flag_ = str.substr(p+1);
      }
    }
  RegexConstant(const std::string& str)
      : Expression() {
    auto p = str.find_last_of('$');
    if (p==std::string::npos){
      str_ = str;
    } else {
      str_ = str.substr(0,p);
      str_flag_ = str.substr(p+1);
    }
  }
  std::string& reg() { return str_; }
  std::string& flag() { return str_flag_; }
  DEFINE_NODE_TYPE(RegexConstant, Expression);
};

class TernaryExpression : public Expression {
 public:
  TernaryExpression(Position &loc, Scope *scope, Handle<Expression> first,
                      Handle<Expression> second, Handle<Expression> third)
    : Expression(loc, scope), first_(first), second_(second),
    third_(third) { }
  TernaryExpression(Handle<Expression> first,
                    Handle<Expression> second, Handle<Expression> third)
      : Expression(), first_(first), second_(second),
        third_(third) {}

  Handle<Expression> first() { return first_; }
  Handle<Expression> second() { return second_; }
  Handle<Expression> third() { return third_; }
  DEFINE_NODE_TYPE(TernaryExpression, Expression);

 private:
  Handle<Expression> first_;
  Handle<Expression> second_;
  Handle<Expression> third_;
};

enum class BinaryOperation {
  kUnknown,
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
  DEFINE_NODE_TYPE(BinaryExpression, Expression);

 public:
  BinaryExpression(Position &loc, Scope *scope, BinaryOperation op,
                     Handle<Expression> lhs, Handle<Expression> rhs)
    : Expression(loc, scope), op_(op), lhs_(lhs), rhs_(rhs) { }
  BinaryExpression(BinaryOperation op, Handle<Expression> lhs,
                   Handle<Expression> rhs)
      : Expression(), op_(op), lhs_(lhs), rhs_(rhs) {}


  BinaryOperation op() { return op_; }
  Handle<Expression> lhs() { return lhs_; }
  Handle<Expression> rhs() { return rhs_; }

 private:
  BinaryOperation op_;
  Handle<Expression> lhs_;
  Handle<Expression> rhs_;
};

enum class DeclarationKind {
    kConst,
};

class Declaration : public Expression {
 public:
  Declaration(Position &loc, Scope *scope, std::string name, Handle<Expression> init)
    : Expression(loc, scope), name_{ name }, init_{ init } { }
  Declaration(Position &loc, Scope *scope, Handle<Expression> expr, Handle<Expression> init)
    : Expression(loc, scope), expr_{ expr }, init_{ init } { }
  Declaration(std::string name, Handle<Expression> init)
      : Expression(), name_{name}, init_{init} {}
  std::string& name() { return name_; }
  void SetKind(DeclarationKind kind) { kind_ = kind; }
  Handle<Expression> expr() { return init_; }
  DEFINE_NODE_TYPE(Declaration, Expression);

 private:
  Handle<Expression> expr_;
  std::string name_;
  Handle<Expression> init_;
  DeclarationKind kind_;
};

class DeclarationList : public Expression {
 public:
  DeclarationList(Position &loc, Scope *scope,
                    std::vector<Handle<Declaration>> exprs)
    : Expression(loc, scope), exprs_{ std::move(exprs) } { }
  DeclarationList(std::vector<Handle<Declaration>> exprs)
      : Expression(), exprs_{std::move(exprs)} {}

  std::vector<Handle<Declaration>>& exprs() { return exprs_; }

  void Append(Handle<Declaration> decl) {
    exprs_.push_back(Handle<Declaration>(decl));
  }
  DEFINE_NODE_TYPE(DeclarationList, Expression);

 private:
  std::vector<Handle<Declaration>> exprs_;
};

class IntegralConstant : public Expression {
 public:
  IntegralConstant(Position &loc, Scope *scope, int value)
    : Expression(loc, scope), value_(value)
    { }
  IntegralConstant(int value) : Expression(), value_(value) {}

  int value() { return value_; }
  DEFINE_NODE_TYPE(IntegralConstant, Expression);

 private:
  int value_;
};

class DoubleConstant : public Expression {
 public:
  DoubleConstant(Position &loc, Scope *scope, double value)
  : Expression(loc, scope), value_(value)
      { }
  DoubleConstant(double value) : Expression(), value_(value) {}
  double value() { return value_; }
  DEFINE_NODE_TYPE(DoubleConstant, Expression);

 private:
  double value_;
};

class BooleanConstant : public Expression {
  bool pred_;

 public:
  BooleanConstant(Position &loc, Scope *scope, bool val)
    : Expression(loc, scope), pred_(val) { }
  BooleanConstant(bool val)
      : Expression(), pred_(val) {}

  bool pred() { return pred_; }
  DEFINE_NODE_TYPE(BooleanConstant, Expression);
};

class NullConstant : public Expression {
 public:
 DEFINE_NODE_TYPE(NullConstant, Expression);
};

enum class MemberAccessKind {
  kCall,
  kClass,
  kDot,
  kIndex,
};

class MemberExpression : public Expression {
 public:
  MemberExpression(Position &loc, Scope *scope, MemberAccessKind kind,
                     Handle<Expression> expr, Handle<Expression> member)
    : Expression(loc, scope), kind_{ kind }, expr_(expr), member_(member) { }
  MemberExpression(MemberAccessKind kind, Handle<Expression> expr,
                   Handle<Expression> member)
      : Expression(), kind_{kind}, expr_(expr), member_(member) {}

  Handle<Expression> member() { return member_; }
  MemberAccessKind kind() { return kind_; }
  void setKind(MemberAccessKind kind) { kind_ = kind; }
  Handle<Expression> expr() { return expr_; }
  inline bool& is_assignment() { return is_assignment_; }
  bool ProduceRValue() override { return !is_assignment_; }
  DEFINE_NODE_TYPE(MemberExpression, Expression);

 private:
  MemberAccessKind kind_;
  Handle<Expression> expr_;
  Handle<Expression> member_;
  bool is_assignment_{false};
};

class CallExpression : public Expression {
 public:
  CallExpression(Position &loc, Scope *scope, MemberAccessKind kind,
                   Handle<Expression> expr, Handle<Expression> args_expr)
    : Expression(loc, scope), kind_{ kind }, expr_(expr), args_expr_(args_expr)
    { }
  CallExpression(Position &loc, Scope *scope, Handle<Expression> callee, Handle<Expression> args_expr)
    : Expression(loc, scope), kind_{MemberAccessKind::kCall}, callee_(callee), args_expr_(args_expr)
    { }
  CallExpression(MemberAccessKind kind, Handle<Expression> expr, Handle<Expression> member, std::vector<Handle<Expression>> args)
      : Expression(), kind_{kind}, expr_(expr), member_(member), args_{std::move(args)} {}
  CallExpression(Handle<Expression> callee, std::vector<Handle<Expression>> args)
      : Expression(), kind_{MemberAccessKind::kCall}, callee_(callee), args_{std::move(args)} {}
  Handle<Expression> member() { return member_; }
  Handle<Expression> callee() { return callee_; }
  Handle<Expression> args_expr() { return args_expr_; }
  std::vector<Handle<Expression>>& args() { return args_; }
  MemberAccessKind kind() { return kind_; }
  Handle<Expression> expr() { return expr_; }
  bool ProduceRValue() override { return false; }
  void InsertArgument(Handle<Expression> arg) { args_.push_back(arg); }
  DEFINE_NODE_TYPE(CallExpression, Expression);

 private:
  MemberAccessKind kind_;
  Handle<Expression> expr_{nullptr};
  Handle<Expression> member_{nullptr};
  Handle<Expression> callee_{nullptr};
  Handle<Expression> args_expr_{nullptr};
  std::vector<Handle<Expression>> args_;
};

enum class PrefixOperation {
  kUnknown,
  kIncrement,
  kDecrement,
  kNot,
  kUnfold,
  kTypeof,
};

class PrefixExpression : public Expression {
 public:
  PrefixExpression(Position &loc, Scope *scope, PrefixOperation op, Handle<Expression> expr)
    : Expression(loc, scope), op_{ op }, expr_{ expr } { }
  PrefixExpression(PrefixOperation op, Handle<Expression> expr)
      : Expression(), op_{op}, expr_{expr} {}
  PrefixOperation op() { return op_; }
  Handle<Expression> expr() { return expr_; }
  DEFINE_NODE_TYPE(PrefixExpression, Expression);

 private:
  PrefixOperation op_;
  Handle<Expression> expr_;
};

enum class PostfixOperation { kIncrement, kDecrement };

class PostfixExpression : public Expression {
 public:
  PostfixExpression(Position &loc, Scope *scope, PostfixOperation op, Handle<Expression> expr)
    : Expression(loc, scope), op_{ op }, expr_{ expr } { }
  PostfixExpression(PostfixOperation op, Handle<Expression> expr)
      : Expression(), op_{op}, expr_{expr} {}

  PostfixOperation op() { return op_; }
  Handle<Expression> expr() { return expr_; }
  DEFINE_NODE_TYPE(PostfixExpression, Expression);

 private:
  PostfixOperation op_;
  Handle<Expression> expr_;
};

enum class ProxyOrder { ProxyArray, ProxyObject };

class ObjectConstant : public Expression {
 public:
  ObjectConstant(Position &loc, Scope *scope, ProxyObject props)
    : Expression(loc, scope), Props{ std::move(props) }
    { }
  ObjectConstant(ProxyObject props)
      : Expression(), Props{std::move(props)} {}

  ProxyObject& proxy() { return Props; }
  bool IsEmpty() { return Props.empty(); }
  ProxyArray &SpreadProperty() { return spread_property_; };
  std::vector<std::pair<ProxyOrder, std::string>> &Orders() { return orders_; } ;
  ProxyObject::size_type GetPropertyCount() { return Props.size(); }

  DEFINE_NODE_TYPE(ObjectConstant, Expression);

 private:
  ProxyObject Props;
  ProxyArray spread_property_;
  std::vector<std::pair<ProxyOrder, std::string>> orders_;
};

class ArrayConstant : public Expression {
 public:
  ArrayConstant(Position &loc, Scope *scope, ProxyArray exprs)
    : Expression(loc, scope), exprs_{ std::move(exprs) } { }
  ArrayConstant(ProxyArray exprs)
      : Expression(), exprs_{std::move(exprs)} {}

  ProxyArray& exprs() { return exprs_; }

  typename ProxyArray::size_type length() { return exprs_.size(); }

  DEFINE_NODE_TYPE(ArrayConstant, Expression);

 private:
  ProxyArray exprs_;
};

enum class AssignOperation {
    kAssign,
    kAssignAdd,
    kAssignSub
};

class AssignExpression : public Expression {
 public:
  AssignExpression(Position &loc, Scope *scope, Handle<Expression> lhs, Handle<Expression> rhs)
    : Expression(loc, scope), lhs_(lhs), rhs_(rhs) { }
  AssignExpression(Handle<Expression> lhs, Handle<Expression> rhs)
      : Expression(), lhs_(lhs), rhs_(rhs) {}
  Handle<Expression> lhs() { return lhs_; }
  Handle<Expression> rhs() { return rhs_; }
  AssignOperation& op() { return op_; }
  DEFINE_NODE_TYPE(AssignExpression, Expression);

 private:
  Handle<Expression> lhs_;
  Handle<Expression> rhs_;
  AssignOperation op_{AssignOperation::kAssign};
};

class UndefinedConstant : public Expression {
public:
    UndefinedConstant(Position &loc, Scope *scope)
    : Expression(loc, scope)
    { }

    DEFINE_NODE_TYPE(UndefinedConstant, Expression);
};

class NewExpression : public Expression {
public:
    NewExpression(Position &loc, Scope *scope, Handle<Expression> name, Handle<ExpressionList> args = nullptr)
    : Expression(loc, scope), name_(name), args_(args) { }
    NewExpression(Handle<Expression> name, Handle<ExpressionList> args = nullptr)
    : Expression(), name_(name), args_(args) { }

    Handle<Expression> name() { return name_; }
    Handle<ExpressionList> args() { return args_; }
    bool ProduceRValue() override { return false; }
    DEFINE_NODE_TYPE(NewExpression, Expression);
private:
    Handle<Expression> name_;
    Handle<ExpressionList> args_;
};

class ThisExpression : public Expression {
public:
    ThisExpression(Position &loc, Scope *scope)
    : Expression(loc, scope)
    { }

    bool ProduceRValue() override { return false; }
    DEFINE_NODE_TYPE(ThisExpression, Expression);
};

class CommaExpression : public Expression {
public:
    CommaExpression(Handle<ExpressionList> exprs)
    : Expression(), exprs_(exprs) {}
    CommaExpression(Position &loc, Scope *scope, Handle<ExpressionList> exprs)
    : Expression(loc, scope), exprs_{ exprs }
    { }

    Handle<ExpressionList> exprs() { return exprs_; }
    DEFINE_NODE_TYPE(CommaExpression, Expression);
private:
    Handle<ExpressionList> exprs_;
};
    
class ClassProperty : public Expression {
public:
    ClassProperty(Position &loc, Scope *scope, std::string name, Handle<Expression> init)
    : Expression(loc, scope), name_(name), init_{init} { }
    inline void set_is_static(bool is_static) { is_static_ = is_static; }
    inline bool is_static() { return is_static_; }
    inline std::string &name() { return name_; }
    inline Handle<Expression>& init() { return init_; }
    DEFINE_NODE_TYPE(ClassProperty, Expression);
private:
    std::string name_;
    Handle<Expression> init_;
    bool is_static_{false};
};

class TryCatchStatement : public Expression {
 public:
  TryCatchStatement(Handle<Expression> try_block,
                    Handle<Expression> catch_expr, Handle<Expression> catch_block, Handle<Expression> finally)
      : Expression(), try_block_{ try_block }, catch_expr_{ catch_expr },
        catch_block_{ catch_block }, finally_{ finally }
  { }


  Handle<Expression> try_block() { return try_block_; }
  Handle<Expression> catch_expr() { return catch_expr_; }
  Handle<Expression> catch_block() { return catch_block_; }
  Handle<Expression> finally() { return finally_; }
  DEFINE_NODE_TYPE(TryCatchStatement,Expression);
 private:
  Handle<Expression> try_block_;
  Handle<Expression> catch_expr_;
  Handle<Expression> catch_block_;
  Handle<Expression> finally_;
};


}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_AST_H_
