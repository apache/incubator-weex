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

#import "WXModalUIModule.h"
#import <objc/runtime.h>
#import "WXUtility.h"
#import "WXAssert.h"

typedef enum : NSUInteger {
    WXModalTypeToast = 1,
    WXModalTypeAlert,
    WXModalTypeConfirm,
    WXModalTypePrompt
} WXModalType;

@interface WXToastInfo : NSObject

@property (nonatomic, strong) UIView *toastView;
@property (nonatomic, weak) UIView *superView;
@property (nonatomic, weak) WXSDKInstance *instance;
@property (nonatomic, assign) double duration;

@end

@implementation WXToastInfo

@end

@interface WXToastManager : NSObject

@property (strong, nonatomic) NSMutableArray<WXToastInfo *> *toastQueue;
@property (strong, nonatomic) UIView *toastingView;

+ (WXToastManager *)sharedManager;

@end

@implementation WXToastManager

+ (WXToastManager *)sharedManager{
    static WXToastManager * shareInstance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        shareInstance = [[WXToastManager alloc] init];
        shareInstance.toastQueue = [NSMutableArray new];
    });
    return shareInstance;
}

@end

@interface WXModalUIModule () <UIAlertViewDelegate>

@end

@implementation WXModalUIModule

@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(toast:))
WX_EXPORT_METHOD(@selector(alert:callback:))
WX_EXPORT_METHOD(@selector(confirm:callback:))
WX_EXPORT_METHOD(@selector(prompt:callback:))

- (instancetype)init
{
    if (self = [super init]) {//!OCLint
    }
    
    return self;
}

- (void)dealloc
{
}

#pragma mark - Toast

static const double WXToastDefaultDuration = 3.0;
static const CGFloat WXToastDefaultFontSize = 16.0;
static const CGFloat WXToastDefaultWidth = 230.0;
static const CGFloat WXToastDefaultHeight = 30.0;
static const CGFloat WXToastDefaultPadding = 30.0;

- (void)toast:(NSDictionary *)param
{
    NSString *message = [self stringValue:param[@"message"]];
    
    if (!message) return;
    
    double duration = [param[@"duration"] doubleValue];
    if (duration <= 0) {
        duration = WXToastDefaultDuration;
    }
    
    WXPerformBlockOnMainThread(^{
        [self toast:message duration:duration];
    });
}

- (void)toast:(NSString *)message duration:(double)duration
{
    WXAssertMainThread();
    UIView *superView = self.weexInstance.rootView.window;
    if (!superView) {
        superView =  self.weexInstance.rootView;
    }
    UIView *toastView = [self toastViewForMessage:message superView:superView];
    WXToastInfo *info = [WXToastInfo new];
    info.instance = self.weexInstance;
    info.toastView = toastView;
    info.superView = superView;
    info.duration = duration;
    [[WXToastManager sharedManager].toastQueue addObject:info];
    
    if (![WXToastManager sharedManager].toastingView) {
        [self showToast:toastView superView:superView duration:duration];
    }
}

