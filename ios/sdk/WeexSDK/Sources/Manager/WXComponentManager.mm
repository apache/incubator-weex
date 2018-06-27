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

#import "WXComponentManager.h"
#import "WXComponent.h"
#import "WXComponent_internal.h"
#import "WXComponent+DataBinding.h"
#import "WXComponentFactory.h"
#import "WXDefine.h"
#import "NSArray+Weex.h"
#import "WXSDKInstance.h"
#import "WXAssert.h"
#import "WXUtility.h"
#import "WXMonitor.h"
#import "WXScrollerProtocol.h"
#import "WXSDKManager.h"
#import "WXSDKError.h"
#import "WXInvocationConfig.h"
#import "WXHandlerFactory.h"
#import "WXValidateProtocol.h"
#import "WXPrerenderManager.h"
#import "WXTracingManager.h"
#import "WXSDKInstance_performance.h"
#import "WXRootView.h"
#import "WXComponent+Layout.h"



static NSThread *WXComponentThread;

#define WXAssertComponentExist(component)  WXAssert(component, @"component not exists")


@implementation WXComponentManager
{
    __weak WXSDKInstance *_weexInstance;
    BOOL _isValid;
    
    BOOL _stopRunning;
    NSUInteger _noTaskTickCount;
    
    // access only on component thread
    NSMapTable<NSString *, WXComponent *> *_indexDict;
    NSMutableArray<dispatch_block_t> *_uiTaskQueue;
    NSMutableDictionary *_uiPrerenderTaskQueue;

    WXComponent *_rootComponent;
    NSMutableArray *_fixedComponents;
    WeexCore::WXCoreLayoutNode* _rootFlexCSSNode;
    CADisplayLink *_displayLink;
    pthread_mutex_t _propertyMutex;
    pthread_mutexattr_t _propertMutexAttr;
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

- (instancetype)initWithWeexInstance:(id)weexInstance
{
    if (self = [self init]) {
        _weexInstance = weexInstance;
        
        _indexDict = [NSMapTable strongToWeakObjectsMapTable];
        _fixedComponents = [NSMutableArray wx_mutableArrayUsingWeakReferences];
        _uiTaskQueue = [NSMutableArray array];
        _isValid = YES;
        pthread_mutexattr_init(&_propertMutexAttr);
        pthread_mutexattr_settype(&_propertMutexAttr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&_propertyMutex, &_propertMutexAttr);
        [self _startDisplayLink];
    }
    
    return self;
}

- (void)dealloc
{
    if(_rootFlexCSSNode){
        if ([[NSThread currentThread].name isEqualToString:WX_COMPONENT_THREAD_NAME]) {
            delete _rootFlexCSSNode;
        }else{
            [WXComponent recycleNodeOnComponentThread:_rootFlexCSSNode gabRef:@"_root_p"];
        }
        _rootFlexCSSNode=nullptr;
    }
    [NSMutableArray wx_releaseArray:_fixedComponents];
    pthread_mutex_destroy(&_propertyMutex);
    pthread_mutexattr_destroy(&_propertMutexAttr);
}

#pragma mark Thread Management

+ (NSThread *)componentThread
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        WXComponentThread = [[NSThread alloc] initWithTarget:[self sharedManager] selector:@selector(_runLoopThread) object:nil];
        [WXComponentThread setName:WX_COMPONENT_THREAD_NAME];
        if(WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"8.0")) {
            [WXComponentThread setQualityOfService:[[NSThread mainThread] qualityOfService]];
        } else {
            [WXComponentThread setThreadPriority:[[NSThread mainThread] threadPriority]];
        }
        
        [WXComponentThread start];
    });
    
    return WXComponentThread;
}

- (void)_runLoopThread
{
    [[NSRunLoop currentRunLoop] addPort:[NSMachPort port] forMode:NSDefaultRunLoopMode];
    
    while (!_stopRunning) {
        @autoreleasepool {
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
        }
    }
}

+ (void)_performBlockOnComponentThread:(void (^)(void))block
{
    if([NSThread currentThread] == [self componentThread]){
        block();
    } else {
        [self performSelector:@selector(_performBlockOnComponentThread:)
                     onThread:WXComponentThread
                   withObject:[block copy]
                waitUntilDone:NO];
    }
}

+ (void)_performBlockSyncOnComponentThread:(void (^)(void))block
{
    if([NSThread currentThread] == [self componentThread]){
        block();
    } else {
        [self performSelector:@selector(_performBlockOnComponentThread:)
                     onThread:WXComponentThread
                   withObject:[block copy]
                waitUntilDone:YES];
    }
}

- (void)startComponentTasks
{
    [self _awakeDisplayLink];
}

