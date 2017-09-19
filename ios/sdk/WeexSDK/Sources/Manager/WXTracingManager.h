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

#define WXTNetworkHanding          @"loadJS"
#define WXTExecJS                  @"execJS"
#define WXTJSCall                  @"jsCall"
#define WXTDomCall                 @"domCall"
#define WXTRender                  @"render"
#define WXTRenderFinish            @"renderFinish"

#define WXTJSBridgeThread          @"JSThread"
#define WXTDOMThread               @"DOMThread"
#define WXTUIThread                @"UIThread"
#define WXTMainThread              @"MainThread"

#define WXTracingBegin             @"B"
#define WXTracingEnd               @"E"
#define WXTracingDuration          @"D"
#define WXTracingInstant           @"i"

#define WXTracingDurationDefault   0

typedef enum : NSUInteger {
    // global
    WXTracingNetworkHanding = 0,
    WXTracingDataHanding,
    WXTracingRender
} WXTracingTag;


@interface WXTracingApi:NSObject

@property (nonatomic, copy) NSString *name; //
@property (nonatomic, copy) NSString *className; //
@property (nonatomic, copy) NSString *method; //

@end

@interface WXTracing:NSObject

@property (nonatomic, copy) NSString *ref; // compnonet id
@property (nonatomic, copy) NSString *parentRef; // compnonet id
@property (nonatomic, copy) NSString *className; // compnonet class name or module class name
@property (nonatomic, copy) NSString *name; // register name
@property (nonatomic, copy) NSString *ph; // phase
@property (nonatomic) NSTimeInterval ts; // time
@property (nonatomic) long long traceId;
@property (nonatomic) NSTimeInterval duration;
@property (nonatomic, copy) NSString *fName; // functionName
@property (nonatomic, copy) NSString *iid; // instance id
@property (nonatomic) long long parentId;// parent event id
@property (nonatomic, copy) NSString *bundleUrl;
@property (nonatomic, copy) NSString *threadName;
@property (nonatomic, strong) NSMutableArray *childrenRefs; // children ids
-(NSDictionary *)dictionary;
@end

@interface WXTracingTask:NSObject

@property (nonatomic, copy) NSString *iid;
@property (nonatomic) long long counter;
@property (nonatomic, copy)  NSString *tag;
@property (nonatomic, copy)  NSString *bundleUrl;
@property (nonatomic, strong) NSMutableArray *tracings;
@property (nonatomic, copy) NSString *bundleJSType; //

@end

@interface WXTracingManager : NSObject

/**
 *  @discusstion  weex perfermance  tracing state
 *  @return isTracing , NO in the release environment.
 */
+(BOOL)isTracing;
/**
 *  @discusstion  set weex perfermance  tracing state , you should use in debug environment
 *  @param isTracing YES weex will be traced, NO weex will not be traced .
 */
+(void)switchTracing:(BOOL)isTracing;
/**
 *  @discusstion  weex perfermance  tracing state
 *  @param iid the instance id.
 *  @param ref the component ref
 *  @param className the module or component class name
 *  @param name  the module or component name
 *  @param phase the trace phase
 *  @param functionName function name
 *  @param options the optional refer:support ts,duration,parentRef
 */
+(void)startTracingWithInstanceId:(NSString *)iid ref:(NSString*)ref className:(NSString *)className name:(NSString *)name phase:(NSString *)phase functionName:(NSString *)functionName options:(NSDictionary *)options;
/**
 *  @discusstion  fetch tracing data
 *  @return  the tracing data
 */
+(NSMutableDictionary*)getTracingData;

/**
 *  @discusstion  clear tracing data
 */
+(void)clearTracingData;

/**
 *  @discusstion  get current time
 */
+(NSTimeInterval)getCurrentTime;

/**
 *  @discusstion set bundle type
 *  @param jsBundleString the bundle source.
 *  @param iid the instance  id.
 */
+(void)setBundleJSType:(NSString *)jsBundleString instanceId:(NSString *)iid;
/**
 *  @discusstion  fetch the weex moudle component handler info
 *  @return  the weex moudle component handler info
 */
+(NSDictionary *)getTacingApi;

/**
 *  @discusstion commit tracing info
 *  @param instanceId the instance  id.
 */
+(void )commitTracing:(NSString *)instanceId;

/**
 *  @discusstion commit summary info
 *  @param instanceId the instance  id.
 */
+ (void)commitTracingSummaryInfo:(NSDictionary *)info withInstanceId:(NSString *)instanceId;

/**
 *  @discusstion destroy Traincg Task by Instance
 *  @param instanceId the instanceId.
 */
+(void)destroyTraincgTaskWithInstance:(NSString *)instanceId;
@end
