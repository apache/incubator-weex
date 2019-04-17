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
#import "WXSDKInstance_performance.h"
#import "WXRootView.h"
#import "WXComponent+Layout.h"
#import "WXCoreBridge.h"
#import "WXComponent_performance.h"
#import "WXAnalyzerCenter.h"
#import "WXDisplayLinkManager.h"

static NSThread *WXComponentThread;

/* Task scheduled on component thread and triggered every N display link ticks.
 If display link is stopped, the tasks are also suspended. */
@interface WXComponentThreadTickTask : NSObject

@property (nonatomic, assign) NSUInteger displayTickCount; // Triggered every N display link ticks
@property (nonatomic, copy) dispatch_block_t block;

@end

@implementation WXComponentThreadTickTask

+ (instancetype)taskWithBlock:(dispatch_block_t)block tickCount:(NSUInteger)tickCount
{
    WXComponentThreadTickTask* task = [[WXComponentThreadTickTask alloc] init];
    task.displayTickCount = tickCount;
    task.block = block;
    return task;
}

@end

#define WXAssertComponentExist(component)  WXAssert(component, @"component not exists")
#define MAX_DROP_FRAME_FOR_BATCH   200

@interface WXComponentManager () <WXDisplayLinkClient>
@end

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
    
    NSUInteger _displayTick;
    NSMutableArray<WXComponentThreadTickTask*> *_displayTaskQueue;
    
    // vdom depth and component count statistics
    NSUInteger _maxVdomDepth;
    NSUInteger _maxVdomDepthReported;
    NSUInteger _maxComponentCount;
    NSUInteger _maxComponentCountReported;

    WXComponent *_rootComponent;
    NSMutableArray *_fixedComponents;

    pthread_mutex_t _propertyMutex;
    pthread_mutexattr_t _propertMutexAttr;
    NSUInteger _syncUITaskCount;
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
        _syncUITaskCount = 0;
        _indexDict = [NSMapTable strongToWeakObjectsMapTable];
        _fixedComponents = [NSMutableArray wx_mutableArrayUsingWeakReferences];
        _uiTaskQueue = [NSMutableArray array];
        _displayTaskQueue = [NSMutableArray array];
        _isValid = YES;
        pthread_mutexattr_init(&_propertMutexAttr);
        pthread_mutexattr_settype(&_propertMutexAttr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&_propertyMutex, &_propertMutexAttr);
        
        WXPerformBlockOnComponentThread(^{
            // We should ensure that [WXDisplayLinkManager sharedInstance] is only invoked in component thread.
            [self _addVdomAndComponentCountTask];
            [self _startDisplayLink];
        });
    }
    
    return self;
}

- (instancetype)init
{
    if (self == [super init]) {
        _suspend = NO;
    }
    return self;
}

