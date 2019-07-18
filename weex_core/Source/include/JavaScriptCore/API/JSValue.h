/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef JSValue_h
#define JSValue_h

#if JSC_OBJC_API_ENABLED

#import <CoreGraphics/CGGeometry.h>

@class JSContext;

/*!
@interface
@discussion A JSValue is a reference to a JavaScript value. Every JSValue
 originates from a JSContext and holds a strong reference to it.
 When a JSValue instance method creates a new JSValue, the new value
 originates from the same JSContext.

 All JSValues values also originate from a JSVirtualMachine
 (available indirectly via the context property). It is an error to pass a
 JSValue to a method or property of a JSValue or JSContext originating from a
 different JSVirtualMachine. Doing so will raise an Objective-C exception.
*/
NS_CLASS_AVAILABLE(10_9, 7_0)
@interface JSValue : NSObject

/*!
@property
@abstract The JSContext that this value originates from.
*/
@property (readonly, strong) JSContext *context;

/*!
@methodgroup Creating JavaScript Values
*/
/*!
@method
@abstract Create a JSValue by converting an Objective-C object.
@discussion The resulting JSValue retains the provided Objective-C object.
@param value The Objective-C object to be converted.
@result The new JSValue.
*/
+ (JSValue *)valueWithObject:(id)value inContext:(JSContext *)context;

/*!
@method
@abstract Create a JavaScript value from a BOOL primitive.
@param context The JSContext in which the resulting JSValue will be created.
@result The new JSValue representing the equivalent boolean value.
*/
+ (JSValue *)valueWithBool:(BOOL)value inContext:(JSContext *)context;

/*!
@method
@abstract Create a JavaScript value from a double primitive.
@param context The JSContext in which the resulting JSValue will be created.
@result The new JSValue representing the equivalent boolean value.
*/
+ (JSValue *)valueWithDouble:(double)value inContext:(JSContext *)context;

/*!
@method
@abstract Create a JavaScript value from an <code>int32_t</code> primitive.
@param context The JSContext in which the resulting JSValue will be created.
@result The new JSValue representing the equivalent boolean value.
*/
+ (JSValue *)valueWithInt32:(int32_t)value inContext:(JSContext *)context;

/*!
@method
@abstract Create a JavaScript value from a <code>uint32_t</code> primitive.
@param context The JSContext in which the resulting JSValue will be created.
@result The new JSValue representing the equivalent boolean value.
*/
+ (JSValue *)valueWithUInt32:(uint32_t)value inContext:(JSContext *)context;

/*!
@method
@abstract Create a new, empty JavaScript object.
@param context The JSContext in which the resulting object will be created.
@result The new JavaScript object.
*/
+ (JSValue *)valueWithNewObjectInContext:(JSContext *)context;

/*!
@method
@abstract Create a new, empty JavaScript array.
@param context The JSContext in which the resulting array will be created.
@result The new JavaScript array.
*/
+ (JSValue *)valueWithNewArrayInContext:(JSContext *)context;

/*!
@method
@abstract Create a new JavaScript regular expression object.
@param pattern The regular expression pattern.
@param flags The regular expression flags.
@param context The JSContext in which the resulting regular expression object will be created.
@result The new JavaScript regular expression object.
*/
+ (JSValue *)valueWithNewRegularExpressionFromPattern:(NSString *)pattern flags:(NSString *)flags inContext:(JSContext *)context;

/*!
@method
@abstract Create a new JavaScript error object.
@param message The error message.
@param context The JSContext in which the resulting error object will be created.
@result The new JavaScript error object.
*/
+ (JSValue *)valueWithNewErrorFromMessage:(NSString *)message inContext:(JSContext *)context;

/*!
@method
@abstract Create the JavaScript value <code>null</code>.
@param context The JSContext to which the resulting JSValue belongs.
@result The JSValue representing the JavaScript value <code>null</code>.
*/
+ (JSValue *)valueWithNullInContext:(JSContext *)context;

/*!
@method
@abstract Create the JavaScript value <code>undefined</code>.
@param context The JSContext to which the resulting JSValue belongs.
@result The JSValue representing the JavaScript value <code>undefined</code>.
*/
+ (JSValue *)valueWithUndefinedInContext:(JSContext *)context;

