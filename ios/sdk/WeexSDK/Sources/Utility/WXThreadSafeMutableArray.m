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

#import "WXThreadSafeMutableArray.h"
#import "WXUtility.h"
#import <os/lock.h>
#import <pthread/pthread.h>

@interface WXThreadSafeMutableArray () {
    pthread_mutex_t _safeThreadArrayMutex;
    pthread_mutexattr_t _safeThreadArrayMutexAttr;
    os_unfair_lock _osUnfairLock;
}

@property (nonatomic, strong) dispatch_queue_t queue;
@property (nonatomic, strong) NSMutableArray* array;

@end

@implementation WXThreadSafeMutableArray

- (instancetype)initCommon
{
    self = [super init];
    if (self) {
        NSString* uuid = [NSString stringWithFormat:@"com.taobao.weex.array_%p", self];
        _queue = dispatch_queue_create([uuid UTF8String], DISPATCH_QUEUE_CONCURRENT);
        pthread_mutexattr_init(&(_safeThreadArrayMutexAttr));
        pthread_mutexattr_settype(&(_safeThreadArrayMutexAttr), PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&(_safeThreadArrayMutex), &(_safeThreadArrayMutexAttr));
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            _osUnfairLock = OS_UNFAIR_LOCK_INIT;
        }
    }
    return self;
}

- (instancetype)init
{
    self = [self initCommon];
    if (self) {
        _array = [NSMutableArray array];
    }
    return self;
}

- (instancetype)initWithCapacity:(NSUInteger)numItems
{
    self = [self initCommon];
    if (self) {
        _array = [NSMutableArray arrayWithCapacity:numItems];
    }
    return self;
}

- (NSArray *)initWithContentsOfFile:(NSString *)path
{
    self = [self initCommon];
    if (self) {
        _array = [NSMutableArray arrayWithContentsOfFile:path];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder
{
    self = [self initCommon];
    if (self) {
        _array = [[NSMutableArray alloc] initWithCoder:aDecoder];
    }
    return self;
}

- (instancetype)initWithObjects:(const id [])objects count:(NSUInteger)cnt
{
    self = [self initCommon];
    if (self) {
        _array = [NSMutableArray array];
        for (NSUInteger i = 0; i < cnt; ++i) {
            [_array addObject:objects[i]];
        }
    }
    return self;
}

- (NSUInteger)count
{
    __block NSUInteger count;
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_sync(_queue, ^{
            count = _array.count;
        });
    } else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_osUnfairLock);
            count = [_array count];
            os_unfair_lock_unlock(&_osUnfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadArrayMutex);
            count = [_array count];
            pthread_mutex_unlock(&_safeThreadArrayMutex);
        }
    }
    return count;
}

- (id)objectAtIndex:(NSUInteger)index
{
    __block id obj;
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_sync(_queue, ^{
            obj = _array[index];
        });
    } else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_osUnfairLock);
            obj = _array[index];
            os_unfair_lock_unlock(&_osUnfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadArrayMutex);
            obj = _array[index];
            pthread_mutex_unlock(&_safeThreadArrayMutex);
        }
    }
    return obj;
}

- (NSEnumerator *)keyEnumerator
{
    __block NSEnumerator *enu;
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_sync(_queue, ^{
            enu = [_array objectEnumerator];
        });
    } else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_osUnfairLock);
            enu = [_array objectEnumerator];
            os_unfair_lock_unlock(&_osUnfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadArrayMutex);
            enu = [_array objectEnumerator];
            pthread_mutex_unlock(&_safeThreadArrayMutex);
        }
    }
    return enu;
}

- (void)insertObject:(id)anObject atIndex:(NSUInteger)index
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_array insertObject:anObject atIndex:index];
        });
    } else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_osUnfairLock);
            [_array insertObject:anObject atIndex:index];
            os_unfair_lock_unlock(&_osUnfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadArrayMutex);
            [_array insertObject:anObject atIndex:index];
            pthread_mutex_unlock(&_safeThreadArrayMutex);
        }
    }
}

- (void)addObject:(id)anObject;
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_array addObject:anObject];
        });
    } else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_osUnfairLock);
            [_array addObject:anObject];
            os_unfair_lock_unlock(&_osUnfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadArrayMutex);
            [_array addObject:anObject];
            pthread_mutex_unlock(&_safeThreadArrayMutex);
        }
    }
}

- (void)removeObjectAtIndex:(NSUInteger)index
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_array removeObjectAtIndex:index];
        });
    } else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_osUnfairLock);
            [_array removeObjectAtIndex:index];
            os_unfair_lock_unlock(&_osUnfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadArrayMutex);
            [_array removeObjectAtIndex:index];
            pthread_mutex_unlock(&_safeThreadArrayMutex);
        }
    }
}

- (void)removeLastObject
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_array removeLastObject];
        });
    } else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_osUnfairLock);
            [_array removeLastObject];
            os_unfair_lock_unlock(&_osUnfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadArrayMutex);
            [_array removeLastObject];
            pthread_mutex_unlock(&_safeThreadArrayMutex);
        }
    }
}

- (void)replaceObjectAtIndex:(NSUInteger)index withObject:(id)anObject
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_array replaceObjectAtIndex:index withObject:anObject];
        });
    } else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_osUnfairLock);
            [_array replaceObjectAtIndex:index withObject:anObject];
            os_unfair_lock_unlock(&_osUnfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadArrayMutex);
            [_array replaceObjectAtIndex:index withObject:anObject];
            pthread_mutex_unlock(&_safeThreadArrayMutex);
        }
    }
}

- (NSUInteger)indexOfObject:(id)anObject
{
    __block NSUInteger index = NSNotFound;
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_sync(_queue, ^{
            for (int i = 0; i < [_array count]; i ++) {
                if ([_array objectAtIndex:i] == anObject) {
                    index = i;
                    break;
                }
            }
        });
    } else {
        if (WX_SYS_VERSION_GREATER_THAN(@"10.0")) {
            os_unfair_lock_lock(&_osUnfairLock);
            index = [_array indexOfObject:anObject];
            os_unfair_lock_unlock(&_osUnfairLock);
        } else {
            pthread_mutex_lock(&_safeThreadArrayMutex);
            index = [_array indexOfObject:anObject];
            pthread_mutex_unlock(&_safeThreadArrayMutex);
        }
    }
    
    return index;
}

- (void)dealloc
{
    if (_queue) {
        _queue = NULL;
    }
    pthread_mutex_destroy(&_safeThreadArrayMutex);
    pthread_mutexattr_destroy(&_safeThreadArrayMutexAttr);
}

@end
