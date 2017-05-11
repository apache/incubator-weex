//
//  WXAnimationLayout.h
//  WeexSDK
//
//  Created by LiuHongfeng on 2017/5/11.
//  Copyright © 2017年 taobao. All rights reserved.
//

#import <Foundation/Foundation.h>
<<<<<<< Updated upstream

@interface WXAnimationLayout : NSObject

=======
#import "WXComponent.h"
#import "WXComponent_internal.h"
#import "NSTimer+Weex.h"

@interface WXAnimationLayoutInfo : NSObject

@property (nonatomic, strong) NSString *propertyName;
@property (nonatomic, strong) id fromValue;
@property (nonatomic, strong) id toValue;

@end


@interface WXAnimationLayout : NSObject

@property (nonatomic,strong) NSTimer *updateStyleTimer;
@property (nonatomic,strong) WXComponent *targetComponent;
@property (nonatomic,strong) NSDate *animationStartDate;
@property (nonatomic,strong) WXAnimationLayoutInfo *widthInfo;
@property (nonatomic,strong) WXAnimationLayoutInfo *heightInfo;
@property (nonatomic,assign) double animationDuration;
@property (nonatomic,assign) double animationDelay;
@property (nonatomic,strong) NSDictionary *needUpdateStyles;
@property (nonatomic, weak) WXSDKInstance *weexInstance;

- (void)layoutForAnimation;

>>>>>>> Stashed changes
@end
