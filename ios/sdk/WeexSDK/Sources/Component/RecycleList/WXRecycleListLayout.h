//
//  WXRecycleListLinearLayout.h
//  WeexSDK
//
//  Created by yinfeng on 2017/9/19.
//  Copyright © 2017年 taobao. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol WXRecycleListLayoutDelegate

- (BOOL)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout isNeedStickyForIndexPath:(NSIndexPath *)indexPath;

@end

@interface WXRecycleListLayout : UICollectionViewFlowLayout

@property (nonatomic, weak) id<WXRecycleListLayoutDelegate> delegate;

@end
