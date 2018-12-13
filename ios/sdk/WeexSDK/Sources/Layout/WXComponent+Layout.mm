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
#import "WXSDKInstance_private.h"
#import "WXComponent+BoxShadow.h"
#import "WXLog.h"
#import "WXMonitor.h"
#import "WXSDKInstance_performance.h"
#import "WXCellComponent.h"
#import "WXCoreBridge.h"

bool flexIsUndefined(float value) {
    return isnan(value);
}

@implementation WXComponent (Layout)

#pragma clang diagnostic ignored "-Wobjc-protocol-method-implementation"

#pragma mark Public

- (void)setNeedsLayout
{
    if (_flexCssNode != nullptr) {
        _flexCssNode->markDirty();
    }
}

- (BOOL)needsLayout
{
    if (_flexCssNode != nullptr) {
        return _flexCssNode->isDirty();
    }
    else {
        return false;
    }
}

- (CGSize (^)(CGSize))measureBlock
{
    return nil;
}

- (void)layoutDidFinish
{
    WXAssertMainThread();
}

- (void)updateLayoutStyles:(NSDictionary*)styles
{
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge callUpdateStyle:self.weexInstance.instanceId ref:self.ref data:styles];
    });
}

#pragma mark Private

- (void)_setRenderObject:(void *)object
{
    if (object) {
        _flexCssNode = static_cast<WeexCore::WXCoreLayoutNode*>(object);
        _flexCssNode->setContext((__bridge void *)self); // bind
        if ([self measureBlock]) {
            _flexCssNode->setMeasureFunc(flexCssNodeMeasure);
        }
    }
    else if (_flexCssNode) {
        _flexCssNode->setContext(nullptr);
        _flexCssNode = nullptr;
    }
}

- (void)_updateCSSNodeStyles:(NSDictionary *)styles
{
    [self _fillCSSNode:styles isUpdate:YES];
}

-(void)_resetCSSNodeStyles:(NSArray *)styles
{
    [self _resetCSSNode:styles];
}

- (void)_frameDidCalculated:(BOOL)isChanged
{
    WXAssertComponentThread();
    if (isChanged && [self isKindOfClass:[WXCellComponent class]]) {
        CGFloat mainScreenWidth = [[UIScreen mainScreen] bounds].size.width;
        CGFloat mainScreenHeight = [[UIScreen mainScreen] bounds].size.height;
        if (mainScreenHeight/2 < _calculatedFrame.size.height && mainScreenWidth/2 < _calculatedFrame.size.width) {
            [self weexInstance].performance.cellExceedNum++;
            [self.weexInstance.apmInstance updateFSDiffStats:KEY_PAGE_STATS_CELL_EXCEED_NUM withDiffValue:1];
        }
    }
    
    if ([self isViewLoaded] && isChanged && [self isViewFrameSyncWithCalculated]) {
        
        __weak typeof(self) weakSelf = self;
        [self.weexInstance.componentManager _addUITask:^{
            __strong typeof(weakSelf) strongSelf = weakSelf;
            
            if (strongSelf == nil) {
                return;
            }
            
            // Check again incase that this property is set to NO in another UI task.
            if (![strongSelf isViewFrameSyncWithCalculated]) {
                return;
            }
            
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
                [strongSelf configBoxShadow:strongSelf->_boxShadow];
            } else {
                if (![strongSelf equalBoxShadow:strongSelf->_boxShadow withBoxShadow:strongSelf->_lastBoxShadow]) {
                    [strongSelf configBoxShadow:strongSelf->_boxShadow];
                }
            }
            
            [strongSelf _resetNativeBorderRadius];
            
            if ([WXUtility enableRTLLayoutDirection]) {
                if ([strongSelf isDirectionRTL] != strongSelf -> _isLastLayoutDirectionRTL) {
                    strongSelf -> _isLastLayoutDirectionRTL = [strongSelf isDirectionRTL];
                    [strongSelf _layoutDirectionDidChanged:[strongSelf isDirectionRTL]];
                }
            }
            
            if (strongSelf->_transform) {
                [strongSelf->_transform applyTransformForView:strongSelf.view];
            }
            
            [strongSelf _adjustForRTL];
            
            if (strongSelf->_backgroundImage) {
                [strongSelf setGradientLayer];
            }
            
            [strongSelf setNeedsDisplay];
        }];
    } else if ([WXUtility enableRTLLayoutDirection]) {
        // if frame is not change, we still need check was layoutDirection changed
        if ([self isDirectionRTL] != _isLastLayoutDirectionRTL) {
            self -> _isLastLayoutDirectionRTL = [self isDirectionRTL];
            __weak typeof(self) weakSelf = self;
            [self.weexInstance.componentManager _addUITask:^{
                __strong typeof(weakSelf) strongSelf = weakSelf;
                [strongSelf _layoutDirectionDidChanged:[strongSelf isDirectionRTL]];
                if (strongSelf->_transform) {
                    [strongSelf->_transform applyTransformForView:strongSelf.view];
                }
                [strongSelf _adjustForRTL];
            }];
        }
    }
}

