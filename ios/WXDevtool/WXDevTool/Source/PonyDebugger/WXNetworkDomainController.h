//
//  WXAFNetworkDomainController.h
//  PonyDebugger
//
//  Created by Mike Lewis on 2/27/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXDomainController.h"
#import "WXNetworkTypes.h"
#import "WXNetworkDomain.h"


@protocol WXPrettyStringPrinting;

@interface WXNetworkDomainController : WXDomainController <WXNetworkCommandDelegate>

@property (nonatomic, strong) WXNetworkDomain *domain;

+ (WXNetworkDomainController *)defaultInstance;
+ (void)injectIntoAllNSURLConnectionDelegateClasses;
+ (void)swizzleNSURLSessionClasses;
+ (void)injectIntoDelegateClass:(Class)cls;

+ (void)registerPrettyStringPrinter:(id<WXPrettyStringPrinting>)prettyStringPrinter;
+ (void)unregisterPrettyStringPrinter:(id<WXPrettyStringPrinting>)prettyStringPrinter;

- (NSCache *)getNetWorkResponseCache;

@end


@interface WXNetworkDomainController (NSURLConnectionHelpers)

- (void)connection:(NSURLConnection *)connection willSendRequest:(NSURLRequest *)request redirectResponse:(NSURLResponse *)response;
- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response;

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data;

- (void)connectionDidFinishLoading:(NSURLConnection *)connection;
- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error;

@end


@interface WXNetworkDomainController (NSURLSessionTaskHelpers)

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task willPerformHTTPRedirection:(NSHTTPURLResponse *)response newRequest:(NSURLRequest *)request;
- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask didReceiveResponse:(NSURLResponse *)response;
- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask didReceiveData:(NSData *)data;
- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error;

@end


@interface WXNetworkRequest (WXNetworkHelpers)

- (id)initWithURLRequest:(NSURLRequest *)request;
+ (WXNetworkRequest *)networkRequestWithURLRequest:(NSURLRequest *)request;

@end


@interface WXNetworkResponse (WXNetworkHelpers)

- (id)initWithURLResponse:(NSURLResponse *)response request:(NSURLRequest *)request;
+ (WXNetworkResponse *)networkResponseWithURLResponse:(NSURLResponse *)response request:(NSURLRequest *)request;

@end


@interface NSURLResponse (WXNetworkHelpers)

- (NSString *)WX_responseType;

@end
