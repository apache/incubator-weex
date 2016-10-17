//
//  WXDatabaseDomainController.m
//  PonyDebugger
//
//  Created by Mike Lewis on 2/29/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXIndexedDBDomainController.h"
#import "WXRuntimeDomainController.h"
#import "WXIndexedDBTypes.h"
#import "WXRuntimeTypes.h"

#import <CoreData/CoreData.h>


static NSString *const WXManagedObjectContextNameUserInfoKey = @"WXManagedObjectContextName";


@interface WXIndexedDBDomainController ()

@property (nonatomic, strong) NSMutableArray *managedObjectContexts;
@property (nonatomic, strong) NSNumber *rootFrameRequestID;

- (NSManagedObjectContext *)_managedObjectContextForName:(NSString *)name;
- (NSString *)_databaseNameForManagedObjectContext:(NSManagedObjectContext *)context;
- (void)_broadcastDatabaseNames;
- (void)_broadcastDatabase:(NSManagedObjectContext *)context requestID:(NSNumber *)requestID;

@end


@implementation WXIndexedDBDomainController

@dynamic domain;

@synthesize managedObjectContexts = _managedObjectContexts;
@synthesize rootFrameRequestID = _rootFrameRequestID;

#pragma mark - Statics

+ (WXIndexedDBDomainController *)defaultInstance;
{
    static WXIndexedDBDomainController *defaultInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaultInstance = [[WXIndexedDBDomainController alloc] init];
    });
    
    return defaultInstance;
}

+ (Class)domainClass;
{
    return [WXIndexedDBDomain class];
}

#pragma mark - Initialization

- (id)init;
{
    self = [super init];
    if (self) {
        self.managedObjectContexts = [[NSMutableArray alloc] init];
    }
    
    return self;
}

- (void)dealloc;
{
    self.managedObjectContexts = nil;
    self.rootFrameRequestID = nil;
}

#pragma mark - WXIndexedDBCommandDelegate

- (void)domain:(WXIndexedDBDomain *)domain requestDatabaseNamesForFrameWithRequestId:(NSNumber *)requestId frameId:(NSString *)frameId callback:(void (^)(id))callback;
{
    callback(nil);
    
    self.rootFrameRequestID = requestId;
    [self _broadcastDatabaseNames];
}

- (void)domain:(WXIndexedDBDomain *)domain requestDatabaseWithRequestId:(NSNumber *)requestId frameId:(NSString *)frameId databaseName:(NSString *)databaseName callback:(void (^)(id))callback;
{
    callback(nil);
    
    [self _broadcastDatabase:[self _managedObjectContextForName:databaseName] requestID:requestId];
}

- (void)domain:(WXIndexedDBDomain *)domain requestDataWithRequestId:(NSNumber *)requestId frameId:(NSString *)frameId databaseName:(NSString *)databaseName objectStoreName:(NSString *)objectStoreName indexName:(NSString *)indexName skipCount:(NSNumber *)skipCount pageSize:(NSNumber *)pageSize keyRange:(WXIndexedDBKeyRange *)keyRange callback:(void (^)(id))callback;
{
    callback(nil);
    
    NSManagedObjectContext *context = [self _managedObjectContextForName:databaseName];
    
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:objectStoreName];

    // Don't show subentities if it's not an abstract entity.
    NSEntityDescription *entityDescription = [NSEntityDescription entityForName:objectStoreName inManagedObjectContext:context];
    if (![entityDescription isAbstract]) {
        fetchRequest.includesSubentities = NO;
    }

    NSInteger totalCount = [context countForFetchRequest:fetchRequest error:NULL];
    
    fetchRequest.fetchOffset = [skipCount integerValue];
    fetchRequest.fetchLimit = [pageSize integerValue];
    
    NSArray *results = [context executeFetchRequest:fetchRequest error:NULL];
    NSMutableArray *dataEntries = [[NSMutableArray alloc] initWithCapacity:results.count];
    
    for (NSManagedObject *object in results) {
        WXIndexedDBDataEntry *dataEntry = [[WXIndexedDBDataEntry alloc] init];
        
        WXIndexedDBKey *primaryKey = [[WXIndexedDBKey alloc] init];
        primaryKey.type = @"string";
        primaryKey.string = [[object.objectID URIRepresentation] absoluteString];
        dataEntry.primaryKey = primaryKey;
        
        if (indexName.length > 0) {
            WXIndexedDBKey *key = [[WXIndexedDBKey alloc] init];
            key.type = @"string";
            key.string = [object valueForKey:indexName];
            dataEntry.key = key;
        } else {
            dataEntry.key = primaryKey;
        }
        
        WXRuntimeRemoteObject *remoteObject = [[WXRuntimeRemoteObject alloc] init];
        remoteObject.objectId = [[WXRuntimeDomainController defaultInstance] registerAndGetKeyForObject:object];
        remoteObject.type = @"object";
        remoteObject.classNameString = remoteObject.objectDescription = objectStoreName;
        
        dataEntry.value = remoteObject;
        
        [dataEntries addObject:dataEntry];
    }
    
    NSNumber *hasMore = [NSNumber numberWithBool:YES];
    if (fetchRequest.fetchOffset + results.count >= totalCount) {
        hasMore = [NSNumber numberWithBool:NO];
    }
    
    [self.domain objectStoreDataLoadedWithRequestId:requestId objectStoreDataEntries:dataEntries hasMore:hasMore];
}

