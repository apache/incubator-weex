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

#import <UIKit/UIKit.h>
#import <WeexSDK/WXComponent.h>
#import <WeexSDK/WXJSExceptionInfo.h>
#import <WeexSDK/WXResourceResponse.h>
#import <WeexSDK/WXResourceRequest.h>
#import <WeexSDK/WXBridgeProtocol.h>
#import <WeexSDK/WXApmForInstance.h>
#import <WeexSDK/WXComponentManager.h>

NS_ASSUME_NONNULL_BEGIN

extern NSString *const bundleUrlOptionKey;

@interface WXSDKInstance : NSObject

/**
 * Init instance and render it using iOS native views.
 * It is the same as initWithRenderType:@"platform"
 **/
- (instancetype)init;

/**
 * Init instance with custom render type.
 **/
- (instancetype)initWithRenderType:(NSString*)renderType;

/**
 * The render type. Default is "platform"
 **/
@property (nonatomic, strong, readonly) NSString* renderType;

/**
 * Returns YES when self.renderType != "platform"
 **/
@property (nonatomic, assign, readonly) BOOL isCustomRenderType;

/*
 * For weex containers in view controller(main containers), we may need to release render buffer
 * of custom render type page to save memory.
 */
@property (nonatomic, assign) BOOL isMainContainerStack;

/**
 * The viewControler which the weex bundle is rendered in.
 **/
@property (nonatomic, weak) UIViewController *viewController;

/**
 * The Native root container used to bear the view rendered by weex file. 
 * The root view is controlled by WXSDKInstance, so you can only get it, but not change it.
 **/
@property (nonatomic, strong) UIView *rootView;

/**
 * Component can freeze the rootview frame through the variable isRootViewFrozen
 * If Component want to freeze the rootview frame, set isRootViewFrozen YES, weex will not change the rootview frame when layout,or set NO.
 **/
@property (nonatomic, assign) BOOL isRootViewFrozen;

/**
 * Which indicates current instance needs to be validated or not to load,default value is false.
 **/
@property (nonatomic, assign) BOOL needValidate;

/**
 * Which indicates current instance use backup JS thread run,default value is false.
 **/
@property (nonatomic, assign) BOOL useBackupJsThread;

/**
 * The scriptURL of weex bundle.
 **/
@property (nonatomic, strong) NSURL *scriptURL;

/**
 * The parent instance.
 **/
@property (nonatomic, weak) WXSDKInstance *parentInstance;

/**
 * The node reference of parent instance.
 **/
@property (nonatomic, weak) NSString *parentNodeRef;

/**
 * The unique id to identify current weex instance.
 **/
@property (nonatomic, strong) NSString *instanceId;

/**
 * Which indicates current instance needs to be prerender or not,default value is false.
 **/
@property (nonatomic, assign) BOOL needPrerender;

/**
 * Custom info.
 **/
@property (nonatomic, strong) NSDictionary* containerInfo;

/**
 * Whether this instance is rendered or not. Please MUST not render an instance twice even if you have called destroyInstance.
 **/
@property (nonatomic, assign, readonly) BOOL isRendered;

/**
 * Get component manager of this instance. You can manipulate components then.
 **/
@property (nonatomic, readonly, strong) WXComponentManager *componentManager;

/**
 * The state of current instance.
 **/
typedef NS_ENUM(NSInteger, WXState) {//state.code
    WeexInstanceAppear = 100,
    WeexInstanceDisappear,
    WeexInstanceForeground,
    WeexInstanceBackground,
    WeexInstanceMemoryWarning,
    WeexInstanceBindChanged,
    WeexInstanceDestroy
};


typedef NS_ENUM(NSInteger, WXErrorType) {//error.domain
    TemplateErrorType = 1,
};

typedef NS_ENUM(NSInteger, WXErrorCode) {//error.code
    PlatformErrorCode = 1000,
    OSVersionErrorCode,
    AppVersionErrorCode,
    WeexSDKVersionErrorCode,
    DeviceModelErrorCode,
    FrameworkVersionErrorCode,
};

typedef enum : NSUInteger {
    WXScrollerComponentCreatedCallback,
    WXTabbarComponentCreatedCallback
} WXSDKInstanceCallbackType;


@property (nonatomic, assign) WXState state;

/**
 *  The callback triggered when the instance finishes creating the body.
 *
 *  @return A block that takes a UIView argument, which is the root view
 **/
@property (nonatomic, copy) void (^onCreate)(UIView *);

/**
 *  The callback triggered when the root container's frame has changed.
 *
 *  @return A block that takes a UIView argument, which is the root view
 **/
@property (nonatomic, copy) void (^onLayoutChange)(UIView *);