- (void)_layoutDirectionDidChanged:(BOOL)isRTL {
    WXAssertMainThread();
    [self layoutDirectionDidChanged:isRTL];
}

- (void)layoutDirectionDidChanged:(BOOL)isRTL {
    
}

- (void)_layoutDidFinish
{
    WXAssertMainThread();

    if (_positionType == WXPositionTypeSticky) {
        [self.ancestorScroller adjustSticky];
    }

    [self layoutDidFinish];
}

#define WX_STYLE_FLEX_NODE_JUDGE_LEGAL(key) styles[key]&&!isnan([WXConvert WXPixelType:styles[key] scaleFactor:self.weexInstance.pixelScaleFactor])

- (CGFloat)WXPixelType:(id)value
{
    return [WXConvert WXPixelType:value scaleFactor:self.weexInstance.pixelScaleFactor];
}

- (void)_fillCSSNode:(NSDictionary *)styles isUpdate:(BOOL)isUpdate
{
    if (_flexCssNode == nullptr) {
        return;
    }

    BOOL needLayout = NO;
    
    // CSS direction for RTL Layout
    if (styles[@"direction"]) {
        _flexCssNode->setDirection([self fxDirection:styles[@"direction"]], isUpdate);
        needLayout = YES;
    }
    
    // flex
    if (styles[@"flex"]) {
        _flexCssNode->set_flex([WXConvert CGFloat:styles[@"flex"]]);
        needLayout = YES;
    }
    if (isnan(_flexCssNode->getFlex())) {
        // to make the default flex value is zero, yoga is nan, maybe this can configured by yoga config
        _flexCssNode->set_flex(0);
        needLayout = YES;
    }
    
    if (styles[@"flexDirection"]) {
        _flexCssNode->setFlexDirection([self fxFlexDirection:styles[@"flexDirection"]], isUpdate);
        needLayout = YES;
    }
    if (styles[@"alignItems"]) {
        _flexCssNode->setAlignItems([self fxAlign:styles[@"alignItems"]]);
        needLayout = YES;
    }
    if (styles[@"alignSelf"]) {
        _flexCssNode->setAlignSelf([self fxAlignSelf:styles[@"alignSelf"]]);
        needLayout = YES;
    }
    if (styles[@"flexWrap"]) {
        _flexCssNode->setFlexWrap([self fxWrap:styles[@"flexWrap"]]);
        needLayout = YES;
    }
    if (styles[@"justifyContent"]) {
        _flexCssNode->setJustifyContent([self fxJustify:styles[@"justifyContent"]]);
        needLayout = YES;
    }
    
    // position
    if (styles[@"position"]) {
        _flexCssNode->setStylePositionType([self fxPositionType:styles[@"position"]]);
        needLayout = YES;
    }
    if (styles[@"top"]) {
        _flexCssNode->setStylePosition(WeexCore::kPositionEdgeTop,
                                       [self judgePropValuePropValue:styles[@"top"] defaultValue:NAN]);
        needLayout = YES;
    }
    if (styles[@"left"]) {
        _flexCssNode->setStylePosition(WeexCore::kPositionEdgeLeft,
                                       [self judgePropValuePropValue:styles[@"left"] defaultValue:NAN]);
        needLayout = YES;
    }
    if(styles[@"right"]) {
        _flexCssNode->setStylePosition(WeexCore::kPositionEdgeRight,
                                       [self judgePropValuePropValue:styles[@"right"] defaultValue:NAN]);
        needLayout = YES;
    }
    if (styles[@"bottom"]) {
        _flexCssNode->setStylePosition(WeexCore::kPositionEdgeBottom,
                                       [self judgePropValuePropValue:styles[@"bottom"] defaultValue:NAN]);
        needLayout = YES;
    }
    
    // dimension
    if (styles[@"width"]) {
        _flexCssNode->setStyleWidth([self judgePropValuePropValue:styles[@"width"] defaultValue:NAN], isUpdate);
        needLayout = YES;
    }
    if (styles[@"height"]) {
        _flexCssNode->setStyleHeight([self judgePropValuePropValue:styles[@"height"] defaultValue:NAN]);
        needLayout = YES;
    }
    if (styles[@"minWidth"]) {
        _flexCssNode->setMinWidth([self judgePropValuePropValue:styles[@"minWidth"] defaultValue:NAN], isUpdate);
        needLayout = YES;
    }
    if (styles[@"minHeight"]) {
        _flexCssNode->setMinHeight([self judgePropValuePropValue:styles[@"minHeight"] defaultValue:NAN]);
        needLayout = YES;
    }
    if (styles[@"maxWidth"]) {
        _flexCssNode->setMaxWidth([self judgePropValuePropValue:styles[@"maxWidth"] defaultValue:NAN], isUpdate);
        needLayout = YES;
    }
    if (styles[@"maxHeight"]) {
        _flexCssNode->setMaxHeight([self judgePropValuePropValue:styles[@"maxHeight"] defaultValue:NAN]);
        needLayout = YES;
    }
    
    // margin
    if (styles[@"margin"]) {
        _flexCssNode->setMargin(WeexCore::kMarginALL,
                                [self judgePropValuePropValue:styles[@"margin"] defaultValue:0]);
        needLayout = YES;
    }
    if (styles[@"marginTop"]) {
        _flexCssNode->setMargin(WeexCore::kMarginTop,
                                [self judgePropValuePropValue:styles[@"marginTop"] defaultValue:0]);
        needLayout = YES;
    }
    if (styles[@"marginBottom"]) {
        _flexCssNode->setMargin(WeexCore::kMarginBottom,
                                [self judgePropValuePropValue:styles[@"marginBottom"] defaultValue:0]);
        needLayout = YES;
    }
    if (styles[@"marginRight"]) {
        _flexCssNode->setMargin(WeexCore::kMarginRight,
                                [self judgePropValuePropValue:styles[@"marginRight"] defaultValue:0]);
    }
    if (styles[@"marginLeft"]) {
        _flexCssNode->setMargin(WeexCore::kMarginLeft,
                                [self judgePropValuePropValue:styles[@"marginLeft"] defaultValue:0]);
        needLayout = YES;
    }
    
    // border
    if (styles[@"borderWidth"]) {
        _flexCssNode->setBorderWidth(WeexCore::kBorderWidthALL,
                                     [self judgePropValuePropValue:styles[@"borderWidth"] defaultValue:0]);
        needLayout = YES;
    }
    if (styles[@"borderTopWidth"]) {
        _flexCssNode->setBorderWidth(WeexCore::kBorderWidthTop,
                                     [self judgePropValuePropValue:styles[@"borderTopWidth"] defaultValue:0]);
        needLayout = YES;
    }
    
    if (styles[@"borderLeftWidth"]) {
        _flexCssNode->setBorderWidth(WeexCore::kBorderWidthLeft,
                                     [self judgePropValuePropValue:styles[@"borderLeftWidth"] defaultValue:0]);
        needLayout = YES;
    }
    
    if (styles[@"borderBottomWidth"]) {
        _flexCssNode->setBorderWidth(WeexCore::kBorderWidthBottom,
                                     [self judgePropValuePropValue:styles[@"borderBottomWidth"] defaultValue:0]);
        needLayout = YES;
    }
    if (styles[@"borderRightWidth"]) {
        _flexCssNode->setBorderWidth(WeexCore::kBorderWidthRight,
                                     [self judgePropValuePropValue:styles[@"borderRightWidth"] defaultValue:0]);
        needLayout = YES;
    }
    
    // padding
    if (styles[@"padding"]) {
        _flexCssNode->setPadding(WeexCore::kPaddingALL,
                                 [self judgePropValuePropValue:styles[@"padding"] defaultValue:0]);
        needLayout = YES;
    }
    if (styles[@"paddingTop"]) {
        _flexCssNode->setPadding(WeexCore::kPaddingTop,
                                 [self judgePropValuePropValue:styles[@"paddingTop"] defaultValue:0]);
        needLayout = YES;
    }
    if (styles[@"paddingLeft"]) {
        _flexCssNode->setPadding(WeexCore::kPaddingLeft,
                                 [self judgePropValuePropValue:styles[@"paddingLeft"] defaultValue:0]);
        needLayout = YES;
    }
    if (styles[@"paddingBottom"]) {
        _flexCssNode->setPadding(WeexCore::kPaddingBottom,
                                 [self judgePropValuePropValue:styles[@"paddingBottom"] defaultValue:0]);
        needLayout = YES;
    }
    if (styles[@"paddingRight"]) {
        _flexCssNode->setPadding(WeexCore::kPaddingRight,
                                 [self judgePropValuePropValue:styles[@"paddingRight"] defaultValue:0]);
        needLayout = YES;
    }
    
    if (needLayout) {
        [self setNeedsLayout];
    }
}

