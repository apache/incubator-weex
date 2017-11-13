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

#import "WXStorageDefaultImpl.h"
#import "WXThreadSafeMutableDictionary.h"
#import "WXThreadSafeMutableArray.h"
#import "WXUtility.h"

static NSString * const WXStorageDirectory            = @"wxstorage";
static NSString * const WXStorageFileName             = @"wxstorage.plist";
static NSString * const WXStorageInfoFileName         = @"wxstorage.info.plist";
static NSString * const WXStorageIndexFileName        = @"wxstorage.index.plist";
static NSUInteger const WXStorageLineLimit            = 1024;
static NSUInteger const WXStorageTotalLimit           = 5 * 1024 * 1024;
static NSString * const WXStorageThreadName           = @"com.taobao.weex.storage";
static NSString * const WXStorageNullValue            = @"#{eulaVlluNegarotSXW}";

@implementation WXStorageDefaultImpl

- (dispatch_queue_t)targetExecuteQueue
{
    return [WXStorageDefaultImpl storageQueue];
}

- (NSUInteger)length
{
    return [[WXStorageDefaultImpl memory] count];
}

- (NSArray *)getAllKeys
{
    return [WXStorageDefaultImpl memory].allKeys;
}

- (void)getItem:(NSString *)key callback:(WXModuleCallback)callback
{
    NSString *value = [self.memory objectForKey:key];
    if ([WXStorageNullValue isEqualToString:value]) {
        value = [[WXUtility globalCache] objectForKey:key];
        if (!value) {
            NSString *filePath = [WXStorageDefaultImpl filePathForKey:key];
            NSString *contents = [WXUtility stringWithContentsOfFile:filePath];
            if (contents) {
                [[WXUtility globalCache] setObject:contents forKey:key cost:contents.length];
                value = contents;
            }
        }
    }
    if (!value) {
        [self removeItem:key];
        if (callback) {
            callback(@{@"result":@"failed",@"data":@"undefined"});
        }
        return;
    }
    [self updateTimestampForKey:key];
    [self updateIndexForKey:key];
    if (callback) {
        callback(@{@"result":@"success",@"data":value});
    }
}

- (BOOL)removeItem:(NSString *)key {
    if ([WXStorageNullValue isEqualToString:self.memory[key]]) {
        [self.memory removeObjectForKey:key];
        NSDictionary *dict = [self.memory copy];
        [self write:dict toFilePath:[WXStorageDefaultImpl filePath]];
        dispatch_async([WXStorageDefaultImpl storageQueue], ^{
            NSString *filePath = [WXStorageDefaultImpl filePathForKey:key];
            [[NSFileManager defaultManager] removeItemAtPath:filePath error:nil];
            [[WXUtility globalCache] removeObjectForKey:key];
        });
    } else if (self.memory[key]) {
        [self.memory removeObjectForKey:key];
        NSDictionary *dict = [self.memory copy];
        [self write:dict toFilePath:[WXStorageDefaultImpl filePath]];
    } else {
        return NO;
    }
    [self removeInfoForKey:key];
    [self removeIndexForKey:key];
    return YES;
}

#pragma mark - Utils
- (void)setObject:(NSString *)obj forKey:(NSString *)key persistent:(BOOL)persistent callback:(WXModuleCallback)callback {
    NSString *filePath = [WXStorageDefaultImpl filePathForKey:key];
    if (obj.length <= WXStorageLineLimit) {
        if ([WXStorageNullValue isEqualToString:self.memory[key]]) {
            [[WXUtility globalCache] removeObjectForKey:key];
            [[NSFileManager defaultManager] removeItemAtPath:filePath error:nil];
        }
        self.memory[key] = obj;
        NSDictionary *dict = [self.memory copy];
        [self write:dict toFilePath:[WXStorageDefaultImpl filePath]];
        [self setInfo:@{@"persistent":@(persistent),@"size":@(obj.length)} ForKey:key];
        [self updateIndexForKey:key];
        [self checkStorageLimit];
        if (callback) {
            callback(@{@"result":@"success"});
        }
        return;
    }
    
    [[WXUtility globalCache] setObject:obj forKey:key cost:obj.length];
    
    if (![WXStorageNullValue isEqualToString:self.memory[key]]) {
        self.memory[key] = WXStorageNullValue;
        NSDictionary *dict = [self.memory copy];
        [self write:dict toFilePath:[WXStorageDefaultImpl filePath]];
    }
    
    dispatch_async([WXStorageDefaultImpl storageQueue], ^{
        [obj writeToFile:filePath atomically:YES encoding:NSUTF8StringEncoding error:NULL];
    });
    
    [self setInfo:@{@"persistent":@(persistent),@"size":@(obj.length)} ForKey:key];
    [self updateIndexForKey:key];
    
    [self checkStorageLimit];
    if (callback) {
        callback(@{@"result":@"success"});
    }
}

