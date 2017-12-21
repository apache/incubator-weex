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
#import "WXComponent_internal.h"
#import "WXComponentManager.h"
#import "WXSDKManager.h"
#import "WXSDKInstance.h"
#import "WXSDKInstance_private.h"
#import "WXDefine.h"
#import "WXLog.h"
#import "WXWeakObjectWrapper.h"
#import "WXUtility.h"
#import "WXConvert.h"
#import "WXMonitor.h"
#import "WXAssert.h"
#import "WXThreadSafeMutableDictionary.h"
#import "WXThreadSafeMutableArray.h"
#import "WXTransform.h"
#import "WXRoundedRect.h"
#import <pthread/pthread.h>
#import "WXComponent+PseudoClassManagement.h"
#import "WXComponent+BoxShadow.h"
#import "WXTracingManager.h"
#import "WXComponent+Events.h"

#pragma clang diagnostic ignored "-Wincomplete-implementation"
#pragma clang diagnostic ignored "-Wobjc-protocol-method-implementation"

@interface WXComponent () <UIGestureRecognizerDelegate>

@end

@implementation WXComponent
{
@private
    NSString *_ref;
    NSMutableDictionary *_styles;
    NSMutableDictionary *_attributes;
    NSMutableArray *_events;
    
    // Protects properties styles/attributes/events/subcomponents which will be accessed from multiple threads.
    pthread_mutex_t _propertyMutex;
    pthread_mutexattr_t _propertMutexAttr;
    
    __weak WXComponent *_supercomponent;
    __weak id<WXScrollerProtocol> _ancestorScroller;
    __weak WXSDKInstance *_weexInstance;
}

#pragma mark Life Cycle

- (instancetype)initWithRef:(NSString *)ref
                       type:(NSString *)type
                     styles:(NSDictionary *)styles
                 attributes:(NSDictionary *)attributes
                     events:(NSArray *)events
               weexInstance:(WXSDKInstance *)weexInstance
{
    if (self = [super init]) {
        pthread_mutexattr_init(&_propertMutexAttr);
        pthread_mutexattr_settype(&_propertMutexAttr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&_propertyMutex, &_propertMutexAttr);
        
        _ref = ref;
        _type = type;
        _weexInstance = weexInstance;
        _componentType = WXComponentTypeCommon;
        _styles = [self parseStyles:styles];
        _attributes = attributes ? [NSMutableDictionary dictionaryWithDictionary:attributes] : [NSMutableDictionary dictionary];
        _events = events ? [NSMutableArray arrayWithArray:events] : [NSMutableArray array];
        _subcomponents = [NSMutableArray array];
        _absolutePosition = CGPointMake(NAN, NAN);
        
        _displayType = WXDisplayTypeBlock;
        _isNeedJoinLayoutSystem = YES;
        _isLayoutDirty = YES;
        _isViewFrameSyncWithCalculated = YES;
        _ariaHidden = nil;
        _accessible = nil;
        _accessibilityHintContent = nil;
        
        _async = NO;
        
        if (styles[kWXTransitionProperty]) {
            _transition = [[WXTransition alloc]initWithStyles:styles];
        }

        //TODO set indicator style 
        if ([type isEqualToString:@"indicator"]) {
            _styles[@"position"] = @"absolute";
            if (!_styles[@"left"] && !_styles[@"right"]) {
                _styles[@"left"] = @0.0f;
            }
            if (!_styles[@"top"] && !_styles[@"bottom"]) {
                _styles[@"top"] = @0.0f;
            }
        }
        
        if (attributes[@"ariaHidden"]) {
            
            _ariaHidden = [WXConvert NSString:attributes[@"ariaHidden"]];
        }
        if (attributes[@"role"]) {
            _roles = attributes[@"role"];
        }
        if (attributes[@"ariaLabel"]) {
            _ariaLabel = [WXConvert NSString:attributes[@"ariaLabel"]];
        }
        if (attributes[@"accessible"]) {
            _accessible = [WXConvert NSString:attributes[@"accessible"]];
        }
        if(attributes[@"accessibilityHint"]) {
            _accessibilityHintContent = [WXConvert NSString:attributes[@"accessibilityHint"]];
        }
        if (attributes[@"groupAccessibilityChildren"]) {
            _groupAccessibilityChildren = [WXConvert NSString:attributes[@"groupAccessibilityChildren"]];
        }
        
        if (attributes[@"testId"]) {
            _testId = [WXConvert NSString:attributes[@"testId"]];
        }
        
        [self _setupNavBarWithStyles:_styles attributes:_attributes];
        [self _initCSSNodeWithStyles:_styles];
        [self _initViewPropertyWithStyles:_styles];
        [self _initCompositingAttribute:_attributes];
        [self _handleBorders:styles isUpdating:NO];
        
    }
    
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    NSInteger copyId = 0;
    @synchronized(self){
        static NSInteger __copy = 0;
        copyId = __copy % (1024*1024);
        __copy++;
    }
    NSString *copyRef = [NSString stringWithFormat:@"%zdcopy_of%@", copyId, _isTemplate ? self.ref : self->_templateComponent.ref];
    WXComponent *component = [[[self class] allocWithZone:zone] initWithRef:copyRef type:self.type styles:self.styles attributes:self.attributes events:self.events weexInstance:self.weexInstance];
    if (_isTemplate) {
        component->_templateComponent = self;
    } else {
        component->_templateComponent = self->_templateComponent;
    }
    memcpy(component->_cssNode, self.cssNode, sizeof(css_node_t));
    component->_cssNode->context = (__bridge void *)component;
    component->_calculatedFrame = self.calculatedFrame;
    
    NSMutableArray *subcomponentsCopy = [NSMutableArray array];
    for (WXComponent *subcomponent in self.subcomponents) {
        WXComponent *subcomponentCopy = [subcomponent copy];
        subcomponentCopy->_supercomponent = component;
        [subcomponentsCopy addObject:subcomponentCopy];
    }
    
    component->_subcomponents = subcomponentsCopy;
    
    WXPerformBlockOnComponentThread(^{
        [self.weexInstance.componentManager addComponent:component toIndexDictForRef:copyRef];
    });
    
    return component;
}

