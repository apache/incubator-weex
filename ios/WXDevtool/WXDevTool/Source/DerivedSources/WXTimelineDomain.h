//
//  WXTimelineDomain.h
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

@class WXTimelineTimelineEvent;

@protocol WXTimelineCommandDelegate;

// Timeline provides its clients with instrumentation records that are generated during the page runtime. Timeline instrumentation can be started and stopped using corresponding commands. While timeline is started, it is generating timeline event records.
@interface WXTimelineDomain : WXDynamicDebuggerDomain 

@property (nonatomic, assign) id <WXTimelineCommandDelegate, WXCommandDelegate> delegate;

// Events

// Fired for every instrumentation event while timeline is started.
// Param record: Timeline event record data.
- (void)eventRecordedWithRecord:(WXTimelineTimelineEvent *)record;

@end

@protocol WXTimelineCommandDelegate <WXCommandDelegate>
@optional

// Starts capturing instrumentation events.
// Param maxCallStackDepth: Samples JavaScript stack traces up to <code>maxCallStackDepth</code>, defaults to 5.
- (void)domain:(WXTimelineDomain *)domain startWithMaxCallStackDepth:(NSNumber *)maxCallStackDepth callback:(void (^)(id error))callback;

// Stops capturing instrumentation events.
- (void)domain:(WXTimelineDomain *)domain stopWithCallback:(void (^)(id error))callback;

// Starts calculating various DOM statistics and sending them as part of timeline events.
// Param enabled: True to start collecting DOM counters.
- (void)domain:(WXTimelineDomain *)domain setIncludeMemoryDetailsWithEnabled:(NSNumber *)enabled callback:(void (^)(id error))callback;

// Tells whether timeline agent supports frame instrumentation.
// Callback Param result: True if timeline supports frame instrumentation.
- (void)domain:(WXTimelineDomain *)domain supportsFrameInstrumentationWithCallback:(void (^)(NSNumber *result, id error))callback;

@end

@interface WXDebugger (WXTimelineDomain)

@property (nonatomic, readonly, strong) WXTimelineDomain *timelineDomain;

@end
