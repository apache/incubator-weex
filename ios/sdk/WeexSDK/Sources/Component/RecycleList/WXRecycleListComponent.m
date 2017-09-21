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
#import "WXRecycleListLayout.h"
#import "WXRecycleListComponent.h"
#import "WXRecycleListDataManager.h"
#import "WXRecycleListTemplateManager.h"
#import "WXRecycleListUpdateManager.h"

@interface WXRecycleListComponent () <WXRecycleListLayoutDelegate, WXRecycleListUpdateDelegate, UICollectionViewDelegateFlowLayout, UICollectionViewDataSource>

@end

@implementation WXRecycleListComponent
{
    WXRecycleListDataManager *_dataManager;
    WXRecycleListTemplateManager *_templateManager;
    WXRecycleListUpdateManager *_updateManager;
    
    NSString *_templateKey;
    NSString *_aliasKey;
    NSString *_indexKey;
    __weak UICollectionView *_collectionView;
    
    NSMutableDictionary *_sizeCache;
    NSMutableDictionary *_stickyCache;
    
    NSUInteger _previousLoadMoreCellNumber;
}

WX_EXPORT_METHOD(@selector(appendData:))
WX_EXPORT_METHOD(@selector(insertData:atIndex:))
WX_EXPORT_METHOD(@selector(updateData:atIndex:))
WX_EXPORT_METHOD(@selector(removeData:))
WX_EXPORT_METHOD(@selector(moveData:toIndex:))
WX_EXPORT_METHOD(@selector(scrollTo:options:))

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
        _updateManager.delegate = self;
        _templateKey = [WXConvert NSString:attributes[@"templateKey"]] ? : @"templateType";
        _aliasKey = [WXConvert NSString:attributes[@"alias"]];
        _indexKey = [WXConvert NSString:attributes[@"index"]];
        _sizeCache = [NSMutableDictionary dictionary];
        _stickyCache = [NSMutableDictionary dictionary];
    }
    
    return self;
}

#pragma mark - WXComponent Methods

- (UIView *)loadView
{
    WXRecycleListLayout *layout = [WXRecycleListLayout new];
    layout.delegate = self;
    // to show cells that original width / height is zero, otherwise cellForItemAtIndexPath will not be called
    layout.minimumLineSpacing = 0.01;
    layout.minimumInteritemSpacing = 0.01;
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
}

- (void)viewWillUnload
{
    [super viewWillUnload];
    
    _collectionView.dataSource = nil;
    _collectionView.delegate = nil;
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    if (attributes[@"listData"]) {
        NSArray *listData = attributes[@"listData"];
        [self _updateListData:listData withCompletion:nil animation:NO];
    }
}

- (CGPoint)absolutePositionForComponent:(WXComponent *)component
{
    CGPoint position = CGPointZero;
    UIView *view = component->_view;
    while (view) {
        if ([view isKindOfClass:[UICollectionViewCell class]]) {
            NSIndexPath *indexPath = [_collectionView indexPathForCell:(UICollectionViewCell *)view];
            if (!indexPath) {
                return CGPointMake(NAN, NAN);
            }
            UICollectionViewLayoutAttributes *attributes = [_collectionView layoutAttributesForItemAtIndexPath:indexPath];
            CGPoint cellOrigin = attributes.frame.origin;
            position = CGPointMake(position.x + cellOrigin.x,
                                   position.y + cellOrigin.y);
            break;
        }
        position = CGPointMake(position.x + view.frame.origin.x,
                               position.y + view.frame.origin.y);
        view = view.superview;
    }
    
    return position;
}

- (void)setContentSize:(CGSize)contentSize
{
    // Do Nothing
}

- (void)adjustSticky
{
    // Do Nothing, sticky is adjusted by layout
}

#pragma mark - Load More Event

- (void)loadMore
{
    [super loadMore];
    
    _previousLoadMoreCellNumber = [_collectionView numberOfItemsInSection:0];
}

- (BOOL)isNeedLoadMore
{
    BOOL superNeedLoadMore = [super isNeedLoadMore];
    return superNeedLoadMore && _previousLoadMoreCellNumber != [_collectionView numberOfItemsInSection:0];
}

- (void)resetLoadmore
{
    [super resetLoadmore];
    _previousLoadMoreCellNumber = 0;
}

#pragma mark - Exported Component Methods

- (void)appendData:(NSArray *)appendingData
{
    if (![appendingData isKindOfClass:[NSArray class]]) {
        WXLogError(@"wrong format of appending data:%@", appendingData);
        return;
    }
    
    NSArray *oldData = [_dataManager data];
    [_updateManager updateWithAppendingData:appendingData oldData:oldData completion:nil animation:NO];
}

- (void)insertData:(id)data atIndex:(NSUInteger)index
{
    // TODO: bring the update logic to UpdateManager
    // TODO: update cell because index has changed
    NSMutableArray *newListData = [[_dataManager data] mutableCopy];
    if (index <= newListData.count) {
        [newListData insertObject:data atIndex:index];
        [_dataManager updateData:newListData];
        
        NSIndexPath *indexPath = [NSIndexPath indexPathForItem:index inSection:0];
        
        [UIView performWithoutAnimation:^{
            [_collectionView insertItemsAtIndexPaths:@[indexPath]];
        }];
    }
}

- (void)updateData:(id)data atIndex:(NSUInteger)index
{
    // TODO: bring the update logic to UpdateManager
    NSMutableArray *newListData = [[_dataManager data] mutableCopy];
    if (index < newListData.count) {
        newListData[index] = data;
        [_dataManager updateData:newListData];
        
        NSIndexPath *indexPath = [NSIndexPath indexPathForItem:index inSection:0];
        UICollectionViewCell *cellView = [_collectionView cellForItemAtIndexPath:indexPath];
        WXCellSlotComponent *cellComponent = (WXCellSlotComponent *)cellView.wx_component;
        if (cellComponent) {
            [self _updateBindingData:data forCell:cellComponent atIndexPath:indexPath];
        }
    }
}

