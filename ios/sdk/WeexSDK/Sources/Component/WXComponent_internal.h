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

#import "WXScrollerProtocol.h"
#import "WXComponent.h"
#import "WXConvert.h"
#import "WXTransform.h"
#import "WXTransition.h"
@class WXTouchGestureRecognizer;
@class WXThreadSafeCounter;

typedef id (^WXDataBindingBlock)(NSDictionary *data, BOOL *needUpdate);

/**
 * The following variables and methods are used in Weex INTERNAL logic.
 * @warning These variables and methods must never be called or overridden.
 */
@interface WXComponent ()
{
@package
    NSString *_type;
    NSMutableArray *_subcomponents;
    /**
     *  Layout
     */
    css_node_t *_cssNode;
    BOOL _isLayoutDirty;
    CGRect _calculatedFrame;
    CGPoint _absolutePosition;
    WXPositionType _positionType;
    
    
    //Transition
    WXTransition *_transition;
    
    /**
     *  View
     */
    UIColor *_backgroundColor;
    NSString *_backgroundImage;
    WXClipType _clipToBounds;
    UIView *_view;
    CGFloat _opacity;
    WXVisibility  _visibility;
    WXBoxShadow *_originalBoxShadow;
    WXBoxShadow *_lastBoxShadow;
    WXBoxShadow *_boxShadow;
    
    /**
     * accessibility support
     */
    NSString * _roles; //accessibility
    NSString * _ariaLabel; //accessibilityLabel
    NSString * _ariaHidden; // accessibilityElementsHidden
    NSString * _accessible; // accessible
    NSString * _accessibilityHintContent; // hint for the action
    NSString * _groupAccessibilityChildren; // voice-over navigation order
    NSString * _testId;// just for auto-test
    
    BOOL _accessibilityMagicTapEvent;
    
    /**
     *  PseudoClass
     */
    NSMutableDictionary *_pseudoClassStyles;
    NSMutableDictionary *_updatedPseudoClassStyles;
    BOOL _isListenPseudoTouch;
    
    /**
     *  Events
     */
    BOOL _appearEvent;
    BOOL _disappearEvent;
    UITapGestureRecognizer *_tapGesture;
    NSMutableArray *_swipeGestures;
    UILongPressGestureRecognizer *_longPressGesture;
    UIPanGestureRecognizer *_panGesture;
    
    BOOL _listenPanStart;
    BOOL _listenPanMove;
    BOOL _listenPanEnd;
    
    BOOL _listenHorizontalPan;
    BOOL _listenVerticalPan;
    
    WXTouchGestureRecognizer* _touchGesture;
    
    /**
     *  Display
     */
    CALayer *_layer;
    BOOL _useCompositing;
    BOOL _isCompositingChild;
    WXThreadSafeCounter *_displayCounter;
    
    UIColor *_borderTopColor;
    UIColor *_borderRightColor;
    UIColor *_borderLeftColor;
    UIColor *_borderBottomColor;
    
    CGFloat _borderTopWidth;
    CGFloat _borderRightWidth;
    CGFloat _borderLeftWidth;
    CGFloat _borderBottomWidth;
    
    CGFloat _borderTopLeftRadius;
    CGFloat _borderTopRightRadius;
    CGFloat _borderBottomLeftRadius;
    CGFloat _borderBottomRightRadius;
    
    WXBorderStyle _borderTopStyle;
    WXBorderStyle _borderRightStyle;
    WXBorderStyle _borderBottomStyle;
    WXBorderStyle _borderLeftStyle;
    
    
    BOOL _isFixed;
    BOOL _async;
    BOOL _isNeedJoinLayoutSystem;
    BOOL _lazyCreateView;
    
    WXTransform *_transform;
    
    /**
     * Data Binding
     */
    BOOL _isTemplate;
    WXComponent *_templateComponent;
    WXDataBindingBlock _bindingMatch;
    WXDataBindingBlock _bindingRepeat;
    NSString *_repeatIndexIdentify;
    NSString *_repeatLabelIdentify;
    NSString *_virtualComponentId;// for recycleList subcomponent
    BOOL _isRepeating;
    BOOL _isSkipUpdate;
    BOOL _dataBindOnce;
    
