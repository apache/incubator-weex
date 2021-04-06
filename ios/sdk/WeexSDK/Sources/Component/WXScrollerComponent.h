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

#import <WeexSDK/WXScrollerProtocol.h>
#import <WeexSDK/WXComponent.h>

NS_ASSUME_NONNULL_BEGIN

@class WXScrollSnapData;

@interface WXScrollerComponent : WXComponent <WXScrollerProtocol, UIScrollViewDelegate>

@property (nonatomic, copy) void (^onScroll)(UIScrollView *scrollView);

@property (nonatomic, copy) void (^scrollEventListener)(WXScrollerComponent* sender, NSString* event, NSDictionary* _Nullable params);

@property (nonatomic, assign) NSUInteger loadmoreretry;

@property (nonatomic, assign) CGSize contentSize;

@property (nonatomic, strong) WXScrollSnapData *snapData;

- (void)handleAppear;

- (CGPoint)absolutePositionForComponent:(WXComponent *)component;

@end

typedef NS_ENUM(NSUInteger, WXScrollSnapStatus) {
    WXScrollSnapToNext,
    WXScrollSnapToPrev,
    WXScrollSnapStay,
    WXScrollSnapNone,
};

typedef NS_ENUM(NSUInteger, WXScrollSnapAlignment) {
    WXScrollSnapAlignStart,
    WXScrollSnapAlignCenter,
    WXScrollSnapAlignEnd,
    WXScrollSnapAlignNone,
};

@interface WXScrollSnapData : NSObject
/// snap开关
@property (nonatomic, assign) BOOL useSnap;
/// 主轴方向
@property (nonatomic, assign) WXScrollDirection axis;
/// 对齐位置
@property (nonatomic, assign) WXScrollSnapAlignment alignment;
/// snap padding
@property (nonatomic, assign) UIEdgeInsets padding;
/// 目标位置
@property (nonatomic, strong) NSIndexPath *targetIndexPath;
/// 绑定的scrollView
@property (nonatomic, weak) UIScrollView *scrollView;
/// 起始位置
@property (nonatomic, assign) CGPoint startPosition;
/// 目标位置
@property (nonatomic, assign) CGPoint targetPosition;
/// 是否正在snapping
@property (nonatomic, assign) BOOL snapping;

/// 绑定scrollView，更新分页属性和减速度
- (void)bindingScrollView:(UIScrollView *)scrollView;
/// 计算snap staus;
- (WXScrollSnapStatus)shouldTriggerSnap:(CGPoint)offset velocity:(CGPoint)velocity;
/// 
- (CGFloat)calcScrollSnapPositionOffset;

@end

NS_ASSUME_NONNULL_END
