//
//  WXNetworkDomain.h
//  PonyDebuggerDerivedSources
//
//  Generated on 8/23/12
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXObject.h"
#import "WXDebugger.h"
#import "WXDynamicDebuggerDomain.h"

@class WXNetworkWebSocketResponse;
@class WXNetworkRequest;
@class WXNetworkWebSocketRequest;
@class WXNetworkResponse;
@class WXNetworkInitiator;
@class WXNetworkCachedResource;
@class WXNetworkWebSocketFrame;

@protocol WXNetworkCommandDelegate;

// Network domain allows tracking network activities of the page. It exposes information about http, file, data and other requests and responses, their headers, bodies, timing, etc.
@interface WXNetworkDomain : WXDynamicDebuggerDomain 

@property (nonatomic, assign) id <WXNetworkCommandDelegate, WXCommandDelegate> delegate;

// Events

// Fired when page is about to send HTTP request.
// Param requestId: Request identifier.
// Param frameId: Frame identifier.
// Param loaderId: Loader identifier.
// Param documentURL: URL of the document this request is loaded for.
// Param request: Request data.
// Param timestamp: Timestamp.
// Param initiator: Request initiator.
// Param redirectResponse: Redirect response data.
- (void)requestWillBeSentWithRequestId:(NSString *)requestId frameId:(NSString *)frameId loaderId:(NSString *)loaderId documentURL:(NSString *)documentURL request:(WXNetworkRequest *)request timestamp:(NSNumber *)timestamp initiator:(WXNetworkInitiator *)initiator redirectResponse:(WXNetworkResponse *)redirectResponse;

// Fired if request ended up loading from cache.
// Param requestId: Request identifier.
- (void)requestServedFromCacheWithRequestId:(NSString *)requestId;

// Fired when HTTP response is available.
// Param requestId: Request identifier.
// Param frameId: Frame identifier.
// Param loaderId: Loader identifier.
// Param timestamp: Timestamp.
// Param type: Resource type.
// Param response: Response data.
- (void)responseReceivedWithRequestId:(NSString *)requestId frameId:(NSString *)frameId loaderId:(NSString *)loaderId timestamp:(NSNumber *)timestamp type:(NSString *)type response:(WXNetworkResponse *)response;

// Fired when data chunk was received over the network.
// Param requestId: Request identifier.
// Param timestamp: Timestamp.
// Param dataLength: Data chunk length.
// Param encodedDataLength: Actual bytes received (might be less than dataLength for compressed encodings).
- (void)dataReceivedWithRequestId:(NSString *)requestId timestamp:(NSNumber *)timestamp dataLength:(NSNumber *)dataLength encodedDataLength:(NSNumber *)encodedDataLength;

// Fired when HTTP request has finished loading.
// Param requestId: Request identifier.
// Param timestamp: Timestamp.
- (void)loadingFinishedWithRequestId:(NSString *)requestId timestamp:(NSNumber *)timestamp;

// Fired when HTTP request has failed to load.
// Param requestId: Request identifier.
// Param timestamp: Timestamp.
// Param errorText: User friendly error message.
// Param canceled: True if loading was canceled.
- (void)loadingFailedWithRequestId:(NSString *)requestId timestamp:(NSNumber *)timestamp errorText:(NSString *)errorText canceled:(NSNumber *)canceled;

// Fired when HTTP request has been served from memory cache.
// Param requestId: Request identifier.
// Param frameId: Frame identifier.
// Param loaderId: Loader identifier.
// Param documentURL: URL of the document this request is loaded for.
// Param timestamp: Timestamp.
// Param initiator: Request initiator.
// Param resource: Cached resource data.
- (void)requestServedFromMemoryCacheWithRequestId:(NSString *)requestId frameId:(NSString *)frameId loaderId:(NSString *)loaderId documentURL:(NSString *)documentURL timestamp:(NSNumber *)timestamp initiator:(WXNetworkInitiator *)initiator resource:(WXNetworkCachedResource *)resource;

// Fired when WebSocket is about to initiate handshake.
// Param requestId: Request identifier.
// Param timestamp: Timestamp.
// Param request: WebSocket request data.
- (void)webSocketWillSendHandshakeRequestWithRequestId:(NSString *)requestId timestamp:(NSNumber *)timestamp request:(WXNetworkWebSocketRequest *)request;

