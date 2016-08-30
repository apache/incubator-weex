//
//  WXIndexedDBDomain.m
//  PonyDebuggerDerivedSources
//
//  Generated on 8/23/12
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXObject.h"
#import "WXIndexedDBDomain.h"
#import "WXObject.h"
#import "WXIndexedDBTypes.h"


@interface WXIndexedDBDomain ()
//Commands

@end

@implementation WXIndexedDBDomain

@dynamic delegate;

+ (NSString *)domainName;
{
    return @"IndexedDB";
}

// Events
- (void)databaseNamesLoadedWithRequestId:(NSNumber *)requestId securityOriginWithDatabaseNames:(WXIndexedDBSecurityOriginWithDatabaseNames *)securityOriginWithDatabaseNames;
{
    NSMutableDictionary *params = [[NSMutableDictionary alloc] initWithCapacity:2];

    if (requestId != nil) {
        [params setObject:[requestId WX_JSONObject] forKey:@"requestId"];
    }
    if (securityOriginWithDatabaseNames != nil) {
        [params setObject:[securityOriginWithDatabaseNames WX_JSONObject] forKey:@"securityOriginWithDatabaseNames"];
    }
    
    [self.debuggingServer sendEventWithName:@"IndexedDB.databaseNamesLoaded" parameters:params];
}
- (void)databaseLoadedWithRequestId:(NSNumber *)requestId databaseWithObjectStores:(WXIndexedDBDatabaseWithObjectStores *)databaseWithObjectStores;
{
    NSMutableDictionary *params = [[NSMutableDictionary alloc] initWithCapacity:2];

    if (requestId != nil) {
        [params setObject:[requestId WX_JSONObject] forKey:@"requestId"];
    }
    if (databaseWithObjectStores != nil) {
        [params setObject:[databaseWithObjectStores WX_JSONObject] forKey:@"databaseWithObjectStores"];
    }
    
    [self.debuggingServer sendEventWithName:@"IndexedDB.databaseLoaded" parameters:params];
}
- (void)objectStoreDataLoadedWithRequestId:(NSNumber *)requestId objectStoreDataEntries:(NSArray *)objectStoreDataEntries hasMore:(NSNumber *)hasMore;
{
    NSMutableDictionary *params = [[NSMutableDictionary alloc] initWithCapacity:3];

    if (requestId != nil) {
        [params setObject:[requestId WX_JSONObject] forKey:@"requestId"];
    }
    if (objectStoreDataEntries != nil) {
        [params setObject:[objectStoreDataEntries WX_JSONObject] forKey:@"objectStoreDataEntries"];
    }
    if (hasMore != nil) {
        [params setObject:[hasMore WX_JSONObject] forKey:@"hasMore"];
    }
    
    [self.debuggingServer sendEventWithName:@"IndexedDB.objectStoreDataLoaded" parameters:params];
}
- (void)indexDataLoadedWithRequestId:(NSNumber *)requestId indexDataEntries:(NSArray *)indexDataEntries hasMore:(NSNumber *)hasMore;
{
    NSMutableDictionary *params = [[NSMutableDictionary alloc] initWithCapacity:3];

    if (requestId != nil) {
        [params setObject:[requestId WX_JSONObject] forKey:@"requestId"];
    }
    if (indexDataEntries != nil) {
        [params setObject:[indexDataEntries WX_JSONObject] forKey:@"indexDataEntries"];
    }
    if (hasMore != nil) {
        [params setObject:[hasMore WX_JSONObject] forKey:@"hasMore"];
    }
    
    [self.debuggingServer sendEventWithName:@"IndexedDB.indexDataLoaded" parameters:params];
}



- (void)handleMethodWithName:(NSString *)methodName parameters:(NSDictionary *)params responseCallback:(WXResponseCallback)responseCallback;
{
    if ([methodName isEqualToString:@"enable"] && [self.delegate respondsToSelector:@selector(domain:enableWithCallback:)]) {
        [self.delegate domain:self enableWithCallback:^(id error) {
            responseCallback(nil, error);
        }];
    } else if ([methodName isEqualToString:@"disable"] && [self.delegate respondsToSelector:@selector(domain:disableWithCallback:)]) {
        [self.delegate domain:self disableWithCallback:^(id error) {
            responseCallback(nil, error);
        }];
    } else if ([methodName isEqualToString:@"requestDatabaseNamesForFrame"] && [self.delegate respondsToSelector:@selector(domain:requestDatabaseNamesForFrameWithRequestId:frameId:callback:)]) {
        [self.delegate domain:self requestDatabaseNamesForFrameWithRequestId:[params objectForKey:@"requestId"] frameId:[params objectForKey:@"frameId"] callback:^(id error) {
            responseCallback(nil, error);
        }];
    } else if ([methodName isEqualToString:@"requestDatabase"] && [self.delegate respondsToSelector:@selector(domain:requestDatabaseWithRequestId:frameId:databaseName:callback:)]) {
        [self.delegate domain:self requestDatabaseWithRequestId:[params objectForKey:@"requestId"] frameId:[params objectForKey:@"frameId"] databaseName:[params objectForKey:@"databaseName"] callback:^(id error) {
            responseCallback(nil, error);
        }];
    } else if ([methodName isEqualToString:@"requestData"] && [self.delegate respondsToSelector:@selector(domain:requestDataWithRequestId:frameId:databaseName:objectStoreName:indexName:skipCount:pageSize:keyRange:callback:)]) {
        [self.delegate domain:self requestDataWithRequestId:[params objectForKey:@"requestId"] frameId:[params objectForKey:@"frameId"] databaseName:[params objectForKey:@"databaseName"] objectStoreName:[params objectForKey:@"objectStoreName"] indexName:[params objectForKey:@"indexName"] skipCount:[params objectForKey:@"skipCount"] pageSize:[params objectForKey:@"pageSize"] keyRange:[params objectForKey:@"keyRange"] callback:^(id error) {
            responseCallback(nil, error);
        }];
    } else {
        [super handleMethodWithName:methodName parameters:params responseCallback:responseCallback];
    }
}

@end


@implementation WXDebugger (WXIndexedDBDomain)

- (WXIndexedDBDomain *)indexedDBDomain;
{
    return [self domainForName:@"IndexedDB"];
}

@end
