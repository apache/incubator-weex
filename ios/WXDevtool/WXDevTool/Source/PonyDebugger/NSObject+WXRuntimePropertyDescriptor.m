//
//  NSObject+WXRuntimePropertyDescriptor.m
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 8/7/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "NSObject+WXRuntimePropertyDescriptor.h"

#import "WXDefinitions.h"
#import "WXRuntimeDomainController.h"
#import "WXRuntimeTypes.h"


#pragma mark - Definitions

NSDictionary *WXExtractPropertyAttributes(objc_property_t property);


#pragma mark - Implementation

@implementation NSObject (WXRuntimePropertyDescriptor)

#pragma mark - Class Methods

+ (WXRuntimeRemoteObject *)WX_remoteObjectRepresentationForObject:(id)object;
{
    id propertyValue = object;
    if ([propertyValue class] == propertyValue) {
        propertyValue = NSStringFromClass(propertyValue);
    }
    
    NSDictionary *propertyTypeDetails = WXRemoteObjectPropertyTypeDetailsForObject(propertyValue);
    
    WXRuntimeRemoteObject *remoteValueObject = [[WXRuntimeRemoteObject alloc] init];
    remoteValueObject.type = [propertyTypeDetails objectForKey:@"type"];
    remoteValueObject.subtype = [propertyTypeDetails objectForKey:@"subtype"];
    remoteValueObject.classNameString = NSStringFromClass([propertyValue class]);

    if ([remoteValueObject.type isEqualToString:@"object"]) {
        if (propertyValue) {
            if (!remoteValueObject.subtype) {
                remoteValueObject.objectId = [[WXRuntimeDomainController defaultInstance] registerAndGetKeyForObject:propertyValue];
            } else if ([remoteValueObject.subtype isEqualToString:@"array"]) {
                remoteValueObject.objectId = [[WXRuntimeDomainController defaultInstance] registerAndGetKeyForObject:propertyValue];
                remoteValueObject.objectDescription = [NSString stringWithFormat:@"%@ <count = %@>", [propertyValue class], @([propertyValue count])];
            } else if ([remoteValueObject.subtype isEqualToString:@"date"]) {
                NSDate *date = propertyValue;
                remoteValueObject.value = [date description];
                remoteValueObject.objectDescription = @"";
            } else if ([remoteValueObject.subtype isEqualToString:@"regexp"]) {
                NSRegularExpression *regEx = propertyValue;
                remoteValueObject.value = regEx.pattern;
                remoteValueObject.objectDescription = @"";
            }
            
            if (!remoteValueObject.objectDescription) {
                remoteValueObject.objectDescription = [NSString stringWithFormat:@"%@ %@", [propertyValue class], [propertyValue description]];
            }
        }
    } else {
        remoteValueObject.value = propertyValue;
    }
    
    if (!propertyValue) {
        remoteValueObject.value = [NSNull null];
        remoteValueObject.type = @"object";
        remoteValueObject.subtype = @"null";
    }
    
    return remoteValueObject;
}

#pragma mark - Instance Methods

- (id)WX_valueForKey:(NSString *)key;
{
    return [self valueForKey:key];
}

- (NSArray *)WX_propertiesForPropertyDescriptors;
{
    NSMutableArray *array = [[NSMutableArray alloc] init];
    [array addObject:@"class"];

    unsigned int outCount, i;
    objc_property_t *properties = class_copyPropertyList([self class], &outCount);
    for (i = 0; i < outCount; i++) {
    	objc_property_t property = properties[i];
    	const char *propName = property_getName(property);
    	if (propName) {
    		NSString *propertyName = [NSString stringWithCString:propName encoding:NSASCIIStringEncoding];
            [array addObject:propertyName];
    	}
    }
    
    free(properties);

    return array;
}

