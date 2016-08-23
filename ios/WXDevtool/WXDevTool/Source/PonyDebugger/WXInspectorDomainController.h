//
//  WXInspectorDomainController.h
//  PonyDebugger
//
//  Created by Ryan Olson on 2012-10-27.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXPonyDebugger.h"
#import "WXInspectorDomain.h"

@interface WXInspectorDomainController : WXDomainController

@property (nonatomic, strong) WXInspectorDomain *domain;

+ (instancetype)defaultInstance;

@end
