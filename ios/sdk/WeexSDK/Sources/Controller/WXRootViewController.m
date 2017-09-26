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

#import "WXRootViewController.h"
#import "WXBaseViewController.h"
#import "WXThreadSafeMutableArray.h"
#import "WXDefine.h"

typedef void(^OperationBlock)(void);

@interface WXRootViewController() <UIGestureRecognizerDelegate>

@property(nonatomic, strong) WXThreadSafeMutableArray *operationArray;
@property (nonatomic, assign) BOOL operationInProcess;

@end

@implementation WXRootViewController


- (void)viewDidLoad
{
    self.interactivePopGestureRecognizer.delegate = self;
}

- (id)initWithSourceURL:(NSURL *)sourceURL
{
    WXBaseViewController *baseViewController = [[WXBaseViewController alloc]initWithSourceURL:sourceURL];
    
    return [super initWithRootViewController:baseViewController];
}

//reduced pop/push animation in iOS 7
- (UIViewController *)popViewControllerAnimated:(BOOL)animated
{
    if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"8.0"))
        return [super popViewControllerAnimated:animated];
    
    __weak typeof(self) weakSelf = self;
    [self addOperationBlock:^{
        if ([self.viewControllers count] > 0 ) {
            UIViewController *viewController = [super popViewControllerAnimated:NO];
            if (!viewController) {
                weakSelf.operationInProcess = NO;
            }
        }
    }];
    
    return nil;
}

- (NSArray<UIViewController *> *)popToViewController:(UIViewController *)viewController animated:(BOOL)animated
{
    if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"8.0"))
        return [super popToViewController:viewController animated:animated];
    
    __weak typeof(self) weakSelf = self;
    [self addOperationBlock:^{
        if ([weakSelf.viewControllers containsObject:viewController]) {
            NSArray *viewControllers = [super popToViewController:viewController animated:NO];
            if (viewControllers.count == 0) {
                weakSelf.operationInProcess = NO;
            }
        } else {
            weakSelf.operationInProcess = NO;
        }
    }];
    
    return nil;
}

- (NSArray<UIViewController *> *)popToRootViewControllerAnimated:(BOOL)animated
{
    if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"8.0"))
        return [super popToRootViewControllerAnimated:animated];
    
    __weak typeof(self) weakSelf = self;
    [self addOperationBlock:^{
        NSArray *viewControllers = [super popToRootViewControllerAnimated:NO];
        if (viewControllers.count == 0) {
            weakSelf.operationInProcess = NO;
        }
    }];
    return nil;
}

- (void)pushViewController:(UIViewController *)viewController animated:(BOOL)animated
{
    if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"8.0"))
        return [super pushViewController:viewController animated:animated];
    
    [self addOperationBlock:^{
        [super pushViewController:viewController animated:NO];
    }];
}

- (void)addOperationBlock:(OperationBlock)operation
{
    
    if (self.operationInProcess && [self.operationArray count]) {
        [self.operationArray addObject:[operation copy]];
    } else {
        _operationInProcess = YES;
        operation();
    }
}

#pragma mark- UIGestureRecognizerDelegate

- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer
{
    if ([self.viewControllers count] == 1) {
        return NO;
    }
    return YES;
}

- (NSMutableArray *)pendingBlocks
{
    
    if (nil == _operationArray) {
        _operationArray = [[WXThreadSafeMutableArray alloc] init];
    }
    
    return _operationArray;
}

@end