/*!
@methodgroup Converting to Objective-C Types
@discussion When converting between JavaScript values and Objective-C objects a copy is
 performed. Values of types listed below are copied to the corresponding
 types on conversion in each direction. For NSDictionaries, entries in the
 dictionary that are keyed by strings are copied onto a JavaScript object.
 For dictionaries and arrays, conversion is recursive, with the same object
 conversion being applied to all entries in the collection.

<pre>
@textblock
   Objective-C type  |   JavaScript type
 --------------------+---------------------
         nil         |     undefined
        NSNull       |        null
       NSString      |       string
       NSNumber      |   number, boolean
     NSDictionary    |   Object object
       NSArray       |    Array object
        NSDate       |     Date object
       NSBlock (1)   |   Function object (1)
          id (2)     |   Wrapper object (2)
        Class (3)    | Constructor object (3)
@/textblock
</pre>

 (1) Instances of NSBlock with supported arguments types will be presented to
 JavaScript as a callable Function object. For more information on supported
 argument types see JSExport.h. If a JavaScript Function originating from an
 Objective-C block is converted back to an Objective-C object the block will
 be returned. All other JavaScript functions will be converted in the same
 manner as a JavaScript object of type Object.

 (2) For Objective-C instances that do not derive from the set of types listed
 above, a wrapper object to provide a retaining handle to the Objective-C
 instance from JavaScript. For more information on these wrapper objects, see
 JSExport.h. When a JavaScript wrapper object is converted back to Objective-C
 the Objective-C instance being retained by the wrapper is returned.

 (3) For Objective-C Class objects a constructor object containing exported
 class methods will be returned. See JSExport.h for more information on
 constructor objects.

 For all methods taking arguments of type id, arguments will be converted
 into a JavaScript value according to the above conversion.
*/
/*!
@method
@abstract Convert this JSValue to an Objective-C object.
@discussion The JSValue is converted to an Objective-C object according 
 to the conversion rules specified above.
@result The Objective-C representation of this JSValue.
*/
- (id)toObject;

/*!
@method
@abstract Convert a JSValue to an Objective-C object of a specific class.
@discussion The JSValue is converted to an Objective-C object of the specified Class. 
 If the result is not of the specified Class then <code>nil</code> will be returned.
@result An Objective-C object of the specified Class or <code>nil</code>.
*/
- (id)toObjectOfClass:(Class)expectedClass;

/*!
@method
@abstract Convert a JSValue to a boolean.
@discussion The JSValue is converted to a boolean according to the rules specified 
 by the JavaScript language.
@result The boolean result of the conversion.
*/
- (BOOL)toBool;

/*!
@method
@abstract Convert a JSValue to a double.
@discussion The JSValue is converted to a number according to the rules specified 
 by the JavaScript language.
@result The double result of the conversion.
*/
- (double)toDouble;

/*!
@method
@abstract Convert a JSValue to an <code>int32_t</code>.
@discussion The JSValue is converted to an integer according to the rules specified 
 by the JavaScript language.
@result The <code>int32_t</code> result of the conversion.
*/
- (int32_t)toInt32;

/*!
@method
@abstract Convert a JSValue to a <code>uint32_t</code>.
@discussion The JSValue is converted to an integer according to the rules specified 
 by the JavaScript language.
@result The <code>uint32_t</code> result of the conversion.
*/
- (uint32_t)toUInt32;

/*!
@method
@abstract Convert a JSValue to a NSNumber.
@discussion If the JSValue represents a boolean, a NSNumber value of YES or NO 
 will be returned. For all other types the value will be converted to a number according 
 to the rules specified by the JavaScript language.
@result The NSNumber result of the conversion.
*/
- (NSNumber *)toNumber;

/*!
@method
@abstract Convert a JSValue to a NSString.
@discussion The JSValue is converted to a string according to the rules specified 
 by the JavaScript language.
@result The NSString containing the result of the conversion.
*/
- (NSString *)toString;

/*!
@method
@abstract Convert a JSValue to a NSDate.
@discussion The value is converted to a number representing a time interval 
 since 1970 which is then used to create a new NSDate instance.
@result The NSDate created using the converted time interval.
*/
- (NSDate *)toDate;

