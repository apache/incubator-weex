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

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class WXBridgeMethod;
@class WXSDKInstance;
@class WXComponent;

#ifdef __cplusplus
extern "C" {
#endif
    
void WXPerformBlockOnComponentThread(void (^block)(void));
void WXPerformBlockSyncOnComponentThread(void (^block)(void));
    
#ifdef __cplusplus
}
#endif

@interface WXComponentManager : NSObject

@property (nonatomic, readonly, weak) WXSDKInstance *weexInstance;
@property (nonatomic, readonly, assign) BOOL isValid;

/**
 * @abstract initialize with weex instance
 **/
- (instancetype)initWithWeexInstance:(WXSDKInstance *)weexInstance;

/**
 * @abstract return the component thread
 **/
+ (NSThread *)componentThread;

/**
 * @abstract starting component tasks
 **/
- (void)startComponentTasks;

/**
 * @abstract tell the component manager that instance root view's frame has been changed
 **/
- (void)rootViewFrameDidChange:(CGRect)frame;

///--------------------------------------
/// @name Component Tree Building
///--------------------------------------

/**
 * @abstract create root component
 **/
- (void)createBody:(NSString*)ref
              type:(NSString*)type
            styles:(NSDictionary*)styles
        attributes:(NSDictionary*)attributes
            events:(NSArray*)events
      renderObject:(void*)renderObject;

/**
 * @abstract add a component to its parent
 **/
- (void)addComponent:(NSString*)ref
                type:(NSString*)type
           parentRef:(NSString*)parentRef
              styles:(NSDictionary*)styles
          attributes:(NSDictionary*)attributes
              events:(NSArray*)events
               index:(NSInteger)index
        renderObject:(void*)renderObject;

/**
 * @abstract move component
 **/
- (void)moveComponent:(NSString *)ref toSuper:(NSString *)superRef atIndex:(NSInteger)index;

/**
 * @abstract remove component
 **/
- (void)removeComponent:(NSString *)ref;

/**
 * @abstract notify that a component tree is built and trigger layout,
    may be called several times rendering a page.
 **/
- (void)appendTreeCreateFinish:(NSString*)ref;

/**
 * @abstract return component for specific ref, must be called on component thread by calling WXPerformBlockOnComponentThread
 */
- (WXComponent *)componentForRef:(NSString *)ref;

/**
 * @abstract return root component
 */
- (WXComponent *)componentForRoot;

/**
 * @abstract number of components created, must be called on component thread by calling WXPerformBlockOnComponentThread
 */
- (NSUInteger)numberOfComponents;

/**
 * @abstract add an existing component to references look-up map
 */
- (void)addComponent:(WXComponent *)component toIndexDictForRef:(NSString *)ref;

/**
 * @abstract remove an existing component to references look-up map
 */
- (void)removeComponentForRef:(NSString *)ref;

///--------------------------------------
/// @name Updating
///--------------------------------------

/**
 * @abstract update styles
 **/
- (void)updateStyles:(NSDictionary *)styles forComponent:(NSString *)ref;

/**
 * @abstract update pseudo class styles
 **/
- (void)updatePseudoClassStyles:(NSDictionary *)styles forComponent:(NSString *)ref;

/**
 * @abstract update attributes
 **/
- (void)updateAttributes:(NSDictionary *)attributes forComponent:(NSString *)ref;

/**
 * @abstract quick check that if a component has non transition properties
 **/
- (BOOL)isTransitionNoneOfComponent:(NSString*)ref; // for quick access

/**
 * @abstract check if component with @ref has any style in @styles which is animated
 **/
- (BOOL)hasTransitionPropertyInStyles:(NSDictionary*)styles forComponent:(NSString*)ref;

/**
 * @abstract layout a component with frame output by weex core layout engine
 **/
- (void)layoutComponent:(WXComponent*)component frame:(CGRect)frame isRTL:(BOOL)isRTL innerMainSize:(CGFloat)innerMainSize;

/**
 * @abstract layout a component on platform side
 **/
- (void)layoutComponent:(WXComponent*)component;

/**
 * @abstract add event
 **/
- (void)addEvent:(NSString *)event toComponent:(NSString *)ref;

/**
 * @abstract remove event
 **/
- (void)removeEvent:(NSString *)event fromComponent:(NSString *)ref;

/**
 * @abstract scroll to specific component
 **/
- (void)scrollToComponent:(NSString *)ref options:(NSDictionary *)options;

///--------------------------------------
/// @name Life Cycle
///--------------------------------------

/**
 * @abstract called when all doms are created
 **/
- (void)createFinish;

/**
 * @abstract called when all doms are refreshed
 **/
- (void)refreshFinish;

/**
 * @abstract called when all doms are updated
 **/
- (void)updateFinish;

/**
 * @abstract called when all doms are created and layout finished
 **/
- (void)renderFinish;

/**
 * @abstract called when render failed
 **/
- (void)renderFailed:(NSError *)error;

/**
 * @abstract unload
 **/
- (void)unload;

/**
 * @abstract invalidate component management, this can be called on main thread.
 **/
- (void)invalidate;

///--------------------------------------
/// @name Fixed
///--------------------------------------

/**
 *  @abstract add a component which has a fixed position
 *
 *  @param fixComponent the fixed component to add
 */
- (void)addFixedComponent:(WXComponent *)fixComponent;

/**
 *  @abstract remove a component which has a fixed position
 *
 *  @param fixComponent the fixed component to remove
 */
- (void)removeFixedComponent:(WXComponent *)fixComponent;

/**
 * @abstract add a task to UI thread
 **/
- (void)_addUITask:(void (^)(void))block;

/**
 * @abstract execute a task to UI thread
 **/
- (void)executePrerenderUITask:(NSString *)url;

/**
 * @param styles a NSDictionary value, styles which will resolve
 * @param component a WXComponent value, the target which you want to resolve
 * @param isUpdateStyles a BOOL value, YES will udpate the component style property, NO will notifiy the lifeStyle of compoenent to handle, default value is NO.
 * @abstract handleStyle will be add to a queue to be executed every frame, but handleStyleOnMainThread will switch to main thread and execute imediately, you can call this for your execution time sequence.
 */
- (void)handleStyleOnMainThread:(NSDictionary*)styles forComponent:(WXComponent *)component isUpdateStyles:(BOOL)isUpdateStyles;

///--------------------------------------
/// @name Enumerating
///--------------------------------------

/**
 * Enumerate components using breadth-first search algorithm,
 must be called on component thread by calling WXPerformBlockOnComponentThread
 */
- (void)enumerateComponentsUsingBlock:(void (^)(WXComponent *, BOOL *stop))block;

#pragma mark batch mark

/**
 a start native batch tag for a group of UI operations, company with performBatchEnd
 @see performBatchEnd
 */
- (void)performBatchBegin;

/**
 an end native batch tag for a group of UI operations, company with performBatchBegin
 @see performBatchBegin
 */
- (void)performBatchEnd;

@end

NS_ASSUME_NONNULL_END
