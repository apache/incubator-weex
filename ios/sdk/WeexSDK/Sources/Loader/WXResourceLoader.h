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
#import <WeexSDK/WXResourceRequest.h>
#import <WeexSDK/WXResourceResponse.h>

NS_ASSUME_NONNULL_BEGIN

@interface WXResourceLoader : NSObject

@property (nonatomic, strong) WXResourceRequest *request;

@property (nonatomic, copy) void (^onDataSent)(unsigned long long /* bytesSent */, unsigned long long /* totalBytesToBeSent */);
@property (nonatomic, copy) void (^onResponseReceived)(const WXResourceResponse *);
@property (nonatomic, copy) void (^onDataReceived)(NSData *);
@property (nonatomic, copy) void (^onFinished)(const WXResourceResponse *, NSData *);
@property (nonatomic, copy) void (^onFailed)(NSError *);

- (instancetype)initWithRequest:(WXResourceRequest *)request;

- (void)start;

- (void)cancel:(NSError **)error;

@end

NS_ASSUME_NONNULL_END
