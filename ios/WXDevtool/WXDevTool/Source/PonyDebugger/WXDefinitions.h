//
//  WXDefinitions.h
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 8/7/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import <CoreData/CoreData.h>
#import <objc/runtime.h>

extern NSString *const WXDebuggerErrorDomain;
extern const NSInteger WXDebuggerUnimplementedDomainMethodErrorCode;
extern const NSInteger WXDebuggerRequiredAttributeMissingCode;

NSString *WXReturnTypeStringForEncodedType(NSString *string);

NSDictionary *WXRemoteObjectPropertyTypeDetailsForObject(id object);
NSDictionary *WXRemoteObjectPropertyTypeDetailsForValue(NSValue *value);

NSDictionary *WXRemoteObjectPropertyTypeDetailsForPropertyDescription(NSPropertyDescription *description);
NSDictionary *WXRemoteObjectPropertyTypeDetailsForAttributeDescription(NSAttributeDescription *description);
NSDictionary *WXRemoteObjectPropertyTypeDetailsForRelationshipDescription(NSRelationshipDescription *description);

NSDictionary *WXExtractPropertyAttributes(objc_property_t property);
