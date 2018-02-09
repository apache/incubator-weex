/*
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

#import "WXJSASTParser.h"
#import "WXLog.h"
#include <vector>
#include <sstream>


typedef enum : NSUInteger {
    WXJSTokenTypeBooleanLiteral = 1,
    WXJSTokenTypeEOF,
    WXJSTokenTypeIdentifier,
    WXJSTokenTypeKeyword,
    WXJSTokenTypeNullLiteral,
    WXJSTokenTypeNumericLiteral,
    WXJSTokenTypePunctuator,
    WXJSTokenTypeStringLiteral,
    WXJSTokenTypeRegularExpression,
    WXJSTokenTypeTemplate
} WXJSTokenType;

struct WXJSToken {
    WXJSTokenType type;
    std::string value;
    double doubleValue;
    bool octal;
    int start;
    int end;
};

struct WXJSMarker {
    int index;
};

static bool isWhiteSpace(int ch) {
    return (ch == 32) ||  // space
    (ch == 9) ||      // tab
    (ch == 0xB) ||
    (ch == 0xC) ||
    (ch == 0xA0);
}

static bool isIdentifierStart(int ch)
{
    return (ch == 36) || (ch == 95) ||  // $ or _
        (ch >= 65 && ch <= 90) ||         // A..Z
        (ch >= 97 && ch <= 122) ||        // a..z
        (ch == 92);                      // \ (backslash)
}

static bool isIdentifierPart(int ch) {
    return (ch == 36) || (ch == 95) ||  // $ or _
    (ch >= 65 && ch <= 90) ||         // A..Z
    (ch >= 97 && ch <= 122) ||        // a..z
    (ch >= 48 && ch <= 57) ||         // 0..9
    (ch == 92);                      // \ (backslash)
}

static bool isDecimalDigit(int ch) {
    return (ch >= 48 && ch <= 57);   // 0..9
}

static bool isHexDigit(int ch) {
    return std::string("0123456789abcdefABCDEF").find(ch) != std::string::npos;
}

static bool isOctalDigit(int ch) {
    return std::string("01234567").find(ch) != std::string::npos;
}

static int binaryPrecedence(WXJSToken *token)
{
    int prec = 0;
    
    if (token->type != WXJSTokenTypePunctuator) {
        return prec;
    }
    
    const std::string &value = token->value;
    
    if (value == "||") {
        prec = 1;
    }
    
    else if (value == "&&") {
        prec = 2;
    }
    
    else if (value == "|") {
        prec = 3;
    }
    
    else if (value == "^") {
        prec = 4;
    }
    
    else if (value == "&") {
        prec = 5;
    }
    
    else if (value == "==" || value == "!=" || value == "===" || value == "!==") {
        prec = 6;
    }
    
    else if (value == "<" || value == ">" || value == "<=" || value == ">=") {
        prec = 7;
    }
    
    else if (value == "<<" || value == ">>" || value == ">>>") {
        prec = 8;
    }
    
    else if (value == "+" || value == "-") {
        prec = 9;
    }
    
    else if (value == "*" || value == "/" || value == "%") {
        prec = 11;
    }
    
    return prec;
}

@implementation WXJSASTParser
{
    std::string _source;
    
    int _length;
    int _index;
    
    WXJSToken *_lookahead;
    
    std::vector<WXJSToken *> _tokens;
}

+ (instancetype)parserWithScript:(NSString *)script
{
    if (!script) {
        WXLogError(@"can not init parser with nil script");
        return nil;
    }
    return [[self alloc] initWithScript:script];
}

- (instancetype)initWithScript:(NSString *)script
{
    if (self = [super init]) {
        _source = std::string([script UTF8String]);;
        _length = (int)(_source.length());
        _index = 0;
        
        _lookahead = NULL;
        
        [self nextToken];
    }
    
    return self;
}

- (WXJSToken *)nextToken
{
    WXJSToken *token = _lookahead;
    
    WXJSToken *next = [self lex];
    
    _lookahead = next;
    
    if (next->type != WXJSTokenTypeEOF) {
        _tokens.push_back(token);
    }
    
    return token;
}

- (WXJSToken *)lex
{
    [self skipSpaces];
    
    if (_index >= _length) {
        WXJSToken *token = new WXJSToken();
        token->type = WXJSTokenTypeEOF;
        token->value = "";
        token->start = _index;
        token->end = _index;
        
        return token;
    }
    
    int ch = _source[_index];
    
    if (isIdentifierStart(ch)) {
        return [self scanIdentifier];
    }
    
    // ( or ) or ;
    if (ch == 40 || ch == 41 || ch == 59) {
        return [self scanPunctuator];
    }
    
    // string starting with ' or " .
    if (ch == 39 || ch == 34 ) {
        return [self scanStringLiteral];
    }
    
    // Dot (.) U+002E can also start a floating-point number, hence the need
    // to check the next
    if (ch == 0x2E) {
        if (isDecimalDigit(_source[_index + 1])) {
            return [self scanNumericLiteral];
        }
        return [self scanPunctuator];
    }
    
    if (isDecimalDigit(ch)) {
        return [self scanNumericLiteral];
    }
    
    // Possible `identifier
    if (ch >= 0xD800 && ch < 0xDFFF) {
        if (isIdentifierStart(ch)) {
            return [self scanIdentifier];
        }
    }
    
    return [self scanPunctuator];
}

- (WXJSToken *)scanIdentifier
{
    int start = _index;
    // Backslash (U+005C) starts an escaped
    std::string identifier = [self getIdentifier];
    WXJSTokenType type;
    if (identifier == "null") {
        type = WXJSTokenTypeNullLiteral;
    } else if (identifier == "true" || identifier == "false") {
        type = WXJSTokenTypeBooleanLiteral;
    } else {
        type = WXJSTokenTypeIdentifier;
    }
    
    WXJSToken *token = new WXJSToken();
    token->type = type;
    token->value = identifier;
    token->start = start;
    token->end = _index;
    
    return token;
}

- (std::string)getIdentifier
{
    int start = _index++;
    while (_index < _length) {
        int ch = _source[_index];
        if (isIdentifierPart(ch)) {
            ++_index;
        } else {
            break;
        }
    }
    
    return _source.substr(start, _index - start);
}

- (WXJSToken *)scanPunctuator
{
    int start = _index;
    
    // Check for most common single-character punctuators.
    int ch = _source[_index];
    std::string str = "";
    switch (ch) {
        // single-character punctuators
        case 46:   // .
        case 40:   // (
        case 41:   // )
        case 59:   // ;
        case 44:   // ,
        case 123:  // {
        case 125:  // }
        case 91:   // [
        case 93:   // ]
        case 58:   // :
        case 63:   // ?
        case 126:  // ~
            ++_index;
            str = std::string(1, ch);
            break;
        default:
            str =  _source.substr(_index, 3);
            if (str == "===" || str == "!==" || str == "**=") {
                _index += 3;
            } else {
                str = str.substr(0, 2);
                if (str == "&&" || str == "||" || str == "==" || str == "!=" ||
                    str == "+=" || str == "-=" || str == "*=" || str == "/=" ||
                    str == "++" || str == "--" || str == "&=" || str == "|=" ||
                    str == "^=" || str == "%=" || str == "<=" || str == ">=" ||
                    str == "=>" || str == "**") {
                    _index += 2;
                }  else {
                    str = _source[_index];
                    // parse 2-character punctuators first
                    if (std::string("<>=!+-*%&|^/").find(ch) != std::string::npos) {
                        ++_index;
                    }
                }
            }
    }
    
    if (_index == start) {
        [self _throwUnexpectedTokenError];
    }
    
    WXJSToken *token = new WXJSToken();
    token->type = WXJSTokenTypePunctuator;
    token->value = str;
    token->start = start;
    token->end = _index;
    
    return token;
}

- (WXJSToken *)scanStringLiteral
{
    int start = _index;
    int quote = _source[start];
    bool octal = false;
    
    ++_index;
    std::string str = "";
    
    while (_index < _length) {
        int ch = _source[_index++];
        
        if (ch == quote) {
            quote = -1;
            break;
        } else if (ch == 92) { // \ (backslash)
            ch = _source[_index++];
            switch (ch) {
                case 'u': {
                    if (_source[_index] == '{') { // {
                        ++_index;
                        str += [self scanUnicodeCodePointEscape];
                    } else {
                        int unescaped = [self scanHexEscape:ch];
                        if (unescaped == -1) {
                            [self _throwError:@"Invalid hexadecimal escape"];
                        } else {
                            str += unescaped;
                        }
                    }
                    break;
                }
                case 'x': {
                    int unescaped = [self scanHexEscape:ch];
                    if (unescaped == -1) {
                        [self _throwError:@"Invalid hexadecimal escape"];
                    } else {
                        str += unescaped;
                    }
                    
                    break;
                }
                case 'n': {
                    str += '\n';
                    break;
                }
                case 'r': {
                    str += '\r';
                    break;
                }
                case 't': {
                    str += '\t';
                    break;
                }
                case 'b': {
                    str += '\b';
                    break;
                }
                case 'f': {
                    str += '\f';
                    break;
                }
                case 'v': {
                    str += '\x0B';
                    break;
                }
                case '8':
                case '9': {
                    str += ch;
                    [self _throwUnexpectedTokenError];
                    break;
                }
                    
                default:
                    if (isOctalDigit(ch)) {
                        int code = ch - '0';
                        
                        // \0 is not octal escape sequence
                        if (code != 0) {
                            octal = true;
                        }
                        
                        if (_index < _length && isOctalDigit(_source[_index])) {
                            octal = true;
                            code = code * 8 + _source[_index++] - '0';
                            
                            // 3 digits are only allowed when string starts
                            // with 0, 1, 2, 3
                            if (std::string(1, '0123').find(ch) != std::string::npos &&
                                _index < _length &&
                                isOctalDigit(_source[_index])) {
                                code = code * 8 + _source[_index++] - '0';
                            }
                        }
                        str += std::string(1, code);
                    } else {
                        str += ch;
                    }
                    break;
            }
        } else {
            str += ch;
        }
    }
    
    if (quote != -1) {
        _index = start;
        [self _throwUnexpectedTokenError];
    }
    
    WXJSToken *token = new WXJSToken();
    token->type = WXJSTokenTypeStringLiteral;
    token->value = str;
    token->octal = octal;
    token->start = start;
    token->end = _index;
    
    return token;
}

- (int)scanUnicodeCodePointEscape {
    int ch = _source[_index];
    int code = 0;
    
    if (ch == '}') { // '}'
        [self _throwError:@"At least one hex digit is required in Unicode"];
    }
    
    while (_index < _length) {
        ch = _source[_index++];
        if (!isHexDigit(ch)) {
            break;
        }
        code = (int)(code * 16 + std::string("0123456789abcdef").find(tolower(ch)));
    }
    
    if (code > 0x10FFFF || ch != '}') {
        [self _throwUnexpectedTokenError];
    }
    
    return code;
}

- (WXJSToken *)scanNumericLiteral
{
    int start = _index;
    int ch = _source[start];
    std::string num;
    if (ch != '.') {
        num = _source[_index++];
        ch = _source[_index];
        
        if (num == "0") {
            if (ch == 'x' || ch == 'X') {
                ++_index;
                return [self scanHexLiteral:start];
            }
            if (ch == 'b' || ch == 'B') {
                ++_index;
                return [self scanBinaryLiteral:start];
            }
            if (ch == 'o' || ch == 'O') {
                return [self scanOctalLiteral:ch start:start];
            }
        }
        
        while (isDecimalDigit(_source[_index])) {
            num += _source[_index++];
        }
        ch = _source[_index];
    }
    
    if (ch == '.') {
        num += _source[_index++];
        while (isDecimalDigit(_source[_index])) {
            num += _source[_index++];
        }
        ch = _source[_index];
    }
    
    if (ch == 'e' || ch == 'E') {
        num += _source[_index++];
        
        ch = _source[_index];
        if (ch == '+' || ch == '-') {
            num += _source[_index++];
        }
        if (isDecimalDigit(_source[_index])) {
            while (isDecimalDigit(_source[_index])) {
                num += _source[_index++];
            }
        } else {
            [self _throwUnexpectedTokenError];
        }
    }
    
    if (isIdentifierStart(_source[_index])) {
        [self _throwUnexpectedTokenError];
    }
    
    WXJSToken *token = new WXJSToken();
    token->type = WXJSTokenTypeNumericLiteral;
    token->doubleValue = atof(num.c_str());
    token->start = start;
    token->end = _index;
    
    return token;
}

- (WXJSToken *)scanHexLiteral:(int)start
{
    std::string num = "";
    while (_index < _length) {
        if (!isHexDigit(_source[_index])) {
            break;
        }
        num += _source[_index++];
    }
    
    if (num.length() == 0) {
        [self _throwUnexpectedTokenError];
    }
    
    if (isIdentifierStart(_source[_index])) {
        [self _throwUnexpectedTokenError];
    }
    
    WXJSToken *token = new WXJSToken();
    token->type = WXJSTokenTypeNumericLiteral;
    token->doubleValue = static_cast<double>(std::stoi(num, nullptr, 16));
    token->start = start;
    token->end = _index;
    
    return token;
}

- (WXJSToken *)scanBinaryLiteral: (int)start
{
    std::string num = "";
    int ch;
    
    while (_index < _length) {
        ch = _source[_index];
        if (ch != '0' && ch != '1') {
            break;
        }
        num += _source[_index++];
    }
    
    if (num.length() == 0) {
        // only 0b or 0B
        [self _throwUnexpectedTokenError];
    }
    
    if (_index < _length) {
        ch = _source[_index];
        /* istanbul ignore else */
        if (isIdentifierStart(ch) || isDecimalDigit(ch)) {
            [self _throwUnexpectedTokenError];
        }
    }
    
    WXJSToken *token = new WXJSToken();
    token->type = WXJSTokenTypeNumericLiteral;
    token->doubleValue = static_cast<double>(std::stoi(num, nullptr, 2));
    token->start = start;
    token->end = _index;
    
    return token;
}
- (WXJSToken *)scanOctalLiteral:(int)prefix start:(int)start
{
    std::string num = "";
    bool octal = false;
    
    if (isOctalDigit(prefix)) {
        octal = true;
        num = '0' + _source[_index++];
    } else {
        ++_index;
    }
    
    while (_index < _length) {
        if (!isOctalDigit(_source[_index])) {
            break;
        }
        num += _source[_index++];
    }
    
    if (!octal && num.length() == 0) {
        // only 0o or 0O
        [self _throwUnexpectedTokenError];
    }
    
    if (isIdentifierStart(_source[_index]) || isDecimalDigit(_source[_index])) {
        [self _throwUnexpectedTokenError];
    }
    
    WXJSToken *token = new WXJSToken();
    token->type = WXJSTokenTypeNumericLiteral;
    token->doubleValue = static_cast<double>(std::stoi(num, nullptr, 8));
    token->octal = octal;
    token->start = start;
    token->end = _index;
    
    return token;
}

