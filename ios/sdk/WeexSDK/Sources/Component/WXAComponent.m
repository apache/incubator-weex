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

#import "WXAComponent.h"
#import "WXNavigationProtocol.h"
#import "WXHandlerFactory.h"
#import "WXLog.h"
#import "WXComponent+Events.h"
#import "WXURLRewriteProtocol.h"

@interface WXAComponent()

@property (nonatomic, strong) UITapGestureRecognizer *tap;
@property (nonatomic, strong) NSString *href;

@end

@implementation WXAComponent

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance
{
    self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance];
    if (self) {
        _tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(openURL)];
        _tap.delegate = self;
        if (attributes[@"href"]) {
            _href = attributes[@"href"];
        }
    }
    return self;
}

- (void)dealloc
{
    if (_tap.delegate) {
        _tap.delegate = nil;
    }
}

- (void)viewDidLoad
{
    [self.view addGestureRecognizer:_tap];
}

- (void)openURL
{
    if (_href && [_href length] > 0) {
        NSMutableString *newHref = [_href mutableCopy];
        WX_REWRITE_URL(_href, WXResourceTypeLink, self.weexInstance, &newHref)
        if (!newHref) {
            return;
        }
        id<WXNavigationProtocol> navigationHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXNavigationProtocol)];
        if ([navigationHandler respondsToSelector:@selector(pushViewControllerWithParam:
                                                            completion:
                                                            withContainer:)]) {
            __weak typeof(self) weexSelf = self;
            [navigationHandler pushViewControllerWithParam:@{@"url":newHref} completion:^(NSString *code, NSDictionary *responseData) {
                WXLogDebug(@"Push success -> %@", weexSelf.href);
            } withContainer:self.weexInstance.viewController];
        } else {
            WXLogError(@"Event handler of class %@ does not respond to pushViewControllerWithParam", NSStringFromClass([navigationHandler class]));
        }
    }
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    if (attributes[@"href"]) {
        _href = attributes[@"href"];
    }
}

#pragma mark
#pragma gesture delegate

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    if ([gestureRecognizer isKindOfClass:[UITapGestureRecognizer class]] && [otherGestureRecognizer isKindOfClass:[UITapGestureRecognizer class]]) {
        return YES;
    }
    
    return NO;
}

@end
