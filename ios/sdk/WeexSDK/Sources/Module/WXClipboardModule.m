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

#import "WXClipboardModule.h"

@implementation WXClipboardModule

WX_EXPORT_METHOD(@selector(setString:))
WX_EXPORT_METHOD(@selector(getString:))

- (dispatch_queue_t)targetExecuteQueue {
    return dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
}

- (void)setString:(NSString *)content
{
    UIPasteboard *clipboard = [UIPasteboard generalPasteboard];
    clipboard.string = (content ? : @"");
}

- (void)getString:(WXModuleKeepAliveCallback)callback{
    UIPasteboard *clipboard = [UIPasteboard generalPasteboard];
    NSDictionary *result = [@{} mutableCopy];
    if(clipboard.string)
    {
        [result setValue:clipboard.string forKey:@"data"];
        [result setValue:@"success" forKey:@"result"];
    }else
    {
        [result setValue:@"" forKey:@"data"];
        [result setValue:@"fail" forKey:@"result"];
    }
    if (callback) {
        callback(result, NO);
    }

}

@end
