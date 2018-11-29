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
 *  @abstract Thread safe NSMutableArray
 */
@interface WXThreadSafeMutableArray : NSMutableArray

/* WXThreadSafeMutableArray inherites from NSMutableArray for backward capability.
 Keep in mind that only the following methods are thread safe guaranteed.
 And MUST not use other methods provideded by NSMutableArray. */

- (instancetype)init;
- (instancetype)initWithCapacity:(NSUInteger)numItems;
- (instancetype)initWithArray:(NSArray *)array;
- (instancetype)initWithCoder:(NSCoder *)aDecoder;
- (instancetype)initWithObjects:(const id [])objects count:(NSUInteger)cnt;

- (NSUInteger)count;
- (id)objectAtIndex:(NSUInteger)index;
- (id)objectAtIndexedSubscript:(NSUInteger)index;
- (id)firstObject;
- (id)lastObject;
- (BOOL)containsObject:(id)anObject;
- (NSEnumerator *)objectEnumerator;
- (NSEnumerator *)reverseObjectEnumerator;
- (void)insertObject:(id)anObject atIndex:(NSUInteger)index;
- (void)setObject:(id)anObject atIndexedSubscript:(NSUInteger)index;
- (void)addObject:(id)anObject;
- (void)removeObject:(id)anObject;
- (void)removeObjectAtIndex:(NSUInteger)index;
- (void)removeLastObject;
- (void)removeAllObjects;
- (void)replaceObjectAtIndex:(NSUInteger)index withObject:(id)anObject;
- (NSUInteger)indexOfObject:(id)anObject;

@end
