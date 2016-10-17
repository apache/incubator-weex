//    
//  WXDatabaseTypes.h
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


// Database object.
@interface WXDatabaseDatabase : WXObject

// Database ID.
@property (nonatomic, strong) NSString *identifier;

// Database domain.
// Type: string
@property (nonatomic, strong) NSString *domain;

// Database name.
// Type: string
@property (nonatomic, strong) NSString *name;

// Database version.
// Type: string
@property (nonatomic, strong) NSString *version;

@end