- (void)rootViewFrameDidChange:(CGRect)frame
{
    WXAssertComponentThread();
        if (_rootFlexCSSNode) {
            [self _applyRootFrame:frame];
            if (!_rootComponent.styles[@"width"]) {
                _rootComponent.flexCssNode->setStyleWidth(frame.size.width ?:FlexUndefined,NO);
            }
            if (!_rootComponent.styles[@"height"]) {
                _rootComponent.flexCssNode->setStyleHeight(frame.size.height ?:FlexUndefined);
            }
        }
    [_rootComponent setNeedsLayout];
    [self startComponentTasks];
}

- (void)_applyRootFrame:(CGRect)rootFrame{
    _rootFlexCSSNode->setStylePosition(WeexCore::kPositionEdgeLeft, self.weexInstance.frame.origin.x);
    _rootFlexCSSNode->setStylePosition(WeexCore::kPositionEdgeTop, self.weexInstance.frame.origin.y);
    _rootFlexCSSNode->setStyleWidth(self.weexInstance.frame.size.width ?: FlexUndefined,NO);
    _rootFlexCSSNode->setStyleHeight(self.weexInstance.frame.size.height ?: FlexUndefined);
}

- (void)_addUITask:(void (^)(void))block
{
    if(!_uiPrerenderTaskQueue){
        _uiPrerenderTaskQueue = [NSMutableDictionary new];
    }
    if(self.weexInstance.needPrerender){
        NSMutableArray<dispatch_block_t> *tasks  = [_uiPrerenderTaskQueue objectForKey:[WXPrerenderManager getTaskKeyFromUrl:self.weexInstance.scriptURL.absoluteString]];
        if(!tasks){
            tasks = [NSMutableArray new];
        }
        [tasks addObject:block];
        [_uiPrerenderTaskQueue setObject:tasks forKey:[WXPrerenderManager getTaskKeyFromUrl:self.weexInstance.scriptURL.absoluteString]];
    }else{
        [_uiTaskQueue addObject:block];
    }
}

- (void)excutePrerenderUITask:(NSString *)url
{
    NSMutableArray *tasks  = [_uiPrerenderTaskQueue objectForKey:[WXPrerenderManager getTaskKeyFromUrl:self.weexInstance.scriptURL.absoluteString]];
    for (id block in tasks) {
        [_uiTaskQueue addObject:block];
    }
    tasks = [NSMutableArray new];
    [_uiPrerenderTaskQueue setObject:tasks forKey:[WXPrerenderManager getTaskKeyFromUrl:self.weexInstance.scriptURL.absoluteString]];
}

#pragma mark Component Tree Building

- (void)createRoot:(NSDictionary *)data
{
    WXAssertComponentThread();
    WXAssertParam(data);
    
    _rootComponent = [self _buildComponentForData:data supercomponent:nil];
        [self _initRootFlexCssNode];
        _rootFlexCSSNode->addChildAt(_rootComponent.flexCssNode, (uint32_t)[_fixedComponents count]);
    
    NSArray *subcomponentsData = [data valueForKey:@"children"];
    if (subcomponentsData) {
        BOOL appendTree = [_rootComponent.attributes[@"append"] isEqualToString:@"tree"];
        for(NSDictionary *subcomponentData in subcomponentsData){
            [self _recursivelyAddComponent:subcomponentData toSupercomponent:_rootComponent atIndex:-1 appendingInTree:appendTree];
        }
    }
    
    __weak typeof(self) weakSelf = self;
    WX_MONITOR_INSTANCE_PERF_END(WXFirstScreenJSFExecuteTime, self.weexInstance);
    [self _addUITask:^{
        [WXTracingManager startTracingWithInstanceId:weakSelf.weexInstance.instanceId ref:data[@"ref"] className:nil name:data[@"type"] phase:WXTracingBegin functionName:@"createBody" options:@{@"threadName":WXTUIThread}];
        __strong typeof(self) strongSelf = weakSelf;
        strongSelf.weexInstance.rootView.wx_component = strongSelf->_rootComponent;
        [strongSelf.weexInstance.rootView addSubview:strongSelf->_rootComponent.view];
        [WXTracingManager startTracingWithInstanceId:weakSelf.weexInstance.instanceId ref:data[@"ref"] className:nil name:data[@"type"] phase:WXTracingEnd functionName:@"createBody" options:@{@"threadName":WXTUIThread}];
    }];
    
    
}