- (int)scanHexEscape:(int)prefix
{
    int i, len, ch, code = 0;
    
    len = (prefix == 'u') ? 4 : 2;
    for (i = 0; i < len; ++i) {
        if (_index < _length && isHexDigit(_source[_index])) {
            ch = _source[_index++];
            code = (int)(code * 16 + std::string("0123456789abcdef").find(tolower(ch)));
        } else {
            return -1;
        }
    }
    return code;
}

- (WXJSExpression *)parseExpression
{
    return [self parseConditionalExpression];
}

- (WXJSExpression *)parseConditionalExpression
{
    WXJSExpression *expr = [self parseBinaryExpression];
    
    if ([self match:"?"]) {
        [self nextToken];
        WXJSExpression *consequent = [self parseConditionalExpression];
        [self expect:":"];
        WXJSExpression *alternate = [self parseConditionalExpression];
        
        WXJSConditionalExpression *conditionalExpr = new WXJSConditionalExpression();
        conditionalExpr->test = expr;
        conditionalExpr->consequent = consequent;
        conditionalExpr->alternate = alternate;
        
        return conditionalExpr;
    }
    
    return expr;
}

- (WXJSExpression *)parseBinaryExpression
{
    WXJSExpression *expr = [self parseUnaryExpression];
    
    std::vector<int> precedenceStack;
    std::vector<WXJSToken *> operatorStack;
    std::vector<WXJSExpression *> expressionStack;
    
    WXJSToken *token = _lookahead;
    int prec = binaryPrecedence(token);
    if (prec == 0) {
        return expr;
    }
    
    [self nextToken];
    
    expressionStack.push_back(expr);
    precedenceStack.push_back(prec);
    operatorStack.push_back(token);
    expressionStack.push_back([self parseUnaryExpression]);
    
    WXJSExpression *right;
    std::string operator_;
    WXJSExpression *left;
    while ((prec = binaryPrecedence(_lookahead)) > 0) {
        while ((expressionStack.size() > 1) && (prec <= precedenceStack[precedenceStack.size() - 1])) {
            right = expressionStack[expressionStack.size() - 1];
            expressionStack.pop_back();
            operator_ = operatorStack[operatorStack.size() - 1]->value;
            operatorStack.pop_back(); precedenceStack.pop_back();
            left = expressionStack[expressionStack.size() - 1]; expressionStack.pop_back();
            expressionStack.push_back([self createBinaryExpression:operator_ left:left right:right]);
        }
        
        // Shift.
        token = [self nextToken];
        precedenceStack.push_back(prec);
        operatorStack.push_back(token);
        expressionStack.push_back([self parseUnaryExpression]);
    }
    
    // Final reduce to clean-up the stack.
    int i = (int)(expressionStack.size() - 1);
    expr = expressionStack[i];
    while (i > 0) {
        expr = [self createBinaryExpression:operatorStack[i - 1]->value left:expressionStack[i - 1] right:expr];
        i--;
    }
    
    return expr;
}

