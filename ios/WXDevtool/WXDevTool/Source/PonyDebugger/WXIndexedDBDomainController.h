//
//  WXDatabaseDomainController.h
//  PonyDebugger
//
//  Created by Mike Lewis on 2/29/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXDomainController.h"
#import "WXIndexedDBDomain.h"
#import "WXIndexedDBTypes.h"

@class NSManagedObjectContext;

@interface WXIndexedDBDomainController : WXDomainController <WXIndexedDBCommandDelegate>

@property (nonatomic, strong) WXIndexedDBDomain *domain;

+ (WXIndexedDBDomainController *)defaultInstance;

- (void)addManagedObjectContext:(NSManagedObjectContext *)context;
- (void)addManagedObjectContext:(NSManagedObjectContext *)context withName:(NSString *)name;

- (void)removeManagedObjectContext:(NSManagedObjectContext *)context;

@end
