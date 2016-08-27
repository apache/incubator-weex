//
//  WXGlobalEventModule.m
//  WeexSDK
//
//  Created by zifan.zx on 8/25/16.
//  Copyright © 2016 taobao. All rights reserved.
//

#import "WXGlobalEventModule.h"
#import "WXThreadSafeMutableDictionary.h"

@interface WXGlobalEventModule()
@property WXThreadSafeMutableDictionary *eventCallback;
@end
@implementation WXGlobalEventModule

@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(addEventListener:callback:))
WX_EXPORT_METHOD(@selector(removeEventListener:))

- (instancetype)init {
    if (self = [super init]) {
        _eventCallback = [WXThreadSafeMutableDictionary new];
    }
    return self;
}

- (void)addEventListener:(NSString *)event callback:(WXModuleKeepAliveCallback)callback
{
    _eventCallback[event] = callback;
    [[NSNotificationCenter defaultCenter] addObserver:weexInstance selector:@selector(fireGlobalEvent:) name:event object:weexInstance];
}

- (void)removeEventListener:(NSString *)event
{
    [_eventCallback removeObjectForKey:event];
    [[NSNotificationCenter defaultCenter] removeObserver:weexInstance forKeyPath:event];
}

- (void)fireGlobalEvent:(NSNotification *)notification
{
    NSDictionary * userInfo = notification.userInfo;
    WXSDKInstance * userWeexInstance = userInfo[@"weexInstance"];
    NSDictionary * param = userInfo[@"param"];
    bool keepAlive = [userInfo[@"keepAlive"] boolValue];
    if (userWeexInstance == weexInstance) {
        WXModuleKeepAliveCallback callback = _eventCallback[notification.name];
        callback(param, keepAlive);
    }
}

@end
