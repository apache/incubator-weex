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
#import "WXDefine.h"

WX_EXTERN_C_BEGIN

void WXAssertInternal(NSString *func, NSString *file, int lineNum, NSString *format, ...);

#if DEBUG
#define WXAssert(condition, ...) \
do{\
    if(!(condition)){\
        WXAssertInternal(@(__func__), @(__FILE__), __LINE__, __VA_ARGS__);\
    }\
}while(0)
#else
#define WXAssert(condition, ...)
#endif

/**
 *  @abstract macro for asserting that a parameter is required.
 */
#define WXAssertParam(name) WXAssert(name, \
@"the parameter '%s' is required", #name)

/**
 *  @abstract macro for asserting if the handler conforms to the protocol
 */
#define WXAssertProtocol(handler, protocol) WXAssert([handler conformsToProtocol:protocol], \
@"handler:%@ does not conform to protocol:%@", handler, protocol)

/**
 *  @abstract macro for asserting that the object is kind of special class.
 */
#define WXAssertClass(name,className) WXAssert([name isKindOfClass:[className class]], \
@"the variable '%s' is not a kind of '%s' class", #name,#className)

/**
 *  @abstract macro for asserting that we are running on the main thread.
 */
#define WXAssertMainThread() WXAssert([NSThread isMainThread], \
@"must be called on the main thread")

/**
 *  @abstract macro for asserting that we are running on the component thread.
 */
#define WXAssertComponentThread() \
WXAssert([[NSThread currentThread].name isEqualToString:WX_COMPONENT_THREAD_NAME], \
@"must be called on the component thread")

/**
 *  @abstract macro for asserting that we are running on the bridge thread.
 */
#define WXAssertBridgeThread() \
WXAssert([[NSThread currentThread].name isEqualToString:WX_BRIDGE_THREAD_NAME] || [[NSThread currentThread].name isEqualToString:WX_BACKUP_BRIDGE_THREAD_NAME], \
@"must be called on the bridge thread")

#define WXAssertNotReached() \
WXAssert(NO, @"should never be reached")

WX_EXTERN_C_END
