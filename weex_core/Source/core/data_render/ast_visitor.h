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
#ifndef CORE_DATA_RENDER_AST_VISITOR_H_
#define CORE_DATA_RENDER_AST_VISITOR_H_

#include "ast.h"
#include "core/data_render/statement.h"
#include "core/data_render/rax_jsx_ast.h"

namespace weex {
namespace core {
namespace data_render {

// More strict visitor which wants you to override every visit member
class BasicASTVisitor {
 public:
  BasicASTVisitor() = default;
  virtual ~BasicASTVisitor() = default;

 protected:
#define DECLARE_VISITOR_METHOD(type) virtual void Visit(type *, void *data) = 0;
  AST_NODE_LIST(DECLARE_VISITOR_METHOD)
#undef DECLARE_VISITOR_METHOD
};

// Lesser strict version of above visitor
class ASTVisitor : public BasicASTVisitor {
 public:
  void visit(Handle<Expression> expr, void *data) { expr->Accept(this, data); }
#define DECLARE_VISITOR_METHOD(type)                               \
  void Visit(type *, void *data) override {                        \
    throw std::runtime_error("Not implemented walker for " #type); \
  }
  AST_NODE_LIST(DECLARE_VISITOR_METHOD)
#undef DECLARE_VISITOR_METHOD
};

template <typename NodeType>
class SingleNodeVisitor {
 public:
  void Visit(NodeType *node){};
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_AST_VISITOR_H_
