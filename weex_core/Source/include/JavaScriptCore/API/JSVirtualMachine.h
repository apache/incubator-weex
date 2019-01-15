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
#import <JavaScriptCore/JavaScriptCore.h>

#if JSC_OBJC_API_ENABLED

/*!
@interface
@discussion An instance of JSVirtualMachine represents a single JavaScript "object space"
 or set of execution resources. Thread safety is supported by locking the
 virtual machine, with concurrent JavaScript execution supported by allocating
 separate instances of JSVirtualMachine.
*/
NS_CLASS_AVAILABLE(10_9, 7_0)
@interface JSVirtualMachine : NSObject

/*!
@methodgroup Creating New Virtual Machines
*/
/*!
@method
@abstract Create a new JSVirtualMachine.
*/
- (instancetype)init;

/*!
@methodgroup Memory Management
*/
/*!
@method
@abstract Notify the JSVirtualMachine of an external object relationship.
@discussion Allows clients of JSVirtualMachine to make the JavaScript runtime aware of 
 arbitrary external Objective-C object graphs. The runtime can then use 
 this information to retain any JavaScript values that are referenced 
 from somewhere in said object graph.

 For correct behavior clients must make their external object graphs 
 reachable from within the JavaScript runtime. If an Objective-C object is 
 reachable from within the JavaScript runtime, all managed references 
 transitively reachable from it as recorded using
 -addManagedReference:withOwner: will be scanned by the garbage collector.
@param object The object that the owner points to.
@param owner The object that owns the pointed to object.
*/ 
- (void)addManagedReference:(id)object withOwner:(id)owner;

/*!
@method
@abstract Notify the JSVirtualMachine that a previous object relationship no longer exists.
@discussion The JavaScript runtime will continue to scan any references that were
 reported to it by -addManagedReference:withOwner: until those references are removed.
@param object The object that was formerly owned.
@param owner The former owner.
*/
- (void)removeManagedReference:(id)object withOwner:(id)owner;

@end

#endif
