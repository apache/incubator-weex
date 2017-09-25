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

#import "WXComponent+DataBinding.h"
#import "WXComponent_internal.h"
#import "WXSDKInstance_private.h"
#import "WXComponentManager.h"
#import "WXAssert.h"
#import "WXJSASTParser.h"

#pragma clang diagnostic ignored "-Wobjc-protocol-method-implementation"

typedef enum : NSUInteger {
    WXDataBindingTypeProp = 0,
    WXDataBindingTypeStyle,
    WXDataBindingTypeAttributes,
    WXDataBindingTypeEvents,
    WXDataBindingTypeCount,
} WXDataBindingType;

static JSContext *jsContext;

@implementation WXComponent (DataBinding)

- (void)updateBindingData:(NSDictionary *)data
{
    WXAssertComponentThread();
    
    if (_isSkipUpdate) {
        _isSkipUpdate = NO;
        return;
    }
    
    WXLogDebug(@"Update binding data:%@, for component:%@", data, self.ref);
    if (!data) {
        WXLogWarning(@"No data for updateBindingData");
        return;
    }
    
    WXComponent *templateComponent = _templateComponent;
    if (!templateComponent) {
        WXLogError(@"No template fount for component:%@", templateComponent);
        return;
    }
    
    if (templateComponent->_bindingProps) {
        NSMutableDictionary *newData = [NSMutableDictionary dictionary];
        [templateComponent->_bindingProps enumerateKeysAndObjectsUsingBlock:^(NSString * _Nonnull key, WXDataBindingBlock  _Nonnull block, BOOL * _Nonnull stop) {
            BOOL needUpdate;
            id value = block(data, &needUpdate);
            if (value) {
                newData[key] = value;
            }
        }];
        
        data = newData;
    }
    
    if (!_isRepeating) {
        WXDataBindingBlock repeatBlock = templateComponent->_bindingRepeat;
        if (repeatBlock) {
            BOOL needUpdate = NO;
            NSArray *repeatData = repeatBlock(data, &needUpdate);
            [self _repeat:repeatData inData:data];
            return;
        }
    }
    
    WXDataBindingBlock matchBlock = templateComponent->_bindingMatch;
    if (matchBlock) {
        BOOL needUpdate = NO;
        BOOL needDisplay = [matchBlock(data, &needUpdate) boolValue];
        if (!needDisplay) {
            self.displayType = WXDisplayTypeNone;
            return;
        } else if (needDisplay && !_isNeedJoinLayoutSystem) {
            self.displayType = WXDisplayTypeBlock;
        }
    }
    
    for (int i = WXDataBindingTypeStyle; i < WXDataBindingTypeCount; i++) {
        NSDictionary *bindingMap = i == WXDataBindingTypeStyle ? templateComponent->_bindingStyles : (i == WXDataBindingTypeAttributes ? templateComponent->_bindingAttributes : templateComponent->_bindingEvents);
        if (!bindingMap || bindingMap.count == 0) {
            continue;
        }
        NSMutableDictionary *newAttributesOrStyles = [NSMutableDictionary dictionary];
        
        [bindingMap enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull attributeOrStyleName, WXDataBindingBlock  _Nonnull bindingBlock, BOOL * _Nonnull stop) {
            BOOL needUpdate = NO;
            id newValue = bindingBlock(data, &needUpdate);
            if (needUpdate) {
                newAttributesOrStyles[attributeOrStyleName] = newValue;
            }
        }];
        
        if (newAttributesOrStyles.count > 0) {
            [self.weexInstance.componentManager startComponentTasks];
            if (i == WXDataBindingTypeStyle) {
                [self.weexInstance.componentManager updateStyles:newAttributesOrStyles forComponent:self.ref];
            } else if (i == WXDataBindingTypeAttributes) {
                [self.weexInstance.componentManager updateAttributes:newAttributesOrStyles forComponent:self.ref];
            } else if (i == WXDataBindingTypeEvents) {
                [self _addEventParams:newAttributesOrStyles];
            }
        }
    }
    
    NSArray *subcomponents = self.subcomponents;
    for (WXComponent *subcomponent in subcomponents) {
        [subcomponent updateBindingData:data];
    }
}

