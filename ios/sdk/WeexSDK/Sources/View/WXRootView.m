//
//  WXRootView.m
//  WeexSDK
//
//  Created by yinfeng on 2016/10/29.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import "WXRootView.h"
#import "WXSDKInstance.h"

@implementation WXRootView

- (void)setFrame:(CGRect)frame
{
    BOOL shouldNotifyLayout;
    if (_instance.onLayout && !CGRectEqualToRect(self.frame, frame)) {
        shouldNotifyLayout = YES;
    }
    
    [super setFrame:frame];
    
    if (shouldNotifyLayout) {
        _instance.onLayout(self);
    }
}

@end
