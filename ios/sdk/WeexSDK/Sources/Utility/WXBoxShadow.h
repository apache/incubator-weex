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
#import "WXInnerLayer.h"

@interface WXBoxShadow : NSObject

@property(nonatomic,strong,nullable) UIColor *shadowColor;
@property CGSize shadowOffset;
@property CGFloat shadowRadius;
@property BOOL isInset;
@property (nonatomic, strong, nullable)WXInnerLayer *innerLayer;
@property CGFloat shadowOpacity;

/**
 *  @abstract get boxshadow from string and adapter phone screen
 *
 *  @param string the boxshadow string
 *
 *  @param scaleFactor the boxshadow set last time
 *
 *  @return A WXBoxShadow object
 */
+(WXBoxShadow *_Nullable)getBoxShadowFromString:(NSString *_Nullable)string scaleFactor:(CGFloat)scaleFactor;
@end
