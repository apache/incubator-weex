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

namespace weex {
namespace core {
namespace data_render {

namespace {

template <typename T>
std::string to_string(T value) {
  std::ostringstream os;
  os << value;
  return os.str();
}

struct ASTParser final {
  /* State */
  const json11::Json& json_;
  std::string& err_;
  bool failed_;
  ASTFactory* factory_;
  std::vector<Handle<Expression>> stacks_;
  ParseResult Fail(std::string&& msg) {
    return Fail(move(msg), ParseResult());
  }
  template <typename T>
  T Fail(std::string&& msg, const T err_ret) {
    if (!failed_)
      err_ = std::move(msg);
    failed_ = true;
    return err_ret;
  }
  void Parsing(Handle<ExpressionList>& list, Json object) {
    do {
      auto items = object.object_items();
      auto body = object["body"];
      if (body.is_object()) {
        ParseObject(list, body);
      }
      auto control = object["control"];
      if (!control.is_null()) {
        ParseControl(list, control);
      }

    } while (0);
  };
  void ParseObject(Handle<ExpressionList>& list, Json object) {
  }
  void ParseControl(Handle<ExpressionList>& list, Json object) {

  }
  ParseResult Parse() {
    ASTParseError error = UNKOWN_ERROR;
    do {
      std::string err_comment;
      auto& json = json_;
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
      if (!ParseStatement(body)) {
        break;
      }
      return ParseResult(chunk);

    } while (0);

    return Fail("parse error:" + to_string(error));
  }
  Handle<Expression> ParseIfControl(Json& json) {
    Handle<Expression> ifExpr = nullptr;
    do {
      Json match = json["match"];
      if (!match.is_string()) {
        break;
      }
      Handle<Expression> expr = ExpressionParser::ParseExpressionByString(match.string_value());
      Handle<Expression> if_block = MakeHandle<BlockStatement>(json, factory_->NewExpressionList());
      ifExpr = factory_->NewIfStatement(match, expr, if_block);
      Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
      statement->PushExpression(ifExpr);
      stacks_.push_back(if_block);

    } while (0);

    return ifExpr;
  }
  Handle<Expression> ParseForControl(Json& json) {
    Handle<Expression> for_expr = nullptr;
    do {
      Json repeat = json["repeat"];
      Json index = repeat["iterator1"];
      if (!index.is_string()) {
        break;
      }
      Handle<DeclarationList> for_init = factory_->NewDeclarationList(repeat);
      //var index=0
      for_init->Append(factory_->NewDeclaration(repeat, index.string_value(),
                                               factory_->NewIntegralConstant(json, 0)));
      Json expression = repeat["for"];
      if (!expression.is_string()) {
        break;
      }
      Json alias = repeat["alias"];
      if (!alias.is_string()) {
        break;
      }
      Handle<Expression> list_expr = ExpressionParser::ParseExpressionByString(
          expression.string_value());
      Handle<Identifier> index_var = factory_->NewIdentifier(index, index.string_value());
      //expr[index]
      Handle<MemberExpression> indexMember = factory_->NewMemberExpression(repeat,
                                                                           MemberAccessKind::kIndex,
                                                                           list_expr, index_var);
      //var iter = expr[index]
      for_init->Append(factory_->NewDeclaration(repeat, alias.string_value(), indexMember));
      Handle<Identifier> size_of_func_id = factory_->NewIdentifier(index, "sizeof");
      //sizeof(expr)
      Handle<CallExpression> size_of_call = factory_->NewCallExpression(size_of_func_id, list_expr);

      //index<sizeof(expr)
      Handle<Expression> for_condition = factory_->NewBinaryExpression(repeat,
                                                                      BinaryOperation::kLessThan,
                                                                      index_var, size_of_call);
      //index++
      Handle<Expression> for_update = factory_->NewPrefixExpression(repeat,
                                                                   PrefixOperation::kIncrement,
                                                                   index_var);
      Handle<Expression> for_block = MakeHandle<BlockStatement>(json, factory_->NewExpressionList());
      for_expr = factory_->NewForStatement(repeat, ForKind::kForOf, for_init, for_condition, for_update,
                                          for_block);
      if (!for_expr) {
        break;
      }
      Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
      statement->PushExpression(for_expr);
      stacks_.push_back(for_block);

    } while (0);

    return for_expr;
  }
  std::vector<Handle<Expression>> ParseControl(Json& json) {
    std::vector<Handle<Expression>> exprs;
    do {
      Handle<Expression> for_expr = ParseForControl(json);
      exprs.push_back(for_expr);

      Handle<Expression> if_expr = ParseIfControl(json);
      exprs.push_back(if_expr);

    } while (0);

    return exprs;
  }
  Handle<Expression> ParseBindingExpression(const Json& json) {
    const Json& exp_str = json["@binding"];
    if (exp_str.is_string()) {
      return ExpressionParser::ParseExpressionByString(exp_str.string_value());
    } else {
      return factory_->NewNullConstant();
    }
  }
  bool ParseStatement(Json& json) {
    bool succ = true;
    do {
      Json control = json["control"];
      std::vector<Handle<Expression>> control_exprs;
      if (control.is_object()) {
        control_exprs = ParseControl(control);
      }
      Json node_id = json["nodeId"];
      Json tag_name = json["tagName"];
      Handle<Expression> node_id_expr = nullptr;
      if (node_id.is_string() && tag_name.is_string()) {
        Handle<Expression> call_expr = nullptr;
        std::vector<Handle<Expression>> args;
        Handle<Expression> func = factory_->NewIdentifier(json, "createElement");
        args.push_back(factory_->NewStringConstant(tag_name, tag_name.string_value()));
        if (control_exprs.size() > 0 && control_exprs[0]->IsForStatement()) {
          Json repeat = control["repeat"];
          Json index = repeat["iterator1"];
          node_id_expr = factory_->NewBinaryExpression(node_id, BinaryOperation::kAddition,
                                                     factory_->NewStringConstant(node_id,
                                                                                 node_id.string_value() +
                                                                                 "_REPEAT_"),
                                                     factory_->NewIdentifier(node_id,
                                                                             index.string_value()));
          args.push_back(node_id_expr);
        } else {
          node_id_expr = factory_->NewStringConstant(node_id, node_id.string_value());
          args.push_back(node_id_expr);
        }
        call_expr = factory_->NewCallExpression(json, func, args);
        Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
        statement->PushExpression(call_expr);
        Handle<BlockStatement> parent_statement =
            control_exprs.size() == 0 ? stacks_[stacks_.size() - 1] : (stacks_[stacks_.size() -
                                                                              control_exprs.size()-1]);
        if (parent_statement->IsChildStatement()) {
          Handle<Expression> append_func = factory_->NewIdentifier(json, "appendChild");
          Handle<Expression> call_append_expr = nullptr;
          args.push_back(
              factory_->NewStringConstant(json, parent_statement->AsChildStatement()->parent()));
          call_append_expr = factory_->NewCallExpression(json, append_func, args);
          statement->PushExpression(call_append_expr);
        }
      }
      Json class_list = json["classList"];
      if (class_list.is_array()) {
        Handle<Expression> func_id = factory_->NewIdentifier(json, "setClassList");
        if (func_id) {
          std::vector<Handle<Expression>> args;
          args.push_back(node_id_expr);
          Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
          for (int i = 0; i < class_list.array_items().size(); i++) {
            Json class_style = class_list[i];
            if (class_style.is_string()) {
              args.push_back(factory_->NewStringConstant(class_style, class_style.string_value()));
              statement->PushExpression(factory_->NewCallExpression(json, func_id, args));
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
          args.push_back(factory_->NewStringConstant(json, node_id.string_value()));
          args.push_back(factory_->NewStringConstant(json, key));
          if (value.is_string()) {
            args.push_back(factory_->NewStringConstant(json, value.string_value()));
          } else {
            args.push_back(ParseBindingExpression(value));
          }

          Handle<Expression> set_attr_func_id = factory_->NewIdentifier(json, "setAttr");
          Handle<CallExpression> call_func = factory_->NewCallExpression(
              json, set_attr_func_id, args);
          Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
          statement->PushExpression(call_func);
        }
      }
      Json childs = json["childNodes"];
      if (childs.is_array() && childs.array_items().size() > 0) {
        for (int i = 0; i < childs.array_items().size(); i++) {
          Json child = childs[i];
          if (child.is_null() || !child.is_object()) {
            continue;
          }
          Handle<Expression> child_statement = factory_->NewChildStatement(child,
                                                                          factory_->NewExpressionList(),
                                                                          node_id.string_value());
          Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
          stacks_.push_back(child_statement);
          if (!ParseStatement(child)) {
            break;
          }
          statement->PushExpression(child_statement);
        }
        if (!succ) {
          break;
        }
      }
      if (control_exprs.size()) {
        for (int i = 0; i < control_exprs.size(); i++) {
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

ParseResult Parser::Parse(const json11::Json& in, std::string& err) {
  ASTParser parser{in, err, false, ASTFactory::GetFactoryInstance()};
  ParseResult result = parser.Parse();
  return result;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