- (void)addComponent:(NSDictionary *)componentData toSupercomponent:(NSString *)superRef atIndex:(NSInteger)index appendingInTree:(BOOL)appendingInTree
{
    WXAssertComponentThread();
    WXAssertParam(componentData);
    WXAssertParam(superRef);
    
    WXComponent *supercomponent = [_indexDict objectForKey:superRef];
    WXAssertComponentExist(supercomponent);
    
    if (!supercomponent) {
        WXLogWarning(@"addComponent,superRef from js never exit ! check JS action, supRef:%@",superRef);
        return;
    }
    
    [self _recursivelyAddComponent:componentData toSupercomponent:supercomponent atIndex:index appendingInTree:appendingInTree];
}

- (void)_recursivelyAddComponent:(NSDictionary *)componentData toSupercomponent:(WXComponent *)supercomponent atIndex:(NSInteger)index appendingInTree:(BOOL)appendingInTree
{
    WXComponent *component = [self _buildComponentForData:componentData supercomponent:supercomponent];
    if (!supercomponent.subcomponents) {
        index = 0;
    } else {
        index = (index == -1 ? supercomponent->_subcomponents.count : index);
    }
    
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> _recursivelyAddComponent : super:(%@,%@):[%f,%f] ,child:(%@,%@):[%f,%f],childClass:%@",
              supercomponent.type,
              supercomponent.ref,
              supercomponent.flexCssNode->getStyleWidth(),
              supercomponent.flexCssNode->getStyleHeight(),
              component.type,
              component.ref,
              component.flexCssNode->getStyleWidth(),
              component.flexCssNode->getStyleHeight()
              ,NSStringFromClass([component class])
              );
#endif //DEBUG

    
    [supercomponent _insertSubcomponent:component atIndex:index];
    // use _lazyCreateView to forbid component like cell's view creating
    if(supercomponent && component && supercomponent->_lazyCreateView) {
        component->_lazyCreateView = YES;
    }
    
    [self recordMaximumVirtualDom:component];
    
    if (!component->_isTemplate) {
        __weak typeof(self) weakSelf = self;
        BOOL isFSCreateFinish = [self weexInstance].isJSCreateFinish;
        [self _addUITask:^{
            [WXTracingManager startTracingWithInstanceId:weakSelf.weexInstance.instanceId ref:componentData[@"ref"] className:nil name:componentData[@"type"] phase:WXTracingBegin functionName:@"addElement" options:@{@"threadName":WXTUIThread}];
            [supercomponent insertSubview:component atIndex:index];
            [WXTracingManager startTracingWithInstanceId:weakSelf.weexInstance.instanceId ref:componentData[@"ref"] className:nil name:componentData[@"type"] phase:WXTracingEnd functionName:@"addElement" options:@{@"threadName":WXTUIThread}];
            [weakSelf onElementChange:isFSCreateFinish];
        }];
    }
    
    NSArray *subcomponentsData = [componentData valueForKey:@"children"];
    
    BOOL appendTree = !appendingInTree && [component.attributes[@"append"] isEqualToString:@"tree"];
    // if ancestor is appending tree, child should not be laid out again even it is appending tree.
    for(NSDictionary *subcomponentData in subcomponentsData){
        [self _recursivelyAddComponent:subcomponentData toSupercomponent:component atIndex:-1 appendingInTree:appendTree || appendingInTree];
    }
    
    [component _didInserted];
    
    if (appendTree) {
        // If appending tree，force layout in case of too much tasks piling up in syncQueue
        [self _layoutAndSyncUI];
    }
}

- (void)moveComponent:(NSString *)ref toSuper:(NSString *)superRef atIndex:(NSInteger)index
{
    WXAssertComponentThread();
    WXAssertParam(ref);
    WXAssertParam(superRef);
    
    WXComponent *component = [_indexDict objectForKey:ref];
    WXComponent *newSupercomponent = [_indexDict objectForKey:superRef];
    WXAssertComponentExist(component);
    WXAssertComponentExist(newSupercomponent);
    
    if (component.supercomponent == newSupercomponent && [newSupercomponent.subcomponents indexOfObject:component] < index) {
        // if the supercomponent moved to is the same as original supercomponent,
        // unify it into the index after removing.
        index--;
    }
    
    [component _moveToSupercomponent:newSupercomponent atIndex:index];
    __weak typeof(self) weakSelf = self;
    [self _addUITask:^{
        [WXTracingManager startTracingWithInstanceId:weakSelf.weexInstance.instanceId ref:ref className:nil name:nil phase:WXTracingBegin functionName:@"addElement" options:@{@"threadName":WXTUIThread}];
        [component moveToSuperview:newSupercomponent atIndex:index];
        [WXTracingManager startTracingWithInstanceId:weakSelf.weexInstance.instanceId ref:ref className:nil name:nil phase:WXTracingEnd functionName:@"addElement" options:@{@"threadName":WXTUIThread}];
    }];
}

