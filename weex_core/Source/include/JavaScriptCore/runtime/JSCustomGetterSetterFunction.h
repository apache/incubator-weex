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

#include "JSFunction.h"

namespace JSC {

class CustomGetterSetter;

class JSCustomGetterSetterFunction : public JSFunction {
public:
    typedef JSFunction Base;

    // The Type is set to the number of arguments the resultant function will have.
    enum class Type { Getter = 0, Setter = 1 };

    static const unsigned StructureFlags = Base::StructureFlags;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        ASSERT(globalObject);
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSFunctionType, StructureFlags), info());
    }

    static JSCustomGetterSetterFunction* create(VM&, JSGlobalObject*, CustomGetterSetter*, const Type, const PropertyName&);

    DECLARE_EXPORT_INFO;

protected:
    static void visitChildren(JSCell*, SlotVisitor&);

private:
    JSCustomGetterSetterFunction(VM&, JSGlobalObject*, Structure*, Type, const PropertyName&);
    void finishCreation(VM&, NativeExecutable*, CustomGetterSetter*, const String&);

    static EncodedJSValue JSC_HOST_CALL customGetterSetterFunctionCall(ExecState*);

    CustomGetterSetter* customGetterSetter() const { return m_getterSetter.get(); }
    bool isSetter() const { return m_type == Type::Setter; }
    const PropertyName& propertyName() const { return m_propertyName; }

    WriteBarrier<CustomGetterSetter> m_getterSetter;
    Type m_type;
    PropertyName m_propertyName;
};

} // namespace JSC