// Fired when WebSocket handshake response becomes available.
// Param requestId: Request identifier.
// Param timestamp: Timestamp.
// Param response: WebSocket response data.
- (void)webSocketHandshakeResponseReceivedWithRequestId:(NSString *)requestId timestamp:(NSNumber *)timestamp response:(WXNetworkWebSocketResponse *)response;

// Fired upon WebSocket creation.
// Param requestId: Request identifier.
// Param url: WebSocket request URL.
- (void)webSocketCreatedWithRequestId:(NSString *)requestId url:(NSString *)url;

// Fired when WebSocket is closed.
// Param requestId: Request identifier.
// Param timestamp: Timestamp.
- (void)webSocketClosedWithRequestId:(NSString *)requestId timestamp:(NSNumber *)timestamp;

// Fired when WebSocket frame is received.
// Param requestId: Request identifier.
// Param timestamp: Timestamp.
// Param response: WebSocket response data.
- (void)webSocketFrameReceivedWithRequestId:(NSString *)requestId timestamp:(NSNumber *)timestamp response:(WXNetworkWebSocketFrame *)response;

// Fired when WebSocket frame error occurs.
// Param requestId: Request identifier.
// Param timestamp: Timestamp.
// Param errorMessage: WebSocket frame error message.
- (void)webSocketFrameErrorWithRequestId:(NSString *)requestId timestamp:(NSNumber *)timestamp errorMessage:(NSString *)errorMessage;

// Fired when WebSocket frame is sent.
// Param requestId: Request identifier.
// Param timestamp: Timestamp.
// Param response: WebSocket response data.
- (void)webSocketFrameSentWithRequestId:(NSString *)requestId timestamp:(NSNumber *)timestamp response:(WXNetworkWebSocketFrame *)response;

@end

@protocol WXNetworkCommandDelegate <WXCommandDelegate>
@optional

// Enables network tracking, network events will now be delivered to the client.
- (void)domain:(WXNetworkDomain *)domain enableWithCallback:(void (^)(id error))callback;

// Disables network tracking, prevents network events from being sent to the client.
- (void)domain:(WXNetworkDomain *)domain disableWithCallback:(void (^)(id error))callback;

// Allows overriding user agent with the given string.
// Param userAgent: User agent to use.
- (void)domain:(WXNetworkDomain *)domain setUserAgentOverrideWithUserAgent:(NSString *)userAgent callback:(void (^)(id error))callback;

// Specifies whether to always send extra HTTP headers with the requests from this page.
// Param headers: Map with extra HTTP headers.
- (void)domain:(WXNetworkDomain *)domain setExtraHTTPHeadersWithHeaders:(NSDictionary *)headers callback:(void (^)(id error))callback;

// Returns content served for the given request.
// Param requestId: Identifier of the network request to get content for.
// Callback Param body: Response body.
// Callback Param base64Encoded: True, if content was sent as base64.
- (void)domain:(WXNetworkDomain *)domain getResponseBodyWithRequestId:(NSString *)requestId callback:(void (^)(NSString *body, NSNumber *base64Encoded, id error))callback;

// Tells whether clearing browser cache is supported.
// Callback Param result: True if browser cache can be cleared.
- (void)domain:(WXNetworkDomain *)domain canClearBrowserCacheWithCallback:(void (^)(NSNumber *result, id error))callback;

// Clears browser cache.
- (void)domain:(WXNetworkDomain *)domain clearBrowserCacheWithCallback:(void (^)(id error))callback;

// Tells whether clearing browser cookies is supported.
// Callback Param result: True if browser cookies can be cleared.
- (void)domain:(WXNetworkDomain *)domain canClearBrowserCookiesWithCallback:(void (^)(NSNumber *result, id error))callback;

// Clears browser cookies.
- (void)domain:(WXNetworkDomain *)domain clearBrowserCookiesWithCallback:(void (^)(id error))callback;

// Toggles ignoring cache for each request. If <code>true</code>, cache will not be used.
// Param cacheDisabled: Cache disabled state.
- (void)domain:(WXNetworkDomain *)domain setCacheDisabledWithCacheDisabled:(NSNumber *)cacheDisabled callback:(void (^)(id error))callback;

@end

@interface WXDebugger (WXNetworkDomain)

@property (nonatomic, readonly, strong) WXNetworkDomain *networkDomain;

@end