- (UIAccessibilityTraits)_parseAccessibilityTraitsWithTraits:(UIAccessibilityTraits)trait roles:(NSString*)roleStr
{
    UIAccessibilityTraits newTrait = trait;
    for (NSString * role in [roleStr componentsSeparatedByString:@" "]) {
        newTrait |= [WXConvert WXUIAccessibilityTraits: role];
    }
    
    return newTrait;
}

- (void)dealloc
{
    free_css_node(_cssNode);

//    [self _removeAllEvents];
    // remove all gesture and all
    if (_tapGesture) {
        [_tapGesture removeTarget:nil action:NULL];
    }
    if ([_swipeGestures count]) {
        for (UISwipeGestureRecognizer *swipeGestures in _swipeGestures) {
            [swipeGestures removeTarget:nil action:NULL];
        }
    }
    
    if (_longPressGesture) {
        [_longPressGesture removeTarget:nil action:NULL];
    }
    
    if (_panGesture) {
        [_panGesture removeTarget:nil action:NULL];
    }
    
    if (_positionType == WXPositionTypeFixed) {
        [self.weexInstance.componentManager removeFixedComponent:self];
    }

    pthread_mutex_destroy(&_propertyMutex);
    pthread_mutexattr_destroy(&_propertMutexAttr);

}

- (NSDictionary *)styles
{
    NSDictionary *styles;
    pthread_mutex_lock(&_propertyMutex);
    styles = _styles;
    pthread_mutex_unlock(&_propertyMutex);
    return styles;
}

- (NSDictionary *)pseudoClassStyles
{
    NSDictionary *pseudoClassStyles;
    pthread_mutex_lock(&_propertyMutex);
    pseudoClassStyles = _pseudoClassStyles;
    pthread_mutex_unlock(&_propertyMutex);
    
    return pseudoClassStyles;
}

- (NSString *)type
{
    return _type;
}

- (NSDictionary *)attributes
{
    NSDictionary *attributes;
    pthread_mutex_lock(&_propertyMutex);
    attributes = _attributes;
    pthread_mutex_unlock(&_propertyMutex);
    
    return attributes;
}

- (NSArray *)events
{
    NSArray *events;
    pthread_mutex_lock(&_propertyMutex);
    events = [_events copy];
    pthread_mutex_unlock(&_propertyMutex);
    
    return events;
}

