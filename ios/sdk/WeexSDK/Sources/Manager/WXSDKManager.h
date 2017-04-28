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
#import "WXBridgeManager.h"

@class WXModuleManager;

@interface WXSDKManager : NSObject

/**
 * @abstract Returns bridge manager
 **/
+ (WXBridgeManager *)bridgeMgr;

/**
 * @abstract Returns weex instance for specific identifier
 **/
+ (WXSDKInstance *)instanceForID:(NSString *)identifier;

/**
 * @abstract Returns weex instance for specific identifier
 **/
+ (void)storeInstance:(WXSDKInstance *)instance forID:(NSString *)identifier;

/**
 * @abstract Returns weex instance for specific identifier
 **/
+ (void)removeInstanceforID:(NSString *)identifier;

/**
 * @abstract unload
 **/
+ (void)unload;

/**
 * @abstract Returns module manager
 **/
+ (WXModuleManager *)moduleMgr DEPRECATED_MSG_ATTRIBUTE();

@end