- (CGFloat)judgePropValuePropValue:(id)propValue defaultValue:(CGFloat)defaultValue
{
    CGFloat convertValue = (CGFloat)[WXConvert WXFlexPixelType:propValue scaleFactor:self.weexInstance.pixelScaleFactor];
    if (!isnan(convertValue)) {
        return convertValue;
    }
    return defaultValue;
}

- (float)getCssStyleValueForKey:(NSString *)key
{
    /*
     *      width, height, min-width, min-height, max-width, max-height,
     *      margin-(left/right/top/bottom)
     *      padding-(left/right/top/bottom)
     *      border-(left/right/top/bottom)-width
     *      left, right, top, bottom
     *      flex-grow
     */
    WXAssert(_flexCssNode != nullptr, @"Css node is null.");
    if (_flexCssNode == nullptr) {
        return NAN;
    }
    
    std::string ckey = [key UTF8String];
    if (ckey == "width") return _flexCssNode->getStyleWidth();
    if (ckey == "height") return _flexCssNode->getStyleHeight();
    if (ckey == "min-width") return _flexCssNode->getMinWidth();
    if (ckey == "min-height") return _flexCssNode->getMinHeight();
    if (ckey == "max-width") return _flexCssNode->getMaxWidth();
    if (ckey == "max-height") return _flexCssNode->getMaxHeight();
    if (ckey == "margin-left") return _flexCssNode->getMarginLeft();
    if (ckey == "margin-right") return _flexCssNode->getMarginRight();
    if (ckey == "margin-top") return _flexCssNode->getMarginTop();
    if (ckey == "margin-bottom") return _flexCssNode->getMarginBottom();
    if (ckey == "padding-left") return _flexCssNode->getPaddingLeft();
    if (ckey == "padding-right") return _flexCssNode->getPaddingRight();
    if (ckey == "padding-top") return _flexCssNode->getPaddingTop();
    if (ckey == "padding-bottom") return _flexCssNode->getPaddingBottom();
    if (ckey == "border-left-width") return _flexCssNode->getBorderWidthLeft();
    if (ckey == "border-right-width") return _flexCssNode->getBorderWidthRight();
    if (ckey == "border-top-width") return _flexCssNode->getBorderWidthTop();
    if (ckey == "border-bottom-width") return _flexCssNode->getBorderWidthBottom();
    if (ckey == "left") return _flexCssNode->getStylePositionLeft();
    if (ckey == "right") return _flexCssNode->getStylePositionRight();
    if (ckey == "top") return _flexCssNode->getStylePositionTop();
    if (ckey == "bottom") return _flexCssNode->getStylePositionBottom();
    if (ckey == "flex-grow") return _flexCssNode->getFlex();
    
    WXAssert(NO, @"Invalid css style key %@", key);
    return NAN;
}

