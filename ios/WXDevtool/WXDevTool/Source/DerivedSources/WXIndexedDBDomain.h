//
//  WXIndexedDBDomain.h
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

@class WXIndexedDBDatabaseWithObjectStores;
@class WXIndexedDBSecurityOriginWithDatabaseNames;
@class WXIndexedDBKeyRange;

@protocol WXIndexedDBCommandDelegate;

@interface WXIndexedDBDomain : WXDynamicDebuggerDomain 

@property (nonatomic, assign) id <WXIndexedDBCommandDelegate, WXCommandDelegate> delegate;

// Events
// Param requestId: Request id.
// Param securityOriginWithDatabaseNames: Frame with database names.
- (void)databaseNamesLoadedWithRequestId:(NSNumber *)requestId securityOriginWithDatabaseNames:(WXIndexedDBSecurityOriginWithDatabaseNames *)securityOriginWithDatabaseNames;
// Param requestId: Request id.
// Param databaseWithObjectStores: Database with an array of object stores.
- (void)databaseLoadedWithRequestId:(NSNumber *)requestId databaseWithObjectStores:(WXIndexedDBDatabaseWithObjectStores *)databaseWithObjectStores;
// Param requestId: Request id.
// Param objectStoreDataEntries: Array of object store data entries.
// Param hasMore: If true, there are more entries to fetch in the given range.
- (void)objectStoreDataLoadedWithRequestId:(NSNumber *)requestId objectStoreDataEntries:(NSArray *)objectStoreDataEntries hasMore:(NSNumber *)hasMore;
// Param requestId: Request id.
// Param indexDataEntries: Array of index data entries.
// Param hasMore: If true, there are more entries to fetch in the given range.
- (void)indexDataLoadedWithRequestId:(NSNumber *)requestId indexDataEntries:(NSArray *)indexDataEntries hasMore:(NSNumber *)hasMore;

@end

@protocol WXIndexedDBCommandDelegate <WXCommandDelegate>
@optional

// Enables events from backend.
- (void)domain:(WXIndexedDBDomain *)domain enableWithCallback:(void (^)(id error))callback;

// Disables events from backend.
- (void)domain:(WXIndexedDBDomain *)domain disableWithCallback:(void (^)(id error))callback;

// Requests database names for given frame's security origin.
// Param requestId: Request id.
// Param frameId: Frame id.
- (void)domain:(WXIndexedDBDomain *)domain requestDatabaseNamesForFrameWithRequestId:(NSNumber *)requestId frameId:(NSString *)frameId callback:(void (^)(id error))callback;

// Requests database with given name in given frame.
// Param requestId: Request id.
// Param frameId: Frame id.
// Param databaseName: Database name.
- (void)domain:(WXIndexedDBDomain *)domain requestDatabaseWithRequestId:(NSNumber *)requestId frameId:(NSString *)frameId databaseName:(NSString *)databaseName callback:(void (^)(id error))callback;

// Requests data from object store or index.
// Param requestId: Request id.
// Param frameId: Frame id.
// Param databaseName: Database name.
// Param objectStoreName: Object store name.
// Param indexName: Index name, empty string for object store data requests.
// Param skipCount: Number of records to skip.
// Param pageSize: Number of records to fetch.
// Param keyRange: Key range.
- (void)domain:(WXIndexedDBDomain *)domain requestDataWithRequestId:(NSNumber *)requestId frameId:(NSString *)frameId databaseName:(NSString *)databaseName objectStoreName:(NSString *)objectStoreName indexName:(NSString *)indexName skipCount:(NSNumber *)skipCount pageSize:(NSNumber *)pageSize keyRange:(WXIndexedDBKeyRange *)keyRange callback:(void (^)(id error))callback;

@end

@interface WXDebugger (WXIndexedDBDomain)

@property (nonatomic, readonly, strong) WXIndexedDBDomain *indexedDBDomain;

@end
