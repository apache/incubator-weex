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

#import "WXPrerenderManager.h"
#import "WXConfigCenterProtocol.h"
#import "WXModuleMethod.h"
#import "WXBridgeManager.h"
#import "WXSDKInstance_private.h"
#import "WXBridgeManager.h"
#import "WXSDKEngine.h"
#import "WXUtility.h"

static NSString *const MSG_PRERENDER_INTERNAL_ERROR = @"internal_error";
static NSString *const MSG_PRERENDER_SUCCESS = @"success";

@interface WXPrerenderTask:NSObject

@property (nonatomic, strong) WXSDKInstance *instance;
@property (nonatomic, strong) UIView *view;
@property (nonatomic, copy) NSString *parentInstanceId;
@property (nonatomic, copy) NSString *url;
@property (nonatomic, strong) NSMutableArray *moduleTasks;
@property (nonatomic, strong) NSError *error;
@property (nonatomic, assign) WXState state;
@property (nonatomic, strong) NSDate *beginDate;
@property (nonatomic) long long cacheTime;

@end
@implementation WXPrerenderTask
@end


@interface WXPrerenderManager()

@property (nonatomic, strong) NSMutableArray *cachedUrlList;
@property (nonatomic, strong) dispatch_queue_t queue;
@property (nonatomic, strong) NSMutableDictionary<NSString *, WXPrerenderTask*> *prerenderTasks;
@property (nonatomic) NSInteger maxCacheNumber;


@end

@implementation WXPrerenderManager

+ (instancetype) sharedInstance{
    
    static WXPrerenderManager *instance = nil;
    static dispatch_once_t once;
    
    dispatch_once(&once, ^{
        instance = [[WXPrerenderManager alloc] initPrivate];
    });
    
    return instance;
}

