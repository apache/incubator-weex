/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXSDKInstance.h"
#import "WXLog.h"
#import "WXCellComponent.h"
#import "WXListComponent.h"
#import "WXComponent_internal.h"
#import "WXDiffUtil.h"

@interface WXCellComponent ()

@end

@implementation WXCellComponent
{
    NSIndexPath *_indexPathBeforeMove;
    BOOL _isUseContainerWidth;
}

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance
{
    self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance];
    
    if (self) {
        _async = attributes[@"async"] ? [WXConvert BOOL:attributes[@"async"]] : YES;
        _isRecycle = attributes[@"recycle"] ? [WXConvert BOOL:attributes[@"recycle"]] : YES;
        _insertAnimation = [WXConvert UITableViewRowAnimation:attributes[@"insertAnimation"]];
        _deleteAnimation = [WXConvert UITableViewRowAnimation:attributes[@"deleteAnimation"]];
        _keepScrollPosition = attributes[@"keepScrollPosition"] ? [WXConvert BOOL:attributes[@"keepScrollPosition"]] : NO;
        _lazyCreateView = YES;
        _isNeedJoinLayoutSystem = NO;
    }
    
    return self;
}

- (void)dealloc
{
    
}

- (BOOL)isEqualToWXObject:(id<WXDiffable>)object
{
    return self == object;
}

- (void)_frameDidCalculated:(BOOL)isChanged
{
    [super _frameDidCalculated:isChanged];
    
    if (isChanged) {
        [self.delegate cellDidLayout:self];
    }
}

- (WXDisplayCompletionBlock)displayCompletionBlock
{
    return ^(CALayer *layer, BOOL finished) {
        if ([super displayCompletionBlock]) {
            [super displayCompletionBlock](layer, finished);
        }
        
        [self.delegate cellDidRendered:self];
    };
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    if (attributes[@"async"]) {
        _async = [WXConvert BOOL:attributes[@"async"]];
    }
    
    if (attributes[@"recycle"]) {
        _isRecycle = [WXConvert BOOL:attributes[@"recycle"]];
    }
    
    if (attributes[@"insertAnimation"]) {
        _insertAnimation = [WXConvert UITableViewRowAnimation:attributes[@"insertAnimation"]];
    }
    
    if (attributes[@"deleteAnimation"]) {
        _deleteAnimation = [WXConvert UITableViewRowAnimation:attributes[@"deleteAnimation"]];
    }
    
    if (attributes[@"keepScrollPosition"]) {
        _keepScrollPosition = [WXConvert BOOL:attributes[@"keepScrollPosition"]];
    }
}

- (void)_moveToSupercomponent:(WXComponent *)newSupercomponent atIndex:(NSUInteger)index
{
    if (self.delegate == newSupercomponent) {
        [self.delegate cell:self didMoveToIndex:index];
        [super _removeFromSupercomponent];
        [newSupercomponent _insertSubcomponent:self atIndex:index];
    } else {
        [super _moveToSupercomponent:newSupercomponent atIndex:index];
    }
}

- (void)_removeFromSupercomponent
{
    [super _removeFromSupercomponent];
    
    [self.delegate cellDidRemove:self];
}

- (void)removeFromSuperview
{
    // do nothing
}

- (void)_calculateFrameWithSuperAbsolutePosition:(CGPoint)superAbsolutePosition gatherDirtyComponents:(NSMutableSet<WXComponent *> *)dirtyComponents
{
    if (self.delegate && (isUndefined(self.cssNode->style.dimensions[CSS_WIDTH]) || _isUseContainerWidth)) {
        self.cssNode->style.dimensions[CSS_WIDTH] = [self.delegate containerWidthForLayout:self];
        //TODO: set _isUseContainerWidth to NO if updateStyles have width
        _isUseContainerWidth = YES;
    }
    
    if ([self needsLayout]) {
        layoutNode(self.cssNode, CSS_UNDEFINED, CSS_UNDEFINED, CSS_DIRECTION_INHERIT);
        if ([WXLog logLevel] >= WXLogLevelDebug) {
            print_css_node(self.cssNode, CSS_PRINT_LAYOUT | CSS_PRINT_STYLE | CSS_PRINT_CHILDREN);
        }
    }
    
    [super _calculateFrameWithSuperAbsolutePosition:superAbsolutePosition gatherDirtyComponents:dirtyComponents];
}
@end

