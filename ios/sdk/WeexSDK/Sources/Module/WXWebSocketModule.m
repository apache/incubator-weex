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

#import "WXWebSocketModule.h"
#import "WXUtility.h"
#import "WXWebSocketHandler.h"
#import "WXHandlerFactory.h"
#import "WXWebSocketLoader.h"
#import "WXConvert.h"

@interface WXWebSocketModule()

@property(nonatomic,copy)WXModuleKeepAliveCallback errorCallBack;
@property(nonatomic,copy)WXModuleKeepAliveCallback messageCallBack;
@property(nonatomic,copy)WXModuleKeepAliveCallback openCallBack;
@property(nonatomic,copy)WXModuleKeepAliveCallback closeCallBack;

@end

@implementation WXWebSocketModule
{
    WXWebSocketLoader *loader;
}
WX_EXPORT_METHOD(@selector(WebSocket:protocol:))
WX_EXPORT_METHOD(@selector(send:))
WX_EXPORT_METHOD(@selector(close:reason:))
WX_EXPORT_METHOD(@selector(onerror:))
WX_EXPORT_METHOD(@selector(onmessage:))
WX_EXPORT_METHOD(@selector(onopen:))
WX_EXPORT_METHOD(@selector(onclose:))

@synthesize weexInstance;

- (void)WebSocket:(NSString *)url protocol:(NSString *)protocol
{
    if(loader)
    {
        [loader clear];
    }
    loader = [[WXWebSocketLoader alloc] initWithUrl:url protocol:protocol];
    __weak typeof(self) weakSelf = self;
    loader.onReceiveMessage = ^(id message) {
        if (weakSelf) {
            NSMutableDictionary *dic = [NSMutableDictionary new];
            if([message isKindOfClass:[NSString class]]) {
                [dic setObject:message forKey:@"data"];
            }else if([message isKindOfClass:[NSData class]]){
                [dic setObject:[WXUtility dataToBase64Dict:message] forKey:@"data"];
            }
            if (weakSelf.messageCallBack) {
                weakSelf.messageCallBack(dic,true);;
            }
        }
    };
    loader.onOpen = ^() {
        if (weakSelf) {
            if (weakSelf.openCallBack) {
                NSMutableDictionary *dict = [NSMutableDictionary new];
                weakSelf.openCallBack(dict,true);;
            }
        }
    };
    loader.onFail = ^(NSError *error) {
        if (weakSelf) {
            WXLogError(@":( Websocket Failed With Error %@", error);
            NSMutableDictionary *dict = [NSMutableDictionary new];
            [dict setObject:error.userInfo?[WXUtility JSONString:error.userInfo]:@"" forKey:@"data"];
            if (weakSelf.errorCallBack) {
                weakSelf.errorCallBack(dict, true);
            }
        }
    };
    loader.onClose = ^(NSInteger code,NSString *reason,BOOL wasClean) {
        if (weakSelf) {
            if (weakSelf.closeCallBack) {
                WXLogInfo(@"Websocket colse ");
                NSMutableDictionary * callbackRsp = [[NSMutableDictionary alloc] init];
                [callbackRsp setObject:[NSNumber numberWithInteger:code] forKey:@"code"];
                [callbackRsp setObject:reason?reason:@"" forKey:@"reason"];
                [callbackRsp setObject:wasClean?@true:@false forKey:@"wasClean"];
                if (weakSelf.closeCallBack) {
                    weakSelf.closeCallBack(callbackRsp,false);
                }
            }
        }
    };
    
    [loader open];
}

- (void)send:(id)data
{
    if([data isKindOfClass:[NSString class]]){
        [loader send:data];
    }else if([data isKindOfClass:[NSDictionary class]]){
        NSData *sendData = [WXUtility base64DictToData:data];
        if(sendData){
            [loader send:sendData];
        }
        
    }
    
    
}

- (void)close
{
    [loader close];
}

- (void)close:(NSString *)code reason:(NSString *)reason
{
    if(!code)
    {
        [loader close];
        return;
    }
    [loader close:[code integerValue] reason:reason];
}

- (void)onerror:(WXModuleKeepAliveCallback)callback
{
    self.errorCallBack = callback;
}

- (void)onmessage:(WXModuleKeepAliveCallback)callback
{
    self.messageCallBack = callback;
}

- (void)onopen:(WXModuleKeepAliveCallback)callback
{
    self.openCallBack = callback;
}

- (void)onclose:(WXModuleKeepAliveCallback)callback
{
    self.closeCallBack = callback;
}

-(void)dealloc
{
    [loader clear];
}

@end
