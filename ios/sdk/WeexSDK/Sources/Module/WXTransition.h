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

#define kWXTransitionDuration                   @"transitionDuration"
#define kWXTransitionProperty                   @"transitionProperty"
#define kWXTransitionDelay                      @"transitionDelay"
#define kWXTransitionTimingFunction             @"transitionTimingFunction"

typedef NS_OPTIONS(NSUInteger, WXTransitionOptions) {
    WXTransitionOptionsNone             = 0,
    WXTransitionOptionsWidth            = 1 << 0,
    WXTransitionOptionsHeight           = 1 << 1,
    WXTransitionOptionsRight            = 1 << 2,
    WXTransitionOptionsLeft             = 1 << 3,
    WXTransitionOptionsBottom           = 1 << 4,
    WXTransitionOptionsTop              = 1 << 5,
    WXTransitionOptionsBackgroundColor  = 1 << 6,
    WXTransitionOptionsTransform        = 1 << 7,
    WXTransitionOptionsOpacity          = 1 << 8
};


@interface WXTransitionInfo : NSObject
@property (nonatomic, strong) id fromValue;
@property (nonatomic, strong) id toValue;
@property (nonatomic, strong) id perValue;
@property (nonatomic, strong) NSString *propertyName;
@end

@interface WXTransition : NSObject
@property(nonatomic,strong) NSMutableDictionary *fromStyles;
@property(nonatomic,strong) NSMutableDictionary *addStyles;
@property(nonatomic,strong) NSMutableArray *propertyArray;
@property(nonatomic,assign) WXTransitionOptions transitionOptions;
- (instancetype) initWithStyles:(NSDictionary *)styles;
- (void)_handleTransitionWithStyles:(NSDictionary *)styles resetStyles:(NSMutableArray *)resetStyles  target:(WXComponent *)targetComponent;
@end


