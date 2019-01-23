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

#include "Identifier.h"
#include "JSDestructibleObject.h"

namespace JSC {

class FunctionPrototype;

class InternalFunction : public JSDestructibleObject {
public:
    typedef JSDestructibleObject Base;
    static const unsigned StructureFlags = Base::StructureFlags | ImplementsHasInstance | ImplementsDefaultHasInstance | TypeOfShouldCallGetCallData;

    DECLARE_EXPORT_INFO;

    JS_EXPORT_PRIVATE static void visitChildren(JSCell*, SlotVisitor&);

    JS_EXPORT_PRIVATE const String& name();
    const String displayName(VM&);
    const String calculatedDisplayName(VM&);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
    { 
        return Structure::create(vm, globalObject, proto, TypeInfo(ObjectType, StructureFlags), info()); 
    }

    JS_EXPORT_PRIVATE static Structure* createSubclassStructure(ExecState*, JSValue newTarget, Structure*);

protected:
    JS_EXPORT_PRIVATE InternalFunction(VM&, Structure*);

    JS_EXPORT_PRIVATE void finishCreation(VM&, const String& name);

    static CallType getCallData(JSCell*, CallData&);
    WriteBarrier<JSString> m_originalName;
};

InternalFunction* asInternalFunction(JSValue);

inline InternalFunction* asInternalFunction(JSValue value)
{
    ASSERT(asObject(value)->inherits(*value.getObject()->vm(), InternalFunction::info()));
    return static_cast<InternalFunction*>(asObject(value));
}

} // namespace JSC