#pragma mark - Public Methods

- (void)addManagedObjectContext:(NSManagedObjectContext *)context;
{
    NSString *name = [self _databaseNameForManagedObjectContext:context];
    [self addManagedObjectContext:context withName:name];
}

- (void)addManagedObjectContext:(NSManagedObjectContext *)context withName:(NSString *)name;
{
    [context.userInfo setObject:name forKey:WXManagedObjectContextNameUserInfoKey];
    [_managedObjectContexts addObject:context];
    
    [self _broadcastDatabaseNames];
}

- (void)removeManagedObjectContext:(NSManagedObjectContext *)context;
{
    [self.managedObjectContexts removeObject:context];
    
    [self _broadcastDatabaseNames];
}

#pragma mark - Private Methods

- (NSManagedObjectContext *)_managedObjectContextForName:(NSString *)name;
{
    for (NSManagedObjectContext *context in self.managedObjectContexts) {
        if ([[context.userInfo objectForKey:WXManagedObjectContextNameUserInfoKey] isEqualToString:name]) {
            return context;
        }
    }
    
    return nil;
}

- (NSString *)_databaseNameForManagedObjectContext:(NSManagedObjectContext *)context;
{
    NSMutableArray *paths = [[NSMutableArray alloc] init];
    for (NSPersistentStore *store in context.persistentStoreCoordinator.persistentStores) {
        NSURL *url = [context.persistentStoreCoordinator URLForPersistentStore:store];
        NSString *pathString = [url.lastPathComponent stringByDeletingPathExtension];
        if (pathString) {
            [paths addObject:pathString];
        }
    }
    
    return [paths componentsJoinedByString:@":"];
}

- (void)_broadcastDatabaseNames;
{
    NSMutableArray *dbNames = [[NSMutableArray alloc] initWithCapacity:_managedObjectContexts.count];

    WXIndexedDBSecurityOriginWithDatabaseNames *databaseNames =  [[WXIndexedDBSecurityOriginWithDatabaseNames alloc] init];
    
    databaseNames.databaseNames = dbNames;
    [self.managedObjectContexts enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSManagedObjectContext *context = obj;
        [dbNames addObject:[context.userInfo objectForKey:WXManagedObjectContextNameUserInfoKey]];
    }];
    
    databaseNames.securityOrigin = [[NSBundle mainBundle] bundleIdentifier];
    
    [self.domain databaseNamesLoadedWithRequestId:_rootFrameRequestID securityOriginWithDatabaseNames:databaseNames];
}

- (void)_broadcastDatabase:(NSManagedObjectContext *)context requestID:(NSNumber *)requestID;
{
    NSMutableArray *objectStores = [[NSMutableArray alloc] init];
    
    for (NSEntityDescription *entity in context.persistentStoreCoordinator.managedObjectModel.entities) {
        WXIndexedDBObjectStore *objectStore = [[WXIndexedDBObjectStore alloc] init];
        
        WXIndexedDBKeyPath *keyPath = [[WXIndexedDBKeyPath alloc] init];
        keyPath.type = @"string";
        keyPath.string = @"objectID";
        
        objectStore.keyPath = keyPath;
        
        NSMutableArray *indexes = [[NSMutableArray alloc] init];
        for (NSAttributeDescription *property in [[entity attributesByName] allValues]) {
            if ([property isIndexed]) {
                WXIndexedDBObjectStoreIndex *index = [[WXIndexedDBObjectStoreIndex alloc] init];
                index.name = property.name;
                
                WXIndexedDBKeyPath *guidKeyPath = [[WXIndexedDBKeyPath alloc] init];
                guidKeyPath.type = @"string";
                guidKeyPath.string = property.name;
                
                index.keyPath = guidKeyPath;
                index.unique = [NSNumber numberWithBool:NO];
                index.multiEntry = [NSNumber numberWithBool:NO];
                
                [indexes addObject:index];
            }
        }
        objectStore.indexes = indexes;
        
        objectStore.autoIncrement = [NSNumber numberWithBool:NO];
        objectStore.name = entity.name;
        
        [objectStores addObject:objectStore];
    }
       
    WXIndexedDBDatabaseWithObjectStores *db = [[WXIndexedDBDatabaseWithObjectStores alloc] init];
    
    db.name = [self _databaseNameForManagedObjectContext:context];
    db.version = @"N/A";
    db.objectStores = objectStores;
    
    [self.domain databaseLoadedWithRequestId:requestID databaseWithObjectStores:db];
}

@end
