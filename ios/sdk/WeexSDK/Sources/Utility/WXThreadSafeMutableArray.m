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

@interface WXThreadSafeMutableArray ()

@property (nonatomic, strong) dispatch_queue_t queue;
@property (nonatomic, strong) NSMutableArray* array;
@property (atomic, strong) NSRecursiveLock * arrayRecursiveLock;

@end

@implementation WXThreadSafeMutableArray

- (instancetype)initCommon
{
    self = [super init];
    if (self) {
        NSString* uuid = [NSString stringWithFormat:@"com.taobao.weex.array_%p", self];
        _queue = dispatch_queue_create([uuid UTF8String], DISPATCH_QUEUE_CONCURRENT);
        _arrayRecursiveLock = [[NSRecursiveLock alloc] init];
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
        [_arrayRecursiveLock lock];
        count = [_array count];
        [_arrayRecursiveLock unlock];
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
        [_arrayRecursiveLock lock];
        obj = _array[index];
        [_arrayRecursiveLock unlock];
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
        [_arrayRecursiveLock lock];
        enu = [_array objectEnumerator];
        [_arrayRecursiveLock unlock];
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
        [_arrayRecursiveLock lock];
        [_array insertObject:anObject atIndex:index];
        [_arrayRecursiveLock unlock];
    }
}

- (void)addObject:(id)anObject;
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_array addObject:anObject];
        });
    } else {
        [_arrayRecursiveLock lock];
        [_array addObject:anObject];
        [_arrayRecursiveLock unlock];
    }
}

- (void)removeObjectAtIndex:(NSUInteger)index
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_array removeObjectAtIndex:index];
        });
    } else {
        [_arrayRecursiveLock lock];
        [_array removeObjectAtIndex:index];
        [_arrayRecursiveLock unlock];
    }
}

- (void)removeLastObject
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_array removeLastObject];
        });
    } else {
        [_arrayRecursiveLock lock];
        [_array removeLastObject];
        [_arrayRecursiveLock unlock];
    }
}

- (void)replaceObjectAtIndex:(NSUInteger)index withObject:(id)anObject
{
    if (![WXUtility threadSafeCollectionUsingLock]) {
        dispatch_barrier_async(_queue, ^{
            [_array replaceObjectAtIndex:index withObject:anObject];
        });
    } else {
        [_arrayRecursiveLock lock];
        [_array replaceObjectAtIndex:index withObject:anObject];
        [_arrayRecursiveLock unlock];
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
        [_arrayRecursiveLock lock];
        index = [_array indexOfObject:anObject];
        [_arrayRecursiveLock unlock];
    }
    
    return index;
}

- (void)dealloc
{
    if (_queue) {
        _queue = NULL;
    }
    _arrayRecursiveLock = nil;
}

@end
