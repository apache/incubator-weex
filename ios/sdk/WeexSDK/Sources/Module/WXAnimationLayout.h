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

#import <Foundation/Foundation.h>

#import "WXComponent.h"
#import "WXComponent_internal.h"
#import "NSTimer+Weex.h"

@interface WXAnimationLayoutInfo : NSObject

@property (nonatomic, strong) NSString *propertyName;
@property (nonatomic, strong) id fromValue;
@property (nonatomic, strong) id toValue;

@end


@interface WXAnimationLayout : NSObject

@property (nonatomic,strong) NSTimer *updateStyleTimer;
@property (nonatomic,strong) WXComponent *targetComponent;
@property (nonatomic,strong) NSDate *animationStartDate;
@property (nonatomic,strong) WXAnimationLayoutInfo *widthInfo;
@property (nonatomic,strong) WXAnimationLayoutInfo *heightInfo;
@property (nonatomic,assign) double animationDuration;
@property (nonatomic,assign) double animationDelay;
@property (nonatomic,strong) NSDictionary *needUpdateStyles;
@property (nonatomic, weak) WXSDKInstance *weexInstance;

- (void)layoutForAnimation;

@end
