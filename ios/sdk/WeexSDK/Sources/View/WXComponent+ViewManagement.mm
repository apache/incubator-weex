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

#import "WXComponent+ViewManagement.h"
#import "WXComponent_internal.h"
#import "WXComponent+BoxShadow.h"
#import "WXAssert.h"
#import "WXView.h"
#import "WXSDKInstance_private.h"
#import "WXTransform.h"
#import "WXSDKManager.h"
#import "WXComponent+Layout.h"

#define WX_BOARD_RADIUS_RESET_ALL(key)\
do {\
    if (styles && [styles containsObject:@#key]) {\
        _borderTopLeftRadius = _borderTopRightRadius = _borderBottomLeftRadius = _borderBottomRightRadius = 0;\
        [self setNeedsDisplay];\
    }\
} while(0);

#define WX_BOARD_RADIUS_RESET(key)\
do {\
    if (styles && [styles containsObject:@#key]) {\
    _##key = 0;\
    [self setNeedsDisplay];\
    }\
} while(0);

#define WX_BOARD_WIDTH_RESET_ALL(key)\
do {\
    if (styles && [styles containsObject:@#key]) {\
        _borderTopWidth = _borderLeftWidth = _borderRightWidth = _borderBottomWidth = 0;\
        [self setNeedsLayout];\
    }\
} while(0);

#define WX_BOARD_WIDTH_RESET(key)\
do {\
    if (styles && [styles containsObject:@#key]) {\
        _##key = 0;\
        [self setNeedsLayout];\
    }\
} while(0);

#define WX_BOARD_RADIUS_COLOR_RESET_ALL(key)\
do {\
    if (styles && [styles containsObject:@#key]) {\
        _borderTopColor = _borderLeftColor = _borderRightColor = _borderBottomColor = [UIColor blackColor];\
        [self setNeedsDisplay];\
    }\
} while(0);

#define WX_BOARD_RADIUS_DARK_SCHEME_COLOR_RESET_ALL(key)\
do {\
    if (styles && [styles containsObject:@#key]) {\
        _darkSchemeBorderTopColor = _darkSchemeBorderLeftColor = _darkSchemeBorderRightColor = _darkSchemeBorderBottomColor = [UIColor blackColor];\
        [self setNeedsDisplay];\
    }\
} while(0);

#define WX_BOARD_RADIUS_LIGHT_SCHEME_COLOR_RESET_ALL(key)\
do {\
    if (styles && [styles containsObject:@#key]) {\
        _lightSchemeBorderTopColor = _lightSchemeBorderLeftColor = _lightSchemeBorderRightColor = _lightSchemeBorderBottomColor = [UIColor blackColor];\
        [self setNeedsDisplay];\
    }\
} while(0);

#define WX_BOARD_COLOR_RESET(key)\
do {\
    if (styles && [styles containsObject:@#key]) {\
        _##key = [UIColor blackColor];\
        [self setNeedsDisplay];\
    }\
} while(0);

#pragma clang diagnostic ignored "-Wobjc-protocol-method-implementation"

@implementation WXComponent (ViewManagement)

#pragma mark Public

- (UIView *)loadView
{
    return [[WXView alloc] init];
}

- (BOOL)isViewLoaded
{
    return (_view != nil);
}

- (void)insertSubview:(WXComponent *)subcomponent atIndex:(NSInteger)index
{
    WXAssertMainThread();
    
    if (subcomponent.displayType == WXDisplayTypeNone) {
        return;
    }
    
    if (_isViewTreeIgnored) {
        // self not added to views, children also ignored.
        subcomponent->_isViewTreeIgnored = YES;
        return;
    }
    
    if (subcomponent->_isViewTreeIgnored) {
        // children not added to views, such as div in list, we do not create view.
        return;
    }
  
    WX_CHECK_COMPONENT_TYPE(self.componentType)
    if (subcomponent->_positionType == WXPositionTypeFixed) {
        [self.weexInstance.rootView addSubview:subcomponent.view];
        return;
    }
    
    // use _lazyCreateView to forbid component like cell's view creating
    if(_lazyCreateView) {
        subcomponent->_lazyCreateView = YES;
    }
    
    if (!subcomponent->_lazyCreateView || (self->_lazyCreateView && [self isViewLoaded])) {
        [self.view insertSubview:subcomponent.view atIndex:index];
    }
}

- (void)willRemoveSubview:(WXComponent *)component
{
    WXAssertMainThread();
}

- (void)removeFromSuperview
{
    WXAssertMainThread();
    
    if ([self isViewLoaded]) {
        [self.view removeFromSuperview];
    }
}

- (void)moveToSuperview:(WXComponent *)newSupercomponent atIndex:(NSUInteger)index
{
    WX_CHECK_COMPONENT_TYPE(self.componentType)
    [self removeFromSuperview];
    [newSupercomponent insertSubview:self atIndex:index];
}

- (void)viewWillLoad
{
    WXAssertMainThread();
}

- (void)viewDidLoad
{
    WXAssertMainThread();
    if (self.componentCallback) {
        self.componentCallback(self, WXComponentViewCreatedCallback, _view);
    }
}

- (void)viewWillUnload
{
    WXAssertMainThread();
}

- (void)viewDidUnload
{
    WXAssertMainThread();
}

#pragma mark Private

- (void)_initViewPropertyWithStyles:(NSDictionary *)styles
{
    self.styleBackgroundColor = styles[@"backgroundColor"] ? [WXConvert UIColor:styles[@"backgroundColor"]] : [UIColor clearColor];
    if (styles[@"weexDarkSchemeBackgroundColor"]) {
        self.darkSchemeBackgroundColor = [WXConvert UIColor:styles[@"weexDarkSchemeBackgroundColor"]];
    }
    if (styles[@"weexLightSchemeBackgroundColor"]) {
        self.lightSchemeBackgroundColor = [WXConvert UIColor:styles[@"weexLightSchemeBackgroundColor"]];
    }
    _backgroundImage = styles[@"backgroundImage"] ? [WXConvert NSString:styles[@"backgroundImage"]]: nil;
    _darkSchemeBackgroundImage = styles[@"weexDarkSchemeBackgroundImage"] ? [WXConvert NSString:styles[@"weexDarkSchemeBackgroundImage"]] : nil;
    _lightSchemeBackgroundImage = styles[@"weexLightSchemeBackgroundImage"] ? [WXConvert NSString:styles[@"weexLightSchemeBackgroundImage"]] : nil;
    _opacity = styles[@"opacity"] ? [WXConvert CGFloat:styles[@"opacity"]] : 1.0;
    _clipToBounds = styles[@"overflow"] ? [WXConvert WXClipType:styles[@"overflow"]] : NO;
    _visibility = styles[@"visibility"] ? [WXConvert WXVisibility:styles[@"visibility"]] : WXVisibilityShow;
    _positionType = styles[@"position"] ? [WXConvert WXPositionType:styles[@"position"]] : WXPositionTypeRelative;
    _transform = styles[@"transform"] || styles[@"transformOrigin"] ?
    [[WXTransform alloc] initWithCSSValue:[WXConvert NSString:styles[@"transform"]] origin:[WXConvert NSString:styles[@"transformOrigin"]] instance:self.weexInstance] :
    [[WXTransform alloc] initWithCSSValue:nil origin:nil instance:self.weexInstance];
    _boxShadow = styles[@"boxShadow"]?[WXConvert WXBoxShadow:styles[@"boxShadow"] scaleFactor:self.weexInstance.pixelScaleFactor]:nil;
    if (styles[@"weexDarkSchemeBoxShadow"]) {
        _darkSchemeBoxShadow = [WXConvert WXBoxShadow:styles[@"weexDarkSchemeBoxShadow"] scaleFactor:self.weexInstance.pixelScaleFactor];
    }
    if (styles[@"weexLightSchemeBoxShadow"]) {
        _lightSchemeBoxShadow = [WXConvert WXBoxShadow:styles[@"weexLightSchemeBoxShadow"] scaleFactor:self.weexInstance.pixelScaleFactor];
    }
}

- (void)_transitionUpdateViewProperty:(NSDictionary *)styles
{
    WX_CHECK_COMPONENT_TYPE(self.componentType)
    if (styles[@"backgroundColor"]) {
        self.styleBackgroundColor = [WXConvert UIColor:styles[@"backgroundColor"]];
    }
    if (styles[@"weexDarkSchemeBackgroundColor"]) {
        self.darkSchemeBackgroundColor = [WXConvert UIColor:styles[@"weexDarkSchemeBackgroundColor"]];
    }
    if (styles[@"weexLightSchemeBackgroundColor"]) {
        self.lightSchemeBackgroundColor = [WXConvert UIColor:styles[@"weexLightSchemeBackgroundColor"]];
    }
    if (styles[@"opacity"]) {
        _opacity = [WXConvert CGFloat:styles[@"opacity"]];
    }
}

- (void)_updateViewStyles:(NSDictionary *)styles
{
    WX_CHECK_COMPONENT_TYPE(self.componentType)
    if (styles[@"boxShadow"]) {
        _boxShadow = styles[@"boxShadow"]?[WXConvert WXBoxShadow:styles[@"boxShadow"] scaleFactor:self.weexInstance.pixelScaleFactor]:nil;
    }
    if (styles[@"weexDarkSchemeBoxShadow"]) {
        _darkSchemeBoxShadow = [WXConvert WXBoxShadow:styles[@"weexDarkSchemeBoxShadow"] scaleFactor:self.weexInstance.pixelScaleFactor];
    }
    if (styles[@"weexLightSchemeBoxShadow"]) {
        _lightSchemeBoxShadow = [WXConvert WXBoxShadow:styles[@"weexLightSchemeBoxShadow"] scaleFactor:self.weexInstance.pixelScaleFactor];
    }
    if (styles[@"boxShadow"] || styles[@"weexDarkSchemeBoxShadow"] || styles[@"weexLightSchemeBoxShadow"]) {
        WXBoxShadow* usingBoxShadow = [self _chooseBoxShadow];
        if (usingBoxShadow) {
            _lastBoxShadow = usingBoxShadow;
            [self configBoxShadow:usingBoxShadow];
        }
        [self setNeedsDisplay];
    }
    
    if (styles[@"backgroundColor"]) {
        self.styleBackgroundColor = [WXConvert UIColor:styles[@"backgroundColor"]];
        [self setNeedsDisplay];
    }
    
    if (styles[@"weexDarkSchemeBackgroundColor"]) {
        self.darkSchemeBackgroundColor = [WXConvert UIColor:styles[@"weexDarkSchemeBackgroundColor"]];
        [self setNeedsDisplay];
    }
    
    if (styles[@"weexLightSchemeBackgroundColor"]) {
        self.lightSchemeBackgroundColor = [WXConvert UIColor:styles[@"weexLightSchemeBackgroundColor"]];
        [self setNeedsDisplay];
    }
    
    if (styles[@"backgroundImage"]) {
        _backgroundImage = [WXConvert NSString:styles[@"backgroundImage"]];
    }
    
    if (styles[@"weexDarkSchemeBackgroundImage"]) {
        _darkSchemeBackgroundImage = [WXConvert NSString:styles[@"weexDarkSchemeBackgroundImage"]];
    }
    if (styles[@"weexLightSchemeBackgroundImage"]) {
        _lightSchemeBackgroundImage = [WXConvert NSString:styles[@"weexLightSchemeBackgroundImage"]];
    }
    
    if (styles[@"backgroundImage"] ||
        styles[@"weexDarkSchemeBackgroundImage"] ||
        styles[@"weexLightSchemeBackgroundImage"]) {
        if (_backgroundImage || _darkSchemeBackgroundImage || _lightSchemeBackgroundImage) {
            [self setGradientLayer];
        }
    }
    
    if (styles[@"opacity"]) {
        _opacity = [WXConvert CGFloat:styles[@"opacity"]];
        _layer.opacity = _opacity;
    }
    
    if (styles[@"overflow"]) {
        _clipToBounds = [WXConvert WXClipType:styles[@"overflow"]];
        _view.clipsToBounds = _clipToBounds;
    }
    
    if (styles[@"position"]) {
        WXPositionType positionType = [WXConvert WXPositionType:styles[@"position"]];
        if (positionType == WXPositionTypeSticky) {
            [self.ancestorScroller addStickyComponent:self];
        } else if (_positionType == WXPositionTypeSticky) {
            [self.ancestorScroller removeStickyComponent:self];
        }
        
        WXPerformBlockOnComponentThread(^{
            if (positionType == WXPositionTypeFixed) {
                [self.weexInstance.componentManager addFixedComponent:self];
            } else if (_positionType == WXPositionTypeFixed) {
                [self.weexInstance.componentManager removeFixedComponent:self];
            }
            
            _positionType = positionType;
        });
    }
    
    if (styles[@"visibility"]) {
        _visibility = [WXConvert WXVisibility:styles[@"visibility"]];
        if (_visibility == WXVisibilityShow) {
            self.view.hidden = NO;
        }
        else {
            self.view.hidden = YES;
        }
    }
    if (styles[@"transform"]) {
        id transformOrigin = styles[@"transformOrigin"] ?: self.styles[@"transformOrigin"];
        WXTransform* transform = [[WXTransform alloc] initWithCSSValue:[WXConvert NSString:styles[@"transform"]] origin:[WXConvert NSString:transformOrigin] instance:self.weexInstance];
        if (!CGRectEqualToRect(self.calculatedFrame, CGRectZero)) {
            [transform applyTransformForView:_view];
            [self _adjustForRTL];
            [_layer setNeedsDisplay];
        }
        self.transform = transform;
    } else if (styles[@"transformOrigin"]) {
        [_transform setTransformOrigin:[WXConvert NSString:styles[@"transformOrigin"]]];
        if (!CGRectEqualToRect(self.calculatedFrame, CGRectZero)) {
            [_transform applyTransformForView:_view];
            [self _adjustForRTL];
            [_layer setNeedsDisplay];
        }
    }
    // for RTL
    if (styles[@"direction"]) {
        [self _adjustForRTL];
    }
}

- (void)resetBorder:(NSArray *)styles
{
    WX_BOARD_RADIUS_RESET_ALL(borderRadius);
    WX_BOARD_RADIUS_RESET(borderTopLeftRadius);
    WX_BOARD_RADIUS_RESET(borderTopRightRadius);
    WX_BOARD_RADIUS_RESET(borderBottomLeftRadius);
    WX_BOARD_RADIUS_RESET(borderBottomRightRadius);
    
    WX_BOARD_WIDTH_RESET_ALL(borderWidth);
    WX_BOARD_WIDTH_RESET(borderTopWidth);
    WX_BOARD_WIDTH_RESET(borderLeftWidth);
    WX_BOARD_WIDTH_RESET(borderRightWidth);
    WX_BOARD_WIDTH_RESET(borderBottomWidth);
    
    WX_BOARD_RADIUS_COLOR_RESET_ALL(borderColor);
    WX_BOARD_COLOR_RESET(borderTopColor);
    WX_BOARD_COLOR_RESET(borderLeftColor);
    WX_BOARD_COLOR_RESET(borderRightColor);
    WX_BOARD_COLOR_RESET(borderBottomColor);
    
    WX_BOARD_RADIUS_DARK_SCHEME_COLOR_RESET_ALL(darkSchemeBorderColor);
    WX_BOARD_COLOR_RESET(darkSchemeBorderTopColor);
    WX_BOARD_COLOR_RESET(darkSchemeBorderLeftColor);
    WX_BOARD_COLOR_RESET(darkSchemeBorderRightColor);
    WX_BOARD_COLOR_RESET(darkSchemeBorderBottomColor);
    
    WX_BOARD_RADIUS_LIGHT_SCHEME_COLOR_RESET_ALL(lightSchemeBorderColor);
    WX_BOARD_COLOR_RESET(lightSchemeBorderTopColor);
    WX_BOARD_COLOR_RESET(lightSchemeBorderLeftColor);
    WX_BOARD_COLOR_RESET(lightSchemeBorderRightColor);
    WX_BOARD_COLOR_RESET(lightSchemeBorderBottomColor);
}

- (void)_resetStyles:(NSArray *)styles
{
    if (styles && [styles containsObject:@"backgroundColor"]) {
        self.styleBackgroundColor = [UIColor clearColor];
        [self setNeedsDisplay];
    }
    if (styles && [styles containsObject:@"weexDarkSchemeBackgroundColor"]) {
        self.darkSchemeBackgroundColor = nil;
        [self setNeedsDisplay];
    }
    if (styles && [styles containsObject:@"weexLightSchemeBackgroundColor"]) {
        self.lightSchemeBackgroundColor = nil;
        [self setNeedsDisplay];
    }
    
    if (styles && [styles containsObject:@"boxShadow"]) {
        _lastBoxShadow = _boxShadow;
        _boxShadow = nil;
        [self setNeedsDisplay];
    }
    if (styles && [styles containsObject:@"weexDarkSchemeBoxShadow"]) {
        _darkSchemeBoxShadow = nil;
        [self setNeedsDisplay];
    }
    if (styles && [styles containsObject:@"weexLightSchemeBoxShadow"]) {
        _lightSchemeBoxShadow = nil;
        [self setNeedsDisplay];
    }
    
    if (styles && [styles containsObject:@"backgroundImage"]) {
        _backgroundImage = nil;
    }
    if (styles && [styles containsObject:@"weexDarkSchemeBackgroundImage"]) {
        _darkSchemeBackgroundImage = nil;
    }
    if (styles && [styles containsObject:@"weexLightSchemeBackgroundImage"]) {
        _lightSchemeBackgroundImage = nil;
    }
    if (styles && ([styles containsObject:@"backgroundImage"] ||
                   [styles containsObject:@"weexDarkSchemeBackgroundImage"] ||
                   [styles containsObject:@"weexLightSchemeBackgroundImage"])) {
        [self setGradientLayer];
    }
    
    [self resetBorder:styles];
}

- (void)_unloadViewWithReusing:(BOOL)isReusing
{
    WXAssertMainThread();
    
    if (isReusing && self->_positionType == WXPositionTypeFixed) {
        return;
    }
    
    [self viewWillUnload];
    
    _view.gestureRecognizers = nil;
    
    [self _removeAllEvents];
    
    if (self.ancestorScroller) {
        [self.ancestorScroller removeStickyComponent:self];
        [self.ancestorScroller removeScrollToListener:self];
    }
    
    for (WXComponent *subcomponents in [self.subcomponents reverseObjectEnumerator]) {
        [subcomponents _unloadViewWithReusing:isReusing];
    }
    
    if ([_view superview]) {
        [_view removeFromSuperview];
    }
    
    if (self->_isTemplate && self.attributes[@"@templateId"]) {
        [[WXSDKManager bridgeMgr] callComponentHook:self.weexInstance.instanceId componentId:self.attributes[@"@templateId"] type:@"lifecycle" hook:@"detach" args:nil competion:nil];
    }
    _view = nil;
    [_layer removeFromSuperlayer];
    _layer = nil;
    
    [self viewDidUnload];
}

- (void)unloadNativeView
{
    WXAssertMainThread();
    
    [self viewWillUnload];
    
    _view.gestureRecognizers = nil;
    
    [self _removeAllEvents];
    
    if ([_view superview]) {
        [_view removeFromSuperview];
    }

    _view = nil;
    [_layer removeFromSuperlayer];
    _layer = nil;
    
    [self viewDidUnload];
}

@end
