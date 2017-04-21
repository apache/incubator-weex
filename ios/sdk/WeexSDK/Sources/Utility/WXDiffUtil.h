/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>

@protocol WXDiffable <NSObject>

- (BOOL)isEqualToWXObject:(id<WXDiffable>)object;

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

@end

@interface WXDiffUtil : NSObject

+ (WXDiffResult *)diffWithMinimumDistance:(NSArray<id<WXDiffable>> *)newArray oldArray:(NSArray<id<WXDiffable>> *)oldArray;

@end
