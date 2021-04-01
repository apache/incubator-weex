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


#ifdef __cplusplus
#include <JavaScriptCore/JavaScriptCore.h>
#include <string>
#endif

NS_ASSUME_NONNULL_BEGIN

@protocol WXUnicornRenderProtocol <NSObject>

@property (nonatomic, assign) CGRect frame;

@property (nonatomic, strong) UIView *rootView;

@property(nonatomic, copy) NSString * instanceId;

- (instancetype)initWithInstanceId:(NSString *)instanceId;

- (void)startEngine:(UIViewController*)parentViewController;

- (void)shutdown;

- (uint64_t)getFirstScreenTimeStamp;
- (uint64_t)getFirstScreenTimeInterval;

- (NSString*)getEngineTimeline;

#ifdef __cplusplus
typedef void(*UnicornRenderFunc)(const std::string& instance_id,
                                 const std::string& module,
                                 const std::string& method,
                                 JSContextRef,
                                 _Nullable JSValueRef* _Nullable,
                                 int argc);
+ (UnicornRenderFunc)getRenderFunc;
#endif

+ (void)installUnicornExternalAdapterImageProvider:(id)provider;

@end

NS_ASSUME_NONNULL_END