- (void)_repeat:(NSArray *)repeatData inData:(NSDictionary *)data
{
    NSMutableDictionary *dataCopy = [data mutableCopy];
    WXComponent *templateComponent = _templateComponent;
    NSArray *subcomponents = self.supercomponent.subcomponents;
    NSUInteger startIndex = [subcomponents indexOfObject:self];
    [repeatData enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        if (templateComponent->_repeatIndexIdentify) {
            dataCopy[templateComponent->_repeatIndexIdentify] = @(idx);
        }
        if (templateComponent->_repeatLabelIdentify) {
            dataCopy[templateComponent->_repeatLabelIdentify] = obj;
        }
        
        WXComponent *exsitingComponent;
        if (startIndex + idx < subcomponents.count) {
            if (subcomponents[startIndex + idx]
                && ((WXComponent *)(subcomponents[startIndex + idx]))->_templateComponent == templateComponent) {
                exsitingComponent = subcomponents[startIndex + idx];
                exsitingComponent.displayType = WXDisplayTypeBlock;
            }
        }
        
        WXComponent *component = exsitingComponent ? : [templateComponent copy];
        component->_isRepeating = YES;
        [component updateBindingData:dataCopy];
        component->_isRepeating = NO;
        
        if (idx > 0 && exsitingComponent) {
            component->_isSkipUpdate = YES;
        }
        
        if (!exsitingComponent) {
            [self.weexInstance.componentManager startComponentTasks];
            [self.supercomponent _insertSubcomponent:component atIndex:startIndex + idx];
            [self.weexInstance.componentManager _addUITask:^{
                [self.supercomponent insertSubview:component atIndex:startIndex + idx];
            }];
        }
    }];
    
    // set displaty:none to the redundant components;
    NSUInteger i = startIndex + repeatData.count;
    while (i < self.supercomponent.subcomponents.count) {
        WXComponent *component = self.supercomponent.subcomponents[i];
        if (component->_templateComponent == templateComponent) {
            component->_isSkipUpdate = YES;
            component.displayType = WXDisplayTypeNone;
        }
        i++;
    }
}

- (void)_storeBindingsWithProps:(NSDictionary *)props styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSDictionary *)events
{
    WXAssertComponentThread();
    
    if (props.count > 0) {
        if (!_bindingProps) {
            _bindingProps = [NSMutableDictionary dictionary];
        }
        [self _storeBindings:props type:WXDataBindingTypeProp];
    }
    
    if (styles.count > 0) {
        if (!_bindingStyles) {
            _bindingStyles = [NSMutableDictionary dictionary];
        }
        [self _storeBindings:styles type:WXDataBindingTypeStyle];
    }
    
    if (attributes.count > 0) {
        if (!_bindingAttributes) {
            _bindingAttributes = [NSMutableDictionary dictionary];
        }
        [self _storeBindings:attributes type:WXDataBindingTypeAttributes];
    }
    
    if (events.count > 0) {
        if (!_bindingEvents) {
            _bindingEvents = [NSMutableDictionary dictionary];
        }
        [self _storeBindings:events type:WXDataBindingTypeEvents];
    }
}

