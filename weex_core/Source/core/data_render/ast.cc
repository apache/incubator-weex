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
#include "core/data_render/ast.h"

namespace weex {
namespace core {
namespace data_render {
void ConstantNode::Accept(Visitor* node, void* data) {
  node->Visit(this, data);
}
void UnaryExpressionNode::Accept(Visitor* node, void* data) {
  node->Visit(this, data);
}
void BinaryExpressionNode::Accept(Visitor* node, void* data) {
  node->Visit(this, data);
}
void VariableNode::Accept(Visitor* node, void* data) {
  node->Visit(this, data);
}
void DotAccessorNode::Accept(Visitor* node, void* data) {
  node->Visit(this, data);
}
void ExpressionListNode::Accept(Visitor* node, void* data) {
  node->Visit(this, data);
}
void FunctionCallNode::Accept(Visitor* node, void* data) {
  node->Visit(this, data);
}
void ForNode::Accept(Visitor* node, void* data) { node->Visit(this, data); }
void IfElseNode::Accept(Visitor* node, void* data) { node->Visit(this, data); }
void BlockNode::Accept(Visitor* node, void* data) { node->Visit(this, data); }
void ChunkNode::Accept(Visitor* node, void* data) { node->Visit(this, data); }
}  // namespace data_render
}  // namespace core
}  // namespace weex