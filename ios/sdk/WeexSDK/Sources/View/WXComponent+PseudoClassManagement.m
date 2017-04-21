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

#import "WXComponent+PseudoClassManagement.h"
#import "WXComponent_internal.h"
#import "WXAssert.h"
#import "WXComponentManager.h"
#import "WXSDKInstance_private.h"
#import "WXUtility.h"

@implementation WXComponent (PseudoClassManagement)

-(NSMutableDictionary *)parseStyles:(NSDictionary *)styles
{
    NSMutableDictionary *newStyles = [NSMutableDictionary new];
    _pseudoClassStyles = [NSMutableDictionary new];
    if (styles && [styles count] > 0 ) {
        for (NSString *key in styles){
            if([key rangeOfString:@":"].location != NSNotFound){
                if ([key rangeOfString:@"active"].location != NSNotFound) { //all active listen
                    _isListenPseudoTouch = YES;
                }
                [_pseudoClassStyles setObject:styles[key] forKey:key];
            }else {
                [newStyles setObject:styles[key] forKey:key];
            }
        }
    }
    return newStyles;
}

- (void)updatePseudoClassStyles:(NSDictionary *)pseudoClassStyles
{
    WXAssertMainThread();
    NSMutableDictionary *styles = [NSMutableDictionary new];
    for (NSString *k in pseudoClassStyles) {
        [styles setObject:pseudoClassStyles[k] forKey:[self getPseudoKey:k]];
    }
    if ([styles count]>0) {
        __weak typeof(self) weakSelf = self;
        WXPerformBlockOnComponentThread(^{
            WXComponentManager *manager = weakSelf.weexInstance.componentManager;
            if (!manager.isValid) {
                return;
            }
            [manager updatePseudoClassStyles:styles forComponent:self.ref];
            [manager startComponentTasks];
        });
    }
    
    if (styles && [styles count] > 0) {
        if(!_updatedPseudoClassStyles) {
            _updatedPseudoClassStyles = [NSMutableDictionary new];
        }
        for (NSString *key in styles) {
            [_updatedPseudoClassStyles setObject:styles[key] forKey:key];
        }
    }
}

-(NSString *)getPseudoKey:(NSString *)key
{
    if ([key rangeOfString:@":"].location == NSNotFound) {
        return key;
    }
    NSRange range = [key rangeOfString:@":"];
    NSString *subKey = [key substringToIndex:range.location];
    return subKey;
}

-(NSMutableDictionary *)getPseudoClassStyles:(NSString *)key
{
    NSMutableDictionary *styles = [NSMutableDictionary new];
    [styles addEntriesFromDictionary:[self getPseudoClassStyles:key level:1]];
    [styles addEntriesFromDictionary:[self getPseudoClassStyles:key level:2]];
    return styles;
}

-(NSMutableDictionary *)getPseudoClassStyles:(NSString *)key level:(NSInteger )level
{
    NSMutableDictionary *styles = [NSMutableDictionary new];
    if (_pseudoClassStyles && [_pseudoClassStyles count] > 0 ) {
        for (NSString *k in _pseudoClassStyles){
            if ([k rangeOfString:key].location != NSNotFound && [WXUtility getSubStringNumber:k subString:@":"] == level) {
                [styles setObject:_pseudoClassStyles[k] forKey:[self getPseudoKey:k]];
            }
        }
    }
    return styles;
}

-(NSMutableDictionary *)getPseudoClassStylesByKeys:(NSArray *)keys
{
    NSMutableDictionary *styles = [NSMutableDictionary new];
    if(keys && [keys count]>0) {
        if (_pseudoClassStyles && [_pseudoClassStyles count] > 0 ) {
            for (NSString *k in _pseudoClassStyles){
                if([WXUtility getSubStringNumber:k subString:@":"] == [keys count]){
                    BOOL isContain = YES;
                    for(NSString *pKey in keys){
                        if ([k rangeOfString:pKey].location == NSNotFound) {
                            isContain = NO;
                            break;
                        }
                    }
                    if (isContain) {
                        [styles setObject:_pseudoClassStyles[k] forKey:[self getPseudoKey:k]];
                    }
                }
            }
        }
    }
    
    return styles;
}

- (void)recoveryPseudoStyles:(NSDictionary *)styles
{
    WXAssertMainThread();
    __weak typeof(self) weakSelf = self;
    NSMutableDictionary *resetStyles = [styles mutableCopy];
    if(_updatedPseudoClassStyles && [_updatedPseudoClassStyles count]>0){
        for (NSString *key in _updatedPseudoClassStyles) {
            if (![styles objectForKey:key] && [key length]>0) {
                [resetStyles setObject:@"" forKey:key];
            }
        }
    }
    WXPerformBlockOnComponentThread(^{
        WXComponentManager *manager = weakSelf.weexInstance.componentManager;
        if (!manager.isValid) {
            return;
        }
        [manager updatePseudoClassStyles:resetStyles forComponent:self.ref];
        [manager startComponentTasks];
    });
}

@end
