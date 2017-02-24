/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>
#import "WXDiffUtil.h"
@class WXComponent;
@class WXCellComponent;
@class WXHeaderComponent;

@interface WXSectionDataController : NSObject <WXDiffable>

@property (nonatomic, strong) NSArray<WXCellComponent *> *cellComponents;
@property (nonatomic, strong) WXHeaderComponent *headerComponent;
@property (nonatomic, strong) WXComponent *footerComponent;

- (NSInteger)numberOfItems;

- (UIView *)cellForItemAtIndex:(NSInteger)index;

- (CGSize)sizeForItemAtIndex:(NSInteger)index;

- (UIView *)viewForHeaderAtIndex:(NSInteger)index;

- (CGSize)sizeForHeaderAtIndex:(NSInteger)index;

- (BOOL)isStickyForHeaderAtIndex:(NSInteger)index;

@end
