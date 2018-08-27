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
#include <sstream>
#include "core/data_render/ast_factory.h"
#include "core/data_render/rax_parser_builder.h"
#include "core/data_render/rax_parser_context.h"
#include "third_party/json11/json11.hpp"
#include "core/data_render/statement.h"

namespace weex {
namespace core {
namespace data_render {

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
  std::vector<Handle<BlockStatement>> stacks_;

  ParseResult Fail(std::string&& msg) { return Fail(move(msg), ParseResult()); }

  template <typename T>
  T Fail(std::string&& msg, const T err_ret) {
    if (!failed_) err_ = std::move(msg);
    failed_ = true;
    return err_ret;
  }


    void AddAppendChildCall(json11::Json& json, Handle<Identifier>& parent_identifier,
                          Handle<Identifier>& child_identifier) {
    Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];

    std::vector<Handle<Expression>> args;
    Handle<Expression> append_func = factory_->NewIdentifier("appendChild");
    Handle<Expression> call_append_expr = nullptr;
    args.push_back(parent_identifier);
    args.push_back(child_identifier);
    call_append_expr = factory_->NewCallExpression(append_func, args);
    statement->PushExpression(call_append_expr);
  }

    void AddSetClassListCall(json11::Json& json, Handle<weex::core::data_render::Identifier> child_identifier,
                           const std::string& prefix) {
    std::string wrap_prefix;
    if (!prefix.empty()) {
      wrap_prefix = "_" + prefix + "_";
    }
    json11::Json class_list = json["classList"];
    if (class_list.is_array()) {
      Handle<Expression> func_id = factory_->NewIdentifier("setClassList");
      if (func_id) {
        std::vector<Handle<Expression>> args;
        args.push_back(child_identifier);
        Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
        for (int i = 0; i < class_list.array_items().size(); i++) {
          json11::Json class_style = class_list[i];
          if (class_style.is_string()) {
            args.push_back(
                factory_->NewStringConstant(wrap_prefix + class_style.string_value()));
            statement->PushExpression(factory_->NewCallExpression(func_id, args));
            args.pop_back();
          }
        }
      }
    }
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
      Handle<ChunkStatement> chunk = factory_->NewChunkStatement(list);
      stacks_.push_back(chunk);
      auto& component = json["components"];
      if (component.is_array()) {
        for (auto it = component.array_items().begin(); it != component.array_items().end(); it++) {
          if (!it->is_object()) {
            continue;
          }
          ParseComponentFunction(const_cast<json11::Json&>(*it));
        }
      }
      if (!ParseBodyFunction(body)) {
        stacks_.pop_back();
        break;
      }
      stacks_.pop_back();
      return ParseResult(chunk);

    } while (0);

    return Fail("parse error:" + to_string(error));
  }

  Handle<Expression> ParseIfControl(json11::Json& json) {
    Handle<Expression> ifExpr = nullptr;
    do {
      json11::Json match = json["match"];
      if (!match.is_string()) {
        break;
      }
      RAXParserBuilder builder(match.string_value());
      RAXParser *parser = builder.parser();
      Handle<Expression> expr = parser->ParseExpression();
      Handle<Expression> if_block =
          MakeHandle<BlockStatement>(factory_->NewExpressionList());
      ifExpr = factory_->NewIfStatement(expr, if_block);
      Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
      statement->PushExpression(ifExpr);
      stacks_.push_back(if_block);

    } while (0);

    return ifExpr;
  }

