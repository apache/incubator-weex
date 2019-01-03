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

class WeakMapData;

class JSWeakMap : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    static JSWeakMap* create(VM& vm, Structure* structure)
    {
        JSWeakMap* instance = new (NotNull, allocateCell<JSWeakMap>(vm.heap)) JSWeakMap(vm, structure);
        instance->finishCreation(vm);
        return instance;
    }

    static JSWeakMap* create(ExecState* exec, Structure* structure)
    {
        return create(exec->vm(), structure);
    }

    WeakMapData* weakMapData() { return m_weakMapData.get(); }

    JSValue get(CallFrame*, JSObject*);
    bool has(CallFrame*, JSObject*);
    bool remove(CallFrame*, JSObject*);

    void set(CallFrame*, JSObject*, JSValue);
    void clear(CallFrame*);

private:
    JSWeakMap(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    void finishCreation(VM&);
    static void visitChildren(JSCell*, SlotVisitor&);
    static String toStringName(const JSObject*, ExecState*);

    WriteBarrier<WeakMapData> m_weakMapData;
};

} // namespace JSC