- (void)removeComponent:(NSString *)ref
{
    WXAssertComponentThread();
    WXAssertParam(ref);
    
    WXComponent *component = [_indexDict objectForKey:ref];
    WXAssertComponentExist(component);
    
    if (!component) {
        WXLogWarning(@"removeComponent ref from js never exit ! check JS action, ref :%@",ref);
        return;
    }
    
    [component _removeFromSupercomponent];
    
    [_indexDict removeObjectForKey:ref];
    
    __weak typeof(self) weakSelf = self;
    BOOL isFSCreateFinish = [self weexInstance].isJSCreateFinish;
    [self _addUITask:^{
        [WXTracingManager startTracingWithInstanceId:weakSelf.weexInstance.instanceId ref:ref className:nil name:nil phase:WXTracingBegin functionName:@"removeElement" options:@{@"threadName":WXTUIThread}];
        if (component.supercomponent) {
            [component.supercomponent willRemoveSubview:component];
        }
        [component removeFromSuperview];
        [WXTracingManager startTracingWithInstanceId:weakSelf.weexInstance.instanceId ref:ref className:nil name:nil phase:WXTracingEnd functionName:@"removeElement" options:@{@"threadName":WXTUIThread}];
        [weakSelf onElementChange:isFSCreateFinish];
    }];
    
    [self _checkFixedSubcomponentToRemove:component];
    
}

- (void)onElementChange:(BOOL)isFSCreateFinish
{
    if (!isFSCreateFinish) {
        return;
    }
    
    UIView *root = [self weexInstance].rootView;
    BOOL hasEvent = TRUE;
    if (root && [root isKindOfClass:[WXRootView class]]) {
        WXRootView* wxRootView = (WXRootView *)root;
        hasEvent = [wxRootView isHasEvent];
    }
    if (hasEvent) {
        return;
    }
    double current = CACurrentMediaTime()*1000;
    
    double diff =  current - [self weexInstance].performance.jsCreateFinishTime;
    if (diff > 8000) {
        return;
    }
    [self weexInstance].performance.interactionTime = current - self.weexInstance.performance.renderTimeOrigin;
}

- (void)recordMaximumVirtualDom:(WXComponent*) component
{
    WXAssertComponentExist(component);
    if(!component){
        return;
    }
    int maxDeep =0;
    while (component) {
        maxDeep++;
        component = component.supercomponent;
    }
    if(maxDeep > [self weexInstance].performance.maxVdomDeep)
    {
        [self weexInstance].performance.maxVdomDeep = maxDeep;
    }
   
}

- (void)_checkFixedSubcomponentToRemove:(WXComponent *)component
{
    for (WXComponent *subcomponent in component.subcomponents) {
        if (subcomponent->_positionType == WXPositionTypeFixed) {
             [self _addUITask:^{
                 [subcomponent removeFromSuperview];
             }];
        }
        
        [self _checkFixedSubcomponentToRemove:subcomponent];
    }
}

- (WXComponent *)componentForRef:(NSString *)ref
{
    WXAssertComponentThread();
    
    return [_indexDict objectForKey:ref];
}

- (WXComponent *)componentForRoot
{
    return _rootComponent;
}

- (NSUInteger)numberOfComponents
{
    WXAssertComponentThread();
    
    return _indexDict.count;
}

