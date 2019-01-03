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

#include "JSCell.h"
#include "Structure.h"

namespace JSC {

class JSPromiseConstructor;

class JSPromiseDeferred : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    JS_EXPORT_PRIVATE static JSPromiseDeferred* create(ExecState*, JSGlobalObject*);
    JS_EXPORT_PRIVATE static JSPromiseDeferred* create(VM&, JSObject* promise, JSValue resolve, JSValue reject);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

    DECLARE_EXPORT_INFO;

    JSObject* promise() const { return m_promise.get(); }
    JSValue resolve() const { return m_resolve.get(); }
    JSValue reject() const { return m_reject.get(); }

    JS_EXPORT_PRIVATE void resolve(ExecState*, JSValue);
    JS_EXPORT_PRIVATE void reject(ExecState*, JSValue);

protected:
    JSPromiseDeferred(VM&, Structure*);
    void finishCreation(VM&, JSObject*, JSValue, JSValue);
    static void visitChildren(JSCell*, SlotVisitor&);

private:
    JSPromiseDeferred(VM&);

    WriteBarrier<JSObject> m_promise;
    WriteBarrier<Unknown> m_resolve;
    WriteBarrier<Unknown> m_reject;
};

JSValue newPromiseCapability(ExecState*, JSGlobalObject*, JSPromiseConstructor*);

} // namespace JSC
