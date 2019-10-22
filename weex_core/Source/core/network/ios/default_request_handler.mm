/**
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
#include "core/network/ios/default_request_handler.h"
#import "WXConvertUtility.h"
#import "WXSDKManager.h"
#import "WXComponentManager.h"
#include <string>

namespace weex {
namespace core {
namespace network {
    DefaultRequestHandler::DefaultRequestHandler() {}

    DefaultRequestHandler::~DefaultRequestHandler() {}

    void DefaultRequestHandler::Send(const char* instance_id, const char* url, Callback callback) {
        NSURL* nsURL = [NSURL URLWithString:NSSTRING(url)];
        WXSDKInstance *instance = [WXSDKManager instanceForID:@(instance_id)];
        [instance.apmInstance onStage:KEY_PAGE_STAGES_DOWN_JS_START];
        __weak typeof(WXSDKInstance*) weakInstance = instance;
        [[WXSDKManager bridgeMgr] DownloadJS:@(instance_id) url:nsURL completion:^(NSString *script) {
            __strong typeof(weakInstance) strongInstance = weakInstance;
            if (!strongInstance) {
                return;
            }
            [strongInstance.apmInstance onStage:KEY_PAGE_STAGES_DOWN_JS_END];
            WXPerformBlockOnBridgeThread(^{
                NSString* bundleType = @"Vue";
                callback([script UTF8String] ? : "", [bundleType UTF8String]);
            });
        }];
    }

    RequestHandler* RequestHandler::CreateDefaultHandler() {
        return new DefaultRequestHandler();
    }
}
}
}
