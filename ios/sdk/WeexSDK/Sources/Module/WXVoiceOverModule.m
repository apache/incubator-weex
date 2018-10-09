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
#import "WXThreadSafeMutableDictionary.h"

@implementation WXVoiceOverModule
{
    BOOL _voiceOverStatus;
    WXThreadSafeMutableDictionary * _announcementCallback;
}
@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(read:callback:))
WX_EXPORT_METHOD(@selector(focusToElement:))
WX_EXPORT_METHOD_SYNC(@selector(isVoiceOverOn))
- (instancetype)init
{
    if ([super init]) {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(voiceOverStatusListener) name:UIAccessibilityVoiceOverStatusChanged object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(voiceOverAnnounceFinish:) name:UIAccessibilityAnnouncementDidFinishNotification object:nil];
        _voiceOverStatus = UIAccessibilityIsVoiceOverRunning();
        _announcementCallback = [WXThreadSafeMutableDictionary new];
    }
    return self;
}
- (void)read:(NSDictionary*)param callback:(WXKeepAliveCallback)callback
{
    if (![param isKindOfClass:[NSDictionary class]]) {
        WXLogError(@"first param must be json type");
        return;
    }
    NSString * string = nil;
    if (param[@"value"]) {
        string = [WXConvert NSString:param[@"value"]];
    }
    
    if (!string ) {
        NSString * errorDesc = @"you didn't specify any value to read";
        WXLogError(@"%@", errorDesc);
        if (callback) {
            callback(@{@"success":@false,@"errorDesc": errorDesc}, NO);
        }
        return;
    }
    
    if (![self isVoiceOverOn]) {
        NSString * errorDesc = @"please check the voice status";
        WXLogInfo(@"%@",errorDesc);
        if (callback) {
            callback(@{@"success":@false, @"errorDesc":errorDesc}, NO);
        }
        return;
    }
    
    float delay = 0.5;
    if (param[@"delay"]) {
        delay = [param[@"delay"] floatValue];
    }
    if (callback) {
        NSMutableArray * callbacks = (NSMutableArray*)_announcementCallback[string];
        if (callbacks) {
            [callbacks addObject:callback];
        } else {
            callbacks = [NSMutableArray arrayWithObject:callback];
        }
        [_announcementCallback setObject:callbacks forKey:string];
    }
    
    if ([string isKindOfClass:[NSString class]]) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, delay * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
            UIAccessibilityPostNotification(UIAccessibilityAnnouncementNotification, string);
        });
    }
}

- (void)voiceOverAnnounceFinish:(NSNotification*)notification
{
    NSString * string = notification.userInfo[UIAccessibilityAnnouncementKeyStringValue];
    if (!_announcementCallback[string]) {
        return;
    }
    BOOL announcementResult = [notification.userInfo[UIAccessibilityAnnouncementKeyWasSuccessful] boolValue];
    
    
    NSMutableArray * callbacks = (NSMutableArray*)_announcementCallback[string];
    if(![callbacks count]) {
        return;
    }
    
    for (WXKeepAliveCallback callback in callbacks) {
        callback(@{@"success":@(announcementResult),
                   @"value":string}, NO);
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
    if (_voiceOverStatus != [self isVoiceOverOn]) {
        _voiceOverStatus = [self isVoiceOverOn];
        if ([weexInstance checkModuleEventRegistered:@"WXVoiceOverStatusChanged" moduleClassName:NSStringFromClass([self class])]) {
            
            [weexInstance fireModuleEvent:[self class] eventName:@"WXVoiceOverStatusChanged" params:@{@"status":[self isVoiceOverOn]?@"on":@"off"}];
        }
    }
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIAccessibilityVoiceOverStatusChanged object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIAccessibilityAnnouncementDidFinishNotification object:nil];
}

@end