/*!
@method
@abstract Convert a JSValue to a NSArray.
@discussion If the value is <code>null</code> or <code>undefined</code> then <code>nil</code> is returned.
 If the value is not an object then a JavaScript TypeError will be thrown.
 The property <code>length</code> is read from the object, converted to an unsigned
 integer, and an NSArray of this size is allocated. Properties corresponding
 to indicies within the array bounds will be copied to the array, with
 JSValues converted to equivalent Objective-C objects as specified.
@result The NSArray containing the recursively converted contents of the 
 converted JavaScript array.
*/
- (NSArray *)toArray;

/*!
@method
@abstract Convert a JSValue to a NSDictionary.
@discussion If the value is <code>null</code> or <code>undefined</code> then <code>nil</code> is returned.
 If the value is not an object then a JavaScript TypeError will be thrown.
 All enumerable properties of the object are copied to the dictionary, with
 JSValues converted to equivalent Objective-C objects as specified.
@result The NSDictionary containing the recursively converted contents of
 the converted JavaScript object.
*/
- (NSDictionary *)toDictionary;

/*!
@methodgroup Accessing Properties
*/
/*!
@method
@abstract Access a property of a JSValue.
@result The JSValue for the requested property or the JSValue <code>undefined</code> 
 if the property does not exist.
*/
- (JSValue *)valueForProperty:(NSString *)property;

/*!
@method
@abstract Set a property on a JSValue.
*/
- (void)setValue:(id)value forProperty:(NSString *)property;

/*!
@method
@abstract Delete a property from a JSValue.
@result YES if deletion is successful, NO otherwise.
*/
- (BOOL)deleteProperty:(NSString *)property;

/*!
@method
@abstract Check if a JSValue has a property.
@discussion This method has the same function as the JavaScript operator <code>in</code>.
@result Returns YES if property is present on the value.
*/
- (BOOL)hasProperty:(NSString *)property;

/*!
@method
@abstract Define properties with custom descriptors on JSValues.
@discussion This method may be used to create a data or accessor property on an object.
 This method operates in accordance with the Object.defineProperty method in the 
 JavaScript language.
*/
- (void)defineProperty:(NSString *)property descriptor:(id)descriptor;

/*!
@method
@abstract Access an indexed (numerical) property on a JSValue.
@result The JSValue for the property at the specified index. 
 Returns the JavaScript value <code>undefined</code> if no property exists at that index. 
*/
- (JSValue *)valueAtIndex:(NSUInteger)index;

/*!
@method
@abstract Set an indexed (numerical) property on a JSValue.
@discussion For JSValues that are JavaScript arrays, indices greater than 
 UINT_MAX - 1 will not affect the length of the array.
*/
- (void)setValue:(id)value atIndex:(NSUInteger)index;

/*!
@functiongroup Checking JavaScript Types
*/

/*!
@property
@abstract Check if a JSValue corresponds to the JavaScript value <code>undefined</code>.
*/ 
@property (readonly) BOOL isUndefined;

/*!
@property
@abstract Check if a JSValue corresponds to the JavaScript value <code>null</code>.
*/
@property (readonly) BOOL isNull;

/*!
@property
@abstract Check if a JSValue is a boolean.
*/
@property (readonly) BOOL isBoolean;

/*!
@property
@abstract Check if a JSValue is a number.
@discussion In JavaScript, there is no differentiation between types of numbers.
 Semantically all numbers behave like doubles except in special cases like bit
 operations. 
*/
@property (readonly) BOOL isNumber;

/*!
@property
@abstract Check if a JSValue is a string.
*/
@property (readonly) BOOL isString;

/*!
@property
@abstract Check if a JSValue is an object.
*/
@property (readonly) BOOL isObject;

/*!
@property
@abstract Check if a JSValue is an array.
*/ 
@property (readonly) BOOL isArray NS_AVAILABLE(10_11, 9_0);

/*!
@property
@abstract Check if a JSValue is a date.
*/ 
@property (readonly) BOOL isDate NS_AVAILABLE(10_11, 9_0);

