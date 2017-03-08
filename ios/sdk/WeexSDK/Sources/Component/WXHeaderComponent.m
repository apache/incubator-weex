/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXHeaderComponent.h"
#import "WXComponent_internal.h"

@implementation WXHeaderComponent
{
    BOOL _isUseContainerWidth;
}

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance
{
    self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance];
    
    if (self) {
        _async = YES;
        _isNeedJoinLayoutSystem = NO;
        _keepScrollPosition = attributes[@"keepScrollPosition"] ? [WXConvert BOOL:attributes[@"keepScrollPosition"]] : NO;
    }
    
    return self;
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    if (attributes[@"keepScrollPosition"]) {
        _keepScrollPosition = [WXConvert BOOL:attributes[@"keepScrollPosition"]];
    }
}

- (BOOL)isSticky
{
    return _positionType == WXPositionTypeSticky;
}

- (void)_frameDidCalculated:(BOOL)isChanged
{
    [super _frameDidCalculated:isChanged];
    
    if (isChanged) {
        [self.delegate headerDidLayout:self];
    }
}

- (void)_removeFromSupercomponent
{
    [super _removeFromSupercomponent];
    
    [self.delegate headerDidRemove:self];
}

- (void)_calculateFrameWithSuperAbsolutePosition:(CGPoint)superAbsolutePosition gatherDirtyComponents:(NSMutableSet<WXComponent *> *)dirtyComponents
{
    if (self.delegate && (isUndefined(self.cssNode->style.dimensions[CSS_WIDTH]) || _isUseContainerWidth)) {
        self.cssNode->style.dimensions[CSS_WIDTH] = [self.delegate headerWidthForLayout:self];
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
