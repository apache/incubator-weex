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

#import "WXTracingManager.h"
#import "WXComponentFactory.h"
#import "WXModuleFactory.h"
#import "WXSDKManager.h"
#import "WXComponentFactory.h"
#import "WXModuleFactory.h"
#import "WXHandlerFactory.h"
#import "WXUtility.h"
#import "WXComponentManager.h"
#import "WXTracingProtocol.h"
#import "WXSDKEngine.h"


@implementation WXTracing

-(NSDictionary *)dictionary {
    return [NSDictionary dictionaryWithObjectsAndKeys:self.ref?:@"",@"ref",self.parentRef?:@"",@"parentRef",self.className?:@"",@"className",self.name?:@"",@"name",self.ph?:@"",@"ph",@(self.ts),@"ts",@(self.traceId),@"traceId",@(self.duration),@"duration",self.fName?:@"",@"fName",self.iid?:@"",@"iid",@(self.parentId)?:0,@"parentId",self.threadName?:@"",@"tName", nil];
}
@end

@implementation WXTracingTask

@end

@interface WXTracingManager()

@property (nonatomic) BOOL isTracing;
@property (nonatomic, strong) NSMutableDictionary *tracingTasks;  // every instance have a task
@property (nonatomic, copy) NSString *currentInstanceId;  // every instance have a task

@end

@implementation WXTracingManager


+ (instancetype) sharedInstance{
    
    static WXTracingManager *instance = nil;
    static dispatch_once_t once;
    
    dispatch_once(&once, ^{
        instance = [[WXTracingManager alloc] initPrivate];
    });
    
    return instance;
}

- (instancetype) initPrivate{
    self = [super init];
    if(self){
        self.isTracing = NO;
    }
    
    return self;
}

+(void)switchTracing:(BOOL )isTracing
{
    [WXTracingManager sharedInstance].isTracing = isTracing;
}

+(BOOL)isTracing
{
    return [WXTracingManager sharedInstance].isTracing;
}

+(void)startTracing:(WXTracing *)tracing
{
    if([self isTracing]){
        if(![WXTracingManager sharedInstance].tracingTasks){
            [WXTracingManager sharedInstance].tracingTasks = [NSMutableDictionary new];
        }
        if(![[WXTracingManager sharedInstance].tracingTasks objectForKey:tracing.iid]){
            WXTracingTask *task = [WXTracingTask new];
            task.iid = tracing.iid;
            if (tracing.iid) {
                [[WXTracingManager sharedInstance].tracingTasks setObject:task forKey:tracing.iid];
            }
        }
        WXTracingTask *task = [[WXTracingManager sharedInstance].tracingTasks objectForKey:tracing.iid];
        if(task.bundleUrl.length == 0){
            if(tracing.bundleUrl.length>0){
                task.bundleUrl = tracing.bundleUrl;
            }
        }
        if(!task.tracings){
            task.tracings = [NSMutableArray new];
        }
        if(WXFloatEqual(tracing.ts,0)){
            NSTimeInterval time=[[NSDate date] timeIntervalSince1970]*1000;
            tracing.ts = time;
        }
        if(WXFloatEqual(tracing.duration,0)){
            tracing.duration = WXTracingDurationDefault;
        }
        if(![WXTracingEnd isEqualToString:tracing.ph]){ // end is should not update
            tracing.traceId = task.counter++;
        }
        if([WXTNetworkHanding isEqualToString:tracing.name] && [WXTracingBegin isEqualToString:tracing.ph]){
            task.tag = WXTNetworkHanding;
        }
        NSTimeInterval ts = [[NSDate date] timeIntervalSince1970]*1000;
        tracing.ts = ts ;
        dispatch_async(dispatch_get_main_queue(), ^{
            [self updateTracings:task tracing:tracing];
        });
    }
}

+(void)setBundleJSType:(NSString *)jsBundleString instanceId:(NSString *)iid
{
    if([self isTracing] && iid.length >0){
        WXTracingTask *task = [[WXTracingManager sharedInstance].tracingTasks objectForKey:iid];
        if(jsBundleString.length >0){
            NSRange range = [jsBundleString rangeOfString:@"}"];
            if (range.location != NSNotFound) {
                NSString *searchStr =  [jsBundleString substringToIndex:range.location+range.length];
                if ([searchStr rangeOfString:@"Vue"].location != NSNotFound){
                    task.bundleJSType = @"Vue";
                }else if([searchStr rangeOfString:@"Rax"].location != NSNotFound){
                    task.bundleJSType = @"Rax";
                }else{
                    task.bundleJSType = @"Weex";
                }
            }
        }
    }
}

