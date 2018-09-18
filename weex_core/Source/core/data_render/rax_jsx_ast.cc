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

#include <sstream>
#include "core/data_render/ast.h"
#include "core/data_render/ast_factory.h"

namespace weex {
namespace core {
namespace data_render {

template <typename T>
    
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

uint64_t JSXNodeExpression::gs_node_ptr_ = 0;

bool JSXNodeExpression::LowerIdentifier() {
    std::string name = Identifier()->AsIdentifier()->GetName();
    return name[0] == tolower(name[0]) ? true : false;
}

std::vector<Handle<Expression>>& JSXNodeExpression::funcexprs() {
    if (!funcexprs_.size()) {
        ASTFactory *factory = ASTFactory::GetFactoryInstance();
        std::string name = Identifier()->AsIdentifier()->GetName();
        std::string vnode_ptr = "vn_" + to_string(gs_node_ptr_++) + "_ptr";
        if (!node_ptr_) {
            node_ptr_ = factory->NewStringConstant(vnode_ptr);
        }
        if (LowerIdentifier()) {
            Handle<Expression> vnode_index_expr = factory->NewIdentifier(JSX_GLOBAL_VNODE_INDEX);
            std::vector<Handle<Expression>> args;
            Handle<Expression> create_element_func_expr = factory->NewIdentifier("createElement");
            args.push_back(factory->NewStringConstant(name));
            args.push_back(vnode_index_expr);
            funcexprs_.push_back(factory->NewDeclaration(vnode_ptr, factory->NewCallExpression(create_element_func_expr, args)));
            funcexprs_.push_back(factory->NewPrefixExpression(PrefixOperation::kIncrement, vnode_index_expr));
            if (props_) {
                std::vector<Handle<Expression>> args;
                Handle<Expression> set_props_func_expr = factory->NewIdentifier("setProps");
                args.push_back(factory->NewIdentifier(vnode_ptr));
                args.push_back(props_);
                funcexprs_.push_back(factory->NewCallExpression(set_props_func_expr, args));
            }
        }
        else if (!is_class_) {
            Handle<Expression> call_func_expr = factory->NewIdentifier(name);
            std::vector<Handle<Expression>> args;
            if (props_) {
                args.push_back(props_);
            }
            else {
                ProxyObject proxy;
                args.push_back(factory->NewObjectConstant(proxy));
            }
            funcexprs_.push_back(factory->NewDeclaration(vnode_ptr,factory->NewCallExpression(call_func_expr, args)));
        }
        else {
            std::string class_inst = vnode_ptr + "_inst";
            funcexprs_.push_back(factory->NewDeclaration(class_inst, factory->NewNewExpression(identifier_)));
            // call constructor
            std::vector<Handle<Expression>> args;
            if (props_) {
                args.push_back(props_);
            }
            else {
                ProxyObject proxy;
                args.push_back(factory->NewObjectConstant(proxy));
            }
            Handle<Expression> class_inst_expr = factory->NewIdentifier(class_inst);
            funcexprs_.push_back(factory->NewCallExpression(MemberAccessKind::kCall, class_inst_expr, factory->NewIdentifier("constructor"), args));
            // if (vnode_ptr.xxxx) then vnode_ptr.xxxx()
            Handle<Expression> render_expr = factory->NewIdentifier("render");
            Handle<MemberExpression> member_func_expr = factory->NewMemberExpression(MemberAccessKind::kClass, class_inst_expr, render_expr);
            funcexprs_.push_back(factory->NewDeclaration(vnode_ptr));
            Handle<Expression> assign_expr = factory->NewAssignExpression(factory->NewIdentifier(vnode_ptr), factory->NewCallExpression(MemberAccessKind::kCall, class_inst_expr, render_expr, {}));
            funcexprs_.push_back(factory->NewIfStatement(member_func_expr, assign_expr));
        }
    }
    return funcexprs_;
}

}
}
}
