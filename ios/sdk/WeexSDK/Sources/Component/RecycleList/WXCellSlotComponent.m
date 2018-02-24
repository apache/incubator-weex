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

#import "WXCellSlotComponent.h"
#import "WXComponent_internal.h"
#import "WXComponentManager.h"
#import "WXSDKInstance_private.h"
#import "WXConvert.h"
#import "WXAssert.h"
#import "WXScrollerComponent.h"

@implementation WXCellSlotComponent

- (instancetype)initWithRef:(NSString *)ref
                       type:(NSString *)type
                     styles:(NSDictionary *)styles
                 attributes:(NSDictionary *)attributes
                     events:(NSArray *)events
               weexInstance:(WXSDKInstance *)weexInstance
{
    self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance];
    if (self) {
        // TODO: isRecycle / insertAnimation / deleteAnimation / keepScrollPosition
        if (attributes[@"default"]) {
            _templateCaseType = @"default";
        }
        _templateCaseType = attributes[@"case"] ? [WXConvert NSString:attributes[@"case"]] : WXDefaultRecycleTemplateType;
        _lazyCreateView = YES;
        _isNeedJoinLayoutSystem = NO;
    }
    
    return self;
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    
}

- (void)updateCellData:(NSDictionary *)data
{
    WXAssertComponentThread();    
    
    [self updateBindingData:data];
    [self triggerLayout];
}

- (void)_didInserted
{
    [self triggerLayout];
}

- (void)triggerLayout
{
    WXAssertComponentThread();
    
    //TODO: _isUseContainerWidth?
    if (isUndefined(self.cssNode->style.dimensions[CSS_WIDTH])) {
        self.cssNode->style.dimensions[CSS_WIDTH] = ((WXScrollerComponent *)(self.supercomponent)).scrollerCSSNode->style.dimensions[CSS_WIDTH];
    }
    
    if ([self needsLayout]) {
        layoutNode(self.cssNode, CSS_UNDEFINED, CSS_UNDEFINED, CSS_DIRECTION_INHERIT);
        if ([WXLog logLevel] >= WXLogLevelDebug) {
            print_css_node(self.cssNode, CSS_PRINT_LAYOUT | CSS_PRINT_STYLE | CSS_PRINT_CHILDREN);
        }
    }
    
    NSMutableSet<WXComponent *> *dirtyComponents = [NSMutableSet set];
    [self _calculateFrameWithSuperAbsolutePosition:CGPointZero gatherDirtyComponents:dirtyComponents];
    for (WXComponent *dirtyComponent in dirtyComponents) {
        [self.weexInstance.componentManager _addUITask:^{
            [dirtyComponent _layoutDidFinish];
        }];
    }
}

@end