+(void)clearTracingData
{
    [WXTracingManager sharedInstance].tracingTasks = nil;
}

+(NSTimeInterval)getCurrentTime
{
    if([self isTracing]){
        return  [[NSDate date] timeIntervalSince1970]*1000;
    }
    return 0;
}

+(void)startTracingWithInstanceId:(NSString *)iid ref:(NSString*)ref className:(NSString *)className name:(NSString *)name phase:(NSString *)phase functionName:(NSString *)functionName options:(NSDictionary *)options
{
    if([self isTracing]){
        WXTracing *tracing = [WXTracing new];
        tracing.parentId = -1;
        if(ref.length>0){
            tracing.ref = ref;
        }else {
            tracing.ref = @"";
        }
        if(className.length>0){
            tracing.className = className;
        }else {
            tracing.className = @"";
        }
        if(name.length>0){
            tracing.name = name;
        }
        if(functionName.length>0){
            tracing.fName = functionName;
        }
        if(phase.length>0){
            tracing.ph = phase;
        }
        if(iid.length>0){
            tracing.iid = iid;
        }
        
        if(options && options[@"ts"]){
            tracing.ts = [options[@"ts"] floatValue];
        }
        if(options && options[@"duration"]){
            tracing.duration =  [options[@"duration"] floatValue];
        }
        if(options && options[@"parentRef"]){
            tracing.parentRef = options[@"parentRef"];
        }
        if(options && options[@"bundleUrl"]){
            tracing.bundleUrl = options[@"bundleUrl"];
        }
        if(options && options[@"threadName"]){
            tracing.threadName = options[@"threadName"];
        }
        if(options && options[@"componentData"]){
            tracing.childrenRefs = [self getChildrenRefs:options[@"componentData"]];
        }
        [self startTracing:tracing];
    }
}

+ (NSMutableArray *)getChildrenRefs:(NSDictionary *)componentData
{
    NSMutableArray *mArray = [NSMutableArray new];
    [self recursively:mArray componentData:componentData];
    return mArray;
}

+(void)recursively:(NSMutableArray *)mArray componentData:(NSDictionary *)componentData
{
    if([componentData valueForKey:@"children"]){
        NSArray *children = [componentData valueForKey:@"children"];
        for(NSDictionary *subcomponentData in children){
            NSString *ref = [subcomponentData objectForKey:@"ref"];
            if(ref){
                [mArray addObject:ref];
            }
            [self recursively:mArray componentData:subcomponentData];
        }
    }
}

+(WXTracing *)copyTracing:(WXTracing *)tracing
{
    WXTracing *newTracing = [WXTracing new];
    if(tracing.ref.length>0){
        newTracing.ref = tracing.ref;
    }
    if(tracing.parentRef.length>0){
        newTracing.parentRef = tracing.parentRef;
    }
    if(tracing.className.length>0){
        newTracing.className = tracing.className;
    }
    if(tracing.name.length>0){
        newTracing.name = tracing.name;
    }
    if(tracing.fName.length>0){
        newTracing.fName = tracing.fName;
    }
    if(tracing.ph.length>0){
        newTracing.ph = tracing.ph;
    }
    if(tracing.iid.length>0){
        newTracing.iid = tracing.iid;
    }
    if(tracing.parentId>0){
        newTracing.parentId = tracing.parentId;
    }
    if(tracing.traceId>0){
        newTracing.traceId = tracing.traceId;
    }
    if(tracing.ts>0){
        newTracing.ts = tracing.ts;
    }
    if(tracing.threadName.length>0){
        newTracing.threadName = tracing.threadName;
    }
    return newTracing;
}

+(NSString *)getclassName:(WXTracing *)tracing
{
    NSString *className = @"";
    if(tracing.ref.length>0 && tracing.name.length >0){
        Class cls = [WXComponentFactory classWithComponentName:tracing.name];
        className = NSStringFromClass(cls);
    }else if(tracing.name.length > 0){
        Class cls = [WXModuleFactory classWithModuleName:tracing.name];
        if(cls){
            className = NSStringFromClass(cls);
        }
    }
    return  className;
}