    NSMutableDictionary<NSString *, WXDataBindingBlock> *_bindingProps;
    NSMutableDictionary<NSString *, WXDataBindingBlock> *_bindingAttributes;
    NSMutableDictionary<NSString *, WXDataBindingBlock> *_bindingStyles;
    NSMutableDictionary<NSString *, WXDataBindingBlock> *_bindingEvents;
    
    NSMutableDictionary<NSString *, NSArray *> *_eventParameters;
}

///--------------------------------------
/// @name Package Internal Methods
///--------------------------------------

- (void)_layoutDidFinish;
- (void)_calculateFrameWithSuperAbsolutePosition:(CGPoint)superAbsolutePosition
                           gatherDirtyComponents:(NSMutableSet<WXComponent *> *)dirtyComponents;


- (void)_willDisplayLayer:(CALayer *)layer;

- (void)_unloadViewWithReusing:(BOOL)isReusing;

- (id<WXScrollerProtocol>)ancestorScroller;

- (void)_insertSubcomponent:(WXComponent *)subcomponent atIndex:(NSInteger)index;
- (void)_removeFromSupercomponent;
- (void)_moveToSupercomponent:(WXComponent *)newSupercomponent atIndex:(NSUInteger)index;

- (void)_updateStylesOnComponentThread:(NSDictionary *)styles resetStyles:(NSMutableArray *)resetStyles isUpdateStyles:(BOOL)isUpdateStyles;
- (void)_updateAttributesOnComponentThread:(NSDictionary *)attributes;
- (void)_updateStylesOnMainThread:(NSDictionary *)styles resetStyles:(NSMutableArray *)resetStyles;
- (void)_updateAttributesOnMainThread:(NSDictionary *)attributes;

- (void)_addEventOnComponentThread:(NSString *)eventName;
- (void)_removeEventOnComponentThread:(NSString *)eventName;
- (void)_addEventOnMainThread:(NSString *)eventName;
- (void)_removeEventOnMainThread:(NSString *)eventName;

///--------------------------------------
/// @name Protected Methods
///--------------------------------------

- (BOOL)_needsDrawBorder;

- (void)_drawBorderWithContext:(CGContextRef)context size:(CGSize)size;

- (void)_frameDidCalculated:(BOOL)isChanged;

- (NSUInteger)_childrenCountForLayout;

///--------------------------------------
/// @name Private Methods
///--------------------------------------

- (void)_modifyStyles:(NSDictionary *)styles;

- (void)_transitionUpdateViewProperty:(NSDictionary *)styles;

- (void)_initCSSNodeWithStyles:(NSDictionary *)styles;

- (void)_updateCSSNodeStyles:(NSDictionary *)styles;

- (void)_resetCSSNodeStyles:(NSArray *)styles;

- (void)_recomputeCSSNodeChildren;

- (void)_handleBorders:(NSDictionary *)styles isUpdating:(BOOL)updating;

- (void)_initViewPropertyWithStyles:(NSDictionary *)styles;

- (void)_updateViewStyles:(NSDictionary *)styles;

- (void)_resetStyles:(NSArray *)styles;

- (void)_initEvents:(NSArray *)events;

- (void)_initPseudoEvents:(BOOL)isListenPseudoTouch;

- (void)_removeAllEvents;

- (void)_addEventParams:(NSDictionary *)params;

- (NSArray *)_paramsForEvent:(NSString *)eventName;

- (void)_setupNavBarWithStyles:(NSMutableDictionary *)styles attributes:(NSMutableDictionary *)attributes;

- (void)_initCompositingAttribute:(NSDictionary *)attributes;

- (BOOL)_bitmapOpaqueWithSize:(CGSize)size;

- (void)_updateNavBarAttributes:(NSDictionary *)attributes;

- (void)_handleFirstScreenTime;

- (void)_resetNativeBorderRadius;

- (void)_updatePseudoClassStyles:(NSString *)key;

- (void)_restoreViewStyles;

- (void)_configWXComponentA11yWithAttributes:(NSDictionary *)attributes;

- (void)setGradientLayer;

- (void)_storeBindingsWithProps:(NSDictionary *)props styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSDictionary *)events;

- (void)_didInserted;

@end