/**
 *  The callback triggered when the instance finishes rendering.
 *
 *  @return A block that takes a UIView argument, which is the root view
 **/
@property (nonatomic, copy) void (^renderFinish)(UIView *);

/**
 *  The callback triggered when the instance finishes refreshing weex view.
 *
 *  @return A block that takes a UIView argument, which is the root view
 **/
@property (nonatomic, copy) void (^refreshFinish)(UIView *);

/**
 * bundleType is the DSL type
 */
@property (nonatomic, strong) NSString * bundleType;

/**
 *  Which decide whether to use data render,default value is false
 */
@property (nonatomic, assign, readonly) BOOL dataRender;

/**
 *  Which decide whether to use binary code render, default value is false
 */
@property (nonatomic, assign, readonly) BOOL wlasmRender;
    
/**
 *  The callback triggered when the instance fails to render.
 *
 *  @return A block that takes a NSError argument, which is the error occured
 **/
@property (nonatomic, copy) void (^onFailed)(NSError *error);

/**
 *
 *  @return instance runtime JavaScript.
 *  you must call it in brige thread. Learn more at WXPerformBlockOnBridgeThread
 **/
- (id<WXBridgeProtocol>)instanceJavaScriptContext;

/**
 *  The callback triggered when js occurs runtime error while executing.
 *
 *  @return A block that takes a WXJSExceptionInfo argument, which is the exception info
 **/
@property (nonatomic, copy) void (^onJSRuntimeException)(WXJSExceptionInfo * jsException);

/**
 *  The callback triggered when the instacne executes scrolling .
 *
 *  @return A block that takes a CGPoint argument, which is content offset of the scroller
 **/
@property (nonatomic, copy) void (^onScroll)(CGPoint contentOffset);

/**
 *  The callback of the instance
 *
 *  When the callbackType is WXScrollerComponentCreatedCallback, the result type is WXScrollerComponent.
 *
 *  @return A block that takes instance, callbackType and a result.
 **/
@property (nonatomic, copy) void (^instanceCallback)(WXSDKInstance* instance, WXSDKInstanceCallbackType callbackType, id result);

/**
 * the callback to be run repeatedly while the instance is rendering.
 *
 * @return A block that takes a CGRect argument, which is the rect rendered
 **/
@property (nonatomic, copy) void (^onRenderProgress)(CGRect renderRect);

/**
 * The callback triggered when the bundleJS request finished in the renderWithURL.
 * @return A block that takes response which the server response,request which send to server,data which the server returned and an error
 */
@property (nonatomic, copy) void(^onJSDownloadedFinish)(WXResourceResponse *response,WXResourceRequest *request,NSData* _Nullable data, NSError* _Nullable error);

/**
 * The callback triggered when the bundleJS request finished in the renderWithURL. If the callback returns YES, the render process will terminate.
 * @return A block that takes response which the server response,request which send to server,data which the server returned and an error
 */
@property (nonatomic, copy) BOOL (^onRenderTerminateWhenJSDownloadedFinish)(WXResourceResponse *response,WXResourceRequest *request,NSData* _Nullable data, NSError* _Nullable error);

@property(nonatomic,strong) NSDictionary* continerInfo;

/**
 *  the frame of current instance.
 **/
@property (nonatomic, assign) CGRect frame;

/**
 *  the info stored by user.
 */
@property (atomic, strong) NSMutableDictionary *userInfo;

/**
 *  scale factor from css unit to device pixel.
 */
@property (nonatomic, assign, readonly) CGFloat pixelScaleFactor;

/**
 * track component render
 */
@property (nonatomic, assign)BOOL trackComponent;
/**
 * Renders weex view with bundle url.
 *
 * @param url The url of bundle rendered to a weex view.
 **/
- (void)renderWithURL:(NSURL *)url;

/**
 * Renders weex view with bundle url and some others.
 *
 * @param url The url of bundle rendered to a weex view.
 *
 * @param options The params passed by user
 *
 * @param data The data the bundle needs when rendered.  Defalut is nil.
 **/
- (void)renderWithURL:(NSURL *)url options:(NSDictionary * _Nullable)options data:(id _Nullable)data;

///**
// * Renders weex view with resource request.
// *
// * @param request The resource request specifying the URL to render with.
// *
// * @param options The params passed by user.
// *
// * @param data The data the bundle needs when rendered.  Defalut is nil.
// **/
//- (void)renderWithRequest:(WXResourceRequest *)request options:(NSDictionary *)options data:(id)data;

/**
 * Renders weex view with source string of bundle and some others.
 *
 * @param options The params passed by user.
 *
 * @param data The data the bundle needs when rendered. Defalut is nil.
 **/