- (void)setDisplayType:(WXDisplayType)displayType
{
    if (_displayType != displayType) {
        _displayType = displayType;
        if (displayType == WXDisplayTypeNone) {
            _isNeedJoinLayoutSystem = NO;
            [self.supercomponent _recomputeCSSNodeChildren];
            WXPerformBlockOnMainThread(^{
                [self removeFromSuperview];
            });
        } else {
            _isNeedJoinLayoutSystem = YES;
            [self.supercomponent _recomputeCSSNodeChildren];
            WXPerformBlockOnMainThread(^{
                [self _buildViewHierarchyLazily];
                // TODO: insert into the correct index
                [self.supercomponent.view addSubview:self.view];
            });
        }
    }
}

- (WXSDKInstance *)weexInstance
{
    return _weexInstance;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@:%p ref=%@> %@", _type, self, _ref, _view];
}

#pragma mark Property

- (UIView *)view
{
    if (_componentType != WXComponentTypeCommon) {
        return nil;
    }
    if ([self isViewLoaded]) {
        return _view;
    } else {
        WXAssertMainThread();
        
        // compositing child will be drew by its composited ancestor
        if (_isCompositingChild) {
            return nil;
        }
        
        [self viewWillLoad];
        
        _view = [self loadView];
        
        _layer = _view.layer;
        _view.frame = _calculatedFrame;
        _view.hidden = _visibility == WXVisibilityShow ? NO : YES;
        _view.clipsToBounds = _clipToBounds;
        if (![self _needsDrawBorder]) {
            _layer.borderColor = _borderTopColor.CGColor;
            _layer.borderWidth = _borderTopWidth;
            [self _resetNativeBorderRadius];
            _layer.opacity = _opacity;
            _view.backgroundColor = _backgroundColor;
        }

        if (_backgroundImage) {
            [self setGradientLayer];
        }
        
        if (_transform) {
            [_transform applyTransformForView:_view];
        }
        
        if (_boxShadow) {
            [self configBoxShadow:_boxShadow];
        }
        
        _view.wx_component = self;
        _view.wx_ref = self.ref;
        _layer.wx_component = self;
        
        if (_roles) {
            [_view setAccessibilityTraits:[self _parseAccessibilityTraitsWithTraits:self.view.accessibilityTraits roles:_roles]];
        }
        
        if (_testId) {
            _view.accessibilityIdentifier = _testId;
        }
        
        if (_accessibilityHintContent) {
            [_view setAccessibilityHint:_accessibilityHintContent];
        }
        
        if (_ariaLabel) {
            _view.accessibilityLabel = _ariaLabel;
        }
        if (_accessible) {
            [_view setIsAccessibilityElement:[WXConvert BOOL:_accessible]];
        }
        
        if (_ariaHidden) {
            [_view setAccessibilityElementsHidden:[WXConvert BOOL:_ariaHidden]];
        }
        if (_groupAccessibilityChildren) {
            [_view setShouldGroupAccessibilityChildren:[WXConvert BOOL:_groupAccessibilityChildren]];
        }
        
        [self _initEvents:self.events];
        [self _initPseudoEvents:_isListenPseudoTouch];
        
        if (_positionType == WXPositionTypeSticky) {
            [self.ancestorScroller addStickyComponent:self];
        }
        
        if (self.supercomponent && self.supercomponent->_async) {
            self->_async = YES;
        }
        
        [self setNeedsDisplay];
        [[NSNotificationCenter defaultCenter] postNotificationName:WX_COMPONENT_NOTIFICATION_VIEW_LOADED object:self];
        [self viewDidLoad];
        
        if (_lazyCreateView) {
            [self _buildViewHierarchyLazily];
        }

        [self _handleFirstScreenTime];
        
        return _view;
    }
}

- (void)_buildViewHierarchyLazily
{
    if (self.supercomponent && !((WXComponent *)self.supercomponent)->_lazyCreateView) {
        NSArray *subcomponents = ((WXComponent *)self.supercomponent).subcomponents;
        
        NSInteger index = [subcomponents indexOfObject:self];
        if (index != NSNotFound) {
            [(WXComponent *)self.supercomponent insertSubview:self atIndex:index];
        }
    }
    
    NSArray *subcomponents = self.subcomponents;
    for (int i = 0; i < subcomponents.count; i++) {
        WXComponent *subcomponent = subcomponents[i];
        [self insertSubview:subcomponent atIndex:i];
    }
}

- (void)_resetNativeBorderRadius
{
    WXRoundedRect *borderRect = [[WXRoundedRect alloc] initWithRect:_calculatedFrame topLeft:_borderTopLeftRadius topRight:_borderTopRightRadius bottomLeft:_borderBottomLeftRadius bottomRight:_borderBottomRightRadius];
    _layer.cornerRadius = borderRect.radii.topLeft;
}

