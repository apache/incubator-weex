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

#import "WXThreadSafeMutableDictionary.h"
#import "WXUtility.h"

@interface WXThreadSafeMutableDictionary ()

@property (nonatomic, strong) dispatch_queue_t queue;
@property (atomic, strong) NSRecursiveLock * recursiveDicLock;
@property (nonatomic, strong) NSMutableDictionary* dict;

@end

@implementation WXThreadSafeMutableDictionary

- (instancetype)initCommon
{
    self = [super init];
    if (self) {
        NSString* uuid = [NSString stringWithFormat:@"com.taobao.weex.dictionary_%p", self];
        _queue = dispatch_queue_create([uuid UTF8String], DISPATCH_QUEUE_CONCURRENT);
        _recursiveDicLock = [[NSRecursiveLock alloc] init];
    }
    return self;
}

- (instancetype)init
{
    self = [self initCommon];
    if (self) {
        _dict = [NSMutableDictionary dictionary];
    }
    return self;
}

- (instancetype)initWithCapacity:(NSUInteger)numItems
{
    self = [self initCommon];
    if (self) {
        _dict = [NSMutableDictionary dictionaryWithCapacity:numItems];
    }
    return self;
}

- (NSDictionary *)initWithContentsOfFile:(NSString *)path
{
    self = [self initCommon];
    if (self) {
        _dict = [NSMutableDictionary dictionaryWithContentsOfFile:path];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder
{
    self = [self initCommon];
    if (self) {
        _dict = [[NSMutableDictionary alloc] initWithCoder:aDecoder];
    }
    return self;
}

- (instancetype)initWithObjects:(const id [])objects forKeys:(const id<NSCopying> [])keys count:(NSUInteger)cnt
{
    self = [self initCommon];
    if (self) {
        _dict = [NSMutableDictionary dictionary];
        for (NSUInteger i = 0; i < cnt; ++i) {
            _dict[keys[i]] = objects[i];
        }
        
    }
    return self;
}

- (NSUInteger)count
{
    __block NSUInteger count;
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_sync(_queue, ^{
            count = _dict.count;
        });
    } else {
        [_recursiveDicLock lock];
        count = [_dict count];
        [_recursiveDicLock unlock];
    }
    return count;
}

- (id)objectForKey:(id)aKey
{
    __block id obj;
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_sync(_queue, ^{
            obj = _dict[aKey];
        });
    } else {
        [_recursiveDicLock lock];
        obj = _dict[aKey];
        [_recursiveDicLock unlock];
    }
    return obj;
}

- (NSEnumerator *)keyEnumerator
{
    __block NSEnumerator *enu;
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_sync(_queue, ^{
            enu = [_dict keyEnumerator];
        });
    } else {
        [_recursiveDicLock lock];
        enu = [_dict keyEnumerator];
        [_recursiveDicLock unlock];
    }
    return enu;
}

- (void)setObject:(id)anObject forKey:(id<NSCopying>)aKey
{
    aKey = [aKey copyWithZone:NULL];
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
        _dict[aKey] = anObject;
        });
    } else {
        [_recursiveDicLock lock];
        _dict[aKey] = anObject;
        [_recursiveDicLock unlock];
    }
}

- (void)removeObjectForKey:(id)aKey
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_dict removeObjectForKey:aKey];
        });
    } else {
        [_recursiveDicLock lock];
        [_dict removeObjectForKey:aKey];
        [_recursiveDicLock unlock];
    }
}

- (void)removeAllObjects
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_dict removeAllObjects];
        });
    }else {
        [_recursiveDicLock lock];
        [_dict removeAllObjects];
        [_recursiveDicLock unlock];
    }
}

- (id)copy{
    __block id copyInstance;
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_sync(_queue, ^{
            copyInstance = [_dict copy];
        });
    } else {
        [_recursiveDicLock lock];
        copyInstance = [_dict copy];
        [_recursiveDicLock unlock];
    }
    
    return copyInstance;
}

@end
