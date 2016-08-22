//
//  WXInspectorDomainController.m
//  PonyDebugger
//
//  Created by Ryan Olson on 2012-10-27.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXInspectorDomainController.h"
#import "WXInspectorDomain.h"

@implementation WXInspectorDomainController

@dynamic domain;

#pragma mark - Class Methods

+ (instancetype)defaultInstance;
{
    static WXInspectorDomainController *defaultInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaultInstance = [[WXInspectorDomainController alloc] init];
    });
    return defaultInstance;
}

+ (Class)domainClass;
{
    return [WXInspectorDomain class];
}

#pragma mark - WXInspectorCommandDelegate
- (void)domain:(WXInspectorDomain *)domain enableWithCallback:(void (^)(id error))callback {
    callback(nil);
}

@end
