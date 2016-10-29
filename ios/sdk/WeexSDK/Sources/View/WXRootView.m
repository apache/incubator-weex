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
    [super setFrame:frame];
    
    if (!CGRectEqualToRect(self.frame, frame) && _instance.onLayout) {
        _instance.onLayout(self);
    }
}

@end
