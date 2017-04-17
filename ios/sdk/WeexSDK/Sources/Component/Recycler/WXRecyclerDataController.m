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

#import "WXRecyclerDataController.h"
#import "WXCellComponent.h"
#import "NSArray+Weex.h"
#import "WXAssert.h"

@interface WXRecyclerDataController ()

@property (nonatomic, strong, readwrite) NSArray<WXSectionDataController *> *sections;
@property (nonatomic, strong, readonly) NSMapTable<WXCellComponent *, NSIndexPath*> *cellToIndexPathTable;
@property (nonatomic, strong, readonly) NSMapTable<WXHeaderComponent *, NSNumber*> *headerToIndexTable;

@end

@implementation WXRecyclerDataController

- (instancetype)init
{
    if (self = [super init]) {
        _sections = [NSArray new];
        _cellToIndexPathTable = [NSMapTable weakToStrongObjectsMapTable];
        _headerToIndexTable = [NSMapTable weakToStrongObjectsMapTable];
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
        if (controller.headerComponent) {
            [_headerToIndexTable setObject:@(idx) forKey:controller.headerComponent];
        }
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

- (BOOL)hasHeaderInSection:(NSInteger)section
{
    WXSectionDataController *sectionController = [self dataControllerForSection:section];
    return sectionController.headerComponent != nil;
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

- (NSUInteger)indexForHeader:(WXHeaderComponent *)header
{
    NSNumber *index = [_headerToIndexTable objectForKey:header];
    return [index unsignedIntegerValue];
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
    [_headerToIndexTable removeAllObjects];
}

@end
