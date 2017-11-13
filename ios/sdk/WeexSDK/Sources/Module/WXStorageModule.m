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

#import "WXStorageModule.h"
#import "WXStorageProtocol.h"
#import "WXHandlerFactory.h"
#import "WXUtility.h"

@implementation WXStorageModule

@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(length:))
WX_EXPORT_METHOD(@selector(getItem:callback:))
WX_EXPORT_METHOD(@selector(setItem:value:callback:))
WX_EXPORT_METHOD(@selector(setItemPersistent:value:callback:))
WX_EXPORT_METHOD(@selector(getAllKeys:))
WX_EXPORT_METHOD(@selector(removeItem:callback:))

- (id<WXStorageProtocol>)storage
{
    id<WXStorageProtocol> storage = [WXHandlerFactory handlerForProtocol:@protocol(WXStorageProtocol)];
    return storage;
}

#pragma mark - Export

- (dispatch_queue_t)targetExecuteQueue {
    id<WXStorageProtocol> storage = [self storage];
    return [storage targetExecuteQueue];
}

- (void)length:(WXModuleCallback)callback
{
    if (callback) {
        id<WXStorageProtocol> storage = [self storage];
        callback(@{@"result":@"success",@"data":@([storage length])});
    }
}

- (void)getAllKeys:(WXModuleCallback)callback
{
    if (callback) {
        id<WXStorageProtocol> storage = [self storage];
        callback(@{@"result":@"success",@"data":[storage getAllKeys]});
    }
}

- (void)getItem:(NSString *)key callback:(WXModuleCallback)callback
{
    if ([self checkInput:key]) {
        if (callback) {
            callback(@{@"result":@"failed",@"data":@"key must a string or number!"}); // forgive my english
        }
        return;
    }
    
    if ([key isKindOfClass:[NSNumber class]]) {
        key = [((NSNumber *)key) stringValue]; // oh no!
    }
    
    if ([WXUtility isBlankString:key]) {
        if (callback) {
            callback(@{@"result":@"failed",@"data":@"invalid_param"});
        }
        return ;
    }
    
    id<WXStorageProtocol> storage = [self storage];
    [storage getItem:key callback:callback];
}

- (void)setItem:(NSString *)key value:(NSString *)value callback:(WXModuleCallback)callback
{
    if ([self checkInput:key]) {
        if (callback) {
            callback(@{@"result":@"failed",@"data":@"key must a string or number!"});
        }
        return;
    }
    if ([self checkInput:value]) {
        if (callback) {
            callback(@{@"result":@"failed",@"data":@"value must a string or number!"});
        }
        return;
    }
    
    if ([key isKindOfClass:[NSNumber class]]) {
        key = [((NSNumber *)key) stringValue];
    }
    
    if ([value isKindOfClass:[NSNumber class]]) {
        value = [((NSNumber *)value) stringValue];
    }
    
    if ([WXUtility isBlankString:key]) {
        if (callback) {
            callback(@{@"result":@"failed",@"data":@"invalid_param"});
        }
        return ;
    }
    id<WXStorageProtocol> storage = [self storage];
    [storage setObject:value forKey:key persistent:NO callback:callback];
}

- (void)setItemPersistent:(NSString *)key value:(NSString *)value callback:(WXModuleCallback)callback
{
    if ([self checkInput:key]) {
        if (callback) {
            callback(@{@"result":@"failed",@"data":@"key must a string or number!"});
        }
        return;
    }
    if ([self checkInput:value]) {
        if (callback) {
            callback(@{@"result":@"failed",@"data":@"value must a string or number!"});
        }
        return;
    }
    
    if ([key isKindOfClass:[NSNumber class]]) {
        key = [((NSNumber *)key) stringValue];
    }
    
    if ([value isKindOfClass:[NSNumber class]]) {
        value = [((NSNumber *)value) stringValue];
    }
    
    if ([WXUtility isBlankString:key]) {
        if (callback) {
            callback(@{@"result":@"failed",@"data":@"invalid_param"});
        }
        return ;
    }
    id<WXStorageProtocol> storage = [self storage];
    [storage setObject:value forKey:key persistent:YES callback:callback];
}

- (void)removeItem:(NSString *)key callback:(WXModuleCallback)callback
{
    if ([self checkInput:key]) {
        if (callback) {
            callback(@{@"result":@"failed",@"data":@"key must a string or number!"});
        }
        return;
    }
    
    if ([key isKindOfClass:[NSNumber class]]) {
        key = [((NSNumber *)key) stringValue];
    }
    
    if ([WXUtility isBlankString:key]) {
        if (callback) {
            callback(@{@"result":@"failed",@"data":@"invalid_param"});
        }
        return ;
    }
    id<WXStorageProtocol> storage = [self storage];
    BOOL removed = [storage removeItem:key];
    if (removed) {
        if (callback) {
            callback(@{@"result":@"success"});
        }
    } else {
        if (callback) {
            callback(@{@"result":@"failed"});
        }
    }
}

- (BOOL)checkInput:(id)input{
    return !([input isKindOfClass:[NSString class]] || [input isKindOfClass:[NSNumber class]]);
}
@end

