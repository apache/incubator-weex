//    
//  WXProfilerTypes.h
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


// Profile header.
@interface WXProfilerProfileHeader : WXObject

// Profile type name.
// Type: string
@property (nonatomic, strong) NSString *typeId;

// Profile title.
// Type: string
@property (nonatomic, strong) NSString *title;

// Unique identifier of the profile.
// Type: integer
@property (nonatomic, strong) NSNumber *uid;

// Last seen JS object Id.
// Type: integer
@property (nonatomic, strong) NSNumber *maxJSObjectId;

@end


// Profile.
@interface WXProfilerProfile : WXObject

// Type: object
@property (nonatomic, strong) NSDictionary *head;

// Type: object
@property (nonatomic, strong) NSDictionary *bottomUpHead;

@end


