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

#ifndef DATA_RENDER_RAX_JSX_AST_
#define DATA_RENDER_RAX_JSX_AST_

#include "ast.h"

namespace weex {
namespace core {
namespace data_render {
    
#define JS_GLOBAL_VNODE_INDEX                "global_vnode_index"
#define JS_GLOBAL_ARGUMENTS                  "arguments"
#define JS_GLOBAL_CONSTRUCTOR                "constructor"
#define JS_GLOBAL_SUPER                      "super"

class JSXNodeExpression : public Expression {
public:
    JSXNodeExpression(Position &loc, Scope *scope, Handle<Expression> identifier, Handle<Expression> props, Handle<Expression> parent, std::vector<Handle<Expression>> childrens)
    : Expression(loc, scope), identifier_(identifier), parent_(parent),
    props_(props), childrens_{std::move(childrens)} { }
    void SetParent(Handle<Expression> parent) { parent_ = parent; }
    Handle<Expression> &Identifier() { return identifier_; }
    void SetClass(bool is_class) { is_class_ = is_class; }
    Handle<Expression> node_ptr() { return node_ptr_; }
    bool LowerIdentifier();
    std::vector<Handle<Expression>>& funcexprs();
    std::vector<Handle<Expression>>& childrens() { return childrens_; }
    DEFINE_NODE_TYPE(JSXNodeExpression, Expression);
    static uint64_t gs_node_ptr_;
private:
    Handle<Expression> identifier_;
    Handle<Expression> node_ptr_;
    Handle<Expression> parent_{nullptr};
    Handle<Expression> props_;
    std::vector<Handle<Expression>> childrens_;
    std::vector<Handle<Expression>> funcexprs_;
    bool is_class_{false};
};
            
}
}
}

#endif
