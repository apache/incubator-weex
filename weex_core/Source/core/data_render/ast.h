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

#include <memory>
#include <vector>
#include "core/data_render/token.h"

namespace weex {
namespace core {
namespace data_render {

class Visitor;

class ASTNode {
 public:
  ASTNode(const Token& token) : token_(token) {}
  virtual ~ASTNode() {}
  virtual void Accept(Visitor* visitor, void* data) = 0;
  inline Token& token() { return token_; }

 protected:
  Token token_;
};

class StatementNode : public ASTNode {
 public:
  StatementNode(const Token& token) : ASTNode(token) {}
  virtual bool IsReturn() const { return false; }
  virtual bool IsBreak() const { return false; }
  virtual bool IsContinue() const { return false; }
  virtual bool IsBlock() const { return false; }
};

class ExpressionNode : public ASTNode {
 public:
  ExpressionNode(const Token& token) : ASTNode(token) {}

  virtual bool IsInteger() const { return false; }
  virtual bool IsNumber() const { return false; }
  virtual bool IsString() const { return false; }
  virtual bool IsVariable() const { return false; }
  virtual bool IsNull() const { return false; }
  virtual bool IsUndefined() const { return false; }
  virtual bool IsConstant() const { return false; }
  virtual bool IsDotAccessorNode() const { return false; }
  virtual bool IsFuncExprNode() const { return false; }
  virtual bool IsBool() const { return false; }
};

class ConstantNode : public ExpressionNode {
 public:
  ConstantNode(const Token& token) : ExpressionNode(token) {}
  virtual void Accept(Visitor* visitor, void* data);
  bool IsConstant() const { return true; }
};

class BoolNode : public ConstantNode {
 public:
  BoolNode(const Token& token) : ConstantNode(token) {}
  inline bool value() { return value_; }
  bool IsBool() const { return true; }

 private:
  bool value_;
};

class NumberNode : public ConstantNode {
 public:
  NumberNode(const Token& token, double value)
      : ConstantNode(token), value_(value) {}
  inline double value() { return value_; }
  bool IsNumber() const { return true; }

 private:
  double value_;
};

class IntegerNode : public NumberNode {
 public:
  IntegerNode(const Token& token, int value) : NumberNode(token, value) {}
};

class StringNode : public ConstantNode {
 public:
  StringNode(const Token& token, const std::string& value)
      : ConstantNode(token), value_(value) {}
  inline std::string value() { return value_; }
  bool IsString() const { return true; }

 private:
  std::string value_;
};

class NullNode : public ConstantNode {
 public:
  NullNode(const Token& token) : ConstantNode(token) {}
  virtual bool IsNull() const { return true; }
};

class UndefinedNode : public ConstantNode {
 public:
  UndefinedNode(const Token& token) : ConstantNode(token) {}
  virtual bool IsUndefined() const { return false; }
};

class UnaryExpressionNode : public ExpressionNode {
 public:
  UnaryExpressionNode(const Token& token, Token::Type op_type,
                      ExpressionNode* expression)
      : ExpressionNode(token), op_type_(op_type), expression_(expression) {}
  virtual void Accept(Visitor* visitor, void* data);

  Token::Type op_type() { return op_type_; }
  ExpressionNode* expression() { return expression_.get(); }

 private:
  std::unique_ptr<ExpressionNode> expression_;
  Token::Type op_type_;
};

class BinaryExpressionNode : public ExpressionNode {
 public:
  BinaryExpressionNode(const Token& token, ExpressionNode* expression1,
                       ExpressionNode* expression2, Token::Type op_type)
      : ExpressionNode(token),
        expression1_(expression1),
        expression2_(expression2),
        op_type_(op_type) {}
  virtual void Accept(Visitor* visitor, void* data);
  ExpressionNode* lhs() { return expression1_.get(); };
  ExpressionNode* rhs() { return expression2_.get(); };
  Token::Type op_type() { return op_type_; }

 private:
  std::unique_ptr<ExpressionNode> expression1_;
  std::unique_ptr<ExpressionNode> expression2_;
  Token::Type op_type_;
};

class VariableNode : public ExpressionNode {
 public:
  VariableNode(const Token& token, const std::string& identifier)
      : ExpressionNode(token), identifier_(identifier) {}
  virtual void Accept(Visitor* visitor, void* data);
  bool IsVariable() const { return true; }
  inline std::string& identifier() { return identifier_; }

 private:
  std::string identifier_;
};

class AssignmentNode : public ExpressionNode {
 public:
  AssignmentNode(const Token& token, Token::Type assignment,
                 ExpressionNode* expression)
      : ExpressionNode(token),
        assignment_(assignment),
        expression_(expression) {}
  virtual void Accept(Visitor* visitor, void* data);
  inline void set_expression(ExpressionNode* expression) {
    expression_.reset(expression);
  }
  inline Token::Type assignment() { return assignment_; }
  inline ExpressionNode* expression() { return expression_.get(); }

 private:
  std::unique_ptr<ExpressionNode> expression_;
  Token::Type assignment_;
};

class VarDeclareNode : public StatementNode {
 public:
  VarDeclareNode(const Token& token, const std::string& identifier,
                 ExpressionNode* expression)
      : StatementNode(token),
        identifier_(identifier),
        expression_(expression) {}
  virtual void Accept(Visitor* visitor, void* data);
  inline std::string& identifier() { return identifier_; }
  inline ASTNode* expression() { return expression_.get(); }

