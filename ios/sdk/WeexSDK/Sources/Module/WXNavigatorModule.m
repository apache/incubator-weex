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

#import "WXNavigatorModule.h"
#import "WXSDKManager.h"
#import "WXUtility.h"
#import "WXBaseViewController.h"
#import "WXNavigationProtocol.h"
#import "WXHandlerFactory.h"
#import "WXConvert.h"

@implementation WXNavigatorModule

@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(open:success:failure:))
WX_EXPORT_METHOD(@selector(close:success:failure:))
WX_EXPORT_METHOD(@selector(push:callback:))
WX_EXPORT_METHOD(@selector(pop:callback:))
WX_EXPORT_METHOD(@selector(setNavBarBackgroundColor:callback:))
WX_EXPORT_METHOD(@selector(setNavBarLeftItem:callback:))
WX_EXPORT_METHOD(@selector(clearNavBarLeftItem:callback:))
WX_EXPORT_METHOD(@selector(setNavBarRightItem:callback:))
WX_EXPORT_METHOD(@selector(clearNavBarRightItem:callback:))
WX_EXPORT_METHOD(@selector(setNavBarMoreItem:callback:))
WX_EXPORT_METHOD(@selector(clearNavBarMoreItem:callback:))
WX_EXPORT_METHOD(@selector(setNavBarTitle:callback:))
WX_EXPORT_METHOD(@selector(clearNavBarTitle:callback:))
WX_EXPORT_METHOD(@selector(setNavBarHidden:callback:))

- (id<WXNavigationProtocol>)navigator
{
    id<WXNavigationProtocol> navigator = [WXHandlerFactory handlerForProtocol:@protocol(WXNavigationProtocol)];
    return navigator;
}

#pragma mark Weex Application Interface

- (void)open:(NSDictionary *)param success:(WXModuleCallback)success failure:(WXModuleCallback)failure
{
    id<WXNavigationProtocol> navigator = [self navigator];
    UIViewController *container = self.weexInstance.viewController;
    if (navigator && [navigator respondsToSelector:@selector(open:success:failure:withContainer:)]) {
        [navigator open:param success:success failure:failure withContainer:container];
    }
}
    
- (void)close:(NSDictionary *)param success:(WXModuleCallback)success failure:(WXModuleCallback)failure
{
    id<WXNavigationProtocol> navigator = [self navigator];
    UIViewController *container = self.weexInstance.viewController;
    if (navigator && [navigator respondsToSelector:@selector(close:success:failure:withContainer:)]) {
        [navigator close:param success:success failure:failure withContainer:container];
    }
}
    
- (void)push:(NSDictionary *)param callback:(WXModuleCallback)callback
{
    id<WXNavigationProtocol> navigator = [self navigator];
    UIViewController *container = self.weexInstance.viewController;
    [navigator pushViewControllerWithParam:param completion:^(NSString *code, NSDictionary *responseData) {
        if (callback && code) {
            callback(code);
        }
    } withContainer:container];
}

- (void)pop:(NSDictionary *)param callback:(WXModuleCallback)callback
{
    id<WXNavigationProtocol> navigator = [self navigator];
    UIViewController *container = self.weexInstance.viewController;
    [navigator popViewControllerWithParam:param completion:^(NSString *code, NSDictionary *responseData) {
        if (callback && code) {
            callback(code);
        }
    } withContainer:container];
}

- (void)setNavBarHidden:(NSDictionary*)param callback:(WXModuleCallback)callback
{
    NSString *result = MSG_FAILED;
    if ([[NSArray arrayWithObjects:@"0",@"1",@0,@1, nil] containsObject:param[@"hidden"]]) {
        id<WXNavigationProtocol> navigator = [self navigator];
        [navigator setNavigationBarHidden:[param[@"hidden"] boolValue] animated:[param[@"animated"] boolValue] withContainer:self.weexInstance.viewController];
        result = MSG_SUCCESS;
    }
    if (callback) {
        callback(result);
    }
}

#pragma mark Navigation Setup

- (void)setNavBarBackgroundColor:(NSDictionary *)param callback:(WXModuleCallback)callback
{
    NSString *backgroundColor = param[@"backgroundColor"];
    if (!backgroundColor) {
        if (callback) {
            callback(MSG_PARAM_ERR);
        }
    }
    
    id<WXNavigationProtocol> navigator = [self navigator];
    UIViewController *container = self.weexInstance.viewController;
    [navigator setNavigationBackgroundColor:[WXConvert UIColor:backgroundColor] withContainer:container];
    if (callback) {
        callback(MSG_SUCCESS);
    }
}

- (void)setNavBarRightItem:(NSDictionary *)param callback:(WXModuleCallback)callback
{
    [self setNavigationItemWithParam:param position:WXNavigationItemPositionRight withCallback:callback];
}

- (void)clearNavBarRightItem:(NSDictionary *)param callback:(WXModuleCallback)callback
{
    [self clearNavigationItemWithParam:param position:WXNavigationItemPositionRight withCallback:callback];
}

- (void)setNavBarLeftItem:(NSDictionary *)param callback:(WXModuleCallback)callback
{
    [self setNavigationItemWithParam:param position:WXNavigationItemPositionLeft withCallback:callback];
}

- (void)clearNavBarLeftItem:(NSDictionary *)param callback:(WXModuleCallback)callback
{
    [self clearNavigationItemWithParam:param position:WXNavigationItemPositionLeft withCallback:callback];
}

- (void)setNavBarMoreItem:(NSDictionary *)param callback:(WXModuleCallback)callback
{
    [self setNavigationItemWithParam:param position:WXNavigationItemPositionMore withCallback:callback];
}

- (void)clearNavBarMoreItem:(NSDictionary *)param callback:(WXModuleCallback)callback
{
    [self clearNavigationItemWithParam:param position:WXNavigationItemPositionMore withCallback:callback];
}

- (void)setNavBarTitle:(NSDictionary *)param callback:(WXModuleCallback)callback
{
    [self setNavigationItemWithParam:param position:WXNavigationItemPositionCenter withCallback:callback];
}

- (void)clearNavBarTitle:(NSDictionary *)param callback:(WXModuleCallback)callback
{
    [self clearNavigationItemWithParam:param position:WXNavigationItemPositionCenter withCallback:callback];
}

- (void)setNavigationItemWithParam:(NSDictionary *)param position:(WXNavigationItemPosition)position withCallback:(WXModuleCallback)callback
{
    id<WXNavigationProtocol> navigator = [self navigator];
    UIViewController *container = self.weexInstance.viewController;
    
    NSMutableDictionary *mutableParam = [param mutableCopy];
    
    if (self.weexInstance.instanceId) {
        [mutableParam setObject:self.weexInstance.instanceId forKey:@"instanceId"];
    }
    
    [navigator setNavigationItemWithParam:mutableParam position:position completion:^(NSString *code, NSDictionary *responseData) {
        if (callback && code) {
            callback(code);
        }
    } withContainer:container];
}

- (void)clearNavigationItemWithParam:(NSDictionary *)param position:(WXNavigationItemPosition)position withCallback:(WXModuleCallback)callback
{
    id<WXNavigationProtocol> navigator = [self navigator];
    UIViewController *container = self.weexInstance.viewController;
    [navigator clearNavigationItemWithParam:param position:position completion:^(NSString *code, NSDictionary *responseData) {
        if (callback && code) {
            callback(code);
        }
    } withContainer:container];
}

@end
