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
#pragma once

#include "JSObject.h"

namespace JSC {

// This is a mixin for the two kinds of Arguments-class objects that arise when you say
// "arguments" inside a function. This class doesn't show up in the JSCell inheritance hierarchy.
template<typename Type>
class GenericArguments : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | InterceptsGetOwnPropertySlotByIndexEvenWhenLengthIsNotZero | OverridesGetPropertyNames;

protected:
    GenericArguments(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    static void visitChildren(JSCell*, SlotVisitor&);
    static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    static bool getOwnPropertySlotByIndex(JSObject*, ExecState*, unsigned propertyName, PropertySlot&);
    static void getOwnPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);
    static bool putByIndex(JSCell*, ExecState*, unsigned propertyName, JSValue, bool shouldThrow);
    static bool deleteProperty(JSCell*, ExecState*, PropertyName);
    static bool deletePropertyByIndex(JSCell*, ExecState*, unsigned propertyName);
    static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool shouldThrow);
    
    void initModifiedArgumentsDescriptor(VM&, unsigned length);
    void initModifiedArgumentsDescriptorIfNecessary(VM&, unsigned length);
    void setModifiedArgumentDescriptor(VM&, unsigned index, unsigned length);
    bool isModifiedArgumentDescriptor(unsigned index, unsigned length);

    void copyToArguments(ExecState*, VirtualRegister firstElementDest, unsigned offset, unsigned length);
    
    AuxiliaryBarrier<bool*> m_modifiedArgumentsDescriptor;
};

} // namespace JSC
