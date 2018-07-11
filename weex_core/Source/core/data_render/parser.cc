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

#include "core/data_render/parser.h"
#include "core/data_render/json/json11.hpp"
#include "core/data_render/ast_factory.h"
#include "core/data_render/statement.h"

using namespace std;
using namespace json11;

namespace weex {
    namespace core {
        namespace data_render {

namespace {

struct ASTParser final {
    /* State */
    const string &str_;
    string &err_;
    bool failed_;
    ASTFactory *factory_;
    ParseResult fail(string &&msg) {
        return fail(move(msg), ParseResult());
    }
    template <typename T>
    T fail(string &&msg, const T err_ret) {
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
            string err_comment;
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
            return ParseResult(chunk);
            
        } while (0);
        
        return fail("parse error:" + to_string(error));
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
        map<string, Json>obj = json.object_items();
        do {
            if (!obj.size()) {
                break;
            }
            ProxyObject proxyObj;
            for (map<string, Json>::iterator iter = obj.begin();iter != obj.end(); iter++) {
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
                        proxyObj.insert(std::make_pair(iter->first, factory_->NewStringLiteral(json, iter->second.string_value())));
                    }
                }
                else {
                    assert(0);
                }
            }
            expr = factory_->NewObjectLiteral(json, proxyObj);
            
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

Handle<Expression> Parser::parseExpression(Json &json, std::string &err) {
    ASTParser parser { "", err, false, ASTFactory::GetFactoryInstance() };
    return parser.parseExpression(json);
}

ParseResult Parser::parse(const std::string &in, std::string &err) {
    ASTParser parser { in, err, false, ASTFactory::GetFactoryInstance() };
    ParseResult result = parser.parse();
    return result;
}

        }  // namespace data_render
    }  // namespace core
}  // namespace weex
