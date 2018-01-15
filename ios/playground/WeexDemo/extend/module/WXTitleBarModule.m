//
//  WXTitleBarModule.m
//  WeexDemo
//
//  Created by zifan.zx on 2018/1/15.
//  Copyright © 2018年 taobao. All rights reserved.
//

#import "WXTitleBarModule.h"

@implementation WXTitleBarModule
@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(setTitle:))
WX_EXPORT_METHOD(@selector(showTitleBar:))

- (void)setTitle:(NSString*)title
{
    if (title) {
        [weexInstance.viewController.navigationController setTitle:title];
    }
}

- (void)setStytle:(NSDictionary*)styles
{
    if (styles[@"backgroundColor"]) {
        [weexInstance.viewController.navigationController.navigationBar setBackgroundColor:[WXConvert UIColor:styles[@"backgroundColor"]]];
    }
    if (styles[@"foregroundColor"]) {
        weexInstance.viewController.navigationController.navigationItem.leftBarButtonItem.tintColor = [WXConvert UIColor:styles[@"foregroundColor"]];
        weexInstance.viewController.navigationController.navigationItem.rightBarButtonItem.tintColor = [WXConvert UIColor:styles[@"foregroundColor"]];
        [weexInstance.viewController.navigationController.navigationBar setTitleTextAttributes:@{NSForegroundColorAttributeName:[WXConvert UIColor:styles[@"foregroundColor"]]}];
    }
}

- (void)showTitleBar:(BOOL)show
{
    [weexInstance.viewController.navigationController setNavigationBarHidden:show];
}

@end
