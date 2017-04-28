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

__attribute__ ((deprecated("Use WXResourceRequestHandler instead")))
@protocol WXNetworkProtocol <NSObject>

/**
 * @abstract send request
 *
 * @param request The URL Request
 *
 * @param sendDataCallback  This block is called periodically to notify the progress.
 *
 * @param responseCallback  This block is called when receiving a response and no further messages will be received until the completion block is called. 
 *
 * @param receiveDataCallback This block is called when data is available.
 *
 * @param completionCallback This block is called when the last message related to a specific task is sent.
 */
- (id)sendRequest:(NSURLRequest *)request withSendingData:(void (^)(int64_t bytesSent, int64_t totalBytes))sendDataCallback
                                             withResponse:(void (^)(NSURLResponse *response))responseCallback
                                          withReceiveData:(void (^)(NSData *data))receiveDataCallback
                                          withCompeletion:(void (^)(NSData *totalData, NSError *error))completionCallback;

@end
