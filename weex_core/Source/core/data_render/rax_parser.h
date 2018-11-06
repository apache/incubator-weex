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

#ifndef DATA_RENDER_RAX_PARSER_
#define DATA_RENDER_RAX_PARSER_

#include "core/data_render/token.h"
#include "core/data_render/ast_factory.h"
#include "core/data_render/tokenizer.h"
#include "core/data_render/rax_source_locator.h"

namespace weex {
namespace core {
namespace data_render {
    
class ASTBuilder;

// ParserFlags := represents various parsing flags
class ParserFlags {
public:
    enum Flags {
        kNonRegex = 1 << 1,
        kAutomaticSemicolonInsertion = 1 << 2,
        kForInLoopParsing = 1 << 3,
    };
    
    ParserFlags() : flags_{ 0 } { }
    
    bool IsForInLoopParsing() { return flags_ & Flags::kForInLoopParsing; }
    void SetForInLoopParsing(bool flag) {
        flag ? flags_ |= Flags::kForInLoopParsing : flags_ ^= Flags::kForInLoopParsing;
    }
    
private:
    // flags are packed inside 64 bit unsigned integer
    uint64_t flags_;
};
    
class RAXParser {
public:
    friend class ForInLoopParsingEnvironment;
    RAXParser(ParserContext *ctx, ASTBuilder *builder, SourceLocator *locator, Tokenizer *lex, ScopeManager *manager);
    ~RAXParser();
    Handle<Expression> ParseProgram();
    Handle<Expression> ParsePrimary();
    Handle<Expression> ParseExpressionOptional();
    Handle<Expression> ParseExpression();
    Handle<Expression> ParseCommaExpression();
    Handle<Expression> ParseAssignExpression();
    Handle<Expression> ParseTernaryExpression();
    Handle<Expression> ParseBinaryExpression();
    Handle<Expression> ParseUnaryExpression();
    Handle<Expression> ParseBinaryExpressionRhs(int precedance, Handle<Expression> lhs);
    Handle<Expression> ParseLeftHandSideExpression();
    Handle<Expression> ParseNewExpression();
    Handle<Expression> ParseMemberExpression();
    Handle<Expression> ParseCallExpression();
    Handle<Expression> ParseDotExpression();
    Handle<Expression> ParseIndexExpression();
    Handle<ExpressionList> ParseArgumentList();
    Handle<Expression> ParseArrayConstant();
    Handle<Expression> ParseObjectConstant();
    Handle<Expression> ParseObjectMethod(const std::string &name);
    // function parser
    Handle<Expression> ParseFunctionStatement();
    std::vector<std::string> ParseParameterList();
    Handle<FunctionPrototype> ParseFunctionPrototype();
    
    Handle<Expression> ParseStatement();
    Handle<Expression> ParseBlockStatement();
    Handle<Expression> ParseVariableStatement();
    Handle<Expression> ParseIfStatement();
    Handle<Expression> ParseElseBranch();
    Handle<Expression> ParseContinueStatement();
    Handle<Expression> ParseSwitchStatement();
    Handle<Expression> ParseTryCatchStatement();
    Handle<Expression> ParseCaseBlock(bool is_default = false);
    Handle<Expression> ParseBreakStatement();
    Handle<Expression> ParseReturnStatement();
    Handle<Expression> ParseForStatement();
    Handle<Expression> ParseJSXNodeStatement();
    Handle<Expression> ParseJSXNodeExpression(Handle<Expression> parent = nullptr);
    Handle<Expression> ParseVariableOrExpressionOptional();
    Handle<Expression> ParseForInStatement(Handle<Expression> inexpr);
    Handle<Declaration> ParseDeclaration();
    Handle<Expression> ParseClassStatement();
    Handle<Expression> ParseClassBody(std::string &clsname);
    Handle<Expression> ParseClassMemberStatement(std::string &clsname);
    Handle<Expression> ParseJSXNodeProperty();
private:
    const std::string& GetIdentifierName();
    const std::string& GetStringConstant();

    double ParseNumber(const Token &token);
    Token::Type Peek();
    void Advance(bool not_regex = false);
    Tokenizer *lex() { return lex_; }
    ParserContext *context() { return ctx_; }
    ScopeManager *scope_manager() { return manager_; }
    SourceLocator *locator() { return locator_; }
    ScopeManager *manager() { return manager_; }
    ASTBuilder *builder() { return builder_; }
private:
    ASTBuilder *builder_;
    Tokenizer *lex_;
    ParserContext *ctx_;
    ScopeManager *manager_;
    SourceLocator *locator_;
    ParserFlags flags_;
    int nid_{0};
};

extern Handle<Expression> ParseProgram(RAXParser *parser);

}
}
}

#endif
