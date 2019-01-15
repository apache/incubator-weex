/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifndef JSManagedValue_h
#define JSManagedValue_h

#import <JavaScriptCore/JSBase.h>
#import <JavaScriptCore/WebKitAvailability.h>

#if JSC_OBJC_API_ENABLED

@class JSValue;
@class JSContext;

/*!
@interface
@discussion JSManagedValue represents a "conditionally retained" JSValue. 
 "Conditionally retained" means that as long as the JSManagedValue's 
 JSValue is reachable through the JavaScript object graph,
 or through the Objective-C object graph reported to the JSVirtualMachine using
 addManagedReference:withOwner:, the corresponding JSValue will 
 be retained. However, if neither graph reaches the JSManagedValue, the 
 corresponding JSValue will be released and set to nil.

The primary use for a JSManagedValue is to store a JSValue in an Objective-C
or Swift object that is exported to JavaScript. It is incorrect to store a JSValue
in an object that is exported to JavaScript, since doing so creates a retain cycle.
*/ 
NS_CLASS_AVAILABLE(10_9, 7_0)
@interface JSManagedValue : NSObject

/*!
@method
@abstract Create a JSManagedValue from a JSValue.
@result The new JSManagedValue.
*/
+ (JSManagedValue *)managedValueWithValue:(JSValue *)value;
+ (JSManagedValue *)managedValueWithValue:(JSValue *)value andOwner:(id)owner NS_AVAILABLE(10_10, 8_0);

/*!
@method
@abstract Create a JSManagedValue.
@result The new JSManagedValue.
*/
- (instancetype)initWithValue:(JSValue *)value;

/*!
@property
@abstract Get the JSValue from the JSManagedValue.
@result The corresponding JSValue for this JSManagedValue or 
 nil if the JSValue has been collected.
*/
@property (readonly, strong) JSValue *value;

@end

#endif // JSC_OBJC_API_ENABLED

#endif // JSManagedValue_h
