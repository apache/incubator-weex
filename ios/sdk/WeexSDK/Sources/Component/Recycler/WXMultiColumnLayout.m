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

#import "WXMultiColumnLayout.h"
#import "NSArray+Weex.h"
#import "WXUtility.h"
#import "WXAssert.h"

NSString * const kCollectionSupplementaryViewKindHeader = @"WXCollectionSupplementaryViewKindHeader";
NSString * const kMultiColumnLayoutHeader = @"WXMultiColumnLayoutHeader";
NSString * const kMultiColumnLayoutCell = @"WXMultiColumnLayoutCell";

@interface WXMultiColumnLayoutHeaderAttributes : UICollectionViewLayoutAttributes

@property (nonatomic, assign) BOOL isSticky;

@end

@implementation WXMultiColumnLayoutHeaderAttributes

- (id)copyWithZone:(NSZone *)zone
{
    WXMultiColumnLayoutHeaderAttributes *copy = [super copyWithZone:zone];
    copy.isSticky = self.isSticky;
    
    return copy;
}

@end

@interface WXMultiColumnLayout ()

@property (nonatomic, strong) NSMutableDictionary<NSString *, NSDictionary<id, UICollectionViewLayoutAttributes *> *> *layoutAttributes;
@property (nonatomic, strong) NSMutableArray<NSNumber *> *columnsMaxHeights;

@end

@implementation WXMultiColumnLayout

- (instancetype)init
{
    if (self = [super init]) {
        _layoutAttributes = [NSMutableDictionary dictionary];
        _columnsMaxHeights = [NSMutableArray array];
    }
    
    return self;
}

//Under system version 10.0, UICollectionViewLayout.collectionView seems be unsafe_unretain rather than weak. sometime when the collectionView is released, and the layout is not released, it may crash.
- (UICollectionView *)weakCollectionView
{
    if ([[[UIDevice currentDevice] systemVersion] floatValue]<10.0f) {
        return self.weak_collectionView;
    } else {
        return self.collectionView;
    }
}

#pragma mark - Public Accessors

- (void)setColumnCount:(WXLength *)columnCount
{
    if (!(columnCount.isAuto && _columnCount.isAuto) || _columnCount.intValue != columnCount.intValue) {
        _columnCount = columnCount;
        [self _cleanComputed];
    }
}

- (void)setColumnWidth:(WXLength *)columnWidth
{
    if (!(columnWidth.isAuto && _columnWidth.isAuto) || _columnWidth.floatValue != columnWidth.floatValue) {
        _columnWidth = columnWidth;
        [self _cleanComputed];
    }
}

- (void)setColumnGap:(float)columnGap
{
    if (_columnGap != columnGap) {
        _columnGap = columnGap;
        [self _cleanComputed];
    }
}

- (void)setLeftGap:(float)leftGap
{
    if (_leftGap != leftGap) {
        _leftGap = leftGap;
        [self _cleanComputed];
    }
}

- (void)setRightGap:(float)rightGap
{
    if (_rightGap != rightGap) {
        _rightGap = rightGap;
        [self _cleanComputed];
    }
}

- (CGFloat)computedColumnWidth
{
    WXAssert([_columnWidth isFixed], @"column width must be calculated by core.");
    return _columnWidth.floatValue;
}

- (int)computedColumnCount
{
    WXAssert([_columnCount isFixed], @"column count must be calculated by core.");
    return _columnCount.intValue;
}

- (CGFloat)computedHeaderWidth
{
    UIEdgeInsets insets = [self.delegate collectionView:[self weakCollectionView] insetForLayout:self];
    return self.contentWidth - (insets.left + insets.right);
}

#pragma mark - Methods to Override for UICollectionViewLayout

