//
//  NSSet+WXRuntimePropertyDescriptor.m
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 2013-02-03.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "NSSet+WXRuntimePropertyDescriptor.h"
#import "NSObject+WXRuntimePropertyDescriptor.h"

#import "WXContainerIndex.h"
#import "WXRuntimeTypes.h"
#import "WXRuntimeDomainController.h"


#pragma mark - Implementation

@implementation NSSet (WXRuntimePropertyDescriptor)

- (id)WX_objectAtIndex:(NSUInteger)index;
{
    return [[self WX_sortedArrayRepresentation] objectAtIndex:index];
}

/**
 * Use this to get a consistent array ordering. Just sort by pointer value, since this won't change, and is 
 * cheap to access.
 */
- (NSArray *)WX_sortedArrayRepresentation;
{
    return [[self allObjects] sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        if ((int)obj1 > (int)obj2) {
            return NSOrderedDescending;
        } else if ((int)obj1 < (int)obj2) {
            return NSOrderedAscending;
        }

        return NSOrderedSame;
    }];
}

/**
 * valueForKey behaves differently for NSArray/NSSet, so define our own behaviour to actually grab the 
 * array's value for a key.
 */
- (id)WX_valueForKey:(NSString *)key;
{
    if ([key isEqualToString:@"class"]) {
        return NSStringFromClass([self class]);
    } else if ([key isEqualToString:@"count"]) {
        return @(self.count);
    }

    return nil;
}

- (NSArray *)WX_propertiesForPropertyDescriptors;
{
    NSMutableArray *properties = [[NSMutableArray alloc] initWithCapacity:self.count];
    
    for (NSInteger index = 0; index < self.count; ++index) {
        NSString *name = [NSString stringWithFormat:@"%ld", (long)index];
        
        WXContainerIndex *containerIndex = [[WXContainerIndex alloc] initWithName:name index:index];
        [properties addObject:containerIndex];
    }

    [properties addObject:@"class"];
    [properties addObject:@"count"];

    return properties;
}

- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForPropertyObject:(NSObject *)property;
{
    WXRuntimePropertyDescriptor *descriptor = [super WX_propertyDescriptorForPropertyObject:property];
    if (!descriptor) {
        if ([property isKindOfClass:[WXContainerIndex class]]) {
            descriptor = [self WX_propertyDescriptorForSetContainerIndex:(WXContainerIndex *)property];
        }
    }
    
    return descriptor;
}

- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForSetContainerIndex:(WXContainerIndex *)containerIndex;
{
    WXRuntimePropertyDescriptor *descriptor = [[WXRuntimePropertyDescriptor alloc] init];
    
    id propertyValue = [self WX_objectAtIndex:containerIndex.index];
    
    WXRuntimeRemoteObject *remoteValueObject = [[WXRuntimeRemoteObject alloc] init];
    
    NSDictionary *propertyTypeDetails = WXRemoteObjectPropertyTypeDetailsForObject(propertyValue);
    remoteValueObject.type = [propertyTypeDetails objectForKey:@"type"];
    remoteValueObject.subtype = [propertyTypeDetails objectForKey:@"subtype"];
    remoteValueObject.classNameString = NSStringFromClass([propertyValue class]);
    if ([remoteValueObject.type isEqualToString:@"object"] && !remoteValueObject.subtype) {
        remoteValueObject.objectId = [[WXRuntimeDomainController defaultInstance] registerAndGetKeyForObject:propertyValue];
        remoteValueObject.objectDescription = remoteValueObject.classNameString;
    } else {
        remoteValueObject.value = propertyValue;
    }
    
    descriptor.name = containerIndex.name;
    descriptor.value = remoteValueObject;
    descriptor.writable = [NSNumber numberWithBool:NO];
    descriptor.configurable = [NSNumber numberWithBool:NO];
    descriptor.enumerable = [NSNumber numberWithBool:YES];
    descriptor.wasThrown = [NSNumber numberWithBool:NO];
    
    return descriptor;
}

@end