/*!
@method
@abstract Compare two JSValues using JavaScript's <code>===</code> operator.
*/
- (BOOL)isEqualToObject:(id)value;

/*!
@method
@abstract Compare two JSValues using JavaScript's <code>==</code> operator.
*/
- (BOOL)isEqualWithTypeCoercionToObject:(id)value;

/*!
@method
@abstract Check if a JSValue is an instance of another object.
@discussion This method has the same function as the JavaScript operator <code>instanceof</code>.
 If an object other than a JSValue is passed, it will first be converted according to
 the aforementioned rules.
*/
- (BOOL)isInstanceOf:(id)value;

/*!
@methodgroup Calling Functions and Constructors
*/
/*!
@method
@abstract Invoke a JSValue as a function.
@discussion In JavaScript, if a function doesn't explicitly return a value then it
 implicitly returns the JavaScript value <code>undefined</code>.
@param arguments The arguments to pass to the function.
@result The return value of the function call. 
*/
- (JSValue *)callWithArguments:(NSArray *)arguments;

/*!
@method
@abstract Invoke a JSValue as a constructor.
@discussion This is equivalent to using the <code>new</code> syntax in JavaScript.
@param arguments The arguments to pass to the constructor.
@result The return value of the constructor call.
*/
- (JSValue *)constructWithArguments:(NSArray *)arguments;

/*!
@method
@abstract Invoke a method on a JSValue.
@discussion Accesses the property named <code>method</code> from this value and 
 calls the resulting value as a function, passing this JSValue as the <code>this</code>
 value along with the specified arguments.
@param method The name of the method to be invoked.
@param arguments The arguments to pass to the method.
@result The return value of the method call.
*/
- (JSValue *)invokeMethod:(NSString *)method withArguments:(NSArray *)arguments;

@end

/*!
@category
@discussion Objective-C methods exported to JavaScript may have argument and/or return
 values of struct types, provided that conversion to and from the struct is
 supported by JSValue. Support is provided for any types where JSValue
 contains both a class method <code>valueWith<Type>:inContext:</code>, and and instance
 method <code>to<Type></code>- where the string <code><Type></code> in these selector names match,
 with the first argument to the former being of the same struct type as the
 return type of the latter.
 Support is provided for structs of type CGPoint, NSRange, CGRect and CGSize.
*/
@interface JSValue (StructSupport)

/*!
@method
@abstract Create a JSValue from a CGPoint.
@result A newly allocated JavaScript object containing properties
 named <code>x</code> and <code>y</code>, with values from the CGPoint.
*/
+ (JSValue *)valueWithPoint:(CGPoint)point inContext:(JSContext *)context;

/*!
@method
@abstract Create a JSValue from a NSRange.
@result A newly allocated JavaScript object containing properties
 named <code>location</code> and <code>length</code>, with values from the NSRange.
*/
+ (JSValue *)valueWithRange:(NSRange)range inContext:(JSContext *)context;

/*!
@method
@abstract 
Create a JSValue from a CGRect.
@result A newly allocated JavaScript object containing properties
 named <code>x</code>, <code>y</code>, <code>width</code>, and <code>height</code>, with values from the CGRect.
*/
+ (JSValue *)valueWithRect:(CGRect)rect inContext:(JSContext *)context;

/*!
@method
@abstract Create a JSValue from a CGSize.
@result A newly allocated JavaScript object containing properties
 named <code>width</code> and <code>height</code>, with values from the CGSize.
*/
+ (JSValue *)valueWithSize:(CGSize)size inContext:(JSContext *)context;

/*!
@method
@abstract Convert a JSValue to a CGPoint.
@discussion Reads the properties named <code>x</code> and <code>y</code> from
 this JSValue, and converts the results to double.
@result The new CGPoint.
*/
- (CGPoint)toPoint;

/*!
@method
@abstract Convert a JSValue to an NSRange.
@discussion Reads the properties named <code>location</code> and
 <code>length</code> from this JSValue and converts the results to double.
@result The new NSRange.
*/
- (NSRange)toRange;

/*!
@method
@abstract Convert a JSValue to a CGRect.
@discussion Reads the properties named <code>x</code>, <code>y</code>, 
 <code>width</code>, and <code>height</code> from this JSValue and converts the results to double.
@result The new CGRect.
*/
- (CGRect)toRect;

