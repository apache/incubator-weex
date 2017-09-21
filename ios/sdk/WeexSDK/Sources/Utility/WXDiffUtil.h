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

@protocol WXDiffable <NSObject>

- (BOOL)weex_isEqualTo:(id<WXDiffable>)object;

@end

@interface WXDiffUpdateIndex : NSObject

@property (nonatomic, assign, readonly) NSUInteger oldIndex;
@property (nonatomic, assign, readonly) NSUInteger newIndex;

@end

@interface WXDiffResult : NSObject

@property (nonatomic, strong, readonly) NSIndexSet *inserts;
@property (nonatomic, strong, readonly) NSIndexSet *deletes;
@property (nonatomic, strong, readonly) NSArray<WXDiffUpdateIndex *> *updates;

- (BOOL)hasChanges;

- (instancetype)initWithInserts:(NSIndexSet *)inserts
                        deletes:(NSIndexSet *)deletes
                        updates:(NSArray<WXDiffUpdateIndex *> *)updates;

@end

@interface WXDiffUtil : NSObject

+ (WXDiffResult *)diffWithMinimumDistance:(NSArray<id<WXDiffable>> *)newArray oldArray:(NSArray<id<WXDiffable>> *)oldArray;

@end

@interface NSNumber (WXDiffable) <WXDiffable>

@end

@interface NSString (WXDiffable) <WXDiffable>

@end

@interface NSArray (WXDiffable) <WXDiffable>

@end

@interface NSDictionary (WXDiffable) <WXDiffable>

@end

