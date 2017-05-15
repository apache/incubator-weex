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

static NSString *const MSG_PRERENDER_INTERNAL_ERROR = @"internal_error";

@interface WXPrerenderTask:NSObject

@property (nonatomic, strong) WXSDKInstance *instance;
@property (nonatomic, strong) UIView *view;
@property (nonatomic, copy) NSString *parentInstanceId;
@property (nonatomic, copy) NSString *url;
@property (nonatomic, strong) NSMutableArray *moduleTasks;
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

- (void) executeTask:(NSString *)urlStr WXInstance:(NSString *)instanceId callback:(WXModuleCallback)callback{
    NSURL *url = [NSURL URLWithString:urlStr];
    if(!url){
        callback(@{@"url":urlStr,@"message":MSG_PRERENDER_INTERNAL_ERROR,@"result":@"error"});
        return;
    }
    
    __weak WXPrerenderManager *weakSelf = self;
    dispatch_async(self.queue, ^{
        [weakSelf prerender:url WXInstance:instanceId callback:callback];
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
        callback(@{@"url":[url absoluteString],@"message":MSG_PRERENDER_INTERNAL_ERROR,@"result":@"error"});
    }
    WXPrerenderTask *task = [WXPrerenderTask new];
    id configCenter = [WXSDKEngine handlerForProtocol:@protocol(WXConfigCenterProtocol)];
    if(![self isSwitchOn]){
        callback(@{@"url":[url absoluteString],@"message":MSG_PRERENDER_INTERNAL_ERROR,@"result":@"error"});
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
    
    if(!self.prerenderTasks || self.prerenderTasks.count<self.maxCacheNumber){
        [self.prerenderTasks setObject:task forKey:url.absoluteString];
        [instance renderWithURL:url options:@{@"bundleUrl":url.absoluteString} data:nil];
        __weak typeof(self) weakSelf = self;
        instance.onCreate = ^(UIView *view) {
            WXPrerenderTask *task = [weakSelf.prerenderTasks objectForKey:url.absoluteString];
            task.view = view;
            [weakSelf.prerenderTasks setObject:task forKey:url.absoluteString];
        };
        
        instance.onFailed = ^(NSError *error) {
            
        };
    }
}

-(NSString *)prerenderUrl:(NSURL *)scriptUrl
{
    if(scriptUrl){
        return scriptUrl.absoluteString;
    }
    return @"";
}

- (BOOL)isTaskExist:(NSString *)url
{
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

- (void) renderFromCache:(NSString *)url
{
    if([self isTaskExist:url])
    {
        WXPrerenderTask *task  = [self.prerenderTasks objectForKey:url];
        task.instance.needPrerender = NO;
        WXPerformBlockOnComponentThread(^{
            [task.instance.componentManager excutePrerenderUITask:url];
        });
        __weak typeof(self) weakSelf = self;
        WXPerformBlockOnBridgeThread(^(){
            [weakSelf excuteModuleTasksForUrl:url];
        });
    }
}

-(UIView *)viewFromUrl:(NSString *)url
{
    WXPrerenderTask *task  = [self.prerenderTasks objectForKey:url];
    return task.view;
}

- (id )instanceFromUrl:(NSString *)url
{
    WXPrerenderTask *task  = [self.prerenderTasks objectForKey:url];
    return task.instance;
}

- (void)removePrerenderTaskforUrl:(NSString *)url
{
    if (url) {
        [self.prerenderTasks removeObjectForKey:url];
    }
}

- (void)storePrerenderModuleTasks:(WXModuleMethod *)method forUrl:(NSString *)url
{
    WXPrerenderTask *task = [self.prerenderTasks objectForKey:url];
    if (!task.moduleTasks){
        task.moduleTasks = [NSMutableArray new];
    }
    [task.moduleTasks addObject:method];
    [self.prerenderTasks setObject:task forKey:url];
}

- (void)excuteModuleTasksForUrl:(NSString *)url
{
    WXPrerenderTask *task = [self.prerenderTasks objectForKey:url];
    
    if (task.moduleTasks && [task.moduleTasks count]>0){
        for (WXModuleMethod *method in task.moduleTasks) {
            [method invoke];
        }
    }
}

- (void)destroyTask:(NSString *)parentInstanceId
{
    if(!self.prerenderTasks || [self.prerenderTasks count] == 0){
        return;
    }
    
    [self.prerenderTasks enumerateKeysAndObjectsUsingBlock:^(id key, id value, BOOL *stop) {
        WXPrerenderTask *task = self.prerenderTasks[key];
        if ([task.parentInstanceId isEqualToString:parentInstanceId]) {
            [self removeTask:task];
        }
    }];
}

- (void)removeTask:(WXPrerenderTask *)task
{
    [task.instance destroyInstance];
    [self.prerenderTasks removeObjectForKey:task.url];
}

@end
