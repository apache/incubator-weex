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

#import "WXDisplayLinkManager.h"
#import <UIKit/UIKit.h>
#import "WXAssert.h"

@implementation WXDisplayLinkManager
{
    CADisplayLink *_displayLink;
    NSMutableArray<id<WXDisplayLinkClient>>* _displayArray;
}

+ (instancetype)sharedInstance {
    WXAssertComponentThread();

    static WXDisplayLinkManager *_sharedDisplayLinkManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedDisplayLinkManager = [[self alloc] init];
    });
    [_sharedDisplayLinkManager _startDisplayLink];
    return _sharedDisplayLinkManager;
}

- (instancetype)init
{
    if (self == [super init]) {
        _displayArray = [NSMutableArray array];
    }
    return self;
}

- (void)registerDisplayClient:(id<WXDisplayLinkClient>)client
{
    WXAssertComponentThread();

    if ([_displayArray containsObject:client]) {
        return;
    }
    [_displayArray addObject:client];
}

- (void)unregisterDisplayClient:(id<WXDisplayLinkClient>)client
{
    WXAssertComponentThread();

    [_displayArray removeObject:client];
    if ([_displayArray count] == 0) {
        [self _stopDisplayLink];
    }
}

- (void)_startDisplayLink
{    
    if (!_displayLink) {
        _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(_handleDisplayLink)];
        [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
}

- (void)_stopDisplayLink
{
    if (_displayLink) {
        [_displayLink invalidate];
        _displayLink = nil;
    }
}

- (void)_handleDisplayLink
{
    for (id<WXDisplayLinkClient> client in _displayArray) {
        if (!client.suspend) {
            [client handleDisplayLink];
        }
    }
}

@end
