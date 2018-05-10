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

#ifndef WXPullRefreshProtocol_h
#define WXPullRefreshProtocol_h

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@protocol WXPullRefreshProtocol <NSObject>
@optional

/**
 * @abstract startPulling means when user start dragging, you can add subview to inSuperView to display customized view
 */
- (void)startPulling:(UIView *)inSuperView;

/**
 * @abstract pulling means when is pulling, inParams contains the distance information
 */
- (void)pulling:(NSDictionary *)inParams;

/**
 * @abstract endPulling will be triggered when user stop dragging
 */
- (void)endPulling;

/**
 * @abstract startRefreshing means when user releases dragging and waiting for network response, you can start an animation for loading
 */
- (void)startRefreshing:(UIView *)inSuperView;

/**
 * @abstract endRefreshing will be triggered when network response comes
 */
- (void)endRefreshing;

/**
 * @abstract startFinalAnimation means when network response comes, show an final animation to let user know the refresh state,
 *  the duration can be customized by js file
 */
- (void)startFinalAnimation:(UIView *)inSuperView duration:(CGFloat)inDuration;

/**
 * @abstract endFinalAnimation will be triggered when final animation finished
 */
- (void)endFinalAnimation;
@end

#endif /* WXPullRefreshProtocol_h */
