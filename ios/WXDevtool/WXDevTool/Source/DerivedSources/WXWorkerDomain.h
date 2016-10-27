//
//  WXWorkerDomain.h
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


@protocol WXWorkerCommandDelegate;

@interface WXWorkerDomain : WXDynamicDebuggerDomain 

@property (nonatomic, assign) id <WXWorkerCommandDelegate, WXCommandDelegate> delegate;

// Events
- (void)workerCreatedWithWorkerId:(NSNumber *)workerId url:(NSString *)url inspectorConnected:(NSNumber *)inspectorConnected;
- (void)workerTerminatedWithWorkerId:(NSNumber *)workerId;
- (void)dispatchMessageFromWorkerWithWorkerId:(NSNumber *)workerId message:(NSDictionary *)message;
- (void)disconnectedFromWorker;

@end

@protocol WXWorkerCommandDelegate <WXCommandDelegate>
@optional
- (void)domain:(WXWorkerDomain *)domain enableWithCallback:(void (^)(id error))callback;
- (void)domain:(WXWorkerDomain *)domain disableWithCallback:(void (^)(id error))callback;
- (void)domain:(WXWorkerDomain *)domain sendMessageToWorkerWithWorkerId:(NSNumber *)workerId message:(NSDictionary *)message callback:(void (^)(id error))callback;
- (void)domain:(WXWorkerDomain *)domain connectToWorkerWithWorkerId:(NSNumber *)workerId callback:(void (^)(id error))callback;
- (void)domain:(WXWorkerDomain *)domain disconnectFromWorkerWithWorkerId:(NSNumber *)workerId callback:(void (^)(id error))callback;
- (void)domain:(WXWorkerDomain *)domain setAutoconnectToWorkersWithValue:(NSNumber *)value callback:(void (^)(id error))callback;

@end

@interface WXDebugger (WXWorkerDomain)

@property (nonatomic, readonly, strong) WXWorkerDomain *workerDomain;

@end
