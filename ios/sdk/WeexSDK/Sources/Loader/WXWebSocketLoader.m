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

#import "WXWebSocketLoader.h"
#import "WXWebSocketHandler.h"
#import "WXHandlerFactory.h"
#import "WXLog.h"

@interface WXWebSocketLoader () <WXWebSocketDelegate>
@property (nonatomic, copy) NSString *identifier;
@property (nonatomic, copy) NSString *url;
@property (nonatomic, copy) NSString *protocol;
@end

@implementation WXWebSocketLoader

- (instancetype)initWithUrl:(NSString *)url protocol:(NSString *)protocol
{
    if (self = [super init]) {
        self.url = url;
        self.protocol = protocol;
    }
    return self;
}

-(id)copyWithZone:(NSZone *)zone {
    
    WXWebSocketLoader *newClass = [[WXWebSocketLoader alloc]init];
    newClass.onOpen = self.onOpen;
    newClass.onReceiveMessage = self.onReceiveMessage;
    newClass.onFail = self.onFail;
    newClass.onClose = self.onClose;
    newClass.protocol = self.protocol;
    newClass.url = self.url;
    newClass.identifier = self.identifier;
    return newClass;
}

-(NSString *)identifier
{
    if(!_identifier)
    {
        CFUUIDRef uuid = CFUUIDCreate(NULL);
        _identifier = CFBridgingRelease(CFUUIDCreateString(NULL, uuid));
        assert(_identifier);
        CFRelease(uuid);
    }
    return _identifier;
}

- (void)open
{
    id<WXWebSocketHandler> requestHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXWebSocketHandler)];
    if (requestHandler) {
        [requestHandler open:self.url protocol:self.protocol identifier:self.identifier withDelegate:self];
    } else {
        WXLogError(@"No resource request handler found!");
    }
}

- (void)send:(NSString *)data
{
    id<WXWebSocketHandler> requestHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXWebSocketHandler)];
    if (requestHandler) {
        [requestHandler send:self.identifier data:data];
    } else {
        WXLogError(@"No resource request handler found!");
    }
}

- (void)close
{
    id<WXWebSocketHandler> requestHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXWebSocketHandler)];
    if (requestHandler) {
        [requestHandler close:self.identifier];
    } else {
        WXLogError(@"No resource request handler found!");
    }
}

- (void)clear
{
    id<WXWebSocketHandler> requestHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXWebSocketHandler)];
    if (requestHandler) {
        [requestHandler clear:self.identifier];
    } else {
        WXLogError(@"No resource request handler found!");
    }
}

- (void)close:(NSInteger)code reason:(NSString *)reason
{
    id<WXWebSocketHandler> requestHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXWebSocketHandler)];
    if (requestHandler) {
        [requestHandler close:self.identifier code:code reason:reason];
    } else {
        WXLogError(@"No resource request handler found!");
    }
}

#pragma mark - WXWebSocketDelegate
- (void)didOpen
{
    if (self.onOpen) {
        self.onOpen();
    }
}
- (void)didFailWithError:(NSError *)error
{
    if(self.onFail) {
        self.onFail(error);
    }
}
- (void)didReceiveMessage:(id)message
{
    if (self.onReceiveMessage) {
        self.onReceiveMessage(message);
    }
}
- (void)didCloseWithCode:(NSInteger)code reason:(NSString *)reason wasClean:(BOOL)wasClean
{
    if (self.onClose) {
        self.onClose(code,reason,wasClean);
    }
}
@end
