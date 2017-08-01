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

#import "WXVoiceOverModule.h"
#import "WXUtility.h"
#import "WXComponent_internal.h"
#import "WXComponentManager.h"

@implementation WXVoiceOverModule
@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(read:param:))
WX_EXPORT_METHOD(@selector(focusToElement:))
WX_EXPORT_METHOD_SYNC(@selector(isVoiceOverOn))
- (instancetype)init {
    if ([super init]) {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(voiceOverStatusListener) name:UIAccessibilityVoiceOverStatusChanged object:nil];
    }
    return self;
}
- (void)read:(NSString *)string param:(NSDictionary*)param
{
    float delay = 0.5;
    if (param[@"delay"]) {
        delay = [param[@"delay"] floatValue];
    }
    if ([string isKindOfClass:[NSString class]]) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, delay * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
            UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, string);
        });
    }
}
- (void)focusToElement:(NSString *)ref {
    WXPerformBlockOnComponentThread(^{
        WXComponent * compoent = [weexInstance componentForRef:ref];
        if (compoent) {
            WXPerformBlockOnMainThread(^{
                UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification, compoent.view);
            });
        }
    });
}

- (BOOL)isVoiceOverOn
{
    return UIAccessibilityIsVoiceOverRunning();
}

- (void)voiceOverStatusListener
{
    if ([weexInstance checkModuleEventRegistered:@"WXVoiceOverStatusChanged" moduleClassName:NSStringFromClass([self class])]) {
        [weexInstance fireModuleEvent:[self class] eventName:@"WXVoiceOverStatusChanged" params:nil];
    }
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIAccessibilityVoiceOverStatusChanged object:self];
}

@end
