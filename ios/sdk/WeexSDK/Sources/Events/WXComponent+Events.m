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

#import "WXComponent+Events.h"
#import "WXComponent.h"
#import "WXComponent_internal.h"
#import "WXSDKInstance.h"
#import "WXComponentManager.h"
#import "WXAssert.h"
#import "WXUtility.h"
#import "WXSDKManager.h"
#import "WXSDKInstance_private.h"
#import "WXDefine.h"
#import "WXRecycleListComponent.h"
#import "WXRecycleListDataManager.h"
#import <objc/runtime.h>
#import <UIKit/UIGestureRecognizerSubclass.h>
#import "WXComponent+PseudoClassManagement.h"
#import "WXCoreBridge.h"

#pragma clang diagnostic ignored "-Wobjc-protocol-method-implementation"

@interface UITouch (WXTouchGestureRecognizer)
@property (nonatomic, strong) NSNumber *wx_identifier;
@property (nonatomic, strong) NSNumber *wx_stopPropagation;
@end

@implementation UITouch (WXTouchGestureRecognizer)
- (NSNumber *)wx_identifier
{
    return objc_getAssociatedObject(self, _cmd);
}

- (void)setWx_identifier:(NSNumber *)wx_identifier
{
    objc_setAssociatedObject(self, @selector(wx_identifier), wx_identifier, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

- (NSNumber *)wx_stopPropagation
{
    return objc_getAssociatedObject(self, _cmd);
}

- (void)setWx_stopPropagation:(NSNumber *)wx_stopPropagation
{
    objc_setAssociatedObject(self, @selector(wx_stopPropagation), wx_stopPropagation, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
@end

@interface UIGestureRecognizer (WXGesture)
@property (nonatomic, strong) NSNumber *wx_identifier;
@end

@implementation UIGestureRecognizer (WXGesture)
- (NSNumber *)wx_identifier
{
    NSNumber *identifier = objc_getAssociatedObject(self, _cmd);
    if (!identifier) {
        static NSUInteger _gestureIdentifier;
        identifier = @(_gestureIdentifier++);
        self.wx_identifier = identifier;
    }
    
    return identifier;
}

- (void)setWx_identifier:(NSNumber *)wx_identifier
{
    objc_setAssociatedObject(self, @selector(wx_identifier), wx_identifier, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
@end

@interface WXTouchGestureRecognizer : UIGestureRecognizer

@property (nonatomic, assign) BOOL listenTouchStart;
@property (nonatomic, assign) BOOL listenTouchMove;
@property (nonatomic, assign) BOOL listenTouchEnd;
@property (nonatomic, assign) BOOL listenTouchCancel;
@property (nonatomic, assign) BOOL listenPseudoTouch;

- (instancetype)initWithComponent:(WXComponent *)component NS_DESIGNATED_INITIALIZER;

@end

@interface WXEventManager :NSObject
+ (instancetype) sharedManager;
- (BOOL)stopPropagation:(NSString *)instanceId ref:(NSString *)ref type:(NSString *)type params:(NSDictionary *)params;
@end

@implementation WXEventManager

- (instancetype) init
{
    self = [super init];
    if (self) {
        
    }
    return self;
}

+ (instancetype)sharedManager
{
    static id _sharedInstance = nil;
    static dispatch_once_t oncePredicate;
    dispatch_once(&oncePredicate, ^{
        _sharedInstance = [[self alloc] init];
    });
    return _sharedInstance;
}

- (BOOL)stopPropagation:(NSString *)instanceId ref:(NSString *)ref type:(NSString *)type params:(NSDictionary *)params
{
    JSValue *value = [[WXSDKManager bridgeMgr] fireEventWithResult:instanceId ref:ref type:type params:params domChanges:nil];
    
    if ([value.toString isEqualToString:@"true"]) {
        return YES;
    }
    return NO;
}

@end

@implementation WXComponent (Events)

#pragma mark Public

- (void)fireEvent:(NSString *)eventName params:(NSDictionary *)params
{
    [self fireEvent:eventName params:params domChanges:nil];
}

- (void)fireEvent:(NSString *)eventName params:(NSDictionary *)params domChanges:(NSDictionary *)domChanges
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    NSTimeInterval timeSp = [[NSDate date] timeIntervalSince1970] * 1000;
    [dict setObject:@(timeSp) forKey:@"timestamp"];
    if (params) {
        [dict addEntriesFromDictionary:params];
    }
    WXRecycleListComponent *recyleListComponent  = (WXRecycleListComponent*)[self getRecycleListComponent];
    if (recyleListComponent) {
        NSIndexPath *indexPath = [((UICollectionView*)recyleListComponent.view) indexPathForItemAtPoint:[self.view.superview
                                                                                                          convertPoint:self.view.center toView:recyleListComponent.view]];
        NSString *templateId = [self recursiveFindTemplateIdWithComponent:self];
        NSString *virtualComponentId = [recyleListComponent.dataManager virtualComponentIdWithIndexPath:indexPath templateId:templateId];
        if (virtualComponentId) {
            dict[@"componentId"] = virtualComponentId;
        }
    }
    
    NSArray *handlerArguments = [self _paramsForEvent:eventName];
    NSString *ref = _templateComponent ? _templateComponent.ref  : self.ref;
    if (self.weexInstance.dataRender) {
        WXPerformBlockOnComponentThread(^{
            [WXCoreBridge fireEvent:self.weexInstance.instanceId ref:ref event:eventName args:dict];
        });
    }
    else {
        [[WXSDKManager bridgeMgr] fireEvent:self.weexInstance.instanceId ref:ref type:eventName params:dict domChanges:domChanges handlerArguments:handlerArguments];
    }
}

- (NSString *)recursiveFindTemplateIdWithComponent:(WXComponent *)component
{
    if (!component) {
        return nil;
    }
    if ([component isKindOfClass:NSClassFromString(@"WXCellSlotComponent")]) {
        return nil;
    }
    if (component.attributes[@"@templateId"]) {
        return component.attributes[@"@templateId"];
    }
    return [self recursiveFindTemplateIdWithComponent:component.supercomponent];
}

- (void)addEvent:(NSString *)addEventName
{
    WXAssertMainThread();
}

- (void)removeEvent:(NSString *)removeEventName
{
    WXAssertMainThread();
}

#pragma mark Add & Remove Event

#define WX_ADD_EVENT(eventName, addSelector) \
if ([addEventName isEqualToString:@#eventName]) {\
    [self addSelector];\
}

#define WX_ADD_EVENTS(eventName1,eventName2, addSelector) \
if ([addEventName isEqualToString:@#eventName1]||[addEventName isEqualToString:@#eventName2]) {\
    [self addSelector:addEventName];\
}

#define WX_REMOVE_EVENT(eventName, removeSelector) \
if ([removeEventName isEqualToString:@#eventName]) {\
    [self removeSelector];\
}

#define WX_REMOVE_EVENTS(eventName1,eventName2, removeSelector) \
if ([removeEventName isEqualToString:@#eventName1]||[removeEventName isEqualToString:@#eventName2]) {\
    [self removeSelector];\
}

- (void)_initEvents:(NSArray *)events
{
    for (NSString *addEventName in events) {
        [self _addEventOnMainThread:addEventName];
    }
}

- (void)_initPseudoEvents:(BOOL)isListenPseudoTouch
{
    if(isListenPseudoTouch) {
        self.touchGesture.listenPseudoTouch = YES;
    }
}

- (void)_addEventOnMainThread:(NSString *)addEventName
{
    if (![self isViewLoaded]) {
        //This action will be ignored While the view is loaded,
        //then it will initEvent according to the records in _events
        return;
    }
    WX_ADD_EVENT(appear, addAppearEvent)
    WX_ADD_EVENT(disappear, addDisappearEvent)
    
    WX_ADD_EVENT(click, addClickEvent)
    WX_ADD_EVENT(swipe, addSwipeEvent)
    WX_ADD_EVENT(longpress, addLongPressEvent)
    
    WX_ADD_EVENT(panstart, addPanStartEvent)
    WX_ADD_EVENT(panmove, addPanMoveEvent)
    WX_ADD_EVENT(panend, addPanEndEvent)
    
    WX_ADD_EVENT(horizontalpan, addHorizontalPanEvent)
    WX_ADD_EVENT(verticalpan, addVerticalPanEvent)
    
    WX_ADD_EVENT(touchstart, addTouchStartEvent)
    WX_ADD_EVENT(touchmove, addTouchMoveEvent)
    WX_ADD_EVENT(touchend, addTouchEndEvent)
    WX_ADD_EVENT(touchcancel, addTouchCancelEvent)
    WX_ADD_EVENT(accessibilityMagicTap, addAccessibilityMagicTapEvent)
    
    WX_ADD_EVENTS(stopPropagation, stoppropagation, addStopPropagationEvent)
    
    if(_isListenPseudoTouch) {
        self.touchGesture.listenPseudoTouch = YES;
    }
    
    [self addEvent:addEventName];
}

- (void)_removeEventOnMainThread:(NSString *)removeEventName
{
    WX_REMOVE_EVENT(appear, removeAppearEvent)
    WX_REMOVE_EVENT(disappear, removeDisappearEvent)
    
    WX_REMOVE_EVENT(click, removeClickEvent)
    WX_REMOVE_EVENT(swipe, removeSwipeEvent)
    WX_REMOVE_EVENT(longpress, removeLongPressEvent)
    
    WX_REMOVE_EVENT(panstart, removePanStartEvent)
    WX_REMOVE_EVENT(panmove, removePanMoveEvent)
    WX_REMOVE_EVENT(panend, removePanEndEvent)
    
    WX_REMOVE_EVENT(horizontalpan, removeHorizontalPanEvent)
    WX_REMOVE_EVENT(verticalpan, removeVerticalPanEvent)
    
    WX_REMOVE_EVENT(touchstart, removeTouchStartEvent)
    WX_REMOVE_EVENT(touchmove, removeTouchMoveEvent)
    WX_REMOVE_EVENT(touchend, removeTouchEndEvent)
    WX_REMOVE_EVENT(touchcancel, removeTouchCancelEvent)
    WX_REMOVE_EVENT(accessibilityMagicTap, removeAccessibilityMagicTapEvent)
    
    WX_REMOVE_EVENTS(stoppropagation,stopPropagation, removeStopPropagationEvent)

    if(_isListenPseudoTouch) {
        self.touchGesture.listenPseudoTouch = NO;
    }

    [self removeEvent:removeEventName];
}

- (void)_removeAllEvents
{
    [self removeClickEvent];
    [self removeLongPressEvent];
    [self removePanStartEvent];
    [self removePanMoveEvent];
    [self removePanEndEvent];
    [self removeHorizontalPanEvent];
    [self removeVerticalPanEvent];
    
    [self removeTouchStartEvent];
    [self removeTouchMoveEvent];
    [self removeTouchEndEvent];
    [self removeTouchCancelEvent];
    [self removeSwipeEvent];
    [self removePseudoTouchEvent];
}

- (void)_collectSubcomponents:(NSMutableArray *)components
{
    for (WXComponent* c in _subcomponents) {
        [components addObject:c];
        [c _collectSubcomponents:components];
    }
}

#pragma mark - Appear & Disappear

- (void)addAppearEvent
{
    _appearEvent = YES;
    [self.ancestorScroller addScrollToListener:self];
}

- (void)addDisappearEvent
{
    _disappearEvent = YES;
    [self.ancestorScroller addScrollToListener:self];
}

- (void)removeAppearEvent
{
    _appearEvent = NO;
    [self.ancestorScroller removeScrollToListener:self];
}

- (void)removeDisappearEvent
{
    _disappearEvent = NO;
    [self.ancestorScroller removeScrollToListener:self];
}

- (void)removePseudoTouchEvent
{
    _touchGesture.listenPseudoTouch = NO;
    [self checkRemoveTouchGesture];
}

#pragma mark - Accessibility Event

- (void)addAccessibilityMagicTapEvent
{
    _accessibilityMagicTapEvent = YES;
}

- (void)removeAccessibilityMagicTapEvent
{
    _accessibilityMagicTapEvent = NO;
}

#pragma mark - StopPropagation

- (void)addStopPropagationEvent:(NSString *)stopPropagationName
{
    _listenStopPropagation = YES;
    _stopPropagationName = stopPropagationName;
    self.touchGesture.listenTouchMove = YES;
}

- (void)removeStopPropagationEvent
{
    _listenStopPropagation = NO;
    self.touchGesture.listenTouchMove = NO;
}

#pragma mark - Click Event

- (void)addClickEvent
{
    if (!_tapGesture) {
        _tapGesture = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onClick:)];
        _tapGesture.delegate = self;
        [self.view addGestureRecognizer:_tapGesture];
    }
}

- (void)removeClickEvent
{
    if (_tapGesture) {
        _tapGesture.delegate = nil;
        if ([self isViewLoaded]) {
            if ([self.view.gestureRecognizers containsObject:_tapGesture]) {
                [self.view removeGestureRecognizer:_tapGesture];
            }
        }
        @try {
            [_tapGesture removeTarget:self action:@selector(onClick:)];
        }@catch(NSException *exception) {
            WXLog(@"%@", exception);
        } @finally {
            
        }
        _tapGesture = nil;
    }
}

- (void)onClick:(__unused UITapGestureRecognizer *)recognizer
{
    NSMutableDictionary *position = [[NSMutableDictionary alloc] initWithCapacity:4];
    CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
    if (![self isViewLoaded]) {
        return;
    }
    if (!CGRectEqualToRect(self.view.frame, CGRectZero)) {
        CGRect frame = [self.view.superview convertRect:self.view.frame toView:self.view.window];
        position[@"x"] = @(frame.origin.x/scaleFactor);
        position[@"y"] = @(frame.origin.y/scaleFactor);
        position[@"width"] = @(frame.size.width/scaleFactor);
        position[@"height"] = @(frame.size.height/scaleFactor);
    }
    [self fireEvent:@"click" params:@{@"position":position}];
}

#pragma mark - Swipe event

- (void)addSwipeEvent
{
    if (_swipeGestures) {
        return;
    }
    
    _swipeGestures = [NSMutableArray arrayWithCapacity:4];
    
    // It's a little weird because the UISwipeGestureRecognizer.direction property is an options-style bit mask, but each recognizer can only handle one direction
    SEL selector = @selector(onSwipe:);
    UISwipeGestureRecognizer *upSwipeRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self
                                                                                            action:selector];
    upSwipeRecognizer.direction = UISwipeGestureRecognizerDirectionUp;
    upSwipeRecognizer.delegate = self;
    [_swipeGestures addObject:upSwipeRecognizer];
    [self.view addGestureRecognizer:upSwipeRecognizer];
    
    UISwipeGestureRecognizer *downSwipeRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self
                                                                                              action:selector];
    downSwipeRecognizer.direction = UISwipeGestureRecognizerDirectionDown;
    downSwipeRecognizer.delegate = self;
    [_swipeGestures addObject:downSwipeRecognizer];
    [self.view addGestureRecognizer:downSwipeRecognizer];
    
    UISwipeGestureRecognizer *rightSwipeRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self
                                                                                               action:selector];
    rightSwipeRecognizer.direction = UISwipeGestureRecognizerDirectionRight;
    rightSwipeRecognizer.delegate = self;
    [_swipeGestures addObject:rightSwipeRecognizer];
    [self.view addGestureRecognizer:rightSwipeRecognizer];
    
    UISwipeGestureRecognizer *leftSwipeRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:self
                                                                                              action:selector];
    leftSwipeRecognizer.direction = UISwipeGestureRecognizerDirectionLeft;
    leftSwipeRecognizer.delegate = self;
    [_swipeGestures addObject:leftSwipeRecognizer];
    [self.view addGestureRecognizer:leftSwipeRecognizer];
}

- (void)removeSwipeEvent
{
    if (_swipeGestures == nil) {
        return;
    }
  
    @try {
        for (UISwipeGestureRecognizer *recognizer in _swipeGestures) {
            recognizer.delegate = nil;
            if([self isViewLoaded]) {
                if ([[self.view gestureRecognizers] containsObject:recognizer]) {
                    [self.view removeGestureRecognizer:recognizer];
                }
            }
            [recognizer removeTarget:self action:@selector(onSwipe:)];
        }
    }@catch(NSException *exception) {
        WXLog(@"%@", exception);
    }@finally {
        
    }
    _swipeGestures = nil;
}

- (void)onSwipe:(UISwipeGestureRecognizer *)gesture
{
    if (![self isViewLoaded]) {
        return;
    }
    
    UISwipeGestureRecognizerDirection direction = gesture.direction;
    
    NSString *directionString;
    switch(direction) {
        case UISwipeGestureRecognizerDirectionLeft:
            directionString = @"left";
            break;
        case UISwipeGestureRecognizerDirectionRight:
            directionString = @"right";
            break;
        case UISwipeGestureRecognizerDirectionUp:
            directionString = @"up";
            break;
        case UISwipeGestureRecognizerDirectionDown:
            directionString = @"down";
            break;
        default:
            directionString = @"unknown";
            break;
    }
    
    CGPoint screenLocation = [gesture locationInView:self.view.window];
    CGPoint pageLoacation = [gesture locationInView:self.weexInstance.rootView];
    NSDictionary *resultTouch = [self touchResultWithScreenLocation:screenLocation pageLocation:pageLoacation identifier:gesture.wx_identifier];
    [self fireEvent:@"swipe" params:@{@"direction":directionString, @"changedTouches":resultTouch ? @[resultTouch] : @[]}];
}

#pragma mark - Long Press

- (void)addLongPressEvent
{
    if (!_longPressGesture) {
        _longPressGesture = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(onLongPress:)];
        _longPressGesture.delegate = self;
        [self.view addGestureRecognizer:_longPressGesture];
    }
}

- (void)removeLongPressEvent
{
    if (_longPressGesture) {
        _longPressGesture.delegate = nil;
        if ([self isViewLoaded]) {
            if([[self.view gestureRecognizers] containsObject:_longPressGesture]) {
                [self.view removeGestureRecognizer:_longPressGesture];
            }
        }
        @try {
            [_longPressGesture removeTarget:self action:@selector(onLongPress:)];
        }@catch(NSException * exception) {
            WXLog(@"%@", exception);
        }@finally {
            
        }
        _longPressGesture = nil;
    }
}

- (void)onLongPress:(UILongPressGestureRecognizer *)gesture
{
    if (![self isViewLoaded]) {
        return;
    }
    
    if (gesture.state == UIGestureRecognizerStateBegan) {
        CGPoint screenLocation = [gesture locationInView:self.view.window];
        CGPoint pageLoacation = [gesture locationInView:self.weexInstance.rootView];
        NSDictionary *resultTouch = [self touchResultWithScreenLocation:screenLocation pageLocation:pageLoacation identifier:gesture.wx_identifier];
        [self fireEvent:@"longpress" params:@{@"changedTouches":resultTouch ? @[resultTouch] : @[]}];
    } else if (gesture.state == UIGestureRecognizerStateEnded) {
        gesture.wx_identifier = nil;
    }
}

#pragma mark - Pan

- (void)addPanGesture
{
    if (!_panGesture) {
        _panGesture = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(onPan:)];
        _panGesture.delegate = self;
        [self.view addGestureRecognizer:_panGesture];
    }
}

- (void)addPanStartEvent
{
    _listenPanStart = YES;
    [self addPanGesture];
}

- (void)addPanMoveEvent
{
    _listenPanMove = YES;
    [self addPanGesture];
}

- (void)addPanEndEvent
{
    _listenPanEnd = YES;
    [self addPanGesture];
}

- (void)addHorizontalPanEvent
{
    _listenHorizontalPan = YES;
    [self addPanGesture];
}

- (void)addVerticalPanEvent
{
    _listenVerticalPan = YES;
    [self addPanGesture];
}


- (void)onPan:(UIPanGestureRecognizer *)gesture
{
    if (![self isViewLoaded]) {
        return;
    }
    
    CGPoint screenLocation = [gesture locationInView:self.view.window];
    CGPoint pageLoacation = [gesture locationInView:self.weexInstance.rootView];
    NSString *eventName;
    NSString *state = @"";
    NSDictionary *resultTouch = [self touchResultWithScreenLocation:screenLocation pageLocation:pageLoacation identifier:gesture.wx_identifier];
    
    if (gesture.state == UIGestureRecognizerStateBegan) {
        if (_listenPanStart) {
            eventName = @"panstart";
        }
        state = @"start";
    } else if (gesture.state == UIGestureRecognizerStateEnded) {
        if (_listenPanEnd) {
            eventName = @"panend";
        }
        state = @"end";
        gesture.wx_identifier = nil;
    } else if (gesture.state == UIGestureRecognizerStateChanged) {
        if (_listenPanMove) {
             eventName = @"panmove";
        }
        state = @"move";
    } else if (gesture.state == UIGestureRecognizerStateCancelled) {
        state = @"cancel";
    }
    
    CGPoint translation = [_panGesture translationInView:self.view];
    
    if (_listenHorizontalPan && (gesture.state != UIGestureRecognizerStateBegan || fabs(translation.y) <= fabs(translation.x))) {
        [self fireEvent:@"horizontalpan" params:@{@"state":state, @"changedTouches":resultTouch ? @[resultTouch] : @[]}];
    }
        
    if (_listenVerticalPan && (gesture.state != UIGestureRecognizerStateBegan || fabs(translation.y) >= fabs(translation.x))) {
        [self fireEvent:@"verticalpan" params:@{@"state":state, @"changedTouches":resultTouch ? @[resultTouch] : @[]}];
    }
        
    if (eventName) {
        [self fireEvent:eventName params:@{@"changedTouches":resultTouch ? @[resultTouch] : @[]}];
    }
}

- (void)removePanStartEvent
{
    _listenPanStart = NO;
    [self checkRemovePanGesture];
}

- (void)removePanMoveEvent
{
    _listenPanMove = NO;
    [self checkRemovePanGesture];
}

- (void)removePanEndEvent
{
    _listenPanEnd = NO;
    [self checkRemovePanGesture];
}

- (void)removeHorizontalPanEvent
{
    _listenHorizontalPan = NO;
    [self checkRemovePanGesture];
}

- (void)removeVerticalPanEvent
{
    _listenVerticalPan = NO;
    [self checkRemovePanGesture];
}

- (void)checkRemovePanGesture
{
    if (_panGesture
        && !_listenPanStart && !_listenPanMove && !_listenPanEnd
        && !_listenHorizontalPan && !_listenVerticalPan
        ) {
        
        if ([self isViewLoaded]) {
            if ([[self.view gestureRecognizers] containsObject:_panGesture]) {
                [self.view removeGestureRecognizer:_panGesture];
            }
        }
        
        _panGesture.delegate = nil;
        @try {
            [_panGesture removeTarget:self action:@selector(onPan:)];
        }@catch(NSException * exception) {
            WXLog(@"%@", exception);
        }@finally {
            
        }
        _panGesture = nil;
    }
}

#pragma mark - Touch Event

- (WXTouchGestureRecognizer *)touchGesture
{
    if (!_touchGesture) {
        _touchGesture = [[WXTouchGestureRecognizer alloc] initWithComponent:self];
        _touchGesture.delegate = self;
        [self.view addGestureRecognizer:_touchGesture];
    }
    return _touchGesture;
}

- (void)addTouchStartEvent
{
    self.touchGesture.listenTouchStart = YES;
}

- (void)addTouchMoveEvent
{
    self.touchGesture.listenTouchMove = YES;
}

- (void)addTouchEndEvent
{
    self.touchGesture.listenTouchEnd = YES;
}

- (void)addTouchCancelEvent
{
    self.touchGesture.listenTouchCancel = YES;
}

- (void)removeTouchStartEvent
{
    _touchGesture.listenTouchStart = NO;
    [self checkRemoveTouchGesture];
}

- (void)removeTouchMoveEvent
{
    _touchGesture.listenTouchMove = NO;
    [self checkRemoveTouchGesture];
}

- (void)removeTouchEndEvent
{
    _touchGesture.listenTouchEnd = NO;
    [self checkRemoveTouchGesture];
}

- (void)removeTouchCancelEvent
{
    _touchGesture.listenTouchCancel = NO;
    [self checkRemoveTouchGesture];
}

- (void)checkRemoveTouchGesture
{
    if (_touchGesture && !_touchGesture.listenTouchStart && !_touchGesture.listenTouchMove && !_touchGesture.listenTouchEnd && !_touchGesture.listenTouchCancel && !_touchGesture.listenPseudoTouch) {
        _touchGesture.delegate = nil;
        if ([self isViewLoaded]) {
            if ([[self.view gestureRecognizers] containsObject:_touchGesture]) {
                [self.view removeGestureRecognizer:_touchGesture];
            }
        }
        _touchGesture = nil;
    }
}

- (BOOL)gestureShouldStopPropagation:(UIGestureRecognizer *)gestureRecognizer shouldReceiveTouch:(UITouch *)touch
{
    if(touch.wx_stopPropagation && [touch.wx_stopPropagation isEqualToNumber:@1]){
        return NO;
    }
    else
    {
        if (_listenStopPropagation)
        {
            NSString *ref = _templateComponent ? _templateComponent.ref : self.ref;
            CGPoint screenLocation = [touch locationInView:touch.window];
            CGPoint pageLocation = [touch locationInView:self.weexInstance.rootView];
            NSDictionary *resultTouch = [self touchResultWithScreenLocation:screenLocation pageLocation:pageLocation identifier:touch.wx_identifier];
            NSString *touchState;
            if (touch.phase == UITouchPhaseBegan) {
                touchState = @"start";
            }
            else if (touch.phase == UITouchPhaseMoved){
                touchState = @"move";
            }
            else{
                touchState = @"end";
            }
            BOOL stopPropagation = [[WXEventManager sharedManager]stopPropagation:self.weexInstance.instanceId ref:ref type:_stopPropagationName params:@{@"changedTouches":resultTouch ? @[resultTouch] : @[],@"action":touchState}];
            touch.wx_stopPropagation = stopPropagation ? @1 : @0;
        }
    }
    return YES;
}

#pragma mark - UIGestureRecognizerDelegate
- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldReceiveTouch:(UITouch *)touch
{
    return [self gestureShouldStopPropagation:gestureRecognizer shouldReceiveTouch:touch];
}

- (BOOL)gestureRecognizerShouldBegin:(UIGestureRecognizer *)gestureRecognizer
{
    if (gestureRecognizer == _panGesture) {
        CGPoint translation = [_panGesture translationInView:self.view];
        if (_listenHorizontalPan && !_listenVerticalPan && fabs(translation.y) > fabs(translation.x)) {
            return NO;
        }
    }
    return YES;
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    NSString * panGestureRecog = [NSString stringWithFormat:@"%@%@%@%@%@%@",@"UIScrollV", @"iewPanG", @"estur",@"eRecog",@"nize",@"r"];
    NSString * textTap = [NSString stringWithFormat:@"%@%@%@%@%@",@"UITe",@"xtTa",@"pReco",@"gniz",@"er"];
    // trigger touches
    if ([gestureRecognizer isKindOfClass:[WXTouchGestureRecognizer class]]) {
        return YES;
    }
    // swipe and scroll
    if ([gestureRecognizer isKindOfClass:[UISwipeGestureRecognizer class]] && [otherGestureRecognizer isKindOfClass:NSClassFromString(panGestureRecog)]) {
        return YES;
    }
    // onclick and textviewInput
    if ([gestureRecognizer isKindOfClass:[UITapGestureRecognizer class]] && [otherGestureRecognizer isKindOfClass: NSClassFromString(textTap)]) {
        return YES;
    }
    
    return NO;
}

#pragma mark - Utils

- (NSDictionary *)touchResultWithScreenLocation:(CGPoint)screenLocation pageLocation:(CGPoint)pageLocation identifier:(NSNumber *)identifier
{
    NSMutableDictionary *resultTouch = [[NSMutableDictionary alloc] initWithCapacity:5];
    CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
    resultTouch[@"screenX"] = @(screenLocation.x/scaleFactor);
    resultTouch[@"screenY"] = @(screenLocation.y/scaleFactor);
    resultTouch[@"pageX"] = @(pageLocation.x/scaleFactor);
    resultTouch[@"pageY"] = @(pageLocation.y/scaleFactor);
    resultTouch[@"identifier"] = identifier;
    
    return resultTouch;
}

// find virtual component's root component
- (WXComponent*)getRecycleListComponent
{
    if ([self isKindOfClass:[WXRecycleListComponent class]]) {
        return self;
    }
    if ([self.ref isEqualToString:WX_SDK_ROOT_REF]) {
        return nil;
    }
    return [self.supercomponent getRecycleListComponent];
}

@end

@implementation WXTouchGestureRecognizer
{
    __weak WXComponent *_component;
    NSUInteger _touchIdentifier;
}

- (instancetype)initWithTarget:(id)target action:(SEL)action
{
    return [self initWithComponent:nil];;
}

- (instancetype)initWithComponent:(WXComponent *)component
{
    if (self = [super initWithTarget:self action:@selector(touchResponse:)]) {
        _component = component;
        
        _listenTouchStart = NO;
        _listenTouchEnd = NO;
        _listenTouchMove = NO;
        _listenTouchCancel = NO;
        
        self.cancelsTouchesInView = NO;
    }
    
    return self;
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [super touchesBegan:touches withEvent:event];
    
    if (_listenTouchStart) {
        [self fireTouchEvent:@"touchstart" withTouches:touches];
    }
    if(_listenPseudoTouch) {
        NSMutableDictionary *styles = [_component getPseudoClassStyles:@"active"];
        [_component updatePseudoClassStyles:styles];
    }

}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [super touchesMoved:touches withEvent:event];
    
    if (_listenTouchMove) {
        [self fireTouchEvent:@"touchmove" withTouches:touches];
    }
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    
    [super touchesEnded:touches withEvent:event];
    
    if (_listenTouchEnd) {
        [self fireTouchEvent:@"touchend" withTouches:touches];
    }
    if(_listenPseudoTouch) {
        [self recoveryPseudoStyles:_component.styles];
    }

}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [super touchesCancelled:touches withEvent:event];
    
    if (_listenTouchCancel) {
        [self fireTouchEvent:@"touchcancel" withTouches:touches];
    }
    if(_listenPseudoTouch) {
        [self recoveryPseudoStyles:_component.styles];
    }
}

