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

#import "WXView.h"
#import "WXComponent.h"
#import "WXLayer.h"
#import "WXComponent_internal.h"

@implementation WXView

+ (Class)layerClass
{
    return [WXLayer class];
}

- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
    /**
     *  Capturing touches on a subview outside the frame of its superview if it does not clips to bounds.
     */
    if (self.hidden || !self.userInteractionEnabled) {
        return nil;
    }
    
    UIView* result = [super hitTest:point withEvent:event];
    if (result) {
        if (self.wx_component && self.wx_component->_eventPenetrationEnabled && result == self) {
            return nil;
        }
        return result;
    }
    
    // if clips to bounds, no need to detect outside views.
    if (self.clipsToBounds) {
        return nil;
    }
    
    for (UIView* subView in [self.subviews reverseObjectEnumerator]) {
        if (subView.hidden) {
            continue;
        }
        CGPoint subPoint = [self convertPoint:point toView:subView];
        result = [subView hitTest:subPoint withEvent:event];
        if (result) {
            return result;
        }
    }
    
    return nil;
}

- (BOOL)accessibilityPerformMagicTap
{
    if (self.wx_component->_accessibilityMagicTapEvent) {
        [self.wx_component fireEvent:@"accessibilityMagicTap" params:nil];
    }
    return [super accessibilityPerformMagicTap];
}

@end