  Handle<Expression> ParseForControl(json11::Json& json) {
    Handle<Expression> for_expr = nullptr;
    do {
      json11::Json repeat = json["repeat"];
      json11::Json index = repeat["iterator1"];
      if (!index.is_string()) {
        break;
      }
      Handle<DeclarationList> for_init = factory_->NewDeclarationList();
      // var index=0
      for_init->Append(factory_->NewDeclaration(index.is_null() ? "index" : index.string_value(), factory_->NewIntegralConstant(0)));
      json11::Json expression = repeat["for"];
      if (!expression.is_string()) {
        break;
      }
      json11::Json alias = repeat["alias"];
      if (!alias.is_string()) {
        break;
      }
      RAXParserBuilder builder(expression.string_value());
      RAXParser *parser = builder.parser();
      Handle<Expression> list_expr = parser->ParseExpression();
      Handle<Identifier> index_var =
          factory_->NewIdentifier(index.is_null() ? "index" : index.string_value());
      // expr[index]
      Handle<MemberExpression> indexMember = factory_->NewMemberExpression(
          MemberAccessKind::kIndex, list_expr, index_var);
      // var iter = expr[index]
      for_init->Append(
          factory_->NewDeclaration(alias.string_value(), indexMember));
      Handle<Identifier> size_of_func_id =
          factory_->NewIdentifier("sizeof");
      // sizeof(expr)
      Handle<CallExpression> size_of_call =
        factory_->NewCallExpression(size_of_func_id, {list_expr});

      // index<sizeof(expr)
      Handle<Expression> for_condition = factory_->NewBinaryExpression(BinaryOperation::kLessThan, index_var, size_of_call);
      // index++
      Handle<Expression> index_update = factory_->NewPrefixExpression(PrefixOperation::kIncrement, index_var);
      // iter = expr[index]
      Handle<Expression> alias_update = factory_->NewAssignExpression(
          factory_->NewIdentifier(alias.string_value()),
          factory_->NewMemberExpression(
              MemberAccessKind::kIndex,
              list_expr, index_var
          )
      );
      std::vector<Handle<Expression>> updates;
      updates.push_back(index_update);
      updates.push_back(alias_update);
      Handle<Expression> for_update = factory_->NewCommaExpression(updates);

      Handle<Expression> for_block =
          MakeHandle<BlockStatement>(factory_->NewExpressionList());
      for_expr =
          factory_->NewForStatement(ForKind::kForOf, for_init,
                                    for_condition, for_update, for_block);
      if (!for_expr) {
        break;
      }
      Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
      statement->PushExpression(for_expr);
      stacks_.push_back(for_block);

    } while (0);

    return for_expr;
  }

  std::vector<Handle<Expression>> ParseControl(json11::Json& json) {
    std::vector<Handle<Expression>> exprs;
    do {
      Handle<Expression> for_expr = ParseForControl(json);
      if (for_expr != nullptr) {
        exprs.push_back(for_expr);
      }

      Handle<Expression> if_expr = ParseIfControl(json);
      if (if_expr != nullptr) {
        exprs.push_back(if_expr);
      }

    } while (0);

    return exprs;
  }

  Handle<Expression> ParseBindingExpression(const json11::Json& json) {
    const json11::Json& exp_str = json["@binding"];
    if (exp_str.is_string()) {
      RAXParserBuilder builder(exp_str.string_value());
      RAXParser *parser = builder.parser();
      return parser->ParseExpression();
    }
    else {
      return factory_->NewNullConstant();
    }
  }

  //function createComponent_xxx(nodeId, this){
  //  //initialState
  //  this = {
  //    count: this.start
  //  }
  //
  //  //ParseNode
  //  var child = createElement(tag_name, nodeId);
  //  appendChild(parent, child);
  //  setClassList(child, class_name);
  //  setAttr(child, key, value);
  //
  //  //return function
  //  return child;
  void ParseComponentFunction(json11::Json& json) {
    auto& name = json["name"];
    auto& initial_state = json["initialState"];
    //auto& styles = json["styles"];
    json11::Json template_obj = json["template"];

    Handle<BlockStatement> chunk = stacks_.back();

    if (!name.is_string() || !template_obj.is_object()) {
      return;
    }

    std::string function_name("createComponent_" + name.string_value());

    //function createComponent_xxx(nodeId, this)
    std::vector<std::string> proto_args;
    proto_args.push_back("nodeId");
    proto_args.push_back("this");
    Handle<FunctionPrototype> func_proto =
        factory_->NewFunctionPrototype(function_name, proto_args);
    Handle<BlockStatement> func_body =
        factory_->NewBlockStatement(factory_->NewExpressionList());

    //{
    stacks_.push_back(func_body);
    //initialState
    //  this = {
    //    count: this.start
    //  }
    if (initial_state.is_object() && initial_state.object_items().size() > 0) {
      ProxyObject initial_obj;
      for (auto it = initial_state.object_items().begin();
           it != initial_state.object_items().end(); it++) {
        const auto& key = it->first;
        const auto& value = it->second;
        Handle<Expression> expr_value;
        if (value.is_string()) {
          expr_value = factory_->NewStringConstant(value.string_value());
        } else {
          expr_value = ParseBindingExpression(value);
        }
        initial_obj.insert({key, expr_value});
      }
      func_body->statements()->Insert(factory_->NewAssignExpression(
          factory_->NewIdentifier("this"),
          factory_->NewObjectConstant(initial_obj)
      ));
    }

    //ParseNode
    ParseNode(template_obj, true, name.string_value());

    //return function
    func_body->statements()->Insert(factory_->NewReturnStatement(factory_->NewIdentifier("child")));
    stacks_.pop_back();
    //}
    Handle<FunctionStatement> func_decl =
        factory_->NewFunctionStatement(func_proto, func_body);
    chunk->statements()->Insert(func_decl);
  }

