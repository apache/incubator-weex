//
//  WXExtendCallNativeManager.h
//  WeexSDK
//
//  Created by 齐山 on 2017/7/27.
//  Copyright © 2017年 taobao. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface WXExtendCallNativeManager : NSObject

+ (id)sendExtendCallNativeEvent:(NSDictionary *)parameters;
+ (void)registerExtendCallNative:(NSString *)name withClass:(Class)clazz;
+ (Class)classWithComponentName:(NSString *)name;
@end