- (WXCoreFlexDirection)getCssStyleFlexDirection
{
    WXAssert(_flexCssNode != nullptr, @"Css node is null.");
    return _flexCssNode ? _flexCssNode->getFlexDirection() : kFlexDirectionColumn;
}

- (WXCoreFlexWrap)getCssStyleFlexWrap
{
    WXAssert(_flexCssNode != nullptr, @"Css node is null.");
    return _flexCssNode ? _flexCssNode->getFlexWrap() : kNoWrap;
}

- (WXCoreJustifyContent)getCssStyleJustifyContent
{
    WXAssert(_flexCssNode != nullptr, @"Css node is null.");
    return _flexCssNode ? _flexCssNode->getJustifyContent() : kJustifyFlexStart;
}

- (WXCoreAlignItems)getCssStyleAlignItems
{
    WXAssert(_flexCssNode != nullptr, @"Css node is null.");
    return _flexCssNode ? _flexCssNode->getAlignItems() : kAlignItemsStretch;
}

- (WXCoreAlignSelf)getCssStyleAlignSelf
{
    WXAssert(_flexCssNode != nullptr, @"Css node is null.");
    return _flexCssNode ? _flexCssNode->getAlignSelf() : kAlignSelfAuto;
}

- (WXCorePositionType)getCssStylePositionType
{
    WXAssert(_flexCssNode != nullptr, @"Css node is null.");
    return _flexCssNode ? _flexCssNode->getStylePositionType() : kRelative;
}

