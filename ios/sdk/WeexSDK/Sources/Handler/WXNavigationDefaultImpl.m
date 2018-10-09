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

#import "WXNavigationDefaultImpl.h"
#import "WXBaseViewController.h"
#import "WXSDKManager.h"
#import "WXComponent.h"
#import "WXImgLoaderProtocol.h"
#import "WXHandlerFactory.h"
#import "WXConvert.h"

@interface WXBarButton :UIButton

@property (nonatomic, strong) NSString *instanceId;
@property (nonatomic, strong) NSString *nodeRef;
@property (nonatomic)  WXNavigationItemPosition position;

@end

@implementation WXBarButton

@end

@implementation WXNavigationDefaultImpl

#pragma mark -
#pragma mark WXNavigationProtocol

- (id<WXImgLoaderProtocol>)imageLoader
{
    static id<WXImgLoaderProtocol> imageLoader;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        imageLoader = [WXHandlerFactory handlerForProtocol:@protocol(WXImgLoaderProtocol)];
    });
    return imageLoader;
}

- (id)navigationControllerOfContainer:(UIViewController *)container
{
    return container.navigationController;
}

- (void)pushViewControllerWithParam:(NSDictionary *)param completion:(WXNavigationResultBlock)block
                      withContainer:(UIViewController *)container
{
    if (0 == [param count] || !param[@"url"] || !container) {
        [self callback:block code:MSG_PARAM_ERR data:nil];
        return;
    }
    
    BOOL animated = YES;
    NSString *obj = [[param objectForKey:@"animated"] lowercaseString];
    if (obj && [obj isEqualToString:@"false"]) {
        animated = NO;
    }
    
    WXBaseViewController *vc = [[WXBaseViewController alloc]initWithSourceURL:[NSURL URLWithString:param[@"url"]]];
    vc.hidesBottomBarWhenPushed = YES;
    [container.navigationController pushViewController:vc animated:animated];
    [self callback:block code:MSG_SUCCESS data:nil];
}

- (void)popViewControllerWithParam:(NSDictionary *)param completion:(WXNavigationResultBlock)block
                     withContainer:(UIViewController *)container
{
    BOOL animated = YES;
    id obj = [param objectForKey:@"animated"];
    if (obj) {
        animated = [WXConvert BOOL:obj];
    }
    
    [container.navigationController popViewControllerAnimated:animated];
    [self callback:block code:MSG_SUCCESS data:nil];
}

- (void)popToRootViewControllerWithParam:(NSDictionary *)param
                              completion:(WXNavigationResultBlock)block
                           withContainer:(UIViewController *)container
{
    BOOL animated = YES;
    NSString *obj = [[param objectForKey:@"animated"] lowercaseString];
    if (obj && [obj isEqualToString:@"false"]) {
        animated = NO;
    }
    
    [container.navigationController popToRootViewControllerAnimated:animated];
    [self callback:block code:MSG_SUCCESS data:nil];
}

- (void)setNavigationBarHidden:(BOOL)hidden animated:(BOOL)animated
                 withContainer:(UIViewController *)container
{
    if (![container isKindOfClass:[WXBaseViewController class]]) {
        return;
    }
    
    container.navigationController.navigationBarHidden = hidden;
}

- (void)setNavigationBackgroundColor:(UIColor *)backgroundColor
                       withContainer:(UIViewController *)container
{
    if (backgroundColor) {
        container.navigationController.navigationBar.barTintColor = backgroundColor;
    }
}

- (void)setNavigationItemWithParam:(NSDictionary *)param
                          position:(WXNavigationItemPosition)position
                        completion:(WXNavigationResultBlock)block
                     withContainer:(UIViewController *)container
{
    switch (position) {
        case WXNavigationItemPositionLeft:
            [self setNaviBarLeftItem:param completion:block withContainer:container];
            break;
        case WXNavigationItemPositionRight:
            [self setNaviBarRightItem:param completion:block withContainer:container];
            break;
        case WXNavigationItemPositionMore:
            break;
        case WXNavigationItemPositionCenter:
            [self setNaviBarTitle:param completion:block withContainer:container];
            break;
        default:
            break;
    }
}

