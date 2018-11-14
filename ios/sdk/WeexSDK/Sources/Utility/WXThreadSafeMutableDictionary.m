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
#import <pthread/pthread.h>

@interface WXThreadSafeMutableDictionary ()
{
    NSMutableDictionary* _dict;
    pthread_mutex_t _safeThreadDictionaryMutex;
    pthread_mutexattr_t _safeThreadDictionaryMutexAttr;
}

@end

@implementation WXThreadSafeMutableDictionary

- (instancetype)initCommon
{
    self = [super init];
    if (self) {
        pthread_mutexattr_init(&(_safeThreadDictionaryMutexAttr));
        pthread_mutexattr_settype(&(_safeThreadDictionaryMutexAttr), PTHREAD_MUTEX_RECURSIVE); // must use recursive lock
        pthread_mutex_init(&(_safeThreadDictionaryMutex), &(_safeThreadDictionaryMutexAttr));
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

- (instancetype)initWithDictionary:(NSDictionary *)dictionary
{
    self = [self initCommon];
    if (self) {
        _dict = [NSMutableDictionary dictionaryWithDictionary:dictionary];
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
    @try {
        pthread_mutex_lock(&_safeThreadDictionaryMutex);
        return [_dict count];
    }
    @finally {
        pthread_mutex_unlock(&_safeThreadDictionaryMutex);
    }
}

- (id)objectForKey:(id)key
{
    if (nil == key) {
        return nil;
    }
    
    @try {
        pthread_mutex_lock(&_safeThreadDictionaryMutex);
        return [_dict objectForKey:key];
    }
    @finally {
        pthread_mutex_unlock(&_safeThreadDictionaryMutex);
    }
}

- (id)objectForKeyedSubscript:(id)key
{
    if (nil == key) {
        return nil;
    }
    
    @try {
        pthread_mutex_lock(&_safeThreadDictionaryMutex);
        return [_dict objectForKeyedSubscript:key];
    }
    @finally {
        pthread_mutex_unlock(&_safeThreadDictionaryMutex);
    }
}

- (NSEnumerator *)keyEnumerator
{
    @try {
        pthread_mutex_lock(&_safeThreadDictionaryMutex);
        return [_dict keyEnumerator];
    }
    @finally {
        pthread_mutex_unlock(&_safeThreadDictionaryMutex);
    }
}

- (void)setObject:(id)anObject forKey:(id<NSCopying>)aKey
{
    id originalObject = nil; // make sure that object is not released in lock
    @try {
        pthread_mutex_lock(&_safeThreadDictionaryMutex);
        originalObject = [_dict objectForKey:aKey];
        [_dict setObject:anObject forKey:aKey];
    }
    @finally {
        pthread_mutex_unlock(&_safeThreadDictionaryMutex);
    }
    originalObject = nil;
}

- (void)setObject:(id)anObject forKeyedSubscript:(id <NSCopying>)key
{
    id originalObject = nil; // make sure that object is not released in lock
    @try {
        pthread_mutex_lock(&_safeThreadDictionaryMutex);
        originalObject = [_dict objectForKey:key];
        [_dict setObject:anObject forKeyedSubscript:key];
    }
    @finally {
        pthread_mutex_unlock(&_safeThreadDictionaryMutex);
    }
    originalObject = nil;
}

- (NSArray *)allKeys
{
    @try {
        pthread_mutex_lock(&_safeThreadDictionaryMutex);
        return [_dict allKeys];
    }
    @finally {
        pthread_mutex_unlock(&_safeThreadDictionaryMutex);
    }
}

- (NSArray *)allValues
{
    @try {
        pthread_mutex_lock(&_safeThreadDictionaryMutex);
        return [_dict allValues];
    }
    @finally {
        pthread_mutex_unlock(&_safeThreadDictionaryMutex);
    }
}

- (void)removeObjectForKey:(id)aKey
{
    id originalObject = nil; // make sure that object is not released in lock
    @try {
        pthread_mutex_lock(&_safeThreadDictionaryMutex);
        originalObject = [_dict objectForKey:aKey];
        if (originalObject) {
            [_dict removeObjectForKey:aKey];
        }
    }
    @finally {
        pthread_mutex_unlock(&_safeThreadDictionaryMutex);
    }
    originalObject = nil;
}

- (void)removeAllObjects
{
    NSArray* allValues = nil; // make sure that objects are not released in lock
    @try {
        pthread_mutex_lock(&_safeThreadDictionaryMutex);
        allValues = [_dict allValues];
        [_dict removeAllObjects];
    }
    @finally {
        pthread_mutex_unlock(&_safeThreadDictionaryMutex);
    }
    allValues = nil;
}

- (id)copy
{
    @try {
        pthread_mutex_lock(&_safeThreadDictionaryMutex);
        return [_dict copy];
    }
    @finally {
        pthread_mutex_unlock(&_safeThreadDictionaryMutex);
    }
}

- (void)dealloc
{
    pthread_mutex_destroy(&_safeThreadDictionaryMutex);
    pthread_mutexattr_destroy(&_safeThreadDictionaryMutexAttr);
}

@end