- (WXCoreDirection)getCssDirection
{
    WXAssert(_flexCssNode != nullptr, @"Css node is null.");
    return _flexCssNode ? _flexCssNode->getDirection() : WEEXCORE_CSS_DEFAULT_DIRECTION;
}

- (NSString*)convertLayoutValueToStyleValue:(NSString*)valueName
{
    if (_flexCssNode == nullptr) {
        return @"0";
    }
    
    float layoutValue = 0;
    if ([valueName isEqualToString:@"left"])
        layoutValue = _flexCssNode->getLayoutPositionLeft();
    else if ([valueName isEqualToString:@"right"])
        layoutValue = _flexCssNode->getLayoutPositionRight();
    else if ([valueName isEqualToString:@"top"])
        layoutValue = _flexCssNode->getLayoutPositionTop();
    else if ([valueName isEqualToString:@"bottom"])
        layoutValue = _flexCssNode->getLayoutPositionBottom();
    else if ([valueName isEqualToString:@"width"])
        layoutValue = _flexCssNode->getLayoutWidth();
    else if ([valueName isEqualToString:@"height"])
        layoutValue = _flexCssNode->getLayoutHeight();
    else
        return @"0";
    
    layoutValue /= self.weexInstance.pixelScaleFactor;
    return [NSString stringWithFormat:@"%f", layoutValue];
}

- (CGFloat)safeContainerStyleWidth
{
    if (_flexCssNode == nullptr) {
        return 0.0f;
    }
    
    CGFloat thisValue = _flexCssNode->getStyleWidth();
    if (isnan(thisValue)) {
        if (_flexCssNode->getParent()) {
            thisValue = _flexCssNode->getParent()->getLayoutWidth(); // parent may be layout done
            if (isnan(thisValue)) {
                thisValue = _flexCssNode->getParent()->getStyleWidth();
            }
        }
    }
    
    if (isnan(thisValue)) {
        thisValue = self.weexInstance.frame.size.width;
    }
    
    if (isnan(thisValue) || thisValue == 0.0f) {
        thisValue = [UIScreen mainScreen].bounds.size.width;
    }
    
    return thisValue;
}

#define WX_FLEX_STYLE_RESET_CSS_NODE(key, defaultValue)\
do {\
    WX_FLEX_STYLE_RESET_CSS_NODE_GIVEN_KEY(key,key,defaultValue)\
} while(0);

