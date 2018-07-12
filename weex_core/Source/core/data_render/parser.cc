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

#include <sstream>
#include "core/data_render/parser.h"
#include "core/data_render/json/json11.hpp"
#include "core/data_render/ast_factory.h"
#include "core/data_render/statement.h"

namespace weex {
namespace core {
namespace data_render {

namespace {

template <typename T>
std::string to_string(T value)
{
    std::ostringstream os;
    os << value ;
    return os.str() ;
}

struct ASTParser final {
    /* State */
    const std::string &str_;
    std::string &err_;
    bool failed_;
    ASTFactory *factory_;
    std::vector<Handle<Expression>> stacks_;
    ParseResult fail(std::string &&msg) {
        return fail(move(msg), ParseResult());
    }
    template <typename T>
    T fail(std::string &&msg, const T err_ret) {
        if (!failed_)
            err_ = std::move(msg);
        failed_ = true;
        return err_ret;
    }
    void parsing(Handle<ExpressionList> &list, Json object) {
        do {
            auto items = object.object_items();
            auto body = object["body"];
            if (body.is_object()) {
                parseObject(list, body);
            }
            auto control = object["control"];
            if (!control.is_null()) {
                parseControl(list, control);
            }
            
        } while (0);
    };
    void parseObject(Handle<ExpressionList> &list, Json object) {
    }
    void parseControl(Handle<ExpressionList> &list, Json object) {
        
    }
    ParseResult parse() {
        ASTParseError error  = UNKOWN_ERROR;
        do {
            std::string err_comment;
            auto json = Json::parse(str_, err_comment);
            if (!json.is_object()) {
                error = FILE_FORMAT_ERROR;
                break;
            }
            auto body = json["body"];
            if (body.is_null()) {
                error = BODY_NONE_ERROR;
                break;
            }
            Handle<ExpressionList> list = factory_->NewExpressionList();
            Handle<ChunkStatement> chunk = factory_->NewChunkStatement(body, list);
            stacks_.push_back(chunk);
            if (!parseStatement(body)) {
                break;
            }
            return ParseResult(chunk);
            
        } while (0);
        
        return fail("parse error:" + to_string(error));
    }
    Handle<Expression> parseControl(Json &json) {
        Handle<Expression> controlExpr = nullptr;
        do {
            Json repeat = json["repeat"];
            Json index = repeat["index"];
            if (!index.is_string()) {
                break;
            }
            Handle<DeclarationList> forInit = factory_->NewDeclarationList(repeat);
            forInit->Append(factory_->NewDeclaration(repeat, index.string_value(), factory_->NewIntegralConstant(json, 0)));
            Json expression = repeat["expression"];
            if (!expression.is_string()) {
                break;
            }
            Json alias = repeat["alias"];
            if (!alias.is_string()) {
                break;
            }
            Handle<Identifier> expressionIdentifier = factory_->NewIdentifier(expression, expression.string_value());
            Handle<Identifier> indexIdentifier = factory_->NewIdentifier(index, index.string_value());
            Handle<MemberExpression> indexMember = factory_->NewMemberExpression(repeat, MemberAccessKind::kIndex, expressionIdentifier, indexIdentifier);
            forInit->Append(factory_->NewDeclaration(repeat, alias.string_value(), indexMember));
            Handle<Identifier> lengthIdentifier = factory_->NewIdentifier(index, "length");
            Handle<MemberExpression> lengthMember = factory_->NewMemberExpression(repeat, MemberAccessKind::kDot, expressionIdentifier, lengthIdentifier);
            Handle<Expression> forCondition = factory_->NewBinaryExpression(repeat, BinaryOperation::kLessThan, indexIdentifier, lengthMember);
            Handle<Expression> forUpdate = factory_->NewPrefixExpression(repeat, PrefixOperation::kIncrement, indexIdentifier);
            Handle<Expression> forBlock = MakeHandle<BlockStatement>(json, factory_->NewExpressionList());
            controlExpr = factory_->NewForStatement(repeat, ForKind::kForOf, forInit, forCondition, forUpdate, forBlock);
            if (!controlExpr) {
                break;
            }
            Handle<BlockStatement>statement = stacks_[stacks_.size() - 1];
            statement->PushExpression(controlExpr);
            stacks_.push_back(forBlock);
            
        } while (0);

        return controlExpr;
    }
    bool parseStatement(Json &json) {
        bool error = true;
        do {
            Json control = json["control"];
            Handle<Expression> controlExpr = nullptr;
            if (control.is_object()) {
                controlExpr = parseControl(control);
            }
            Json nodeId = json["nodeId"];
            Json tagName = json["tagName"];
            Handle<Expression> nodeIdExpr = nullptr;
            if (nodeId.is_string() && tagName.is_string()) {
                Handle<Expression> callExpr = nullptr;
                std::vector<Handle<Expression>> args;
                Handle<Expression> func = factory_->NewIdentifier(json, "createElement");
                args.push_back(factory_->NewStringConstant(tagName, tagName.string_value()));
                if (controlExpr && controlExpr->IsForStatement()) {
                    Json repeat = control["repeat"];
                    Json index = repeat["index"];
                    nodeIdExpr = factory_->NewBinaryExpression(nodeId, BinaryOperation::kAddition, factory_->NewStringConstant(nodeId, nodeId.string_value() + "_REPEAT_"), factory_->NewIdentifier(nodeId, index.string_value()));
                    args.push_back(nodeIdExpr);
                }
                else {
                    nodeIdExpr = factory_->NewStringConstant(nodeId, nodeId.string_value());
                    args.push_back(nodeIdExpr);
                }
                callExpr = factory_->NewCallExpression(json, func, args);
                Handle<BlockStatement>statement = stacks_[stacks_.size() - 1];
                statement->PushExpression(callExpr);
                Handle<BlockStatement> parentStatement = controlExpr == nullptr ? stacks_[stacks_.size() - 1] : (stacks_[stacks_.size() - 2]);
                if (parentStatement->IsChildStatement()) {
                    Handle<Expression> appendFunc = factory_->NewIdentifier(json, "appendChild");
                    Handle<Expression> callAppendExpr = nullptr;
                    args.push_back(factory_->NewStringConstant(json, parentStatement->AsChildStatement()->parent()));
                    callAppendExpr = factory_->NewCallExpression(json, appendFunc, args);
                    statement->PushExpression(callAppendExpr);
                }
            }
            Json attributes = json["attributes"];
            if (attributes.is_object()) {
                Handle<Expression> attrExpr = parseExpression(attributes);
                if (attrExpr) {
                    Handle<Expression> func = factory_->NewIdentifier(json, "setAttr");
                    std::vector<Handle<Expression>> args;
                    args.push_back(attrExpr);
                    Handle<CallExpression> callExpr = factory_->NewCallExpression(attributes, func, args);
                    Handle<BlockStatement>statement = stacks_[stacks_.size() - 1];
                    statement->PushExpression(callExpr);
                }
            }
            Json childs = json["childNodes"];
            if (childs.is_array() && childs.array_items().size() > 0) {
                for (int i = 0;i < childs.array_items().size(); i++) {
                    Json child = childs[i];
                    if (child.is_null() || !child.is_object()) {
                        continue;
                    }
                    Handle<Expression> childStatement = factory_->NewChildStatement(child, factory_->NewExpressionList(), nodeId.string_value());
                    stacks_.push_back(childStatement);
                    if (!parseStatement(child)) {
                        break;
                    }
                }
                if (error) {
                    break;
                }
            }
            if (controlExpr) {
                stacks_.pop_back();
            }
            error = false;

        } while (0);
        
        stacks_.pop_back();
        return error;
    }
    Handle<Expression> parseArrayExpression(Json &json) {
        Handle<Expression> expr = nullptr;
        do {
            if (!json.array_items().size()) {
                break;
            }
            ProxyArray proxyArray;
            for (int i = 0; i < json.array_items().size(); i++) {
                Json item = json[i];
                Handle<Expression> itemExpr = parseExpression(item);
                if (itemExpr) {
                    proxyArray.push_back(itemExpr);
                }
            }
            
        } while (0);
        
        return expr;
    }
    Handle<Expression> parseObjExpression(Json &json) {
        Handle<Expression> expr = nullptr;
        std::map<std::string, Json>obj = json.object_items();
        do {
            if (!obj.size()) {
                break;
            }
            ProxyObject proxyObj;
            for (std::map<std::string, Json>::iterator iter = obj.begin();iter != obj.end(); iter++) {
                if (iter->second.is_object()) {
                    Handle<Expression> objExpr = parseObjExpression(iter->second);
                    if (objExpr) {
                        proxyObj.insert(std::make_pair(iter->first, objExpr));
                    }
                }
                else if (iter->second.is_array()) {
                    Handle<Expression> arrayExpr = parseArrayExpression(iter->second);
                    if (arrayExpr) {
                        proxyObj.insert(std::make_pair(iter->first, arrayExpr));
                    }
                }
                else if (iter->second.is_string()) {
                    if (iter->first == "@binding") {
                        std::string strval = iter->second.string_value();
                        std::size_t found = strval.find('.');
                        if (found != std::string::npos) {
                            std::string strlhs = strval.substr(0, found);
                            std::string strrhs = strval.substr(found + 1);
                            Handle<Identifier> lhs = factory_->NewIdentifier(json, strlhs);
                            Handle<Identifier> rhs = factory_->NewIdentifier(json, strrhs);
                            Handle<Expression> bindingExpr = factory_->NewMemberExpression(json, MemberAccessKind::kDot, lhs, rhs);
                            return bindingExpr;
                        }
                        else {
                            return factory_->NewIdentifier(json, strval);
                        }
                    }
                    else {
                        proxyObj.insert(std::make_pair(iter->first, factory_->NewStringConstant(json, iter->second.string_value())));
                    }
                }
                else {
                    assert(0);
                }
            }
            expr = factory_->NewObjectConstant(json, proxyObj);
            
        } while (0);
        
        return expr;
    }
    Handle<Expression> parseExpression(Json &json) {
        Handle<Expression> expr = nullptr;
        do {
            if (json.is_object()) {
                expr = parseObjExpression(json);
                break;
            }
            else if (json.is_array()) {
                expr = parseArrayExpression(json);
                break;
            }
            
        } while (0);
        return expr;
    };
};
}
    
ParseResult Parser::parse(const std::string &in, std::string &err) {
    ASTParser parser { in, err, false, ASTFactory::GetFactoryInstance() };
    ParseResult result = parser.parse();
    return result;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
