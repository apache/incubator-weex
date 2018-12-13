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

typedef WeexCore::WXCoreFlexDirection WXCoreFlexDirection;
typedef WeexCore::WXCoreFlexWrap WXCoreFlexWrap;
typedef WeexCore::WXCoreJustifyContent WXCoreJustifyContent;
typedef WeexCore::WXCoreAlignItems WXCoreAlignItems;
typedef WeexCore::WXCoreAlignSelf WXCoreAlignSelf;
typedef WeexCore::WXCorePositionType WXCorePositionType;
typedef WeexCore::WXCoreDirection WXCoreDirection;

extern "C" {
#endif
    bool flexIsUndefined(float value);
#ifdef __cplusplus
}
#endif

#ifndef __cplusplus
// Ensure that .m files can use css style enum definitions.
#include "flex_enum.h"

typedef enum WXCoreFlexDirection WXCoreFlexDirection;
typedef enum WXCoreFlexWrap WXCoreFlexWrap;
typedef enum WXCoreJustifyContent WXCoreJustifyContent;
typedef enum WXCoreAlignItems WXCoreAlignItems;
typedef enum WXCoreAlignSelf WXCoreAlignSelf;
typedef enum WXCorePositionType WXCorePositionType;
typedef enum WXCoreDirection WXCoreDirection;

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
    BOOL _isLastLayoutDirectionRTL;
    BOOL _isLayoutDirectionRTL;
}

/**
 * @abstract Return the css node used to layout.
 *
 * @warning Subclasses must not override this.
 */
#ifdef __cplusplus
@property (nonatomic, readonly, assign) WeexCore::WXCoreLayoutNode *flexCssNode;
#endif

/**
 * @abstract Get css style value for key. The key should be of CSS standard form.
 *  This method is for convenience use in C/ObjC environment. And if you want to
 *  retrieve all style values or in C++, you could use flexCssNode directly.
 *
 *  Thread usage:
 *      This method should be invoked in component thread by WXPerformBlockOnComponentThread.
 *      Note that all initWithRef methods of WXComponent and its subclasses are performed in
 *      component thread by default. Therefore you can call this method directly in initWithRef.
 *
 *  Supported keys:
 *      width, height, min-width, min-height, max-width, max-height,
 *      margin-(left/right/top/bottom)
 *      padding-(left/right/top/bottom)
 *      border-(left/right/top/bottom)-width
 *      left, right, top, bottom
 *      flex-grow
 */
- (float)getCssStyleValueForKey:(NSString *)key;

/**
 * @abstract Get css style flex-direction. Thread usage the same as getCssStyleValueForKey.
 */
- (WXCoreFlexDirection)getCssStyleFlexDirection;

/**
 * @abstract Get css style flex-wrap. Thread usage the same as getCssStyleValueForKey.
 */
- (WXCoreFlexWrap)getCssStyleFlexWrap;

/**
 * @abstract Get css style justify-content. Thread usage the same as getCssStyleValueForKey.
 */
- (WXCoreJustifyContent)getCssStyleJustifyContent;

/**
 * @abstract Get css style align-items. Thread usage the same as getCssStyleValueForKey.
 */
- (WXCoreAlignItems)getCssStyleAlignItems;

/**
 * @abstract Get css style align-self. Thread usage the same as getCssStyleValueForKey.
 */
- (WXCoreAlignSelf)getCssStyleAlignSelf;

/**
 * @abstract Get css style position. Thread usage the same as getCssStyleValueForKey.
 */
- (WXCorePositionType)getCssStylePositionType;

/**
 * @abstract Get css layout direction. Thread usage the same as getCssStyleValueForKey.
 */
- (WXCoreDirection)getCssDirection;

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

#pragma mark - RTL

@property (nonatomic, assign, readonly) BOOL isDirectionRTL;

// Now we scrollView RTL solution is tranform
// so scrollView need tranform subviews when RTL by default
// if your component view is not scrollView but also implement RTL layout by tranform，you need return YES
- (BOOL)shouldTransformSubviewsWhenRTL;

- (void)layoutDirectionDidChanged:(BOOL)isRTL;

@end
