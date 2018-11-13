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

#import "WXHeaderComponent.h"
#import "WXComponent_internal.h"
#import "WXComponent+Layout.h"
#import "WXAssert.h"

@implementation WXHeaderComponent
{
    BOOL _isUseContainerWidth;
}

- (instancetype)initWithRef:(NSString *)ref type:(NSString *)type styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events weexInstance:(WXSDKInstance *)weexInstance
{
    self = [super initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:weexInstance];
    
    if (self) {
        _async = YES;
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

- (BOOL)_isCalculatedFrameChanged:(CGRect)frame
{
    return !CGSizeEqualToSize(frame.size, _calculatedFrame.size); // only compares sizes
}

- (void)_assignCalculatedFrame:(CGRect)frame
{
    frame.origin = CGPointZero;
    WXAssert(!isnan(frame.size.height), @"Height of header should not be NAN.");
    if (isnan(frame.size.height) || frame.size.height < 0.0f) {
        frame.size.height = 0.0f;
    }
    _calculatedFrame = frame;
}

@end