- (WXJSExpression *)parseUnaryExpression
{
    WXJSToken *token;
    
    if ([self match:"++"] || [self match:"--"] || [self match:"+"] || [self match:"-"] || [self match:"~"] || [self match:"!"]) {
        token = [self nextToken];
        WXJSExpression *argument = [self parseUnaryExpression];
        WXJSUnaryExpression *expr = new WXJSUnaryExpression();
        expr->operator_ = token->value;
        expr->prefix = ([self match:"++"] || [self match:"--"]) ? true : false;
        expr->argument = argument;
        return expr;
    }
    
    return [self parseMemberExpression];
}

- (WXJSExpression *)parsePrimaryExpression
{
    int type = _lookahead->type;
    
    if (type == WXJSTokenTypePunctuator) {
        if (_lookahead->value == "[") {
            return [self parseArrayExpression];
        } else if (_lookahead->value == "(") {
            return [self parseGroupExpression];
        }
    }
    if (type == WXJSTokenTypeIdentifier) {
        WXJSIdentifier *identifier = new WXJSIdentifier();
        identifier->name = [self nextToken]->value;
        return identifier;
    }
    
    if (type == WXJSTokenTypeStringLiteral || type == WXJSTokenTypeNumericLiteral || type == WXJSTokenTypeBooleanLiteral || type == WXJSTokenTypeNullLiteral) {
        return [self createLiteral:[self nextToken]];
    } else {
        [self _throwUnexpectedTokenError];
        return new WXJSIdentifier();
    }
}

