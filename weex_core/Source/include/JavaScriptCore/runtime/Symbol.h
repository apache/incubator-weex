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

#include "JSString.h"
#include "PrivateName.h"

namespace JSC {

class Symbol final : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal | OverridesToThis;

    DECLARE_EXPORT_INFO;

    static const bool needsDestruction = true;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(SymbolType, StructureFlags), info());
    }

    static Symbol* create(VM&);
    static Symbol* create(ExecState*, JSString* description);
    JS_EXPORT_PRIVATE static Symbol* create(VM&, SymbolImpl& uid);

    const PrivateName& privateName() const { return m_privateName; }
    String descriptiveString() const;

    JSValue toPrimitive(ExecState*, PreferredPrimitiveType) const;
    bool getPrimitiveNumber(ExecState*, double& number, JSValue&) const;
    JSObject* toObject(ExecState*, JSGlobalObject*) const;
    double toNumber(ExecState*) const;

    static ptrdiff_t offsetOfSymbolImpl()
    {
        // PrivateName is just a Ref<SymbolImpl> which can just be used as a SymbolImpl*.
        return OBJECT_OFFSETOF(Symbol, m_privateName);
    }

protected:
    static void destroy(JSCell*);

    Symbol(VM&);
    Symbol(VM&, const String&);
    Symbol(VM&, SymbolImpl& uid);

    void finishCreation(VM&);

    PrivateName m_privateName;
};

Symbol* asSymbol(JSValue);

inline Symbol* asSymbol(JSValue value)
{
    ASSERT(value.asCell()->isSymbol());
    return jsCast<Symbol*>(value.asCell());
}

} // namespace JSC
