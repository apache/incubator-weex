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

#define SOLVE_EPS(dur) (1. / (1000. * (dur)))


#import "WXComponent+Layout.h"
#import "WXComponent_internal.h"
#import "WXTransform.h"
#import "WXAssert.h"
#import "WXComponent_internal.h"
#import "WXSDKInstance_private.h"
#import "WXComponent+BoxShadow.h"
#import "WXLayoutDefine.h"

@implementation WXComponent (Layout)

#pragma clang diagnostic ignored "-Wobjc-protocol-method-implementation"

#pragma mark Public

- (void)setNeedsLayout
{
    _isLayoutDirty = YES;
    WXComponent *supercomponent = [self supercomponent];
    if(supercomponent){
        [supercomponent setNeedsLayout];
    }
}

- (BOOL)needsLayout
{
    return _isLayoutDirty;
}

- (CGSize (^)(CGSize))measureBlock
{
    return nil;
}

- (void)layoutDidFinish
{
    WXAssertMainThread();
}

#pragma mark Private

- (void)_initCSSNodeWithStyles:(NSDictionary *)styles
{
    _cssNode = new_css_node();
    
    _cssNode->print = cssNodePrint;
    _cssNode->get_child = cssNodeGetChild;
    _cssNode->is_dirty = cssNodeIsDirty;
    if ([self measureBlock]) {
        _cssNode->measure = cssNodeMeasure;
    }
    _cssNode->context = (__bridge void *)self;
    
    [self _recomputeCSSNodeChildren];
    [self _fillCSSNode:styles];
    
    // To be in conformity with Android/Web, hopefully remove this in the future.
    if ([self.ref isEqualToString:WX_SDK_ROOT_REF]) {
        if (isUndefined(_cssNode->style.dimensions[CSS_HEIGHT]) && self.weexInstance.frame.size.height) {
            _cssNode->style.dimensions[CSS_HEIGHT] = self.weexInstance.frame.size.height;
        }
        
        if (isUndefined(_cssNode->style.dimensions[CSS_WIDTH]) && self.weexInstance.frame.size.width) {
            _cssNode->style.dimensions[CSS_WIDTH] = self.weexInstance.frame.size.width;
        }
    }
}

- (void)_updateCSSNodeStyles:(NSDictionary *)styles
{
    [self _fillCSSNode:styles];
}

-(void)_resetCSSNodeStyles:(NSArray *)styles
{
    [self _resetCSSNode:styles];
}

- (void)_recomputeCSSNodeChildren
{
    _cssNode->children_count = (int)[self _childrenCountForLayout];
}

- (NSUInteger)_childrenCountForLayout
{
    NSArray *subcomponents = _subcomponents;
    NSUInteger count = subcomponents.count;
    for (WXComponent *component in subcomponents) {
        if (!component->_isNeedJoinLayoutSystem) {
            count--;
        }
    }
    return (int)(count);
}


