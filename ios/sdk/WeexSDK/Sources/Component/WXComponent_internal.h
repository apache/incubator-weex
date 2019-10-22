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

#ifdef __cplusplus
#import "WXJSASTParser.h"
#include <vector>
#endif // __cplusplus

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
    
    //Transition
    WXTransition *_transition;
    
    /**
     *  View
     */
    UIColor *_styleBackgroundColor;
    NSString *_backgroundImage;
    NSString *_clipRadius;
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
    
    BOOL _userInteractionEnabled;
    BOOL _eventPenetrationEnabled;
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
    
    BOOL _cancelsTouchesInView;
    
    BOOL _listenPanStart;
    BOOL _listenPanMove;
    BOOL _listenPanEnd;
    
    BOOL _listenHorizontalPan;
    BOOL _listenVerticalPan;
    
    BOOL _listenStopPropagation;
    BOOL _customEvent;
    NSString *_stopPropagationName;
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
    
    NSInteger _lastBorderRecords; // Records last border drawing
    
    BOOL _isViewTreeIgnored; // Component is added to super, but it is not added to views.
    BOOL _isFixed;
    BOOL _async;
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
    NSMutableDictionary *_virtualElementInfo;

    BOOL _isRepeating;
    BOOL _isSkipUpdate;
    BOOL _dataBindOnce;
    
    NSMutableDictionary<NSString *, WXDataBindingBlock> *_bindingProps;
    NSMutableDictionary<NSString *, WXDataBindingBlock> *_bindingAttributes;
    NSMutableDictionary<NSString *, WXDataBindingBlock> *_bindingStyles;
    NSMutableDictionary<NSString *, WXDataBindingBlock> *_bindingEvents;
#ifdef __cplusplus
    std::vector<WXJSExpression *> *_bindingExpressions;
#endif // __cplusplus
    
    NSMutableDictionary<NSString *, NSArray *> *_eventParameters;
}

/* DO NOT use "_transform = XXX" directly.
 Ivar access in ObjC is compiled to code with additional release or retain. So use Ivar in mutiple
 thread may lead to crash. Use an ATOMIC property is well enough. */
@property (atomic, strong) WXTransform *transform;

/**
 DO NOT use "_backgroundColor" directly. The same reason as '_transform'.
 */
@property (atomic, strong) UIColor* styleBackgroundColor;

///--------------------------------------
/// @name Package Internal Methods
///--------------------------------------

- (void)_layoutDidFinish;

- (void)_layoutPlatform;

- (void)_willDisplayLayer:(CALayer *)layer;

- (void)_unloadViewWithReusing:(BOOL)isReusing;

- (id<WXScrollerProtocol>)ancestorScroller;

// return if the component is actually inserted
- (BOOL)_insertSubcomponent:(WXComponent *)subcomponent atIndex:(NSInteger)index;

- (void)_removeFromSupercomponent;
- (void)_removeFromSupercomponent:(BOOL)remove;
- (void)_moveToSupercomponent:(WXComponent *)newSupercomponent atIndex:(NSUInteger)index;

- (BOOL)_isTransitionNone;
- (BOOL)_hasTransitionPropertyInStyles:(NSDictionary *)styles;
- (void)_updateStylesOnComponentThread:(NSDictionary *)styles resetStyles:(NSMutableArray *)resetStyles isUpdateStyles:(BOOL)isUpdateStyles;
- (void)_updateAttributesOnComponentThread:(NSDictionary *)attributes;
- (void)_updateStylesOnMainThread:(NSDictionary *)styles resetStyles:(NSMutableArray *)resetStyles;
- (void)_updateAttributesOnMainThread:(NSDictionary *)attributes;

- (void)_addEventOnComponentThread:(NSString *)eventName;
- (void)_removeEventOnComponentThread:(NSString *)eventName;
- (void)_addEventOnMainThread:(NSString *)eventName;
- (void)_removeEventOnMainThread:(NSString *)eventName;

- (void)_collectSubcomponents:(NSMutableArray *)components;

///--------------------------------------
/// @name Protected Methods
///--------------------------------------

- (BOOL)_needsDrawBorder;

- (void)_drawBorderWithContext:(CGContextRef)context size:(CGSize)size;

- (void)_frameDidCalculated:(BOOL)isChanged;

///--------------------------------------
/// @name Private Methods
///--------------------------------------

- (void)_setRenderObject:(void *)object;

- (BOOL)_isCalculatedFrameChanged:(CGRect)frame;

- (CGFloat)_getInnerContentMainSize;

- (void)_assignInnerContentMainSize:(CGFloat)value;

- (void)_assignCalculatedFrame:(CGRect)frame;

- (void)_modifyStyles:(NSDictionary *)styles;

- (void)_transitionUpdateViewProperty:(NSDictionary *)styles;

- (void)_updateCSSNodeStyles:(NSDictionary *)styles;

- (void)_resetCSSNodeStyles:(NSArray *)styles;

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

- (void)attachSlotEvent:(NSDictionary *)data;

- (void)detachSlotEvent:(NSDictionary *)data;

- (void)_buildViewHierarchyLazily;

- (void)_setIsLayoutRTL:(BOOL)isRTL;

- (void)_adjustForRTL;

- (BOOL)_isAffineTypeAs:(NSString *)type;

@end
