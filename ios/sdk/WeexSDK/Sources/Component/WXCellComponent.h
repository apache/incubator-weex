/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXComponent.h"
#import "WXDiffUtil.h"
@class WXCellComponent;

@protocol WXCellRenderDelegate <NSObject>

- (float)containerWidthForLayout:(WXCellComponent *)cell;

- (void)cellDidLayout:(WXCellComponent *)cell;

- (void)cellDidRendered:(WXCellComponent *)cell;

- (void)cellDidRemove:(WXCellComponent *)cell;

- (void)cell:(WXCellComponent *)cell didMoveToIndex:(NSUInteger)index;

@end

@interface WXCellComponent : WXComponent <WXDiffable>

@property (nonatomic, strong) NSString *scope;
@property (nonatomic, assign) BOOL isRecycle;
@property (nonatomic, assign) BOOL isLayoutComplete;
@property (nonatomic, assign) UITableViewRowAnimation insertAnimation;
@property (nonatomic, assign) UITableViewRowAnimation deleteAnimation;
@property (nonatomic, assign) BOOL keepScrollPosition;
@property (nonatomic, weak) id<WXCellRenderDelegate> delegate;

@end
