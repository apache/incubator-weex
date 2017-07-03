/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#import <Foundation/Foundation.h>
#import "WXSDKInstance.h"

#define WXTracingDataHanding          @"WXTracingDataHanding"
#define WXTracingJSCall          @"jsCall"

#define WXTracingBegin             @"B"
#define WXTracingEnd               @"E"
#define WXTracingDuration          @"D"

typedef enum : NSUInteger {
    // global
    WXTDataHanding = 0,
    WXTRender
} WXTracingTag;

@interface WXTracing:NSObject

@property (nonatomic, copy) NSString *ref; // compnonet id
@property (nonatomic, copy) NSString *className; // compnonet class name or module class name
@property (nonatomic, copy) NSString *name; // register name
@property (nonatomic, copy) NSString *ph; // phase
@property (nonatomic) NSTimeInterval ts; // time
@property (nonatomic) long long traceId;
@property (nonatomic) NSTimeInterval duration;
@property (nonatomic, copy) NSString *fName; // functionName
@property (nonatomic, copy) NSString *iid; // instance id
@property (nonatomic, copy) NSString *parentId;// parent event id

@end

@interface WXTracingTask:NSObject

@property (nonatomic, copy) NSString *iid;
@property (nonatomic) long long counter;
@property (nonatomic)  WXTracingTag tag;
@property (nonatomic, strong) NSMutableArray *tracings;

@end

@interface WXTracingManager : NSObject
+(void)switchTracing:(BOOL)isTracing;
+(BOOL)isTracing;
+(void)startTracing:(WXTracing*)tracing;
+(void)startTracing:(NSString *)iid ref:(NSString*)ref className:(NSString *)className name:(NSString *)name ph:(NSString *)ph fName:(NSString *)fName parentId:(NSString *)parentId;
+ (void)tracingGloabalTask:(NSString *)fName instanceId:(NSString *)instanceId ph:(NSString *)ph;
+(void)getTracingData:(NSString *)instanceId;

@end