- (void)_handleFirstScreenTime
{
    if (WX_MONITOR_INSTANCE_PERF_IS_RECORDED(WXPTFirstScreenRender, self.weexInstance)) {
        return;
    }
    CGPoint absolutePosition = [self.supercomponent.view convertPoint:_view.frame.origin toView:_weexInstance.rootView];
    if (absolutePosition.y + _view.frame.size.height > self.weexInstance.rootView.frame.size.height + 1) {
        WX_MONITOR_INSTANCE_PERF_END(WXPTFirstScreenRender, self.weexInstance);
    }
}

- (CALayer *)layer
{
    return _layer;
}

- (CGRect)calculatedFrame
{
    return _calculatedFrame;
}

- (CGPoint)absolutePosition
{
    return _absolutePosition;
}

- (css_node_t *)cssNode
{
    return _cssNode;
}

- (void)_addEventParams:(NSDictionary *)params
{
    pthread_mutex_lock(&_propertyMutex);
    if (!_eventParameters) {
        _eventParameters = [NSMutableDictionary dictionary];
    }
    [_eventParameters addEntriesFromDictionary:params];
    pthread_mutex_unlock(&_propertyMutex);
}

- (NSArray *)_paramsForEvent:(NSString *)eventName
{
    NSArray *params;
    pthread_mutex_lock(&_propertyMutex);
    params = _eventParameters[eventName];
    pthread_mutex_unlock(&_propertyMutex);
    
    return params;
}

#pragma mark Component Hierarchy 

- (NSArray<WXComponent *> *)subcomponents
{
    NSArray<WXComponent *> *subcomponents;
    pthread_mutex_lock(&_propertyMutex);
    subcomponents = [_subcomponents copy];
    pthread_mutex_unlock(&_propertyMutex);
    
    return subcomponents;
}

- (WXComponent *)supercomponent
{
    return _supercomponent;
}

- (void)_insertSubcomponent:(WXComponent *)subcomponent atIndex:(NSInteger)index
{
    WXAssert(subcomponent, @"The subcomponent to insert to %@ at index %d must not be nil", self, index);
    if (index > [_subcomponents count]) {
        WXLogError(@"the index of inserted %ld is out of range as the current is %lu", (long)index, (unsigned long)[_subcomponents count]);
        return;
    }
    
    subcomponent->_supercomponent = self;
    
    pthread_mutex_lock(&_propertyMutex);
    [_subcomponents insertObject:subcomponent atIndex:index];
    pthread_mutex_unlock(&_propertyMutex);
    
    if (subcomponent->_positionType == WXPositionTypeFixed) {
        [self.weexInstance.componentManager addFixedComponent:subcomponent];
        subcomponent->_isNeedJoinLayoutSystem = NO;
    }
    
    if (_useCompositing || _isCompositingChild) {
        subcomponent->_isCompositingChild = YES;
    }
    
    [self _recomputeCSSNodeChildren];
    [self setNeedsLayout];
}

- (void)_removeSubcomponent:(WXComponent *)subcomponent
{
    pthread_mutex_lock(&_propertyMutex);
    [_subcomponents removeObject:subcomponent];
    pthread_mutex_unlock(&_propertyMutex);
}

- (void)_removeFromSupercomponent
{
    [self.supercomponent _removeSubcomponent:self];
    [self.supercomponent _recomputeCSSNodeChildren];
    [self.supercomponent setNeedsLayout];
    
    if (_positionType == WXPositionTypeFixed) {
        [self.weexInstance.componentManager removeFixedComponent:self];
        self->_isNeedJoinLayoutSystem = YES;
    }
}

- (void)_moveToSupercomponent:(WXComponent *)newSupercomponent atIndex:(NSUInteger)index
{
    [self _removeFromSupercomponent];
    [newSupercomponent _insertSubcomponent:self atIndex:index];
}

- (void)_didInserted
{
    
}

- (id<WXScrollerProtocol>)ancestorScroller
{
    if(!_ancestorScroller) {
        WXComponent *supercomponent = self.supercomponent;
        while (supercomponent) {
            if([supercomponent conformsToProtocol:@protocol(WXScrollerProtocol)]) {
                _ancestorScroller = (id<WXScrollerProtocol>)supercomponent;
                break;
            }
            supercomponent = supercomponent.supercomponent;
        }
    }
    
    return _ancestorScroller;
}

