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

#import "WXRecyclerComponent.h"
#import "WXComponent_internal.h"
#import "WXSDKInstance_private.h"
#import "WXRecyclerDataController.h"
#import "WXRecyclerUpdateController.h"
#import "WXMultiColumnLayout.h"
#import "WXHeaderComponent.h"
#import "WXFooterComponent.h"
#import "WXCellComponent.h"
#import "WXAssert.h"
#import "WXConvert.h"

static NSString * const kCollectionCellReuseIdentifier = @"WXRecyclerCell";
static NSString * const kCollectionHeaderReuseIdentifier = @"WXRecyclerHeader";
static float const kRecyclerNormalColumnGap = 32;

typedef enum : NSUInteger {
    WXRecyclerLayoutTypeMultiColumn,
    WXRecyclerLayoutTypeFlex,
    WXRecyclerLayoutTypeGrid,
} WXRecyclerLayoutType;

@interface WXCollectionView : UICollectionView

@end

@implementation WXCollectionView

- (void)insertSubview:(UIView *)view atIndex:(NSInteger)index
{
    [super insertSubview:view atIndex:index];
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    [self.wx_component layoutDidFinish];
}

- (void)setContentOffset:(CGPoint)contentOffset
{
    [super setContentOffset:contentOffset];
}

@end

@interface WXCollectionViewCell : UICollectionViewCell

@end

@implementation WXCollectionViewCell

- (void)prepareForReuse
{
    [super prepareForReuse];

    WXCellComponent *cellComponent = (WXCellComponent *)self.wx_component;
    if (cellComponent.isRecycle && [cellComponent isViewLoaded] && [self.contentView.subviews containsObject:cellComponent.view]) {
        [cellComponent _unloadViewWithReusing:YES];
    }
}

@end

@interface WXRecyclerComponent () <UICollectionViewDataSource, UICollectionViewDelegate, WXMultiColumnLayoutDelegate, WXRecyclerUpdateControllerDelegate, WXCellRenderDelegate, WXHeaderRenderDelegate>

@property (nonatomic, strong, readonly) WXRecyclerDataController *dataController;
@property (nonatomic, strong, readonly) WXRecyclerUpdateController *updateController;
@property (nonatomic, weak, readonly) UICollectionView *collectionView;

@end

@implementation WXRecyclerComponent
{
    WXRecyclerLayoutType _layoutType;
    UICollectionViewLayout *_collectionViewlayout;
    
    UIEdgeInsets _padding;
    NSUInteger _previousLoadMoreCellNumber;
}

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance
{
    if (self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance]) {
        [self _fillPadding];
        
        if ([type isEqualToString:@"waterfall"] || (attributes[@"layout"] && [attributes[@"layout"] isEqualToString:@"multi-column"])) {
            // TODO: abstraction
            _layoutType = WXRecyclerLayoutTypeMultiColumn;
            CGFloat scaleFactor = weexInstance.pixelScaleFactor;
            _collectionViewlayout = [WXMultiColumnLayout new];
            WXMultiColumnLayout *layout = (WXMultiColumnLayout *)_collectionViewlayout;
            layout.columnWidth = [WXConvert WXLength:attributes[@"columnWidth"] isFloat:YES scaleFactor:scaleFactor] ? : [WXLength lengthWithFloat:0.0 type:WXLengthTypeAuto];
            layout.columnCount = [WXConvert WXLength:attributes[@"columnCount"] isFloat:NO scaleFactor:1.0] ? : [WXLength lengthWithInt:1 type:WXLengthTypeFixed];
            layout.columnGap = [self _floatValueForColumnGap:([WXConvert WXLength:attributes[@"columnGap"] isFloat:YES scaleFactor:scaleFactor] ? : [WXLength lengthWithFloat:0.0 type:WXLengthTypeNormal])];

            layout.delegate = self;
        } else {
            _collectionViewlayout = [UICollectionViewLayout new];
        }
        
        _dataController = [WXRecyclerDataController new];
        _updateController = [WXRecyclerUpdateController new];
        _updateController.delegate = self;
        [self fixFlicker];
    }
    
    return self;
}

- (void)dealloc
{
    _collectionView.delegate = nil;
    _collectionView.dataSource = nil;
}

#pragma mark - Public Subclass Methods

