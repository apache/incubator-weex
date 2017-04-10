/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>
#import "WXSectionDataController.h"

@interface WXRecyclerDataController : NSObject

@property (nonatomic, strong, readonly) NSArray<WXSectionDataController *> *sections;

- (void)updateData:(NSArray<WXSectionDataController *> *)newData;

- (NSInteger)numberOfSections;

- (NSInteger)numberOfItemsInSection:(NSInteger)section;

- (UIView *)cellForItemAtIndexPath:(NSIndexPath *)indexPath;

- (CGSize)sizeForItemAtIndexPath:(NSIndexPath *)indexPath;

- (UIView *)viewForHeaderAtIndexPath:(NSIndexPath *)indexPath;

- (CGSize)sizeForHeaderAtIndexPath:(NSIndexPath *)indexPath;

- (BOOL)isStickyForHeaderAtIndexPath:(NSIndexPath *)indexPath;

- (BOOL)hasHeaderInSection:(NSInteger)section;

- (NSIndexPath *)indexPathForCell:(WXCellComponent *)cell;

- (NSUInteger)indexForHeader:(WXHeaderComponent *)header;

@end
