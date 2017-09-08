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

#import "WXLog.h"
#import "WXUtility.h"
#import "WXComponent_internal.h"
#import "WXComponentManager.h"
#import "WXSDKInstance_private.h"

#import "WXCellSlotComponent.h"
#import "WXRecycleListComponent.h"
#import "WXRecycleListDataManager.h"
#import "WXRecycleListTemplateManager.h"
#import "WXRecycleListUpdateManager.h"

@interface WXRecycleListComponent ()

@end

@implementation WXRecycleListComponent
{
    WXRecycleListDataManager *_dataManager;
    WXRecycleListTemplateManager *_templateManager;
    WXRecycleListUpdateManager *_updateManager;
    
    NSString *_templateKey;
    __weak UICollectionView *_collectionView;
    
    NSMutableDictionary *_sizeCache;
}

- (instancetype)initWithRef:(NSString *)ref
                       type:(NSString *)type
                     styles:(NSDictionary *)styles
                 attributes:(NSDictionary *)attributes
                     events:(NSArray *)events
               weexInstance:(WXSDKInstance *)weexInstance
{
    if (self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance]) {
        _dataManager = [[WXRecycleListDataManager alloc] initWithData:attributes[@"listData"]];
        _templateManager = [WXRecycleListTemplateManager new];
        _updateManager = [WXRecycleListUpdateManager new];
        _templateKey = [WXConvert NSString:attributes[@"templateKey"]] ? : @"templateType";
        _sizeCache = [NSMutableDictionary dictionary];
    }
    
    return self;
}

#pragma mark - WXComponent Methods

- (UIView *)loadView
{
    UICollectionViewFlowLayout *layout = [UICollectionViewFlowLayout new];
    return [[UICollectionView alloc] initWithFrame:CGRectZero collectionViewLayout:layout];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _collectionView = (UICollectionView *)self.view;
    _collectionView.allowsSelection = NO;
    _collectionView.allowsMultipleSelection = NO;
    _collectionView.dataSource = self;
    _collectionView.delegate = self;
    
    _templateManager.collectionView = _collectionView;
    _updateManager.collectionView = _collectionView;
//    [_updateManager reload];
}

- (void)viewWillUnload
{
    [super viewWillUnload];
    
    _collectionView.dataSource = nil;
    _collectionView.delegate = nil;
}

- (void)setContentSize:(CGSize)contentSize
{
    // Do Nothing
}

- (void)adjustSticky
{
    // Do Nothing, sticky is adjusted by layout
}

#pragma mark - WXComonent Internal Methods

- (void)_insertSubcomponent:(WXComponent *)subcomponent atIndex:(NSInteger)index
{
   [super _insertSubcomponent:subcomponent atIndex:index];
    
    if ([subcomponent isKindOfClass:[WXCellSlotComponent class]]) {
        WXCellSlotComponent *cell = (WXCellSlotComponent*)subcomponent;
        [self.weexInstance.componentManager _addUITask:^{
            [_templateManager addTemplate:cell];
        }];
        
        //TODO: update collection view if adding template
    }
}

#pragma mark - UICollectionViewDataSource

- (NSInteger)numberOfSectionsInCollectionView:(UICollectionView *)collectionView
{
    return 1;
}

- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    return [_dataManager numberOfItems];
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath
{
    // 1. get the data relating to the cell
    NSDictionary *data = [_dataManager dataAtIndex:indexPath.row];
    if (!data || ![data isKindOfClass:[NSDictionary class]]) {
        WXLogError(@"No data or wrong data format for index:%zd, data:%@", indexPath.row, data);
        return nil;
    }
    
    // 2. get the template type specified by data
    NSString *templateType = data[_templateKey];
    if (!templateType) {
        WXLogError(@"Each data should have a value for %@ to indicate template type", _templateKey);
        return nil;
    }
    
    // 3. dequeue a cell component by template type
    UICollectionViewCell *cellView = [_collectionView dequeueReusableCellWithReuseIdentifier:templateType forIndexPath:indexPath];
    WXCellSlotComponent *cellComponent = (WXCellSlotComponent *)cellView.wx_component;
    if (!cellComponent) {
        cellComponent = [_templateManager dequeueCellSlotWithType:templateType forIndexPath:indexPath];
        cellView.wx_component = cellComponent;
        WXPerformBlockOnComponentThread(^{
            //TODO: How can we avoid this?
            [super _insertSubcomponent:cellComponent atIndex:self.subcomponents.count];
        });
    }
    
    
    // 4. binding the data to the cell component
    WXPerformBlockOnComponentThread(^{
        [cellComponent updateCellData:data];
        WXPerformBlockOnMainThread(^{
            NSValue *cacheSize = _sizeCache[indexPath];
            if (!cacheSize || !CGSizeEqualToSize([cacheSize CGSizeValue] , cellComponent.calculatedFrame.size)) {
                _sizeCache[indexPath] = [NSValue valueWithCGSize:cellComponent.calculatedFrame.size];
                [UIView performWithoutAnimation:^{
                    [_collectionView reloadItemsAtIndexPaths:@[indexPath]];
                }];
            }
        });
    });
    
    // 5. Add cell component's view to content view.
    UIView *contentView = cellComponent.view;
    if (contentView.superview == cellView.contentView) {
        return cellView;
    }
    
    for (UIView *view in cellView.contentView.subviews) {
        [view removeFromSuperview];
    }
    [cellView.contentView addSubview:contentView];
    [cellView setAccessibilityIdentifier:contentView.accessibilityIdentifier];
    
    return cellView;
}

- (UICollectionReusableView *)collectionView:(UICollectionView *)collectionView viewForSupplementaryElementOfKind:(NSString *)kind atIndexPath:(NSIndexPath *)indexPath
{
    return nil;
}

#pragma mark - UICollectionViewDelegate

- (void)collectionView:(UICollectionView *)collectionView willDisplayCell:(UICollectionViewCell *)cell forItemAtIndexPath:(NSIndexPath *)indexPath
{
    WXLogDebug(@"will display cell:%@, at index path:%@", cell, indexPath);
}

- (void)collectionView:(UICollectionView *)collectionView didEndDisplayingCell:(UICollectionViewCell *)cell forItemAtIndexPath:(NSIndexPath *)indexPath
{
    WXLogDebug(@"Did end displaying cell:%@, at index path:%@", cell, indexPath);
}

#pragma mark - UICollectionViewDelegateFlowLayout
- (CGSize)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath *)indexPath
{
    NSValue *size = _sizeCache[indexPath];
    if (size) {
        return [size CGSizeValue];
    } else {
        NSDictionary *data = [_dataManager dataAtIndex:indexPath.row];
        WXCellSlotComponent *cell = [_templateManager templateWithType:data[_templateKey]];
        CGSize size = cell.calculatedFrame.size;
        _sizeCache[indexPath] = [NSValue valueWithCGSize:size];
        return size;
    }
}

@end
