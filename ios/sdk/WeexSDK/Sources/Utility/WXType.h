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
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, WXLayoutDirection) {
    WXLayoutDirectionLTR,
    WXLayoutDirectionRTL,
    WXLayoutDirectionAuto,
};

typedef NS_ENUM(NSUInteger, WXComponentType) {
    WXComponentTypeCommon = 0,
    WXComponentTypeVirtual
};

typedef NS_ENUM(NSUInteger, WXScrollDirection) {
    WXScrollDirectionVertical,
    WXScrollDirectionHorizontal,
    WXScrollDirectionNone,
};

typedef NS_ENUM(NSUInteger, WXTextStyle) {
    WXTextStyleNormal = 0,
    WXTextStyleItalic
};

typedef NS_ENUM(NSInteger, WXTextDecoration) {
    WXTextDecorationNone = 0,
    WXTextDecorationUnderline,
    WXTextDecorationLineThrough
};

typedef NS_ENUM(NSInteger, WXImageQuality) {
    WXImageQualityOriginal = -1,
    WXImageQualityLow = 0,
    WXImageQualityNormal,
    WXImageQualityHigh,
    WXImageQualityNone,
};

typedef NS_ENUM(NSInteger, WXImageSharp) {
    WXImageSharpeningNone = 0,
    WXImageSharpening
};

typedef NS_ENUM(NSInteger, WXVisibility) {
    WXVisibilityShow = 0,
    WXVisibilityHidden
};

typedef NS_ENUM(NSInteger, WXBorderStyle) {
    WXBorderStyleNone = 0,
    WXBorderStyleDotted,
    WXBorderStyleDashed,
    WXBorderStyleSolid
};

typedef NS_ENUM(NSInteger, WXPositionType) {
    WXPositionTypeRelative = 0,
    WXPositionTypeAbsolute,
    WXPositionTypeSticky,
    WXPositionTypeFixed
};

typedef NS_ENUM(NSInteger, WXGradientType) {
    WXGradientTypeToTop = 0,
    WXGradientTypeToBottom,
    WXGradientTypeToLeft,
    WXGradientTypeToRight,
    WXGradientTypeToTopleft,
    WXGradientTypeToBottomright,
};

NS_ASSUME_NONNULL_END