- (UIView *)loadView
{
    return [[WXCollectionView alloc] initWithFrame:CGRectZero collectionViewLayout:_collectionViewlayout];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _collectionView = (UICollectionView *)self.view;
    _collectionView.allowsSelection = NO;
    _collectionView.allowsMultipleSelection = NO;
    _collectionView.dataSource = self;
    _collectionView.delegate = self;
    
    [_collectionView registerClass:[WXCollectionViewCell class] forCellWithReuseIdentifier:kCollectionCellReuseIdentifier];
    [_collectionView registerClass:[UICollectionReusableView class] forSupplementaryViewOfKind:kCollectionSupplementaryViewKindHeader withReuseIdentifier:kCollectionHeaderReuseIdentifier];
    
    [self performUpdatesWithCompletion:^(BOOL finished) {
        
    }];
}

- (void)viewWillUnload
{
    [super viewWillUnload];
    
    _collectionView.dataSource = nil;
    _collectionView.delegate = nil;
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    [super updateAttributes:attributes];
    
    if (_layoutType == WXRecyclerLayoutTypeMultiColumn) {
        CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
        WXMultiColumnLayout *layout = (WXMultiColumnLayout *)_collectionViewlayout;
        BOOL needUpdateLayout = NO;
        if (attributes[@"columnWidth"]) {
            layout.columnWidth = [WXConvert WXLength:attributes[@"columnWidth"] isFloat:YES scaleFactor:scaleFactor];
            needUpdateLayout = YES;
        }
        
        if (attributes[@"columnCount"]) {
            layout.columnCount = [WXConvert WXLength:attributes[@"columnCount"] isFloat:NO scaleFactor:1.0];
            
            needUpdateLayout = YES;
        }
        if (attributes[@"columnGap"]) {
            layout.columnGap = [self _floatValueForColumnGap:([WXConvert WXLength:attributes[@"columnGap"] isFloat:YES scaleFactor:scaleFactor])];
            needUpdateLayout = YES;
        }
        
        if (needUpdateLayout) {
            for (WXComponent *component in self.subcomponents) {
                [component setNeedsLayout];
            }
            
            [self.collectionView reloadData];
            [self.collectionView.collectionViewLayout invalidateLayout];
        }
    }
    
}

- (void)setContentSize:(CGSize)contentSize
{
    // Do Nothing
}

- (void)adjustSticky
{
    // Do Nothing, sticky is adjusted by layout
}

#pragma mark - Private Subclass Methods

- (void)_updateStylesOnComponentThread:(NSDictionary *)styles resetStyles:(NSMutableArray *)resetStyles isUpdateStyles:(BOOL)isUpdateStyles
{
    [super _updateStylesOnComponentThread:styles resetStyles:resetStyles isUpdateStyles:isUpdateStyles];
    
    [self _fillPadding];
}

- (void)_handleFirstScreenTime
{
    // Do Nothing， firstScreenTime is set by cellDidRendered:
}

- (void)scrollToComponent:(WXComponent *)component withOffset:(CGFloat)offset animated:(BOOL)animated
{
    CGPoint contentOffset = _collectionView.contentOffset;
    CGFloat contentOffsetY = 0;
    
    CGRect rect;
    while (component) {
        if ([component isKindOfClass:[WXCellComponent class]]) {
            NSIndexPath *toIndexPath = [self.dataController indexPathForCell:(WXCellComponent *)component];
            UICollectionViewLayoutAttributes *attributes = [_collectionView layoutAttributesForItemAtIndexPath:toIndexPath];
            rect = attributes.frame;
            break;
        }
        if ([component isKindOfClass:[WXHeaderComponent class]]) {
            NSUInteger toIndex = [self.dataController indexForHeader:(WXHeaderComponent *)component];
            UICollectionViewLayoutAttributes *attributes = [_collectionView layoutAttributesForSupplementaryElementOfKind:kCollectionSupplementaryViewKindHeader atIndexPath:[NSIndexPath indexPathWithIndex:toIndex]];
            rect = attributes.frame;
            break;
        }
        contentOffsetY += component.calculatedFrame.origin.y;
        component = component.supercomponent;
    }
    
    contentOffsetY += rect.origin.y;
    contentOffsetY += offset * self.weexInstance.pixelScaleFactor;
    
    if (_collectionView.contentSize.height >= _collectionView.frame.size.height && contentOffsetY > _collectionView.contentSize.height - _collectionView.frame.size.height) {
        contentOffset.y = _collectionView.contentSize.height - _collectionView.frame.size.height;
    } else {
        contentOffset.y = contentOffsetY;
    }
    
    [_collectionView setContentOffset:contentOffset animated:animated];

}