- (WXComponent *)_buildComponentForData:(NSDictionary *)data supercomponent:(WXComponent *)supercomponent
{
    double buildSartTime = CACurrentMediaTime()*1000;
    NSString *ref = data[@"ref"];
    NSString *type = data[@"type"];
    NSDictionary *styles = data[@"style"];
    NSDictionary *attributes = data[@"attr"];
    NSArray *events = data[@"event"];
    
    if (self.weexInstance.needValidate) {
        id<WXValidateProtocol> validateHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXValidateProtocol)];
        if (validateHandler) {
            WXComponentValidateResult* validateResult;
            if ([validateHandler respondsToSelector:@selector(validateWithWXSDKInstance:component:supercomponent:)]) {
                validateResult = [validateHandler validateWithWXSDKInstance:self.weexInstance component:type supercomponent:supercomponent];
            }
            if (validateResult==nil || !validateResult.isSuccess) {
                type = validateResult.replacedComponent? validateResult.replacedComponent : @"div";
                WXLogError(@"%@",[validateResult.error.userInfo objectForKey:@"errorMsg"]);
            }
        }
    }
    
    WXComponentConfig *config = [WXComponentFactory configWithComponentName:type];
    BOOL isTemplate = [config.properties[@"isTemplate"] boolValue] || (supercomponent && supercomponent->_isTemplate);
    NSDictionary *bindingStyles;
    NSDictionary *bindingAttibutes;
    NSDictionary *bindingEvents;
    NSDictionary *bindingProps;
    if (isTemplate) {
        bindingProps = [self _extractBindingProps:&attributes];
        bindingStyles = [self _extractBindings:&styles];
        bindingAttibutes = [self _extractBindings:&attributes];
        bindingEvents = [self _extractBindingEvents:&events];
    }
    
    Class clazz = NSClassFromString(config.clazz);;
    WXComponent *component = [[clazz alloc] initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:self.weexInstance];
    if (isTemplate) {
        component->_isTemplate = YES;
        [component _storeBindingsWithProps:bindingProps styles:bindingStyles attributes:bindingAttibutes events:bindingEvents];
    }

    WXAssert(component, @"Component build failed for data:%@", data);
    
    [_indexDict setObject:component forKey:component.ref];
    [component readyToRender];// notify redyToRender event when init
    
    double diffTime = CACurrentMediaTime()*1000 - buildSartTime;
    if (!(self.weexInstance.isJSCreateFinish)) {
        self.weexInstance.performance.fsComponentCount++;
        self.weexInstance.performance.fsComponentCreateTime+=diffTime;
    }
    self.weexInstance.performance.componentCount++;
    self.weexInstance.performance.componentCreateTime+=diffTime;
    
    return component;
}

- (void)addComponent:(WXComponent *)component toIndexDictForRef:(NSString *)ref
{
    [_indexDict setObject:component forKey:ref];
}

- (NSDictionary *)_extractBindings:(NSDictionary **)attributesOrStylesPoint
{
    NSDictionary *attributesOrStyles = *attributesOrStylesPoint;
    if (!attributesOrStyles) {
        return nil;
    }
    
    NSMutableDictionary *newAttributesOrStyles = [attributesOrStyles mutableCopy];
    NSMutableDictionary *bindingAttributesOrStyles = [NSMutableDictionary dictionary];
    
    [attributesOrStyles enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull attributeOrStyleName, id  _Nonnull attributeOrStyle, BOOL * _Nonnull stop) {
        if ([WXBindingMatchIdentify isEqualToString:attributeOrStyleName] // match
            ||  [WXBindingRepeatIdentify isEqualToString:attributeOrStyleName] // repeat
            ||  [WXBindingOnceIdentify isEqualToString:attributeOrStyleName] // once
            ||([attributeOrStyle isKindOfClass:[NSDictionary class]] && attributeOrStyle[WXBindingIdentify])) {  // {"attributeOrStyleName": {"@binding":"bindingExpression"}
            bindingAttributesOrStyles[attributeOrStyleName] = attributeOrStyle;
            [newAttributesOrStyles removeObjectForKey:attributeOrStyleName];
        } else if ([attributeOrStyle isKindOfClass:[NSArray class]]) {
            // {"attributeOrStyleName":[..., "string", {"@binding":"bindingExpression"}, "string", {"@binding":"bindingExpression"}, ...]
            __block BOOL isBinding = NO;
            [attributeOrStyle enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
                if ([obj isKindOfClass:[NSDictionary class]] && obj[WXBindingIdentify]) {
                    isBinding = YES;
                    *stop = YES;
                }
            }];
            
            if (isBinding) {
                bindingAttributesOrStyles[attributeOrStyleName] = attributeOrStyle;
                [newAttributesOrStyles removeObjectForKey:attributeOrStyleName];
            }
        }
    }];
    
    *attributesOrStylesPoint = newAttributesOrStyles;
    
    return bindingAttributesOrStyles;
}

- (NSDictionary *)_extractBindingEvents:(NSArray **)eventsPoint
{
    NSArray *events = *eventsPoint;
    NSMutableArray *newEvents = [events mutableCopy];
    NSMutableDictionary *bindingEvents = [NSMutableDictionary dictionary];
    [events enumerateObjectsUsingBlock:^(id  _Nonnull event, NSUInteger idx, BOOL * _Nonnull stop) {
        if ([event isKindOfClass:[NSDictionary class]] && event[@"type"] && event[@"params"]) {
            NSString *eventName = event[@"type"];
            NSString *bindingParams = event[@"params"];
            bindingEvents[eventName] = bindingParams;
            newEvents[idx] = eventName;
        }
    }];
    
    *eventsPoint = newEvents;
    return bindingEvents;
}

