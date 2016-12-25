/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>

@interface WXServiceFactory : NSObject

/**
 * @abstract Registers a component for a given name, options and js code
 *
 * @param name The service name to register
 *
 * @param options The service options to register
 *
 * @param code service js code to invoke
 *
 */
+ (NSString *)registerService:(NSString *)name withService:(NSString *)serviceScript withOptions:(NSDictionary *)options;

+ (NSString *)unregisterService:(NSString *)name;

@end
