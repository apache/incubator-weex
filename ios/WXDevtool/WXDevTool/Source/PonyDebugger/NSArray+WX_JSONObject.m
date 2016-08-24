//
//  NSArray+WX_JSONObject.m
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 8/6/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "NSArray+WX_JSONObject.h"

@implementation NSArray (WX_JSONObject)

- (NSArray *)WX_JSONObject;
{
    NSMutableArray *ret = [[NSMutableArray alloc] initWithCapacity:self.count];
    
    for (id obj in self) {
        [ret addObject:[obj WX_JSONObject]];
    }
    
    return ret;
}

@end
