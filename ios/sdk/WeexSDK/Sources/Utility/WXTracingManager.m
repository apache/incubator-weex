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

@implementation WXComponentTracing

@end

@implementation WXTracingTask

@end

@interface WXTracingManager()

@property (nonatomic) BOOL isTracing;
@property (nonatomic, strong) NSMutableDictionary *tracingTasks;

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

+(void)monitorComponent:(WXComponentTracing *)componentTracing
{
    if([self isTracing]){
        if(![WXTracingManager sharedInstance].tracingTasks){
            [WXTracingManager sharedInstance].tracingTasks = [NSMutableDictionary new];
        }
        if(![[WXTracingManager sharedInstance].tracingTasks objectForKey:componentTracing.instance.instanceId]){
            WXTracingTask *task = [WXTracingTask new];
            task.instance = componentTracing.instance;
            [[WXTracingManager sharedInstance].tracingTasks setObject:task forKey:componentTracing.instance.instanceId];
        }
        WXTracingTask *task = [[WXTracingManager sharedInstance].tracingTasks objectForKey:componentTracing.instance.instanceId];
        //        NSTimeInterval time=[[NSDate date] timeIntervalSince1970]*1000;
        //        componentTracing.ts = time;
        if(!task.componentTracings){
            task.componentTracings = [NSMutableArray new];
        }
        [task.componentTracings addObject:componentTracing];
    }
}

+(double)getMonitorComponent:(WXComponentTracing *)componentTracing
{
    WXTracingTask *task = [[WXTracingManager sharedInstance].tracingTasks objectForKey:componentTracing.instance.instanceId];
    NSArray *componentTracings = [task.componentTracings copy];
    NSString *ref = componentTracing.ref;
    NSTimeInterval b = 0;
    for (WXComponentTracing *tracing in componentTracings) {
        if(![ref isEqualToString:tracing.ref]){
            continue;
        }
        if([@"B" isEqualToString:tracing.ph]){
            b = tracing.ts;
        }
        if([@"E" isEqualToString:tracing.ph]){
            double dt = tracing.ts - b;
            NSLog(@"%@ %f",componentTracing.name,dt);
            return dt;
        }
    }
    return 0;
}

@end
