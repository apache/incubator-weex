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
    if ([self measureBlock]) {
        YGNodeMarkDirty(self.cssNode);
    }
    YGNodeSetHasNewLayout(self.cssNode, true);
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
    _cssNode = YGNodeNewWithConfig([[self class] yogaConfig]);
    YGNodeSetPrintFunc(_cssNode, cssNodePrint);
//    _cssNode->get_child = cssNodeGetChild;
//    _cssNode->is_dirty = cssNodeIsDirty;
    if ([self measureBlock]) {
        YGNodeSetMeasureFunc(_cssNode, cssNodeMeasure);
    }
    YGNodeSetContext(_cssNode, (__bridge void *)self);
    [self _recomputeCSSNodeChildren];
    [self _fillCSSNode:styles];
    
    // To be in conformity with Android/Web, hopefully remove this in the future.
    if ([self.ref isEqualToString:WX_SDK_ROOT_REF]) {
        if (YGFloatIsUndefined(YGNodeStyleGetHeight(_cssNode).value) && self.weexInstance.frame.size.height) {
            YGNodeStyleSetHeight(_cssNode, self.weexInstance.frame.size.height);
        }
        
        if (YGFloatIsUndefined(YGNodeStyleGetWidth(_cssNode).value) && self.weexInstance.frame.size.width) {
            YGNodeStyleSetWidth(_cssNode, self.weexInstance.frame.size.width);
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
//    _cssNode->children_count = (int)[self _childrenCountForLayout];
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
    if (!YGNodeGetHasNewLayout(_cssNode)) {
        return;
    }
    YGNodeSetHasNewLayout(_cssNode, false);
    
    if (YGNodeStyleGetDisplay(_cssNode) == YGDisplayNone) {
        return;
    }
    _isLayoutDirty = NO;
    CGRect newFrame = CGRectZero;
    newFrame = CGRectMake(WXRoundPixelValue(YGFloatIsUndefined(YGNodeLayoutGetLeft(_cssNode))?0:YGNodeLayoutGetLeft(_cssNode)),
                          WXRoundPixelValue((YGFloatIsUndefined(YGNodeLayoutGetTop(_cssNode)))?0:YGNodeLayoutGetTop(_cssNode)),
                          WXRoundPixelValue(YGFloatIsUndefined(YGNodeLayoutGetWidth(_cssNode))?0:YGNodeLayoutGetWidth(_cssNode)),
                          WXRoundPixelValue(YGFloatIsUndefined(YGNodeLayoutGetHeight(_cssNode))?0:YGNodeLayoutGetHeight(_cssNode)));
    
    BOOL isFrameChanged = NO;
    if (!CGRectEqualToRect(newFrame, _calculatedFrame)) {
        isFrameChanged = YES;
        _calculatedFrame = newFrame;
        [dirtyComponents addObject:self];
    }
    
    CGPoint newAbsolutePosition = [self computeNewAbsolutePosition:superAbsolutePosition];
    
//    _cssNode->layout.dimensions[CSS_WIDTH] = CSS_UNDEFINED;
//    _cssNode->layout.dimensions[CSS_HEIGHT] = CSS_UNDEFINED;
//    _cssNode->layout.position[CSS_LEFT] = 0;
//    _cssNode->layout.position[CSS_TOP] = 0;
    
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

- (CGFloat)WXPixelType:(id)value
{
    return [WXConvert WXPixelType:value scaleFactor:self.weexInstance.pixelScaleFactor];
}

- (void)_fillCSSNode:(NSDictionary *)styles
{
    // flex
    if (styles[@"flex"]) {
        YGNodeStyleSetFlex(_cssNode, [WXConvert CGFloat:styles[@"flex"]]);
    } else {
        // to make the default flex value is zero, yoga is nan, maybe this can configured by yoga config
        YGNodeStyleSetFlex(_cssNode, 0);
    }
    if (styles[@"flexDirection"]) {
        YGNodeStyleSetFlexDirection(_cssNode, [WXConvert YGFlexDirection:styles[@"flexDirection"]]);
    }
    if (styles[@"alignItems"]) {
        YGNodeStyleSetAlignItems(_cssNode, [WXConvert YGAlign:styles[@"alignItems"]]);
    }
    if (styles[@"alignSelf"]) {
        YGNodeStyleSetAlignSelf(_cssNode, [WXConvert YGAlign:styles[@"alignSelf"]]);
    }
    if (styles[@"flexWrap"]) {
        YGNodeStyleSetFlexWrap(_cssNode, [WXConvert YGWrap:styles[@"flexWrap"]]);
    }
    if (styles[@"justifyContent"]) {
        YGNodeStyleSetJustifyContent(_cssNode, [WXConvert YGJustify:styles[@"justifyContent"]]);
    }
    
    // position
    if (styles[@"position"]) {
        YGNodeStyleSetPositionType(_cssNode, [WXConvert YGPositionType:styles[@"position"]]);
    }
    if (styles[@"top"]) {
        YGNodeStyleSetPosition(_cssNode, YGEdgeTop, [WXConvert WXPixelType:styles[@"top"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"left"]) {
        YGNodeStyleSetPosition(_cssNode, YGEdgeLeft, [WXConvert WXPixelType:styles[@"left"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if(styles[@"right"]) {
        YGNodeStyleSetPosition(_cssNode, YGEdgeRight, [WXConvert WXPixelType:styles[@"right"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"bottom"]) {
        YGNodeStyleSetPosition(_cssNode, YGEdgeBottom, [WXConvert WXPixelType:styles[@"bottom"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    
    // dimension
    if (styles[@"width"]) {
        YGNodeStyleSetWidth(_cssNode, [WXConvert WXPixelType:styles[@"width"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"height"]) {
        YGNodeStyleSetHeight(_cssNode, [WXConvert WXPixelType:styles[@"height"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"minWidth"]) {
        YGNodeStyleSetMinWidth(_cssNode, [WXConvert WXPixelType:styles[@"minWidth"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"minHeight"]) {
        YGNodeStyleSetMinHeight(_cssNode, [WXConvert WXPixelType:styles[@"minHeight"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"maxWidth"]) {
        YGNodeStyleSetMaxWidth(_cssNode, [WXConvert WXPixelType:styles[@"maxWidth"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"maxHeight"]) {
        YGNodeStyleSetMaxHeight(_cssNode, [WXConvert WXPixelType:styles[@"maxHeight"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    
    // margin
    if (styles[@"margin"]) {
        YGNodeStyleSetMargin(_cssNode, YGEdgeAll, [WXConvert WXPixelType:styles[@"margin"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"marginTop"]) {
        YGNodeStyleSetMargin(_cssNode, YGEdgeTop, [WXConvert WXPixelType:styles[@"marginTop"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"marginBottom"]) {
        YGNodeStyleSetMargin(_cssNode, YGEdgeBottom, [WXConvert WXPixelType:styles[@"marginBottom"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"marginRight"]) {
        YGNodeStyleSetMargin(_cssNode, YGEdgeRight, [WXConvert WXPixelType:styles[@"marginRight"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"marginLeft"]) {
        YGNodeStyleSetMargin(_cssNode, YGEdgeLeft, [WXConvert WXPixelType:styles[@"marginLeft"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    
    // border
    if (styles[@"border"]) {
        YGNodeStyleSetBorder(_cssNode, YGEdgeAll, [WXConvert WXPixelType:styles[@"border"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"borderTopWidth"]) {
        YGNodeStyleSetBorder(_cssNode, YGEdgeTop, [WXConvert WXPixelType:styles[@"borderTopWidth"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    
    if (styles[@"borderLeftWidth"]) {
        YGNodeStyleSetBorder(_cssNode, YGEdgeLeft, [WXConvert WXPixelType:styles[@"borderLeftWidth"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    
    if (styles[@"borderBottomWidth"]) {
        YGNodeStyleSetBorder(_cssNode, YGEdgeBottom, [WXConvert WXPixelType:styles[@"borderBottomWidth"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"borderRightWidth"]) {
        YGNodeStyleSetBorder(_cssNode, YGEdgeRight, [WXConvert WXPixelType:styles[@"borderRightWidth"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    
    // padding
    if (styles[@"padding"]) {
        YGNodeStyleSetPadding(_cssNode, YGEdgeAll, [WXConvert WXPixelType:styles[@"padding"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"paddingTop"]) {
        YGNodeStyleSetPadding(_cssNode, YGEdgeTop, [WXConvert WXPixelType:styles[@"paddingTop"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"paddingLeft"]) {
        YGNodeStyleSetPadding(_cssNode, YGEdgeLeft, [WXConvert WXPixelType:styles[@"paddingLeft"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"paddingBottom"]) {
        YGNodeStyleSetPadding(_cssNode, YGEdgeBottom, [WXConvert WXPixelType:styles[@"paddingBottom"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    if (styles[@"paddingRight"]) {
        YGNodeStyleSetPadding(_cssNode, YGEdgeRight, [WXConvert WXPixelType:styles[@"paddingRight"] scaleFactor:self.weexInstance.pixelScaleFactor]);
    }
    [self setNeedsLayout];
}

- (void)_resetCSSNode:(NSArray *)styles;
{
    // flex
    if ([styles containsObject:@"flex"]){
        YGNodeStyleSetFlex(_cssNode, 0);
    }
    if ([styles containsObject:@"flexDirection"]) {
        YGNodeStyleSetFlexDirection(_cssNode, YGFlexDirectionColumn);
    }
    if ([styles containsObject:@"alignItems"]) {
        YGNodeStyleSetAlignItems(_cssNode, YGAlignStretch);
    }
    if ([styles containsObject:@"alignSelf"]) {
        YGNodeStyleSetAlignSelf(_cssNode, YGAlignAuto);
    }
    if ([styles containsObject:@"flexWrap"]) {
        YGNodeStyleSetFlexWrap(_cssNode, YGWrapNoWrap);
    }
    if ([styles containsObject:@"justifyContent"]) {
        YGNodeStyleSetJustifyContent(_cssNode, YGJustifyFlexStart);
    }

    // position
    if ([styles containsObject:@"position"]) {
        YGNodeStyleSetPositionType(_cssNode, YGPositionTypeRelative);
    }
    if ([styles containsObject:@"top"]) {
        YGNodeStyleSetPosition(_cssNode, YGEdgeTop, YGUndefined);
    }
    if ([styles containsObject:@"left"]) {
        YGNodeStyleSetPosition(_cssNode, YGEdgeLeft, YGUndefined);
    }
    if ([styles containsObject:@"right"]) {
        YGNodeStyleSetPosition(_cssNode, YGEdgeRight, YGUndefined);
    }
    if([styles containsObject:@"botttom"]) {
        YGNodeStyleSetPosition(_cssNode, YGEdgeBottom, YGUndefined);
    }
    
    // dimension
    if ([styles containsObject:@"width"]) {
        YGNodeStyleSetWidth(_cssNode, YGUndefined);
    }
    if ([styles containsObject:@"height"]) {
        YGNodeStyleSetHeight(_cssNode, YGUndefined);
    }
    if ([styles containsObject:@"minWidth"]) {
        YGNodeStyleSetMinWidth(_cssNode, YGUndefined);
    }
    if ([styles containsObject:@"minHeight"]) {
        YGNodeStyleSetMinHeight(_cssNode, YGUndefined);
    }
    if ([styles containsObject:@"maxWidth"]) {
        YGNodeStyleSetMaxWidth(_cssNode, YGUndefined);
    }
    if ([styles containsObject:@"maxHeight"]) {
        YGNodeStyleSetMaxHeight(_cssNode, YGUndefined);
    }
    
    // margin
    if ([styles containsObject:@"margin"]) {
        YGNodeStyleSetMargin(_cssNode, YGEdgeAll, 0.0);
    }
    if ([styles containsObject:@"marginTop"]) {
        YGNodeStyleSetMargin(_cssNode, YGEdgeTop, 0.0);
    }
    if ([styles containsObject:@"marginLeft"]) {
        YGNodeStyleSetMargin(_cssNode, YGEdgeLeft, 0.0);
    }
    if ([styles containsObject:@"marginRight"]) {
        YGNodeStyleSetMargin(_cssNode, YGEdgeRight, 0.0);
    }
    if ([styles containsObject:@"marginBottom"]) {
        YGNodeStyleSetMargin(_cssNode, YGEdgeBottom, 0.0);
    }
    
    // border
    if ([styles containsObject:@"border"]) {
        YGNodeStyleSetBorder(_cssNode, YGEdgeAll, 0.0);
    }
    if ([styles containsObject:@"borderTopWidth"]) {
        YGNodeStyleSetBorder(_cssNode, YGEdgeTop, 0.0);
    }
    if ([styles containsObject:@"borderLeftWidth"]) {
        YGNodeStyleSetBorder(_cssNode, YGEdgeLeft, 0.0);
    }
    if ([styles containsObject:@"borderRightWidth"]) {
        YGNodeStyleSetBorder(_cssNode, YGEdgeRight, 0.0);
    }
    if ([styles containsObject:@"borderBottomWidth"]) {
        YGNodeStyleSetBorder(_cssNode, YGEdgeBottom, 0.0);
    }
    
    // padding
    if ([styles containsObject:@"padding"]) {
        YGNodeStyleSetPadding(_cssNode, YGEdgeAll, 0.0);
    }
    if ([styles containsObject:@"paddingTop"]) {
        YGNodeStyleSetPadding(_cssNode, YGEdgeTop, 0.0);
    }
    if ([styles containsObject:@"paddingLeft"]) {
        YGNodeStyleSetPadding(_cssNode, YGEdgeLeft, 0.0);
    }
    if ([styles containsObject:@"paddingRight"]) {
        YGNodeStyleSetPadding(_cssNode, YGEdgeLeft, 0.0);
    }
    if ([styles containsObject:@"paddingBottom"]) {
        YGNodeStyleSetPadding(_cssNode, YGEdgeBottom, 0.0);
    }
    
    [self setNeedsLayout];
}

#pragma mark CSS Node Override

static void cssNodePrint(YGNodeRef node)
{
    WXComponent *component = (__bridge WXComponent *)(YGNodeGetContext(node));
    // TODO:
    printf("%s:%s ", component.ref.UTF8String, component->_type.UTF8String);
}

//static css_node_t * cssNodeGetChild(void *context, int i)
//{
//    WXComponent *component = (__bridge WXComponent *)context;
//    NSArray *subcomponents = component->_subcomponents;
//    for (int j = 0; j <= i && j < subcomponents.count; j++) {
//        WXComponent *child = subcomponents[j];
//        if (!child->_isNeedJoinLayoutSystem) {
//            i++;
//        }
//    }
//    
//    if(i >= 0 && i < subcomponents.count){
//        WXComponent *child = subcomponents[i];
//        return child->_cssNode;
//    }
//    
//    
//    WXAssert(NO, @"Can not find component:%@'s css node child at index: %ld, totalCount:%ld", component, i, subcomponents.count);
//    return NULL;
//}

//static bool cssNodeIsDirty(void *context)
//{
//    WXAssertComponentThread();
//    
//    WXComponent *component = (__bridge WXComponent *)context;
//    BOOL needsLayout = [component needsLayout];
//    
//    return needsLayout;
//}


static YGSize cssNodeMeasure(YGNodeRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode)
{
    WXComponent *component = (__bridge WXComponent *)(YGNodeGetContext(node));
    CGSize (^measureBlock)(CGSize) = [component measureBlock];
    
    if (!measureBlock) {
        return (YGSize){NAN, NAN};
    }
    
    CGSize constrainedSize = CGSizeMake(width, height);
    CGSize resultSize = measureBlock(constrainedSize);
    
    return (YGSize){resultSize.width, resultSize.height};
}

@end
