/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>
#import "WXSectionDataController.h"
@class WXRecyclerUpdateController;

@protocol WXRecyclerUpdateControllerDelegate <NSObject>

- (void)updateController:(WXRecyclerUpdateController *)controller willPerformUpdateWithNewData:(NSArray<WXSectionDataController *> *)newData;

- (void)updateController:(WXRecyclerUpdateController *)controller didPerformUpdateWithFinished:(BOOL)finished;

- (void)updateController:(WXRecyclerUpdateController *)controller willCrashWithException:(NSException *)exception oldData:(NSArray<WXSectionDataController *> *)oldData newData:(NSArray<WXSectionDataController *> *)newData;

@end

@interface WXRecyclerUpdateController : NSObject

@property (nonatomic, weak) id<WXRecyclerUpdateControllerDelegate> delegate;

- (void)performUpdatesWithNewData:(NSArray<WXSectionDataController *> *)newData
                          oldData:(NSArray<WXSectionDataController *> *)oldData
                             view:(UICollectionView *)collectionView;

- (void)reloadItemsAtIndexPath:(NSIndexPath *)indexPath;

@end

