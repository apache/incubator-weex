//
//  WXImageUtils.h
//  WeexSDK
//
//  Created by xiayun on 16/12/8.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface WXImageUtils : NSObject

+ (UIImage *)toGaussianBluredImage:(UIImage *)originalImage blurRadius:(CGFloat)blurRadius;

@end
