//
//  NSObject+WXRuntimePropertyDescriptor.h
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 8/7/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import <objc/runtime.h>


@class WXRuntimeRemoteObject;
@class WXRuntimePropertyDescriptor;


@interface NSObject (WXRuntimePropertyDescriptor)

+ (WXRuntimeRemoteObject *)WX_remoteObjectRepresentationForObject:(id)object;

- (id)WX_valueForKey:(NSString *)key;

- (NSArray *)WX_propertiesForPropertyDescriptors;
- (NSArray *)WX_propertyDescriptors;

- (WXRuntimeRemoteObject *)WX_propertyDescriptorValueForSelector:(SEL)selector;
- (WXRuntimeRemoteObject *)WX_propertyDescriptorValueForObject:(id)object;

- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForPropertyObject:(NSObject *)property;

- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForSelector:(SEL)selector;
- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForPropertyName:(NSString *)propertyName property:(objc_property_t)property;

@end