- (void)checkStorageLimit {
    NSInteger size = [self totalSize] - WXStorageTotalLimit;
    if (size > 0) {
        [self removeItemsBySize:size];
    }
}

- (void)removeItemsBySize:(NSInteger)size {
    NSArray *indexs = [[self indexs] copy];
    if (size < 0 || indexs.count == 0) {
        return;
    }
    
    NSMutableArray *removedKeys = [NSMutableArray array];
    for (NSInteger i = 0; i < indexs.count; i++) {
        NSString *key = indexs[i];
        NSDictionary *info = [self getInfoForKey:key];
        
        // persistent data, can't be removed
        if ([info[@"persistent"] boolValue]) {
            continue;
        }
        
        [removedKeys addObject:key];
        size -= [info[@"size"] integerValue];
        
        if (size < 0) {
            break;
        }
    }
    
    // actually remove data
    for (NSString *key in removedKeys) {
        [self removeItem:key];
    }
}

- (void)write:(NSDictionary *)dict toFilePath:(NSString *)filePath{
    [dict writeToFile:filePath atomically:YES];
}

+ (NSString *)filePathForKey:(NSString *)key
{
    NSString *safeFileName = [WXUtility md5:key];
    
    return [[WXStorageDefaultImpl directory] stringByAppendingPathComponent:safeFileName];
}

+ (void)setupDirectory{
    BOOL isDirectory = NO;
    BOOL fileExists = [[NSFileManager defaultManager] fileExistsAtPath:[WXStorageDefaultImpl directory] isDirectory:&isDirectory];
    if (!isDirectory && !fileExists) {
        [[NSFileManager defaultManager] createDirectoryAtPath:[WXStorageDefaultImpl directory]
                                  withIntermediateDirectories:YES
                                                   attributes:nil
                                                        error:NULL];
    }
}

+ (NSString *)directory {
    static NSString *storageDirectory = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        storageDirectory = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
        storageDirectory = [storageDirectory stringByAppendingPathComponent:WXStorageDirectory];
    });
    return storageDirectory;
}

+ (NSString *)filePath {
    static NSString *storageFilePath = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        storageFilePath = [[WXStorageDefaultImpl directory] stringByAppendingPathComponent:WXStorageFileName];
    });
    return storageFilePath;
}

+ (NSString *)infoFilePath {
    static NSString *infoFilePath = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        infoFilePath = [[WXStorageDefaultImpl directory] stringByAppendingPathComponent:WXStorageInfoFileName];
    });
    return infoFilePath;
}

+ (NSString *)indexFilePath {
    static NSString *indexFilePath = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        indexFilePath = [[WXStorageDefaultImpl directory] stringByAppendingPathComponent:WXStorageIndexFileName];
    });
    return indexFilePath;
}

+ (dispatch_queue_t)storageQueue {
    static dispatch_queue_t storageQueue;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        storageQueue = dispatch_queue_create([WXStorageThreadName cStringUsingEncoding:NSASCIIStringEncoding], DISPATCH_QUEUE_SERIAL);
    });
    return storageQueue;
}

+ (WXThreadSafeMutableDictionary<NSString *, NSString *> *)memory {
    static WXThreadSafeMutableDictionary<NSString *,NSString *> *memory;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [WXStorageDefaultImpl setupDirectory];
        
        if ([[NSFileManager defaultManager] fileExistsAtPath:[WXStorageDefaultImpl filePath]]) {
            NSDictionary *contents = [NSDictionary dictionaryWithContentsOfFile:[WXStorageDefaultImpl filePath]];
            if (contents) {
                memory = [[WXThreadSafeMutableDictionary alloc] initWithDictionary:contents];
            }
        }
        if (!memory) {
            memory = [WXThreadSafeMutableDictionary new];
        }
        //        [[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationDidReceiveMemoryWarningNotification object:nil queue:nil usingBlock:^(__unused NSNotification *note) {
        //            [memory removeAllObjects];
        //        }];
    });
    return memory;
}