/*!
@method
@abstract Convert a JSValue to a CGSize.
@discussion Reads the properties named <code>width</code> and
 <code>height</code> from this JSValue and converts the results to double.
@result The new CGSize.
*/
- (CGSize)toSize;

@end

/*!
@category
@discussion Instances of JSValue implement the following methods in order to enable
 support for subscript access by key and index, for example:

@textblock
    JSValue *objectA, *objectB;
    JSValue *v1 = object[@"X"]; // Get value for property "X" from 'object'.
    JSValue *v2 = object[42];   // Get value for index 42 from 'object'.
    object[@"Y"] = v1;          // Assign 'v1' to property "Y" of 'object'.
    object[101] = v2;           // Assign 'v2' to index 101 of 'object'.
@/textblock

 An object key passed as a subscript will be converted to a JavaScript value,
 and then the value converted to a string used as a property name.
*/
@interface JSValue (SubscriptSupport)

- (JSValue *)objectForKeyedSubscript:(id)key;
- (JSValue *)objectAtIndexedSubscript:(NSUInteger)index;
- (void)setObject:(id)object forKeyedSubscript:(NSObject <NSCopying> *)key;
- (void)setObject:(id)object atIndexedSubscript:(NSUInteger)index;

@end

/*!
@category
@discussion  These functions are for bridging between the C API and the Objective-C API.
*/
@interface JSValue (JSValueRefSupport)

/*!
@method
@abstract Creates a JSValue, wrapping its C API counterpart.
@result The Objective-C API equivalent of the specified JSValueRef.
*/
+ (JSValue *)valueWithJSValueRef:(JSValueRef)value inContext:(JSContext *)context;

/*!
@property
@abstract Returns the C API counterpart wrapped by a JSContext.
@result The C API equivalent of this JSValue.
*/
@property (readonly) JSValueRef JSValueRef;
@end

#ifdef __cplusplus
extern "C" {
#endif

/*!
@group Property Descriptor Constants
@discussion These keys may assist in creating a property descriptor for use with the
 defineProperty method on JSValue.
 Property descriptors must fit one of three descriptions:

 Data Descriptor:
  - A descriptor containing one or both of the keys <code>value</code> and <code>writable</code>,
    and optionally containing one or both of the keys <code>enumerable</code> and
    <code>configurable</code>. A data descriptor may not contain either the <code>get</code> or
    <code>set</code> key.
    A data descriptor may be used to create or modify the attributes of a
    data property on an object (replacing any existing accessor property).

 Accessor Descriptor:
  - A descriptor containing one or both of the keys <code>get</code> and <code>set</code>, and
    optionally containing one or both of the keys <code>enumerable</code> and
    <code>configurable</code>. An accessor descriptor may not contain either the <code>value</code>
    or <code>writable</code> key.
    An accessor descriptor may be used to create or modify the attributes of
    an accessor property on an object (replacing any existing data property).

 Generic Descriptor:
  - A descriptor containing one or both of the keys <code>enumerable</code> and
    <code>configurable</code>. A generic descriptor may not contain any of the keys
    <code>value</code>, <code>writable</code>, <code>get</code>, or <code>set</code>.
    A generic descriptor may be used to modify the attributes of an existing
    data or accessor property, or to create a new data property.
*/
/*!
@const 
*/
JS_EXPORT extern NSString * const JSPropertyDescriptorWritableKey;
/*!
@const 
*/
JS_EXPORT extern NSString * const JSPropertyDescriptorEnumerableKey;
/*!
@const 
*/
JS_EXPORT extern NSString * const JSPropertyDescriptorConfigurableKey;
/*!
@const 
*/
JS_EXPORT extern NSString * const JSPropertyDescriptorValueKey;
/*!
@const 
*/
JS_EXPORT extern NSString * const JSPropertyDescriptorGetKey;
/*!
@const 
*/
JS_EXPORT extern NSString * const JSPropertyDescriptorSetKey;

#ifdef __cplusplus
} // extern "C"
#endif

#endif

#endif // JSValue_h
