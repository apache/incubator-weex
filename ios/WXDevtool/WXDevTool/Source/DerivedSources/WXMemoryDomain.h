//
//  WXMemoryDomain.h
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

@class WXMemoryMemoryBlock;
@class WXMemoryStringStatistics;

@protocol WXMemoryCommandDelegate;

@interface WXMemoryDomain : WXDynamicDebuggerDomain 

@property (nonatomic, assign) id <WXMemoryCommandDelegate, WXCommandDelegate> delegate;

@end

@protocol WXMemoryCommandDelegate <WXCommandDelegate>
@optional
- (void)domain:(WXMemoryDomain *)domain getDOMNodeCountWithCallback:(void (^)(NSArray *domGroups, WXMemoryStringStatistics *strings, id error))callback;
// Callback Param distribution: An object describing all memory allocated by the process
- (void)domain:(WXMemoryDomain *)domain getProcessMemoryDistributionWithCallback:(void (^)(WXMemoryMemoryBlock *distribution, id error))callback;

@end

@interface WXDebugger (WXMemoryDomain)

@property (nonatomic, readonly, strong) WXMemoryDomain *memoryDomain;

@end
