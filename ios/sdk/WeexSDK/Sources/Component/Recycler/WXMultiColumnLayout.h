/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
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

@property (nonatomic, assign) float columnGap;

@property (nonatomic, assign, readonly) CGFloat computedColumnWidth;
@property (nonatomic, assign, readonly) int computedColumnCount;
@property (nonatomic, assign, readonly) CGFloat computedHeaderWidth;

@end
