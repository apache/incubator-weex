//
//  WXDatabaseDomain.h
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

@class WXDatabaseDatabase;

@protocol WXDatabaseCommandDelegate;

@interface WXDatabaseDomain : WXDynamicDebuggerDomain 

@property (nonatomic, assign) id <WXDatabaseCommandDelegate, WXCommandDelegate> delegate;

// Events
- (void)addDatabaseWithDatabase:(WXDatabaseDatabase *)database;
- (void)sqlTransactionSucceededWithTransactionId:(NSNumber *)transactionId columnNames:(NSArray *)columnNames values:(NSArray *)values;
- (void)sqlTransactionFailedWithTransactionId:(NSNumber *)transactionId sqlError:(NSDictionary *)sqlError;

@end

@protocol WXDatabaseCommandDelegate <WXCommandDelegate>
@optional

// Enables database tracking, database events will now be delivered to the client.
- (void)domain:(WXDatabaseDomain *)domain enableWithCallback:(void (^)(id error))callback;

// Disables database tracking, prevents database events from being sent to the client.
- (void)domain:(WXDatabaseDomain *)domain disableWithCallback:(void (^)(id error))callback;
- (void)domain:(WXDatabaseDomain *)domain getDatabaseTableNamesWithDatabaseId:(NSString *)databaseId callback:(void (^)(NSArray *tableNames, id error))callback;
- (void)domain:(WXDatabaseDomain *)domain executeSQLWithDatabaseId:(NSString *)databaseId query:(NSString *)query callback:(void (^)(NSNumber *success, NSNumber *transactionId, id error))callback;

@end

@interface WXDebugger (WXDatabaseDomain)

@property (nonatomic, readonly, strong) WXDatabaseDomain *databaseDomain;

@end