#pragma mark LayoutAnimationDisplayLink
- (void)_startLayoutAnimationDisplayLink
{
    WXAssertComponentThread();
    if (!_layoutAnimationDisplayLink) {
        _layoutAnimationDisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(_handleLayoutAnimationDisplayLink)];
        [_layoutAnimationDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
    else{
        [self _awakeLayoutAnimationDisplayLink];
    }
    
}

- (void)_stopLayoutAnimationDisplayLink
{
    WXAssertComponentThread();
    if (_layoutAnimationDisplayLink) {
        [_layoutAnimationDisplayLink invalidate];
        _layoutAnimationDisplayLink = nil;
    }
}

- (void)_suspendLayoutAnimationDisplayLink
{
    WXAssertComponentThread();
    if(_layoutAnimationDisplayLink && !_layoutAnimationDisplayLink.paused)
    {
        _layoutAnimationDisplayLink.paused = YES;
    }
}


- (void)_awakeLayoutAnimationDisplayLink
{
    WXAssertComponentThread();
    if (_layoutAnimationDisplayLink && _layoutAnimationDisplayLink.paused) {
        _layoutAnimationDisplayLink.paused = NO;
    }
}

- (void)_handleLayoutAnimationDisplayLink
{
    WXAssertComponentThread();
    int count = _layoutAnimationDuration * 60 / 1000;
    if (_layoutAnimationCount >= count) {
        [self _suspendLayoutAnimationDisplayLink];
        [self _resetProcessAnimationParameter];
        return;
    }
    else
    {
        [self _calculateLayoutAnimationProcessingStyle];
    }
    _layoutAnimationCount ++;
}


- (void)_resetProcessAnimationParameter
{
    
    _layoutAnimationCount = 0;
    _layoutAnimationDuration = 0;
    _widthInfo = nil;
    _heightInfo = nil;
    _leftInfo = nil;
    _rightInfo = nil;
    _topInfo = nil;
    _bottomInfo = nil;
}

- (void)_handleLayoutAnimationWithStyles:(NSDictionary *)styles
{
    [self _suspendLayoutAnimationDisplayLink];
    
    if (!_addStyles) {
        _fromStyles = [NSMutableDictionary dictionaryWithDictionary:self.styles];
        _addStyles = [NSMutableDictionary dictionaryWithDictionary:styles];
    }
    else
    {
        [_addStyles addEntriesFromDictionary:styles];
    }//保证_addStyles是唯一的
    
    _toStyles = [NSMutableDictionary dictionaryWithDictionary:_fromStyles];
    [_toStyles addEntriesFromDictionary:_addStyles];
    
    _layoutAnimationDuration = _fromStyles[@"transitionDuration"] ? [WXConvert CGFloat:_fromStyles[@"transitionDuration"]] : 0;
    _layoutAnimationDelay = _fromStyles[@"transitionDelay"] ? [WXConvert CGFloat:_fromStyles[@"transitionDelay"]] : 0;
    _layoutAnimationTimingFunction = [WXConvert CAMediaTimingFunction:_fromStyles[@"transitionTimingFunction"]];
    
    
    if (_layoutAnimationDuration == 0) {
        [self _fillCSSNode:styles];
        return;//如果duration为零直接关闭动画效果
    }
    
    if (![[NSString stringWithFormat:@"%@",_layoutAnimationTimingFunction] isEqualToString: kCAMediaTimingFunctionLinear]) {
        float vec[4] = {0.};
        [_layoutAnimationTimingFunction getControlPointAtIndex:1 values:&vec[0]];
        [_layoutAnimationTimingFunction getControlPointAtIndex:2 values:&vec[2]];
        [self unitBezierp1x:vec[0] p1y:vec[1] p2x:vec[2] p2y:vec[3]];
    }
    
    
    NSString *layoutAnimationProperty = _fromStyles[@"transitionProperty"];
    if ([layoutAnimationProperty containsString:@"width"]) {
        _widthInfo = [WXLayoutAnimationInfo new];
        _widthInfo.isAnimated = YES;
        _widthInfo.fromValue = @(_fromStyles[@"width"] ? [WXConvert CGFloat:_fromStyles[@"width" ]] : 0);
        _widthInfo.toValue = @(_toStyles[@"width"] ? [WXConvert CGFloat:_toStyles[@"width"]] : 0 );
        _widthInfo.perValue = @([_widthInfo.toValue doubleValue] - [_widthInfo.fromValue doubleValue]);
        
    }
    if ([layoutAnimationProperty containsString:@"height"])
    {
        _heightInfo = [WXLayoutAnimationInfo new];
        _heightInfo.isAnimated = YES;
        _heightInfo.fromValue = @(_fromStyles[@"height"] ? [WXConvert CGFloat:_fromStyles[@"height" ]] : 0);
        _heightInfo.toValue = @(_toStyles[@"height"] ? [WXConvert CGFloat:_toStyles[@"height"]] : 0 );
        _heightInfo.perValue = @([_heightInfo.toValue doubleValue] - [_heightInfo.fromValue doubleValue]);
        
    }
    if ([layoutAnimationProperty containsString:@"left"])
    {
        _leftInfo = [WXLayoutAnimationInfo new];
        _leftInfo.isAnimated = YES;
        _leftInfo.fromValue = @(_fromStyles[@"left"] ? [WXConvert CGFloat:_fromStyles[@"left" ]] : 0);
        _leftInfo.toValue = @(_toStyles[@"left"] ? [WXConvert CGFloat:_toStyles[@"left"]] : 0 );
        _leftInfo.perValue = @([_leftInfo.toValue doubleValue] - [_leftInfo.fromValue doubleValue]);
        
    }
    if ([layoutAnimationProperty containsString:@"right"])
    {
        _rightInfo = [WXLayoutAnimationInfo new];
        _rightInfo.isAnimated = YES;
        _rightInfo.fromValue = @(_fromStyles[@"right"] ? [WXConvert CGFloat:_fromStyles[@"right" ]] : 0);
        _rightInfo.toValue = @(_toStyles[@"right"] ? [WXConvert CGFloat:_toStyles[@"right"]] : 0 );
        _rightInfo.perValue = @([_rightInfo.toValue doubleValue] - [_rightInfo.fromValue doubleValue]);
        
    }
    if ([layoutAnimationProperty containsString:@"top"])
    {
        _topInfo = [WXLayoutAnimationInfo new];
        _topInfo.isAnimated = YES;
        _topInfo.fromValue = @(_fromStyles[@"top"] ? [WXConvert CGFloat:_fromStyles[@"top" ]] : 0);
        _topInfo.toValue = @(_toStyles[@"top"] ? [WXConvert CGFloat:_toStyles[@"top"]] : 0 );
        _topInfo.perValue = @([_topInfo.toValue doubleValue] - [_topInfo.fromValue doubleValue]);
        
    }
    if ([layoutAnimationProperty containsString:@"bottom"])
    {
        _bottomInfo = [WXLayoutAnimationInfo new];
        _bottomInfo.isAnimated = YES;
        _bottomInfo.fromValue = @(_fromStyles[@"bottom"] ? [WXConvert CGFloat:_fromStyles[@"bottom" ]] : 0);
        _bottomInfo.toValue = @(_toStyles[@"bottom"] ? [WXConvert CGFloat:_toStyles[@"bottom"]] : 0 );
        _bottomInfo.perValue = @([_widthInfo.toValue doubleValue] - [_bottomInfo.fromValue doubleValue]);
        
    }
    if ([layoutAnimationProperty containsString:@"transform"]) {
        
        
    }
    
    
    [self performSelector:@selector(_startLayoutAnimationDisplayLink) withObject:self afterDelay:_layoutAnimationDelay/1000];
    //    [self _startLayoutAnimationDisplayLink];
    
}


- (void)_calculateLayoutAnimationProcessingStyle
{
    //linear 在做贝塞尔曲线模型
    double per = 1000 * (_layoutAnimationCount + 1 ) / (60 * _layoutAnimationDuration);//linear
    if (![[NSString stringWithFormat:@"%@",_layoutAnimationTimingFunction] isEqualToString: kCAMediaTimingFunctionLinear]) {
        per = [self solveWithx:((_layoutAnimationCount+2)*16)/_layoutAnimationDuration epsilon:SOLVE_EPS(_layoutAnimationDuration)];
    }
    
    double currentWidth = [_widthInfo.fromValue doubleValue] + [_widthInfo.perValue doubleValue] * per;
    double currentHeight = [_heightInfo.fromValue doubleValue] + [_heightInfo.perValue doubleValue] * per;
    double currentLeft = [_leftInfo.fromValue doubleValue] + [_leftInfo.perValue doubleValue] * per;
    double currentRight = [_rightInfo.fromValue doubleValue] + [_rightInfo.perValue doubleValue] * per;
    double currentTop = [_topInfo.fromValue doubleValue] + [_topInfo.perValue doubleValue] * per;
    double currentBottom = [_bottomInfo.fromValue doubleValue] + [_bottomInfo.perValue doubleValue] * per;
    
    
    
    _widthInfo.isAnimated ? [_fromStyles setObject:@(currentWidth) forKey:@"width"]:0;
    _heightInfo.isAnimated ? [_fromStyles setObject:@(currentHeight) forKey:@"height"]:0;
    _leftInfo.isAnimated ? [_fromStyles setObject:@(currentLeft) forKey:@"left"]:0;
    _rightInfo.isAnimated ? [_fromStyles setObject:@(currentRight) forKey:@"right"]:0;
    _topInfo.isAnimated ? [_fromStyles setObject:@(currentTop) forKey:@"top"]:0;
    _bottomInfo.isAnimated ? [_fromStyles setObject:@(currentBottom) forKey:@"bottom"]:0;
    
    NSLog(@"%@",_fromStyles);
    
    [self _fillCSSNode:_fromStyles];
    
}




- (void)_frameDidCalculated:(BOOL)isChanged
{
    WXAssertComponentThread();
    
    if ([self isViewLoaded] && isChanged && [self isViewFrameSyncWithCalculated]) {
        
        __weak typeof(self) weakSelf = self;
        [self.weexInstance.componentManager _addUITask:^{
            __strong typeof(weakSelf) strongSelf = weakSelf;
            if (strongSelf->_transform && !CATransform3DEqualToTransform(strongSelf.layer.transform, CATransform3DIdentity)) {
                // From the UIView's frame documentation:
                // https://developer.apple.com/reference/uikit/uiview#//apple_ref/occ/instp/UIView/frame
                // Warning : If the transform property is not the identity transform, the value of this property is undefined and therefore should be ignored.
                // So layer's transform must be reset to CATransform3DIdentity before setFrame, otherwise frame will be incorrect
                strongSelf.layer.transform = CATransform3DIdentity;
            }
            
            if (!CGRectEqualToRect(strongSelf.view.frame,strongSelf.calculatedFrame)) {
                strongSelf.view.frame = strongSelf.calculatedFrame;
                strongSelf->_absolutePosition = CGPointMake(NAN, NAN);
                [strongSelf configBoxShadow:_boxShadow];
            } else {
                if (![strongSelf equalBoxShadow:_boxShadow withBoxShadow:_lastBoxShadow]) {
                    [strongSelf configBoxShadow:_boxShadow];
                }
            }
            
            [self _resetNativeBorderRadius];
            
            if (strongSelf->_transform) {
                [strongSelf->_transform applyTransformForView:strongSelf.view];
            }
            
            if (strongSelf->_backgroundImage) {
                [strongSelf setGradientLayer];
            }
            [strongSelf setNeedsDisplay];
        }];
    }
}

- (void)_calculateFrameWithSuperAbsolutePosition:(CGPoint)superAbsolutePosition
                           gatherDirtyComponents:(NSMutableSet<WXComponent *> *)dirtyComponents
{
    WXAssertComponentThread();
    
    if (!_cssNode->layout.should_update) {
        return;
    }
    _cssNode->layout.should_update = false;
    _isLayoutDirty = NO;
    
    CGRect newFrame = CGRectMake(WXRoundPixelValue(_cssNode->layout.position[CSS_LEFT]),
                                 WXRoundPixelValue(_cssNode->layout.position[CSS_TOP]),
                                 WXRoundPixelValue(_cssNode->layout.dimensions[CSS_WIDTH]),
                                 WXRoundPixelValue(_cssNode->layout.dimensions[CSS_HEIGHT]));
    
    BOOL isFrameChanged = NO;
    if (!CGRectEqualToRect(newFrame, _calculatedFrame)) {
        isFrameChanged = YES;
        _calculatedFrame = newFrame;
        [dirtyComponents addObject:self];
    }
    
    CGPoint newAbsolutePosition = [self computeNewAbsolutePosition:superAbsolutePosition];
    
    _cssNode->layout.dimensions[CSS_WIDTH] = CSS_UNDEFINED;
    _cssNode->layout.dimensions[CSS_HEIGHT] = CSS_UNDEFINED;
    _cssNode->layout.position[CSS_LEFT] = 0;
    _cssNode->layout.position[CSS_TOP] = 0;
    
    [self _frameDidCalculated:isFrameChanged];
    
    for (WXComponent *subcomponent in _subcomponents) {
        [subcomponent _calculateFrameWithSuperAbsolutePosition:newAbsolutePosition gatherDirtyComponents:dirtyComponents];
    }
}

- (CGPoint)computeNewAbsolutePosition:(CGPoint)superAbsolutePosition
{
    // Not need absolutePosition any more
    return superAbsolutePosition;
}

- (void)_layoutDidFinish
{
    WXAssertMainThread();
    
    if (_positionType == WXPositionTypeSticky) {
        [self.ancestorScroller adjustSticky];
    }
    [self layoutDidFinish];
}

#define WX_STYLE_FILL_CSS_NODE(key, cssProp, type)\
do {\
    id value = styles[@#key];\
    if (value) {\
        typeof(_cssNode->style.cssProp) convertedValue = (typeof(_cssNode->style.cssProp))[WXConvert type:value];\
        _cssNode->style.cssProp = convertedValue;\
        [self setNeedsLayout];\
    }\
} while(0);

#define WX_STYLE_FILL_CSS_NODE_PIXEL(key, cssProp)\
do {\
    id value = styles[@#key];\
    if (value) {\
        CGFloat pixel = [self WXPixelType:value];\
        if (isnan(pixel)) {\
            WXLogError(@"Invalid NaN value for style:%@, ref:%@", @#key, self.ref);\
        } else {\
            _cssNode->style.cssProp = pixel;\
            [self setNeedsLayout];\
        }\
    }\
} while(0);

#define WX_STYLE_FILL_CSS_NODE_ALL_DIRECTION(key, cssProp)\
do {\
    WX_STYLE_FILL_CSS_NODE_PIXEL(key, cssProp[CSS_TOP])\
    WX_STYLE_FILL_CSS_NODE_PIXEL(key, cssProp[CSS_LEFT])\
    WX_STYLE_FILL_CSS_NODE_PIXEL(key, cssProp[CSS_RIGHT])\
    WX_STYLE_FILL_CSS_NODE_PIXEL(key, cssProp[CSS_BOTTOM])\
} while(0);


- (CGFloat)WXPixelType:(id)value
{
    return [WXConvert WXPixelType:value scaleFactor:self.weexInstance.pixelScaleFactor];
}

- (void)_fillCSSNode:(NSDictionary *)styles
{
    // flex
    WX_STYLE_FILL_CSS_NODE(flex, flex, CGFloat)
    WX_STYLE_FILL_CSS_NODE(flexDirection, flex_direction, css_flex_direction_t)
    WX_STYLE_FILL_CSS_NODE(alignItems, align_items, css_align_t)
    WX_STYLE_FILL_CSS_NODE(alignSelf, align_self, css_align_t)
    WX_STYLE_FILL_CSS_NODE(flexWrap, flex_wrap, css_wrap_type_t)
    WX_STYLE_FILL_CSS_NODE(justifyContent, justify_content, css_justify_t)
    
    // position
    WX_STYLE_FILL_CSS_NODE(position, position_type, css_position_type_t)
    WX_STYLE_FILL_CSS_NODE_PIXEL(top, position[CSS_TOP])
    WX_STYLE_FILL_CSS_NODE_PIXEL(left, position[CSS_LEFT])
    WX_STYLE_FILL_CSS_NODE_PIXEL(right, position[CSS_RIGHT])
    WX_STYLE_FILL_CSS_NODE_PIXEL(bottom, position[CSS_BOTTOM])
    
    // dimension
    WX_STYLE_FILL_CSS_NODE_PIXEL(width, dimensions[CSS_WIDTH])
    WX_STYLE_FILL_CSS_NODE_PIXEL(height, dimensions[CSS_HEIGHT])
    WX_STYLE_FILL_CSS_NODE_PIXEL(minWidth, minDimensions[CSS_WIDTH])
    WX_STYLE_FILL_CSS_NODE_PIXEL(minHeight, minDimensions[CSS_HEIGHT])
    WX_STYLE_FILL_CSS_NODE_PIXEL(maxWidth, maxDimensions[CSS_WIDTH])
    WX_STYLE_FILL_CSS_NODE_PIXEL(maxHeight, maxDimensions[CSS_HEIGHT])
    
    // margin
    WX_STYLE_FILL_CSS_NODE_ALL_DIRECTION(margin, margin)
    WX_STYLE_FILL_CSS_NODE_PIXEL(marginTop, margin[CSS_TOP])
    WX_STYLE_FILL_CSS_NODE_PIXEL(marginLeft, margin[CSS_LEFT])
    WX_STYLE_FILL_CSS_NODE_PIXEL(marginRight, margin[CSS_RIGHT])
    WX_STYLE_FILL_CSS_NODE_PIXEL(marginBottom, margin[CSS_BOTTOM])
    
    // border
    WX_STYLE_FILL_CSS_NODE_ALL_DIRECTION(borderWidth, border)
    WX_STYLE_FILL_CSS_NODE_PIXEL(borderTopWidth, border[CSS_TOP])
    WX_STYLE_FILL_CSS_NODE_PIXEL(borderLeftWidth, border[CSS_LEFT])
    WX_STYLE_FILL_CSS_NODE_PIXEL(borderRightWidth, border[CSS_RIGHT])
    WX_STYLE_FILL_CSS_NODE_PIXEL(borderBottomWidth, border[CSS_BOTTOM])
    
    // padding
    WX_STYLE_FILL_CSS_NODE_ALL_DIRECTION(padding, padding)
    WX_STYLE_FILL_CSS_NODE_PIXEL(paddingTop, padding[CSS_TOP])
    WX_STYLE_FILL_CSS_NODE_PIXEL(paddingLeft, padding[CSS_LEFT])
    WX_STYLE_FILL_CSS_NODE_PIXEL(paddingRight, padding[CSS_RIGHT])
    WX_STYLE_FILL_CSS_NODE_PIXEL(paddingBottom, padding[CSS_BOTTOM])
}

#define WX_STYLE_RESET_CSS_NODE(key, cssProp, defaultValue)\
do {\
    if (styles && [styles containsObject:@#key]) {\
        _cssNode->style.cssProp = defaultValue;\
        [self setNeedsLayout];\
    }\
} while(0);

#define WX_STYLE_RESET_CSS_NODE_ALL_DIRECTION(key, cssProp, defaultValue)\
do {\
    WX_STYLE_RESET_CSS_NODE(key, cssProp[CSS_TOP], defaultValue)\
    WX_STYLE_RESET_CSS_NODE(key, cssProp[CSS_LEFT], defaultValue)\
    WX_STYLE_RESET_CSS_NODE(key, cssProp[CSS_RIGHT], defaultValue)\
    WX_STYLE_RESET_CSS_NODE(key, cssProp[CSS_BOTTOM], defaultValue)\
} while(0);

- (void)_resetCSSNode:(NSArray *)styles;
{
    // flex
    WX_STYLE_RESET_CSS_NODE(flex, flex, 0.0)
    WX_STYLE_RESET_CSS_NODE(flexDirection, flex_direction, CSS_FLEX_DIRECTION_COLUMN)
    WX_STYLE_RESET_CSS_NODE(alignItems, align_items, CSS_ALIGN_STRETCH)
    WX_STYLE_RESET_CSS_NODE(alignSelf, align_self, CSS_ALIGN_AUTO)
    WX_STYLE_RESET_CSS_NODE(flexWrap, flex_wrap, CSS_NOWRAP)
    WX_STYLE_RESET_CSS_NODE(justifyContent, justify_content, CSS_JUSTIFY_FLEX_START)

    // position
    WX_STYLE_RESET_CSS_NODE(position, position_type, CSS_POSITION_RELATIVE)
    WX_STYLE_RESET_CSS_NODE(top, position[CSS_TOP], CSS_UNDEFINED)
    WX_STYLE_RESET_CSS_NODE(left, position[CSS_LEFT], CSS_UNDEFINED)
    WX_STYLE_RESET_CSS_NODE(right, position[CSS_RIGHT], CSS_UNDEFINED)
    WX_STYLE_RESET_CSS_NODE(bottom, position[CSS_BOTTOM], CSS_UNDEFINED)
    
    // dimension
    WX_STYLE_RESET_CSS_NODE(width, dimensions[CSS_WIDTH], CSS_UNDEFINED)
    WX_STYLE_RESET_CSS_NODE(height, dimensions[CSS_HEIGHT], CSS_UNDEFINED)
    WX_STYLE_RESET_CSS_NODE(minWidth, minDimensions[CSS_WIDTH], CSS_UNDEFINED)
    WX_STYLE_RESET_CSS_NODE(minHeight, minDimensions[CSS_HEIGHT], CSS_UNDEFINED)
    WX_STYLE_RESET_CSS_NODE(maxWidth, maxDimensions[CSS_WIDTH], CSS_UNDEFINED)
    WX_STYLE_RESET_CSS_NODE(maxHeight, maxDimensions[CSS_HEIGHT], CSS_UNDEFINED)
    
    // margin
    WX_STYLE_RESET_CSS_NODE_ALL_DIRECTION(margin, margin, 0.0)
    WX_STYLE_RESET_CSS_NODE(marginTop, margin[CSS_TOP], 0.0)
    WX_STYLE_RESET_CSS_NODE(marginLeft, margin[CSS_LEFT], 0.0)
    WX_STYLE_RESET_CSS_NODE(marginRight, margin[CSS_RIGHT], 0.0)
    WX_STYLE_RESET_CSS_NODE(marginBottom, margin[CSS_BOTTOM], 0.0)
    
    // border
    WX_STYLE_RESET_CSS_NODE_ALL_DIRECTION(borderWidth, border, 0.0)
    WX_STYLE_RESET_CSS_NODE(borderTopWidth, border[CSS_TOP], 0.0)
    WX_STYLE_RESET_CSS_NODE(borderLeftWidth, border[CSS_LEFT], 0.0)
    WX_STYLE_RESET_CSS_NODE(borderRightWidth, border[CSS_RIGHT], 0.0)
    WX_STYLE_RESET_CSS_NODE(borderBottomWidth, border[CSS_BOTTOM], 0.0)
    
    // padding
    WX_STYLE_RESET_CSS_NODE_ALL_DIRECTION(padding, padding, 0.0)
    WX_STYLE_RESET_CSS_NODE(paddingTop, padding[CSS_TOP], 0.0)
    WX_STYLE_RESET_CSS_NODE(paddingLeft, padding[CSS_LEFT], 0.0)
    WX_STYLE_RESET_CSS_NODE(paddingRight, padding[CSS_RIGHT], 0.0)
    WX_STYLE_RESET_CSS_NODE(paddingBottom, padding[CSS_BOTTOM], 0.0)
}

#pragma mark CSS Node Override

static void cssNodePrint(void *context)
{
    WXComponent *component = (__bridge WXComponent *)context;
    // TODO:
    printf("%s:%s ", component.ref.UTF8String, component->_type.UTF8String);
}

static css_node_t * cssNodeGetChild(void *context, int i)
{
    WXComponent *component = (__bridge WXComponent *)context;
    NSArray *subcomponents = component->_subcomponents;
    for (int j = 0; j <= i && j < subcomponents.count; j++) {
        WXComponent *child = subcomponents[j];
        if (!child->_isNeedJoinLayoutSystem) {
            i++;
        }
    }
    
    if(i >= 0 && i < subcomponents.count){
        WXComponent *child = subcomponents[i];
        return child->_cssNode;
    }
    
    
    WXAssert(NO, @"Can not find component:%@'s css node child at index: %ld, totalCount:%ld", component, i, subcomponents.count);
    return NULL;
}

static bool cssNodeIsDirty(void *context)
{
    WXAssertComponentThread();
    
    WXComponent *component = (__bridge WXComponent *)context;
    BOOL needsLayout = [component needsLayout];
    
    return needsLayout;
}

static css_dim_t cssNodeMeasure(void *context, float width, css_measure_mode_t widthMode, float height, css_measure_mode_t heightMode)
{
    WXComponent *component = (__bridge WXComponent *)context;
    CGSize (^measureBlock)(CGSize) = [component measureBlock];
    
    if (!measureBlock) {
        return (css_dim_t){NAN, NAN};
    }
    
    CGSize constrainedSize = CGSizeMake(width, height);
    CGSize resultSize = measureBlock(constrainedSize);
    
    return (css_dim_t){resultSize.width, resultSize.height};
}

//贝塞尔曲线计算
- (void)unitBezierp1x:(double)p1x p1y:(double)p1y p2x:(double)p2x p2y:(double)p2y
{
    cx = 3.0 * p1x;
    bx = 3.0 * (p2x - p1x) - cx;
    ax = 1.0 - cx -bx;
    
    cy = 3.0 * p1y;
    by = 3.0 * (p2y - p1y) - cy;
    ay = 1.0 - cy - by;
}
- (double)sampleCurveX:(double)t
{
    return ((ax * t + bx) * t + cx) * t;
}

- (double)sampleCurveY:(double)t
{
    return ((ay * t + by) * t + cy) * t;
}

- (double)sampleCurveDerivativeX:(double)t
{
    return (3.0 * ax * t + 2.0 * bx) * t + cx;
}

- (double)solveCurveX:(double)x epsilon:(double)epsilon
{
    double t0;
    double t1;
    double t2;
    double x2;
    double d2;
    int i;
    
    for (t2 = x, i = 0; i < 8; i++) {
        x2 = [self sampleCurveX:t2] - x;
        if (fabs (x2) < epsilon)
            return t2;
        d2 = [self sampleCurveDerivativeX:t2];
        if (fabs(d2) < 1e-6)
            break;
        t2 = t2 - x2 / d2;
    }
    t0 = 0.0;
    t1 = 1.0;
    t2 = x;
    
    if (t2 < t0)
        return t0;
    if (t2 > t1)
        return t1;
    
    while (t0 < t1) {
        x2 = [self sampleCurveX:t2];
        if (fabs(x2 - x) < epsilon)
            return t2;
        if (x > x2)
            t0 = t2;
        else
            t1 = t2;
        t2 = (t1 - t0) * .5 + t0;
    }
    return t2;
}

- (double)solveWithx:(double)x epsilon:(double)epsilon
{
    return [self sampleCurveY:([self solveCurveX:x epsilon:epsilon])];
}



@end
