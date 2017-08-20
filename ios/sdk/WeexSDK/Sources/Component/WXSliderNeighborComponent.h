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
@class WXIndicatorView;

@protocol WXSliderNeighborDelegate <NSObject>
@optional

- (void)sliderNeighborWillBeginScrollingAnimation:(WXSliderNeighborView * _Nonnull )sliderNeighbor;
- (void)sliderNeighborDidEndScrollingAnimation:(WXSliderNeighborView *_Nonnull)sliderNeighbor;
- (void)sliderNeighborDidScroll:(WXSliderNeighborView * _Nonnull)sliderNeighbor;
- (void)sliderNeighborCurrentItemIndexDidChange:(WXSliderNeighborView * _Nonnull)sliderNeighbor from:(NSInteger) from to:(NSInteger)to;
- (void)sliderNeighborWillBeginDragging:(WXSliderNeighborView * _Nonnull)sliderNeighbor;
- (void)sliderNeighborDidEndDragging:(WXSliderNeighborView * _Nonnull)sliderNeighbor willDecelerate:(BOOL)decelerate;
- (void)sliderNeighborWillBeginDecelerating:(WXSliderNeighborView * _Nonnull)sliderNeighbor;
- (void)sliderNeighborDidEndDecelerating:(WXSliderNeighborView * _Nonnull)sliderNeighbor;

- (BOOL)sliderNeighbor:(WXSliderNeighborView * _Nonnull)sliderNeighbor shouldSelectItemAtIndex:(NSInteger)index;
- (void)sliderNeighbor:(WXSliderNeighborView * _Nonnull)sliderNeighbor didSelectItemAtIndex:(NSInteger)index;
- (void)sliderNeighbor:(WXSliderNeighborView * _Nonnull)sliderNeighbor didScrollToItemAtIndex:(NSInteger)index;

- (CGFloat)sliderNeighborItemWidth:(WXSliderNeighborView * _Nonnull)sliderNeighbor;
- (CGFloat)sliderNeighbor:(WXSliderNeighborView * _Nonnull)sliderNeighbor valueForOption:(WXSliderNeighborOption)option withDefault:(CGFloat)value;

@end

@protocol WXSliderNeighborDataSource <NSObject>

- (NSInteger)numberOfItemsInSliderNeighbor:(WXSliderNeighborView * _Nonnull)sliderNeighbor;
- (UIView * _Nonnull)sliderNeighbor:(WXSliderNeighborView * _Nonnull)sliderNeighbor viewForItemAtIndex:(NSInteger)index reusingView:( UIView * _Nonnull)view;

@optional

- (NSInteger)numberOfPlaceholdersInsliderNeighbor:(WXSliderNeighborView * _Nonnull)sliderNeighbor;
- (UIView * _Nonnull)sliderNeighbor:(WXSliderNeighborView * _Nonnull)sliderNeighbor placeholderViewAtIndex:(NSInteger)index reusingView:( UIView * _Nonnull)view;

@end

@interface WXSliderNeighborComponent : WXComponent<WXSliderNeighborDelegate, WXSliderNeighborDataSource>

- (void)setIndicatorView:(WXIndicatorView *_Nonnull)indicatorView;

@end

