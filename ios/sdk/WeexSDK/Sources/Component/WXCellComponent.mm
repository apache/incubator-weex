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

#import "WXSDKInstance.h"
#import "WXLog.h"
#import "WXCellComponent.h"
#import "WXListComponent.h"
#import "WXComponent_internal.h"
#import "WXDiffUtil.h"
#import "WXComponent+Layout.h"

@interface WXCellComponent ()

@end

@implementation WXCellComponent
{
    NSIndexPath *_indexPathBeforeMove;
    BOOL _isUseContainerWidth;
}

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance
{
#ifdef DEBUG
    WXLogDebug(@"flexLayout -> init Cell: ref:%@, styles:%@",ref,styles);
#endif
    
    self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance];
    
    if (self) {
        _async = attributes[@"async"] ? [WXConvert BOOL:attributes[@"async"]] : YES;
        _isRecycle = attributes[@"recycle"] ? [WXConvert BOOL:attributes[@"recycle"]] : YES;
        _insertAnimation = [WXConvert UITableViewRowAnimation:attributes[@"insertAnimation"]];
        _deleteAnimation = [WXConvert UITableViewRowAnimation:attributes[@"deleteAnimation"]];
        _keepScrollPosition = attributes[@"keepScrollPosition"] ? [WXConvert BOOL:attributes[@"keepScrollPosition"]] : NO;
        _lazyCreateView = YES;
        _isNeedJoinLayoutSystem = NO;
        if (attributes[@"zIndex"]) {
            _zIndex = [WXConvert NSString:attributes[@"zIndex"]];
        }
    }
    
    return self;
}

- (void)dealloc
{
    
}

- (BOOL)weex_isEqualTo:(id<WXDiffable>)object
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

- (void)didFinishDrawingLayer:(BOOL)success
{
    [self.delegate cellDidRendered:self];
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
    if (self.delegate == (id<WXCellRenderDelegate>)newSupercomponent) {
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
        if (self.delegate && (flexIsUndefined(self.flexCssNode->getStyleWidth()) || _isUseContainerWidth)) {
            self.flexCssNode->setStyleWidth([self.delegate containerWidthForLayout:self],NO);
            _isUseContainerWidth = YES;
        }
        
        if ([self needsLayout]) {
            std::pair<float, float> renderPageSize;
            renderPageSize.first = self.weexInstance.frame.size.width;
            renderPageSize.second = self.weexInstance.frame.size.height;
            self.flexCssNode->calculateLayout(renderPageSize);
            if ([WXLog logLevel] >= WXLogLevelDebug) {
                
            }
        }
    [super _calculateFrameWithSuperAbsolutePosition:superAbsolutePosition gatherDirtyComponents:dirtyComponents];
}
@end

