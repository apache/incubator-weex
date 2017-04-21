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

#import "WXWebViewModule.h"
#import "WXWebComponent.h"
#import "WXSDKManager.h"
#import "WXSDKInstance_private.h"

@implementation WXWebViewModule

@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(notifyWebview:data:))
WX_EXPORT_METHOD(@selector(reload:))
WX_EXPORT_METHOD(@selector(goBack:))
WX_EXPORT_METHOD(@selector(goForward:))

- (void)performBlockWithWebView:(NSString *)elemRef block:(void (^)(WXWebComponent *))block {
    if (!elemRef) {
        return;
    }

    __weak typeof(self) weakSelf = self;
    
    WXPerformBlockOnComponentThread(^{
        WXWebComponent *webview = (WXWebComponent *)[weakSelf.weexInstance componentForRef:elemRef];
        if (!webview) {
            return;
        }
        
        [weakSelf performSelectorOnMainThread:@selector(doBlock:) withObject:^() {
            block(webview);
        } waitUntilDone:NO];
    });
}

- (void)doBlock:(void (^)())block {
    block();
}

- (void)notifyWebview:(NSString *)elemRef data:(NSDictionary *)data {
    [self performBlockWithWebView:elemRef block:^void (WXWebComponent *webview) {
        [webview notifyWebview:data];
    }];
}

- (void)reload:(NSString *)elemRef {
    [self performBlockWithWebView:elemRef block:^void (WXWebComponent *webview) {
        [webview reload];
    }];
}

- (void)goBack:(NSString *)elemRef {
    [self performBlockWithWebView:elemRef block:^void (WXWebComponent *webview) {
        [webview goBack];
    }];
}

- (void)goForward:(NSString *)elemRef {
    [self performBlockWithWebView:elemRef block:^void (WXWebComponent *webview) {
        [webview goForward];
    }];
}

@end