- (void)dealloc
{
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
    CGSize size = _weexInstance.frame.size;
    [WXCoreBridge setDefaultDimensionIntoRoot:_weexInstance.instanceId
                                        width:size.width height:size.height
                           isWidthWrapContent:size.width == 0.0f isHeightWrapContent:size.height == 0.0f];
    [_rootComponent setNeedsLayout];
    [self startComponentTasks];
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

- (void)executePrerenderUITask:(NSString *)url
{
    NSMutableArray *tasks  = [_uiPrerenderTaskQueue objectForKey:[WXPrerenderManager getTaskKeyFromUrl:self.weexInstance.scriptURL.absoluteString]];
    for (id block in tasks) {
        [_uiTaskQueue addObject:block];
    }
    tasks = [NSMutableArray new];
    [_uiPrerenderTaskQueue setObject:tasks forKey:[WXPrerenderManager getTaskKeyFromUrl:self.weexInstance.scriptURL.absoluteString]];
}

#pragma mark Component Tree Building

- (void)createBody:(NSString*)ref
              type:(NSString*)type
            styles:(NSDictionary*)styles
        attributes:(NSDictionary*)attributes
            events:(NSArray*)events
      renderObject:(void*)renderObject
{
    WXAssertComponentThread();
    WXAssertParam(ref);
    WXAssertParam(type);
    WXAssertParam(renderObject);
    WXAssert(_rootComponent == nil, @"Create body is invoked twice.");
    
    _rootComponent = [self _buildComponent:ref type:type supercomponent:nil styles:styles attributes:attributes events:events renderObject:renderObject];
    
    CGSize size = _weexInstance.frame.size;
    [WXCoreBridge setDefaultDimensionIntoRoot:_weexInstance.instanceId
                                        width:size.width height:size.height
                           isWidthWrapContent:size.width == 0.0f isHeightWrapContent:size.height == 0.0f];
    
    __weak typeof(self) weakSelf = self;
    WX_MONITOR_INSTANCE_PERF_END(WXFirstScreenJSFExecuteTime, self.weexInstance);
    [self _addUITask:^{
        __strong typeof(self) strongSelf = weakSelf;
        if (strongSelf == nil) {
            return;
        }
        
        strongSelf.weexInstance.rootView.wx_component = strongSelf->_rootComponent;
        [strongSelf.weexInstance.rootView addSubview:strongSelf->_rootComponent.view];
    }];
}

- (void)addComponent:(NSString*)ref
                type:(NSString*)type
           parentRef:(NSString*)parentRef
              styles:(NSDictionary*)styles
          attributes:(NSDictionary*)attributes
              events:(NSArray*)events
               index:(NSInteger)index
        renderObject:(void*)renderObject
{
    WXAssertComponentThread();
    WXAssertParam(ref);
    WXAssertParam(type);
    WXAssertParam(parentRef);
    WXAssertParam(renderObject);
    
    WXComponent *supercomponent = [_indexDict objectForKey:parentRef];
    WXAssertComponentExist(supercomponent);
    
    if (!supercomponent) {
        WXLogWarning(@"addComponent,superRef from js never exit ! check JS action, supRef:%@", parentRef);
        return;
    }
    if([WXAnalyzerCenter isInteractionLogOpen]){
         WXLogDebug(@"wxInteractionAnalyzer: [client][addElementStart]%@,%@,%@",supercomponent.weexInstance.instanceId,type,ref);
    }
    
    supercomponent.weexInstance.apmInstance.hasAddView = YES;
    
    WXComponent *component = [self _buildComponent:ref type:type supercomponent:supercomponent styles:styles attributes:attributes events:events renderObject:renderObject];
    if (!supercomponent.subcomponents) {
        index = 0;
    } else {
        index = (index == -1 ? supercomponent->_subcomponents.count : index);
    }
    if (supercomponent.ignoreInteraction) {
        component.ignoreInteraction = YES;
    }
    component.ignoreInteraction = [[component.attributes objectForKey:@"ignoreInteraction"] boolValue];
    
#ifdef DEBUG
    WXLogDebug(@"flexLayout -> _recursivelyAddComponent : super:(%@,%@):[%f,%f] ,child:(%@,%@):[%f,%f],childClass:%@",
               supercomponent.type,
               supercomponent.ref,
               supercomponent.flexCssNode->getStyleWidth(),
               supercomponent.flexCssNode->getStyleHeight(),
               component.type,
               component.ref,
               component.flexCssNode->getStyleWidth(),
               component.flexCssNode->getStyleHeight(),
               NSStringFromClass([component class])
               );
#endif //DEBUG
    
    BOOL inserted = [supercomponent _insertSubcomponent:component atIndex:index];
    if (!inserted) {
        // component is not inserted, ignore
        [component _setRenderObject:nullptr]; // unbind with RenderObject
        return;
    }
    
    // use _lazyCreateView to forbid component like cell's view creating
    if (supercomponent && component && supercomponent->_lazyCreateView) {
        component->_lazyCreateView = YES;
    }
    
    // update max vdom depth & component count, and will update apm data on next display task.
    [self recordMaximumVirtualDom:component];
    if ([_indexDict count] > _maxComponentCount) {
        _maxComponentCount = [_indexDict count];
    }
    
    if (!component->_isTemplate) {
        __weak typeof(self) weakSelf = self;
        [self _addUITask:^{
            __strong typeof(self) strongSelf = weakSelf;
            if (strongSelf == nil) {
                return;
            }
            
            [supercomponent insertSubview:component atIndex:index];
        }];
    }
    if([WXAnalyzerCenter isInteractionLogOpen]){
        WXLogDebug(@"wxInteractionAnalyzer: [client][addElementEnd]%@,%@,%@",supercomponent.weexInstance.instanceId,type,ref);
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
    
    [component _moveToSupercomponent:newSupercomponent atIndex:index];
    __weak typeof(self) weakSelf = self;
    [self _addUITask:^{
        __strong typeof(self) strongSelf = weakSelf;
        if (strongSelf == nil) {
            return;
        }
        
        [component moveToSuperview:newSupercomponent atIndex:index];
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
    
    [component _setRenderObject:nullptr]; // unbind with RenderObject
    [component _removeFromSupercomponent];
    
    [_indexDict removeObjectForKey:ref];
    
    // remove subcomponents of component from _indexDict and unbind them
    NSMutableArray* subcomponents = [[NSMutableArray alloc] init];
    [component _collectSubcomponents:subcomponents];
    for (WXComponent* c in subcomponents) {
        [c _setRenderObject:nullptr];
        [_indexDict removeObjectForKey:c.ref];
    }
    
    __weak typeof(self) weakSelf = self;
    [self _addUITask:^{
        __strong typeof(self) strongSelf = weakSelf;
        if (strongSelf == nil) {
            return;
        }
        
        if (component.supercomponent) {
            [component.supercomponent willRemoveSubview:component];
        }
        [component removeFromSuperview];
    }];
    
    [self _checkFixedSubcomponentToRemove:component];
}

- (void)appendTreeCreateFinish:(NSString*)ref
{
    WXAssertComponentThread();
    
    // If appending tree，force layout in case of too much tasks piling up in syncQueue
    [self _layoutAndSyncUI];
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
    
    if (maxDeep > [self weexInstance].performance.maxVdomDeep) {
        [self weexInstance].performance.maxVdomDeep = maxDeep;
    }
    
    if (maxDeep > _maxVdomDepth) {
        _maxVdomDepth = maxDeep;
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

- (WXComponent *)_buildComponent:(NSString *)ref
                            type:(NSString*)type
                  supercomponent:(WXComponent *)supercomponent
                          styles:(NSDictionary*)styles
                      attributes:(NSDictionary*)attributes
                          events:(NSArray*)events
                    renderObject:(void*)renderObject
{
    double buildStartTime = CACurrentMediaTime()*1000;
    
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
    NSDictionary *bindingStyles = nil;
    NSDictionary *bindingAttibutes = nil;
    NSDictionary *bindingEvents = nil;
    NSDictionary *bindingProps = nil;
    if (isTemplate) {
        bindingProps = [self _extractBindingProps:&attributes];
        bindingStyles = [self _extractBindings:&styles];
        bindingAttibutes = [self _extractBindings:&attributes];
        bindingEvents = [self _extractBindingEvents:&events];
    }
    
    Class clazz = NSClassFromString(config.clazz);
    WXComponent *component = [[clazz alloc] init];
    if (component) {
        if (renderObject) {
            [component _setRenderObject:renderObject];
        }
        component = [component initWithRef:ref type:type styles:styles attributes:attributes events:events weexInstance:self.weexInstance];
        if (isTemplate) {
            component->_isTemplate = YES;
            [component _storeBindingsWithProps:bindingProps styles:bindingStyles attributes:bindingAttibutes events:bindingEvents];
        }
    }
    
    WXAssert(component, @"Component build failed for ref:%@, type:%@", ref, type);
    
    [_indexDict setObject:component forKey:component.ref];
    [component readyToRender];// notify redyToRender event when init
    
    double diffTime = CACurrentMediaTime()*1000 - buildStartTime;
    [self.weexInstance.performance recordComponentCreatePerformance:diffTime forComponent:component];
    
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
    if (events == nil) {
        return nil;
    }
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
    if (attributes == nil) {
        return nil;
    }
    if (attributes[@"@componentProps"]) {
        NSMutableDictionary *newAttributes = [attributes mutableCopy];
        [newAttributes removeObjectForKey:@"@componentProps"];
        *attributesPoint = newAttributes;
        return attributes[@"@componentProps"];
    }
    
    return nil;
}

#pragma mark Reset

- (BOOL)isShouldReset:(id )value
{
    if([value isKindOfClass:[NSString class]]) {
        if(!value || [@"" isEqualToString:value]) {
            return YES;
        }
    }
    return NO;
}

- (void)filterStyles:(NSDictionary *)styles normalStyles:(NSMutableDictionary *)normalStyles resetStyles:(NSMutableArray *)resetStyles
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
    [component _updateAttributesOnComponentThread:attributes];
    __weak typeof(self) weakSelf = self;
    [self _addUITask:^{
        __strong typeof(self) strongSelf = weakSelf;
        if (strongSelf == nil) {
            return;
        }
        
        [component _updateAttributesOnMainThread:attributes];
        [component readyToRender];
    }];
}

- (BOOL)isTransitionNoneOfComponent:(NSString*)ref
{
    WXAssertComponentThread();
    
    WXComponent *component = [_indexDict objectForKey:ref];
    WXAssertComponentExist(component);
    
    return [component _isTransitionNone];
}

- (BOOL)hasTransitionPropertyInStyles:(NSDictionary*)styles forComponent:(NSString*)ref
{
    WXAssertComponentThread();
    
    WXComponent *component = [_indexDict objectForKey:ref];
    WXAssertComponentExist(component);
    
    return [component _hasTransitionPropertyInStyles:styles];
}

- (void)layoutComponent:(WXComponent*)component frame:(CGRect)frame isRTL:(BOOL)isRTL innerMainSize:(CGFloat)innerMainSize
{
    WXAssertComponentThread();
    WXAssertParam(component);
    
    [component _setIsLayoutRTL:isRTL];
    if (component == _rootComponent) {
        // Synchronize view frame with root component, especially for content wrap mode.
        WXPerformBlockOnMainThread(^{
            if (!self.weexInstance.isRootViewFrozen &&
                (!CGSizeEqualToSize(frame.size, self.weexInstance.frame.size) || !CGSizeEqualToSize(frame.size, self.weexInstance.rootView.frame.size))) {
                CGRect rect = self.weexInstance.rootView.frame; // no change of origin
                rect.size = frame.size;
                self.weexInstance.rootView.frame = rect;
            }
        });
    }
    
    if ([component _isCalculatedFrameChanged:frame]) {
        [component _assignCalculatedFrame:frame];
        [component _assignInnerContentMainSize:innerMainSize];
        [component _frameDidCalculated:YES];
        
        [self _addUITask:^{
            [component _layoutDidFinish];
        }];
    }
    else {
        CGFloat oldValue = [component _getInnerContentMainSize];
        if (oldValue >= 0 && oldValue != innerMainSize) {
            [component _assignCalculatedFrame:frame];
            [component _assignInnerContentMainSize:innerMainSize];
            [component _frameDidCalculated:YES];
            
            [self _addUITask:^{
                [component _layoutDidFinish];
            }];
        }
        else {
            [component _frameDidCalculated:NO];
        }
    }
}

- (void)layoutComponent:(WXComponent*)component
{
    WXAssertComponentThread();
    WXAssertParam(component);
    [component _layoutPlatform];
}

- (void)addEvent:(NSString *)event toComponent:(NSString *)ref
{
    WXAssertComponentThread();
    WXAssertParam(event);
    WXAssertParam(ref);
    
    WXComponent *component = [_indexDict objectForKey:ref];
    WXAssertComponentExist(component);
    
    [component _addEventOnComponentThread:event];
    
    [self _addUITask:^{
        [component _addEventOnMainThread:event];
    }];
}

- (void)removeEvent:(NSString *)event fromComponent:(NSString *)ref
{
    WXAssertComponentThread();
    WXAssertParam(event);
    WXAssertParam(ref);
    
    WXComponent *component = [_indexDict objectForKey:ref];
    WXAssertComponentExist(component);
    
    [component _removeEventOnComponentThread:event];
    
    [self _addUITask:^{
        [component _removeEventOnMainThread:event];
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
        
        WX_MONITOR_INSTANCE_PERF_END(WXPTFirstScreenRender, instance);
        WX_MONITOR_INSTANCE_PERF_END(WXPTAllRender, instance);
        WX_MONITOR_SUCCESS(WXMTJSBridge);
        WX_MONITOR_SUCCESS(WXMTNativeRender);
    }];
    [instance updatePerDicAfterCreateFinish];
}

- (void)updateFinish
{
    WXAssertComponentThread();
    
    WXSDKInstance *instance = self.weexInstance;
    WXComponent *root = [_indexDict objectForKey:WX_SDK_ROOT_REF];
    
    [self _addUITask:^{
        if (instance.updateFinish) {
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
        if (instance.refreshFinish) {
            instance.refreshFinish(root.view);
        }
    }];
}

- (void)renderFinish
{
    WXAssertComponentThread();
    
    WXSDKInstance *instance  = self.weexInstance;
    [self _addUITask:^{
        UIView *rootView = instance.rootView;
        [instance.performance onInstanceRenderSuccess:instance];
        if (instance.renderFinish) {
            instance.renderFinish(rootView);
        }
    }];
}

- (void)renderFailed:(NSError *)error {
    WXAssertComponentThread();

    WXSDKInstance *instance  = self.weexInstance;
    [self _addUITask:^{
        if (instance.onFailed) {
            instance.onFailed(error);
        }
    }];
}

- (void)unload
{
    WXAssertComponentThread();
    [self invalidate];
    [self _stopDisplayLink];
    
    // first, unbind with underneath RenderObjects
    {
        NSEnumerator* enumerator = [_indexDict objectEnumerator];
        WXComponent *component;
        while ((component = [enumerator nextObject])) {
            [component _setRenderObject:nullptr];
        }
    }
    
    // second, unload views and finally release components in UI thread
    {
        __block WXComponent* rootComponent = _rootComponent;
        NSEnumerator *enumerator = [[_indexDict copy] objectEnumerator];
        dispatch_async(dispatch_get_main_queue(), ^{
            WXComponent *component;
            while ((component = [enumerator nextObject])) {
                [component _unloadViewWithReusing:NO];
            }
            rootComponent = nil; // finally release all components
        });
    }
    
    // clear containers
    _rootComponent = nil;
    [_indexDict removeAllObjects];
    [_fixedComponents removeAllObjects];
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

#pragma mark Display link task

- (void)_addVdomAndComponentCountTask
{
    __weak WXComponentManager* wself = self;
    [_displayTaskQueue addObject:[WXComponentThreadTickTask taskWithBlock:^{
        __strong WXComponentManager* sself = wself;
        if (sself) {
            if (sself->_maxComponentCount != sself->_maxComponentCountReported) {
                [sself.weexInstance.apmInstance updateMaxStats:KEY_PAGE_STATS_MAX_COMPONENT_NUM curMaxValue:sself->_maxComponentCount];
                sself->_maxComponentCountReported = sself->_maxComponentCount;
            }
            
            if (sself->_maxVdomDepth != sself->_maxVdomDepthReported) {
                [sself.weexInstance.apmInstance updateMaxStats:KEY_PAGE_STATS_MAX_DEEP_DOM curMaxValue:sself->_maxVdomDepth];
                sself->_maxVdomDepthReported = sself->_maxVdomDepth;
            }
        }
    } tickCount:30 /* triggered about every 500ms */]];
}

- (void)_startDisplayLink
{
    WXAssertComponentThread();
    [[WXDisplayLinkManager sharedInstance] registerDisplayClient:self];
    _displayTick = 0;
}

- (void)_stopDisplayLink
{
    WXAssertComponentThread();
    [[WXDisplayLinkManager sharedInstance] unregisterDisplayClient:self];
}

- (void)_suspendDisplayLink
{
    WXAssertComponentThread();
    _suspend = YES;
    [self _executeDisplayTask:YES]; // on suspend, executes every task once
}

- (void)_awakeDisplayLink
{
    WXAssertComponentThread();
    _suspend = NO;
    _displayTick = 0;
}

- (void)_handleDisplayLink
{
    WXAssertComponentThread();
    
    [self _layoutAndSyncUI];
    
    if (!_suspend) {
        // execute tasks in _displayTaskQueue
        _displayTick ++;
        [self _executeDisplayTask:NO];
    }
}

- (void)_executeDisplayTask:(BOOL)onSuspend
{
    for (WXComponentThreadTickTask* task in _displayTaskQueue) {
        if (onSuspend || (_displayTick % task.displayTickCount == 0)) {
            if (task.block) {
                task.block();
            }
        }
    }
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
    [WXCoreBridge layoutPage:_weexInstance.instanceId forced:[_rootComponent needsLayout]];
}

- (void) _printFlexComponentFrame:(WXComponent *)component
{
#ifdef DEBUG
    WXLogDebug(@"node ref:%@, type:%@ , frame:%@",
          component.ref,
          component.type,
          NSStringFromCGRect(component.view.layer.frame)
          );
#endif
    
    for (WXComponent *childComponent in component.subcomponents) {
        [self _printFlexComponentFrame:childComponent];
    }
}

- (void)_syncUITasks
{
    NSInteger mismatchBeginIndex = _uiTaskQueue.count;
    for (NSInteger i = _uiTaskQueue.count - 1;i >= 0;i --) {
        if (_uiTaskQueue[i] == WXPerformUITaskBatchEndBlock) {
            _syncUITaskCount = 0;
            // clear when find the matches for end and begin tag
            break;
        }
        if (_uiTaskQueue[i] == WXPerformUITaskBatchBeginBlock) {
            mismatchBeginIndex = i;
            break;
        }
    }
    
    if (mismatchBeginIndex == _uiTaskQueue.count) {
        // here we get end tag or there are not begin and end directives
    } else {
        _syncUITaskCount ++;
        // we only find begin tag but missing end tag,
        if (_syncUITaskCount > (MAX_DROP_FRAME_FOR_BATCH)) {
            // when the wait times come to MAX_DROP_FRAME_FOR_BATCH, we will pop all the stashed operations for user experience.
            mismatchBeginIndex = _uiTaskQueue.count;
            _syncUITaskCount = 0;
        }
    }
    
    if (mismatchBeginIndex > 0) {
        NSArray<dispatch_block_t> *blocks = [_uiTaskQueue subarrayWithRange:NSMakeRange(0, mismatchBeginIndex)];
        [_uiTaskQueue removeObjectsInRange:NSMakeRange(0, mismatchBeginIndex)];
        if (blocks.count) {
            dispatch_async(dispatch_get_main_queue(), ^{
                for(dispatch_block_t block in blocks) {
                    block();
                }
            });
        }
    }
}

#pragma mark Fixed 

- (void)addFixedComponent:(WXComponent *)fixComponent
{
    pthread_mutex_lock(&_propertyMutex);
    [_fixedComponents addObject:fixComponent];
    pthread_mutex_unlock(&_propertyMutex);
}

- (void)removeFixedComponent:(WXComponent *)fixComponent
{
    pthread_mutex_lock(&_propertyMutex);
    [_fixedComponents removeObject:fixComponent];
    pthread_mutex_unlock(&_propertyMutex);
}

#pragma mark Enumerating

- (void)enumerateComponentsUsingBlock:(void (^)(WXComponent *, BOOL *stop))block
{
    if (block == nil || _rootComponent == nil) {
        return;
    }
    
    NSMutableArray* components = [[NSMutableArray alloc] init];
    [components addObject:_rootComponent];
    
    while ([components count] > 0) {
        BOOL stop = NO;
        
        NSArray* thisLevelComponents = [components copy];
        [components removeAllObjects];
        
        // enumerate thisLevelComponents and add next level components to components
        for (WXComponent* c in thisLevelComponents) {
            block(c, &stop);
            if (stop) {
                break;
            }
            
            for (WXComponent* nextLevelComponent in c->_subcomponents) {
                [components addObject:nextLevelComponent];
            }
        }
        
        if (stop) {
            break;
        }
    }
}

static void (^WXPerformUITaskBatchBeginBlock)(void) = ^ () {
#if DEBUG
    WXLogDebug(@"directive BatchBeginBlock");
#endif
};
static void (^WXPerformUITaskBatchEndBlock)(void) = ^ () {
#if DEBUG
    WXLogDebug(@"directive BatchEndBlock");
#endif
};

- (void)performBatchBegin
{
    [self _addUITask:WXPerformUITaskBatchBeginBlock];
}

- (void)performBatchEnd
{
    [self _addUITask:WXPerformUITaskBatchEndBlock];
}

- (void)handleDisplayLink {
    [self _handleDisplayLink];
}

@synthesize suspend=_suspend;


@end

void WXPerformBlockOnComponentThread(void (^block)(void))
{
    [WXComponentManager _performBlockOnComponentThread:block];
}

void WXPerformBlockSyncOnComponentThread(void (^block)(void))
{
    [WXComponentManager _performBlockSyncOnComponentThread:block];
}
