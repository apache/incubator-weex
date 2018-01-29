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
#import <pthread/pthread.h>
#import <os/lock.h>

@interface WXThreadSafeMutableDictionary ()
{
    pthread_mutex_t _safeThreadDictionaryMutex;
    pthread_mutexattr_t _safeThreadDictionaryMutexAttr;
    os_unfair_lock _unfairLock;// this type of lock is not recurisive
}

@property (nonatomic, strong) dispatch_queue_t queue;
@property (nonatomic, strong) NSMutableDictionary* dict;

@end

@implementation WXThreadSafeMutableDictionary

- (instancetype)initCommon
{
    self = [super init];
    if (self) {
        NSString* uuid = [NSString stringWithFormat:@"com.taobao.weex.dictionary_%p", self];
        _queue = dispatch_queue_create([uuid UTF8String], DISPATCH_QUEUE_CONCURRENT);
        pthread_mutexattr_init(&(_safeThreadDictionaryMutexAttr));
        pthread_mutexattr_settype(&(_safeThreadDictionaryMutexAttr), PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&(_safeThreadDictionaryMutex), &(_safeThreadDictionaryMutexAttr));
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            _unfairLock = OS_UNFAIR_LOCK_INIT;
        }
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
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_unfairLock);
            count = [_dict count];
            os_unfair_lock_unlock(&_unfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadDictionaryMutex);
            count = [_dict count];
            pthread_mutex_unlock(&_safeThreadDictionaryMutex);
        }
    }
    return count;
}

- (id)objectForKey:(id)aKey
{
    if (nil == aKey){
        return nil;
    }
    __block id obj;
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_sync(_queue, ^{
            obj = _dict[aKey];
        });
    } else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_unfairLock);
            obj = _dict[aKey];
            os_unfair_lock_unlock(&_unfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadDictionaryMutex);
            obj = _dict[aKey];
            pthread_mutex_unlock(&_safeThreadDictionaryMutex);
        }
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
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_unfairLock);
            enu = [_dict keyEnumerator];
            os_unfair_lock_unlock(&_unfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadDictionaryMutex);
            enu = [_dict keyEnumerator];
            pthread_mutex_unlock(&_safeThreadDictionaryMutex);
        }
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
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_unfairLock);
            _dict[aKey] = anObject;
            os_unfair_lock_unlock(&_unfairLock);
        }else {
            pthread_mutex_lock(&_safeThreadDictionaryMutex);
            _dict[aKey] = anObject;
            pthread_mutex_unlock(&_safeThreadDictionaryMutex);
        }
    }
}

- (void)removeObjectForKey:(id)aKey
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_dict removeObjectForKey:aKey];
        });
    } else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_unfairLock);
            [_dict removeObjectForKey:aKey];
            os_unfair_lock_unlock(&_unfairLock);
        }else {
            pthread_mutex_lock(&_safeThreadDictionaryMutex);
            [_dict removeObjectForKey:aKey];
            pthread_mutex_unlock(&_safeThreadDictionaryMutex);
        }
    }
}

- (void)removeAllObjects
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_dict removeAllObjects];
        });
    }else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_unfairLock);
            [_dict removeAllObjects];
            os_unfair_lock_unlock(&_unfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadDictionaryMutex);
            [_dict removeAllObjects];
            pthread_mutex_unlock(&_safeThreadDictionaryMutex);
        }
    }
}

- (id)copy{
    __block id copyInstance;
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_sync(_queue, ^{
            copyInstance = [_dict copy];
        });
    } else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_unfairLock);
            copyInstance = [_dict copy];
            os_unfair_lock_unlock(&_unfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadDictionaryMutex);
            copyInstance = [_dict copy];
            pthread_mutex_unlock(&_safeThreadDictionaryMutex);
        }
    }
    
    return copyInstance;
}

- (void)dealloc
{
    pthread_mutex_destroy(&_safeThreadDictionaryMutex);
    pthread_mutexattr_destroy(&_safeThreadDictionaryMutexAttr);
}

@end