- (void)fireTouchEvent:(NSString *)eventName withTouches:(NSSet<UITouch *> *)touches
{
    NSMutableArray *resultTouches = [NSMutableArray new];
    
    for (UITouch *touch in touches) {
        CGPoint screenLocation = [touch locationInView:touch.window];
        CGPoint pageLocation = [touch locationInView:_component.weexInstance.rootView];
        if (!touch.wx_identifier) {
            touch.wx_identifier = @(_touchIdentifier++);
        }
        NSDictionary *resultTouch = [_component touchResultWithScreenLocation:screenLocation pageLocation:pageLocation identifier:touch.wx_identifier];
        NSMutableDictionary * mutableResultTouch = [resultTouch mutableCopy];
        
        if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"9.0")) {
            float value = touch.force*60;
            float maxValue = touch.maximumPossibleForce*60;
            if (touch.maximumPossibleForce) {
                // the forece value will be range 1 from 0.
                [mutableResultTouch setObject:[NSNumber numberWithFloat:value/maxValue] forKey:@"force"];
            }else {
                [mutableResultTouch setObject:[NSNumber numberWithFloat:0.0] forKey:@"force"];
            }
        }
        
        [resultTouches addObject:mutableResultTouch];
    }
    
    [_component fireEvent:eventName params:@{@"changedTouches":resultTouches ?: @[]}];
}

- (void)recoveryPseudoStyles:(NSDictionary *)styles
{
    [_component recoveryPseudoStyles:styles];
}

- (void)touchResponse:(UIGestureRecognizer *)gesture
{
    
}

@end
