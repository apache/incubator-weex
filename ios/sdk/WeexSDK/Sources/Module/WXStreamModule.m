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

#import "WXStreamModule.h"
#import "WXSDKManager.h"
#import "WXUtility.h"
#import "WXHandlerFactory.h"
#import "WXNetworkProtocol.h"
#import "WXURLRewriteProtocol.h"
#import "WXResourceLoader.h"
#import "WXSDKEngine.h"

@implementation WXStreamModule

@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(sendHttp:callback:))
WX_EXPORT_METHOD(@selector(fetch:callback:progressCallback:))
WX_EXPORT_METHOD(@selector(fetchWithArrayBuffer:options:callback:progressCallback:))

- (void)fetch:(NSDictionary *)options callback:(WXModuleKeepAliveCallback)callback progressCallback:(WXModuleKeepAliveCallback)progressCallback
{
    __block NSInteger received = 0;
    __block NSHTTPURLResponse *httpResponse = nil;
    __block NSMutableDictionary * callbackRsp =[[NSMutableDictionary alloc] init];
    __block NSString *statusText = @"ERR_CONNECT_FAILED";
    
    //build stream request
    WXResourceRequest * request = [self _buildRequestWithOptions:options callbackRsp:callbackRsp];
    if (!request) {
        if (callback) {
            callback(callbackRsp, NO);
        }
        // failed with some invaild inputs
        return ;
    }
    
    // notify to start request state
    if (progressCallback) {
        progressCallback(callbackRsp, TRUE);
    }
    
    WXResourceLoader *loader = [[WXResourceLoader alloc] initWithRequest:request];
    __weak typeof(self) weakSelf = self;
    loader.onResponseReceived = ^(const WXResourceResponse *response) {
        httpResponse = (NSHTTPURLResponse*)response;
        if (weakSelf) {
            [callbackRsp setObject:@{ @"HEADERS_RECEIVED" : @2 } forKey:@"readyState"];
            [callbackRsp setObject:[NSNumber numberWithInteger:httpResponse.statusCode] forKey:@"status"];
            [callbackRsp setObject:httpResponse.allHeaderFields forKey:@"headers"];
            statusText = [WXStreamModule _getStatusText:httpResponse.statusCode];
            [callbackRsp setObject:statusText forKey:@"statusText"];
            [callbackRsp setObject:[NSNumber numberWithInteger:received] forKey:@"length"];
            if (progressCallback) {
                progressCallback(callbackRsp, TRUE);
            }
        }
    };
    
    loader.onDataReceived = ^(NSData *data) {
        if (weakSelf) {
            [callbackRsp setObject:@{ @"LOADING" : @3 } forKey:@"readyState"];
            received += [data length];
            [callbackRsp setObject:[NSNumber numberWithInteger:received] forKey:@"length"];
            if (progressCallback) {
                progressCallback(callbackRsp, TRUE);
            }
        }
    };
    
    loader.onFinished = ^(const WXResourceResponse * response, NSData *data) {
        if (weakSelf && callback) {
             [weakSelf _loadFinishWithResponse:[response copy] data:data callbackRsp:callbackRsp];
             callback(callbackRsp, NO);
        }
    };
    
    loader.onFailed = ^(NSError *error) {
        if (weakSelf && callback) {
            [weakSelf _loadFailedWithError:error callbackRsp:callbackRsp];
            callback(callbackRsp, NO);
        }
    };
    
    [loader start];
}

- (void)fetchWithArrayBuffer:(id)arrayBuffer options:(NSDictionary *)options callback:(WXModuleKeepAliveCallback)callback progressCallback:(WXModuleKeepAliveCallback)progressCallback
{
    NSMutableDictionary *newOptions = [options mutableCopy];
    if([arrayBuffer isKindOfClass:[NSDictionary class]]){
        NSData *sendData = [WXUtility base64DictToData:arrayBuffer];
        if(sendData){
            [newOptions setObject:sendData forKey:@"body"];
        }
    }
    [self fetch:newOptions callback:callback progressCallback:progressCallback];
}