- (instancetype) initPrivate{
    self = [super init];
    if(self){
        self.cachedUrlList = [[NSMutableArray alloc] init];
        self.queue = dispatch_queue_create("Prerender", DISPATCH_QUEUE_SERIAL);
        self.prerenderTasks = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (void) dealloc{
    self.cachedUrlList = nil;
    self.prerenderTasks = nil;
}

+ (void) addTask:(NSString *)url instanceId:(NSString *)instanceId callback:(WXModuleCallback)callback{
    NSURL *newUrl = [NSURL URLWithString:url];
    if(!newUrl){
        if(callback){
            callback(@{@"url":url,@"message":MSG_PRERENDER_INTERNAL_ERROR,@"result":@"error"});
        }
        return;
    }
    
    WXPrerenderManager *manager = [WXPrerenderManager sharedInstance];
    __weak WXPrerenderManager *weakSelf = manager;
    dispatch_async(manager.queue, ^{
        [weakSelf prerender:newUrl WXInstance:instanceId callback:callback];
    });
}

-(BOOL)isSwitchOn
{
    BOOL switchOn = YES; // defautle YES
    id configCenter = [WXSDKEngine handlerForProtocol:@protocol(WXConfigCenterProtocol)];
    if ([configCenter respondsToSelector:@selector(configForKey:defaultValue:isDefault:)]) {
        id switchOnValue = [configCenter configForKey:@"weex_prerender_config.is_switch_on" defaultValue:@true isDefault:NULL];
        if(switchOnValue){
            switchOn = [switchOnValue boolValue];
        }
        if(!switchOn){
            return NO;
        }
    }
    return YES;
}

- (void) prerender:(NSURL *)url WXInstance:(NSString *)instanceId callback:(WXModuleCallback) callback{

    NSString *str = url.absoluteString;
    if(str.length==0){
        if(callback){
            callback(@{@"url":[url absoluteString],@"message":MSG_PRERENDER_INTERNAL_ERROR,@"result":@"error"});
        }
        return;
    }
    WXPrerenderTask *task = [WXPrerenderTask new];
    id configCenter = [WXSDKEngine handlerForProtocol:@protocol(WXConfigCenterProtocol)];
    if(![self isSwitchOn]){
        if(callback){
            callback(@{@"url":[url absoluteString],@"message":MSG_PRERENDER_INTERNAL_ERROR,@"result":@"error"});
        }
        return;
    }
    task.beginDate = [NSDate date];
    task.cacheTime = 300000;
    if ([configCenter respondsToSelector:@selector(configForKey:defaultValue:isDefault:)]) {
        long long time = [[configCenter configForKey:@"weex_prerender_config.cacheTime" defaultValue:@300000 isDefault:NULL] longLongValue];
        if(time){
            task.cacheTime = time;
        }
    }
    self.maxCacheNumber = 1;
    if ([configCenter respondsToSelector:@selector(configForKey:defaultValue:isDefault:)]) {
        NSInteger max = [[configCenter configForKey:@"weex_prerender_config.max_cache_num" defaultValue:@1 isDefault:NULL] integerValue];
        if(max){
            self.maxCacheNumber = max;
        }
    }
    WXSDKInstance *instance = [[WXSDKInstance alloc] init];
    instance.needPrerender = YES;
    task.instance = instance;
    task.parentInstanceId = instanceId;
    task.url = url.absoluteString;
    
    if(self.prerenderTasks && self.prerenderTasks.count<self.maxCacheNumber){
        [self.prerenderTasks setObject:task forKey:url.absoluteString];
        WXPerformBlockOnMainThread(^{
            [instance renderWithURL:url options:@{@"bundleUrl":url.absoluteString} data:nil];
            WXPrerenderManager *manager = [WXPrerenderManager sharedInstance];
            __weak typeof(self) weakSelf = manager;
            instance.onCreate = ^(UIView *view) {
                WXPrerenderTask *task = [weakSelf.prerenderTasks objectForKey:url.absoluteString];
                task.view = view;
                if(task){
                    [weakSelf.prerenderTasks setObject:task forKey:url.absoluteString];
                }
            };
            
            instance.onFailed = ^(NSError *error) {
                WXPrerenderTask *task  = [weakSelf.prerenderTasks objectForKey:url.absoluteString];
                task.error = error;
                if(task){
                    [weakSelf.prerenderTasks setObject:task forKey:url.absoluteString];
                }
            };
        });
        if(callback){
            callback(@{@"url":url.absoluteString,@"message":MSG_PRERENDER_SUCCESS,@"result":@"success"});
        }
    }
}

+ (BOOL)isTaskExist:(NSString *)url{
    return [[WXPrerenderManager sharedInstance]isTaskExist:url];
}

- (BOOL)isTaskExist:(NSString *)url
{
    if( !url ||url.length == 0){
        return NO;
    }
    WXPrerenderTask *task  = [self.prerenderTasks objectForKey:url];
    if(!task ){
        return NO;
    }
    if(![self isSwitchOn]){
        return NO;
    }
    // compare cache time with begin time
    NSTimeInterval time = [[NSDate date] timeIntervalSinceDate:task.beginDate];
    if(time > task.cacheTime){
        return NO;
    }
    
    if(task ){
        return YES;
    }
    return NO;
}

+ (void)renderFromCache:(NSString *)url
{
    WXPrerenderManager *manager = [WXPrerenderManager sharedInstance];
    if([manager isTaskExist:url])
    {
        WXPrerenderTask *task  = [manager.prerenderTasks objectForKey:url];
        task.instance.needPrerender = NO;
        UIView *view = [self viewFromUrl:url];
        NSError *error = [self errorFromUrl:url];
        if(task.instance.onCreate){
            task.instance.onCreate(view);
        }
        if(error && task.instance.onFailed){
            task.instance.onFailed(error);
        }
        WXPerformBlockOnComponentThread(^{
            [task.instance.componentManager excutePrerenderUITask:url];
        });
        WXPerformBlockOnBridgeThread(^(){
            [WXPrerenderManager excuteModuleTasksForUrl:url];
        });
    }
}

+ (UIView *)viewFromUrl:(NSString *)url
{
    WXPrerenderManager *manager = [WXPrerenderManager sharedInstance];
    WXPrerenderTask *task  = [manager.prerenderTasks objectForKey:url];
    return task.view;
}

+ (NSError *)errorFromUrl:(NSString *)url
{
    WXPrerenderManager *manager = [WXPrerenderManager sharedInstance];
    WXPrerenderTask *task  = [manager.prerenderTasks objectForKey:url];
    return task.error;
}

+ (id )instanceFromUrl:(NSString *)url
{
    WXPrerenderManager *manager = [WXPrerenderManager sharedInstance];
    WXPrerenderTask *task  = [manager.prerenderTasks objectForKey:url];
    return task.instance;
}

+ (void)removePrerenderTaskforUrl:(NSString *)url
{
    if (url) {
        WXPrerenderManager *manager = [WXPrerenderManager sharedInstance];
        [manager.prerenderTasks removeObjectForKey:url];
    }
}

+ (void)storePrerenderModuleTasks:(WXModuleMethod *)method forUrl:(NSString *)url
{
    WXPrerenderManager *manager = [WXPrerenderManager sharedInstance];
    WXPrerenderTask *task = [manager.prerenderTasks objectForKey:url];
    if (!task.moduleTasks){
        task.moduleTasks = [NSMutableArray new];
    }
    [task.moduleTasks addObject:method];
    if(task){
        [manager.prerenderTasks setObject:task forKey:url];
    }
}

+ (void)excuteModuleTasksForUrl:(NSString *)url
{
    WXPrerenderManager *manager = [WXPrerenderManager sharedInstance];
    WXPrerenderTask *task = [manager.prerenderTasks objectForKey:url];
    
    if (task.moduleTasks && [task.moduleTasks count]>0){
        for (WXModuleMethod *method in task.moduleTasks) {
            [method invoke];
        }
    }
}

+ (void)destroyTask:(NSString *)parentInstanceId
{
    WXPrerenderManager *manager = [WXPrerenderManager sharedInstance];
    if(!manager.prerenderTasks || [manager.prerenderTasks count] == 0){
        return;
    }
    
    [manager.prerenderTasks enumerateKeysAndObjectsUsingBlock:^(id key, id value, BOOL *stop) {
        WXPrerenderTask *task = manager.prerenderTasks[key];
        if ([task.parentInstanceId isEqualToString:parentInstanceId]) {
            [manager removeTask:task];
        }
    }];
}

- (void)removeTask:(WXPrerenderTask *)task
{
    [task.instance destroyInstance];
    [self.prerenderTasks removeObjectForKey:task.url];
}

@end