#define WX_FLEX_STYLE_RESET_CSS_NODE_GIVEN_KEY(judgeKey, propKey, defaultValue)\
do {\
    if (styles && [styles containsObject:@#judgeKey]) {\
        NSMutableDictionary *resetStyleDic = [[NSMutableDictionary alloc] init];\
        [resetStyleDic setValue:defaultValue forKey:@#propKey];\
        [self _updateCSSNodeStyles:resetStyleDic];\
        [self setNeedsLayout];\
    }\
} while(0);

#define WX_FLEX_STYLE_RESET_CSS_NODE_GIVEN_DIRECTION_KEY(judgeKey, propTopKey,propLeftKey,propRightKey,propBottomKey, defaultValue)\
do {\
    if (styles && [styles containsObject:@#judgeKey]) {\
        NSMutableDictionary *resetStyleDic = [[NSMutableDictionary alloc] init];\
        [resetStyleDic setValue:defaultValue forKey:@#propTopKey];\
        [resetStyleDic setValue:defaultValue forKey:@#propLeftKey];\
        [resetStyleDic setValue:defaultValue forKey:@#propRightKey];\
        [resetStyleDic setValue:defaultValue forKey:@#propBottomKey];\
        [self _updateCSSNodeStyles:resetStyleDic];\
        [self setNeedsLayout];\
    }\
} while(0);


- (void)_resetCSSNode:(NSArray *)styles
{
        if (styles.count<=0) {
            return;
        }
    
        WX_FLEX_STYLE_RESET_CSS_NODE(direction, @(WeexCore::kDirectionInherit))
    
        WX_FLEX_STYLE_RESET_CSS_NODE(flex, @0.0)
        WX_FLEX_STYLE_RESET_CSS_NODE(flexDirection, @(WeexCore::kFlexDirectionColumn))
        WX_FLEX_STYLE_RESET_CSS_NODE(alignItems, @(WeexCore::kAlignItemsStretch))
        WX_FLEX_STYLE_RESET_CSS_NODE(alignSelf, @(WeexCore::kAlignSelfAuto))
        WX_FLEX_STYLE_RESET_CSS_NODE(flexWrap, @(WeexCore::kNoWrap))
        WX_FLEX_STYLE_RESET_CSS_NODE(justifyContent, @(WeexCore::kJustifyFlexStart))
        
        // position
        WX_FLEX_STYLE_RESET_CSS_NODE(position, @(WeexCore::kRelative))
        WX_FLEX_STYLE_RESET_CSS_NODE(top, @(NAN))
        WX_FLEX_STYLE_RESET_CSS_NODE(left, @(NAN))
        WX_FLEX_STYLE_RESET_CSS_NODE(right, @(NAN))
        WX_FLEX_STYLE_RESET_CSS_NODE(bottom, @(NAN))
        
        // dimension
        WX_FLEX_STYLE_RESET_CSS_NODE(width, @(NAN))
        WX_FLEX_STYLE_RESET_CSS_NODE(height, @(NAN))
        WX_FLEX_STYLE_RESET_CSS_NODE(minWidth, @(NAN))
        WX_FLEX_STYLE_RESET_CSS_NODE(minHeight, @(NAN))
        WX_FLEX_STYLE_RESET_CSS_NODE(maxWidth, @(NAN))
        WX_FLEX_STYLE_RESET_CSS_NODE(maxHeight, @(NAN))
        
        // margin
        WX_FLEX_STYLE_RESET_CSS_NODE_GIVEN_DIRECTION_KEY(margin
                                                         ,marginTop
                                                         ,marginLeft
                                                         ,marginRight
                                                         ,marginBottom
                                                         , @(0.0))
        WX_FLEX_STYLE_RESET_CSS_NODE(marginTop, @(0.0))
        WX_FLEX_STYLE_RESET_CSS_NODE(marginLeft, @(0.0))
        WX_FLEX_STYLE_RESET_CSS_NODE(marginRight, @(0.0))
        WX_FLEX_STYLE_RESET_CSS_NODE(marginBottom, @(0.0))
        
        // border
        WX_FLEX_STYLE_RESET_CSS_NODE_GIVEN_DIRECTION_KEY(borderWidth
                                                         , borderTopWidth
                                                         , borderLeftWidth
                                                         , borderRightWidth
                                                         , borderBottomWidth
                                                         , @(0.0))
        WX_FLEX_STYLE_RESET_CSS_NODE(borderTopWidth, @(0.0))
        WX_FLEX_STYLE_RESET_CSS_NODE(borderLeftWidth, @(0.0))
        WX_FLEX_STYLE_RESET_CSS_NODE(borderRightWidth, @(0.0))
        WX_FLEX_STYLE_RESET_CSS_NODE(borderBottomWidth, @(0.0))
        
        // padding
        WX_FLEX_STYLE_RESET_CSS_NODE_GIVEN_DIRECTION_KEY(padding
                                                         , paddingTop
                                                         , paddingLeft
                                                         , paddingRight
                                                         , paddingBottom
                                                         , @(0.0))
        WX_FLEX_STYLE_RESET_CSS_NODE(paddingTop, @(0.0))
        WX_FLEX_STYLE_RESET_CSS_NODE(paddingLeft, @(0.0))
        WX_FLEX_STYLE_RESET_CSS_NODE(paddingRight, @(0.0))
        WX_FLEX_STYLE_RESET_CSS_NODE(paddingBottom, @(0.0))

}

#pragma mark CSS Node Override

static WeexCore::WXCoreSize flexCssNodeMeasure(WeexCore::WXCoreLayoutNode *node, float width, WeexCore::MeasureMode widthMeasureMode,float height, WeexCore::MeasureMode heightMeasureMode){
    
    if (node->getContext() == nullptr) { //为空
        return WeexCore::WXCoreSize();
    }
    WXComponent *component = (__bridge WXComponent *)(node->getContext());
    
    if (![component respondsToSelector:@selector(measureBlock)]) {
        return WeexCore::WXCoreSize();
    }
    
    CGSize (^measureBlock)(CGSize) = [component measureBlock];
    
    if (!measureBlock) {
        return WeexCore::WXCoreSize();
    }
    
    CGSize constrainedSize = CGSizeMake(width, height);
    CGSize resultSize = measureBlock(constrainedSize);
#ifdef DEBUG
    WXLogDebug(@"flexLayout -> measureblock %@, resultSize:%@",
          component.type,
          NSStringFromCGSize(resultSize)
          );
#endif
    WeexCore::WXCoreSize size;
    size.height=(float)resultSize.height;
    size.width=(float)resultSize.width;
    return size;
}

-(WeexCore::WXCorePositionType)fxPositionType:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        if ([value isEqualToString:@"absolute"]) {
            return WeexCore::kAbsolute;
        } else if ([value isEqualToString:@"relative"]) {
            return WeexCore::kRelative;
        } else if ([value isEqualToString:@"fixed"]) {
            return WeexCore::kFixed;
        } else if ([value isEqualToString:@"sticky"]) {
            return WeexCore::kRelative;
        }
    }
    return WeexCore::kRelative;
}

- (WeexCore::WXCoreDirection)fxDirection:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        if ([value isEqualToString:@"rtl"]) {
            return WeexCore::kDirectionRTL;
        } else if ([value isEqualToString:@"ltr"]) {
            return WeexCore::kDirectionLTR;
        }
    }
    return WeexCore::kDirectionInherit;
}

- (WeexCore::WXCoreFlexDirection)fxFlexDirection:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        if ([value isEqualToString:@"column"]) {
            return WeexCore::kFlexDirectionColumn;
        } else if ([value isEqualToString:@"column-reverse"]) {
            return WeexCore::kFlexDirectionColumnReverse;
        } else if ([value isEqualToString:@"row"]) {
            return WeexCore::kFlexDirectionRow;
        } else if ([value isEqualToString:@"row-reverse"]) {
            return WeexCore::kFlexDirectionRowReverse;
        }
    }
    return WeexCore::kFlexDirectionColumn;
}

//TODO
- (WeexCore::WXCoreAlignItems)fxAlign:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        if ([value isEqualToString:@"stretch"]) {
            return WeexCore::kAlignItemsStretch;
        } else if ([value isEqualToString:@"flex-start"]) {
            return WeexCore::kAlignItemsFlexStart;
        } else if ([value isEqualToString:@"flex-end"]) {
            return WeexCore::kAlignItemsFlexEnd;
        } else if ([value isEqualToString:@"center"]) {
            return WeexCore::kAlignItemsCenter;
            //return WXCoreFlexLayout::WXCore_AlignItems_Center;
        } else if ([value isEqualToString:@"auto"]) {
//            return YGAlignAuto;
        } else if ([value isEqualToString:@"baseline"]) {
//            return YGAlignBaseline;
        }
    }
    
    return WeexCore::kAlignItemsStretch;
}

