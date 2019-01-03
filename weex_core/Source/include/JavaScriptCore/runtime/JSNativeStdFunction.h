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

class JSGlobalObject;
class NativeStdFunctionCell;

typedef std::function<EncodedJSValue (ExecState*)> NativeStdFunction;

class JSNativeStdFunction : public JSFunction {
public:
    typedef JSFunction Base;

    const static unsigned StructureFlags = Base::StructureFlags;

    DECLARE_EXPORT_INFO;

    JS_EXPORT_PRIVATE static JSNativeStdFunction* create(VM&, JSGlobalObject*, int length, const String& name, NativeStdFunction&&, Intrinsic = NoIntrinsic, NativeFunction nativeConstructor = callHostFunctionAsConstructor);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        ASSERT(globalObject);
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSFunctionType, StructureFlags), info());
    }

    NativeStdFunctionCell* nativeStdFunctionCell() { return m_functionCell.get(); }

protected:
    static void visitChildren(JSCell*, SlotVisitor&);

    void finishCreation(VM&, NativeExecutable*, int length, const String& name, NativeStdFunctionCell*);

private:
    JSNativeStdFunction(VM&, JSGlobalObject*, Structure*);

    WriteBarrier<NativeStdFunctionCell> m_functionCell;
};

} // namespace JSC