#pragma mark Updating
- (void)_updateStylesOnComponentThread:(NSDictionary *)styles resetStyles:(NSMutableArray *)resetStyles isUpdateStyles:(BOOL)isUpdateStyles
{
    BOOL isTransitionTag = _transition ? [self _isTransitionTag:styles] : NO;
    if (isTransitionTag) {
        [_transition _handleTransitionWithStyles:styles resetStyles:resetStyles target:self];
    } else {
        styles = [self parseStyles:styles];
        [self _updateCSSNodeStyles:styles];
        [self _resetCSSNodeStyles:resetStyles];
    }
    if (isUpdateStyles) {
        [self _modifyStyles:styles];
    }
}

- (BOOL)_isTransitionTag:(NSDictionary *)styles
{
    BOOL yesOrNo = false;
    if (_transition.transitionOptions != WXTransitionOptionsNone) {
        yesOrNo = true;
    }
    return yesOrNo;
}

- (BOOL)_isTransitionOnMainThreadStyles:(NSDictionary *)styles
{
    BOOL yesOrNo = false;
    if (_transition.transitionOptions != WXTransitionOptionsNone) {
        if ((_transition.transitionOptions & WXTransitionOptionsBackgroundColor &&styles[@"backgroundColor"])
            ||(_transition.transitionOptions & WXTransitionOptionsTransform &&styles[@"transform"])
            ||(_transition.transitionOptions & WXTransitionOptionsOpacity &&styles[@"opacity"])) {
            yesOrNo = true;
        }
    }
    return yesOrNo;
}

- (void)_modifyStyles:(NSDictionary *)styles
{
    pthread_mutex_lock(&_propertyMutex);
    [_styles addEntriesFromDictionary:styles];
    pthread_mutex_unlock(&_propertyMutex);
}

- (void)_updateAttributesOnComponentThread:(NSDictionary *)attributes
{
    pthread_mutex_lock(&_propertyMutex);
    [_attributes addEntriesFromDictionary:attributes];
    pthread_mutex_unlock(&_propertyMutex);
}

- (void)_addEventOnComponentThread:(NSString *)eventName
{
    pthread_mutex_lock(&_propertyMutex);
    [_events addObject:eventName];
    pthread_mutex_unlock(&_propertyMutex);
}

- (void)_removeEventOnComponentThread:(NSString *)eventName
{
    pthread_mutex_lock(&_propertyMutex);
    [_events removeObject:eventName];
    pthread_mutex_unlock(&_propertyMutex);
}

- (void)_updateStylesOnMainThread:(NSDictionary *)styles resetStyles:(NSMutableArray *)resetStyles
{
    WXAssertMainThread();
    if (![self _isTransitionOnMainThreadStyles:styles]) {
        [self _updateViewStyles:styles];
    } else {
        [self _transitionUpdateViewProperty:styles];
    }
    [self _resetStyles:resetStyles];
    [self _handleBorders:styles isUpdating:YES];
    [self updateStyles:styles];
    [self resetStyles:resetStyles];
}

- (void)_updateAttributesOnMainThread:(NSDictionary *)attributes
{
    WXAssertMainThread();
    
    [self _updateNavBarAttributes:attributes];
    
    [self updateAttributes:attributes];
    [self _configWXComponentA11yWithAttributes:attributes];
}

- (void)updateStyles:(NSDictionary *)styles
{
    WXAssertMainThread();
}

- (void)updateAttributes:(NSDictionary *)attributes
{
    WXAssertMainThread();
}

- (void)setNativeTransform:(CGAffineTransform)transform
{
    WXAssertMainThread();
    
    _transform = [[WXTransform alloc] initWithNativeTransform:CATransform3DMakeAffineTransform(transform) instance:self.weexInstance];
    if (!CGRectEqualToRect(self.calculatedFrame, CGRectZero)) {
        [_transform applyTransformForView:_view];
        [_layer setNeedsDisplay];
    }
}

- (void)readyToRender
{
    if (self.weexInstance.trackComponent) {
        [self.supercomponent readyToRender];
    }
}