- (WeexCore::WXCoreAlignSelf)fxAlignSelf:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        if ([value isEqualToString:@"stretch"]) {
            return WeexCore::kAlignSelfStretch;
        } else if ([value isEqualToString:@"flex-start"]) {
            return WeexCore::kAlignSelfFlexStart;
        } else if ([value isEqualToString:@"flex-end"]) {
            return WeexCore::kAlignSelfFlexEnd;
        } else if ([value isEqualToString:@"center"]) {
            return WeexCore::kAlignSelfCenter;
        } else if ([value isEqualToString:@"auto"]) {
            return WeexCore::kAlignSelfAuto;
        } else if ([value isEqualToString:@"baseline"]) {
            //            return YGAlignBaseline;
        }
    }
    
    return WeexCore::kAlignSelfStretch;
}

- (WeexCore::WXCoreFlexWrap)fxWrap:(id)value
{
    if([value isKindOfClass:[NSString class]]) {
        if ([value isEqualToString:@"nowrap"]) {
            return WeexCore::kNoWrap;
        } else if ([value isEqualToString:@"wrap"]) {
            return WeexCore::kWrap;
        } else if ([value isEqualToString:@"wrap-reverse"]) {
            return WeexCore::kWrapReverse;
        }
    }
    return WeexCore::kNoWrap;
}

