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
#import "WXResourceRequest.h"
#import "WXResourceResponse.h"

@protocol WXResourceRequestDelegate <NSObject>

// Periodically informs the delegate of the progress of sending content to the server.
- (void)request:(WXResourceRequest *)request didSendData:(unsigned long long)bytesSent totalBytesToBeSent:(unsigned long long)totalBytesToBeSent;

// Tells the delegate that the request received the initial reply (headers) from the server.
- (void)request:(WXResourceRequest *)request didReceiveResponse:(WXResourceResponse *)response;

// Tells the delegate that the request has received some of the expected data.
- (void)request:(WXResourceRequest *)request didReceiveData:(NSData *)data;

// Tells the delegate that the request finished transferring data.
- (void)requestDidFinishLoading:(WXResourceRequest *)request;

// Tells the delegate that the request failed to load successfully.
- (void)request:(WXResourceRequest *)request didFailWithError:(NSError *)error;

@end

@protocol WXResourceRequestHandler <NSObject>

// Send a resource request with a delegate
- (void)sendRequest:(WXResourceRequest *)request withDelegate:(id<WXResourceRequestDelegate>)delegate;

@optional

// Cancel the ongoing request
- (void)cancelRequest:(WXResourceRequest *)request;

@end