- (UIView *)toastViewForMessage:(NSString *)message superView:(UIView *)superView
{
    CGFloat padding = WXToastDefaultPadding;
    UILabel *messageLabel = [[UILabel alloc] initWithFrame:CGRectMake(padding/2, padding/2, WXToastDefaultWidth, WXToastDefaultHeight)];
    messageLabel.numberOfLines =  0;
    messageLabel.textAlignment = NSTextAlignmentCenter;
    messageLabel.text = message;
    messageLabel.font = [UIFont boldSystemFontOfSize:WXToastDefaultFontSize];
    messageLabel.lineBreakMode = NSLineBreakByTruncatingTail;
    messageLabel.textColor = [UIColor whiteColor];
    messageLabel.backgroundColor = [UIColor clearColor];
    [messageLabel sizeToFit];

    UIView *toastView = [[UIView alloc] initWithFrame:
                         CGRectMake(
                                    (superView.frame.size.width-messageLabel.frame.size.width-padding)/2,
                                    (superView.frame.size.height-messageLabel.frame.size.height-padding)/2,
                                    messageLabel.frame.size.width+padding,
                                    messageLabel.frame.size.height+padding
                                    )];
    
    CGPoint point = CGPointZero;
    UIWindow *window = [[[UIApplication sharedApplication] windows] objectAtIndex:0];
    
    // adjust to screen orientation
    UIInterfaceOrientation orientation = (UIInterfaceOrientation)[[UIApplication sharedApplication] statusBarOrientation];
    switch (orientation) {
        case UIDeviceOrientationPortrait: {
            point = CGPointMake(window.frame.size.width/2, window.frame.size.height/2);
            break;
        }
        case UIDeviceOrientationPortraitUpsideDown: {
            toastView.transform = CGAffineTransformMakeRotation(M_PI);
            float width = window.frame.size.width;
            float height = window.frame.size.height;
            point = CGPointMake(width/2, height/2);
            break;
        }
        case UIDeviceOrientationLandscapeLeft: {
            toastView.transform = CGAffineTransformMakeRotation(M_PI/2); //rotation in radians
            point = CGPointMake(window.frame.size.width/2, window.frame.size.height/2);
            break;
        }
        case UIDeviceOrientationLandscapeRight: {
            toastView.transform = CGAffineTransformMakeRotation(-M_PI/2);
            point = CGPointMake(window.frame.size.width/2, window.frame.size.height/2);
            break;
        }
        default:
            break;
    }
    
    toastView.center = point;
    toastView.frame = CGRectIntegral(toastView.frame);
    
    [toastView addSubview:messageLabel];
    toastView.layer.cornerRadius = 7;
    toastView.backgroundColor=[UIColor colorWithWhite:0 alpha:0.7];
    
    return toastView;
}

- (void)showToast:(UIView *)toastView superView:(UIView *)superView duration:(double)duration
{
    if (!toastView || !superView) {
        return;
    }
    
    [WXToastManager sharedManager].toastingView = toastView;
    [superView addSubview:toastView];
    __weak typeof(self) weakSelf = self;
    [UIView animateWithDuration:0.2 delay:duration options:UIViewAnimationOptionCurveEaseInOut animations:^{
        toastView.transform = CGAffineTransformConcat(toastView.transform, CGAffineTransformMakeScale(0.8, 0.8)) ;
    } completion:^(BOOL finished) {
        [UIView animateWithDuration:0.2 delay:0.2 options:UIViewAnimationOptionCurveEaseInOut animations:^{
            toastView.alpha = 0;
        } completion:^(BOOL finished){
            [toastView removeFromSuperview];
            [WXToastManager sharedManager].toastingView = nil;
            
            NSMutableArray *queue = [WXToastManager sharedManager].toastQueue;
            if (queue.count > 0) {
                [queue removeObjectAtIndex:0];
                
                // remove invalid toasts
                for (NSInteger i = [queue count] - 1; i >= 0; i --) {
                    WXToastInfo *info = queue[i];
                    if (info.instance == nil) {
                        [queue removeObjectAtIndex:i];
                    }
                }
                
                if (queue.count > 0) {
                    WXToastInfo *info = [queue firstObject];
                    [weakSelf showToast:info.toastView superView:info.superView duration:info.duration];
                }
            }
        }];
    }];
}

#pragma mark - Alert

- (void)alert:(NSDictionary *)param callback:(WXModuleKeepAliveCallback)callback
{
    NSString *message = [self stringValue:param[@"message"]];
    NSString *okTitle = [self stringValue:param[@"okTitle"]];
    
    if ([WXUtility isBlankString:okTitle]) {
        okTitle = @"OK";
    }

    [self alert:message okTitle:nil cancelTitle:okTitle defaultText:nil type:WXModalTypeAlert callback:callback];
}

#pragma mark - Confirm

- (void)confirm:(NSDictionary *)param callback:(WXModuleKeepAliveCallback)callback
{
    NSString *message = [self stringValue:param[@"message"]];
    NSString *okTitle = [self stringValue:param[@"okTitle"]];
    NSString *cancelTitle = [self stringValue:param[@"cancelTitle"]];
    
    if ([WXUtility isBlankString:okTitle]) {
        okTitle = @"OK";
    }
    if ([WXUtility isBlankString:cancelTitle]) {
        cancelTitle = @"Cancel";
    }

    [self alert:message okTitle:okTitle cancelTitle:cancelTitle defaultText:nil type:WXModalTypeConfirm callback:callback];
}