- (NSDictionary *)_extractBindingProps:(NSDictionary **)attributesPoint
{
    NSDictionary *attributes = *attributesPoint;
    if (attributes[@"@componentProps"]) {
        NSMutableDictionary *newAttributes = [attributes mutableCopy];
        [newAttributes removeObjectForKey:@"@componentProps"];
        *attributesPoint = newAttributes;
        return attributes[@"@componentProps"];
    }
    
    return nil;
}

#pragma mark Reset
-(BOOL)isShouldReset:(id )value
{
    if([value isKindOfClass:[NSString class]]) {
        if(!value || [@"" isEqualToString:value]) {
            return YES;
        }
    }
    return NO;
}

-(void)filterStyles:(NSDictionary *)styles normalStyles:(NSMutableDictionary *)normalStyles resetStyles:(NSMutableArray *)resetStyles
{
    for (NSString *key in styles) {
        id value = [styles objectForKey:key];
        if([self isShouldReset:value]) {
            [resetStyles addObject:key];
        }else{
            [normalStyles setObject:styles[key] forKey:key];
        }
    }
}

- (void)updateStyles:(NSDictionary *)styles forComponent:(NSString *)ref
{
    [self handleStyles:styles forComponent:ref isUpdateStyles:YES];
}

- (void)updatePseudoClassStyles:(NSDictionary *)styles forComponent:(NSString *)ref
{
    [self handleStyles:styles forComponent:ref isUpdateStyles:NO];
}

- (void)handleStyleOnMainThread:(NSDictionary*)styles forComponent:(WXComponent *)component isUpdateStyles:(BOOL)isUpdateStyles
{
    WXAssertParam(styles);
    WXAssertParam(component);
    WXAssertMainThread();
    
    NSMutableDictionary *normalStyles = [NSMutableDictionary new];
    NSMutableArray *resetStyles = [NSMutableArray new];
    [self filterStyles:styles normalStyles:normalStyles resetStyles:resetStyles];
    [component _updateStylesOnMainThread:normalStyles resetStyles:resetStyles];
    [component readyToRender];
    
    WXPerformBlockOnComponentThread(^{
        [component _updateStylesOnComponentThread:normalStyles resetStyles:resetStyles isUpdateStyles:isUpdateStyles];
    });
}

- (void)handleStyles:(NSDictionary *)styles forComponent:(NSString *)ref isUpdateStyles:(BOOL)isUpdateStyles
{
    WXAssertParam(styles);
    WXAssertParam(ref);
    
    WXComponent *component = [_indexDict objectForKey:ref];
    WXAssertComponentExist(component);
    
    NSMutableDictionary *normalStyles = [NSMutableDictionary new];
    NSMutableArray *resetStyles = [NSMutableArray new];
    [self filterStyles:styles normalStyles:normalStyles resetStyles:resetStyles];
    [component _updateStylesOnComponentThread:normalStyles resetStyles:resetStyles isUpdateStyles:isUpdateStyles];
    [self _addUITask:^{
        [component _updateStylesOnMainThread:normalStyles resetStyles:resetStyles];
        [component readyToRender];
    }];
}

- (void)updateAttributes:(NSDictionary *)attributes forComponent:(NSString *)ref
{
    WXAssertParam(attributes);
    WXAssertParam(ref);
    
    WXComponent *component = [_indexDict objectForKey:ref];
    WXAssertComponentExist(component);
    
    [component _updateAttributesOnComponentThread:attributes];
    __weak typeof(self) weakSelf = self;
    [self _addUITask:^{
        [WXTracingManager startTracingWithInstanceId:weakSelf.weexInstance.instanceId ref:ref className:nil name:nil phase:WXTracingBegin functionName:@"updateAttrs" options:@{@"threadName":WXTUIThread}];
        [component _updateAttributesOnMainThread:attributes];
        [component readyToRender];
        [WXTracingManager startTracingWithInstanceId:weakSelf.weexInstance.instanceId ref:ref className:nil name:nil phase:WXTracingEnd functionName:@"updateAttrs" options:@{@"threadName":WXTUIThread}];
    }];
}

- (void)addEvent:(NSString *)eventName toComponent:(NSString *)ref
{
    WXAssertComponentThread();
    WXAssertParam(eventName);
    WXAssertParam(ref);
    
    WXComponent *component = [_indexDict objectForKey:ref];
    WXAssertComponentExist(component);
    
    [component _addEventOnComponentThread:eventName];
    
    [self _addUITask:^{
        [component _addEventOnMainThread:eventName];
    }];
}

- (void)removeEvent:(NSString *)eventName fromComponent:(NSString *)ref
{
    WXAssertComponentThread();
    WXAssertParam(eventName);
    WXAssertParam(ref);
    
    WXComponent *component = [_indexDict objectForKey:ref];
    WXAssertComponentExist(component);
    
    [component _removeEventOnComponentThread:eventName];
    
    [self _addUITask:^{
        [component _removeEventOnMainThread:eventName];
    }];
}