+(NSInteger )getParentId:(WXTracingTask *)task tracing:(WXTracing *)tracing
{
    NSMutableArray *tracings = task.tracings;
    if([tracing.threadName isEqualToString:WXTJSBridgeThread]) {
        return -1;
    }
    if(tracings && [tracings count]>0){
        for (NSInteger i = [tracings count] - 1; i >= 0; i--) {
            WXTracing *t = tracings[i];
//            if([t.threadName isEqualToString:WXTJSBridgeThread]&& [self compareRef:tracing.ref withTracing:t] && ([t.name isEqualToString:tracing.name] || [t.name isEqualToString:WXTJSCall])){
            if([t.threadName isEqualToString:WXTJSBridgeThread]&& [self compareRef:tracing.ref withTracing:t]){
                if([t.fName isEqualToString:tracing.fName]){
                    return (NSInteger)t.traceId;
                }
            }
        }
    }
    return -1;
}

+(BOOL)compareRef:(NSString *)ref withTracing:(WXTracing *)tracing
{
    if([tracing.ref isEqualToString:ref]){
        return YES;
    }
    if(tracing.childrenRefs && [tracing.childrenRefs count] > 0){
        for (NSString *childRef in tracing.childrenRefs) {
            if([childRef isEqualToString:ref]){
                return YES;
            }
        }
    }
    return NO;
}

+(void)updateTracings:(WXTracingTask *)task tracing:(WXTracing *)tracing
{
    if([WXTNetworkHanding isEqualToString:task.tag]){
        if([WXTExecJS isEqualToString:tracing.name]){
            NSMutableArray *tracings = task.tracings;
            [tracings enumerateObjectsWithOptions:NSEnumerationReverse usingBlock:^(WXTracing *bTracing, NSUInteger idx, BOOL *stop) {
                if(([WXTNetworkHanding isEqualToString:bTracing.name] || [bTracing.ref isEqualToString:tracing.ref])&&[WXTracingBegin isEqualToString:bTracing.ph]){
                    WXTracing *newTracing = [self copyTracing:bTracing];
                    newTracing.iid = tracing.iid;
                    newTracing.ph = WXTracingEnd;
                    newTracing.ts = tracing.ts ;
                    newTracing.duration = newTracing.ts - bTracing.ts ;
                    bTracing.duration = newTracing.duration;
                    [task.tracings addObject:newTracing];
                    *stop = YES;
                }
            }];
            task.tag = WXTExecJS;
        }
    }
    
    if([WXTracingBegin isEqualToString:tracing.ph]){
        if(tracing.ref.length>0){
            WXPerformBlockOnComponentThread(^{
                WXSDKInstance *instance = [WXSDKManager instanceForID:task.iid];
                WXComponent *com = [instance componentForRef:tracing.ref];
                if(task.bundleUrl.length == 0){
                    task.bundleUrl = [instance.scriptURL absoluteString];
                }
                if(com.supercomponent){
                    tracing.parentRef = com.supercomponent.ref;
                }
                tracing.name = com.type;
                if(tracing.className.length == 0){
                    tracing.className = [self getclassName:tracing];
                }
            });
            if(tracing.parentId == -1){
                tracing.parentId = [self getParentId:task tracing:tracing];
            }
        } else {
            if(tracing.parentId == -1){
                tracing.parentId = [self getParentId:task tracing:tracing];
            }
        }
        
    }
    if([WXTracingEnd isEqualToString:tracing.ph]){  // deal end
        NSMutableArray *tracings = task.tracings;
        [tracings enumerateObjectsWithOptions:NSEnumerationReverse usingBlock:^(WXTracing *bTracing, NSUInteger idx, BOOL *stop) {
            if(tracing.ref.length > 0 && bTracing.ref.length>0){
                if(![tracing.ref isEqualToString:bTracing.ref]){
                    return ;
                }
            }
            
            if([bTracing.fName isEqualToString:tracing.fName] &&[WXTracingBegin isEqualToString:bTracing.ph] ){
                tracing.iid = bTracing.iid;
                if(bTracing.ref.length > 0){
                    tracing.ref = bTracing.ref;
                }
                if(bTracing.parentRef.length > 0){
                    tracing.parentRef = bTracing.parentRef;
                }
                tracing.parentId = bTracing.parentId;
                tracing.duration = tracing.ts - bTracing.ts ;
                tracing.traceId = bTracing.traceId;
                tracing.threadName = bTracing.threadName;
                bTracing.duration = tracing.duration;
                tracing.name = bTracing.name;
                *stop = YES;
            }
        }];
    }
    
    [task.tracings addObject:tracing];
    if([@"renderFinish" isEqualToString:tracing.fName] && [WXTracingInstant isEqualToString:tracing.ph] && [WXTUIThread isEqualToString:tracing.threadName]) {
        [WXTracingManager commitTracing:task.iid];
    }
}