- (void)performUpdatesWithCompletion:(void (^)(BOOL finished))completion
{
    WXAssertMainThread();
    
    //TODO: support completion
    
    if (![self isViewLoaded]) {
        completion(NO);
    }
    
    NSArray *oldData = [self.dataController.sections copy];
    NSArray *newData = [self _sectionArrayFromComponents:self.subcomponents];
    
    [_updateController performUpdatesWithNewData:newData oldData:oldData view:_collectionView];
}

- (void)_insertSubcomponent:(WXComponent *)subcomponent atIndex:(NSInteger)index
{
    if ([subcomponent isKindOfClass:[WXCellComponent class]]) {
        ((WXCellComponent *)subcomponent).delegate = self;
    } else if ([subcomponent isKindOfClass:[WXHeaderComponent class]]) {
        ((WXHeaderComponent *)subcomponent).delegate = self;
    }
    
    [super _insertSubcomponent:subcomponent atIndex:index];
    
    if (![subcomponent isKindOfClass:[WXHeaderComponent class]]
        && ![subcomponent isKindOfClass:[WXCellComponent class]]) {
        return;
    }
    
    WXPerformBlockOnMainThread(^{
        [self performUpdatesWithCompletion:^(BOOL finished) {
    
        }];
    });
}

- (void)insertSubview:(WXComponent *)subcomponent atIndex:(NSInteger)index
{
    //Here will not insert cell/header/footer's view again
    if (![subcomponent isKindOfClass:[WXCellComponent class]]
        && ![subcomponent isKindOfClass:[WXHeaderComponent class]]
        && ![subcomponent isKindOfClass:[WXFooterComponent class]]) {
        [super insertSubview:subcomponent atIndex:index];
    }
}

#pragma mark - WXRecyclerUpdateControllerDelegate

- (void)updateController:(WXRecyclerUpdateController *)controller willPerformUpdateWithNewData:(NSArray<WXSectionDataController *> *)newData
{
    if (newData) {
        [self.dataController updateData:newData];
    }
}

- (void)updateController:(WXRecyclerUpdateController *)controller didPerformUpdateWithFinished:(BOOL)finished
{
    
}

#pragma mark - UICollectionViewDataSource

- (NSInteger)numberOfSectionsInCollectionView:(UICollectionView *)collectionView
{
    WXLogDebug(@"section number:%zi", [self.dataController numberOfSections]);
    return [self.dataController numberOfSections];
}

- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    NSInteger numberOfItems = [self.dataController numberOfItemsInSection:section];
    
    WXLogDebug(@"Number of items is %ld in section:%ld", numberOfItems, section);
    
    return numberOfItems;
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath
{
    WXLogDebug(@"Getting cell at indexPath:%@", indexPath);
    
    WXCollectionViewCell *cellView = [_collectionView dequeueReusableCellWithReuseIdentifier:kCollectionCellReuseIdentifier forIndexPath:indexPath];
    
    UIView *contentView = [self.dataController cellForItemAtIndexPath:indexPath];
    
    cellView.wx_component = contentView.wx_component;
    
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
    UICollectionReusableView *reusableView = nil;
    if ([kind isEqualToString:kCollectionSupplementaryViewKindHeader]) {
        reusableView = [collectionView dequeueReusableSupplementaryViewOfKind:kind withReuseIdentifier:kCollectionHeaderReuseIdentifier forIndexPath:indexPath];
        UIView *contentView = [self.dataController viewForHeaderAtIndexPath:indexPath];
        if (contentView.superview != reusableView) {
            for (UIView *view in reusableView.subviews) {
                [view removeFromSuperview];
            }

            [reusableView addSubview:contentView];
        }
    }
    
    return reusableView;
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

#pragma mark - WXMultiColumnLayoutDelegate

- (UIEdgeInsets)collectionView:(UICollectionView *)collectionView insetForLayout:(UICollectionViewLayout *)collectionViewLayout
{
    return _padding;
}

- (CGFloat)collectionView:(UICollectionView *)collectionView contentWidthForLayout:(UICollectionViewLayout *)collectionViewLayout
{
    return self.scrollerCSSNode->style.dimensions[CSS_WIDTH];
}

- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout heightForItemAtIndexPath:(NSIndexPath *)indexPath
{
    CGSize itemSize = [self.dataController sizeForItemAtIndexPath:indexPath];
    return itemSize.height;
}

- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout heightForHeaderInSection:(NSInteger)section
{
    CGSize headerSize = [self.dataController sizeForHeaderAtIndexPath:[NSIndexPath indexPathForItem:0 inSection:section]];
    return headerSize.height;
}

- (BOOL)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout hasHeaderInSection:(NSInteger)section
{
    return [self.dataController hasHeaderInSection:section];
}

- (CGFloat)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout isNeedStickyForHeaderInSection:(NSInteger)section
{
    return [self.dataController isStickyForHeaderAtIndexPath:[NSIndexPath indexPathForItem:0 inSection:section]];
}

#pragma mark - WXHeaderRenderDelegate

- (float)headerWidthForLayout:(WXHeaderComponent *)header
{
    if (_layoutType == WXRecyclerLayoutTypeMultiColumn) {
        return ((WXMultiColumnLayout *)_collectionViewlayout).computedHeaderWidth;
    }
    
    return 0.0;
}

- (void)headerDidLayout:(WXHeaderComponent *)header
{
    WXPerformBlockOnMainThread(^{
        [self.collectionView.collectionViewLayout invalidateLayout];
    });
}

- (void)headerDidRemove:(WXHeaderComponent *)header
{
    WXPerformBlockOnMainThread(^{
        [self performUpdatesWithCompletion:^(BOOL finished) {
            
        }];
    });
}

#pragma mark - WXCellRenderDelegate

- (float)containerWidthForLayout:(WXCellComponent *)cell
{
    if (_layoutType == WXRecyclerLayoutTypeMultiColumn) {
        return ((WXMultiColumnLayout *)_collectionViewlayout).computedColumnWidth;
    }
    
    return 0.0;
}

- (void)cellDidLayout:(WXCellComponent *)cell
{
    BOOL previousLayoutComplete = cell.isLayoutComplete;
    cell.isLayoutComplete = YES;
    WXPerformBlockOnMainThread(^{
        if (previousLayoutComplete) {
            [self.updateController reloadItemsAtIndexPath:[self.dataController indexPathForCell:cell]];
        } else {
            [self performUpdatesWithCompletion:^(BOOL finished) {
            }];
        }
    });
}

- (void)cellDidRendered:(WXCellComponent *)cell
{
    if (WX_MONITOR_INSTANCE_PERF_IS_RECORDED(WXPTFirstScreenRender, self.weexInstance) && !self.weexInstance.onRenderProgress) {
        return;
    }
    
    NSIndexPath *indexPath = [self.dataController indexPathForCell:cell];
    
    UICollectionViewLayoutAttributes *attributes = [self.collectionView layoutAttributesForItemAtIndexPath:indexPath];
    CGRect cellRect = attributes.frame;
    if (cellRect.origin.y + cellRect.size.height >= _collectionView.frame.size.height) {
        WX_MONITOR_INSTANCE_PERF_END(WXPTFirstScreenRender, self.weexInstance);
    }
    
    if (self.weexInstance.onRenderProgress) {
        CGRect renderRect = [_collectionView convertRect:cellRect toView:self.weexInstance.rootView];
        self.weexInstance.onRenderProgress(renderRect);
    }
}

- (void)cellDidRemove:(WXCellComponent *)cell
{
    if (cell.isLayoutComplete) {
        WXPerformBlockOnMainThread(^{
            [self performUpdatesWithCompletion:^(BOOL finished) {
            }];
        });
    }
}

- (void)cell:(WXCellComponent *)cell didMoveToIndex:(NSUInteger)index
{
    if (cell.isLayoutComplete) {
        WXPerformBlockOnMainThread(^{
            [self performUpdatesWithCompletion:^(BOOL finished) {
            }];
        });
    }
}

#pragma mark - Load More Event

- (void)setLoadmoreretry:(NSUInteger)loadmoreretry
{
    if (loadmoreretry != self.loadmoreretry) {
        _previousLoadMoreCellNumber = 0;
    }
    
    [super setLoadmoreretry:loadmoreretry];
}

- (void)loadMore
{
    [super loadMore];
    
    _previousLoadMoreCellNumber = [self totalNumberOfCells];
}

- (BOOL)isNeedLoadMore
{
    BOOL superNeedLoadMore = [super isNeedLoadMore];
    return superNeedLoadMore && _previousLoadMoreCellNumber != [self totalNumberOfCells];
}

- (NSUInteger)totalNumberOfCells
{
    NSUInteger cellNumber = 0;
    NSUInteger sectionCount = [_collectionView numberOfSections];
    for (int section = 0; section < sectionCount; section ++) {
        cellNumber += [_collectionView numberOfItemsInSection:section];
    }
    
    return cellNumber;
}

- (void)resetLoadmore{
    [super resetLoadmore];
    _previousLoadMoreCellNumber = 0;
}

#pragma mark - Private

- (float)_floatValueForColumnGap:(WXLength *)gap
{
    if (gap.isNormal) {
        return kRecyclerNormalColumnGap * self.weexInstance.pixelScaleFactor;
    } else {
        return gap.floatValue;
    }
}

- (void)_fillPadding
{
    UIEdgeInsets padding = {
        WXFloorPixelValue(self.cssNode->style.padding[CSS_TOP] + self.cssNode->style.border[CSS_TOP]),
        WXFloorPixelValue(self.cssNode->style.padding[CSS_LEFT] + self.cssNode->style.border[CSS_LEFT]),
        WXFloorPixelValue(self.cssNode->style.padding[CSS_BOTTOM] + self.cssNode->style.border[CSS_BOTTOM]),
        WXFloorPixelValue(self.cssNode->style.padding[CSS_RIGHT] + self.cssNode->style.border[CSS_RIGHT])
    };
    
    if (!UIEdgeInsetsEqualToEdgeInsets(padding, _padding)) {
        _padding = padding;
        [self setNeedsLayout];
        
        for (WXComponent *component in self.subcomponents) {
            [component setNeedsLayout];
        }
        
        if (_collectionView) {
            WXPerformBlockOnMainThread(^{
                [_collectionView.collectionViewLayout invalidateLayout];
            });
        }
    }
}

- (NSArray<WXSectionDataController *> *)_sectionArrayFromComponents:(NSArray<WXComponent *> *)components
{
    NSMutableArray<WXSectionDataController *> *sectionArray = [NSMutableArray array];
    NSMutableArray<WXCellComponent *> *cellArray = [NSMutableArray array];
    WXSectionDataController *currentSection;
    
    for (int i = 0; i < components.count; i++) {
        if (!currentSection) {
            currentSection = [WXSectionDataController new];
        }
        
        WXComponent* component = components[i];
        
        if ([component isKindOfClass:[WXHeaderComponent class]]) {
            if (i != 0 && (currentSection.headerComponent || cellArray.count > 0)) {
                currentSection.cellComponents = [cellArray copy];
                [sectionArray addObject:currentSection];
                currentSection = [WXSectionDataController new];
                [cellArray removeAllObjects];
            }
            currentSection.headerComponent = (WXHeaderComponent *)component;
        } else if ([component isKindOfClass:[WXCellComponent class]]
                   && ((WXCellComponent *)component).isLayoutComplete) {
            [cellArray addObject:(WXCellComponent *)component];
        } else if ([component isKindOfClass:[WXFooterComponent class]]) {
            currentSection.footerComponent = component;
        } else {
            continue;
        }
    }
    
    if (cellArray.count > 0 || currentSection.headerComponent) {
        currentSection.cellComponents = [cellArray copy];
        [sectionArray addObject:currentSection];
    }
    
    return sectionArray;
}

- (void)fixFlicker
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // FIXME:(ง •̀_•́)ง┻━┻ Stupid scoll view, always reset content offset to zero by calling _adjustContentOffsetIfNecessary after insert cells.
        // So if you pull down list while list is rendering, the list will be flickering.
        // Demo:
        // Have to hook _adjustContentOffsetIfNecessary here.
        // Any other more elegant way?
        NSString *a = @"ntOffsetIfNe";
        NSString *b = @"adjustConte";
        
        NSString *originSelector = [NSString stringWithFormat:@"_%@%@cessary", b, a];
        [[self class] weex_swizzle:[WXCollectionView class] Method:NSSelectorFromString(originSelector) withMethod:@selector(fixedFlickerSelector)];
    });
}

- (void)fixedFlickerSelector
{
    // DO NOT delete this method.
}

@end
