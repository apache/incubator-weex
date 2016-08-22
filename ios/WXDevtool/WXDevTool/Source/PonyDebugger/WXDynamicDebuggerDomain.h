//
//  WXDebuggerDomain.h
//  PonyDebugger
//
//  Created by Mike Lewis on 11/5/11.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import <Foundation/Foundation.h>


// Must call this to send the response
typedef void (^WXResponseCallback)(NSDictionary *result, id error);

@protocol WXCommandDelegate;
@class WXDebugger;


@interface WXDynamicDebuggerDomain : NSObject

@property (nonatomic, assign, readonly) WXDebugger * debuggingServer;
@property (nonatomic, assign) id <WXCommandDelegate> delegate;

+ (NSString *)domainName;

- (void)handleMethodWithName:(NSString *)methodName parameters:(NSDictionary *)params responseCallback:(WXResponseCallback)responseCallback;
- (id)initWithDebuggingServer:(WXDebugger *)debuggingServer;

@end


@protocol WXCommandDelegate <NSObject>

- (void)domain:(WXDynamicDebuggerDomain *)domain enableWithCallback:(void (^)(id error))callback;
- (void)domain:(WXDynamicDebuggerDomain *)domain disableWithCallback:(void (^)(id error))callback;

@end
