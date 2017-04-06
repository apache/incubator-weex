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

#import <UIKit/UIKit.h>
#import "WXDefine.h"
#import "WXSDKInstance.h"

#define MSG_SUCCESS     @"WX_SUCCESS"
#define MSG_NO_HANDLER  @"WX_NO_HANDLER"
#define MSG_NO_PERMIT   @"WX_NO_PERMISSION"
#define MSG_FAILED      @"WX_FAILED"
#define MSG_PARAM_ERR   @"WX_PARAM_ERR"
#define MSG_EXP         @"WX_EXCEPTION"

@protocol WXModuleProtocol <NSObject>

/**
 * @abstract the module callback , result can be string or dictionary.
 * @discussion callback data to js, the id of callback function will be removed to save memory.
 */
typedef void (^WXModuleCallback)(id result);

/**
 * @abstract the module callback , result can be string or dictionary.
 * @discussion callback data to js, you can specify the keepAlive parameter to keep callback function id keepalive or not. If the keepAlive is true, it won't be removed until instance destroyed, so you can call it repetitious.
 */
typedef void (^WXModuleKeepAliveCallback)(id result, BOOL keepAlive);

#define WX_EXPORT_MODULE(module) 

@optional

/**
 *  @abstract returns the execute queue for the module
 *
 *  @return dispatch queue that module's methods will be invoked on
 *
 *  @discussion the implementation is optional. Implement it if you want to execute module actions in the special queue.
 *  Default dispatch queue will be the main queue.
 *
 */
- (dispatch_queue_t)targetExecuteQueue;

/**
 *  @abstract returns the execute thread for the module
 *
 *  @return  thread that module's methods will be invoked on
 *
 *  @discussion the implementation is optional. If you want to execute module actions in the special thread, you can create a new one. 
 *  If `targetExecuteQueue` is implemented,  the queue returned will be respected first.
 *  Default is the main thread.
 *
 */
- (NSThread *)targetExecuteThread;

/**
 *  @abstract the instance bind to this module. It helps you to get many useful properties related to the instance.
 */
@property (nonatomic, weak) WXSDKInstance *weexInstance;

@end
