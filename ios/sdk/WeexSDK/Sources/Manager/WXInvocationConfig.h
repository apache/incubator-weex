//
//  WXRuntimeConfig.h
//  WeexSDK
//
//  Created by zifan.zx on 01/11/2016.
//  Copyright © 2016 taobao. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "WXBridgeMethod.h"

@interface WXInvocationConfig : NSObject

@property (nonatomic, strong) NSString *name;
@property (nonatomic, strong) NSString *clazz;
/**
 *  The methods map
 **/
@property (nonatomic, strong) NSMutableDictionary   *methods;

+ (instancetype)sharedInstance;
- (instancetype)initWithName:(NSString *)name class:(NSString *)clazz;
- (void)registerMethods;
- (void)dispatchMethod:(WXBridgeMethod*)method;
- (NSInvocation*)invocationWithTargetMethod:(id)target method:(WXBridgeMethod*)method;
@end
