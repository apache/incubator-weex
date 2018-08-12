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
// Created by pengtao.pt on 2018/7/25.
//

#include "core/data_render/ast.h"
#include "core/data_render/ast_factory.h"

namespace weex {
namespace core {
namespace data_render {

uint64_t JSXNodeExpression::s_node_id_ = 0;

bool JSXNodeExpression::LowerIdentifier() {
    std::string name = Identifier()->AsIdentifier()->GetName();
    return name[0] == tolower(name[0]) ? true : false;
}

std::vector<Handle<Expression>>& JSXNodeExpression::funcexprs() {
    if (!funcexprs_.size()) {
        ASTFactory *factory = ASTFactory::GetFactoryInstance();
        std::string name = Identifier()->AsIdentifier()->GetName();
        if (LowerIdentifier()) {
            std::vector<Handle<Expression>> args;
            Handle<Expression> create_element_func_expr = factory->NewIdentifier("createElement");
            args.push_back(factory->NewStringConstant(name));
            node_id_ = factory->NewStringConstant("vn_" + std::to_string(s_node_id_++));
            args.push_back(node_id_);
            funcexprs_.push_back(factory->NewDeclaration("vnode_ptr", factory->NewCallExpression(create_element_func_expr, args)));
            if (props_) {
                std::vector<Handle<Expression>> args;
                Handle<Expression> set_props_func_expr = factory->NewIdentifier("setProps");
                args.push_back(factory->NewIdentifier("vnode_ptr"));
                args.push_back(props_);
                funcexprs_.push_back(factory->NewCallExpression(set_props_func_expr, args));
            }
            if (childrens_.size() > 0) {
                
            }
        }
        else if (!is_class_) {
            Handle<Expression> call_func_expr = factory->NewIdentifier(name);
            std::vector<Handle<Expression>> args;
            if (props_) {
                args.push_back(props_);
            }
            else {
                args.push_back(factory->NewNullConstant());
            }
            funcexprs_.push_back(factory->NewCallExpression(call_func_expr, args));
        }
        else {
            funcexprs_.push_back(factory->NewDeclaration("vnode_ptr", factory->NewNewExpression(identifier_)));
            // call constructor
            std::vector<Handle<Expression>> args;
            if (props_) {
                args.push_back(props_);
            }
            else {
                ProxyObject proxy;
                args.push_back(factory->NewObjectConstant(proxy));
            }
            funcexprs_.push_back(factory->NewCallExpression(MemberAccessKind::kCall, factory->NewIdentifier("vnode_ptr"), factory->NewIdentifier("constructor"), args));
        }
    }
    return funcexprs_;
}

}
}
}
