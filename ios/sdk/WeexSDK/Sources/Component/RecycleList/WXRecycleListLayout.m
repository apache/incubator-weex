//
//  WXRecycleListLinearLayout.m
//  WeexSDK
//
//  Created by yinfeng on 2017/9/19.
//  Copyright © 2017年 taobao. All rights reserved.
//

#import "WXRecycleListLayout.h"

@implementation WXRecycleListLayout
{
    NSMutableDictionary<NSNumber *, UICollectionViewLayoutAttributes *> *_stickyCellsAttributes;
}

- (NSArray *)layoutAttributesForElementsInRect:(CGRect)rect
{
    NSArray *cellAttributes = [super layoutAttributesForElementsInRect:rect];
    NSMutableDictionary *lastCellsAttributes = [NSMutableDictionary dictionary];
    
    __block NSInteger currentStickyIndex = -1;
    [cellAttributes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        UICollectionViewLayoutAttributes *attributes = obj;
        NSIndexPath *indexPath = attributes.indexPath;
        if ([self.delegate collectionView:self.collectionView layout:self isNeedStickyForIndexPath:indexPath]) {
            if (!_stickyCellsAttributes) {
                _stickyCellsAttributes = [NSMutableDictionary dictionary];
            }
            
            currentStickyIndex = indexPath.item;
            [_stickyCellsAttributes setObject:attributes forKey:@(indexPath.item)];
        } else {
            [_stickyCellsAttributes removeObjectForKey:@(indexPath.item)];
            
            // bottom cell above sticky cell
            UICollectionViewLayoutAttributes *currentLastCell = [lastCellsAttributes objectForKey:@(currentStickyIndex)];
            if (!currentLastCell || indexPath.item > currentLastCell.indexPath.item) {
                [lastCellsAttributes setObject:obj forKey:@(currentStickyIndex)];
            }
        }
        
        attributes.zIndex = 1;
    }];
    
    NSMutableArray *newCellAttributes = [cellAttributes mutableCopy];
    [lastCellsAttributes enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        UICollectionViewLayoutAttributes *attributes = obj;
        
        UICollectionViewLayoutAttributes *stickyCell = _stickyCellsAttributes[key];
        if (!stickyCell) {
            NSInteger item = attributes.indexPath.item;
            while (item >= 0) {
                if (_stickyCellsAttributes[@(item)]) {
                    stickyCell = [self.collectionView layoutAttributesForItemAtIndexPath:[NSIndexPath indexPathForItem:item inSection:0]];
                    break;
                } else {
                    item --;
                }
            }
        }
        
        if (stickyCell) {
            [newCellAttributes addObject:stickyCell];
            [self _adjustStickyForCellAttributes:stickyCell lastCellAttributes:attributes];
        }
    }];
    
    return newCellAttributes;
}

- (void)_adjustStickyForCellAttributes:(UICollectionViewLayoutAttributes *)cell
                    lastCellAttributes:(UICollectionViewLayoutAttributes *)lastCell
{
    cell.zIndex = 99;
    cell.hidden = NO;
    
    CGFloat maxY = CGRectGetMaxY(lastCell.frame) - cell.frame.size.height;
    CGFloat minY = CGRectGetMinY(self.collectionView.bounds) + self.collectionView.contentInset.top;
    CGFloat y = MIN(MAX(minY, cell.frame.origin.y), maxY);

//    NSLog(@"%zi : %zi, %.1f, %.1f, %.1f, %.1f", cell.indexPath.item, lastCell.indexPath.item, maxY, minY, cell.frame.origin.y, y);
    
    CGPoint origin = cell.frame.origin;
    origin.y = y;
    
    cell.frame = (CGRect){
        origin,
        cell.frame.size
    };
}

- (BOOL)shouldInvalidateLayoutForBoundsChange:(CGRect)newBounds
{
    if (_stickyCellsAttributes.count > 0) {
        // always return yes to trigger resetting sticky header's frame.
        return YES;
    }
    
    return [super shouldInvalidateLayoutForBoundsChange:newBounds];
}

@end
