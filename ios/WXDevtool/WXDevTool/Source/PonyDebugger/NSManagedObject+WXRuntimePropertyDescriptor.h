//
//  NSManagedObject+WXRuntimePropertyDescriptor.h
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 8/7/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import <CoreData/CoreData.h>


@class WXRuntimeRemoteObject;
@class WXRuntimePropertyDescriptor;


@interface NSManagedObject (WXRuntimePropertyDescriptor)

- (NSArray *)WX_propertiesForPropertyDescriptors;
- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForPropertyObject:(NSObject *)property;

- (WXRuntimeRemoteObject *)WX_propertyDescriptorValueForPropertyDescription:(NSPropertyDescription *)property;

- (WXRuntimePropertyDescriptor *)WX_propertyDescriptorForPropertyDescription:(NSPropertyDescription *)property;

@end
