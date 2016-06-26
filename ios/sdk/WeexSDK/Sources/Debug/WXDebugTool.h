/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>
#import "WXModuleProtocol.h"

@interface WXDebugTool : NSObject<WXModuleProtocol>

//+ (void)showFPS;

+ (void)setDebug:(BOOL)isDebug;

+ (BOOL)isDebug;

+ (void)setReplacedBundleJS:(NSURL*)url;

+ (NSString*)getReplacedBundleJS;

+ (void)setReplacedJSFramework:(NSURL*)url;

+ (NSString*)getReplacedJSFramework;

/**
 *  launch weex inspector
 *  @param url  : ws://ip:9000/device, ip is your computer device ip or localhost
 **/
+ (void)launchInspectorWithSocketUrl:(NSURL *)url;

+ (void)launchDebugWithSocketUrl:(NSString *)url;

@end
