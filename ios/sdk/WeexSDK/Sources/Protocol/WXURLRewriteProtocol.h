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

@class WXSDKInstance;

#define WX_REWRITE_URL(url, resourceType, instance)\
do {\
    id<WXURLRewriteProtocol> rewriteHandler = [WXSDKEngine handlerForProtocol:@protocol(WXURLRewriteProtocol)];\
    if ([rewriteHandler respondsToSelector:@selector(rewriteURL:withResourceType:withInstance:)]) {\
        newURL = [[rewriteHandler rewriteURL:url withResourceType:resourceType withInstance:instance].absoluteString copy];\
    }\
} while(0);


@protocol WXURLRewriteProtocol <NSObject>

/**
 * @abstract rewrite and complete URL
 *
 * @param url The original URL to be rewritten
 *
 * @param resourceType resource type which the url is sent for
 *
 * @param instance related instance
 *
 * @return a new url
 */
- (NSURL *)rewriteURL:(NSString *)url withResourceType:(WXResourceType)resourceType withInstance:(WXSDKInstance *)instance;

@end
