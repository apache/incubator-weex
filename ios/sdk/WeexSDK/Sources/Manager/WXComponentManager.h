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

@class WXBridgeMethod;
@class WXSDKInstance;
@class WXComponent;

#ifdef __cplusplus
extern "C" {
#endif
    
void WXPerformBlockOnComponentThread(void (^block)());
void WXPerformBlockSyncOnComponentThread(void (^block)());
    
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
 * @abstract create root of component tree
 **/
- (void)createRoot:(NSDictionary *)data;

/**
 * @abstract add component
 **/
- (void)addComponent:(NSDictionary *)componentData toSupercomponent:(NSString *)superRef atIndex:(NSInteger)index appendingInTree:(BOOL)appendingInTree;

/**
 * @abstract remove component
 **/
- (void)removeComponent:(NSString *)ref;

/**
 * @abstract move component
 **/
- (void)moveComponent:(NSString *)ref toSuper:(NSString *)superRef atIndex:(NSInteger)index;

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

- (void)addComponent:(WXComponent *)component toIndexDictForRef:(NSString *)ref;

///--------------------------------------
/// @name Updating
///--------------------------------------

/**
 * @abstract update styles
 **/
- (void)updateStyles:(NSDictionary *)styles forComponent:(NSString *)ref;

///--------------------------------------
/// @name Updating pseudo class
///--------------------------------------

/**
 * @abstract update  pseudo class styles
 **/

- (void)updatePseudoClassStyles:(NSDictionary *)styles forComponent:(NSString *)ref;

/**
 * @abstract update attributes
 **/
- (void)updateAttributes:(NSDictionary *)attributes forComponent:(NSString *)ref;

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

- (void)_addUITask:(void (^)())block;

- (void)excutePrerenderUITask:(NSString *)url;

/**
 * @param styles a NSDictionary value, styles which will resolve
 * @param component a WXComponent value, the target which you want to resolve
 * @param isUpdateStyles a BOOL value, YES will udpate the component style property, NO will notifiy the lifeStyle of compoenent to handle, default value is NO.
 * @abstract handleStyle will be add to a queue to be executed every frame, but handleStyleOnMainThread will switch to main thread and execute imediately, you can call this for your execution time sequence.
 */
- (void)handleStyleOnMainThread:(NSDictionary*)styles forComponent:(WXComponent *)component isUpdateStyles:(BOOL)isUpdateStyles;
@end