- (WXResourceRequest*)_buildRequestWithOptions:(NSDictionary*)options callbackRsp:(NSMutableDictionary*)callbackRsp
{
    // parse request url
    NSString *urlStr = [options objectForKey:@"url"];
    NSString *newURL = [urlStr copy];
    WX_REWRITE_URL(urlStr, WXResourceTypeLink, self.weexInstance)
    urlStr = newURL;
    
    if (!options || [WXUtility isBlankString:urlStr]) {
        [callbackRsp setObject:@(-1) forKey:@"status"];
        [callbackRsp setObject:@NO forKey:@"ok"];
        
        return nil;
    }
    
    WXResourceRequest *request = [WXResourceRequest requestWithURL:[NSURL URLWithString:urlStr] resourceType:WXResourceTypeOthers referrer:nil cachePolicy:NSURLRequestUseProtocolCachePolicy];
    
    // parse http method
    NSString *method = [options objectForKey:@"method"];
    if ([WXUtility isBlankString:method]) {
        // default HTTP method is GET
        method = @"GET";
    }
    request.HTTPMethod = method;
    
    //parse responseType
    NSString *responseType = [options objectForKey:@"type"];
    if ([responseType isKindOfClass:[NSString class]]) {
        [callbackRsp setObject:responseType? responseType.lowercaseString:@"" forKey:@"responseType"];
    }
    
    //parse timeout
    if ([options valueForKey:@"timeout"]){
        //the time unit is ms
        [request setTimeoutInterval:([[options valueForKey:@"timeout"] floatValue])/1000];
    }
    
    //install client userAgent
    request.userAgent = [WXUtility userAgent];
    
    // parse custom http headers
    NSDictionary *headers = [options objectForKey:@"headers"];
    for (NSString *header in headers) {
        NSString *value = [headers objectForKey:header];
        [request setValue:value forHTTPHeaderField:header];
    }
    
    //parse custom body
    if ([options objectForKey:@"body"]) {
        NSData * body = nil;
        if ([[options objectForKey:@"body"] isKindOfClass:[NSString class]]) {
            // compatible with the string body
            body = [[options objectForKey:@"body"] dataUsingEncoding:NSUTF8StringEncoding];
        }
        if ([[options objectForKey:@"body"] isKindOfClass:[NSDictionary class]]) {
            body = [[WXUtility JSONString:[options objectForKey:@"body"]] dataUsingEncoding:NSUTF8StringEncoding];
        }
        if (!body) {
            [callbackRsp setObject:@(-1) forKey:@"status"];
            [callbackRsp setObject:@NO forKey:@"ok"];
            return nil;
        }
        
        [request setHTTPBody:body];
    }
    
    [callbackRsp setObject:@{ @"OPENED": @1 } forKey:@"readyState"];
    
    return request;
}

- (void)_loadFailedWithError:(NSError*)error callbackRsp:(NSMutableDictionary*)callbackRsp
{
    [callbackRsp removeObjectForKey:@"readyState"];
    [callbackRsp removeObjectForKey:@"length"];
    [callbackRsp removeObjectForKey:@"keepalive"];
    [callbackRsp removeObjectForKey:@"responseType"];
    
    [callbackRsp setObject:@(-1) forKey:@"status"];
    [callbackRsp setObject:[NSString stringWithFormat:@"%@(%ld)",[error localizedDescription], (long)[error code]] forKey:@"data"];
    NSString * statusText = @"";
    
    switch ([error code]) {
        case -1000:
        case -1002:
        case -1003:
            statusText = @"ERR_INVALID_REQUEST";
            break;
        default:
            break;
    }
    [callbackRsp setObject:statusText forKey:@"statusText"];
    
}
- (void)_loadFinishWithResponse:(WXResourceResponse*)response data:(NSData*)data callbackRsp:(NSMutableDictionary*)callbackRsp
{
    [callbackRsp removeObjectForKey:@"readyState"];
    [callbackRsp removeObjectForKey:@"length"];
    [callbackRsp removeObjectForKey:@"keepalive"];
    
    [callbackRsp setObject:((NSHTTPURLResponse*)response).statusCode >= 200 && ((NSHTTPURLResponse*)response).statusCode <= 299 ? @YES : @NO forKey:@"ok"];
    
    NSString *responseData = [self _stringfromData:data encode:((NSHTTPURLResponse*)response).textEncodingName];
    NSString * responseType = [callbackRsp objectForKey:@"responseType"];
    [callbackRsp removeObjectForKey:@"responseType"];
    if ([responseType isEqualToString:@"json"] || [responseType isEqualToString:@"jsonp"]) {
        //handle json format
        if ([responseType isEqualToString:@"jsonp"]) {
            //TODO: to be more elegant
            NSUInteger start = [responseData rangeOfString:@"("].location + 1 ;
            NSUInteger end = [responseData rangeOfString:@")" options:NSBackwardsSearch].location;
            if (end < [responseData length] && end > start) {
                responseData = [responseData substringWithRange:NSMakeRange(start, end-start)];
            }
        }
        id jsonObj = [self _JSONObjFromData:[responseData dataUsingEncoding:NSUTF8StringEncoding]];
        if (jsonObj) {
            [callbackRsp setObject:jsonObj forKey:@"data"];
        }
        
    } else {
        // return original Data
        if (responseData) {
            [callbackRsp setObject:responseData forKey:@"data"];
        }
    }
}

- (NSString*)_stringfromData:(NSData *)data encode:(NSString *)encoding
{
    NSMutableString *responseData = nil;
    if (data) {
        if (!encoding) {
            encoding = @"utf-8";
        }
        CFStringEncoding cfStrEncoding = CFStringConvertIANACharSetNameToEncoding((CFStringRef)encoding);
        if (cfStrEncoding == kCFStringEncodingInvalidId) {
            WXLogError(@"not supported encode");
        } else {
            NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(cfStrEncoding);
            responseData = [[NSMutableString alloc]initWithData:data encoding:encoding];
        }
    }
    return responseData;
}

