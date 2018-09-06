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

#import "WXInstanceHandlerFactory.h"
#import "WXThreadSafeMutableDictionary.h"
#import "WXAssert.h"

@interface WXInstanceHandlerFactory()
@property (nonatomic, strong) WXThreadSafeMutableDictionary *handlers;
@end

@implementation WXInstanceHandlerFactory

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.handlers = [WXThreadSafeMutableDictionary new];
    }
    return self;
}

- (void)registerHandler:(id)handler withProtocol:(Protocol *)protocol
{
    WXAssert(handler && protocol, @"Handler or protocol for registering can not be nil.");
    WXAssertProtocol(handler, protocol);
    [self.handlers setObject:handler forKey:NSStringFromProtocol(protocol)];
}

- (id)handlerForProtocol:(Protocol *)protocol
{
    WXAssert(protocol, @"Can not find handler for a nil protocol");
    id handler = [self.handlers objectForKey:NSStringFromProtocol(protocol)];
    return handler;
}

@end