- (void)prepareLayout
{
    [super prepareLayout];
    
    [self _cleanup];
    
    NSInteger numberOfSections = [[self weakCollectionView]  numberOfSections];
    UIEdgeInsets insets = [self.delegate collectionView:[self weakCollectionView]  insetForLayout:self];
    
    float columnWidth = self.computedColumnWidth;
    int columnCount = self.computedColumnCount;
    float columnGap = self.columnGap;
    
    CGFloat currentHeight = insets.top;
    NSMutableDictionary *headersAttributes = [NSMutableDictionary dictionaryWithCapacity:numberOfSections];
    NSMutableDictionary *cellAttributes = [NSMutableDictionary dictionary];
    for (NSInteger i = 0; i < columnCount; i++) {
        [self.columnsMaxHeights addObject:@(currentHeight)];
    }
    
    for (NSInteger section = 0; section < numberOfSections; section++) {
        BOOL hasHeader = [self.delegate collectionView:[self weakCollectionView]  layout:self hasHeaderInSection:section];
        // header
        if (hasHeader) {
            CGFloat headerHeight = [self.delegate collectionView:[self weakCollectionView]  layout:self heightForHeaderInSection:section];
            WXMultiColumnLayoutHeaderAttributes *headerAttributes = [WXMultiColumnLayoutHeaderAttributes layoutAttributesForSupplementaryViewOfKind:kCollectionSupplementaryViewKindHeader withIndexPath:[NSIndexPath indexPathForItem:0 inSection:section]];
            headerAttributes.frame = CGRectMake(insets.left, currentHeight, self.contentWidth - (insets.left + insets.right), headerHeight);
            headerAttributes.isSticky = [self.delegate collectionView:[self weakCollectionView] layout:self isNeedStickyForHeaderInSection:section];
            headerAttributes.zIndex = headerAttributes.isSticky ? 1 : 0;
            headersAttributes[@(section)] = headerAttributes;
            
            currentHeight = CGRectGetMaxY(headerAttributes.frame);
            [self _columnsReachToHeight:currentHeight];
        }
        
        // cells
        
        @try {
            for (NSInteger item = 0; item < [[self weakCollectionView] numberOfItemsInSection:section]; item++) {
                NSIndexPath *indexPath = [NSIndexPath indexPathForItem:item inSection:section];
                CGFloat itemHeight = [self.delegate collectionView:[self weakCollectionView] layout:self heightForItemAtIndexPath:indexPath];
                UICollectionViewLayoutAttributes *itemAttributes = [UICollectionViewLayoutAttributes layoutAttributesForCellWithIndexPath:indexPath];
                NSUInteger column = [self _minHeightColumnForAllColumns];
                CGFloat x = insets.left + (columnWidth + columnGap) * column+_leftGap;
                if (column >= [self.columnsMaxHeights count]) {
                    return;
                }
                CGFloat y = [self.columnsMaxHeights[column] floatValue];
                itemAttributes.frame = CGRectMake(x, y, columnWidth, itemHeight);
                cellAttributes[indexPath] = itemAttributes;
                
                self.columnsMaxHeights[column] = @(CGRectGetMaxY(itemAttributes.frame));
            }
        } @catch (NSException *exception) {
            WXLog(@"%@", exception);
        }
        currentHeight = [self _maxHeightForAllColumns];
        [self _columnsReachToHeight:currentHeight];
    }
    
    currentHeight = currentHeight + insets.bottom;
    [self _columnsReachToHeight:currentHeight];
    
    self.layoutAttributes[kMultiColumnLayoutHeader] = headersAttributes;
    self.layoutAttributes[kMultiColumnLayoutCell] = cellAttributes;
}

- (CGSize)collectionViewContentSize
{
    NSInteger numberOfSections = [[self weakCollectionView] numberOfSections];
    if (numberOfSections == 0) {
        return CGSizeZero;
    }
    
    return CGSizeMake(self.contentWidth, self.contentHeight);
}

- (NSArray<UICollectionViewLayoutAttributes *> *)layoutAttributesForElementsInRect:(CGRect)rect
{
    NSMutableArray<WXMultiColumnLayoutHeaderAttributes *> *stickyHeaders = [NSMutableArray array];
    NSMutableArray<UICollectionViewLayoutAttributes *> *result = [NSMutableArray array];
    
    [self.layoutAttributes enumerateKeysAndObjectsUsingBlock:^(NSString * _Nonnull kind, NSDictionary<id,UICollectionViewLayoutAttributes *> * _Nonnull dictionary, BOOL * _Nonnull stop) {
        [dictionary enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, UICollectionViewLayoutAttributes * _Nonnull attributes, BOOL * _Nonnull stop) {
            if (attributes.representedElementKind == kCollectionSupplementaryViewKindHeader
                && [self.delegate collectionView:[self weakCollectionView] layout:self isNeedStickyForHeaderInSection:attributes.indexPath.section]) {
                [stickyHeaders addObject:(WXMultiColumnLayoutHeaderAttributes *)attributes];
            } else if (CGRectIntersectsRect(rect, attributes.frame)) {
                [result addObject:attributes];
            }
        }];
    }];
    
    [stickyHeaders sortUsingComparator:^NSComparisonResult(WXMultiColumnLayoutHeaderAttributes *obj1, WXMultiColumnLayoutHeaderAttributes *obj2) {
        if (obj1.indexPath.section < obj2.indexPath.section) {
            return NSOrderedAscending;
        } else {
            return NSOrderedDescending;
        }
    }];
    
    for (int i = 0; i < stickyHeaders.count; i++) {
        WXMultiColumnLayoutHeaderAttributes *header = stickyHeaders[i];
        [self _adjustStickyForHeaderAttributes:header next:(i == stickyHeaders.count - 1) ? nil : stickyHeaders[i + 1]];
        [result addObject:header];
    }
    
    WXLogDebug(@"return result attributes:%@ for rect:%@", result, NSStringFromCGRect(rect));
    
    return result;
}