- (NSArray *)WX_propertyDescriptors;
{
    NSArray *properties = [self WX_propertiesForPropertyDescriptors];
    NSMutableArray *descriptors = [[NSMutableArray alloc] initWithCapacity:properties.count];
    for (NSObject *property in properties) {
        WXRuntimePropertyDescriptor *descriptor = [self WX_propertyDescriptorForPropertyObject:property];
        if (descriptor) {
            [descriptors addObject:descriptor];
        }
    }
    
    return descriptors;
}

- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForPropertyObject:(NSObject *)property;
{
    WXRuntimePropertyDescriptor *descriptor = nil;
    if ([property isKindOfClass:[NSString class]]) {
        NSString *propertyName = (NSString *)property;
        
        objc_property_t propertyDefinition = class_getProperty([self class], [propertyName cStringUsingEncoding:NSASCIIStringEncoding]);
        if (propertyDefinition != NULL) {
            descriptor = [self WX_propertyDescriptorForPropertyName:propertyName property:propertyDefinition];
        } else {
            SEL selector = NSSelectorFromString(propertyName);
            if (selector != nil) {
                descriptor = [self WX_propertyDescriptorForSelector:selector];
            }
        }
    }
    
    return descriptor;
}

- (WXRuntimeRemoteObject *)WX_propertyDescriptorValueForSelector:(SEL)selector;
{
    id value = [self WX_valueForKey:NSStringFromSelector(selector)];

    WXRuntimeRemoteObject *remoteValueObject = [self WX_propertyDescriptorValueForObject:value];
    
    // Determine the real class name.
    static const int retValueBufferSize = 32;
    char retVal[retValueBufferSize];
    
    Method method = class_getInstanceMethod([self class], selector);
    method_getReturnType(method, retVal, retValueBufferSize);
    NSString *encodedReturnType = [[NSString alloc] initWithBytes:retVal length:retValueBufferSize encoding:NSASCIIStringEncoding];
    NSString *returnType = WXReturnTypeStringForEncodedType(encodedReturnType);
    
    if ([returnType isEqualToString:@"id"]) {
        remoteValueObject.classNameString = NSStringFromClass([value class]);
    } else {
        remoteValueObject.classNameString = returnType;
    }
    
    return remoteValueObject;
}

- (WXRuntimeRemoteObject *)WX_propertyDescriptorValueForObject:(id)object;
{
    WXRuntimeRemoteObject *remoteValueObject = [[self class] WX_remoteObjectRepresentationForObject:object];
    return remoteValueObject;
}

- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForSelector:(SEL)selector;
{
    if (![self respondsToSelector:selector]) {
        return nil;
    }
    
    WXRuntimePropertyDescriptor *descriptor = [[WXRuntimePropertyDescriptor alloc] init];
    
    descriptor.name = NSStringFromSelector(selector);
    descriptor.value = [self WX_propertyDescriptorValueForSelector:selector];
    descriptor.writable = [NSNumber numberWithBool:NO];
    descriptor.configurable = [NSNumber numberWithBool:NO];
    descriptor.enumerable = [NSNumber numberWithBool:NO];
    descriptor.wasThrown = [NSNumber numberWithBool:NO];
    
    return descriptor;
}

- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForPropertyName:(NSString *)propertyName property:(objc_property_t)property
{
    id object = nil;
    @try {
        object = [self valueForKey:propertyName];
    } @catch (NSException *exception) {
        // valueForKey won't work for certain types (like selectors, pointers, blocks).
        // TODO: Make introspection for primitive types work.
        object = @"<WXDebugger: unsupported type>";
    }

    WXRuntimePropertyDescriptor *descriptor = [[WXRuntimePropertyDescriptor alloc] init];

    descriptor.name = propertyName;
    descriptor.value = [self WX_propertyDescriptorValueForObject:object];
    descriptor.writable = [NSNumber numberWithBool:NO];
    descriptor.configurable = [NSNumber numberWithBool:NO];
    descriptor.enumerable = [NSNumber numberWithBool:YES];
    descriptor.wasThrown = [NSNumber numberWithBool:NO];
    
    return descriptor;
}

@end


