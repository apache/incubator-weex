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

#import "WXTimerModule.h"
#import "WXSDKManager.h"
#import "WXLog.h"
#import "WXAssert.h"

@interface WXTimerTarget : NSObject

- (instancetype)initWithCallback:(NSString *)callbackID shouldRepeat:(BOOL)shouldRepeat weexInstance:(WXSDKInstance *)weexInstance;

@end

@implementation WXTimerTarget
{
    NSString * _callbackID;
    __weak WXSDKInstance *_weexInstance;
    BOOL _shouldRepeat;
}

- (instancetype)initWithCallback:(NSString *)callbackID shouldRepeat:(BOOL)shouldRepeat weexInstance:(WXSDKInstance *)weexInstance
{
    if (self = [super init]) {
        _callbackID = callbackID;
        _weexInstance = weexInstance;
        _shouldRepeat = shouldRepeat;
    }
    
    return self;
}

- (void)trigger
{
    [[WXSDKManager bridgeMgr] callBack:_weexInstance.instanceId funcId:_callbackID params:nil keepAlive:_shouldRepeat];
}

@end

@implementation WXTimerModule
{
    NSMutableDictionary *_timers;
}

@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(setTimeout:time:))
WX_EXPORT_METHOD(@selector(clearTimeout:))
WX_EXPORT_METHOD(@selector(setInterval:time:))
WX_EXPORT_METHOD(@selector(clearInterval:))

- (instancetype)init
{
    if (self = [super init]) {
        _timers = [NSMutableDictionary dictionary];
    }
    
    return self;
}

- (void)createTimerWithCallback:(NSString *)callbackID time:(NSTimeInterval)milliseconds shouldRepeat:(BOOL)shouldRepeat
{
    WXAssert(callbackID, @"callbackID for timer must not be nil.");
    
    if (milliseconds == 0 && !shouldRepeat) {
        [[WXSDKManager bridgeMgr] callBack:self.weexInstance.instanceId funcId:callbackID params:nil keepAlive:NO];
    }
    
    WXTimerTarget *target = [[WXTimerTarget alloc] initWithCallback:callbackID shouldRepeat:shouldRepeat weexInstance:self.weexInstance];
    
    [self createTimerWithCallback:callbackID time:milliseconds target:target selector:@selector(trigger) shouldRepeat:shouldRepeat];
}

# pragma mark Timer API

- (void)setTimeout:(NSString *)callbackID time:(NSTimeInterval)time
{
    [self createTimerWithCallback:callbackID time:time shouldRepeat:NO];
}

- (void)setInterval:(NSString *)callbackID time:(NSTimeInterval)time
{
    [self createTimerWithCallback:callbackID time:time shouldRepeat:YES];
}

- (void)clearTimeout:(NSString *)callbackID
{
    if (!callbackID) {
        WXLogError(@"no callbackID for clearTimeout/clearInterval");
        return;
    }
    
    NSTimer *timer = _timers[callbackID];
    if (!timer) {
        WXLogWarning(@"no timer found for callbackID:%@", callbackID);
        return;
    }

    [timer invalidate];
    [_timers removeObjectForKey:callbackID];
}

- (void)clearInterval:(NSString *)callbackID
{
    [self clearTimeout:callbackID];
}

- (void)dealloc
{
    if (_timers) {
        for (NSString *callbackID in _timers) {
            NSTimer *timer = _timers[callbackID];
            [timer invalidate];
        }
        [_timers removeAllObjects];
        _timers = nil;
    }
}

# pragma mark Unit Test

- (void)createTimerWithCallback:(NSString *)callbackID time:(NSTimeInterval)milliseconds target:(id)target selector:(SEL)selector shouldRepeat:(BOOL)shouldRepeat {
    
    NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:milliseconds/1000.0f target:target selector:selector userInfo:nil repeats:shouldRepeat];
    [[NSRunLoop mainRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
    
    if (!_timers[callbackID]) {
        _timers[callbackID] = timer;
    }
}

- (NSMutableDictionary *)timers
{
    return _timers;
}

@end