+ (WXThreadSafeMutableDictionary<NSString *, NSDictionary *> *)info {
    static WXThreadSafeMutableDictionary<NSString *,NSDictionary *> *info;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [WXStorageDefaultImpl setupDirectory];
        
        if ([[NSFileManager defaultManager] fileExistsAtPath:[WXStorageDefaultImpl infoFilePath]]) {
            NSDictionary *contents = [NSDictionary dictionaryWithContentsOfFile:[WXStorageDefaultImpl infoFilePath]];
            if (contents) {
                info = [[WXThreadSafeMutableDictionary alloc] initWithDictionary:contents];
            }
        }
        if (!info) {
            info = [WXThreadSafeMutableDictionary new];
        }
    });
    return info;
}

+ (WXThreadSafeMutableArray *)indexs {
    static WXThreadSafeMutableArray *indexs;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [WXStorageDefaultImpl setupDirectory];
        
        if ([[NSFileManager defaultManager] fileExistsAtPath:[WXStorageDefaultImpl indexFilePath]]) {
            NSArray *contents = [NSArray arrayWithContentsOfFile:[WXStorageDefaultImpl indexFilePath]];
            if (contents) {
                indexs = [[WXThreadSafeMutableArray alloc] initWithArray:contents];
            }
        }
        if (!indexs) {
            indexs = [WXThreadSafeMutableArray new];
        }
    });
    return indexs;
}

- (WXThreadSafeMutableDictionary<NSString *, NSString *> *)memory {
    return [WXStorageDefaultImpl memory];
}

- (WXThreadSafeMutableDictionary<NSString *, NSDictionary *> *)info {
    return [WXStorageDefaultImpl info];
}

- (WXThreadSafeMutableArray *)indexs {
    return [WXStorageDefaultImpl indexs];
}

#pragma mark
#pragma mark - Storage Info method
- (NSDictionary *)getInfoForKey:(NSString *)key {
    NSDictionary *info = [[self info] objectForKey:key];
    if (!info) {
        return nil;
    }
    return info;
}

- (void)setInfo:(NSDictionary *)info ForKey:(NSString *)key {
    NSAssert(info, @"info must not be nil");
    
    // save info for key
    NSMutableDictionary *newInfo = [NSMutableDictionary dictionaryWithDictionary:info];
    NSTimeInterval interval = [[NSDate date] timeIntervalSince1970];
    [newInfo setObject:@(interval) forKey:@"ts"];
    
    [[self info] setObject:[newInfo copy] forKey:key];
    NSDictionary *dict = [[self info] copy];
    [self write:dict toFilePath:[WXStorageDefaultImpl infoFilePath]];
}

- (void)removeInfoForKey:(NSString *)key {
    [[self info] removeObjectForKey:key];
    NSDictionary *dict = [[self info] copy];
    [self write:dict toFilePath:[WXStorageDefaultImpl infoFilePath]];
}

- (void)updateTimestampForKey:(NSString *)key {
    NSTimeInterval interval = [[NSDate date] timeIntervalSince1970];
    NSDictionary *info = [[self info] objectForKey:key];
    if (!info) {
        info = @{@"persistent":@(NO),@"size":@(0),@"ts":@(interval)};
    } else {
        NSMutableDictionary *newInfo = [NSMutableDictionary dictionaryWithDictionary:info];
        [newInfo setObject:@(interval) forKey:@"ts"];
        info = [newInfo copy];
    }
    
    [[self info] setObject:info forKey:key];
    NSDictionary *dict = [[self info] copy];
    [self write:dict toFilePath:[WXStorageDefaultImpl infoFilePath]];
}

- (NSInteger)totalSize {
    NSInteger totalSize = 0;
    for (NSDictionary *info in [self info].allValues) {
        totalSize += (info[@"size"] ? [info[@"size"] integerValue] : 0);
    }
    return totalSize;
}

#pragma mark
#pragma mark - Storage Index method
- (void)updateIndexForKey:(NSString *)key {
    [[self indexs] removeObject:key];
    [[self indexs] addObject:key];
    [self write:[[self indexs] copy] toFilePath:[WXStorageDefaultImpl indexFilePath]];
}

- (void)removeIndexForKey:(NSString *)key {
    [[self indexs] removeObject:key];
    [self write:[[self indexs] copy] toFilePath:[WXStorageDefaultImpl indexFilePath]];
}

@end