- (void)scrollToComponent:(NSString *)ref options:(NSDictionary *)options
{
    WXAssertComponentThread();
    WXAssertParam(ref);
    
    WXComponent *toComponent = [_indexDict objectForKey:ref];
    WXAssertComponentExist(toComponent);

    id<WXScrollerProtocol> scrollerComponent = toComponent.ancestorScroller;
    if (!scrollerComponent) {
        return;
    }

    CGFloat offset = [[options objectForKey:@"offset"] floatValue];
    BOOL animated = YES;
    if ([options objectForKey:@"animated"]) {
        animated = [[options objectForKey:@"animated"] boolValue];
    }
    
    [self _addUITask:^{
        [scrollerComponent scrollToComponent:toComponent withOffset:offset animated:animated];
    }];
}

#pragma mark Life Cycle

- (void)createFinish
{
    WXAssertComponentThread();
    
    WXSDKInstance *instance  = self.weexInstance;
    [self _addUITask:^{
        UIView *rootView = instance.rootView;
        
        WX_MONITOR_INSTANCE_PERF_END(WXPTFirstScreenRender, instance);
        WX_MONITOR_INSTANCE_PERF_END(WXPTAllRender, instance);
        WX_MONITOR_SUCCESS(WXMTJSBridge);
        WX_MONITOR_SUCCESS(WXMTNativeRender);
        
        if(instance.renderFinish){
            [WXTracingManager startTracingWithInstanceId:instance.instanceId ref:nil className:nil name:nil phase:WXTracingInstant functionName:WXTRenderFinish options:@{@"threadName":WXTUIThread}];
            instance.renderFinish(rootView);
        }
    }];
    [instance updatePerDicAfterCreateFinish];
}

- (void)updateFinish
{
    WXAssertComponentThread();
    
    WXSDKInstance *instance = self.weexInstance;
    WXComponent *root = [_indexDict objectForKey:WX_SDK_ROOT_REF];
    
    [self _addUITask:^{
        if(instance.updateFinish){
            instance.updateFinish(root.view);
        }
    }];
}

- (void)refreshFinish
{
    WXAssertComponentThread();
    
    WXSDKInstance *instance = self.weexInstance;
    WXComponent *root = [_indexDict objectForKey:WX_SDK_ROOT_REF];
    
    [self _addUITask:^{
        if(instance.refreshFinish){
            instance.refreshFinish(root.view);
        }
    }];
}

- (void)unload
{
    WXAssertComponentThread();
    [self invalidate];
    [self _stopDisplayLink];
    NSEnumerator *enumerator = [[_indexDict copy] objectEnumerator];
    dispatch_async(dispatch_get_main_queue(), ^{
        WXComponent *component;
        while ((component = [enumerator nextObject])) {
            [component _unloadViewWithReusing:NO];
        }
        _rootComponent = nil;
    });
    
    [_indexDict removeAllObjects];
    [_uiTaskQueue removeAllObjects];
}

- (void)invalidate
{
    _isValid = NO;
}

- (BOOL)isValid
{
    return _isValid;
}

#pragma mark Layout Batch

