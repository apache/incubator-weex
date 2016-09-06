//
//  WXSliderNeighbor.h
//  WeexSDK
//
//  Created by zifan.zx on 8/30/16.
//  Copyright © 2016 taobao. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WXComponent.h"

typedef NS_ENUM(NSInteger, WXSliderNeighborOption)
{
    WXSliderNeighborOptionWrap = 0,
    WXSliderNeighborOptionShowBackfaces,
    WXSliderNeighborOptionOffsetMultiplier,
    WXSliderNeighborOptionVisibleItems,
    WXSliderNeighborOptionCount,
    WXSliderNeighborOptionArc,
    WXSliderNeighborOptionAngle,
    WXSliderNeighborOptionRadius,
    WXSliderNeighborOptionTilt,
    WXSliderNeighborOptionSpacing,
    WXSliderNeighborOptionFadeMin,
    WXSliderNeighborOptionFadeMax,
    WXSliderNeighborOptionFadeRange,
    WXSliderNeighborOptionFadeMinAlpha
};

@class WXSliderNeighborView;

@protocol WXSliderNeighborDelegate <NSObject>
@optional

- (void)sliderNeighborWillBeginScrollingAnimation:(WXSliderNeighborView *)sliderNeighbor;
- (void)sliderNeighborDidEndScrollingAnimation:(WXSliderNeighborView *)sliderNeighbor;
- (void)sliderNeighborDidScroll:(WXSliderNeighborView *)sliderNeighbor;
- (void)sliderNeighborCurrentItemIndexDidChange:(WXSliderNeighborView *)sliderNeighbor;
- (void)sliderNeighborWillBeginDragging:(WXSliderNeighborView *)sliderNeighbor;
- (void)sliderNeighborDidEndDragging:(WXSliderNeighborView *)sliderNeighbor willDecelerate:(BOOL)decelerate;
- (void)sliderNeighborWillBeginDecelerating:(WXSliderNeighborView *)sliderNeighbor;
- (void)sliderNeighborDidEndDecelerating:(WXSliderNeighborView *)sliderNeighbor;

- (BOOL)sliderNeighbor:(WXSliderNeighborView *)sliderNeighbor shouldSelectItemAtIndex:(NSInteger)index;
- (void)sliderNeighbor:(WXSliderNeighborView *)sliderNeighbor didSelectItemAtIndex:(NSInteger)index;
- (void)sliderNeighbor:(WXSliderNeighborView *)sliderNeighbor didScrollToItemAtIndex:(NSInteger)index;

- (CGFloat)sliderNeighborItemWidth:(WXSliderNeighborView *)sliderNeighbor;
- (CATransform3D)sliderNeighbor:(WXSliderNeighborView *)sliderNeighbor itemTransformForOffset:(CGFloat)offset baseTransform:(CATransform3D)transform;
- (CGFloat)sliderNeighbor:(WXSliderNeighborView *)sliderNeighbor valueForOption:(WXSliderNeighborOption)option withDefault:(CGFloat)value;

@end

@protocol WXSliderNeighborDataSource <NSObject>

- (NSInteger)numberOfItemsInSliderNeighbor:(WXSliderNeighborView *)sliderNeighbor;
- (UIView *)sliderNeighbor:(WXSliderNeighborView *)sliderNeighbor viewForItemAtIndex:(NSInteger)index reusingView:(nullable UIView *)view;

@optional

- (NSInteger)numberOfPlaceholdersInsliderNeighbor:(WXSliderNeighborView *)sliderNeighbor;
- (UIView *)sliderNeighbor:(WXSliderNeighborView *)sliderNeighbor placeholderViewAtIndex:(NSInteger)index reusingView:(nullable UIView *)view;

@end

@interface WXSliderNeighborComponent : WXComponent<WXSliderNeighborDelegate, WXSliderNeighborDataSource>

@end

