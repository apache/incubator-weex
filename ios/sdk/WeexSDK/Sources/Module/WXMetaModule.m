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
#import "WXMetaModule.h"
#import "WXConvert.h"
#import "WXUtility.h"
#import "WXSDKInstance_private.h"
#import "WXSDKInstance.h"

@implementation WXMetaModule

@synthesize weexInstance;

WX_EXPORT_METHOD_SYNC(@selector(setViewport:))

- (void)setViewport:(NSDictionary *)viewportArguments
{
    CGFloat viewportWidthFloat;
    id viewportWidth = viewportArguments[@"width"];
    if ([viewportWidth isKindOfClass:[NSString class]]) {
        if ([viewportWidth isEqualToString:@"device-width"]) {
            viewportWidthFloat = [WXUtility portraitScreenSize].width;
        } else if ([viewportWidth isEqualToString:@"device-height"]) {
            viewportWidthFloat = [WXUtility portraitScreenSize].height;
        } else {
            viewportWidthFloat = [WXConvert CGFloat:viewportWidth];
        }
    } else {
        viewportWidthFloat = [WXConvert CGFloat:viewportWidth];
    }
    NSString *viewportFit = viewportArguments[@"viewport-fit"];
    if (viewportFit) {
        // handle viewport-fit for iPhone X
        // viewportFit can be `cover`,`contain`
        
        //handle cover
        if ([viewportFit isEqualToString:@"cover"]) {
            WXPerformBlockSyncOnMainThread(^{
                weexInstance.frame = [UIScreen mainScreen].bounds;
            });
        }else if ([viewportFit isEqualToString:@"contain"]) {
            WXPerformBlockSyncOnMainThread(^{
                CGRect newFrame = weexInstance.rootView.frame;
                // here we only handle the full screen case.
                if (CGRectEqualToRect(newFrame, [UIScreen mainScreen].bounds)) {
                    if (@available(iOS 11.0, *)) {
                        newFrame = UIEdgeInsetsInsetRect(weexInstance.rootView.frame, weexInstance.rootView.safeAreaInsets);
                    } else {
                        // Fallback on earlier versions
                    }
                    weexInstance.frame = newFrame;
                }
            });
        }
    }
    
    if (viewportWidthFloat > 0) {
        self.weexInstance.viewportWidth = viewportWidthFloat;
    }
}

@end