 private:
  std::unique_ptr<ExpressionNode> expression_;
  std::string identifier_;
};

class DotAccessorNode : public ExpressionNode {
 public:
  DotAccessorNode(const Token& token, ExpressionNode* base,
                  const std::string identifier)
      : ExpressionNode(token), base_(base), identifier_(identifier) {}
  virtual void Accept(Visitor* visitor, void* data);
  bool IsDotAccessorNode() const { return true; }

 private:
  std::unique_ptr<ExpressionNode> base_;
  std::string identifier_;
};

// TODO
class ArrayNode : public ExpressionNode {
 public:
  ArrayNode(const Token& token) : ExpressionNode(token) {}
};

// TODO
class PropertyNode : public ExpressionNode {
 public:
  PropertyNode(const Token& token) : ExpressionNode(token) {}
};

class ExpressionListNode : public ExpressionNode {
 public:
  ExpressionListNode(const Token& token)
      : ExpressionNode(token), expressions_() {}
  virtual void Accept(Visitor* visitor, void* data);
  inline std::vector<std::unique_ptr<ExpressionNode>>& expressions() {
    return expressions_;
  }

 private:
  std::vector<std::unique_ptr<ExpressionNode>> expressions_;
};

class FunctionCallNode : public ExpressionNode {
 public:
  FunctionCallNode(const Token& token, ExpressionNode* caller,
                   ExpressionListNode* arguments)
      : ExpressionNode(token), caller_(caller), arguments_(arguments) {}
  virtual void Accept(Visitor* visitor, void* data);
  bool IsFuncExprNode() const { return true; }
  inline ExpressionNode* caller() { return caller_.get(); }
  inline ExpressionListNode* arguments() { return arguments_.get(); }

 private:
  std::unique_ptr<ExpressionNode> caller_;
  std::unique_ptr<ExpressionListNode> arguments_;
};

class ContinueNode : public StatementNode {
 public:
  ContinueNode(const Token& token) : StatementNode(token) {}
  bool IsContinue() const { return true; }
};

class BreakNode : public StatementNode {
 public:
  BreakNode(const Token& token) : StatementNode(token) {}
  bool IsBreak() const { return true; }
};

class ReturnNode : public StatementNode {
 public:
  ReturnNode(const Token& token) : StatementNode(token) {}
  ASTNode* expression() { return expression_.get(); }
  bool IsReturn() const { return true; }

 private:
  std::unique_ptr<ASTNode> expression_;
};

class ForNode : public StatementNode {
 public:
  ForNode(const Token& token, ExpressionNode* expression1,
          ExpressionNode* expression2, ExpressionNode* expression3,
          StatementNode* statement)
      : StatementNode(token),
        expression1_(expression1),
        expression2_(expression2),
        expression3_(expression3),
        statement_(statement) {}
  virtual void Accept(Visitor* visitor, void* data);

 private:
  std::unique_ptr<ExpressionNode> expression1_;
  std::unique_ptr<ExpressionNode> expression2_;
  std::unique_ptr<ExpressionNode> expression3_;
  std::unique_ptr<StatementNode> statement_;
};

class IfElseNode : public StatementNode {
 public:
  IfElseNode(const Token& token, ExpressionNode* condition,
             StatementNode* if_block, StatementNode* else_blcok)
      : StatementNode(token),
        condition_(condition),
        if_block_(if_block),
        else_block_(else_blcok) {}
  virtual void Accept(Visitor* visitor, void* data);

  inline ExpressionNode* condition() { return condition_.get(); }

  inline StatementNode* if_block() { return if_block_.get(); }

  inline StatementNode* else_block() { return else_block_.get(); }

 private:
  std::unique_ptr<ExpressionNode> condition_;
  std::unique_ptr<StatementNode> if_block_;
  std::unique_ptr<StatementNode> else_block_;
};

class FunctionNode : public StatementNode {
 public:
  FunctionNode(const Token& token, FunctionCallNode* func)
      : StatementNode(token), func_(func) {}
  virtual void Accept(Visitor* visitor, void* data);
  inline FunctionCallNode* func() { return func_.get(); }

 private:
  std::unique_ptr<FunctionCallNode> func_;
};

class BlockNode : public StatementNode {
 public:
  BlockNode(const Token& token) : StatementNode(token), statements_() {}
  virtual void Accept(Visitor* visitor, void* data);
  bool IsBlock() const { return true; }
  std::vector<std::unique_ptr<StatementNode>>& statements() {
    return statements_;
  }

 private:
  std::vector<std::unique_ptr<StatementNode>> statements_;
};

class ChunkNode : public BlockNode {
 public:
  ChunkNode(const Token& token) : BlockNode(token) {}
  virtual void Accept(Visitor* visitor, void* data);
};

class Visitor {
 public:
  virtual void Visit(ConstantNode* node, void* data) = 0;
  virtual void Visit(UnaryExpressionNode* node, void* data) = 0;
  virtual void Visit(BinaryExpressionNode* node, void* data) = 0;
  virtual void Visit(VariableNode* node, void* data) = 0;
  virtual void Visit(AssignmentNode* node, void* data) = 0;
  virtual void Visit(VarDeclareNode* node, void* data) = 0;
  virtual void Visit(DotAccessorNode* node, void* data) = 0;
  virtual void Visit(ExpressionListNode* node, void* data) = 0;
  virtual void Visit(FunctionCallNode* node, void* data) = 0;
  virtual void Visit(ForNode* node, void* data) = 0;
  virtual void Visit(IfElseNode* node, void* data) = 0;
  virtual void Visit(FunctionNode* node, void* data) = 0;
  virtual void Visit(BlockNode* node, void* data) = 0;
  virtual void Visit(ChunkNode* node, void* data) = 0;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_AST_H_