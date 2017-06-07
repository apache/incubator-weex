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

#import "WXComponent.h"
#import "WXDiffUtil.h"
@class WXCellComponent;

@protocol WXCellRenderDelegate <NSObject>

- (float)containerWidthForLayout:(WXCellComponent *)cell;

- (void)cellDidLayout:(WXCellComponent *)cell;

- (void)cellDidRendered:(WXCellComponent *)cell;

- (void)cellDidRemove:(WXCellComponent *)cell;

- (void)cell:(WXCellComponent *)cell didMoveToIndex:(NSUInteger)index;

@end

@interface WXCellComponent : WXComponent <WXDiffable>

@property (nonatomic, strong) NSString *scope;
@property (nonatomic, assign) BOOL isRecycle;
@property (nonatomic, assign) BOOL isLayoutComplete;
@property (nonatomic, assign) UITableViewRowAnimation insertAnimation;
@property (nonatomic, assign) UITableViewRowAnimation deleteAnimation;
@property (nonatomic, assign) BOOL keepScrollPosition;
@property (nonatomic, weak) id<WXCellRenderDelegate> delegate;

@end
