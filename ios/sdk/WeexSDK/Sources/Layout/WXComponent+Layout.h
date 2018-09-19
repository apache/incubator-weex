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
#import "WXSDKInstance.h"
#import "WXUtility.h"

#define FlexUndefined NAN

#ifdef __cplusplus
#include "layout.h"

extern "C" {
#endif
    bool flexIsUndefined(float value);
#ifdef __cplusplus
}
#endif

@interface WXComponent ()
{
    @package
#ifdef __cplusplus
    WeexCore::WXCoreLayoutNode *_flexCssNode;
#endif // __cplusplus

    CGRect _calculatedFrame;
    CGPoint _absolutePosition;
    WXPositionType _positionType;
}

/**
 * @abstract Return the css node used to layout.
 *
 * @warning Subclasses must not override this.
 */
#ifdef __cplusplus
@property(nonatomic, readonly, assign) WeexCore::WXCoreLayoutNode *flexCssNode;
#endif

/**
 * @abstract Convert layout dimension value like 'left', 'width' to style value in js considering viewport and scale.
 */
- (NSString*)convertLayoutValueToStyleValue:(NSString*)valueName;

/**
 * @abstract Get style width of a container(scroller like) with safe value. No NAN, No zero.
 */
- (CGFloat)safeContainerStyleWidth;

/**
 * @abstract Delete css node of a subcomponent.
 */
- (void)removeSubcomponentCssNode:(WXComponent *)subcomponent;

@end
