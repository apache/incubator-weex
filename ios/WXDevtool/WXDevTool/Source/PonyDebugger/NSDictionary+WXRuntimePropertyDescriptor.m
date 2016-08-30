//
//  NSDictionary+WXRuntimePropertyDescriptor.m
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 2013-02-02.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "NSDictionary+WXRuntimePropertyDescriptor.h"
#import "NSObject+WXRuntimePropertyDescriptor.h"

#import "WXRuntimeTypes.h"


#pragma mark - Private Classes

@interface _WXDictionaryPropertyKey : NSObject

- (id)initWithKey:(id <NSCopying>)key stringRepresentation:(NSString *)stringKey;

@property (nonatomic, copy) id <NSCopying> key;
@property (nonatomic, copy) NSString *stringKey;

@end


#pragma mark - Implementation

@implementation NSDictionary (WXRuntimePropertyDescriptor)

- (NSArray *)WX_propertiesForPropertyDescriptors;
{
    NSMutableArray *properties = [[NSMutableArray alloc] initWithCapacity:self.count];
    
    [self enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        NSString *stringKey = [NSString stringWithFormat:@"%@", key];
        
        _WXDictionaryPropertyKey *containerIndex = [[_WXDictionaryPropertyKey alloc] initWithKey:key stringRepresentation:stringKey];
        [properties addObject:containerIndex];
    }];
    
    return properties;
}

- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForPropertyObject:(NSObject *)property;
{
    WXRuntimePropertyDescriptor *descriptor = [super WX_propertyDescriptorForPropertyObject:property];
    if (!descriptor) {
        if ([property isKindOfClass:[_WXDictionaryPropertyKey class]]) {
            descriptor = [self WX_propertyDescriptorForDictionaryContainerKey:(_WXDictionaryPropertyKey *)property];
        }
    }
    
    return descriptor;
}

- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForDictionaryContainerKey:(_WXDictionaryPropertyKey *)containerKey;
{
    WXRuntimePropertyDescriptor *descriptor = [[WXRuntimePropertyDescriptor alloc] init];
    
    id propertyValue = [self objectForKey:containerKey.key];
    
    descriptor.name = containerKey.stringKey;
    descriptor.value = [self WX_propertyDescriptorValueForObject:propertyValue];
    descriptor.writable = [NSNumber numberWithBool:NO];
    descriptor.configurable = [NSNumber numberWithBool:NO];
    descriptor.enumerable = [NSNumber numberWithBool:YES];
    descriptor.wasThrown = [NSNumber numberWithBool:NO];
    
    return descriptor;
}

@end


#pragma mark - _WXDictionaryPropertyKey

@implementation _WXDictionaryPropertyKey

@synthesize stringKey = _stringKey;
@synthesize key = _key;

- (id)initWithKey:(id<NSCopying>)key stringRepresentation:(NSString *)stringKey;
{
    if (!(self = [super self])) {
        return nil;
    }
    
    self.key = key;
    self.stringKey = stringKey;
    
    return self;
}

@end
