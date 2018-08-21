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

#import "WXSDKManager.h"
#import "WXThreadSafeMutableDictionary.h"

@interface WXSDKManager ()

@property (nonatomic, strong) WXBridgeManager *bridgeMgr;

@property (nonatomic, strong) WXThreadSafeMutableDictionary *instanceDict;

@end

@implementation WXSDKManager

static WXSDKManager *_sharedInstance = nil;

+ (WXSDKManager *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (!_sharedInstance) {
            _sharedInstance = [[self alloc] init];
            _sharedInstance.instanceDict = [[WXThreadSafeMutableDictionary alloc] init];
        }
    });
    return _sharedInstance;
}

- (instancetype)init
{
    if (self = [super init]){
        
    }
    return self;
}

+ (WXBridgeManager *)bridgeMgr
{
    WXBridgeManager *bridgeMgr = [self sharedInstance].bridgeMgr;
    if (!bridgeMgr) {
        bridgeMgr = [[WXBridgeManager alloc] init];
        [self sharedInstance].bridgeMgr = bridgeMgr;
    }
    return bridgeMgr;
}

+ (id)instanceForID:(NSString *)identifier
{
    return [[self sharedInstance].instanceDict objectForKey:identifier];
}

+ (void)storeInstance:(WXSDKInstance *)instance forID:(NSString *)identifier
{
    [[self sharedInstance].instanceDict setObject:instance forKey:identifier];
}

+ (void)removeInstanceforID:(NSString *)identifier
{
    if (identifier) {
        [[self sharedInstance].instanceDict removeObjectForKey:identifier];
    }
}

+ (void)unload
{
    for (NSString *instanceID in [self sharedInstance].instanceDict) {
        WXSDKInstance *instance = [[self sharedInstance].instanceDict objectForKey:instanceID];
        [instance destroyInstance];
    }
    [[self sharedInstance].bridgeMgr unload];
    [self sharedInstance].bridgeMgr = nil;
}

+ (WXModuleManager *)moduleMgr
{
    return nil;
}

@end