- (void)setGradientLayer
{
    if (CGRectEqualToRect(self.view.frame, CGRectZero)) {
        return;
    }
    NSDictionary * linearGradient = [WXUtility linearGradientWithBackgroundImage:_backgroundImage];
    if (!linearGradient) {
        return ;
    }
    
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf;
        if(strongSelf) {
            UIColor * startColor = (UIColor*)linearGradient[@"startColor"];
            UIColor * endColor = (UIColor*)linearGradient[@"endColor"];
            CAGradientLayer * gradientLayer = [WXUtility gradientLayerFromColors:@[startColor, endColor] locations:nil frame:strongSelf.view.bounds gradientType:[linearGradient[@"gradientType"] integerValue]];
            if (gradientLayer) {
                _backgroundColor = [UIColor colorWithPatternImage:[strongSelf imageFromLayer:gradientLayer]];
                strongSelf.view.backgroundColor = _backgroundColor;
            }
        }
    });
}

- (void)_configWXComponentA11yWithAttributes:(NSDictionary *)attributes
{
    WX_CHECK_COMPONENT_TYPE(self.componentType)
    if (attributes[@"role"]){
        _roles = attributes[@"role"];
        [self.view setAccessibilityTraits:[self _parseAccessibilityTraitsWithTraits:self.view.accessibilityTraits roles:_roles]];
    }
    if (attributes[@"ariaHidden"]) {
        _ariaHidden = [WXConvert NSString:attributes[@"ariaHidden"]];
        [self.view setAccessibilityElementsHidden:[WXConvert BOOL:_ariaHidden]];
    }
    if (attributes[@"accessible"]) {
        _accessible = [WXConvert NSString:attributes[@"accessible"]];
        [self.view setIsAccessibilityElement:[WXConvert BOOL:_accessible]];
    }
    if (attributes[@"ariaLabel"]) {
        _ariaLabel = [WXConvert NSString:attributes[@"ariaLabel"]];
        self.view.accessibilityValue = _ariaLabel;
    }
    if (attributes[@"accessibilityHint"]) {
        _accessibilityHintContent = [WXConvert NSString:attributes[@"accessibilityHint"]];
        [self.view setAccessibilityHint:_accessibilityHintContent];
    }
    
    if (attributes[@"groupAccessibilityChildren"]) {
        _groupAccessibilityChildren = [WXConvert NSString:attributes[@"groupAccessibilityChildren"]];
        [self.view setShouldGroupAccessibilityChildren:[WXConvert BOOL:_groupAccessibilityChildren]];
    }

    
    if (attributes[@"testId"]) {
        [self.view setAccessibilityIdentifier:[WXConvert NSString:attributes[@"testId"]]];
    }

}

- (UIImage *)imageFromLayer:(CALayer *)layer
{
    UIGraphicsBeginImageContextWithOptions(layer.frame.size, NO, 0);
    [layer renderInContext:UIGraphicsGetCurrentContext()];
    UIImage *outputImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return outputImage;
}

#pragma mark Reset
- (void)resetStyles:(NSArray *)styles
{
    WXAssertMainThread();
}

#pragma mark Layout

/**
 *  @see WXComponent+Layout.m
 */

#pragma mark View Management

/**
 *  @see WXComponent+ViewManagement.m
 */

#pragma mark Events

/**
 *  @see WXComponent+Events.m
 */

#pragma mark Display

/**
 *  @see WXComponent+Display.m
 */

@end


@implementation UIView (WXComponent)

- (WXComponent *)wx_component
{
    WXWeakObjectWrapper *weakWrapper = objc_getAssociatedObject(self, @selector(wx_component));
    return [weakWrapper weakObject];
}

- (void)setWx_component:(WXComponent *)wx_component
{
    id weakWrapper = [[WXWeakObjectWrapper alloc] initWithWeakObject:wx_component];
    objc_setAssociatedObject(self, @selector(wx_component), weakWrapper, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

- (NSString *)wx_ref
{
    WXWeakObjectWrapper *weakWrapper = objc_getAssociatedObject(self, @selector(wx_ref));
    return [weakWrapper weakObject];
}

- (void)setWx_ref:(NSString *)wx_ref
{
    id weakWrapper = [[WXWeakObjectWrapper alloc] initWithWeakObject:wx_ref];
    objc_setAssociatedObject(self, @selector(wx_ref), weakWrapper, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

@end

@implementation CALayer (WXComponents_new)

- (WXComponent *)wx_component
{
    WXWeakObjectWrapper *weakWrapper = objc_getAssociatedObject(self, @selector(wx_component));
    return [weakWrapper weakObject];
}

- (void)setWx_component:(WXComponent *)wx_component
{
    id weakWrapper = [[WXWeakObjectWrapper alloc] initWithWeakObject:wx_component];
    objc_setAssociatedObject(self, @selector(wx_component), weakWrapper, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

@end
