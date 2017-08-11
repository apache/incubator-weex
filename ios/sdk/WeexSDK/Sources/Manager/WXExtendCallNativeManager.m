/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#import "WXExtendCallNativeManager.h"
#import "WXExtendCallNativeProtocol.h"
#import <objc/runtime.h>

@interface WXExtendCallNativeManager ()

@property (nonatomic, strong) NSMutableDictionary *names;

@end

@implementation WXExtendCallNativeManager
{
    NSLock *_configLock;
}
+ (instancetype) sharedInstance{
    
    static WXExtendCallNativeManager *instance = nil;
    static dispatch_once_t once;
    
    dispatch_once(&once, ^{
        instance = [[WXExtendCallNativeManager alloc] initPrivate];
    });
    
    return instance;
}

- (instancetype) initPrivate{
    self = [super init];
    if(self){
        _configLock = [[NSLock alloc] init];
        self.names = [NSMutableDictionary new];
    }
    
    return self;
}

+ (void)registerExtendCallNative:(NSString *)name withClass:(Class)clazz
{
    [[WXExtendCallNativeManager sharedInstance].names setObject:NSStringFromClass(clazz) forKey:name];
}

+ (Class)classWithExtendCallNativeName:(NSString *)name
{
    NSString *clazz = [[WXExtendCallNativeManager sharedInstance].names objectForKey:name];
    return NSClassFromString(clazz);
}

+(id)sendExtendCallNativeEvent:(NSDictionary *)parameters
{
    if(parameters[@"className"]){
        Class<WXExtendCallNativeProtocol> receiveClass = [self classWithExtendCallNativeName:parameters[@"className"]];
        if(receiveClass && class_conformsToProtocol(receiveClass,@protocol(WXExtendCallNativeProtocol))){
            BOOL receivedItem = [receiveClass checkParameters:parameters];
            if(receivedItem){
                return [receiveClass excuteCallNative:parameters];
            }
        }
    }
    return @(-1);
}
@end
