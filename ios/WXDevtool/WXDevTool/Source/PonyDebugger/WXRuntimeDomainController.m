//
//  WXRuntimeDomainController.m
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 8/7/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXRuntimeDomainController.h"
#import "WXRuntimeTypes.h"

#import "NSObject+WXRuntimePropertyDescriptor.h"
#import "NSManagedObject+WXRuntimePropertyDescriptor.h"
#import "NSArray+WXRuntimePropertyDescriptor.h"
#import "NSSet+WXRuntimePropertyDescriptor.h"
#import "NSOrderedSet+WXRuntimePropertyDescriptor.h"
#import "NSDictionary+WXRuntimePropertyDescriptor.h"


@interface WXRuntimeDomainController () <WXRuntimeCommandDelegate>

// Dictionary where key is a unique objectId, and value is a reference of the value.
@property (nonatomic, strong) NSMutableDictionary *objectReferences;

// Values are arrays of object references.
@property (nonatomic, strong) NSMutableDictionary *objectGroups;

+ (NSString *)_generateUUID;

- (void)_releaseObjectID:(NSString *)objectID;
- (void)_releaseObjectGroup:(NSString *)objectGroup;

@end


@implementation WXRuntimeDomainController

@dynamic domain;

@synthesize objectReferences = _objectReferences;
@synthesize objectGroups = _objectGroups;

#pragma mark - Statics

+ (WXRuntimeDomainController *)defaultInstance;
{
    static WXRuntimeDomainController *defaultInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaultInstance = [[WXRuntimeDomainController alloc] init];
    });
    
    return defaultInstance;
}

+ (Class)domainClass;
{
    return [WXRuntimeDomain class];
}

+ (NSString *)_generateUUID;
{
	CFUUIDRef UUIDRef = CFUUIDCreate(nil);
    NSString *newGuid = (__bridge_transfer NSString *) CFUUIDCreateString(nil, UUIDRef);
    CFRelease(UUIDRef);
    return newGuid;
}

#pragma mark - Initialization

- (id)init;
{
    if (!(self = [super init])) {
        return nil;
    }
    
    self.objectReferences = [[NSMutableDictionary alloc] init];
    self.objectGroups = [[NSMutableDictionary alloc] init];
    
    return self;
}

- (void)dealloc;
{
    self.objectReferences = nil;
    self.objectGroups = nil;
}

#pragma mark - WXRuntimeCommandDelegate

- (void)domain:(WXRuntimeDomain *)domain getPropertiesWithObjectId:(NSString *)objectId ownProperties:(NSNumber *)ownProperties callback:(void (^)(NSArray *result, id error))callback;
{
    NSObject *object = [self.objectReferences objectForKey:objectId];
    if (!object) {
        NSString *errorMessage = [NSString stringWithFormat:@"Object with objectID '%@' does not exist.", objectId];
        NSError *error = [NSError errorWithDomain:WXDebuggerErrorDomain code:100 userInfo:[NSDictionary dictionaryWithObject:errorMessage forKey:NSLocalizedDescriptionKey]];
        
        callback(nil, error);
        return;
    }
    
    NSArray *properties = [object WX_propertyDescriptors];
    callback(properties, nil);
}

- (void)domain:(WXRuntimeDomain *)domain releaseObjectWithObjectId:(NSString *)objectId callback:(void (^)(id error))callback;
{
    callback(nil);
    
    [self _releaseObjectID:objectId];
}

- (void)domain:(WXRuntimeDomain *)domain releaseObjectGroupWithObjectGroup:(NSString *)objectGroup callback:(void (^)(id error))callback;
{
    callback(nil);
    
    [self _releaseObjectGroup:objectGroup];
}

#pragma mark - Public Methods

/**
 * Registers and returns a string associated with the object to retain.
 */
- (NSString *)registerAndGetKeyForObject:(id)object;
{
    NSString *key = [WXRuntimeDomainController _generateUUID];
    
    [self.objectReferences setObject:object forKey:key];
    
    return key;
}

/**
 * Clears object references given the string returned by registerAndGetKeyForObject:
 */
- (void)clearObjectReferencesByKey:(NSArray *)objectKeys;
{
    [self.objectReferences removeObjectsForKeys:objectKeys];
}

/**
 * Clears all object references.
 */
- (void)clearAllObjectReferences;
{
    [self.objectReferences removeAllObjects];
    [self.objectGroups removeAllObjects];
}

#pragma mark - Private Methods

- (void)_releaseObjectID:(NSString *)objectID;
{
    if (![self.objectReferences objectForKey:objectID]) {
        return;
    }
    
    [self.objectReferences removeObjectForKey:objectID];
}

- (void)_releaseObjectGroup:(NSString *)objectGroup;
{
    NSArray *objectIDs = [self.objectGroups objectForKey:objectGroup];
    if (objectIDs) {
        for (NSString *objectID in objectIDs) {
            [self _releaseObjectID:objectID];
        }
        
        [self.objectGroups removeObjectForKey:objectGroup];
    }
}

@end
