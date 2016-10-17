//
//  WXWebGLDomain.h
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

@class WXWebGLTraceLog;

@protocol WXWebGLCommandDelegate;

@interface WXWebGLDomain : WXDynamicDebuggerDomain 

@property (nonatomic, assign) id <WXWebGLCommandDelegate, WXCommandDelegate> delegate;

@end

@protocol WXWebGLCommandDelegate <WXCommandDelegate>
@optional

// Enables WebGL inspection.
- (void)domain:(WXWebGLDomain *)domain enableWithCallback:(void (^)(id error))callback;

// Disables WebGL inspection.
- (void)domain:(WXWebGLDomain *)domain disableWithCallback:(void (^)(id error))callback;
- (void)domain:(WXWebGLDomain *)domain dropTraceLogWithTraceLogId:(NSString *)traceLogId callback:(void (^)(id error))callback;
- (void)domain:(WXWebGLDomain *)domain captureFrameWithCallback:(void (^)(NSString *traceLogId, id error))callback;
- (void)domain:(WXWebGLDomain *)domain getTraceLogWithTraceLogId:(NSString *)traceLogId callback:(void (^)(WXWebGLTraceLog *traceLog, id error))callback;
- (void)domain:(WXWebGLDomain *)domain replayTraceLogWithTraceLogId:(NSString *)traceLogId stepNo:(NSNumber *)stepNo callback:(void (^)(NSString *screenshotDataUrl, id error))callback;

@end

@interface WXDebugger (WXWebGLDomain)

@property (nonatomic, readonly, strong) WXWebGLDomain *webGLDomain;

@end
