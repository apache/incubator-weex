//
//  WXComponent+GradientColor.h
//  Pods
//
//  Created by bobning on 16/12/23.
//
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "WXComponent.h"

typedef NS_ENUM(NSInteger, WXGradientType) {
    WXGradientTypeToTop,
    WXGradientTypeToBottom,
    WXGradientTypeToLeft,
    WXGradientTypeToRight,
    WXGradientTypeToTopleft,
    WXGradientTypeToBottomright,
};

@interface WXComponent (GradientColor)

- (void)setGradientLayer;

- (WXGradientType)gradientType:(NSString *)value;

- (UIImage *)gradientColorImageFromColors:(NSArray*)colors gradientType:(WXGradientType)gradientType imgSize:(CGSize)imgSize;

@end