- (id)_JSONObjFromData:(NSData *)data
{
    NSError * error = nil;
    id jsonObj = [WXUtility JSONObject:data error:&error];
    if (error) {
        WXLogDebug(@"%@", [error description]);
    }
    return jsonObj;
}

+ (NSString*)_getStatusText:(NSInteger)code
{    
    switch (code) {
        case -1:
            return @"ERR_INVALID_REQUEST";
        case 100:
            return @"Continue";
            break;
        case 101:
            return @"Switching Protocol";
        case 102:
            return @"Processing";
            
        case 200:
            return @"OK";
        case 201:
            return @"Created";
        case 202:
            return @"Accepted";
        case 203:
            return @"Non-Authoritative Information";
        case 204:
            return @"No Content";
        case 205:
            return @" Reset Content";
        case 206:
            return @"Partial Content";
        case 207:
            return @"Multi-Status";
        case 208:
            return @"Already Reported";
        case 226:
            return @"IM Used";
            
        case 300:
            return @"Multiple Choices";
        case 301:
            return @"Moved Permanently";
        case 302:
            return @"Found";
        case 303:
            return @"See Other";
        case 304:
            return @"Not Modified";
        case 305:
            return @"Use Proxy";
        case 306:
            return @"Switch Proxy";
        case 307:
            return @"Temporary Redirect";
        case 308:
            return @"Permanent Redirect";
            
        case 400:
            return @"Bad Request";
        case 401:
            return @"Unauthorized";
        case 402:
            return @"Payment Required";
        case 403:
            return @"Forbidden";
        case 404:
            return @"Not Found";
        case 405:
            return @"Method Not Allowed";
        case 406:
            return @"Not Acceptable";
        case 407:
            return @"Proxy Authentication Required";
        case 408:
            return @"Request Timeout";
        case 409:
            return @"Conflict";
        case 410:
            return @"Gone";
        case 411:
            return @"Length Required";
        case 412:
            return @"Precondition Failed";
        case 413:
            return @"Payload Too Large";
        case 414:
            return @"URI Too Long";
        case 415:
            return @"Unsupported Media Type";
        case 416:
            return @"Range Not Satisfiable";
        case 417:
            return @"Expectation Failed";
        case 418:
            return @"I'm a teapot";
        case 421:
            return @"Misdirected Request";
        case 422:
            return @"Unprocessable Entity";
        case 423:
            return @"Locked";
        case 424:
            return @"Failed Dependency";
        case 426:
            return @"Upgrade Required";
        case 428:
            return @"Precondition Required";
        case 429:
            return @"Too Many Requests";
        case 431:
            return @"Request Header Fields Too Large";
        case 451:
            return @"Unavailable For Legal Reasons";
            
        case 500:
            return @"Internal Server Error";
        case 501:
            return @"Not Implemented";
        case 502:
            return @"Bad Gateway";
        case 503:
            return @"Service Unavailable";
        case 504:
            return @"Gateway Timeout";
        case 505:
            return @"HTTP Version Not Supported";
        case 506:
            return @"Variant Also Negotiates";
        case 507:
            return @"Insufficient Storage";
        case 508:
            return @"Loop Detected";
        case 510:
            return @"Not Extended";
        case 511:
            return @"Network Authentication Required";
        default:
            break;
    }
    
    return @"Unknown";
}

#pragma mark - Deprecated

- (void)sendHttp:(NSDictionary*)param callback:(WXModuleKeepAliveCallback)callback
{
    NSString* method = [param objectForKey:@"method"];
    NSString* urlStr = [param objectForKey:@"url"];
    NSDictionary* headers = [param objectForKey:@"header"];
    NSString* body = [param objectForKey:@"body"];
    
    NSURL *url = [NSURL URLWithString:urlStr];
    
    //TODO:referrer
    WXResourceRequest *request = [WXResourceRequest requestWithURL:url resourceType:WXResourceTypeOthers referrer:nil cachePolicy:NSURLRequestUseProtocolCachePolicy];
    request.HTTPMethod = method;
    request.timeoutInterval = 60.0;
    request.userAgent = [WXUtility userAgent];
    
    for (NSString *key in headers) {
        NSString *value = [headers objectForKey:key];
        [request setValue:value forHTTPHeaderField:key];
    }
    [request setHTTPBody:[body dataUsingEncoding:NSUTF8StringEncoding]];
    
    WXResourceLoader *loader = [[WXResourceLoader alloc] initWithRequest:request];
    loader.onFinished = ^(const WXResourceResponse * response, NSData *data) {
        NSString* responseData = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        callback(responseData,NO);
    };
    
    loader.onFailed = ^(NSError *error) {
        if (callback) {
            callback(nil,NO);
        }
    };

    [loader start];
}

@end
