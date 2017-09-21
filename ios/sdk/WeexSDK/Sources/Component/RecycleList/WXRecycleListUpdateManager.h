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

typedef void(^WXRecycleListUpdateCompletion)(BOOL isFinished);
@class WXRecycleListUpdateManager;

@protocol WXRecycleListUpdateDelegate

- (void)updateManager:(WXRecycleListUpdateManager *)manager willUpdateData:(id)newData;

- (void)updateManager:(WXRecycleListUpdateManager *)manager didUpdateData:(id)newData withSuccess:(BOOL)finished;

@end

@interface WXRecycleListUpdateManager : NSObject

@property (nonatomic, weak) UICollectionView *collectionView;
@property (nonatomic, weak) id<WXRecycleListUpdateDelegate> delegate;

- (void)reload;

- (void)updateWithNewData:(NSArray *)newData
                  oldData:(NSArray *)oldData
               completion:(WXRecycleListUpdateCompletion)completion
                animation:(BOOL)isAnimated;

- (void)updateWithAppendingData:(NSArray *)appendingData
                        oldData:(NSArray *)oldData
                     completion:(WXRecycleListUpdateCompletion)completion
                      animation:(BOOL)isAnimated;

@end
