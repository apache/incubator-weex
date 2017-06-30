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
        if(![[WXTracingManager sharedInstance].tracingTasks objectForKey:tracing.instanceId]){
            WXTracingTask *task = [WXTracingTask new];
            task.instanceId = tracing.instanceId;
            [[WXTracingManager sharedInstance].tracingTasks setObject:task forKey:tracing.instanceId];
        }
        WXTracingTask *task = [[WXTracingManager sharedInstance].tracingTasks objectForKey:tracing.instanceId];
        if(!task.tracings){
            task.tracings = [NSMutableArray new];
        }
        NSTimeInterval time=[[NSDate date] timeIntervalSince1970]*1000;
        tracing.ts = time;
        [self updateTracings:task tracing:tracing];
    }
}

+(void)updateTracings:(WXTracingTask *)task tracing:(WXTracing *)tracing
{
    if([WXTracingEnd isEqualToString:tracing.ph]){
        NSMutableArray *tracings = [task.tracings copy];
        for (WXTracing *bTracing in tracings) {
            if(([bTracing.fName isEqualToString:tracing.fName] || [bTracing.ref isEqualToString:tracing.ref])&&[WXTracingBegin isEqualToString:bTracing.ph]){
                bTracing.ph = WXTracingDuration;
                bTracing.duration = [[NSDate date] timeIntervalSince1970]*1000 - bTracing.ts ;
                return;
            }
        }
    }
    [task.tracings addObject:tracing];
}

+ (void)tracingGloabalTask:(NSString *)fName instanceId:(NSString *)instanceId ph:(NSString *)ph
{
    WXTracing *tracing = [WXTracing new];
    tracing.instanceId = instanceId;
    tracing.name = @"global";
    tracing.ph = ph;
    tracing.fName = fName;
    dispatch_async(dispatch_get_main_queue(), ^{
        [WXTracingManager startTracing:tracing];
    });
}

+(void)getTracingData:(NSString *)instanceId{
    WXTracingTask *task = [[WXTracingManager sharedInstance].tracingTasks objectForKey:instanceId];
    NSArray *tracings = [task.tracings copy];
    for (WXTracing *tracing in tracings) {
        NSLog(@"%@  %@  %f",tracing.fName,tracing.name,tracing.duration);
    }
}

@end
