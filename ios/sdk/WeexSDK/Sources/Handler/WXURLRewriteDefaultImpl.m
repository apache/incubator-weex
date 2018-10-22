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

#import "WXURLRewriteDefaultImpl.h"
#import "WXLog.h"
#import "WXSDKInstance.h"

static NSString *const WXURLLocalScheme = @"local";

@implementation WXURLRewriteDefaultImpl

- (NSURL *)rewriteURL:(NSString *)url
     withResourceType:(WXResourceType)resourceType
         withInstance:(WXSDKInstance *)instance
{
    NSURL *completeURL = [NSURL URLWithString:url];
    if ([completeURL isFileURL]) {
        return completeURL;
    } else if ([self isLocalURL:completeURL]) {
        NSString *resourceName = [[completeURL host]?:@"" stringByAppendingString:[completeURL path]];
        NSURL *resourceURL = [[NSBundle mainBundle] URLForResource:resourceName withExtension:@""];
        if (!resourceURL) {
            WXLogError(@"Invalid local resource URL:%@, no resouce found.", url);
        }
        
        return resourceURL;
    } else {
        return [instance completeURL:url];
    }
}

- (BOOL)isLocalURL:(NSURL *)url
{
    return[[[url scheme] lowercaseString] isEqualToString:WXURLLocalScheme];
}

@end
