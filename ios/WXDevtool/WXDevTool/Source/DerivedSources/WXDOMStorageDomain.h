//
//  WXDOMStorageDomain.h
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

@class WXDOMStorageEntry;

@protocol WXDOMStorageCommandDelegate;

@interface WXDOMStorageDomain : WXDynamicDebuggerDomain 

@property (nonatomic, assign) id <WXDOMStorageCommandDelegate, WXCommandDelegate> delegate;

// Events
- (void)addDOMStorageWithStorage:(WXDOMStorageEntry *)storage;
- (void)domStorageUpdatedWithStorageId:(NSString *)storageId;

@end

@protocol WXDOMStorageCommandDelegate <WXCommandDelegate>
@optional

// Enables storage tracking, storage events will now be delivered to the client.
- (void)domain:(WXDOMStorageDomain *)domain enableWithCallback:(void (^)(id error))callback;

// Disables storage tracking, prevents storage events from being sent to the client.
- (void)domain:(WXDOMStorageDomain *)domain disableWithCallback:(void (^)(id error))callback;
- (void)domain:(WXDOMStorageDomain *)domain getDOMStorageEntriesWithStorageId:(NSString *)storageId callback:(void (^)(NSArray *entries, id error))callback;
- (void)domain:(WXDOMStorageDomain *)domain setDOMStorageItemWithStorageId:(NSString *)storageId key:(NSString *)key value:(NSString *)value callback:(void (^)(NSNumber *success, id error))callback;
- (void)domain:(WXDOMStorageDomain *)domain removeDOMStorageItemWithStorageId:(NSString *)storageId key:(NSString *)key callback:(void (^)(NSNumber *success, id error))callback;

@end

@interface WXDebugger (WXDOMStorageDomain)

@property (nonatomic, readonly, strong) WXDOMStorageDomain *DOMStorageDomain;

@end