- (WXJSArrayExpression *)parseArrayExpression
{
    std::vector<WXJSExpression *> expressions;
    
    [self expect:"["];
    
    while (![self match:"]"]) {
        if ([self match:","]) {
            [self nextToken];
            expressions.push_back(NULL);
        } else {
            expressions.push_back([self parseConditionalExpression]);
            
            if (![self match:"]"]) {
                [self expect:","];
            }
        }
    }
    
    [self expect:"]"];
    
    WXJSArrayExpression *array = new WXJSArrayExpression();
    array->expressions = expressions;
    
    return array;
}

- (WXJSExpression *)parseGroupExpression
{
    WXJSExpression *expr;
    [self expect:"("];
    
    expr = [self parseConditionalExpression];
    
    [self expect:")"];
    
    return expr;
}


- (WXJSExpression *)parseMemberExpression
{
    WXJSExpression *expr = [self parsePrimaryExpression];
    if ([self match:"."]) {
        [self expect:"."];
        WXJSExpression * property = [self parseMemberExpression];
        WXJSMemberExpression *memberExpr = new WXJSMemberExpression();
        memberExpr->object = expr;
        memberExpr->property = property;
        memberExpr->computed = false;
        return memberExpr;
    } else if ([self match:"["]) {
        [self expect:"["];
        WXJSExpression *property = [self parseConditionalExpression];
        [self expect:"]"];
        WXJSMemberExpression *memberExpr = new WXJSMemberExpression();
        memberExpr->object = expr;
        memberExpr->property = property;
        memberExpr->computed = true;
        return memberExpr;
    }
    
    return expr;
}