- (void)_storeBindings:(NSDictionary *)stylesOrAttributesOrEvents type:(WXDataBindingType)type
{
    WXAssertComponentThread();
    
    NSMutableDictionary *bindingMap;
    switch (type) {
        case WXDataBindingTypeProp:
            bindingMap = _bindingProps;
            break;
        case WXDataBindingTypeStyle:
            bindingMap = _bindingStyles;
            break;
        case WXDataBindingTypeAttributes:
            bindingMap = _bindingAttributes;
            break;
        case WXDataBindingTypeEvents:
            bindingMap = _bindingEvents;
            break;
        default:
            WXAssert(NO, @"error binding type:%z", type);
            break;
    }
    
    [stylesOrAttributesOrEvents enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull name, id  _Nonnull binding, BOOL * _Nonnull stop) {
        if ([binding isKindOfClass:[NSDictionary class]] && binding[WXBindingIdentify]) {
            // {"attributeOrStyleName":{"@binding":"bindingExpression"}
            NSString *bindingExpression = binding[WXBindingIdentify];
            WXJSASTParser *parser = [WXJSASTParser parserWithScript:bindingExpression];
            WXJSExpression *expression = [parser parseExpression];
            WXDataBindingBlock block = [self bindingBlockWithExpression:expression];
            bindingMap[name] = block;
        } else if ([binding isKindOfClass:[NSArray class]]) {
            // {"attributeOrStyleName":[..., "string", {"@binding":"bindingExpression"}, "string", {"@binding":"bindingExpression"}, ...]
            NSMutableDictionary *bindingBlocksForIndex = [NSMutableDictionary dictionary];
            __block BOOL isBinding = NO;
            [binding enumerateObjectsUsingBlock:^(id  _Nonnull bindingInArray, NSUInteger idx, BOOL * _Nonnull stop) {
                if ([bindingInArray isKindOfClass:[NSDictionary class]] && bindingInArray[WXBindingIdentify]) {
                    isBinding = YES;
                    NSString *bindingExpression = bindingInArray[WXBindingIdentify];
                    WXJSASTParser *parser = [WXJSASTParser parserWithScript:bindingExpression];
                    WXJSExpression *expression = [parser parseExpression];
                    WXDataBindingBlock block = [self bindingBlockWithExpression:expression];
                    bindingBlocksForIndex[@(idx)] = block;
                }
            }];
            
            bindingMap[name] = ^id(NSDictionary *data, BOOL *needUpdate) {
                NSMutableArray *newArray = [binding mutableCopy];
                [binding enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
                    BOOL _needUpdate = NO;
                    WXDataBindingBlock block = bindingBlocksForIndex[@(idx)];
                    if (block) {
                        id newValue = block(data, &_needUpdate);
                        if (newValue) {
                             newArray[idx] = newValue;
                        }  
                        if (_needUpdate) {
                            *needUpdate = YES;
                        }
                    }
                }];
                
                return type == WXDataBindingTypeEvents ? newArray : [newArray componentsJoinedByString:@""];
            };
        }
        
        if (type == WXDataBindingTypeAttributes) {
            if ([WXBindingMatchIdentify isEqualToString:name]) {
                WXJSASTParser *parser = [WXJSASTParser parserWithScript:binding];
                WXJSExpression *expression = [parser parseExpression];
                _bindingMatch = [self bindingBlockWithExpression:expression];
            } else if ([WXBindingRepeatIdentify isEqualToString:name]) {
                WXJSASTParser *parser = [WXJSASTParser parserWithScript:binding[WXBindingRepeatExprIdentify]];
                WXJSExpression *expression = [parser parseExpression];
                _bindingRepeat = [self bindingBlockWithExpression:expression];
                _repeatIndexIdentify = binding[WXBindingRepeatIndexIdentify];
                _repeatLabelIdentify = binding[WXBindingRepeatLabelIdentify];
            }
        }
    }];
}

