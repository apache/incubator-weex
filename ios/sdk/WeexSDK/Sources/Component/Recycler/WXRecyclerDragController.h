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
#import "WXSectionDataController.h"
#import "WXComponent.h"

@class WXRecyclerDragController;

typedef enum : NSUInteger {
    WXRecyclerDragTriggerNormal,
    WXRecyclerDragTriggerPan
} WXRecyclerDragTriggerType;


@protocol WXRecyclerDragControllerDelegate <NSObject>
- (void)dragFireEvent:(NSString *)eventName params:(NSDictionary *)params;
- (void)updateDataSource;

@end

@interface WXRecyclerDragController : NSObject
@property (nonatomic,weak) id<WXRecyclerDragControllerDelegate> delegate;
@property (nonatomic,weak) UICollectionView *collectionView;
@property (nonatomic, strong) UILongPressGestureRecognizer *currentLongPress;
@property (nonatomic, strong) NSIndexPath  *startIndexPath;
@property (nonatomic, strong) NSIndexPath  *dragingIndexPath;
@property (nonatomic, strong) NSIndexPath  *targetIndexPath;
@property (nonatomic, strong) NSMutableArray *excludedAry;
@property (nonatomic, strong) UICollectionViewCell *dragingCell;
@property (nonatomic, assign) BOOL isDragable;
@property (nonatomic, assign) BOOL isDragAnchor;
@property (nonatomic, assign) WXRecyclerDragTriggerType dragTriggerType;

- (void)goThroughAnchor:(WXComponent *)wxComponent indexPath:(NSIndexPath *)indexPath;
@end
