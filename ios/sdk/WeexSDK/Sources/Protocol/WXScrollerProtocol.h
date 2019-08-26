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

#import <WeexSDK/WXType.h>

NS_ASSUME_NONNULL_BEGIN

@class WXComponent;
@protocol WXScrollerProtocol <NSObject>

/**
 * @abstract add sticky component
 */
- (void)addStickyComponent:(WXComponent *)sticky;

/**
 * @abstract remove sticky component
 */
- (void)removeStickyComponent:(WXComponent *)sticky;

/**
 * @abstract adjust sticky components
 */
- (void)adjustSticky;

/**
 * @abstract add scroll listener
 */
- (void)addScrollToListener:(WXComponent *)target;

/**
 * @abstract remove scroll listener
 */
- (void)removeScrollToListener:(WXComponent *)target;

- (void)scrollToComponent:(WXComponent *)component withOffset:(CGFloat)offset animated:(BOOL)animated;

- (BOOL)isNeedLoadMore;

- (void)loadMore;

- (CGPoint)contentOffset;

- (void)setContentOffset:(CGPoint)contentOffset;
- (void)setContentOffset:(CGPoint)contentOffset animated:(BOOL)animated;

- (CGSize)contentSize;

- (void)setContentSize:(CGSize)size;

- (UIEdgeInsets)contentInset;

- (void)setContentInset:(UIEdgeInsets)contentInset;

- (void)resetLoadmore;

- (void)addScrollDelegate:(id<UIScrollViewDelegate>)delegate;
- (void)removeScrollDelegate:(id<UIScrollViewDelegate>)delegate;

- (WXScrollDirection)scrollDirection;

@optional

- (NSString*)refreshType;
- (BOOL)requestGestureShouldStopPropagation:(UIGestureRecognizer *)gestureRecognizer shouldReceiveTouch:(UITouch *)touch;
/**
 * @abstract adjust for RTL
 */
- (void)adjustForRTL;

@end

NS_ASSUME_NONNULL_END
