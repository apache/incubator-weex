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

#import <Foundation/Foundation.h>
#include <string>
#include <vector>

typedef enum : NSUInteger {
    WXJSExpressionTypeUnary,
    WXJSExpressionTypeBinary,
    WXJSExpressionTypeBinaryConditional,
} WXJSExpressionType;

struct WXJSExpression {
    virtual ~WXJSExpression() {}
    template <class T> bool is() {
        return dynamic_cast<T *>(this) != NULL;
    }
};

struct WXJSLiteral :WXJSExpression {
};

struct WXJSNullLiteral : WXJSLiteral {
};

struct WXJSStringLiteral : WXJSLiteral {
    std::string value;
};

struct WXJSNumericLiteral : WXJSLiteral {
    double value;
};

struct WXJSBooleanLiteral : WXJSLiteral {
    bool value;
};

struct WXJSIdentifier : WXJSExpression {
    std::string name;
};

struct WXJSMemberExpression : WXJSExpression {
    WXJSExpression *object;
    WXJSExpression *property;
    bool computed;
};

struct WXJSArrayExpression : WXJSExpression {
    std::vector<WXJSExpression *> expressions;
};

struct WXJSUnaryExpression : WXJSExpression {
    std::string operator_;
    bool prefix;
    WXJSExpression *argument;
};

struct WXJSBinaryExpression : WXJSExpression {
    std::string operator_;
    WXJSExpression *left;
    WXJSExpression *right;
};

struct WXJSLogicalExpression : WXJSExpression {
    std::string operator_;
    WXJSExpression *left;
    WXJSExpression *right;
};

struct WXJSConditionalExpression : WXJSExpression {
    WXJSExpression *test;
    WXJSExpression *alternate;
    WXJSExpression *consequent;
};


@interface WXJSASTParser : NSObject

+ (instancetype)parserWithScript:(NSString *)script;

- (WXJSExpression *)parseExpression;

@end