- (void)clearNavigationItemWithParam:(NSDictionary *)param
                            position:(WXNavigationItemPosition)position
                          completion:(WXNavigationResultBlock)block
                       withContainer:(UIViewController *)container
{
    switch (position) {
        case WXNavigationItemPositionLeft:
            [self clearNaviBarLeftItem:param completion:block withContainer:container];
            break;
        case WXNavigationItemPositionRight:
            [self clearNaviBarRightItem:param completion:block withContainer:container];
            break;
        case WXNavigationItemPositionMore:
            break;
        case WXNavigationItemPositionCenter:
            [self clearNaviBarTitle:param completion:block withContainer:container];
            break;
        default:
            break;
    }
}

- (void)setNaviBarLeftItem:(NSDictionary *)param completion:(WXNavigationResultBlock)block
              withContainer:(UIViewController *)container
{
    if (0 == [param count]) {
        [self callback:block code:MSG_PARAM_ERR data:nil];
        return;
    }
    
    UIView *view = [self barButton:param position:WXNavigationItemPositionLeft withContainer:container];
    
    if (!view) {
        [self callback:block code:MSG_FAILED data:nil];
        return;
    }
    
    container.navigationItem.leftBarButtonItem = [[UIBarButtonItem alloc] initWithCustomView:view];
    
    [self callback:block code:MSG_SUCCESS data:nil];
}

- (void)clearNaviBarLeftItem:(NSDictionary *) param completion:(WXNavigationResultBlock)block
                withContainer:(UIViewController *)container
{
    container.navigationItem.leftBarButtonItem = nil;
    
    [self callback:block code:MSG_SUCCESS data:nil];
}

- (void)setNaviBarRightItem:(NSDictionary *)param completion:(WXNavigationResultBlock)block
              withContainer:(UIViewController *)container
{
    if (0 == [param count]) {
        [self callback:block code:MSG_PARAM_ERR data:nil];
        return;
    }
    
    UIView *view = [self barButton:param position:WXNavigationItemPositionRight withContainer:container];
    
    if (!view) {
        [self callback:block code:MSG_FAILED data:nil];
        return;
    }
    
    container.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithCustomView:view];
    
    [self callback:block code:MSG_SUCCESS data:nil];
}

- (void)clearNaviBarRightItem:(NSDictionary *) param completion:(WXNavigationResultBlock)block
                withContainer:(UIViewController *)container
{
    container.navigationItem.rightBarButtonItem = nil;
    
    [self callback:block code:MSG_SUCCESS data:nil];
}

- (void)setNaviBarTitle:(NSDictionary *)param completion:(WXNavigationResultBlock)block
          withContainer:(UIViewController *)container
{
    if (0 == [param count]) {
        [self callback:block code:MSG_PARAM_ERR data:nil];
        return;
    }
    
    container.navigationItem.title = param[@"title"];
    
    [self callback:block code:MSG_SUCCESS data:nil];;
}

- (void)clearNaviBarTitle:(NSDictionary *)param completion:(WXNavigationResultBlock)block
            withContainer:(UIViewController *)container
{
    container.navigationItem.title = @"";
    
    [self callback:block code:MSG_SUCCESS data:nil];
}

