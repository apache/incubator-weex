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
#include "core/data_render/ast_visitor.h"

namespace weex {
namespace core {
namespace data_render {

#define DEFINE_ACCEPT(type) \
  void type::Accept(ASTVisitor *v, void *data) { v->Visit(this, data); }

AST_NODE_LIST(DEFINE_ACCEPT)
#undef DEFINE_ACCEPT

const char *type_as_string[(int)ASTNodeType::kNrType] = {
    "kUnknownType",
#define AS_STRING(type) #type,
    AST_NODE_LIST(AS_STRING)
#undef AS_STRING
};

}  // namespace data_render
}  // namespace core
}  // namespace weex