- (WeexCore::WXCoreJustifyContent)fxJustify:(id)value
{
    if([value isKindOfClass:[NSString class]]){
        if ([value isEqualToString:@"flex-start"]) {
            return WeexCore::kJustifyFlexStart;
        } else if ([value isEqualToString:@"center"]) {
            return WeexCore::kJustifyCenter;
        } else if ([value isEqualToString:@"flex-end"]) {
            return WeexCore::kJustifyFlexEnd;
        } else if ([value isEqualToString:@"space-between"]) {
            return WeexCore::kJustifySpaceBetween;
        } else if ([value isEqualToString:@"space-around"]) {
            return WeexCore::kJustifySpaceAround;
        }
    }
    return WeexCore::kJustifyFlexStart;
}

- (void)removeSubcomponentCssNode:(WXComponent *)subcomponent
{
    auto node = subcomponent->_flexCssNode;
    if (node) {
        if (_flexCssNode) {
            _flexCssNode->removeChild(node);
        }
        
        [subcomponent _setRenderObject:nullptr];
        
        // unbind subcomponents of subcomponent
        NSMutableArray* sub_subcomponents = [[NSMutableArray alloc] init];
        [subcomponent _collectSubcomponents:sub_subcomponents];
        for (WXComponent* c in sub_subcomponents) {
            [c _setRenderObject:nullptr];
        }
        
        [WXCoreBridge removeRenderObjectFromMap:subcomponent.weexInstance.instanceId object:node];
        delete node; // also will delete all children recursively
    }
}

#pragma mark - RTL

- (BOOL)isDirectionRTL {
    if (![WXUtility enableRTLLayoutDirection]) return NO;

    return _isLayoutDirectionRTL;
}

- (void)_adjustForRTL {
    if (![WXUtility enableRTLLayoutDirection]) return;
    
    if (self->_positionType == WXPositionTypeFixed) return;
    
    if (self.supercomponent && self.supercomponent.isDirectionRTL && [self.supercomponent shouldTransformSubviewsWhenRTL]) {
        if (_transform) {
            self.view.layer.transform = CATransform3DConcat(self.view.layer.transform, CATransform3DScale(CATransform3DIdentity, -1, 1, 1));
        } else {
            self.view.layer.transform = CATransform3DScale(CATransform3DIdentity, -1, 1, 1);
        }
    } else {
        if (!_transform) {
            self.view.layer.transform = CATransform3DIdentity;
        }
    }
}

// Now we scrollView RTL solution is tranform
// so scrollView need tranform subviews when RTL by default
// if your component view is not scrollView but also implement RTL layout by tranform，you need return YES
- (BOOL)shouldTransformSubviewsWhenRTL {
    return [self.view isKindOfClass:[UIScrollView class]];
}

- (void)_setIsLayoutRTL:(BOOL)isRTL {
    _isLayoutDirectionRTL = isRTL;
}

@end
