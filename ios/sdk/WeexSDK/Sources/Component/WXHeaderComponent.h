/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <WeexSDK/WeexSDK.h>
@class WXHeaderComponent;

@protocol WXHeaderRenderDelegate <NSObject>

- (float)headerWidthForLayout:(WXHeaderComponent *)header;

- (void)headerDidLayout:(WXHeaderComponent *)header;

- (void)headerDidRemove:(WXHeaderComponent *)header;

@end

@interface WXHeaderComponent : WXComponent

@property (nonatomic, weak) id<WXHeaderRenderDelegate> delegate;
@property (nonatomic, assign, readonly) BOOL isSticky;
@property (nonatomic, assign, readonly) BOOL keepScrollPosition;

@end
