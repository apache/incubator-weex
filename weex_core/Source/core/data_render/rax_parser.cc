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

#include "core/data_render/rax_parser.h"
#include "core/data_render/common_error.h"
#include "core/data_render/rax_parser_context.h"
#include "core/data_render/rax_parser_statistics.h"
#include "core/data_render/ast_builder.h"
#include <sstream>
#include <string>
#include <cstdlib>
#include "base/LogDefines.h"

namespace weex {
namespace core {
namespace data_render {
    
#define EXPECT(tok)     \
do {    \
if (Peek() != tok)  \
throw SyntaxError(lex()->CurrentToken(),    \
std::string("expected a ") + Token::Str(tok)); \
Advance();  \
} while (0)
    
AssignOperation MapAssignOperation(Token& tok);
    
class ForInLoopParsingEnvironment {
public:
    ForInLoopParsingEnvironment(RAXParser *parser)
    : parser_{ parser }, prev_flags_{ parser->flags_ }
    {
        parser_->flags_.SetForInLoopParsing(true);
    }
    
    ~ForInLoopParsingEnvironment()
    {
        parser_->flags_ = prev_flags_;
    }
    
private:
    RAXParser *parser_;
    ParserFlags prev_flags_;
};
    
RAXParser::RAXParser(ParserContext *context, ASTBuilder *builder, SourceLocator *locator, Tokenizer *lex, ScopeManager *manager)
    : ctx_{ context }, builder_{ builder }, locator_{locator}, lex_{ lex }, manager_{ manager }
{
    
}
    
RAXParser::~RAXParser()
{
    
}
    
const std::string& RAXParser::GetIdentifierName() {
    return lex()->CurrentToken().view();
}
    
Token::Type RAXParser::Peek() {
    return lex()->Peek();
}
    
void RAXParser::Advance(bool not_regex) {
    lex()->Advance(not_regex);
}
    
Handle<Expression> RAXParser::ParseExpressionOptional()
{
    auto tok = Peek();
    if (tok == Token::SEMICOLON) {
        return builder()->NewUndefinedConstant();
    }
    else {
        return ParseExpression();
    }
}
    
Handle<Expression> RAXParser::ParseExpression()
{
    return ParseCommaExpression();
}
 
Handle<Expression> RAXParser::ParseAssignExpression()
{
    auto lhs = ParseTernaryExpression();
    auto tok = Peek();

    if (!Token::IsAssignmentOp(tok))
        return lhs;

    // TODO: information about what kind of assignment is done here should
    //  be stored here. (in the AST?)
    auto token = lex()->CurrentToken();
    AssignOperation op = MapAssignOperation(token);
    Advance();
    if (lhs->IsMemberExpression()) {
        lhs->AsMemberExpression()->is_assignment() = true;
    }
    auto rhs = ParseAssignExpression();
    Handle<AssignExpression>expr = builder()->NewAssignExpression((lhs), (rhs));
    expr->op() = op;
    return expr;
}
    
Handle<Expression> RAXParser::ParseTernaryExpression()
{
    auto first = ParseBinaryExpression();
    
    auto tok = Peek();
    if (tok != Token::CONDITIONAL) {
        return first;
    }
    
    // now we're parsing conditional expression
    // eat '?'
    Advance();
    auto second = ParseAssignExpression();
    
    tok = Peek();
    if (tok != Token::COLON) {
        throw SyntaxError(lex()->CurrentToken(), "expected a ':'");
    }
    
    // eat ':'
    Advance();
    auto third = ParseAssignExpression();
    
    return builder()->NewTernaryExpression(first, second, third);
}
    
Handle<Expression> RAXParser::ParseBinaryExpression()
{
    auto lhs = ParseUnaryExpression();
    
    // parse the rhs, if any
    return ParseBinaryExpressionRhs(3, lhs);
}

BinaryOperation MapBinaryOperator(Token& tok) {
    switch (tok.type()) {
        case Token::ADD:
            return BinaryOperation::kAddition;
        case Token::SUB:
            return BinaryOperation::kSubtraction;
        case Token::MUL:
            return BinaryOperation::kMultiplication;
        case Token::DIV:
            return BinaryOperation::kDivision;
        case Token::MOD:
            return BinaryOperation::kMod;
        case Token::SHL:
            return BinaryOperation::kShiftLeft;
        case Token::SAR:
            return BinaryOperation::kShiftRight;
        case Token::SHR:
            return BinaryOperation::kShiftZeroRight;
        case Token::LT:
            return BinaryOperation::kLessThan;
        case Token::GT:
            return BinaryOperation::kGreaterThan;
        case Token::LTE:
            return BinaryOperation::kLessThanEqual;
        case Token::GTE:
            return BinaryOperation::kGreaterThanEqual;
        case Token::EQ:
            return BinaryOperation::kEqual;
        case Token::NE:
            return BinaryOperation::kNotEqual;
        case Token::EQ_STRICT:
            return BinaryOperation::kStrictEqual;
        case Token::NE_STRICT:
            return BinaryOperation::kStrictNotEqual;
        case Token::AND:
            return BinaryOperation::kAnd;
        case Token::OR:
            return BinaryOperation::kOr;
        case Token::BIT_AND:
            return BinaryOperation::kBitAnd;
        case Token::BIT_OR:
            return BinaryOperation::kBitOr;
        case Token::BIT_XOR:
            return BinaryOperation::kBitXor;
        default:
            LOGE("unexpected token as binary operator %s", tok.view().c_str());
            throw SyntaxError(tok, "unexpected token as binary operator");
    }
    return BinaryOperation::kUnknown;
}
    
PrefixOperation MapTokenWithPrefixOperator(Token& tok) {
    switch (tok.type()) {
        case Token::INC:
            return PrefixOperation::kIncrement;
        case Token::DEC:
            return PrefixOperation::kDecrement;
        case Token::NOT:
            return PrefixOperation::kNot;
        case Token::UNFOLD:
            return PrefixOperation::kUnfold;
        case Token::TYPEOF:
            return PrefixOperation::kTypeof;
        default:
            LOGE("error prefix opration: %s", tok.view().c_str());
    }
    return PrefixOperation::kUnknown;
}
    
AssignOperation MapAssignOperation(Token& tok) {
    switch (tok.type()) {
        case Token::ASSIGN_ADD:
        return AssignOperation::kAssignAdd;
        case Token::ASSIGN:
        return AssignOperation::kAssign;
        case Token::ASSIGN_SUB:
        return AssignOperation::kAssignSub;
        default:
        LOGE("unexpected token as binary operator %s", tok.view().c_str());
        throw SyntaxError(tok, "unexpected token as binary operator");
    }
    return AssignOperation::kAssign;
}
    
Handle<Expression> RAXParser::ParseBinaryExpressionRhs(int prec, Handle<Expression> lhs)
{
    while (true) {
        int tokprec = Token::Precedence(Peek());
        
        if (tokprec < prec) {
            return lhs;
        }
        
        // now we definitely have a binary operator
        auto op =  MapBinaryOperator(lex()->CurrentToken());
        if (op == BinaryOperation::kLessThan && lhs->IsStringConstant()) {
            return lhs;
        }
        Advance();
        auto rhs = ParseUnaryExpression();
        
        auto nextprec = Token::Precedence(Peek());
        if (tokprec < nextprec) {
            rhs = ParseBinaryExpressionRhs(tokprec + 1, rhs);
        }
        
        
        // merge lhs and rhs
        lhs = builder()->NewBinaryExpression(op, lhs, rhs);
    }
}
    
Handle<Expression> RAXParser::ParseUnaryExpression()
{
    // UnaryExpression :
    //     PostfixExpression
    //     delete UnaryExpression
    //     void UnaryExpression
    //     typeof UnaryExpression
    //     ++ UnaryExpression
    //     -- UnaryExpression
    //     + UnaryExpression
    //     - UnaryExpression
    //     ~ UnaryExpression
    //     ! UnaryExpression
    auto tok = Peek();
    
    if (tok == Token::ADD) {
        Advance();
        // convert + (Expr) to Expr * 1
        return builder()->NewBinaryExpression(BinaryOperation::kMultiplication,
                                              ParseUnaryExpression(), builder()->NewIntegralConstant(1));
    }
    else if (tok == Token::SUB) {
        Advance();
        
        // similarly for `-Expr` to `Expr * -1`
        return builder()->NewBinaryExpression(BinaryOperation::kMultiplication,
                                              ParseUnaryExpression(), builder()->NewIntegralConstant(-1));
    }
    else if (tok == Token::INC || tok == Token::DEC || tok == Token::NOT || tok == Token::BIT_NOT
               || tok == Token::TYPEOF || tok == Token::DELETE || tok == Token::VOID || tok == Token::UNFOLD) {
        auto token = lex()->CurrentToken();
        lex()->Advance();
        return builder()->NewPrefixExpression(MapTokenWithPrefixOperator(token),
                                              ParseUnaryExpression());
    }
    else if (tok == Token::LT) {
        return ParseJSXNodeStatement();
    }
    
    // PostfixExpression :
    //      LeftHandSideExpression
    //      LeftHandSideExpression [no LineTerminator here] ++
    //      LeftHandSideExpression [no LineTerminator here] --
    auto left = ParseLeftHandSideExpression();
    
    tok = Peek();
    if (tok == Token::INC) {
        Advance();
        return builder()->NewPostfixExpression(PostfixOperation::kIncrement, left);
    }
    else if (tok == Token::DEC) {
        Advance();
        return builder()->NewPostfixExpression(PostfixOperation::kDecrement, left);
    }
    else if (tok == Token::ARROW_FUNCTION) {
        Advance();
        Handle <Expression> body;
        if(Peek() == Token::LBRACE){
            body = ParseStatement();
        } else{
            body = ParseExpression();
        }
        if (left) {
            return builder()->NewArrowFunctionStatement(body, {left});
        }
        else {
            return builder()->NewArrowFunctionStatement(body, {});
        }
    }
    else {
        return left;
    }
}
    
Handle<Expression> RAXParser::ParseLeftHandSideExpression()
{
    if (Peek() == Token::NEW) {
        return ParseNewExpression();
    }
    else {
        return ParseCallExpression();
    }
}
    
Handle<Expression> RAXParser::ParseNewExpression()
{
    if (Peek() != Token::NEW) {
        return ParseMemberExpression();
    }
    Advance();
    auto member = ParseNewExpression();
    return builder()->NewNewExpression(member);
}
    
Handle<Expression> RAXParser::ParseCallExpression()
{
    auto func = ParseMemberExpression();
    auto tok = Peek();
    
    if (tok != Token::PERIOD && tok != Token::LBRACK && tok != Token::LPAREN)
        return func;
    Handle<Expression> member = func, temp = nullptr;
    MemberAccessKind kind;
    
    while (true) {
        tok = Peek();
        if (tok == Token::PERIOD) {
            temp = ParseDotExpression();
            kind = MemberAccessKind::kDot;
        }
        else if (tok == Token::LBRACK) {
            temp = ParseIndexExpression();
            kind = MemberAccessKind::kIndex;
        }
        else if (tok == Token::LPAREN) {
            auto args = ParseArgumentList();
            temp = builder()->NewArgumentList(args);
            kind = MemberAccessKind::kCall;
        }
        else {
            break;
        }
        if (member->IsIdentifier()){
            member = builder()->NewCallExpression(member, temp);
        } else{
            member = builder()->NewCallExpression(kind, member, temp);
        }
    }
    return member;
}
    
Handle<Expression> RAXParser::ParseDotExpression()
{
    // eat the '.'
    Advance();
    
    auto tok = Peek();
    
    // this token should be a valid identifier
    if (tok != Token::IDENTIFIER && !Token::IsKeyword(tok))
        throw SyntaxError(lex()->CurrentToken(), "expected a valid identifier");
    auto name = GetIdentifierName();
    auto ident = builder()->NewIdentifier(name);
    Advance(true);
    return ident;
}
    
Handle<Expression> RAXParser::ParseIndexExpression()
{
    // eat the '['
    Advance();
    auto expr = ParseAssignExpression();
    if (lex()->Peek() != Token::RBRACK)
        throw SyntaxError(lex()->CurrentToken(), "expected a ']'");
    
    Advance(true); // consumex ']'
    return expr;
}
    
double RAXParser::ParseNumber(const Token &token)
{
    try {
        double num = std::strtod(token.view().c_str(), nullptr);
        return num;
    }
    catch(std::exception &e) {
        throw SyntaxError(token, "invalid number (" + token.view() + ")");
    }
}
    
Handle<Expression> RAXParser::ParsePrimary()
{
    auto tok = Peek();
    Handle<Expression> result;
    
    if (tok == Token::NULL_LITERAL) {
        result = builder()->NewNullConstant();
    }
    else if (tok == Token::NUMBER) {
        result = builder()->NewDoubleConstant(ParseNumber(lex()->CurrentToken()));
    }
    else if (tok == Token::TEMPLATE) {
        // can't support
        throw SyntaxError(lex()->CurrentToken(), "can't support template");
    }
    else if (tok == Token::REGEXP_LITERAL) {
        result = builder()->NewRegexConstant(lex()->CurrentToken().view());
    }
    else if (tok == Token::STRING) {
        result = builder()->NewStringConstant(lex()->CurrentToken().view());
    }
    else if (tok == Token::INTEGER) {
        result = builder()->NewIntegralConstant(atoi((lex()->CurrentToken().view()).c_str()));
    }
    else if (tok == Token::IDENTIFIER) {
        result = builder()->NewIdentifier(lex()->CurrentToken().view());
    }
    else if (tok == Token::TRUE_LITERAL) {
        result = builder()->NewBooleanConstant(true);
    }
    else if (tok == Token::FALSE_LITERAL) {
        result = builder()->NewBooleanConstant(false);
    }
    else if (tok == Token::THIS) {
        result = builder()->NewThisExpression();
    }
    else if (tok == Token::LPAREN) {
        Advance();    // eat '('
        tok = Peek();
        if (tok != Token::RPAREN) {
            result = ParseCommaExpression();
        }
        tok = Peek();
        if (tok != Token::RPAREN) {
            throw SyntaxError(lex()->CurrentToken(), "expected a ')'");
        }
    }
    else if (tok == Token::LBRACK) {
        result = ParseArrayConstant();
    }
    else if (tok == Token::LBRACE) {
        result = ParseObjectConstant();
    }
    else if (tok == Token::FUNCTION) {
        result = ParseFunctionStatement();
        return result;
    }
    else if (tok == Token::SUPER) {
        result = builder()->NewIdentifier(lex()->CurrentToken().view());
    }
    else if (tok == Token::UNDEFINED) {
        result = builder()->NewUndefinedConstant();
    }
    else {
        throw SyntaxError(lex()->CurrentToken(), "expected a primary expression");
    }
    
    Advance(true);
    return result;
}
    
Handle<Expression> RAXParser::ParseArrayConstant()
{
    ProxyArray exprs;
    // eat '['
    Advance();
    auto tok = Peek();
    
    if (tok == Token::RBRACK) {
        // done
        return builder()->NewArrayConstant(exprs);
    }
    while (true) {
        auto one = ParseAssignExpression();
        exprs.push_back(Handle<Expression>(one));
        
        tok = Peek();
        
        if (tok == Token::RBRACK)
            break;
        EXPECT(Token::COMMA);
    }
    
    return builder()->NewArrayConstant(exprs);
}
   
const std::string& RAXParser::GetStringConstant()
{
    return lex()->CurrentToken().view();
}
    
Handle<Expression> RAXParser::ParseObjectConstant()
{
    ProxyObject proxy;
    ProxyArray spread_property;
    std::vector<std::pair<ProxyOrder, std::string>> orders;
    // eat the left brace '{'
    Advance();
    auto tok = Peek();
    
    // if next tok is '}' then nothing to be done
    if (tok == Token::RBRACE) {
        return builder()->NewObjectConstant(proxy);
    }
    
    std::string name;
    Handle<Expression> prop;
    while (true) {
        tok = Peek();
        if (tok == Token::STRING) {
            name = GetStringConstant();
        }
        else if (tok == Token::IDENTIFIER || Token::IsKeyword(tok) || tok == Token::NUMBER) {
            name = lex()->CurrentToken().view();
        }
        else if (tok == Token::UNFOLD) {
            Handle<Expression> unfold_expr = ParseAssignExpression();
            if (Peek() == Token::COMMA) {
                spread_property.push_back(unfold_expr);
                orders.push_back(std::make_pair(ProxyOrder::ProxyArray, base::to_string((int)spread_property.size() - 1)));
                Advance();
                continue;
            }
            else if (spread_property.size() > 0) {
                spread_property.push_back(unfold_expr);
                orders.push_back(std::make_pair(ProxyOrder::ProxyArray, base::to_string((int)spread_property.size() - 1)));
                break;
            }
            return unfold_expr;
        }
        else {
            throw SyntaxError(lex()->CurrentToken(), "expected an Identifier or a string");
        }
        
        Advance();
        
        if (Peek() == Token::COLON) {
            Advance();
            prop = ParseAssignExpression();
        }
        else if (Peek() == Token::LPAREN) {
            prop = ParseObjectMethod(name);
        }
        else
            // TODO: create a getter list in the ProxyObject class that will keep track of getters
            // and setters
            if (Peek() == Token::IDENTIFIER && (name == "get" || name == "set")) {
                name = lex()->CurrentToken().view();
                Advance();
                prop = ParseObjectMethod(name);
            }
        
        if (prop == NULL) {
            prop = builder()->NewIdentifier(name);
        }
        proxy[name] = Handle<Expression>(prop);
        orders.push_back(std::make_pair(ProxyOrder::ProxyObject, name));

        // next token should be a ',' or '}'
        tok = Peek();
        if (tok == Token::RBRACE)
            break;
        if (tok != Token::COMMA)
            throw SyntaxError(lex()->CurrentToken(), "expected a ',' or '}'");
        Advance();
    }
    
    Handle<Expression> obj_expr = builder()->NewObjectConstant(proxy);
    if (spread_property.size() > 0) {
        for (int i = 0; i < spread_property.size(); i++) {
            obj_expr->AsObjectConstant()->SpreadProperty().push_back(spread_property[i]);
        }
        obj_expr->AsObjectConstant()->Orders() = orders;
    }
    return obj_expr;
}
    
Handle<Expression> RAXParser::ParseObjectMethod(const std::string &name)
{
    auto args = ParseParameterList();
    auto body = ParseBlockStatement();
    auto proto = builder()->NewFunctionPrototype(name, args);
    return builder()->NewFunctionStatement(proto->AsFunctionPrototype(), body);
}
    
std::vector<std::string> RAXParser::ParseParameterList()
{
    auto tok = Peek();
    auto result = std::vector<std::string>();
    
    if (tok != Token::LPAREN)
        throw SyntaxError(lex()->CurrentToken(), "expected a '('");
    Advance();
    
    // check for ')'
    tok = Peek();
    if (tok == Token::RPAREN) {
        Advance();
        return { }; // return an empty vector
    }
    
    while (true) {
        tok = Peek();
        
        if (tok != Token::IDENTIFIER)
            throw SyntaxError(lex()->CurrentToken(), "expected an identifier");
        
        result.push_back(GetIdentifierName());
        Advance();
        
        tok = Peek();
        if (tok == Token::RPAREN)
            break;
        
        if (tok != Token::COMMA)
            throw SyntaxError(lex()->CurrentToken(), "expected a ',' or ')'");
        Advance();
    }
    
    // eat the ')'
    Advance();
    return result;
}

Handle<FunctionPrototype> RAXParser::ParseFunctionPrototype()
{
    // eat 'function'
    Advance();
    auto tok = Peek();
    
    std::string name;
    if (tok == Token::IDENTIFIER) {
        name = GetIdentifierName();
        // eat the IDENT
        Advance();
    }
    
    // parse the argument list
    auto args = ParseParameterList();
    return builder()->NewFunctionPrototype(name, args)->AsFunctionPrototype();
}

Handle<Expression> RAXParser::ParseFunctionStatement()
{
    auto proto = ParseFunctionPrototype();
    auto body = ParseStatement();
    
    return builder()->NewFunctionStatement(proto, body);
}

Handle<Expression> RAXParser::ParseBlockStatement()
{
    Handle<ExpressionList> stmts = builder()->NewExpressionList();
    Advance(); // eat '{'
    
    while (true) {
        auto tok = Peek();
        if (tok == Token::RBRACE)
            break;
        
        auto stmt = ParseStatement();
        stmts->Insert(stmt);
    }
    
    Advance(); // eat '}'
    return builder()->NewBlockStatement(stmts);
}

Handle<Expression> RAXParser::ParseReturnStatement()
{
    Advance(); // eat 'return'
    auto tok = Peek();
    
    if (tok == Token::SEMICOLON) {
        return builder()->NewReturnStatement(nullptr);
    }
    if (tok == Token::LPAREN) {
        Advance();
        tok = Peek();
        Handle<Expression> content;
        if (tok == Token::LT) {
            content = ParseJSXNodeStatement();
        }
        else {
            content = ParseExpression();
        }
        EXPECT(Token::RPAREN); // eat ')'
        auto stmt = builder()->NewReturnStatement(content);
        EXPECT(Token::SEMICOLON);
        return stmt;
    }
    auto expr = ParseCommaExpression();
    tok = Peek();
    
    if (tok != Token::SEMICOLON)
        throw SyntaxError(lex()->CurrentToken(), "expected a ';'");
    Advance();
    return builder()->NewReturnStatement(expr);
}
    
Handle<Expression> RAXParser::ParseMemberExpression()
{
    Handle<Expression> primary = nullptr;
    if (Peek() == Token::NEW) {
        Advance();
        primary = builder()->NewNewExpression(ParseMemberExpression());
    }
    else {
        primary = ParsePrimary();
    }
    // if next token is neither '[' or '.'
    if (Peek() != Token::LBRACK && Peek() != Token::PERIOD && Peek() != Token::LPAREN)
        return primary;
    
    Handle<Expression> temp;
    Handle<Expression> member = primary;
    MemberAccessKind kind;
    while (true) {
        if (Peek() == Token::PERIOD) {
            temp = ParseDotExpression();
            kind = MemberAccessKind::kDot;
        }
        else if (Peek() == Token::LBRACK) {
            temp = ParseIndexExpression();
            kind = MemberAccessKind::kIndex;
        }
        else if (Peek() == Token::LPAREN) {
            auto args = ParseArgumentList();
            temp = builder()->NewArgumentList(args);
            kind = MemberAccessKind::kCall;
        }
        else {
            break;
        }
        if (kind == MemberAccessKind::kCall) {
            if (member->IsMemberExpression()) {
                member->AsMemberExpression()->setKind(kind);
            }
            if (member->IsIdentifier() && member->AsIdentifier()->GetName() != "super")
            {
                member = builder()->NewCallExpression(member, temp);
            } else{
                member = builder()->NewCallExpression(kind, member, temp);
            }
        }
        else {
            member = builder()->NewMemberExpression(kind, member, temp);
        }
    }
    
    return member;
}
    
Handle<ExpressionList> RAXParser::ParseArgumentList()
{
    Handle<ExpressionList> exprs = builder()->NewExpressionList();
    
    auto tok = Peek();
    EXPECT(Token::LPAREN);
    
    tok = Peek();
    if (tok == Token::RPAREN) {
        Advance(true);
        return { };
    }
    
    while (true) {
        auto one = ParseAssignExpression();
        exprs->Insert(one);
        
        tok = Peek();
        if (tok == Token::RPAREN)
            break;
        if (tok != Token::COMMA)
            throw SyntaxError(lex()->CurrentToken(), "expected a ',' or ')'");
        Advance();
    }
    
    Advance(true); // eat the last ')'
    return exprs;
}

Handle<Expression> RAXParser::ParseCommaExpression()
{
    auto one = ParseAssignExpression();
    auto tok = lex()->Peek();
    
    // if we have a comma ',', then we definitely have to parse
    // expr of type (expr, expr*)
    if (tok != Token::COMMA)
        return one;
    
    Handle<ExpressionList> exprs = builder()->NewExpressionList();
    exprs->Insert(one);
    
    // loop until we don't find a ','
    while (true) {
        Advance();
        one = ParseAssignExpression();
        exprs->Insert(one);
        
        tok = Peek();
        if (tok != Token::COMMA)
            break;
    }
    
    return builder()->NewCommaExpression(exprs);
}
    
Handle<Expression> RAXParser::ParseJSXNodeStatement() {
    auto tok = Peek();
    if (tok == Token::LPAREN) {
        Advance(); // eat '('
    }
    EXPECT(Token::LT); // eat '<'
    if (Peek() != Token::IDENTIFIER) {
        throw SyntaxError(lex()->CurrentToken(), "expected a identifier name");
    }
    Handle<Expression>expr = ParseJSXNodeExpression();
    if (Peek() == Token::JSX_TAG_END || Peek() == Token::GT) {
        Advance(); // eat '>' or '/>'
    }
    if (tok == Token::LPAREN && Peek() == Token::RPAREN) {
        Advance(); // eat ')'
    }
    return expr;
}
    
Handle<Expression> RAXParser::ParseJSXNodeProperty() {
    Handle<Expression> props = nullptr;
    ProxyObject proxy;
    ProxyArray spread_property;
    std::vector<std::pair<ProxyOrder, std::string>> orders;
    std::string key;
    do {
        Handle<Expression> prop;
        while (true) {
            auto tok = Peek();
            if (tok == Token::STRING) {
                key = GetStringConstant();
            }
            else if (tok == Token::IDENTIFIER || Token::IsKeyword(tok) || tok == Token::NUMBER) {
                key = lex()->CurrentToken().view();
            }
            else if (tok == Token::UNFOLD) {
                Handle<Expression> unfold_expr = ParseAssignExpression();
                spread_property.push_back(unfold_expr);
                orders.push_back(std::make_pair(ProxyOrder::ProxyArray, base::to_string((int)spread_property.size() - 1)));
                continue;
            }
            else if (tok == Token::RBRACE || tok == Token::LBRACE) {
                Advance();
                continue;
            }
            else if (tok == Token::JSX_TAG_END) {
                break;
            }
            else {
                throw SyntaxError(lex()->CurrentToken(), "expected an Identifier or a string");
            }
            Advance();
            if (Peek() == Token::ASSIGN) {
                Advance();
                EXPECT(Token::LBRACE);
                prop = ParseAssignExpression();
                EXPECT(Token::RBRACE);
            }
            if (prop == NULL) {
                prop = builder()->NewIdentifier(key);
            }
            proxy[key] = Handle<Expression>(prop);
            orders.push_back(std::make_pair(ProxyOrder::ProxyObject, key));
        }
        
    } while (0);
    
    if (proxy.size() > 0 || spread_property.size() > 1) {
        props = builder()->NewObjectConstant(proxy);
        if (spread_property.size() > 0) {
            for (int i = 0; i < spread_property.size(); i++) {
                props->AsObjectConstant()->SpreadProperty().push_back(spread_property[i]);
            }
            props->AsObjectConstant()->Orders() = orders;
        }
    }
    else {
        props = spread_property[0];
    }
    return props;
}
    
Handle<Expression> RAXParser::ParseJSXNodeExpression(Handle<Expression> parent) {
    Handle<Expression> expr = nullptr;
    Handle<Expression> props = nullptr;
    std::string name = GetIdentifierName();
    Advance(); // eat 'name'
    auto tok = Peek();
    // props process
    if (tok == Token::LBRACE) {
        props = ParseJSXNodeProperty();
    }
    else if (tok == Token::IDENTIFIER) {
        // props process
        std::vector<std::pair<ProxyOrder, std::string>> orders;
        ProxyArray spread_property;
        ProxyObject proxyObj;
        while (true) {
            auto token = Peek();
            if (token == Token::JSX_TAG_END || token == Token::GT) {
                break;
            }
            std::string key;
            if (token == Token::IDENTIFIER) {
                while (Peek() == Token::SUB || Peek() == Token::IDENTIFIER) {
                    key += lex_->CurrentToken().view();
                    Advance();
                }
                EXPECT(Token::ASSIGN);
            }
            if (Peek() == Token::LBRACE) {
                if (key.length() > 0) {
                    EXPECT(Token::LBRACE);
                    proxyObj.insert(std::make_pair(key, ParseAssignExpression()));
                    orders.push_back(std::make_pair(ProxyOrder::ProxyObject, key));
                    EXPECT(Token::RBRACE);
                }
                else {
                    Advance();
                    token = Peek();
                    if (token == Token::UNFOLD) {
                        Handle<Expression> unfold_expr = ParseAssignExpression();
                        spread_property.push_back(unfold_expr);
                        orders.push_back(std::make_pair(ProxyOrder::ProxyArray, base::to_string((int)spread_property.size() - 1)));
                    }
                    EXPECT(Token::RBRACE);
                    continue;
                }
            }
            else {
                std::string value = GetIdentifierName();
                EXPECT(Token::STRING);
                proxyObj.insert(std::make_pair(key, builder()->NewStringConstant(value)));
                orders.push_back(std::make_pair(ProxyOrder::ProxyObject, key));
            }
        }
        props = builder()->NewObjectConstant(proxyObj);
        if (spread_property.size() > 0) {
            for (int i = 0; i < spread_property.size(); i++) {
                props->AsObjectConstant()->SpreadProperty().push_back(spread_property[i]);
            }
            props->AsObjectConstant()->Orders() = orders;
        }
    }
    tok = Peek();
    expr = builder()->NewJSXNodeExpression(builder()->NewIdentifier(name), props, parent, {});
    if (tok == Token::JSX_TAG_END) {
        // no childrens
//        Advance(); // eat '/'
    }
    else if (tok == Token::GT) {
        Advance();
        while (true) {
            tok = Peek();
            if (tok != Token::LBRACE && tok != Token::LT && tok != Token::JSX_TAG_CLOSE && tok != Token::STRING && tok != Token::IDENTIFIER) {
                throw SyntaxError(lex()->CurrentToken(), "expected a string name");
                break;
            }
            if (tok == Token::IDENTIFIER) {
                expr->AsJSXNodeExpression()->childrens().push_back(builder()->NewStringConstant(GetIdentifierName()));
                Advance();
            }
            else if (tok == Token::STRING) {
                expr->AsJSXNodeExpression()->childrens().push_back(ParseExpression());
            }
            else if (tok == Token::LBRACE) {
                Advance(); // eat '{'
                Handle<Expression> children = ParseExpression();
                expr->AsJSXNodeExpression()->childrens().push_back(children);
                EXPECT(Token::RBRACE);
            }
            else if (tok == Token::LT) {
                Advance(); // eat '<'
                tok = Peek();
                if (tok == Token::IDENTIFIER) {
                    Handle<Expression> children = ParseJSXNodeExpression(expr);
                    expr->AsJSXNodeExpression()->childrens().push_back(children);
                    if (Peek() == Token::JSX_TAG_END || Peek() == Token::GT) {
                        Advance(); // eat '>' or '/>'
                    }
                }
            } else if(tok == Token::JSX_TAG_CLOSE){
                Advance();
                auto tok = Peek();
                if (tok != Token::IDENTIFIER) {
                    throw SyntaxError(lex()->CurrentToken(), "expected a identifier name");
                }
                if (name != GetIdentifierName()) {
                    throw SyntaxError(lex()->CurrentToken(), "expected identifier name not equal");
                }
                Advance(); // eat 'Identifier'
                break;
            }
        }
    }
    return expr;
}

Handle<Expression> RAXParser::ParseSwitchStatement()
{
    Advance();
    EXPECT(Token::LPAREN);

    Handle<Expression> expr = ParseAssignExpression();
    EXPECT(Token::RPAREN);
    EXPECT(Token::LBRACE);

    bool has_default = false;
    std::vector<Handle<Expression>> list;
    while (true) {
        Handle<Expression> temp = nullptr;
        if (Peek() == Token::CASE) {
            Handle<Expression> tempList = ParseCaseBlock();

            list.push_back(tempList);
        } else if (Peek() == Token::DEFAULT) {
            if (has_default) {
                throw SyntaxError(lex()->CurrentToken(),"switch statement has already has one default case");
            }
            Handle<Expression> tempList = ParseCaseBlock(true);
            has_default = true;
            list.push_back(tempList);
        } else if (Peek() != Token::RBRACE) {
            throw SyntaxError(lex()->CurrentToken(), "expected a '}'");
        } else {
            Advance();
            break;
        }
    }

    return builder()->NewSwitchStatement(expr, list);
}

Handle<Expression> RAXParser::ParseCaseBlock(bool is_default) {
    Advance();

    Handle<Expression> clause;
    if (!is_default) {
        clause = ParseAssignExpression();
    }
    EXPECT(Token::COLON);

    Handle<ExpressionList> list = builder()->NewExpressionList();

    if (Peek() != Token::CASE && Peek() != Token::DEFAULT && Peek() != Token::RBRACE){
        //not empty case;
        do {
            Handle<Expression> stmt = ParseStatement();
            list->Insert(stmt);
        } while (Peek() != Token::CASE && Peek() != Token::DEFAULT && Peek() != Token::RBRACE);
    }

    auto block = builder()->NewCaseStatement(clause, list);
    block->AsCaseStatement()->set_default(is_default);
    return block;
}
    
Handle<Expression> RAXParser::ParseBreakStatement()
{
    Advance();
    Handle<Expression> label = nullptr;
    if (Peek() == Token::IDENTIFIER) {
        label = builder()->NewIdentifier(GetIdentifierName());
    }
    EXPECT(Token::SEMICOLON);
    return builder()->NewBreakStatement(label);
}
    
Handle<Expression> RAXParser::ParseContinueStatement()
{
    Advance();
    Handle<Expression> label = nullptr;
    if (Peek() == Token::IDENTIFIER) {
        label = builder()->NewIdentifier(GetIdentifierName());
    }    
    return builder()->NewContinueStatement(label);
}

Handle<Expression> RAXParser::ParseStatement()
{
    auto tok = Peek();
    switch (tok) {
        default:
        {
            auto result = ParseExpressionOptional();
            tok = Peek();
            if (tok == Token::COLON && result->IsIdentifier()) {
                auto label = builder()->NewLabelledStatement(result->AsIdentifier()->GetName(), result);
                Advance();
                return label;
            }
            if (tok != Token::SEMICOLON)
                throw SyntaxError(lex()->CurrentToken(), "expected a ';'");
            Advance();
            return result;
        }
        case Token::IF:
            return ParseIfStatement();
        case Token::FOR:
            return ParseForStatement();
        case Token::FUNCTION:
            return ParseFunctionStatement();
        case Token::LBRACE:
            return ParseBlockStatement();
        case Token::RETURN:
            return ParseReturnStatement();
        case Token::CLASS:
            return ParseClassStatement();
        case Token::CONST:
        {
            Handle<DeclarationList> decl_list = ParseVariableStatement();
            if (decl_list->IsDeclarationList()) {
                for (int i = 0; i < decl_list->exprs().size(); i++) {
                    Handle<Declaration> decl = decl_list->exprs()[i];
                    decl->SetKind(DeclarationKind::kConst);
                }
            }
            return decl_list;
        }
        case Token::VAR:
        case Token::LET:
            return ParseVariableStatement();
        case Token::BREAK:
            return ParseBreakStatement();
        case Token::CONTINUE:
            return ParseContinueStatement();
        case Token::SWITCH:
            return ParseSwitchStatement();
        case Token::TRY:
            return ParseTryCatchStatement();
//        case THROW:
//            return ParseThrowStatement();
    }
}
    
Handle<Expression> RAXParser::ParseClassStatement() {
    EXPECT(Token::CLASS);
    auto tok = Peek();
    if (tok != Token::IDENTIFIER) {
        throw SyntaxError(lex()->CurrentToken(), "expected a class identifier name");
    }
    Handle<Expression> clssuper_expr = nullptr;
    std::string clsname = GetIdentifierName();
    Advance();
    tok = Peek();
    if (tok == Token::EXTENDS) {
        Advance();
        tok = Peek();
        if (tok != Token::IDENTIFIER) {
            throw SyntaxError(lex()->CurrentToken(), "expected a super class identifier name");
        }
        clssuper_expr = builder()->NewIdentifier(GetIdentifierName());
        Advance();
    }
    return builder()->NewClassStatement(builder()->NewIdentifier(clsname), clssuper_expr, ParseClassBody(clsname));
}
    
Handle<Expression> RAXParser::ParseClassBody(std::string &clsname) {
    Handle<ClassBody> clsbody = builder()->NewClassBody();
    EXPECT(Token::LBRACE);
    while (true) {
        auto one = ParseClassMemberStatement(clsname);
        clsbody->Insert(one);
        auto tok = Peek();
        if (tok == Token::SEMICOLON) {
            Advance();
            tok = Peek();
        }
        if (tok == Token::RBRACE) {
            break;
        }
    }
    EXPECT(Token::RBRACE);
    return clsbody;
}
    
Handle<Expression> RAXParser::ParseClassMemberStatement(std::string &clsname) {
    auto tok = Peek();
    bool is_static = false;
    if (tok == Token::STATIC) {
        is_static = true;
        Advance();
        tok = Peek();
    }
    if (tok != Token::IDENTIFIER) {
        throw SyntaxError(lex()->CurrentToken(), "expected a method identifier name");
    }
    std::string identifier = GetIdentifierName();
    Advance();
    tok = Peek();
    if (tok == Token::ASSIGN) {
        Advance();
        Handle<Expression> expr = ParseAssignExpression();
        if (expr->IsArrowFunctionStatement()) {
            expr->AsArrowFunctionStatement()->name() = identifier;
            return expr;
        }
        else {
            Handle<ClassProperty> props = builder()->NewClassProperty(identifier, expr);
            props->set_is_static(is_static);
            return props;
        }
    }
    else {
        auto args = ParseParameterList();
        auto body = ParseBlockStatement();
        auto proto = builder()->NewFunctionPrototype(identifier, args);
        proto->AsFunctionPrototype()->SetClassName(clsname);
        return builder()->NewFunctionStatement(proto->AsFunctionPrototype(), body);
    }
}
    
Handle<Expression> RAXParser::ParseIfStatement()
{
    Handle<Expression> result;
    
    // eat 'if'
    Advance();
    
    auto tok = Peek();
    EXPECT(Token::LPAREN);
    
    // parse the condition of if statement
    auto condition = ParseCommaExpression();
    EXPECT(Token::RPAREN);
    
    // parse the body of 'if'
    auto body = ParseStatement();
    
    tok = Peek();
    if (tok == Token::ELSE) {
        result = builder()->NewIfElseStatement(condition, body, ParseElseBranch());
    }
    else {
        result = builder()->NewIfStatement(condition, body);
    }    
    return result;
}
    
Handle<Expression> RAXParser::ParseVariableOrExpressionOptional() {
    auto tok = Peek();
    if (tok == Token::VAR || tok == Token::LET || tok == Token::CONST) {
        return ParseVariableStatement();
    }
    return ParseExpressionOptional();
}
    
Handle<Expression> RAXParser::ParseForStatement()
{
    // eat 'for'
    Advance();
    auto tok = Peek();
    EXPECT(Token::LPAREN);
    
    // parse 'for ( >>this<< ;...' part
    ForInLoopParsingEnvironment env(this);
    auto init = ParseVariableOrExpressionOptional();
    
    if (Peek() == Token::IN) {
        Advance();
        auto expr = ParseAssignExpression();
        /* this is bit wrong as ast for this kind of syntax will be like
         *
         *          in
         *         /  \
         *        /     \
         * DeclList     expr
         *
         * We need to simplify the ast to simple `a in x` expression
         */
        init = builder()->NewBinaryExpression(BinaryOperation::kIn, init, expr);
        return ParseForInStatement(init);
    }
    if (init && init->IsBinaryExpression()) {
        auto op = init->AsBinaryExpression()->op();
        if (op == BinaryOperation::kIn) {
            return ParseForInStatement(init);
        }
    }
    
    if (init && !init->IsDeclarationList()) {
        EXPECT(Token::SEMICOLON);
    }
    
    if (!init) {
        EXPECT(Token::SEMICOLON);
    }
    
    Handle<Expression> condition;
    if (Peek() == Token::SEMICOLON) {
        condition = builder()->NewBooleanConstant(true);
    }
    else {
        // parse 'for (x = 10; >>this<< ...' part
        condition = ParseCommaExpression();
    }
    
    tok = Peek();
    if (tok != Token::SEMICOLON)
        throw SyntaxError(lex()->CurrentToken(), "expected a ';'");
    Advance();
    
    Handle<Expression> update;
    if (Peek() != Token::RPAREN) {
        // parse 'for (x = 10; x < 100; >>this<<...' part
        update = ParseCommaExpression();
    } else {
        update = builder()->NewUndefinedConstant();
    }
    
    tok = Peek();
    if (tok != Token::RPAREN)
        throw SyntaxError(lex()->CurrentToken(), "expected a ')'");
    Advance();
    
    // parse 'for (x = 10; x < 100; x = x + 1) >>rest<<...' part
    auto body = ParseStatement();
    return builder()->NewForStatement(ForKind::kForOf, init, condition, update, body);
}
    
Handle<Expression> RAXParser::ParseForInStatement(Handle<Expression> inexpr)
{
    EXPECT(Token::RPAREN);
    
    // parse 'for (x = 10; x < 100; x = x + 1) >>rest<<...' part
    auto body = ParseStatement();
    return builder()->NewForStatement(ForKind::kForIn, inexpr, nullptr, nullptr, body);
}
    
Handle<Expression> RAXParser::ParseElseBranch()
{
    // eat 'else'
    Advance();
    return ParseStatement();
}
    
Handle<Expression> RAXParser::ParseProgram()
{
    Handle<ExpressionList> exprs = builder()->NewExpressionList();
    Handle<ChunkStatement> chunk = builder()->NewChunkStatement(exprs);
    exprs->Insert(builder()->NewDeclaration(JS_GLOBAL_VNODE_INDEX, builder()->NewIntegralConstant(1)));
    exprs->Insert(builder()->NewDeclaration(JS_GLOBAL_ARGUMENTS, builder()->NewArrayConstant({})));
    try {
        while (Peek() != Token::EOS) {
            exprs->Insert(ParseStatement());
        }
    } catch (std::exception &e) {
        auto error = static_cast<SyntaxError *>(&e);
        if (error) {
            std::cerr << error->what() << std::endl;
        }
        throw;
    }
    return chunk;
}
    
Handle<Declaration> RAXParser::ParseDeclaration()
{
    auto tok = Peek();
    if (tok == Token::IDENTIFIER) {
        std::string name = GetIdentifierName();
        Advance();
        tok = Peek();
        if (tok == Token::SEMICOLON || tok == Token::COMMA || (flags_.IsForInLoopParsing() && tok == Token::IN)) {
            return builder()->NewDeclaration(name);
        }
        else if (tok != Token::ASSIGN) {
            throw SyntaxError(lex()->CurrentToken(), "expected a '='");
        }
        Advance();
        return builder()->NewDeclaration(name, ParseAssignExpression());
    }
    else if (tok == Token::LBRACE) {
        Advance();
        ProxyObject proxyObj;
        while (true) {
            tok = Peek();
            if (tok == Token::RBRACE) {
                break;
            }
            if (tok != Token::IDENTIFIER) {
                throw SyntaxError(lex()->CurrentToken(), "expected an identifier");
            }
            Handle<Expression> expr = ParseExpression();
            tok = Peek();
            if (tok == Token::COLON) {
                if (!expr->IsIdentifier()) {
                    throw SyntaxError(lex()->CurrentToken(), ": before expected an identifier");
                }
                std::string key_name = expr->AsIdentifier()->GetName();
                Advance();
                if (Peek() != Token::IDENTIFIER) {
                    throw SyntaxError(lex()->CurrentToken(), "expected an identifier");
                }
                std::string value_name = GetIdentifierName();
                Advance();
                proxyObj.insert(std::make_pair(key_name, builder()->NewIdentifier(value_name)));
            }
            else if (expr->IsIdentifier()) {
                std::string name = expr->AsIdentifier()->GetName();
                proxyObj.insert(std::make_pair(name, builder()->NewIdentifier(name)));
            }
            else if (expr->IsCommaExpression()) {
                Handle<ExpressionList> exprs = expr->AsCommaExpression()->exprs();
                for (int i = 0; i < exprs->Size(); i++) {
                    if (exprs->raw_list()[i]->IsIdentifier()) {
                        std::string name = exprs->raw_list()[i]->AsIdentifier()->GetName();
                        proxyObj.insert(std::make_pair(name, builder()->NewIdentifier(name)));
                    }
                    else if (exprs->raw_list()[i]->IsAssignExpression()) {
                        std::string name = exprs->raw_list()[i]->AsAssignExpression()->lhs()->AsIdentifier()->GetName();
                        proxyObj.insert(std::make_pair(name, exprs->raw_list()[i]));
                    }
                }
            }
            tok = Peek();
            if (tok != Token::COMMA) {
                break;
            }
            Advance();
        }
        EXPECT(Token::RBRACE);
        tok = Peek();
        Handle<Expression> expr = builder()->NewObjectConstant(proxyObj);
        if (tok == Token::SEMICOLON || tok == Token::COMMA || (flags_.IsForInLoopParsing() && tok == Token::IN)) {
            return builder()->NewDeclaration(expr);
        }
        else if (tok != Token::ASSIGN) {
            throw SyntaxError(lex()->CurrentToken(), "expected a '='");
        }
        Advance();
        return builder()->NewDeclaration(expr, ParseAssignExpression());
    }
    else {
        throw SyntaxError(lex()->CurrentToken(), "expected an identifier or {");
    }
}
    
Handle<Expression> RAXParser::ParseVariableStatement()
{
    Advance();    // eat 'var'
    
    std::vector<Handle<Declaration>> decl_list;
    while (true) {
        decl_list.push_back(Handle<Declaration>(ParseDeclaration()));

        auto tok = Peek();
        if (tok == Token::SEMICOLON) {
            Advance();
            break;
        }
        else if (tok == Token::IN || tok == Token::CONST) {
            break;
        }
        else if (tok == Token::CLASS) {
            break;
        }
        else if (tok != Token::COMMA) {
            throw SyntaxError(lex()->CurrentToken(), "expected a ',' or ';'");
        }
        Advance(); // eat ','
    }
    
    return builder()->factory()->NewDeclarationList(builder()->locator()->location(), scope_manager()->current(), decl_list);
}

Handle<Expression> RAXParser::ParseTryCatchStatement() {
    Handle<Expression> try_block = nullptr;
    Handle<Expression> catch_expr = nullptr;
    Handle<Expression> catch_block = nullptr;
    Handle<Expression> finally = nullptr;
    EXPECT(Token::TRY);
    try_block = ParseBlockStatement();
    if (Peek() == Token::CATCH) {
        Advance();
        EXPECT(Token::LPAREN);
        catch_expr = ParseExpression();
        EXPECT(Token::RPAREN);
        catch_block = ParseBlockStatement();
    }
    if (Peek() == Token::FINALLY) {
        Advance();
        finally = ParseBlockStatement();
    }
    return builder()->NewTryCatchStatement(try_block, catch_expr, catch_block,
                                           finally);
}

Handle<Expression> ParseProgram(RAXParser *parser)
{
    // parse program and return AST
    return parser->ParseProgram();    
}
    
}
}
}
