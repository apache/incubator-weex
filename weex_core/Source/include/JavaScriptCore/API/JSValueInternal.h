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
#ifndef JSValueInternal_h
#define JSValueInternal_h

#import <JavaScriptCore/JavaScriptCore.h>
#import <JavaScriptCore/JSValue.h>

#if JSC_OBJC_API_ENABLED

@interface JSValue(Internal)

JSValueRef valueInternalValue(JSValue *);

- (JSValue *)initWithValue:(JSValueRef)value inContext:(JSContext *)context;

JSValueRef objectToValue(JSContext *, id);
id valueToObject(JSContext *, JSValueRef);
id valueToNumber(JSGlobalContextRef, JSValueRef, JSValueRef* exception);
id valueToString(JSGlobalContextRef, JSValueRef, JSValueRef* exception);
id valueToDate(JSGlobalContextRef, JSValueRef, JSValueRef* exception);
id valueToArray(JSGlobalContextRef, JSValueRef, JSValueRef* exception);
id valueToDictionary(JSGlobalContextRef, JSValueRef, JSValueRef* exception);

+ (SEL)selectorForStructToValue:(const char *)structTag;
+ (SEL)selectorForValueToStruct:(const char *)structTag;

@end

NSInvocation *typeToValueInvocationFor(const char* encodedType);
NSInvocation *valueToTypeInvocationFor(const char* encodedType);

#endif

#endif // JSValueInternal_h