- (WXDataBindingBlock)bindingBlockWithExpression:(WXJSExpression *)expression
{
    if (!expression) {
        return nil;
    }
    WXDataBindingBlock block = ^id(NSDictionary *data, BOOL *needUpdate) {
        if (expression->is<WXJSStringLiteral>()) {
            *needUpdate = NO;
            return [NSString stringWithCString:(((WXJSStringLiteral *)expression)->value).c_str() encoding:[NSString defaultCStringEncoding]];
        } else if (expression->is<WXJSNumericLiteral>()) {
            *needUpdate = NO;
            return @(((WXJSNumericLiteral *)expression)->value);
        } else if (expression->is<WXJSBooleanLiteral>()) {
            *needUpdate = NO;
            return @(((WXJSBooleanLiteral *)expression)->value);
        } else if (expression->is<WXJSNullLiteral>()) {
            *needUpdate = NO;
            return nil;
        } else if (expression->is<WXJSIdentifier>()) {
            NSString *identiferName = [NSString stringWithCString:(((WXJSIdentifier *)expression)->name).c_str() encoding:[NSString defaultCStringEncoding]];
            if (data[identiferName]) {
                *needUpdate = YES;
                return data[identiferName];
            } else {
                WXLogError(@"identifer:%@ not found", identiferName);
                return nil;
            }
        } else if (expression->is<WXJSMemberExpression>()) {
            WXJSMemberExpression *member = (WXJSMemberExpression *)expression;
            BOOL objectNeedUpdate = NO, propertyNeedUpdate = NO;
            id object = [self bindingBlockWithExpression:member->object](data, &objectNeedUpdate);
            if (member->computed) {
                id propertyName = [self bindingBlockWithExpression:member->property](data, &propertyNeedUpdate);
                *needUpdate = objectNeedUpdate || propertyNeedUpdate;
                if ([object isKindOfClass:[NSDictionary class]] && [propertyName isKindOfClass:[NSString class]]) {
                    return object[propertyName];
                } else if ([object isKindOfClass:[NSArray class]] && [propertyName isKindOfClass:[NSNumber class]]) {
                    return [object objectAtIndex:[propertyName unsignedIntegerValue]];
                }
            } else {
                NSString *propertyName = [NSString stringWithCString:(((WXJSStringLiteral *)member->property)->value).c_str() encoding:[NSString defaultCStringEncoding]];
                *needUpdate = objectNeedUpdate;
                return object[propertyName];
            }
            
            return nil;
        } else if (expression->is<WXJSArrayExpression>()) {
            WXJSArrayExpression *expr = (WXJSArrayExpression *)expression;
            std::vector<WXJSExpression *> expressions = expr->expressions;
            NSMutableArray *array = [NSMutableArray array];
            for(WXJSExpression *expr : expressions) {
                if (expr == NULL) {
                    continue;
                }
                WXDataBindingBlock block = [self bindingBlockWithExpression:expr];
                *needUpdate = NO;
                if (block) {
                    BOOL elementNeedUpdate;
                    id object = block(data, &elementNeedUpdate);
                    if (object) {
                        *needUpdate = *needUpdate || elementNeedUpdate;
                        [array addObject:object];
                    }
                }
            }
            
            return array;
        } else if (expression->is<WXJSUnaryExpression>()) {
            WXJSUnaryExpression *expr = (WXJSUnaryExpression *)expression;
            std::string operator_ = expr->operator_;
            id argument = [self bindingBlockWithExpression:expr->argument](data, needUpdate);
            if (operator_ == "+") {
                return @([argument doubleValue]);
            } else if (operator_ == "-") {
                return @(-[argument doubleValue]);
            } else if (operator_ == "!") {
                return @(![argument boolValue]);
            } else {
                WXLogError(@"Not supported unary operator:%s", operator_.c_str());
                return nil;
            }
        } else if (expression->is<WXJSBinaryExpression>()) {
            WXJSBinaryExpression *expr = (WXJSBinaryExpression *)expression;
            std::string operator_ = expr->operator_;
            BOOL leftNeedUpdate = NO, rightNeedUpdate = NO;
            id left = [self bindingBlockWithExpression:expr->left](data, &leftNeedUpdate);
            id right = [self bindingBlockWithExpression:expr->right](data, &rightNeedUpdate);
            *needUpdate = leftNeedUpdate || rightNeedUpdate;
            if (operator_ == "+") {
                return @([left doubleValue] + [right doubleValue]);
            } else if (operator_ == "-") {
                return @([left doubleValue] - [right doubleValue]);
            } else if (operator_ == "*") {
                return @([left doubleValue] * [right doubleValue]);
            } else if (operator_ == "/") {
                return @([left doubleValue] / [right doubleValue]);
            } else if (operator_ == "%") {
                return @([left integerValue] % [right integerValue]);
            } else if (operator_ == ">") {
                return @([left doubleValue] > [right doubleValue]);
            } else if (operator_ == ">=") {
                return @([left doubleValue] >= [right doubleValue]);
            } else if (operator_ == "<") {
                return @([left doubleValue] < [right doubleValue]);
            } else if (operator_ == "<=") {
                return @([left doubleValue] <= [right doubleValue]);
            } else if (operator_ == "===" || operator_ == "==") {
                if ([left isKindOfClass:[NSString class]]) {
                    return @([left isEqualToString:right]);
                } else if ([left isKindOfClass:[NSNumber class]]) {
                    return  @([left doubleValue] == [right doubleValue]);
                } else {
                    WXLogError(@"Wrong type %@ at left of '===' or '=='", NSStringFromClass([left class]));
                    return nil;
                }
            } else if (operator_ == "!==" || operator_ == "!=") {
                if ([left isKindOfClass:[NSString class]]) {
                    return @(![left isEqualToString:right]);
                } else if ([left isKindOfClass:[NSNumber class]]) {
                    return  @([left doubleValue] != [right doubleValue]);
                } else {
                    WXLogError(@"Wrong type %@ at left of '!==' or '!='", NSStringFromClass([left class]));
                    return nil;
                }
            } else if (operator_ == "||") {
                return @([left boolValue] || [right boolValue]);
            } else if (operator_ == "&&") {
                return @([left boolValue] && [right boolValue]);
            } else {
                WXLogError(@"Not supported binary operator:%s", operator_.c_str());
                return nil;
            }
        } else if (expression->is<WXJSConditionalExpression>()) {
            WXJSConditionalExpression *conditional = (WXJSConditionalExpression *)expression;
            BOOL testNeedUpdate = NO, conditionalNeedUpdate = NO, alternateNeedUpdate = NO;
            id testResult = [self bindingBlockWithExpression:conditional->test](data, &testNeedUpdate);
            id result;
            if ([testResult boolValue]) {
                result = [self bindingBlockWithExpression:conditional->consequent](data, &conditionalNeedUpdate);
            } else {
                result = [self bindingBlockWithExpression:conditional->alternate](data, &alternateNeedUpdate);
            }
            *needUpdate = testNeedUpdate || conditionalNeedUpdate || alternateNeedUpdate;
            return result;
        } else {
            WXAssert(NO, @"expression type error");
            return nil;
        }
    };
    
    return block;
}

@end
