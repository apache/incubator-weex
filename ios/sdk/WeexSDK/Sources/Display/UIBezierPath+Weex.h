//
//  UIBezierPath+Weex.h
//  WeexSDK
//
//  Created by yinfeng on 2016/11/17.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface UIBezierPath (Weex)

+ (instancetype)wx_bezierPathWithRoundedRect:(CGRect)rect
                                     topLeft:(CGFloat)topLeftRadius
                                    topRight:(CGFloat)topRightRadius
                                  bottomLeft:(CGFloat)bottomLeftRadius
                                 bottomRight:(CGFloat)bottomRightRadius;

@end
