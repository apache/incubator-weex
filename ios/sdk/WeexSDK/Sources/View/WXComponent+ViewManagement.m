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
    _backgroundColor = styles[@"backgroundColor"] ? [WXConvert UIColor:styles[@"backgroundColor"]] : [UIColor clearColor];
    _backgroundImage = styles[@"backgroundImage"] ? [[WXConvert NSString:styles[@"backgroundImage"]]stringByReplacingOccurrencesOfString:@" " withString:@""]: nil;
    _opacity = styles[@"opacity"] ? [WXConvert CGFloat:styles[@"opacity"]] : 1.0;
    _clipToBounds = styles[@"overflow"] ? [WXConvert WXClipType:styles[@"overflow"]] : NO;
    _visibility = styles[@"visibility"] ? [WXConvert WXVisibility:styles[@"visibility"]] : WXVisibilityShow;
    _positionType = styles[@"position"] ? [WXConvert WXPositionType:styles[@"position"]] : WXPositionTypeRelative;
    _transform = styles[@"transform"] || styles[@"transformOrigin"] ?
    [[WXTransform alloc] initWithCSSValue:[WXConvert NSString:styles[@"transform"]] origin:[WXConvert NSString:styles[@"transformOrigin"]] instance:self.weexInstance] :
    [[WXTransform alloc] initWithCSSValue:nil origin:nil instance:self.weexInstance];
    _boxShadow = styles[@"boxShadow"]?[WXConvert WXBoxShadow:styles[@"boxShadow"] scaleFactor:self.weexInstance.pixelScaleFactor]:nil;
    if (_boxShadow) {
        _lastBoxShadow = _boxShadow;
    }
}

- (void)_updateViewStyles:(NSDictionary *)styles
{
    WX_CHECK_COMPONENT_TYPE(self.componentType)
    if (styles[@"boxShadow"]) {
        _lastBoxShadow = _boxShadow;
        _boxShadow = styles[@"boxShadow"]?[WXConvert WXBoxShadow:styles[@"boxShadow"] scaleFactor:self.weexInstance.pixelScaleFactor]:nil;
        [self configBoxShadow:_boxShadow];
        [self setNeedsDisplay];
    }
    
    if (styles[@"backgroundColor"]) {
        _backgroundColor = [WXConvert UIColor:styles[@"backgroundColor"]];
        [self setNeedsDisplay];
    }
    
    if (styles[@"backgroundImage"]) {
        _backgroundImage = styles[@"backgroundImage"] ? [[WXConvert NSString:styles[@"backgroundImage"]]stringByReplacingOccurrencesOfString:@" " withString:@""]: nil;
        
        if (_backgroundImage) {
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
        
        if (positionType == WXPositionTypeFixed) {
            [self.weexInstance.componentManager addFixedComponent:self];
            _isNeedJoinLayoutSystem = NO;
            [self.supercomponent _recomputeCSSNodeChildren];
        } else if (_positionType == WXPositionTypeFixed) {
            [self.weexInstance.componentManager removeFixedComponent:self];
            _isNeedJoinLayoutSystem = YES;
            [self.supercomponent _recomputeCSSNodeChildren];
        }
        
        _positionType = positionType;
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
    
    if (styles[@"transformOrigin"] || styles[@"transform"]) {
        id transform = styles[@"transform"] ? : self.styles[@"transform"];
        id transformOrigin = styles[@"transformOrigin"] ? [WXConvert NSString:styles[@"transformOrigin"]] : [WXConvert NSString:self.styles[@"transformOrigin"]];
        _transform = [[WXTransform alloc] initWithCSSValue:[WXConvert NSString:transform] origin:transformOrigin instance:self.weexInstance];
        if (!CGRectEqualToRect(self.calculatedFrame, CGRectZero)) {
            [_transform applyTransformForView:_view];
            [_layer setNeedsDisplay];
        }
    }
}

-(void)_resetStyles:(NSArray *)styles
{
    if (styles && [styles containsObject:@"backgroundColor"]) {
        _backgroundColor = [UIColor clearColor];
        [self setNeedsDisplay];
    }
    if (styles && [styles containsObject:@"boxShadow"]) {
        _lastBoxShadow = _boxShadow;
        _boxShadow = nil;
        [self setNeedsDisplay];
    }
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
    
    if(self.ancestorScroller){
        [self.ancestorScroller removeStickyComponent:self];
        [self.ancestorScroller removeScrollToListener:self];
    }
    
    for (WXComponent *subcomponents in [self.subcomponents reverseObjectEnumerator]) {
        [subcomponents _unloadViewWithReusing:isReusing];
    }
    
    [_view removeFromSuperview];
    _view = nil;
    [_layer removeFromSuperlayer];
    _layer = nil;
    
    [self viewDidUnload];
}

@end
