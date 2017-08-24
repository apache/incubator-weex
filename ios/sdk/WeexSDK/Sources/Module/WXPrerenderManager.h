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
#import "WXModuleProtocol.h"

@class WXModuleMethod;

@interface WXPrerenderManager : NSObject

- (instancetype) init NS_UNAVAILABLE;

/**
 *  @abstract add prerender task
 *
 *  @param url The prerender url string
 *
 *  @param instanceId  The instanceId that its instance add prerender task
 *
 *  @param callback  the module method callback
 *
 **/
+ (void) addTask:(NSString *) url instanceId:(NSString *)instanceId callback:(WXModuleCallback)callback;

/**
 *  @abstract Returns true if url is exist in task .
 *
 **/
+ (BOOL) isTaskExist:(NSString *)url;

/**
 *  @abstract Returns key from url .
 *
 **/
+ (NSString *)getTaskKeyFromUrl:(NSString *)url;

/**
 *  @abstract Returns error if url is prerender fail .
 *
 **/
+ (NSError *)errorFromUrl:(NSString *)url;

/**
 *  @abstract excute render from prerender cache .
 * @discussion this method should be excuted after oncreat and onfail block created
 *
 **/
+ (void) renderFromCache:(NSString *)url;

/**
 *  @abstract Returns prerender root view .
 *
 **/
+ (UIView *)viewFromUrl:(NSString *)url;

/**
 *  @abstract Returns prerender instance .
 *
 **/
+ (id)instanceFromUrl:(NSString *)url;

/**
 *  @abstract Remove prerender task by url .
 *
 **/
+ (void)removePrerenderTaskforUrl:(NSString *)url;

/**
 *  @abstract store prerender module task by url .
 *
 **/
+ (void)storePrerenderModuleTasks:(WXModuleMethod *)prerenderModuleTask forUrl:(NSString *)url;

/**
 *  @abstract destroy task , it will destroy instance also.
 *
 **/
+ (void)destroyTask:(NSString *)parentInstanceId;
@end
