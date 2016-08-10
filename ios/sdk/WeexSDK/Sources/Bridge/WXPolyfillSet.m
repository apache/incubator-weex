//
//  WXPolyfillSet.m
//  WeexSDK
//
//  Created by yinfeng on 16/8/10.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import "WXPolyfillSet.h"

@implementation WXPolyfillSet
{
    NSMutableSet *_set;
}

+ (instancetype)create
{
    WXPolyfillSet *jsSet = [WXPolyfillSet new];
    jsSet->_set = [NSMutableSet set];
    return jsSet;
}

- (BOOL)has:(id)value
{
    return [_set containsObject:value];
}

- (NSUInteger)size
{
    return _set.count;
}

- (void)add:(id)value
{
    [_set addObject:value];
}

- (BOOL)delete:(id)value
{
    if ([_set containsObject:value]) {
        [_set removeObject:value];
        return YES;
    }
    
    return NO;
}

- (void)clear
{
    [_set removeAllObjects];
}

@end