- (void)renderView:(id)source options:(NSDictionary * _Nullable)options data:(id _Nullable)data;

/**
 * Reload the js bundle from the current URL and rerender.
 *
 * @param forcedReload when this parameter is true, the js bundle will always be reloaded from the server. If it is false, the instance may reload the js bundle from its cache. Default is false.
 *
 **/
- (void)reload:(BOOL)forcedReload;

/**
 * Refreshes current instance components' layout after setting custom view port/device width.
 **/
- (void)reloadLayout;

/**
 * Refreshes current instance with data.
 *
 * @param data The data the bundle needs when rendered.
 **/
- (void)refreshInstance:(id)data;

/**
 * Destroys current instance. An instance destroyed should not be used for rendering again, please create another instance.
 **/
- (void)destroyInstance;

/**
 * Trigger full GC, for dev and debug only.
 **/
- (void)forceGarbageCollection;

/**
 * get module instance by class
 */
- (id)moduleForClass:(Class)moduleClass;

/**
 * get Component instance by ref, must be called on component thread by calling WXPerformBlockOnComponentThread
 */
- (WXComponent *)componentForRef:(NSString *)ref;

/**
 * Number of components created, must be called on component thread by calling WXPerformBlockOnComponentThread
 */
- (NSUInteger)numberOfComponents;

/**
 * Enumerate components using breadth-first search algorithm,
 must be called on component thread by calling WXPerformBlockOnComponentThread
 */
- (void)enumerateComponentsUsingBlock:(void (^)(WXComponent *component, BOOL *stop))block;

/**
 * check whether the module eventName is registered
 */
- (BOOL)checkModuleEventRegistered:(NSString*)event moduleClassName:(NSString*)moduleClassName;

/**
 * fire module event;
 * @param module which module you fire event to
 * @param eventName the event name
 * @param params event params
 */
- (void)fireModuleEvent:(Class)module eventName:(NSString *)eventName params:(NSDictionary* _Nullable)params;

/**
 * fire global event
 */
- (void)fireGlobalEvent:(NSString *)eventName params:(NSDictionary * _Nullable)params;

/**
 * complete url based with bundle url
 */
- (NSURL *)completeURL:(NSString *)url;

/**
 * jsbundle str ,may be nil (weak)
 */
- (NSString* _Nullable) bundleTemplate;

/**
 * application performance statistics
 */
@property (nonatomic, strong) NSString *bizType;
@property (nonatomic, strong) NSString *pageName;
@property (nonatomic, weak) id pageObject;

//Deprecated, use @WXApmForInstance
@property (nonatomic, strong) NSMutableDictionary *performanceDict;
@property (nonatomic, strong) WXApmForInstance* apmInstance;

@property (nonatomic, assign) BOOL appearState;

/*
 * For custom render page to release/restore OpenGL resources, etc.
 */
- (void)willAppear;
- (void)didDisappear;

/**
 * Raw css styles are dropped after applied to layout nodes in WeexCore.
 * If a page needs hot refresh(without recreating instance and reload js) after screen orientation changes or
 * after setting custom view-port-width/screen-width/, you need to call setPageNeedsRawCssStyles to store all css styles
 * internally for later use. Or you can use MetaModule's setPageArguments method and provide "reserveCssStyles" as "true" before rendering the page.
 */
- (void)setPageKeepRawCssStyles;
- (void)isKeepingRawCssStyles:(void(^)(BOOL))callback;

/**
 * Set additional argument value for WeexCore
 */
- (void)setPageArgument:(NSString*)key value:(NSString*)value;

/**
 * Set specific required page width and height to prevent this page using global values.
 */
- (void)setPageRequiredWidth:(CGFloat)width height:(CGFloat)height;

/**
 * Set specific required view port width prevent this page using global value (750px).
 */
- (void)setViewportWidth:(CGFloat)width;

/**
 * @abstract Get information about the last rendered page.
 Useful fot debugging and fixing online bugs.
 */
+ (NSDictionary*)lastPageInfo;

/** 
 * Deprecated 
 */
@property (nonatomic, strong) NSDictionary *properties DEPRECATED_MSG_ATTRIBUTE();
@property (nonatomic, assign) NSTimeInterval networkTime DEPRECATED_MSG_ATTRIBUTE();
@property (nonatomic, copy) void (^updateFinish)(UIView *);

@end

@interface WXSDKInstance (Deprecated)

- (void)finishPerformance DEPRECATED_MSG_ATTRIBUTE();
- (void)reloadData:(id)data  DEPRECATED_MSG_ATTRIBUTE("Use refreshInstance: method instead.");
- (void)creatFinish DEPRECATED_MSG_ATTRIBUTE();

@end

NS_ASSUME_NONNULL_END