- (WXJSExpression *)createBinaryExpression:(const std::string &)operator_ left:(WXJSExpression *)left right:(WXJSExpression *)right
{
    WXJSBinaryExpression *node = new WXJSBinaryExpression();
    node->operator_ = operator_;
    node->left = left;
    node->right = right;
    return node;
}

- (WXJSLiteral *)createLiteral:(WXJSToken *)token
{
    if (token->type == WXJSTokenTypeNumericLiteral) {
        WXJSNumericLiteral *node = new WXJSNumericLiteral();
        node->value = token->doubleValue;
        return node;
    } else if (token->type == WXJSTokenTypeNullLiteral) {
        WXJSNullLiteral *node = new WXJSNullLiteral();
        return node;
    } else if (token->type == WXJSTokenTypeStringLiteral) {
        WXJSStringLiteral *node = new WXJSStringLiteral();
        node->value = token->value;
        return node;
    } else if (token->type == WXJSTokenTypeBooleanLiteral) {
        WXJSBooleanLiteral *node = new WXJSBooleanLiteral();
        node->value = token->value == "true";
        return node;
    } else {
        assert(false);
    }
}

- (void)skipSpaces
{
    int ch;
    while (_index < _length) {
        ch = _source[_index];
        if (isWhiteSpace(ch)) {
            ++_index;
        } else {
            break;
        }
    }
}

- (bool)match:(std::string)value
{
    return _lookahead->type == WXJSTokenTypePunctuator && _lookahead->value == value;
}

- (void)expect:(std::string)value
{
    WXJSToken *token = [self nextToken];
    if (token->type != WXJSTokenTypePunctuator || token->value != value) {
        [self _throwUnexpectedTokenError];
    }
}

- (void)_throwUnexpectedTokenError
{
    [self _throwError:@"Unexpected Token"];
}

- (void)_throwError:(NSString *)errorMessage
{
    WXLogError(@"%@, index:%d, script:%s", errorMessage, _index, _source.c_str());
}

@end
