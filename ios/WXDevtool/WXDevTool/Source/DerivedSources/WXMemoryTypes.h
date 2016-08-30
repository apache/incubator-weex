//    
//  WXMemoryTypes.h
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


// Number of nodes with given name.
@interface WXMemoryNodeCount : WXObject

// Type: string
@property (nonatomic, strong) NSString *nodeName;

// Type: integer
@property (nonatomic, strong) NSNumber *count;

@end


// Number of JS event listeners by event type.
@interface WXMemoryListenerCount : WXObject

// Type: string
@property (nonatomic, strong) NSString *type;

// Type: integer
@property (nonatomic, strong) NSNumber *count;

@end


// Character data statistics for the page.
@interface WXMemoryStringStatistics : WXObject

// Type: integer
@property (nonatomic, strong) NSNumber *dom;

// Type: integer
@property (nonatomic, strong) NSNumber *js;

// Type: integer
@property (nonatomic, strong) NSNumber *shared;

@end


@interface WXMemoryDOMGroup : WXObject

// Type: integer
@property (nonatomic, strong) NSNumber *size;

// Type: string
@property (nonatomic, strong) NSString *title;

// Type: string
@property (nonatomic, strong) NSString *documentURI;

// Type: array
@property (nonatomic, strong) NSArray *nodeCount;

// Type: array
@property (nonatomic, strong) NSArray *listenerCount;

@end


@interface WXMemoryMemoryBlock : WXObject

// Size of the block in bytes if available
// Type: number
@property (nonatomic, strong) NSNumber *size;

// Unique name used to identify the component that allocated this block
// Type: string
@property (nonatomic, strong) NSString *name;

// Type: array
@property (nonatomic, strong) NSArray *children;

@end


