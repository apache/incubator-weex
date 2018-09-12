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

#import <UIKit/UIKit.h>
#import "WXLength.h"

extern NSString * const kCollectionSupplementaryViewKindHeader;

@protocol WXMultiColumnLayoutDelegate <NSObject>

- (UIEdgeInsets)collectionView:(UICollectionView *)collectionView insetForLayout:(UICollectionViewLayout *)collectionViewLayout;

- (CGFloat)collectionView:(UICollectionView *)collectionView contentWidthForLayout:(UICollectionViewLayout *)collectionViewLayout;

- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout heightForItemAtIndexPath:(NSIndexPath *)indexPath;

- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout heightForHeaderInSection:(NSInteger)section;

- (BOOL)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout hasHeaderInSection:(NSInteger)section;

- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout isNeedStickyForHeaderInSection:(NSInteger)section;

@end

@interface WXMultiColumnLayout : UICollectionViewLayout

@property (nonatomic, weak) id<WXMultiColumnLayoutDelegate> delegate;

@property (nonatomic, strong) WXLength *columnCount;

@property (nonatomic, strong) WXLength *columnWidth;
@property (nonatomic, assign) float leftGap;
@property (nonatomic, assign) float rightGap;

@property (nonatomic, assign) float columnGap;

@property (nonatomic, assign, readonly) CGFloat computedColumnWidth;
@property (nonatomic, assign, readonly) int computedColumnCount;
@property (nonatomic, assign, readonly) CGFloat computedHeaderWidth;

//Under system version 10.0, UICollectionViewLayout.collectionView seems be unsafe_unretain rather than weak. so here add one weak collectionview to use inside the imp to avoid bad access.
@property (nonatomic, weak) UICollectionView *weak_collectionView;

@end
