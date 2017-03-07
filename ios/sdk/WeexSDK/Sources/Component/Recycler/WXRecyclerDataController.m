/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXRecyclerDataController.h"
#import "WXCellComponent.h"
#import "NSArray+Weex.h"
#import "WXAssert.h"

@interface WXRecyclerDataController ()

@property (nonatomic, strong, readwrite) NSArray<WXSectionDataController *> *sections;
@property (nonatomic, strong, readonly) NSMapTable<WXCellComponent *, NSIndexPath*> *cellToIndexPathTable;

@end

@implementation WXRecyclerDataController

- (instancetype)init
{
    if (self = [super init]) {
        _sections = [NSArray new];
        _cellToIndexPathTable = [NSMapTable weakToStrongObjectsMapTable];
    }
    
    return self;
}

#pragma mark - Public

- (void)updateData:(NSArray<WXSectionDataController *> *)newData
{
    WXAssertMainThread();
    
    [self cleanup];
    _sections = [newData copy];
    
    [newData enumerateObjectsUsingBlock:^(WXSectionDataController * _Nonnull controller, NSUInteger idx, BOOL * _Nonnull stop) {
        [controller.cellComponents enumerateObjectsUsingBlock:^(WXCellComponent * _Nonnull obj, NSUInteger idx2, BOOL * _Nonnull stop) {
            NSIndexPath *indexPath = [NSIndexPath indexPathForItem:idx2 inSection:idx];
            [_cellToIndexPathTable setObject:indexPath forKey:obj];
        }];
    }];
}

- (NSInteger)numberOfSections
{
    WXAssertMainThread();
    return self.sections.count;
}

- (NSInteger)numberOfItemsInSection:(NSInteger)section
{
    WXSectionDataController *sectionController = [self dataControllerForSection:section];
    WXAssert(sectionController, @"No section controller found for section:%ld", section);
    
    return [sectionController numberOfItems];
}

- (UIView *)cellForItemAtIndexPath:(NSIndexPath *)indexPath
{
    WXSectionDataController *sectionController = [self dataControllerForSection:indexPath.section];
    UIView *contentView = [sectionController cellForItemAtIndex:indexPath.item];
    
    return contentView;
}

- (CGSize)sizeForItemAtIndexPath:(NSIndexPath *)indexPath
{
    WXSectionDataController *sectionController = [self dataControllerForSection:indexPath.section];
    return [sectionController sizeForItemAtIndex:indexPath.item];
}

- (UIView *)viewForHeaderAtIndexPath:(NSIndexPath *)indexPath;
{
    WXSectionDataController *sectionController = [self dataControllerForSection:indexPath.section];
    return [sectionController viewForHeaderAtIndex:indexPath.item];
}

- (CGSize)sizeForHeaderAtIndexPath:(NSIndexPath *)indexPath
{
    WXSectionDataController *sectionController = [self dataControllerForSection:indexPath.section];
    return [sectionController sizeForHeaderAtIndex:indexPath.item];
}

- (BOOL)isStickyForHeaderAtIndexPath:(NSIndexPath *)indexPath
{
    WXSectionDataController *sectionController = [self dataControllerForSection:indexPath.section];
    return [sectionController isStickyForHeaderAtIndex:indexPath.item];
}

- (NSIndexPath *)indexPathForCell:(WXCellComponent *)cell
{
    return [_cellToIndexPathTable objectForKey:cell];
}

#pragma mark - Private

- (WXSectionDataController *)dataControllerForSection:(NSInteger)section
{
    WXAssertMainThread();
    return [self.sections wx_safeObjectAtIndex:section];
}

- (void)cleanup
{
    [_cellToIndexPathTable removeAllObjects];
}

@end