  //function main(this);
  bool ParseBodyFunction(json11::Json& body) {
    std::vector<std::string> proto_args;
    proto_args.push_back("this");
    Handle<FunctionPrototype> main_func_proto =
        factory_->NewFunctionPrototype("main", proto_args);
    Handle<BlockStatement> main_func_body =
        factory_->NewBlockStatement(factory_->NewExpressionList());
    //{
    stacks_.push_back(main_func_body);
    //var parent = null;
    main_func_body->PushExpression(factory_->NewDeclaration("parent"));
    //  ...
    ParseNode(body, false, "");

    stacks_.pop_back();
    //}
    Handle<FunctionStatement> main_func_decl = factory_->NewFunctionStatement(main_func_proto,
                                                                              main_func_body);
    Handle<BlockStatement> chunk = stacks_[stacks_.size() - 1];

    //function main(this);
    chunk->statements()->Insert(main_func_decl);

    std::vector<Handle<Expression>> args;
    std::vector<Handle<Expression>> merge_args;

    merge_args.push_back(factory_->NewIdentifier("_data_main"));
    merge_args.push_back(factory_->NewIdentifier("_init_data"));

    //merge(_data_main,_init_data)
    args.push_back(
        factory_->NewCallExpression(
            factory_->NewIdentifier("merge"),
            merge_args
        )
    );

    //main(merge(_data_main,_init_data)
    chunk->statements()->Insert(factory_->NewCallExpression(
        factory_->NewIdentifier("main"),
        args
    ));
    return true;
  }

  bool ParseComponent(json11::Json& json, const std::string& component_name) {
    static Handle<Identifier> parent_identifier =
        factory_->NewIdentifier("parent");
    static Handle<Identifier> child_identifier =
        factory_->NewIdentifier("child");
    static Handle<Identifier> attr_identifier =
        factory_->NewIdentifier("attr");

    json11::Json control = json["control"];
    std::vector<Handle<Expression>> control_exprs;
    if (control.is_object()) {
      control_exprs = ParseControl(control);
    }
    json11::Json node_id = json["nodeId"];
    json11::Json tag_name = json["tagName"];
    json11::Json component_alias = json["componentAlias"];

    if (node_id.is_string() && tag_name.is_string()) {
      const std::string& component_real_name = component_alias[tag_name.string_value()].string_value();


      Handle<Expression> call_expr = nullptr;
      Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
      std::vector<Handle<Expression>> args;

      //var attr = { key1:value1 }
      ProxyObject attr_obj;
      json11::Json attributes = json["attributes"];
      if (attributes.is_object()) {
        auto items = attributes.object_items();
        for (auto it = items.begin(); it != items.end(); ++it) {
          const auto& key = it->first;
          const auto& value = it->second;
          Handle<Expression> expr_value;
          if (value.is_string()) {
            expr_value = factory_->NewStringConstant(value.string_value());
          } else {
            expr_value = ParseBindingExpression(value);
          }
          attr_obj.insert({key, expr_value});
        }
      }

      Handle<ObjectConstant> attr_init = factory_->NewObjectConstant(attr_obj);
      Handle<Declaration> attr_decl = factory_->NewDeclaration("attr", attr_init);
      statement->PushExpression(attr_decl);

      // var child = createComponent_xxx("node_id", attr);
      {
        //createComponent_xxx("node_id", attr))
        Handle<Expression> func = factory_->NewIdentifier("createComponent_" + component_real_name);

        args.push_back(ParseNodeId(control, control_exprs, node_id.string_value()));
        args.push_back(attr_identifier);

        // var child = createComponent_xxx("node_id", attr);
        call_expr = factory_->NewCallExpression(func, args);
        Handle<Declaration> child_declaration =
            factory_->NewDeclaration("child", call_expr);
        statement->PushExpression(child_declaration);
      }

      //appendChild(parent, child);
      AddAppendChildCall(json, parent_identifier, child_identifier);

      //setClassList(child, class_name);
      AddSetClassListCall(json, child_identifier, component_name);
    }
    return true;
  }
  Handle<Expression> ParseNodeId(const json11::Json& body,
                                 const std::vector<Handle<Expression>>& control_exprs,
                                 const std::string& node_id) {
    Handle<Expression> node_id_expr = nullptr;
    node_id_expr = this->factory_->NewStringConstant(node_id);
    return node_id_expr;
  }


