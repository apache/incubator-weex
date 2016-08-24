//
//  NSManagedObject+WXRuntimePropertyDescriptor.m
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 8/7/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import <objc/runtime.h>

#import "WXDefinitions.h"
#import "WXRuntimeDomainController.h"
#import "WXRuntimeTypes.h"
#import "NSManagedObject+WXRuntimePropertyDescriptor.h"
#import "NSObject+WXRuntimePropertyDescriptor.h"


@implementation NSManagedObject (WXRuntimePropertyDescriptor)

- (NSArray *)WX_propertiesForPropertyDescriptors;
{
    NSMutableArray *properties = (NSMutableArray *)[super WX_propertiesForPropertyDescriptors];
    if (properties) {
        // NSObject's implementation will catch the @property definitions, so replace those with the
        // NSPropertyDescription instance.
        for (NSPropertyDescription *propertyDescription in self.entity.properties) {
            if ([properties containsObject:propertyDescription.name]) {
                [properties removeObject:propertyDescription.name];
            }
        }

        return [properties arrayByAddingObjectsFromArray:self.entity.properties];
    }
    
    return self.entity.properties;
}

- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForPropertyObject:(NSObject *)property;
{
    WXRuntimePropertyDescriptor *descriptor = [super WX_propertyDescriptorForPropertyObject:property];
    if (!descriptor) {
        if ([property isKindOfClass:[NSPropertyDescription class]]) {
            descriptor = [self WX_propertyDescriptorForPropertyDescription:(NSPropertyDescription *)property];
        }
    }
    
    return descriptor;
}

- (WXRuntimeRemoteObject *)WX_propertyDescriptorValueForPropertyDescription:(NSPropertyDescription *)property;
{
    id value = [self valueForKey:property.name];
    
    WXRuntimeRemoteObject *remoteValueObject = [self WX_propertyDescriptorValueForObject:value];

    NSDictionary *propertyTypeDetails = WXRemoteObjectPropertyTypeDetailsForPropertyDescription(property);
    remoteValueObject.type = [propertyTypeDetails objectForKey:@"type"];
    remoteValueObject.subtype = [propertyTypeDetails objectForKey:@"subtype"];
    
    if ([property isKindOfClass:[NSAttributeDescription class]]) {
        remoteValueObject.classNameString = ((NSAttributeDescription *)property).attributeValueClassName;
    } else if ([property isKindOfClass:[NSRelationshipDescription class]]) {
        NSRelationshipDescription *relationshipProperty = (NSRelationshipDescription *)property;
        remoteValueObject.classNameString = relationshipProperty.destinationEntity.name;
        
        if ([relationshipProperty isToMany]) {
            remoteValueObject.objectDescription = [NSString stringWithFormat:@"%@<%@> <count = %@>", [value class], remoteValueObject.classNameString, @([value count])];
        } else {
            remoteValueObject.objectDescription = remoteValueObject.classNameString;
        }
    }
    
    // Core Data doesn't seem to do [NSNumber numberWithBool:] when initializing boolean NSNumbers, so do it manually.
    if ([remoteValueObject.type isEqualToString:@"boolean"]) {
        remoteValueObject.value = [NSNumber numberWithBool:[value boolValue]];
    }
    
    if (!value) {
        value = [NSNull null];
        remoteValueObject.type = @"object";
        remoteValueObject.subtype = @"null";
    }
    
    return remoteValueObject;
}

- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForPropertyDescription:(NSPropertyDescription *)property;
{
    WXRuntimePropertyDescriptor *descriptor = [[WXRuntimePropertyDescriptor alloc] init];

    descriptor.name = property.name;
    descriptor.value = [self WX_propertyDescriptorValueForPropertyDescription:property];
    descriptor.writable = [NSNumber numberWithBool:NO];
    descriptor.configurable = [NSNumber numberWithBool:NO];
    descriptor.wasThrown = [NSNumber numberWithBool:NO];
    descriptor.enumerable = [NSNumber numberWithBool:YES];
    
    return descriptor;
}


@end
