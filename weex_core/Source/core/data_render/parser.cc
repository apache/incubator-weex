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
#include "core/data_render/expression_parser.h"
#include "token.h"

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
    Handle<Expression> parseIfControl(Json &json) {
        Handle<Expression> ifExpr = nullptr;
        do {
            Json match = json["match"];
            if (!match.is_string()) {
                break;
            }
            Handle<Expression> expr = ExpressionParser::ParseExpressionByString(match.string_value());
            Handle<Expression> ifBlock = MakeHandle<BlockStatement>(json, factory_->NewExpressionList());
            ifExpr = factory_->NewIfStatement(match, expr, ifBlock);
            Handle<BlockStatement>statement = stacks_[stacks_.size() - 1];
            statement->PushExpression(ifExpr);
            stacks_.push_back(ifBlock);

        } while (0);

        return ifExpr;
    }
    Handle<Expression> parseForControl(Json &json) {
        Handle<Expression> forExpr = nullptr;
        do {
            Json repeat = json["repeat"];
            Json index = repeat["iterator1"];
            if (!index.is_string()) {
                break;
            }
            Handle<DeclarationList> forInit = factory_->NewDeclarationList(repeat);
            forInit->Append(factory_->NewDeclaration(repeat, index.string_value(), factory_->NewIntegralConstant(json, 0)));
            Json expression = repeat["for"];
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
            forExpr = factory_->NewForStatement(repeat, ForKind::kForOf, forInit, forCondition, forUpdate, forBlock);
            if (!forExpr) {
                break;
            }
            Handle<BlockStatement>statement = stacks_[stacks_.size() - 1];
            statement->PushExpression(forExpr);
            stacks_.push_back(forBlock);

        } while (0);

        return forExpr;
    }
    std::vector<Handle<Expression>> parseControl(Json &json) {
        std::vector<Handle<Expression>> exprs;
        do {
            Handle<Expression> ifExpr = parseIfControl(json);
            exprs.push_back(ifExpr);
            Handle<Expression> forExpr = parseForControl(json);
            exprs.push_back(forExpr);

        } while (0);

        return exprs;
    }
  Handle<Expression> parseBindingExpression(const Json& json) {
    const Json& exp_str = json["@binding"];
    if(exp_str.is_string()){
      return ExpressionParser::ParseExpressionByString(exp_str.string_value());
    } else{
      return factory_->NewNullConstant();
    }
  }
  bool parseStatement(Json &json) {
        bool succ = true;
        do {
            Json control = json["control"];
            std::vector<Handle<Expression>> controlExprs;
            if (control.is_object()) {
                controlExprs = parseControl(control);
            }
            Json nodeId = json["nodeId"];
            Json tagName = json["tagName"];
            Handle<Expression> nodeIdExpr = nullptr;
            if (nodeId.is_string() && tagName.is_string()) {
                Handle<Expression> callExpr = nullptr;
                std::vector<Handle<Expression>> args;
                Handle<Expression> func = factory_->NewIdentifier(json, "createElement");
                args.push_back(factory_->NewStringConstant(tagName, tagName.string_value()));
                if (controlExprs.size() > 0 && controlExprs[controlExprs.size() - 1]->IsForStatement()) {
                    Json repeat = control["repeat"];
                    Json index = repeat["iterator1"];
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
                Handle<BlockStatement> parentStatement = controlExprs.size() == 0 ? stacks_[stacks_.size() - 1] : (stacks_[stacks_.size() - controlExprs.size()]);
                if (parentStatement->IsChildStatement()) {
                    Handle<Expression> appendFunc = factory_->NewIdentifier(json, "appendChild");
                    Handle<Expression> callAppendExpr = nullptr;
                    args.push_back(factory_->NewStringConstant(json, parentStatement->AsChildStatement()->parent()));
                    callAppendExpr = factory_->NewCallExpression(json, appendFunc, args);
                    statement->PushExpression(callAppendExpr);
                }
            }
            Json classList = json["classList"];
            if (classList.is_array()) {
                Handle<Expression> fuctor = factory_->NewIdentifier(json, "setClassList");
                if (fuctor) {
                    std::vector<Handle<Expression>> args;
                    args.push_back(nodeIdExpr);
                    Handle<BlockStatement>statement = stacks_[stacks_.size() - 1];
                    for (int i = 0; i < classList.array_items().size(); i++) {
                        Json classStyle = classList[i];
                        if (classStyle.is_string()) {
                            args.push_back(factory_->NewStringConstant(classStyle, classStyle.string_value()));
                            statement->PushExpression(factory_->NewCallExpression(json, fuctor, args));
                            args.pop_back();
                        }
                    }
                }
            }
            Json attributes = json["attributes"];
            if (attributes.is_object()) {
                auto items = attributes.object_items();
                for (auto it = items.begin(); it != items.end(); ++it) {
                    const auto& key = it->first;
                    const auto& value = it->second;
                    std::vector<Handle<Expression>> args;
                    args.push_back(factory_->NewStringConstant(json, nodeId.string_value()));
                    args.push_back(factory_->NewStringConstant(json, key));
                    if (value.is_string()){
                        args.push_back(factory_->NewStringConstant(json, value.string_value()));
                    } else{
                        args.push_back(parseBindingExpression(value));
                    }

                    Handle<Expression> setAttrFunc = factory_->NewIdentifier(json, "setAttr");
                    Handle<CallExpression> callFunc = factory_->NewCallExpression(
                       json, setAttrFunc, args);
                    Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
                    statement->PushExpression(callFunc);
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
                    Handle<BlockStatement>statement = stacks_[stacks_.size() - 1];
                    stacks_.push_back(childStatement);
                    if (!parseStatement(child)) {
                        break;
                    }
                    statement->PushExpression(childStatement);
                }
                if (!succ) {
                    break;
                }
            }
            if (controlExprs.size()) {
                for (int i = 0; i < controlExprs.size(); i++) {
                    stacks_.pop_back();
                }
            }
            succ = true;

        } while (0);

        stacks_.pop_back();
        return succ;
    }
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
