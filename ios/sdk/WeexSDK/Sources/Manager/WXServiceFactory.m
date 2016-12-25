/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXServiceFactory.h"
#import "WXAssert.h"
#import "WXLog.h"

#import <objc/runtime.h>

@implementation WXServiceFactory
{

}

#pragma mark Life Cycle

+ (instancetype)sharedInstance {
    static id _sharedInstance = nil;
    static dispatch_once_t oncePredicate;
    dispatch_once(&oncePredicate, ^{
        _sharedInstance = [[self alloc] init];
    });
    return _sharedInstance;
}

- (instancetype)init
{
    if(self = [super init]){

    }
    return self;
}

#pragma mark Public
+ (NSString *)registerService:(NSString *)name withService:(NSString *)serviceScript withOptions:(NSDictionary *)options
{
    NSDictionary *dic = [[self sharedInstance] registerService:name withService:serviceScript withOptions:options];
    return [dic objectForKey:@"script"];
}


+ (NSString *)unregisterService:(NSString *)name
{
    return [[self sharedInstance] getUnregisterServiceScript:name];
}


#pragma mark Private

- (NSDictionary *)registerService:(NSString *)name withService:(NSString *)serviceScript withOptions:(NSDictionary *)options
{
    WXAssert(name && options && serviceScript, @"name or options or code must not be nil for registering service.");
    
    NSDictionary *serverConfig = @{
                                   @"name": name,
                                   @"options": options,
                                   @"script": [self getServiceScript:name withService:serviceScript withOptions:options]
                                   };
    
    return serverConfig;
}

- (NSString *) getServiceScript:(NSString *)name withService:(NSString *)serviceScript withOptions:(NSDictionary *)options {
    NSMutableDictionary *optionDic = [NSMutableDictionary dictionaryWithDictionary:options];
    [optionDic setObject:name forKey:@"serviceName"];
    NSDictionary *funcDic = @{
                              @"register": @"global.registerService",
                              @"unregister": @"global.unregisterService"
                              };
    NSString *script = [NSString stringWithFormat:@";(fucntion(service, options){ %@ })(%@, %@);"
                        , serviceScript, funcDic, optionDic];
    return script;
}

- (NSString *) getUnregisterServiceScript: (NSString *) name
{
    NSString *script = [NSString stringWithFormat:@";global.unregisterService(%@);", name];
    return script;
}

@end