  // Code Detail:
  //    var parent = nil;
  //    var child = createElement(tag_name, node_id);
  //    appendChild(parent, child);
  //    setClassList(child, class_name);
  //    setAttr(child, key, value);
  //    {//child 1
  //      var parent = child;
  //      for: / if:
  //        var child = createElement(tag_name, node_id);
  //        appendChild(parent, child);
  //        setClassList(child, class_name);
  //        setAttr(child, key, value);
  //        {
  //            ...
  //        }
  //      end for / end if
  //    }
  //    {//child 2
  //      var parent = child;
  //      for: / if:
  //        var child = createElement(tag_name, node_id);
  //        appendChild(parent, child);
  //        setClassList(child, class_name);
  //        setAttr(child, key, value);
  //        {
  //            ...
  //        }
  //      end for / end if
  //    }
  //    {//child 3, is XXX component
  //      var parent = child;
  //      for: / if:
  //        var attr = {
  //          key1 : expr1;
  //          key2 : expr2;
  //        };
  //        var child = createComponent_xxx(node_id, attr);
  //        appendChild(parent, child);
  //        setClassList(child, class_name);
  //      end for / end if
  //    }
  bool ParseNode(json11::Json& json, bool component_root, const std::string& component_name) {
    static Handle<Identifier> parent_identifier =
        factory_->NewIdentifier("parent");
    static Handle<Identifier> child_identifier =
        factory_->NewIdentifier("child");
    bool succ = true;
    do {
      json11::Json control = json["control"];
      std::vector<Handle<Expression>> control_exprs;
      if (control.is_object()) {
        control_exprs = ParseControl(control);
      }
      json11::Json node_id = json["nodeId"];
      json11::Json tag_name = json["tagName"];
      Handle<Expression> node_id_expr = nullptr;
      if (tag_name.is_string()) {
        Handle<Expression> call_expr = nullptr;
        Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
        std::vector<Handle<Expression>> args;

        // var child = createElement(tag_name, node_id);
        {
          Handle<Expression> func =
              factory_->NewIdentifier("createElement");
          args.push_back(
              factory_->NewStringConstant(tag_name.string_value()));
          if (component_root) {
            node_id_expr = factory_->NewIdentifier("nodeId");
          } else {
            node_id_expr = ParseNodeId(control, control_exprs, node_id.string_value());
          }
          args.push_back(node_id_expr);
          call_expr = factory_->NewCallExpression(func, args);
          Handle<Declaration> child_declaration =
              factory_->NewDeclaration("child", call_expr);
          statement->PushExpression(child_declaration);
        }
        if (!component_root) {
          // appendChild(parent, child);
          AddAppendChildCall(json, parent_identifier, child_identifier);
        }
      }

      // setClassList(child, class_name)
      AddSetClassListCall(json, child_identifier, component_name);

      // setAttr(child, key, value)
      json11::Json attributes = json["attributes"];
      if (attributes.is_object()) {
        auto items = attributes.object_items();
        for (auto it = items.begin(); it != items.end(); ++it) {
          const auto& key = it->first;
          const auto& value = it->second;
          std::vector<Handle<Expression>> args;
          args.push_back(child_identifier);
          args.push_back(factory_->NewStringConstant(key));
          if (value.is_string()) {
            args.push_back(
                factory_->NewStringConstant(value.string_value()));
          } else {
            args.push_back(ParseBindingExpression(value));
          }

          Handle<Expression> set_attr_func_id =
              factory_->NewIdentifier("setAttr");
          Handle<CallExpression> call_func =
              factory_->NewCallExpression(set_attr_func_id, args);
          Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
          statement->PushExpression(call_func);
        }
      }

      // new block for appending grandson
      // {
      //    var parent = child;
      // }
      json11::Json childs = json["childNodes"];
      if (childs.is_array() && childs.array_items().size() > 0) {
        for (int i = 0; i < childs.array_items().size(); i++) {
          json11::Json child = childs[i];
          if (child.is_null() || !child.is_object()) {
            continue;
          }
          Handle<BlockStatement> child_block =
              factory_->NewBlockStatement(factory_->NewExpressionList());
          Handle<Declaration> parent_declaration =
              factory_->NewDeclaration("parent", child_identifier);
          child_block->statements()->Insert(parent_declaration);
          Handle<BlockStatement> cur_block = stacks_[stacks_.size() - 1];
          stacks_.push_back(child_block);

          const json11::Json& is_component = child["isComponent"];
          if (is_component.is_bool() && is_component.bool_value()) {
            if (!ParseComponent(child, component_name)) {
              stacks_.pop_back();
              break;
            }
          } else {
            if (!ParseNode(child, false, component_name)) {
              stacks_.pop_back();
              break;
            }
          }
          stacks_.pop_back();
          cur_block->PushExpression(child_block);
        }
      }
      for (int i = 0; i < control_exprs.size(); i++) {
        stacks_.pop_back();
      }
      succ = true;

    } while (0);
    return succ;
  }
};

ParseResult Parser::Parse(const json11::Json& in, std::string& err) {
  ASTParser parser{in, err, false, ASTFactory::GetFactoryInstance()};
  return parser.Parse();
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
