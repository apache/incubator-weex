//
//  WXApplicationCacheDomain.h
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

@class WXApplicationCacheApplicationCache;

@protocol WXApplicationCacheCommandDelegate;

@interface WXApplicationCacheDomain : WXDynamicDebuggerDomain 

@property (nonatomic, assign) id <WXApplicationCacheCommandDelegate, WXCommandDelegate> delegate;

// Events
// Param frameId: Identifier of the frame containing document whose application cache updated status.
// Param manifestURL: Manifest URL.
// Param status: Updated application cache status.
- (void)applicationCacheStatusUpdatedWithFrameId:(NSString *)frameId manifestURL:(NSString *)manifestURL status:(NSNumber *)status;
- (void)networkStateUpdatedWithIsNowOnline:(NSNumber *)isNowOnline;

@end

@protocol WXApplicationCacheCommandDelegate <WXCommandDelegate>
@optional

// Returns array of frame identifiers with manifest urls for each frame containing a document associated with some application cache.
// Callback Param frameIds: Array of frame identifiers with manifest urls for each frame containing a document associated with some application cache.
- (void)domain:(WXApplicationCacheDomain *)domain getFramesWithManifestsWithCallback:(void (^)(NSArray *frameIds, id error))callback;

// Enables application cache domain notifications.
- (void)domain:(WXApplicationCacheDomain *)domain enableWithCallback:(void (^)(id error))callback;

// Returns manifest URL for document in the given frame.
// Param frameId: Identifier of the frame containing document whose manifest is retrieved.
// Callback Param manifestURL: Manifest URL for document in the given frame.
- (void)domain:(WXApplicationCacheDomain *)domain getManifestForFrameWithFrameId:(NSString *)frameId callback:(void (^)(NSString *manifestURL, id error))callback;

// Returns relevant application cache data for the document in given frame.
// Param frameId: Identifier of the frame containing document whose application cache is retrieved.
// Callback Param applicationCache: Relevant application cache data for the document in given frame.
- (void)domain:(WXApplicationCacheDomain *)domain getApplicationCacheForFrameWithFrameId:(NSString *)frameId callback:(void (^)(WXApplicationCacheApplicationCache *applicationCache, id error))callback;

@end

@interface WXDebugger (WXApplicationCacheDomain)

@property (nonatomic, readonly, strong) WXApplicationCacheDomain *applicationCacheDomain;

@end
