//
//  WXConfigCenterProtocol.h
//  WeexSDK
//
//  Created by zifan.zx on 2017/5/4.
//  Copyright © 2017年 taobao. All rights reserved.
//

@protocol WXConfigCenterProtocol <NSObject>

/**
 get config from config center handler
 @param key the key for config
 @param defaultValue default value for key if the key is not exist.
 @param isDefault whether the value is default value
 @return the value for config key
 */
- (id)configForKey:(NSString*)key defaultValue:(id)defaultValue isDefault:(BOOL*)isDefault;

@end