+(NSMutableDictionary *)getTracingData
{
    if(![self isTracing]){
        return nil;
    }
    
    return [WXTracingManager sharedInstance].tracingTasks;
}

+(NSDictionary *)getTacingApi
{
    if(![self isTracing]){
        return @{};
     }
    NSMutableDictionary *dict = [NSMutableDictionary new];
    NSMutableArray *componetArray = [NSMutableArray new];
    NSMutableArray *moduleArray = [NSMutableArray new];
    NSMutableArray *handleArray = [NSMutableArray new];
    NSDictionary *componentConfigs = [WXComponentFactory componentConfigs];
    void (^componentBlock)(id, id, BOOL *) = ^(id mKey, id mObj, BOOL * mStop) {
        NSMutableDictionary *componentConfig = [mObj mutableCopy];
        NSDictionary *cDict = [WXComponentFactory componentSelectorMapsWithName:componentConfig[@"name"]];
        if(cDict && [cDict count]>0 && [cDict[@"methods"] count]>0){
            [componentConfig setObject:cDict[@"methods"] forKey:@"methods"];
        }
        [componetArray addObject:componentConfig];
    };
    [componentConfigs enumerateKeysAndObjectsUsingBlock:componentBlock];
    if(componetArray && [componetArray count]>0){
        [dict setObject:componetArray forKey:@"componet"];
    }
    NSDictionary *moduleConfigs = [WXModuleFactory moduleConfigs];
    void (^moduleBlock)(id, id, BOOL *) = ^(id mKey, id mObj, BOOL * mStop) {
        NSDictionary *mDict = [WXModuleFactory moduleSelectorMapsWithName:mKey];
        NSMutableDictionary *subDict = [NSMutableDictionary new];
        [subDict setObject:mKey forKey:@"name"];
        [subDict setObject:mObj forKey:@"class"];
        if([mDict objectForKey:mKey]){
            [subDict setObject:[mDict objectForKey:mKey] forKey:@"methods"];
        }
        [moduleArray addObject:subDict];
    };
    [moduleConfigs enumerateKeysAndObjectsUsingBlock:moduleBlock];
    if(moduleArray && [moduleArray count]>0){
        [dict setObject:moduleArray forKey:@"module"];
    }
    
    NSDictionary *handleConfigs = [[WXHandlerFactory handlerConfigs] mutableCopy];
    void (^handleBlock)(id, id, BOOL *) = ^(id mKey, id mObj, BOOL * mStop) {
        NSMutableDictionary *subDict = [NSMutableDictionary new];
        [subDict setObject:mKey forKey:@"class"];
        [subDict setObject:NSStringFromClass([mObj class]) forKey:@"name"];
        [handleArray addObject:subDict];
    };
    [handleConfigs enumerateKeysAndObjectsUsingBlock:handleBlock];
    if(handleArray && [handleArray count]>0){
        [dict setObject:handleArray forKey:@"handle"];
    }
    
    return dict;
}

+ (void)commitTracing:(NSString *)instanceId
{
    if(![self isTracing]){
        return ;
    }
    id tracingHandle = [WXSDKEngine handlerForProtocol:@protocol(WXTracingProtocol)];
    if ([tracingHandle respondsToSelector:@selector(commitTracingInfo:)]){
        WXTracingTask *task = [[WXTracingManager sharedInstance].tracingTasks objectForKey:instanceId];
        if(task) {
            [tracingHandle commitTracingInfo:task];
        }
        
    }
}

+ (void)commitTracingSummaryInfo:(NSDictionary *)info withInstanceId:(NSString *)instanceId
{
    if(![self isTracing]){
        return ;
    }
    id tracingHandle = [WXSDKEngine handlerForProtocol:@protocol(WXTracingProtocol)];
    if ([tracingHandle respondsToSelector:@selector(commitTracingSummaryInfo:)]){
        if(info) {
            NSMutableDictionary *newInfo = [info mutableCopy];
            if(instanceId.length > 0){
                [newInfo setObject:instanceId forKey:@"instanceId"];
                [newInfo setObject:[[WXUtility getEnvironment] objectForKey:@"platform"] forKey:@"platform"];
            }
            [tracingHandle commitTracingSummaryInfo:newInfo];
        }
        
    }
}

+(void)destroyTraincgTaskWithInstance:(NSString *)instanceId
{
    if(![self isTracing]){
        return ;
    }
    WXTracingTask *task = [[WXTracingManager sharedInstance].tracingTasks objectForKey:instanceId];
    if(task){
        [[WXTracingManager sharedInstance].tracingTasks removeObjectForKey:instanceId];
    }
}

@end