#pragma mark - Prompt

- (void)prompt:(NSDictionary *)param callback:(WXModuleKeepAliveCallback)callback
{
    NSString *message = [self stringValue:param[@"message"]];
    NSString *defaultValue = [self stringValue:param[@"default"]];
    NSString *okTitle = [self stringValue:param[@"okTitle"]];
    NSString *cancelTitle = [self stringValue:param[@"cancelTitle"]];
    
    if ([WXUtility isBlankString:okTitle]) {
        okTitle = @"OK";
    }
    if ([WXUtility isBlankString:cancelTitle]) {
        cancelTitle = @"Cancel";
    }
    
    [self alert:message okTitle:okTitle cancelTitle:cancelTitle defaultText:defaultValue type:WXModalTypePrompt callback:callback];
}

#pragma mark - Private

- (void)alert:(NSString *)message okTitle:(NSString *)okTitle cancelTitle:(NSString *)cancelTitle defaultText:(NSString *)defaultText type:(WXModalType)type callback:(WXModuleKeepAliveCallback)callback
{
    if (!message) {
        if (callback) {
            callback(@"Error: message should be passed correctly.",NO);
        }
        return;
    }
    
    __weak WXModalUIModule* wModuleSelf = self;
    
    WXPerformBlockOnMainThread(^{
        __strong WXModalUIModule* sModuleSelf = wModuleSelf;
        
        if (sModuleSelf) {
            void (^handleAction)(UIAlertController*, UIAlertAction*) = ^(UIAlertController* alertView, UIAlertAction* action) {
                if (callback) {
                    id result = @"";
                    switch (type) {
                        case WXModalTypeAlert: {
                            break;
                        }
                        case WXModalTypeConfirm: {
                            result = action.title ?: @"";
                            break;
                        }
                        case WXModalTypePrompt: {
                            NSString *clickTitle = action.title ?: @"";
                            NSString *text = [alertView.textFields firstObject].text ?: @"";
                            result = @{ @"result": clickTitle, @"data": text };
                            break;
                        }
                        default:
                            break;
                    }
                    
                    callback(result, NO);
                }
            };
            
            UIAlertController *alertView = [UIAlertController alertControllerWithTitle:@"" message:message preferredStyle:UIAlertControllerStyleAlert];
            __weak UIAlertController* weakAlertView = alertView;
            
            UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:cancelTitle ?: @"OK" style:UIAlertActionStyleCancel handler:^(UIAlertAction *action) {
                handleAction(weakAlertView, action);
            }];
            
            UIAlertAction *OKAction = nil;
            if (okTitle) {
                OKAction = [UIAlertAction actionWithTitle:okTitle style:UIAlertActionStyleDefault handler:^(UIAlertAction *action) {
                    handleAction(weakAlertView, action);
                }];
            }
            
            if (type == WXModalTypePrompt) {
                [alertView addTextFieldWithConfigurationHandler:^(UITextField * _Nonnull textField) {
                    textField.placeholder = defaultText ?: @"";
                }];
            }
            
            [alertView addAction:cancelAction];
            
            if (OKAction) {
                [alertView addAction:OKAction];
            }
            
            UIViewController* selfVC = sModuleSelf.weexInstance.viewController;
            if (selfVC == nil) {
                selfVC = [UIApplication sharedApplication].delegate.window.rootViewController;
                WXLogWarning(@"Unable to find view controller for instance. Use Application root vc to show AlertViewController.");
            }
            
            if (selfVC) {
                [selfVC presentViewController:alertView animated:YES completion:nil];
            }
        }
    });
}

- (NSString*)stringValue:(id)value
{
    if ([value isKindOfClass:[NSString class]]) {
        return value;
    }
    if ([value isKindOfClass:[NSNumber class]]) {
        return [value stringValue];
    }
    return nil;
}

@end
