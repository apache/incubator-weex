//
//  WXRuntimeDomainController.h
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 8/7/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXPonyDebugger.h"
#import "WXRuntimeDomain.h"


@interface WXRuntimeDomainController : WXDomainController

+ (WXRuntimeDomainController *)defaultInstance;

@property (nonatomic, strong) WXRuntimeDomain *domain;

- (NSString *)registerAndGetKeyForObject:(id)object;
- (void)clearObjectReferencesByKey:(NSArray *)objectKeys;
- (void)clearAllObjectReferences;

@end