- (UIView *)barButton:(NSDictionary *)param position:(WXNavigationItemPosition)position
        withContainer:(UIViewController *)container
{
    if (param[@"title"]) {
        NSString *title = param[@"title"];

        NSDictionary *attribute = @{NSFontAttributeName: [UIFont systemFontOfSize:18]};
        CGSize size = [title boundingRectWithSize:CGSizeMake(70.0f, 18.0f) options: NSStringDrawingTruncatesLastVisibleLine | NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading attributes:attribute context:nil].size;
        
        WXBarButton *button = [WXBarButton buttonWithType:UIButtonTypeRoundedRect];
        button.frame = CGRectMake(0, 0, size.width, size.height);
        button.titleLabel.font = [UIFont systemFontOfSize:16];
        [button setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
        [button setTitleColor:[UIColor whiteColor]  forState:UIControlStateHighlighted];
        [button setTitle:title forState:UIControlStateNormal];
        [button setTitle:title forState:UIControlStateHighlighted];
        [button addTarget:self action:@selector(onClickBarButton:) forControlEvents:UIControlEventTouchUpInside];
        
        button.instanceId = param[@"instanceId"];
        button.nodeRef = param[@"nodeRef"];
        button.position = position;
        
        NSString *color = param[@"titleColor"];
        if (color) {
            [button setTitleColor:[WXConvert UIColor:color] forState:UIControlStateNormal];
            [button setTitleColor:[WXConvert UIColor:color] forState:UIControlStateHighlighted];
        }
        
        return button;
    }
    else if (param[@"icon"]) {
        NSString *icon = param[@"icon"];
        
        if (icon) {
            WXBarButton *button = [WXBarButton buttonWithType:UIButtonTypeRoundedRect];
            button.frame = CGRectMake(0, 0, 25, 25);
            button.instanceId = param[@"instanceId"];
            button.nodeRef = param[@"nodeRef"];
            button.position = position;
            [button addTarget:self action:@selector(onClickBarButton:) forControlEvents:UIControlEventTouchUpInside];
            
            [[self imageLoader] downloadImageWithURL:icon imageFrame:CGRectMake(0, 0, 25, 25) userInfo:@{@"instanceId":self.weexInstance.instanceId} completed:^(UIImage *image, NSError *error, BOOL finished) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    [button setBackgroundImage:image forState:UIControlStateNormal];
                    [button setBackgroundImage:image forState:UIControlStateHighlighted];
                });
            }];
            
            return button;
        }
    }
    
    return nil;
}

- (UIView *)titleView:(NSDictionary *)param
{
    UIView *titleView = nil;
    
    if (param[@"title"]) {
        UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(0.0f, 0.0f, 200.0f, 20.0f)];
        label.backgroundColor = [UIColor clearColor];
        label.textAlignment = NSTextAlignmentCenter;
        label.lineBreakMode = NSLineBreakByTruncatingTail;
        label.textColor = [UIColor whiteColor];
        label.font = [UIFont systemFontOfSize:18.0f];
        label.text = param[@"title"];
        
        UIColor *titleColor = param[@"titleColor"];
        if (titleColor) {
            label.textColor = [WXConvert UIColor:titleColor];
        }
        else {
            label.textColor = [UIColor blueColor];
        }
        titleView = label;
    }
    
    return titleView;
}

- (void)onClickBarButton:(id)sender
{
    WXBarButton *button = (WXBarButton *)sender;
    if (button.instanceId) {
        if (button.nodeRef)
        {
            [[WXSDKManager bridgeMgr] fireEvent:button.instanceId ref:button.nodeRef type:@"click" params:nil domChanges:nil] ;
        }
        else
        {
            NSString *eventType;
            switch (button.position) {
                case WXNavigationItemPositionLeft:
                    eventType = @"clickleftitem";
                    break;
                case WXNavigationItemPositionRight:
                    eventType = @"clickrightitem";
                    break;
                case WXNavigationItemPositionMore:
                    eventType = @"clickmoreitem";
                    break;
                default:
                    break;
            }
            
           [[WXSDKManager bridgeMgr] fireEvent:button.instanceId ref:WX_SDK_ROOT_REF type:eventType params:nil domChanges:nil];
        }
    }
}

- (void)callback:(WXNavigationResultBlock)block code:(NSString *)code data:(NSDictionary *)reposonData
{
    if (block) {
        block(code, reposonData);
    }
}

@end
