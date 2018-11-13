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

#import <Foundation/Foundation.h>

/**
 *  @abstract Thread safe NSMutableDictionary
 */
@interface WXThreadSafeMutableDictionary<KeyType, ObjectType> : NSMutableDictionary

/* WXThreadSafeMutableDictionary inherites from NSMutableDictionary for backward capability.
 Keep in mind that only the following methods are thread safe guaranteed.
 And MUST not use other methods provideded by NSMutableDictionary. */

- (instancetype)init;
- (instancetype)initWithCapacity:(NSUInteger)numItems;
- (instancetype)initWithDictionary:(NSDictionary *)dictionary;
- (instancetype)initWithCoder:(NSCoder *)aDecoder;
- (instancetype)initWithObjects:(const id [])objects forKeys:(const id<NSCopying> [])keys count:(NSUInteger)cnt;

- (NSUInteger)count;
- (id)objectForKey:(id)key;
- (id)objectForKeyedSubscript:(id)key;
- (NSEnumerator *)keyEnumerator;
- (void)setObject:(id)anObject forKey:(id<NSCopying>)aKey;
- (void)setObject:(id)anObject forKeyedSubscript:(id <NSCopying>)key;
- (NSArray *)allKeys;
- (NSArray *)allValues;
- (void)removeObjectForKey:(id)aKey;
- (void)removeAllObjects;

@end
