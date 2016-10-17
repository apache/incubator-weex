//    
//  WXWebGLTypes.h
//  PonyDebuggerDerivedSources
//
//  Generated on 8/23/12
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//
    
#import "WXObject.h"
#import "WXDebugger.h"
#import "WXDynamicDebuggerDomain.h"


@interface WXWebGLCall : WXObject

// Type: string
@property (nonatomic, strong) NSString *functionName;

@end


@interface WXWebGLTraceLog : WXObject

@property (nonatomic, strong) NSString *identifier;

// Type: array
@property (nonatomic, strong) NSArray *calls;

@end