- (void)_adjustStickyForHeaderAttributes:(WXMultiColumnLayoutHeaderAttributes *)header
                                   next:(WXMultiColumnLayoutHeaderAttributes *)nextHeader
{
    CGRect bounds = [self weakCollectionView].bounds;
    CGFloat originY = header.frame.origin.y;
    CGFloat maxY = nextHeader ? (nextHeader.frame.origin.y - header.frame.size.height) : (CGRectGetMaxY(bounds) - header.frame.size.height);
    CGFloat currentY = CGRectGetMaxY(bounds) - bounds.size.height + [self weakCollectionView].contentInset.top;
    
    CGFloat resultY = MIN(MAX(currentY, originY), maxY);
    CGPoint origin = header.frame.origin;
    origin.y = resultY;
    
    header.frame = (CGRect){origin, header.frame.size};
    header.hidden = NO;
}

- (UICollectionViewLayoutAttributes *)layoutAttributesForSupplementaryViewOfKind:(NSString *)elementKind atIndexPath:(NSIndexPath *)indexPath
{
    if ([elementKind isEqualToString:kCollectionSupplementaryViewKindHeader]) {
        UICollectionViewLayoutAttributes *attributes = self.layoutAttributes[kMultiColumnLayoutHeader][@(indexPath.section)];
        if (!attributes) {
            attributes = [UICollectionViewLayoutAttributes layoutAttributesForSupplementaryViewOfKind:elementKind withIndexPath:indexPath];
            attributes.frame = CGRectZero;
            attributes.hidden = YES;
        }
        WXLogDebug(@"return header attributes:%@ for index path:%@", attributes, indexPath);
        
        return attributes;
    }
    
    return nil;
}

- (UICollectionViewLayoutAttributes *)layoutAttributesForItemAtIndexPath:(NSIndexPath *)indexPath
{
    if (self.layoutAttributes.count == 0) {
        [self prepareLayout];
    }
    
    UICollectionViewLayoutAttributes *attributes = self.layoutAttributes[kMultiColumnLayoutCell][indexPath];
    WXLogDebug(@"return item attributes:%@ for index path:%@", attributes, indexPath);
    return attributes;
}

- (BOOL)shouldInvalidateLayoutForBoundsChange:(CGRect)newBounds
{
    __block BOOL hasStickyHeader = NO;
    [self.layoutAttributes[kMultiColumnLayoutHeader] enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, UICollectionViewLayoutAttributes * _Nonnull obj, BOOL * _Nonnull stop) {
        WXMultiColumnLayoutHeaderAttributes *attribute = (WXMultiColumnLayoutHeaderAttributes *)obj;
        if (attribute.isSticky) {
            hasStickyHeader = YES;
            *stop = YES;
        }
    }];
    
    if (hasStickyHeader) {
        // always return yes no trigger resetting sticky header's frame.
        return YES;
    } else {
        CGRect oldBounds = [self weakCollectionView].bounds;
        if (CGRectGetWidth(newBounds) != CGRectGetWidth(oldBounds)) {
            return YES;
        }
    }
    
    return NO;
}

#pragma mark - Private

- (CGFloat)contentWidth
{
    return [self.delegate collectionView:[self weakCollectionView] contentWidthForLayout:self];
}

- (CGFloat)contentHeight
{
    return [self _maxHeightForAllColumns];
}

- (CGFloat)_maxHeightForAllColumns
{
    CGFloat maxHeight = 0.0;
    for (NSNumber *number in self.columnsMaxHeights) {
        CGFloat height = [number floatValue];
        if (height > maxHeight) {
            maxHeight = height;
        }
    }
    
    return maxHeight;
}

- (NSUInteger)_minHeightColumnForAllColumns
{
    __block NSUInteger index = 0;
    __block CGFloat minHeight = FLT_MAX;
    
    [self.columnsMaxHeights enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        CGFloat height = [obj floatValue];
        if (height < minHeight) {
            minHeight = height;
            index = idx;
        }
    }];
    
    return index;
}

- (void)_columnsReachToHeight:(CGFloat)height
{
    for (NSInteger i = 0; i < self.columnsMaxHeights.count; i ++) {
        self.columnsMaxHeights[i] = @(height);
    }
}

- (void)_cleanup
{
    [self.layoutAttributes removeAllObjects];
    [self.columnsMaxHeights removeAllObjects];
}

- (void)_cleanComputed
{
}

- (void)invalidateLayout
{
    [super invalidateLayout];
    
    [self _cleanComputed];
}

@end
