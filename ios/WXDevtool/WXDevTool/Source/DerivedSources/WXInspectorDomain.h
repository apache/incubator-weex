//
//  WXInspectorDomain.h
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

@class WXRuntimeRemoteObject;

@protocol WXInspectorCommandDelegate;

@interface WXInspectorDomain : WXDynamicDebuggerDomain 

@property (nonatomic, assign) id <WXInspectorCommandDelegate, WXCommandDelegate> delegate;

// Events
- (void)evaluateForTestInFrontendWithTestCallId:(NSNumber *)testCallId script:(NSString *)script;
- (void)inspectWithObject:(WXRuntimeRemoteObject *)object hints:(NSDictionary *)hints;

@end

@protocol WXInspectorCommandDelegate <WXCommandDelegate>
@optional

// Enables inspector domain notifications.
- (void)domain:(WXInspectorDomain *)domain enableWithCallback:(void (^)(id error))callback;

// Disables inspector domain notifications.
- (void)domain:(WXInspectorDomain *)domain disableWithCallback:(void (^)(id error))callback;

@end

@interface WXDebugger (WXInspectorDomain)

@property (nonatomic, readonly, strong) WXInspectorDomain *inspectorDomain;

@end
