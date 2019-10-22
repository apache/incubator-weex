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
#import <WeexSDK/WXModuleProtocol.h>

NS_ASSUME_NONNULL_BEGIN

@protocol WXWebSocketDelegate<NSObject>
- (void)didOpen;
- (void)didFailWithError:(NSError *)error;
- (void)didReceiveMessage:(id)message;
- (void)didCloseWithCode:(NSInteger)code reason:(nullable NSString *)reason wasClean:(BOOL)wasClean;
@end

@protocol WXWebSocketHandler<NSObject>

- (void)open:(NSString *)url protocol:(NSString *)protocol identifier:(nullable NSString *)identifier withDelegate:(id<WXWebSocketDelegate>)delegate;
- (void)send:(NSString *)identifier data:(NSString *)data;
- (void)close:(NSString *)identifier;
- (void)close:(NSString *)identifier code:(NSInteger)code reason:(nullable NSString *)reason;
- (void)clear:(NSString *)identifier;
@end

NS_ASSUME_NONNULL_END