- (void)removeData:(NSArray *)indexes
{
    // TODO: bring the update logic to UpdateManager
    NSMutableArray *newListData = [[_dataManager data] mutableCopy];
    NSMutableIndexSet *indexSet = [NSMutableIndexSet new];
    NSMutableArray *indexPaths = [NSMutableArray array];
    for (NSNumber *index in indexes) {
        if ([index unsignedIntegerValue] >= newListData.count) {
            WXLogError(@"invalid remove index:%@", index);
            continue;
        }
        [indexSet addIndex:[index unsignedIntegerValue]];
        [indexPaths addObject:[NSIndexPath indexPathForItem:[index unsignedIntegerValue] inSection:0]];
    }
    
    [newListData removeObjectsAtIndexes:indexSet];
    [_dataManager updateData:newListData];
    [UIView performWithoutAnimation:^{
        [_collectionView deleteItemsAtIndexPaths:indexPaths];
    }];
}

- (void)moveData:(NSUInteger)fromIndex toIndex:(NSUInteger)toIndex
{
    // TODO: bring the update logic to UpdateManager
    NSMutableArray *newListData = [[_dataManager data] mutableCopy];
    id data = newListData[fromIndex];
    [newListData removeObjectAtIndex:fromIndex];
    [newListData insertObject:data atIndex:toIndex];
    [_dataManager updateData:newListData];
    
    NSIndexPath *fromIndexPath = [NSIndexPath indexPathForItem:fromIndex inSection:0];
    NSIndexPath *toIndexPath = [NSIndexPath indexPathForItem:toIndex inSection:0];
    [UIView performWithoutAnimation:^{
        [_collectionView moveItemAtIndexPath:fromIndexPath toIndexPath:toIndexPath];
    }];
}

- (void)scrollTo:(NSUInteger)index options:(NSDictionary *)options
{
    NSIndexPath *toIndexPath = [NSIndexPath indexPathForItem:index inSection:0];
    BOOL animated = options[@"animated"] ? [WXConvert BOOL:options[@"animated"]] : NO;
    [_collectionView scrollToItemAtIndexPath:toIndexPath atScrollPosition:UICollectionViewScrollPositionTop animated:animated];
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

#pragma mark - Private

- (void)_updateBindingData:(NSDictionary *)data forCell:(WXCellSlotComponent *)cellComponent atIndexPath:(NSIndexPath *)indexPath
{
    if (_aliasKey) {
        data = @{_aliasKey:data};
    }
    if (_indexKey) {
        NSMutableDictionary *dataNew = [data mutableCopy];
        dataNew[_indexKey] = @(indexPath.item);
        data = dataNew;
    }
    
#ifdef DEBUG
    NSDate *startTime = [NSDate date];
#endif
    WXPerformBlockSyncOnComponentThread(^{
        [cellComponent updateCellData:data];
    });
#ifdef DEBUG
    double duration = -[startTime timeIntervalSinceNow] * 1000;
    WXLogDebug(@"cell:%zi update data time:%f", indexPath.item, duration);
#endif
    
    NSValue *cachedSize = _sizeCache[indexPath];
    if (!cachedSize || !CGSizeEqualToSize([cachedSize CGSizeValue] , cellComponent.calculatedFrame.size)) {
        _sizeCache[indexPath] = [NSValue valueWithCGSize:cellComponent.calculatedFrame.size];
        [_collectionView.collectionViewLayout invalidateLayout];
    }
    NSNumber *cachedSticky = _stickyCache[indexPath];
    BOOL isSticky = cellComponent->_positionType == WXPositionTypeSticky;
    if (!cachedSticky || [cachedSticky boolValue] != isSticky) {
        _stickyCache[indexPath] = @(isSticky);
    }
}

- (void)_updateListData:(NSArray *)newData
        withCompletion:(WXRecycleListUpdateCompletion)completion
             animation:(BOOL)animation
{
    if (![newData isKindOfClass:[NSArray class]]) {
        WXLogError(@"wrong format of list data:%@", newData);
        completion(NO);
        return;
    }
    
    NSArray *oldData = [_dataManager data];
    [_updateManager updateWithNewData:newData oldData:oldData completion:completion animation:animation];
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
        WXLogError(@"No data or wrong data format for index:%zd, data:%@", indexPath.item, data);
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
    [self _updateBindingData:data forCell:cellComponent atIndexPath:indexPath];

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
    
    WXLogDebug(@"Return cell view:%@, indexPath:%@", cellView, indexPath);
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self handleAppear];
    });
    
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
        return CGSizeMake(_collectionView.frame.size.width, size.height);
    }
}

#pragma mark - WXRecycleListLayoutDelegate

- (BOOL)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout isNeedStickyForIndexPath:(NSIndexPath *)indexPath
{
    NSNumber *cachedSticky = _stickyCache[indexPath];
    if (cachedSticky) {
        return [cachedSticky boolValue];
    } else {
        return NO;
    }
}

#pragma mark - WXRecycleListUpdateDelegate

- (void)updateManager:(WXRecycleListUpdateManager *)manager willUpdateData:(id)newData
{
    [_dataManager updateData:newData];
}

- (void)updateManager:(WXRecycleListUpdateManager *)manager didUpdateData:(id)newData withSuccess:(BOOL)finished
{
    
}

@end
