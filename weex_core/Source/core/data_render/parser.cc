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
#include <cstdlib>
#include <sstream>

#include "base/string_util.h"
#include "core/data_render/ast_factory.h"
#include "core/data_render/rax_parser_builder.h"
#include "core/data_render/rax_parser_context.h"
#include "core/data_render/statement.h"
#include "third_party/json11/json11.hpp"

namespace weex {
namespace core {
namespace data_render {

static const char kCreateComponentRecursively[] =
    "__createComponentRecursively";
static const char kCreateBodyFunction[] = "main";
static const char kAttrRepeatControl[] = "[[repeat]]";

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

  int GetTemplateId(const json11::Json& component) {
    json11::Json json_template_id = component["templateId"];
    if (json_template_id.type() == Json::Type::NUMBER) {
      return json_template_id.int_value();
    } else {
      return std::atoi(json_template_id.string_value().c_str());
    }
  }

  inline void SetAttributeStatement(std::vector<Handle<Expression>> args,
                                    Handle<BlockStatement>& block) {
    Handle<Expression> set_attr_func_id = factory_->NewIdentifier("setAttr");
    Handle<CallExpression> call_func =
        factory_->NewCallExpression(set_attr_func_id, args);
    block->PushExpression(call_func);
  }

  inline void AddEventStatement(std::vector<Handle<Expression>> args,
                                Handle<BlockStatement>& block) {
    Handle<Expression> set_attr_func_id = factory_->NewIdentifier("addEvent");
    Handle<CallExpression> call_func =
        factory_->NewCallExpression(set_attr_func_id, args);
    block->PushExpression(call_func);
  }

  void AddAppendChildCall(json11::Json& json,
                          Handle<Identifier>& parent_identifier,
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

  void AddSetClassListCall(
      json11::Json& json,
      Handle<weex::core::data_render::Identifier> child_identifier,
      const std::string& prefix) {
    std::string wrap_prefix;
    if (!prefix.empty() && prefix.compare(kCreateBodyFunction) != 0) {
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
            args.push_back(factory_->NewStringConstant(
                wrap_prefix + class_style.string_value()));
          } else {
            args.push_back(factory_->NewBinaryExpression(
                BinaryOperation::kAddition,
                factory_->NewStringConstant(wrap_prefix),
                ParseBindingExpression(class_style)));
          }
          statement->PushExpression(factory_->NewCallExpression(func_id, args));
          args.pop_back();
        }
      }
    }
  }

