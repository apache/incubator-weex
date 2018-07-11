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
#include "core/data_render/ast_factory.h"
#include "core/data_render/statement.h"

namespace weex {
namespace core {
namespace data_render {
    
    ASTFactory *ASTFactory::GetFactoryInstance()
    {
        static std::unique_ptr<ASTFactory> factory_instance;
        
        if (!factory_instance) {
            factory_instance.reset(new ASTFactory());
        }
        return factory_instance.get();
    }
    
    Handle<ExpressionList> ASTFactory::NewExpressionList()
    {
        return MakeHandle<ExpressionList>();
    }
    Handle<Expression> ASTFactory::NewIdentifier(Json &json, std::string name) {
        return MakeHandle<Identifier>(json, name);
    }
    Handle<Expression> ASTFactory::NewStringLiteral(Json &json, std::string str)
    {
        return MakeHandle<StringLiteral>(json, str);
    }
    Handle<Expression> ASTFactory::NewArgumentList(Json &json, Handle<ExpressionList> args)
    {
        return MakeHandle<ArgumentList>(json, std::move(args));
    }
    Handle<Expression> ASTFactory::NewBlockStatement(Json &json, Handle<ExpressionList> list)
    {
        Handle<Expression> expr = MakeHandle<BlockStatement>(json, list);
        expr->Pasing(json);
        return expr;
    }
    Handle<ChunkStatement> ASTFactory::NewChunkStatement(Json &json, Handle<ExpressionList> list) {
        Handle<ChunkStatement> expr = MakeHandle<ChunkStatement>(json, list);
        expr->Pasing(json);
        return expr;
    }
    Handle<Expression> ASTFactory::NewFunctionPrototype(Json &json, std::string name, std::vector<std::string> args)
    {
        return MakeHandle<FunctionPrototype>(json, name, std::move(args));
    }
    Handle<Expression> ASTFactory::NewChildBlockStatement(Json &json, Handle<ExpressionList> list, std::string parent) {
        Handle<Expression> expr = MakeHandle<ChildBlockStatement>(json, list, parent);
        expr->Pasing(json);
        return expr;
    }
    Handle<Expression> ASTFactory::NewControlStatement(Json &json) {
        Handle<Expression> controlExpr = nullptr;
        Json control = json["control"];
        do {
            Json repeat = control["repeat"];
            if (!repeat.is_null() && repeat.is_object()) {
                Json index = repeat["index"];
                do {
                    if (!index.is_string()) {
                        break;
                    }
                    Handle<DeclarationList> forInit = NewDeclarationList(repeat);
                    forInit->Append(NewDeclaration(repeat, index.string_value(), NewIntegralLiteral(json, 0)));
                    Json expression = repeat["expression"];
                    if (!expression.is_string()) {
                        break;
                    }
                    Json alias = repeat["alias"];
                    if (!alias.is_string()) {
                        break;
                    }
                    Handle<Identifier> expressionIdentifier = NewIdentifier(expression, expression.string_value());
                    Handle<Identifier> indexIdentifier = NewIdentifier(index, index.string_value());
                    Handle<MemberExpression> indexMember = NewMemberExpression(repeat, MemberAccessKind::kIndex, expressionIdentifier, indexIdentifier);
                    forInit->Append(NewDeclaration(repeat, alias.string_value(), indexMember));
                    Handle<Identifier> lengthIdentifier = NewIdentifier(index, "length");
                    Handle<MemberExpression> lengthMember = NewMemberExpression(repeat, MemberAccessKind::kDot, expressionIdentifier, lengthIdentifier);
                    Handle<Expression> forCondition = NewBinaryExpression(repeat, BinaryOperation::kLessThan, indexIdentifier, lengthMember);
                    Handle<Expression> forUpdate = NewPrefixExpression(repeat, PrefixOperation::kIncrement, indexIdentifier);
                    Handle<Expression> forBlock = MakeHandle<BlockStatement>(json, NewExpressionList());
                    controlExpr = NewForStatement(repeat, ForKind::kForOf, forInit, forCondition, forUpdate, forBlock);
                    
                } while (0);
            }
            
        } while (0);
        
        return controlExpr;
    }
    Handle<Expression> ASTFactory::NewDeclaration(Json &json, std::string name, Handle<Expression> init) {
        return MakeHandle<Declaration>(json, name, init);
    }
    Handle<Expression> ASTFactory::NewDeclarationList(Json &json)
    {
        return MakeHandle<DeclarationList>(json);
    }
    Handle<Expression> ASTFactory::NewDeclarationList(Json &json, std::vector<Handle<Declaration>> decls)
    {
        return MakeHandle<DeclarationList>(json, std::move(decls));
    }
    Handle<Expression> ASTFactory::NewBinaryExpression(Json &json, BinaryOperation op, Handle<Expression> lhs, Handle<Expression> rhs)
    {
        return MakeHandle<BinaryExpression>(json, op, lhs, rhs);
    }
    Handle<Expression> ASTFactory::NewIntegralLiteral(Json &json, int value) {
        return MakeHandle<IntegralLiteral>(json, value);
    }
    Handle<Expression> ASTFactory::NewCallExpression(Json &json, MemberAccessKind kind, Handle<Expression> func, Handle<Expression> args) {
        return MakeHandle<CallExpression>(json, kind, func, args);
    }
    Handle<Expression> ASTFactory::NewCallExpression(Json &json, Handle<Expression>func, std::vector<Handle<Expression>> args) {
        return MakeHandle<CallExpression>(json, func, args);
    }
    Handle<Expression> ASTFactory::NewMemberExpression(Json &json, MemberAccessKind kind, Handle<Expression> expr, Handle<Expression> mem) {
        return MakeHandle<MemberExpression>(json, kind, expr, mem);
    }
    Handle<Expression> ASTFactory::NewPrefixExpression(Json &json, PrefixOperation op, Handle<Expression> expr)
    {
        return MakeHandle<PrefixExpression>(json, op, expr);
    }
    Handle<Expression> ASTFactory::NewForStatement(Json &json, ForKind kind, Handle<Expression> init, Handle<Expression> condition, Handle<Expression> update, Handle<Expression> body)
    {
        return MakeHandle<ForStatement>(json, kind, init, condition, update, body);
    }
    Handle<Expression> ASTFactory::NewObjectLiteral(Json &json, ProxyObject obj)
    {
        return MakeHandle<ObjectLiteral>(json, std::move(obj));
    }
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
