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

bool flexIsUndefined(float value) {
    return isnan(value);
}



@implementation WXComponent (Layout)

#pragma clang diagnostic ignored "-Wobjc-protocol-method-implementation"

#pragma mark Public

- (void)setNeedsLayout
{
    _isLayoutDirty = YES;
    WXComponent *supercomponent = [self supercomponent];
    
    //protet nil ptr
    if (self.flexCssNode) {
        self.flexCssNode->markDirty();
    }else{
        WXLogError(@"flexCssNode is nil");
    }
    
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
        _flexCssNode = new WeexCore::WXCoreLayoutNode();
        if ([self measureBlock]) {
            _flexCssNode->setMeasureFunc(flexCssNodeMeasure);
        }
        _flexCssNode->setContext((__bridge void *)self);
        [self _recomputeCSSNodeChildren];
        [self _fillCSSNode:styles isUpdate:NO];
        
        if ([self.ref isEqualToString:WX_SDK_ROOT_REF]) {
            if (flexIsUndefined(_flexCssNode->getStyleHeight()) && self.weexInstance.frame.size.height) {
                _flexCssNode->setStyleHeight(self.weexInstance.frame.size.height);
            }
            
            if (flexIsUndefined(_flexCssNode->getStyleWidth()) && self.weexInstance.frame.size.width) {
                _flexCssNode->setStyleWidth(self.weexInstance.frame.size.width,NO);
            }
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

- (void)_recomputeCSSNodeChildren
{
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
    if (isChanged && [self isKindOfClass:[WXCellComponent class]]) {
        CGFloat mainScreenWidth = [[UIScreen mainScreen] bounds].size.width;
        CGFloat mainScreenHeight = [[UIScreen mainScreen] bounds].size.height;
        if (mainScreenHeight/2 < _calculatedFrame.size.height && mainScreenWidth/2 < _calculatedFrame.size.width) {
            [self weexInstance].performance.cellExceedNum++;
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

        if (self.flexCssNode->hasNewLayout()) {
            self.flexCssNode->setHasNewLayout(false);
            _isLayoutDirty = NO;
            CGRect newFrame = CGRectMake(
                                         isnan(WXRoundPixelValue(_flexCssNode->getLayoutPositionLeft()))?0:WXRoundPixelValue(_flexCssNode->getLayoutPositionLeft())
                                         ,isnan(WXRoundPixelValue(_flexCssNode->getLayoutPositionTop()))?0:WXRoundPixelValue(_flexCssNode->getLayoutPositionTop())
                                         ,isnan(WXRoundPixelValue(_flexCssNode->getLayoutWidth()))?0:WXRoundPixelValue(_flexCssNode->getLayoutWidth())
                                         ,isnan(WXRoundPixelValue(_flexCssNode->getLayoutHeight()))?0:WXRoundPixelValue(_flexCssNode->getLayoutHeight())
                                         );
            BOOL isFrameChanged = NO;
            
            if (!CGRectEqualToRect(newFrame, _calculatedFrame)) {
                
                isFrameChanged = YES;
                _calculatedFrame = newFrame;
                [dirtyComponents addObject:self];
            }
            
            [self _frameDidCalculated:isFrameChanged];
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> newFrame ,type:%@,ref:%@, parentRef:%@,size :%@ ,instance:%@",self.type,self.ref,self.supercomponent.ref,NSStringFromCGRect(newFrame),self.weexInstance.instanceId);
#endif
        }
    
        NSArray * subcomponents = [_subcomponents copy];
        for (WXComponent *subcomponent in subcomponents) {
            [subcomponent _calculateFrameWithSuperAbsolutePosition:superAbsolutePosition gatherDirtyComponents:dirtyComponents];
        }
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
        // flex
        if (styles[@"flex"]) {
            _flexCssNode->setFlex([WXConvert CGFloat:styles[@"flex"]]);
        }
        if (isnan(_flexCssNode->getFlex())) {
            // to make the default flex value is zero, yoga is nan, maybe this can configured by yoga config
            _flexCssNode->setFlex(0);
        }
        
        if (styles[@"flexDirection"]) {
            _flexCssNode->setFlexDirection([self fxFlexDirection:styles[@"flexDirection"]],isUpdate);
        }
        if (styles[@"alignItems"]) {
            _flexCssNode->setAlignItems([self fxAlign:styles[@"alignItems"]]);
        }
        if (styles[@"alignSelf"]) {
            _flexCssNode->setAlignSelf([self fxAlignSelf:styles[@"alignSelf"]]);
        }
        if (styles[@"flexWrap"]) {
            _flexCssNode->setFlexWrap([self fxWrap:styles[@"flexWrap"]]);
        }
        if (styles[@"justifyContent"]) {
            _flexCssNode->setJustifyContent([self fxJustify:styles[@"justifyContent"]]);
        }
        
        // position
        if (styles[@"position"]) {
            _flexCssNode->setStylePositionType([self fxPositionType:styles[@"position"]]);
        }
        if (styles[@"top"]) {
            _flexCssNode->setStylePosition(WeexCore::kPositionEdgeTop,
                                           [self judgePropValuePropValue:styles[@"top"] defaultValue:NAN]);
        }
        if (styles[@"left"]) {
            _flexCssNode->setStylePosition(WeexCore::kPositionEdgeLeft,
                                           [self judgePropValuePropValue:styles[@"left"] defaultValue:NAN]);
        }
        if(styles[@"right"]) {
            _flexCssNode->setStylePosition(WeexCore::kPositionEdgeRight,
                                           [self judgePropValuePropValue:styles[@"right"] defaultValue:NAN]);
        }
        if (styles[@"bottom"]) {
            _flexCssNode->setStylePosition(WeexCore::kPositionEdgeBottom,
                                           [self judgePropValuePropValue:styles[@"bottom"] defaultValue:NAN]);
        }
        
        // dimension
        if (styles[@"width"]) {
            _flexCssNode->setStyleWidth([self judgePropValuePropValue:styles[@"width"] defaultValue:NAN]
                                        ,isUpdate);
        }
        if (styles[@"height"]) {
            _flexCssNode->setStyleHeight([self judgePropValuePropValue:styles[@"height"] defaultValue:NAN]);
        }
        if (styles[@"minWidth"]) {
            _flexCssNode->setMinWidth([self judgePropValuePropValue:styles[@"minWidth"] defaultValue:NAN]
                                      ,isUpdate);
        }
        if (styles[@"minHeight"]) {
            _flexCssNode->setMinHeight([self judgePropValuePropValue:styles[@"minHeight"] defaultValue:NAN]);
        }
        if (styles[@"maxWidth"]) {
            _flexCssNode->setMaxWidth([self judgePropValuePropValue:styles[@"maxWidth"] defaultValue:NAN]
                                      ,isUpdate);
        }
        if (styles[@"maxHeight"]) {
            _flexCssNode->setMaxHeight([self judgePropValuePropValue:styles[@"maxHeight"] defaultValue:NAN]);
        }
        
        // margin
        if (styles[@"margin"]) {
            _flexCssNode->setMargin(WeexCore::kMarginALL,
                                    [self judgePropValuePropValue:styles[@"margin"] defaultValue:0]);
        }
        if (styles[@"marginTop"]) {
            _flexCssNode->setMargin(WeexCore::kMarginTop,
                                    [self judgePropValuePropValue:styles[@"marginTop"] defaultValue:0]);
        }
        if (styles[@"marginBottom"]) {
            _flexCssNode->setMargin(WeexCore::kMarginBottom,
                                    [self judgePropValuePropValue:styles[@"marginBottom"] defaultValue:0]);
        }
        if (styles[@"marginRight"]) {
            _flexCssNode->setMargin(WeexCore::kMarginRight,
                                    [self judgePropValuePropValue:styles[@"marginRight"] defaultValue:0]);
        }
        if (styles[@"marginLeft"]) {
            _flexCssNode->setMargin(WeexCore::kMarginLeft,
                                    [self judgePropValuePropValue:styles[@"marginLeft"] defaultValue:0]);
        }
        
        // border
        if (styles[@"borderWidth"]) {
            _flexCssNode->setBorderWidth(WeexCore::kBorderWidthALL,
                                         [self judgePropValuePropValue:styles[@"borderWidth"] defaultValue:0]);
        }
        if (styles[@"borderTopWidth"]) {
            _flexCssNode->setBorderWidth(WeexCore::kBorderWidthTop,
                                         [self judgePropValuePropValue:styles[@"borderTopWidth"] defaultValue:0]);
        }
        
        if (styles[@"borderLeftWidth"]) {
            _flexCssNode->setBorderWidth(WeexCore::kBorderWidthLeft,
                                         [self judgePropValuePropValue:styles[@"borderLeftWidth"] defaultValue:0]);
        }
        
        if (styles[@"borderBottomWidth"]) {
            _flexCssNode->setBorderWidth(WeexCore::kBorderWidthBottom,
                                         [self judgePropValuePropValue:styles[@"borderBottomWidth"] defaultValue:0]);
        }
        if (styles[@"borderRightWidth"]) {
            _flexCssNode->setBorderWidth(WeexCore::kBorderWidthRight,
                                         [self judgePropValuePropValue:styles[@"borderRightWidth"] defaultValue:0]);
        }
        
        // padding
        if (styles[@"padding"]) {
            _flexCssNode->setPadding(WeexCore::kPaddingALL,
                                     [self judgePropValuePropValue:styles[@"padding"] defaultValue:0]);
        }
        if (styles[@"paddingTop"]) {
            _flexCssNode->setPadding(WeexCore::kPaddingTop,
                                     [self judgePropValuePropValue:styles[@"paddingTop"] defaultValue:0]);
        }
        if (styles[@"paddingLeft"]) {
            _flexCssNode->setPadding(WeexCore::kPaddingLeft,
                                     [self judgePropValuePropValue:styles[@"paddingLeft"] defaultValue:0]);
        }
        if (styles[@"paddingBottom"]) {
            _flexCssNode->setPadding(WeexCore::kPaddingBottom,
                                     [self judgePropValuePropValue:styles[@"paddingBottom"] defaultValue:0]);
        }
        if (styles[@"paddingRight"]) {
            _flexCssNode->setPadding(WeexCore::kPaddingRight,
                                     [self judgePropValuePropValue:styles[@"paddingRight"] defaultValue:0]);
        }
        
        [self setNeedsLayout];
}

-(CGFloat)judgePropValuePropValue:(NSString *)propValue defaultValue:(CGFloat)defaultValue{
    CGFloat convertValue = (CGFloat)[WXConvert WXFlexPixelType:propValue scaleFactor:self.weexInstance.pixelScaleFactor];
    if (!isnan(convertValue)) {
        return convertValue;
    }
    return defaultValue;
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


- (NSInteger) getActualNodeIndex:(WXComponent*)subcomponent atIndex:(NSInteger) index
{
    NSInteger actualIndex = 0; //实际除去不需要布局的subComponent，此时所在的正确位置
    for (WXComponent *child in _subcomponents) {
        if ([child.ref isEqualToString:subcomponent.ref]) {
            break;
        }
        if (child->_isNeedJoinLayoutSystem) {
            actualIndex ++;
        }
    }
    return actualIndex;
}

- (void)_insertChildCssNode:(WXComponent*)subcomponent atIndex:(NSInteger)index
{
    self.flexCssNode->addChildAt(subcomponent.flexCssNode, (uint32_t)index);
}

- (void)_rmChildCssNode:(WXComponent *)subcomponent
{
    self.flexCssNode->removeChild(subcomponent->_flexCssNode);
#ifdef DEBUG
    WXLogDebug(@"flexLayout -> ref:%@ ,flexCssNode->removeChild ,childRef:%@",self.ref,subcomponent.ref);
#endif
}


+ (void) recycleNodeOnComponentThread:(WeexCore::WXCoreLayoutNode * ) garbageNode gabRef:(NSString *)ref {
    if (nullptr == garbageNode) {
#ifdef DEBUG
        WXLogDebug(@"flexlayout->recycle garbageNode ref:%@ is null ",ref);
#endif
        return;
    }
    WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
        WXLogDebug(@"flexlayout->recycle  ref:%@ ,node:%p",ref,garbageNode );
#endif
        if(nullptr != garbageNode){
            delete garbageNode;
        }
    });
    //domthread
}

@end
