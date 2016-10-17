//
//  WXPageDomainController.h
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 8/6/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXPonyDebugger.h"
#import "WXPageDomain.h"


@interface WXPageDomainController : WXDomainController

+ (WXPageDomainController *)defaultInstance;

@property (nonatomic, strong) WXPageDomain *domain;

@end
