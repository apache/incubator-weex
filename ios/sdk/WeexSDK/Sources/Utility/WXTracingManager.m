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

@implementation WXTracing

@end

@implementation WXTracingTask

@end

@interface WXTracingManager()

@property (nonatomic) BOOL isTracing;
@property (nonatomic, strong) NSMutableDictionary *tracingTasks;  // every instance have a task

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
        self.isTracing = YES;
    }
    
    return self;
}

+(void)switchTracing:(BOOL )isTracing
{
    [WXTracingManager sharedInstance].isTracing = isTracing;
}

+(BOOL)isTracing
{
    return YES;
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
            [[WXTracingManager sharedInstance].tracingTasks setObject:task forKey:tracing.iid];
        }
        WXTracingTask *task = [[WXTracingManager sharedInstance].tracingTasks objectForKey:tracing.iid];
        if(!task.tracings){
            task.tracings = [NSMutableArray new];
        }
        NSTimeInterval time=[[NSDate date] timeIntervalSince1970]*1000;
        tracing.ts = time;
        if(![WXTracingEnd isEqualToString:tracing.ph]){ // end is should not update
            tracing.traceId = task.counter++;
        }
        if([WXTracingDataHanding isEqualToString:tracing.name] && [WXTracingBegin isEqualToString:tracing.ph]){
            task.tag = WXTDataHanding;
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            [self updateTracings:task tracing:tracing];
        });
    }
}

+(void)startTracing:(NSString *)iid ref:(NSString*)ref className:(NSString *)className name:(NSString *)name ph:(NSString *)ph fName:(NSString *)fName parentId:(NSString *)parentId
{
    if([self isTracing]){
        WXTracing *tracing = [WXTracing new];
        if(ref.length>0){
            tracing.ref = ref;
        }
        if(className.length>0){
            tracing.className = className;
        }
        if(name.length>0){
            tracing.name = name;
        }
        if(fName.length>0){
            tracing.fName = fName;
        }
        if(ph.length>0){
            tracing.ph = ph;
        }
        if(iid.length>0){
            tracing.iid = iid;
        }
        if(parentId.length>0){
            tracing.parentId = parentId;
        }
        [self startTracing:tracing];
    }
}

+(WXTracing *)copyTracing:(WXTracing *)tracing
{
    WXTracing *newTracing = [WXTracing new];
    if(tracing.ref.length>0){
        newTracing.ref = tracing.ref;
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
    if(tracing.parentId.length>0){
        newTracing.parentId = tracing.parentId;
    }
    if(tracing.traceId>0){
        newTracing.traceId = tracing.traceId;
    }
    if(tracing.ts>0){
        newTracing.ts = tracing.ts;
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

+(void)updateTracings:(WXTracingTask *)task tracing:(WXTracing *)tracing
{
    tracing.className = [self getclassName:tracing];
    if(task.tag == WXTDataHanding){
        if([WXTracingJSCall isEqualToString:tracing.name]){
            NSMutableArray *tracings = [task.tracings copy];
            [tracings enumerateObjectsWithOptions:NSEnumerationReverse usingBlock:^(WXTracing *bTracing, NSUInteger idx, BOOL *stop) {
                if(([WXTracingDataHanding isEqualToString:bTracing.name] || [bTracing.ref isEqualToString:tracing.ref])&&[WXTracingBegin isEqualToString:bTracing.ph]){
                    WXTracing *newTracing = [self copyTracing:bTracing];
                    newTracing.iid = newTracing.iid;
                    newTracing.ph = WXTracingEnd;
                    newTracing.duration = [[NSDate date] timeIntervalSince1970]*1000 - bTracing.ts ;
                    [task.tracings addObject:newTracing];
                    *stop = YES;
                }
            }];
            task.tag = WXTRender;
        }
    }
    if([WXTracingEnd isEqualToString:tracing.ph]){  // deal end
        NSMutableArray *tracings = [task.tracings copy];
        [tracings enumerateObjectsWithOptions:NSEnumerationReverse usingBlock:^(WXTracing *bTracing, NSUInteger idx, BOOL *stop) {
            if(([bTracing.fName isEqualToString:tracing.fName] || [bTracing.ref isEqualToString:tracing.ref])&&[WXTracingBegin isEqualToString:bTracing.ph]){
                tracing.iid = bTracing.iid;
                tracing.duration = [[NSDate date] timeIntervalSince1970]*1000 - bTracing.ts ;
                *stop = YES;
            }
        }];
    }
    [task.tracings addObject:tracing];
}

+ (void)tracingGloabalTask:(NSString *)fName instanceId:(NSString *)instanceId ph:(NSString *)ph
{
    WXTracing *tracing = [WXTracing new];
    tracing.iid = instanceId;
    tracing.name = @"jsCall";
    tracing.ph = ph;
    tracing.fName = fName;
    [WXTracingManager startTracing:tracing];
}

+(void)getTracingData:(NSString *)instanceId{
    WXTracingTask *task = [[WXTracingManager sharedInstance].tracingTasks objectForKey:instanceId];
    NSArray *tracings = [task.tracings copy];
    for (WXTracing *tracing in tracings) {
        if(![WXTracingBegin isEqualToString:tracing.ph]){
            NSLog(@"%@  %@  %@  %f",tracing.fName,tracing.name,tracing.className,tracing.duration);
        }
    }
}

@end
