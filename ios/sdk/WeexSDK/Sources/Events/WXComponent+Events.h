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

#import "WXComponent.h"
#import <objc/runtime.h>

@interface UITouch (WXTouchGestureRecognizer)

@property (nonatomic, strong) NSNumber *wx_identifier;
@property (nonatomic, strong) NSNumber *wx_stopPropagation;

@end

@implementation UITouch (WXTouchGestureRecognizer)

- (NSNumber *)wx_identifier
{
    return objc_getAssociatedObject(self, _cmd);
}

- (void)setWx_identifier:(NSNumber *)wx_identifier
{
    objc_setAssociatedObject(self, @selector(wx_identifier), wx_identifier, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

- (NSNumber *)wx_stopPropagation
{
    return objc_getAssociatedObject(self, _cmd);
}

- (void)setWx_stopPropagation:(NSNumber *)wx_stopPropagation
{
    objc_setAssociatedObject(self, @selector(wx_stopPropagation), wx_stopPropagation, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

@end


@interface WXComponent (Events) <UIGestureRecognizerDelegate>
- (BOOL)gestureShouldStopPropagation:(UIGestureRecognizer *)gestureRecognizer shouldReceiveTouch:(UITouch *)touch;
- (BOOL)requestGestureShouldStopPropagation:(UIGestureRecognizer *)gestureRecognizer shouldReceiveTouch:(UITouch *)touch;
@end
