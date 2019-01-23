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

#include "ArgumentsMode.h"
#include "JSObject.h"

namespace JSC {

// This is an Arguments-class object that we create when you do function.arguments, or you say
// "arguments" inside a function in strict mode. It behaves almpst entirely like an ordinary
// JavaScript object. All of the arguments values are simply copied from the stack (possibly via
// some sophisticated ValueRecovery's if an optimizing compiler is in play) and the appropriate
// properties of the object are populated. The only reason why we need a special class is to make
// the object claim to be "Arguments" from a toString standpoint, and to avoid materializing the
// caller/callee/@@iterator properties unless someone asks for them.
class ClonedArguments : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | OverridesGetPropertyNames;
    
private:
    ClonedArguments(VM&, Structure*, Butterfly*);

public:
    static ClonedArguments* createEmpty(VM&, Structure*, JSFunction* callee, unsigned length);
    static ClonedArguments* createEmpty(ExecState*, JSFunction* callee, unsigned length);
    static ClonedArguments* createWithInlineFrame(ExecState* myFrame, ExecState* targetFrame, InlineCallFrame*, ArgumentsMode);
    static ClonedArguments* createWithMachineFrame(ExecState* myFrame, ExecState* targetFrame, ArgumentsMode);
    static ClonedArguments* createByCopyingFrom(ExecState*, Structure*, Register* argumentsStart, unsigned length, JSFunction* callee);
    
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);
    static Structure* createSlowPutStructure(VM&, JSGlobalObject*, JSValue prototype);

    static void visitChildren(JSCell*, SlotVisitor&);

    DECLARE_INFO;

private:
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype, IndexingType);

    static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    static void getOwnPropertyNames(JSObject*, ExecState*, PropertyNameArray&, EnumerationMode);
    static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);
    static bool deleteProperty(JSCell*, ExecState*, PropertyName);
    static bool defineOwnProperty(JSObject*, ExecState*, PropertyName, const PropertyDescriptor&, bool shouldThrow);
    
    bool specialsMaterialized() const { return !m_callee; }
    void materializeSpecials(ExecState*);
    void materializeSpecialsIfNecessary(ExecState*);
    
    WriteBarrier<JSFunction> m_callee; // Set to nullptr when we materialize all of our special properties.
};

static const PropertyOffset clonedArgumentsLengthPropertyOffset = 100;

} // namespace JSC
