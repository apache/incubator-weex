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
#ifndef JSVirtualMachineInternal_h
#define JSVirtualMachineInternal_h

#if JSC_OBJC_API_ENABLED

#import <JavaScriptCore/JavaScriptCore.h>

namespace JSC {
class VM;
class SlotVisitor;
}

#if defined(__OBJC__)
@class NSMapTable;

@interface JSVirtualMachine(Internal)

JSContextGroupRef getGroupFromVirtualMachine(JSVirtualMachine *);

+ (JSVirtualMachine *)virtualMachineWithContextGroupRef:(JSContextGroupRef)group;

- (JSContext *)contextForGlobalContextRef:(JSGlobalContextRef)globalContext;
- (void)addContext:(JSContext *)wrapper forGlobalContextRef:(JSGlobalContextRef)globalContext;

@end
#endif // defined(__OBJC__)

void scanExternalObjectGraph(JSC::VM&, JSC::SlotVisitor&, void* root);
void scanExternalRememberedSet(JSC::VM&, JSC::SlotVisitor&);

#endif // JSC_OBJC_API_ENABLED

#endif // JSVirtualMachineInternal_h
