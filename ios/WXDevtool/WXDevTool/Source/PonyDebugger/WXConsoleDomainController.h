//
//  WXConsoleDomainController.h
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 2013-01-30.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXPonyDebugger.h"
#import "WXConsoleDomain.h"

@interface WXConsoleDomainController : WXDomainController

@property (nonatomic, strong) WXConsoleDomain *domain;

+ (WXConsoleDomainController *)defaultInstance;

- (void)logWithArguments:(NSArray *)args severity:(NSString *)severity;
- (void)clear;

@end
