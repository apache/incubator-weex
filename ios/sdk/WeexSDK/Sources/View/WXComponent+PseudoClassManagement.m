/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXComponent+PseudoClassManagement.h"
#import "WXComponent_internal.h"
#import "WXAssert.h"

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

- (void)updateTouchPseudoClassStyles:(NSDictionary *)pseudoClassStyles
{
    WXAssertMainThread();
    NSMutableDictionary *styles = [NSMutableDictionary new];
    for (NSString *k in pseudoClassStyles) {
        if([k rangeOfString:@"active"].location != NSNotFound){
            [styles setObject:pseudoClassStyles[k] forKey:[self getPseudoKey:k]];
        }
    }
    if ([styles count]>0) {
        [self _updateViewStyles:styles];
    }
}

-(NSString *)getPseudoKey:(NSString *)key
{
    NSRange range = [key rangeOfString:@":"];
    NSString *subKey = [key substringToIndex:range.location];
    return subKey;
}

- (void)recoveryPseudoStyles:(NSDictionary *)styles
{
    WXAssertMainThread();
    [self _updateViewStyles:styles];
}

@end