- (void)_startDisplayLink
{
    WXAssertComponentThread();
    
    if(!_displayLink){
        _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(_handleDisplayLink)];
        [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
}

- (void)_stopDisplayLink
{
    WXAssertComponentThread();
    
    if(_displayLink){
        [_displayLink invalidate];
        _displayLink = nil;
    }
}

- (void)_suspendDisplayLink
{
    WXAssertComponentThread();
    
    if(_displayLink && !_displayLink.paused) {
        _displayLink.paused = YES;
    }
}

- (void)_awakeDisplayLink
{
    WXAssertComponentThread();
    
    if(_displayLink && _displayLink.paused) {
        _displayLink.paused = NO;
    }
}

- (void)_handleDisplayLink
{
    WXAssertComponentThread();
    
    [self _layoutAndSyncUI];
}

- (void)_layoutAndSyncUI
{
    [self _layout];
    if(_uiTaskQueue.count > 0){
        [self _syncUITasks];
        _noTaskTickCount = 0;
    } else {
        // suspend display link when there's no task for 1 second, in order to save CPU time.
        _noTaskTickCount ++;
        if (_noTaskTickCount > 60) {
            [self _suspendDisplayLink];
        }
    }
}

- (void)_layout
{
    BOOL needsLayout = NO;

//    NSEnumerator *enumerator = [_indexDict objectEnumerator];
//    WXComponent *component;
//    while ((component = [enumerator nextObject])) {
//        if ([component needsLayout]) {
//            needsLayout = YES;
//            break;
//        }
//    }
    
    needsLayout = [_rootComponent needsLayout];

    if (!needsLayout) {
        return;
    }
#ifdef DEBUG
    WXLogDebug(@"flexLayout -> action__ calculateLayout root");
#endif
    
        std::pair<float, float> renderPageSize;
        renderPageSize.first = self.weexInstance.frame.size.width;
        renderPageSize.second = self.weexInstance.frame.size.height;
        _rootFlexCSSNode->calculateLayout(renderPageSize);
    NSMutableSet<WXComponent *> *dirtyComponents = [NSMutableSet set];
    [_rootComponent _calculateFrameWithSuperAbsolutePosition:CGPointZero gatherDirtyComponents:dirtyComponents];
    [self _calculateRootFrame];
  
    for (WXComponent *dirtyComponent in dirtyComponents) {
        [self _addUITask:^{
            [dirtyComponent _layoutDidFinish];
        }];
    }
}

- (void) _printFlexComonentFrame:(WXComponent *)component
{
#ifdef DEBUG
    WXLogDebug(@"node ref:%@, type:%@ , frame:%@",
          component.ref,
          component.type,
          NSStringFromCGRect(component.view.layer.frame)
          );
#endif
    
  
    
    for (WXComponent *childComponent in component.subcomponents) {
        [self _printFlexComonentFrame:childComponent];
    }

    
}

- (void)_syncUITasks
{
    NSArray<dispatch_block_t> *blocks = _uiTaskQueue;
    _uiTaskQueue = [NSMutableArray array];
    dispatch_async(dispatch_get_main_queue(), ^{
        for(dispatch_block_t block in blocks) {
            block();
        }
    });
}
- (void)_initRootFlexCssNode
{
    _rootFlexCSSNode = new WeexCore::WXCoreLayoutNode();
    [self _applyRootFrame:self.weexInstance.frame];
    _rootFlexCSSNode->setFlexWrap(WeexCore::kNoWrap);
    _rootFlexCSSNode->setContext((__bridge void *)(self));
}

- (void)_calculateRootFrame
{
        if(!_rootFlexCSSNode->hasNewLayout()){
            return;
        }
        _rootFlexCSSNode->setHasNewLayout(false);
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> root _calculateRootFrame");
#endif
        
        
        CGRect frame = CGRectMake(WXRoundPixelValue(_rootFlexCSSNode->getLayoutPositionLeft()),
                                  WXRoundPixelValue(_rootFlexCSSNode->getLayoutPositionTop()),
                                  WXRoundPixelValue(_rootFlexCSSNode->getLayoutWidth()),
                                  WXRoundPixelValue(_rootFlexCSSNode->getLayoutHeight()));
        WXPerformBlockOnMainThread(^{
            if(!self.weexInstance.isRootViewFrozen) {
                self.weexInstance.rootView.frame = frame;
            }
        });
        //   _rootFlexCSSNode->reset();
        
        //    resetNodeLayout(_rootFlexCSSNode);
    
   
}


#pragma mark Fixed 

- (void)addFixedComponent:(WXComponent *)fixComponent
{
    pthread_mutex_lock(&_propertyMutex);
    [_fixedComponents addObject:fixComponent];
    _rootFlexCSSNode->addChildAt(fixComponent.flexCssNode, (uint32_t)([_fixedComponents count]-1));
    pthread_mutex_unlock(&_propertyMutex);
}

- (void)removeFixedComponent:(WXComponent *)fixComponent
{
    pthread_mutex_lock(&_propertyMutex);
    [_fixedComponents removeObject:fixComponent];
    pthread_mutex_unlock(&_propertyMutex);
    [self removeFixFlexNode:fixComponent->_flexCssNode];
}

- (void)removeFixFlexNode:(WeexCore::WXCoreLayoutNode* )fixNode{
    if (nullptr == fixNode) {
        return;
    }
    if ([[NSThread currentThread].name isEqualToString:WX_COMPONENT_THREAD_NAME]) {
        _rootFlexCSSNode->removeChild(fixNode);
    }else{
        WXPerformBlockOnComponentThread(^{
            if (nullptr == fixNode) {
                return;
            }
            _rootFlexCSSNode->removeChild(fixNode);
        });
    }
}


@end

void WXPerformBlockOnComponentThread(void (^block)(void))
{
    [WXComponentManager _performBlockOnComponentThread:block];
}

void WXPerformBlockSyncOnComponentThread(void (^block)(void))
{
    [WXComponentManager _performBlockSyncOnComponentThread:block];
}
