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

#import "MainViewController.h"

#import <WeexSDK/WXDebugTool.h>
#import <WeexSDK/WXPrerenderManager.h>
#import <WeexSDK/WXMonitor.h>
#import <WeexSDK/WXSDKManager.h>
#import <WeexSDK/WXSDKInstance.h>
#import <WeexSDK/WXUtility.h>

@interface MainViewController ()

@property (nonatomic, strong) WXSDKInstance *instance;
@property (nonatomic, strong) UIView *weexView;
@property (nonatomic, assign) CGFloat weexHeight;

@end

@implementation MainViewController

- (void)dealloc {
    [_instance destroyInstance];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    self.view.backgroundColor = [UIColor whiteColor];
    [self.view setClipsToBounds:YES];

    _weexHeight = self.view.frame.size.height - 64;
    [self render];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void)viewDidLayoutSubviews
{
    _weexHeight = self.view.frame.size.height;
    UIEdgeInsets safeArea = UIEdgeInsetsZero;
#ifdef __IPHONE_11_0
    if (@available(iOS 11.0, *)) {
        safeArea = self.view.safeAreaInsets;
    }
#endif
    _instance.frame = CGRectMake(safeArea.left, safeArea.top,
                                 self.view.frame.size.width - safeArea.left - safeArea.right,
                                 _weexHeight - safeArea.bottom);
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    [self updateInstanceState:WeexInstanceAppear];
}

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
    [self updateInstanceState:WeexInstanceDisappear];
}

- (void)render {
    CGFloat width = self.view.frame.size.width;
    [_instance destroyInstance];
    _instance = [[WXSDKInstance alloc] init];
    if([WXPrerenderManager isTaskExist:[self.url absoluteString]]){
        _instance = [WXPrerenderManager instanceFromUrl:self.url.absoluteString];
    }

    _instance.viewController = self;
    UIEdgeInsets safeArea = UIEdgeInsetsZero;

#ifdef __IPHONE_11_0
    if (@available(iOS 11.0, *)) {
        safeArea = self.view.safeAreaInsets;
    }
#endif
    _instance.frame = CGRectMake(self.view.frame.size.width - width, 0,
                                 width, _weexHeight - safeArea.bottom);

    __weak typeof(self) weakSelf = self;
    _instance.onCreate = ^(UIView *view) {
        [weakSelf.weexView removeFromSuperview];
        weakSelf.weexView = view;
        [weakSelf.view addSubview:weakSelf.weexView];
        UIAccessibilityPostNotification(UIAccessibilityScreenChangedNotification, weakSelf.weexView);
    };
    _instance.onFailed = ^(NSError *error) {
        if ([[error domain] isEqualToString:@"1"]) {
            dispatch_async(dispatch_get_main_queue(), ^{
                NSMutableString *errMsg=[NSMutableString new];
                [errMsg appendFormat:@"ErrorType:%@\n",[error domain]];
                [errMsg appendFormat:@"ErrorCode:%ld\n",(long)[error code]];
                [errMsg appendFormat:@"ErrorInfo:%@\n", [error userInfo]];
                WXLogError(@"%@", errMsg);
            });
        }
    };

    _instance.renderFinish = ^(UIView *view) {
        WXLogDebug(@"%@", @"Finish rendering.");
        [weakSelf updateInstanceState:WeexInstanceAppear];
    };

    _instance.updateFinish = ^(UIView *view) {
        WXLogDebug(@"%@", @"Finish update.");
    };
    if (!self.url) {
        WXLogError(@"Error: Render URL is nil.");
        return;
    }
    if([WXPrerenderManager isTaskExist:[self.url absoluteString]]){
        WX_MONITOR_INSTANCE_PERF_START(WXPTJSDownload, _instance);
        WX_MONITOR_INSTANCE_PERF_END(WXPTJSDownload, _instance);
        WX_MONITOR_INSTANCE_PERF_START(WXPTFirstScreenRender, _instance);
        WX_MONITOR_INSTANCE_PERF_START(WXPTAllRender, _instance);
        [WXPrerenderManager renderFromCache:[self.url absoluteString]];
        return;
    }
    NSString *randomURL = [NSString stringWithFormat:@"%@%@random=%d", self.url.absoluteString, self.url.query?@"&":@"?", arc4random()];
    [_instance renderWithURL:[NSURL URLWithString:randomURL] options:@{@"bundleUrl":self.url.absoluteString} data:nil];
}

- (void)updateInstanceState:(WXState)state {
    if (_instance && _instance.state != state) {
        _instance.state = state;

        if (state == WeexInstanceAppear) {
            [[WXSDKManager bridgeMgr] fireEvent:_instance.instanceId ref:WX_SDK_ROOT_REF type:@"viewappear" params:nil domChanges:nil];
        }
        else if (state == WeexInstanceDisappear) {
            [[WXSDKManager bridgeMgr] fireEvent:_instance.instanceId ref:WX_SDK_ROOT_REF type:@"viewdisappear" params:nil domChanges:nil];
        }
    }
}

@end