  void AddEventForVNode(
      const json11::Json& events,
      Handle<weex::core::data_render::Identifier> child_identifier) {
    if (!events.is_array()) return;
    for (auto it = events.array_items().begin();
         it != events.array_items().end(); it++) {
      auto event = *it;
      if (event.is_object()) {
        auto event_type = event["type"];
        auto params = event["params"];
        Handle<StringConstant> event_constant =
            factory_->NewStringConstant(event_type.string_value());
        std::vector<Handle<Expression>> args{child_identifier, event_constant};
        if (params.is_array()) {
          for (auto jt = params.array_items().begin();
               jt != params.array_items().end(); jt++) {
            args.push_back(ParseExpression(*jt));
          }
        }
        AddEventStatement(args, stacks_[stacks_.size() - 1]);
      } else if (event.is_string()) {
        Handle<StringConstant> event_constant =
            factory_->NewStringConstant(event.string_value());
        AddEventStatement({child_identifier, event_constant},
                          stacks_[stacks_.size() - 1]);
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
        for (auto it = component.array_items().begin();
             it != component.array_items().end(); it++) {
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

    return Fail("parse error:" + base::to_string(error));
  }

  Handle<Expression> ParseIfControl(json11::Json& json) {
    Handle<Expression> ifExpr = nullptr;
    do {
      json11::Json match = json["match"];
      if (!match.is_string()) {
        break;
      }
      RAXParserBuilder builder(match.string_value());
      RAXParser* parser = builder.parser();
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
      Handle<DeclarationList> for_init = factory_->NewDeclarationList();
      // var index=0
      for_init->Append(factory_->NewDeclaration(
          index.is_null() ? "index" : index.string_value(),
          factory_->NewIntegralConstant(0)));
      json11::Json expression = repeat["for"];
      if (!expression.is_string()) {
        break;
      }
      json11::Json alias = repeat["alias"];
      if (!alias.is_string()) {
        break;
      }
      RAXParserBuilder builder(expression.string_value());
      RAXParser* parser = builder.parser();
      Handle<Expression> list_expr = parser->ParseExpression();
      Handle<Identifier> index_var = factory_->NewIdentifier(
          index.is_null() ? "index" : index.string_value());
      // expr[index]
      Handle<MemberExpression> indexMember = factory_->NewMemberExpression(
          MemberAccessKind::kIndex, list_expr, index_var);
      // var iter = expr[index]
      for_init->Append(
          factory_->NewDeclaration(alias.string_value(), indexMember));
      Handle<Identifier> size_of_func_id = factory_->NewIdentifier("sizeof");
      // sizeof(expr)
      Handle<CallExpression> size_of_call =
          factory_->NewCallExpression(size_of_func_id, {list_expr});

      // index<sizeof(expr)
      Handle<Expression> for_condition = factory_->NewBinaryExpression(
          BinaryOperation::kLessThan, index_var, size_of_call);
      // index++
      Handle<Expression> index_update =
          factory_->NewPrefixExpression(PrefixOperation::kIncrement, index_var);
      // iter = expr[index]
      Handle<Expression> alias_update = factory_->NewAssignExpression(
          factory_->NewIdentifier(alias.string_value()),
          factory_->NewMemberExpression(MemberAccessKind::kIndex, list_expr,
                                        index_var));
      std::vector<Handle<Expression>> updates;
      updates.push_back(index_update);
      updates.push_back(alias_update);
      Handle<Expression> for_update = factory_->NewCommaExpression(updates);

      Handle<Expression> for_block =
          MakeHandle<BlockStatement>(factory_->NewExpressionList());
      for_expr = factory_->NewForStatement(
          ForKind::kForOf, for_init, for_condition, for_update, for_block);
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

  inline Handle<Expression> ParseExpression(const json11::Json& json) {
    if (json.is_string()) {
      return factory_->NewStringConstant(json.string_value());
    } if (json.is_number()) {
        return factory_->NewIntegralConstant(json.number_value());
    } else {
      return ParseBindingExpression(json);
    }
  }

  Handle<Expression> ParseBindingExpression(const json11::Json& json) {
    const json11::Json& exp_str = json["@binding"];
    if (exp_str.is_string()) {
      RAXParserBuilder builder(exp_str.string_value());
      RAXParser* parser = builder.parser();
      return parser->ParseExpression();
    } else {
      return factory_->NewNullConstant();
    }
  }

  std::string ParseCreateNodeInComponentFunction(
      json11::Json& template_object, const std::string& component_name) {
    std::string function_name("createNodeInComponent_" + component_name);

    // Declare function createNodeInComponent_xxx(this)
    std::vector<std::string> proto_args;
    proto_args.push_back("this");
    proto_args.push_back("component");
    proto_args.push_back(kCreateComponentRecursively);
    Handle<FunctionPrototype> func_proto =
        factory_->NewFunctionPrototype(function_name, proto_args);
    Handle<BlockStatement> func_body =
        factory_->NewBlockStatement(factory_->NewExpressionList());

    stacks_.push_back(func_body);

    // ParseNode
    ParseNode(template_object, true, component_name);

    // Declare return
    func_body->statements()->Insert(
        factory_->NewReturnStatement(factory_->NewIdentifier("child")));
    stacks_.pop_back();

    Handle<FunctionStatement> func_decl =
        factory_->NewFunctionStatement(func_proto, func_body);
    Handle<BlockStatement> chunk = stacks_.front();
    // Declare createNodeInComponent function in global
    chunk->statements()->Insert(func_decl);
    return function_name;
  }

  // function createComponent_xxx(nodeId, this){
  //  //initialState
  //  this = {
  //    count: this.start
  //  }
  //  // template_id
  //  var template_id = 1;
  //  //ParseNode
  //  var child = createElement(tag_name, nodeId);
  //  appendChild(parent, child);
  //  setClassList(child, class_name);
  //  setAttr(child, key, value);
  //
  //  //return function
  //  return child;
  std::string ParseComponentFunction(json11::Json& json) {
    auto& name = json["name"];
    json11::Json template_obj = json["template"];
    int template_id = GetTemplateId(json);

    if (!name.is_string() || !template_obj.is_object()) {
      return "";
    }

    return ParseComponentFunction(template_obj, name.string_value(),
                                  template_id, false);
  }

  std::string ParseComponentFunction(json11::Json& template_obj,
                                     const std::string& template_name,
                                     int template_id, bool is_body) {
    Handle<BlockStatement> chunk = stacks_.back();

    std::string function_name("createComponent_" + template_name);

    // Make function createComponent_xxx(this)
    std::vector<std::string> proto_args;
    proto_args.push_back("this");
    proto_args.push_back(kCreateComponentRecursively);
    Handle<FunctionPrototype> func_proto =
        factory_->NewFunctionPrototype(function_name, proto_args);
    Handle<BlockStatement> func_body =
        factory_->NewBlockStatement(factory_->NewExpressionList());

    stacks_.push_back(func_body);

    func_body->statements()->Insert(factory_->NewDeclaration(
        "template_id", factory_->NewIntegralConstant(template_id)));
    func_body->statements()->Insert(factory_->NewDeclaration(
        "template_name", factory_->NewStringConstant(template_name)));

    std::vector<Handle<Expression>> merge_args;
    Handle<Identifier> component_data = factory_->NewMemberExpression(
        MemberAccessKind::kDot, factory_->NewIdentifier("_components_data"),
        factory_->NewIdentifier(template_name));
    Handle<Declaration> data_declaration =
        factory_->NewDeclaration("data", component_data);
    Handle<Identifier> component_props = factory_->NewMemberExpression(
        MemberAccessKind::kDot, factory_->NewIdentifier("_components_props"),
        factory_->NewIdentifier(template_name));
    Handle<Declaration> props_declaration =
        factory_->NewDeclaration("props", component_props);
    Handle<Identifier> component_computed = factory_->NewMemberExpression(
        MemberAccessKind::kDot, factory_->NewIdentifier("_components_computed"),
        factory_->NewIdentifier(template_name));
    Handle<Declaration> computed_declaration =
        factory_->NewDeclaration("computed", component_computed);
    func_body->PushExpression(data_declaration);
    func_body->PushExpression(props_declaration);
    func_body->PushExpression(computed_declaration);

    // Update props
    merge_args.clear();
    merge_args.push_back(factory_->NewIdentifier("props"));
    merge_args.push_back(factory_->NewIdentifier("this"));
    func_body->PushExpression(factory_->NewAssignExpression(
        factory_->NewIdentifier("props"),
        factory_->NewCallExpression(factory_->NewIdentifier("merge"),
                                    merge_args)));

    // Declare createNodeInComponent function in global
    const std::string& create_node_func =
        ParseCreateNodeInComponentFunction(template_obj, template_name);

    // Component Node call createComponent
    Handle<Expression> func = factory_->NewIdentifier("createComponent");
    std::vector<Handle<Expression>> args;
    args.push_back(factory_->NewIdentifier("template_id"));
    args.push_back(factory_->NewIdentifier("template_name"));
    args.push_back(factory_->NewStringConstant(create_node_func));
    Handle<Declaration> component_declaration = factory_->NewDeclaration(
        "component", factory_->NewCallExpression(func, args));
    func_body->statements()->Insert(component_declaration);
    args.clear();
    args.push_back(factory_->NewIdentifier("component"));
    args.push_back(factory_->NewIdentifier("props"));
    args.push_back(factory_->NewIdentifier("data"));
    func_body->statements()->Insert(factory_->NewCallExpression(
        factory_->NewIdentifier("saveComponentPropsAndData"), args));

    // if (PreventCreateComponentRecursively) {
    //    this = merge(props, this);
    //    this = merge(data, this);
    //    createNodeInComponent(this);
    // }
    args.clear();
    auto if_expression_list = factory_->NewExpressionList();
    // merge props
    if_expression_list->Insert(factory_->NewAssignExpression(
        factory_->NewIdentifier("this"), factory_->NewIdentifier("props")));
    // merge data
    merge_args.clear();
    merge_args.push_back(factory_->NewIdentifier("data"));
    merge_args.push_back(factory_->NewIdentifier("this"));
    if_expression_list->Insert(factory_->NewAssignExpression(
        factory_->NewIdentifier("this"),
        factory_->NewCallExpression(factory_->NewIdentifier("merge"),
                                    merge_args)));
    // merge computed
    merge_args.clear();
    merge_args.push_back(factory_->NewIdentifier("computed"));
    merge_args.push_back(factory_->NewIdentifier("this"));
    if_expression_list->Insert(factory_->NewAssignExpression(
        factory_->NewIdentifier("this"),
        factory_->NewCallExpression(factory_->NewIdentifier("merge"),
                                    merge_args)));

    args.clear();
    args.push_back(factory_->NewIdentifier("this"));
    args.push_back(factory_->NewIdentifier("component"));
    args.push_back(factory_->NewIdentifier(kCreateComponentRecursively));
    auto call_create_node = factory_->NewCallExpression(
        factory_->NewIdentifier(create_node_func), args);
    if_expression_list->Insert(call_create_node);
    auto if_block = factory_->NewBlockStatement(if_expression_list);
    auto if_stat = factory_->NewIfStatement(
        factory_->NewIdentifier(kCreateComponentRecursively), if_block);
    func_body->statements()->Insert(if_stat);

    // return function
    func_body->statements()->Insert(
        factory_->NewReturnStatement(factory_->NewIdentifier("component")));
    stacks_.pop_back();

    // Declare createComponent function in global
    Handle<FunctionStatement> func_decl =
        factory_->NewFunctionStatement(func_proto, func_body);
    chunk->statements()->Insert(func_decl);
    return function_name;
  }

  // function main(this);
  bool ParseBodyFunction(json11::Json& body) {
    const std::string& main =
        ParseComponentFunction(body, kCreateBodyFunction, 0, true);
    Handle<BlockStatement> chunk = stacks_[stacks_.size() - 1];
    std::vector<Handle<Expression>> args;
    args.push_back(factory_->NewIdentifier("_init_data"));
    args.push_back(factory_->NewBooleanConstant(true));
    // main(merge(_data_main,_init_data)
    chunk->statements()->Insert(
        factory_->NewCallExpression(factory_->NewIdentifier(main), args));
    return true;
  }

  bool ParseComponent(json11::Json& json, const std::string& component_name) {
    static Handle<Identifier> parent_identifier =
        factory_->NewIdentifier("parent");
    static Handle<Identifier> child_identifier =
        factory_->NewIdentifier("child");
    static Handle<Identifier> attr_identifier = factory_->NewIdentifier("attr");

    json11::Json control = json["control"];
    std::vector<Handle<Expression>> control_exprs;
    if (control.is_object()) {
      control_exprs = ParseControl(control);
    }
    json11::Json node_id = json["nodeId"];
    json11::Json tag_name = json["tagName"];
    json11::Json component_alias = json["componentAlias"];

    if (node_id.is_string() && tag_name.is_string()) {
      const std::string& component_real_name =
          component_alias[tag_name.string_value()].string_value();

      Handle<Expression> call_expr = nullptr;
      Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
      std::vector<Handle<Expression>> args;

      // var attr = { key1:value1 }
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

      // var child = createComponent_xxx("node_id", attr);
      {
        // createComponent_xxx("node_id", attr))
        Handle<Expression> func =
            factory_->NewIdentifier("createComponent_" + component_real_name);

        args.push_back(attr_init);
        args.push_back(factory_->NewIdentifier(kCreateComponentRecursively));

        // var child = createComponent_xxx("node_id", attr);
        call_expr = factory_->NewCallExpression(func, args);
        Handle<Declaration> child_declaration =
            factory_->NewDeclaration("child", call_expr);
        statement->PushExpression(child_declaration);
      }

      // appendChild(parent, child);
      AddAppendChildCall(json, parent_identifier, child_identifier);
    }
    for (int i = 0; i < control_exprs.size(); i++) {
      stacks_.pop_back();
    }
    return true;
  }
  Handle<Expression> ParseNodeId(
      const json11::Json& body,
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
  //        setStyle(child, key, value);
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
  bool ParseNode(json11::Json& json, bool component_root,
                 const std::string& component_name) {
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
        if (!control["repeat"].is_null()) {
          SetAttributeStatement(
              {child_identifier,
               factory_->NewStringConstant(kAttrRepeatControl),
               factory_->NewStringConstant("")},
              stacks_[stacks_.size() - 1]);
        }
      }
      json11::Json node_id = json["nodeId"];
      json11::Json tag_name = json["tagName"];
      json11::Json ref = json11::Json("");
      if (json["attributes"].is_object() &&
          json["attributes"]["ref"].is_string()) {
        ref = json["attributes"]["ref"];
      }
      Handle<Expression> node_id_expr = nullptr;
      if (tag_name.is_string()) {
        Handle<Expression> call_expr = nullptr;
        Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
        std::vector<Handle<Expression>> args;

        // var child = createElement(tag_name, node_id, ref);
        {
          Handle<Expression> func = factory_->NewIdentifier("createElement");
          node_id_expr =
              ParseNodeId(control, control_exprs, node_id.string_value());
          args.push_back(factory_->NewStringConstant(tag_name.string_value()));
          args.push_back(node_id_expr);
          args.push_back(factory_->NewStringConstant(ref.string_value()));
          call_expr = factory_->NewCallExpression(func, args);
          Handle<Declaration> child_declaration =
              factory_->NewDeclaration("child", call_expr);
          statement->PushExpression(child_declaration);
          if (component_root) {
            args.clear();
            args.push_back(factory_->NewIdentifier("component"));
            args.push_back(child_identifier);
            statement->PushExpression(factory_->NewCallExpression(
                factory_->NewIdentifier("setComponentRoot"), args));
          }
        }
        if (!component_root) {
          // appendChild(parent, child);
          AddAppendChildCall(json, parent_identifier, child_identifier);
        }
      }

      // setClassList(child, class_name)
      AddSetClassListCall(json, child_identifier, component_name);

      // setStyle(child, key, value)
      json11::Json style = json["style"];
      if (style.is_object()) {
        auto items = style.object_items();
        for (auto it = items.begin(); it != items.end(); ++it) {
          const auto& key = it->first;
          const auto& value = it->second;
          std::vector<Handle<Expression>> args;
          args.push_back(child_identifier);
          args.push_back(factory_->NewStringConstant(key));
          if (value.is_string()) {
            args.push_back(factory_->NewStringConstant(value.string_value()));
          } else {
            args.push_back(ParseBindingExpression(value));
          }

          Handle<Expression> set_style_func_expr =
              factory_->NewIdentifier("setStyle");
          Handle<CallExpression> call_func =
              factory_->NewCallExpression(set_style_func_expr, args);
          Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
          statement->PushExpression(call_func);
        }
      } else if (style.is_array()) {
        auto items = style.array_items();
        for (auto it = items.begin(); it != items.end(); ++it) {
          auto item = *it;
          if (item.is_object()) {
            std::vector<Handle<Expression>> args;
            args.push_back(child_identifier);
            args.push_back(ParseBindingExpression(item));
            Handle<Expression> set_style_func_expr =
                    factory_->NewIdentifier("setStyle");
            Handle<CallExpression> call_func =
                    factory_->NewCallExpression(set_style_func_expr, args);
            Handle<BlockStatement> statement = stacks_[stacks_.size() - 1];
            statement->PushExpression(call_func);
          }
        }
      }

      // setAttr(child, key, value)
      json11::Json attributes = json["attributes"];
      if (attributes.is_object()) {
        auto items = attributes.object_items();
        for (auto it = items.begin(); it != items.end(); ++it) {
          const auto& key = it->first;
          const auto& value = it->second;
          Handle<Expression> value_statements;
          if (value.is_string()) {
            value_statements =
                factory_->NewStringConstant(value.string_value());
          } else if (value.is_array()) {
            for (auto jt = value.array_items().begin();
                 jt != value.array_items().end(); jt++) {
              if (value_statements) {
                value_statements = factory_->NewBinaryExpression(
                    BinaryOperation::kAddition, value_statements,
                    ParseExpression(*jt));
              } else {
                value_statements = ParseExpression(*jt);
              }
            }
          } else {
            value_statements = ParseBindingExpression(value);
          }
          SetAttributeStatement(
              {child_identifier, factory_->NewStringConstant(key),
               value_statements},
              stacks_[stacks_.size() - 1]);
        }
      }

      // addEvent
      AddEventForVNode(json["event"], child_identifier);

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
