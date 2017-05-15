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
#import <WeexSDk/WeexSDK.h>

@class WXModuleMethod;
typedef NS_ENUM(NSUInteger, PrerenderStatus) {
    PrerenderSucceed = 0,
    PrerenderFail = 1,
};
typedef NS_ENUM(NSUInteger, PrerenderTaskType) {
    PrerenderUITaskType = 0,
    PrerenderMethodType = 1,
};

typedef void (^PrerenderCallback) (PrerenderStatus status, NSString* urlStr, NSString * msg);

@interface WXPrerenderManager : NSObject

- (instancetype) init NS_UNAVAILABLE;
+ (instancetype) sharedInstance;
+ (void) addTask:(NSString *) urlStr WXInstance:(NSString *) instanceId callback:(WXModuleCallback)callback;
+ (BOOL) isTaskExist:(NSString *)url;
+ (NSError *)errorFromUrl:(NSString *)url;
+ (void) renderFromCache:(NSString *)url;
+ (UIView *)viewFromUrl:(NSString *)url;
+ (id)instanceFromUrl:(NSString *)url;
+ (void)removePrerenderTaskforUrl:(NSString *)url;
+ (void)storePrerenderModuleTasks:(WXModuleMethod *)prerenderModuleTask forUrl:(NSString *)url;
+ (void)excuteModuleTasksForUrl:(NSString *)url;
+ (void)destroyTask:(NSString *)parentInstanceId;
@end
