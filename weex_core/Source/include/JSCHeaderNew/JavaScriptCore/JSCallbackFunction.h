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
#ifndef JSCallbackFunction_h
#define JSCallbackFunction_h

#include "InternalFunction.h"
#include "JSCPoison.h"
#include "JSObjectRef.h"

namespace JSC {

class JSCallbackFunction final : public InternalFunction {
    friend struct APICallbackFunction;
public:
    typedef InternalFunction Base;

    template<typename CellType>
    static IsoSubspace* subspaceFor(VM& vm)
    {
        return &vm.callbackFunctionSpace;
    }

    static JSCallbackFunction* create(VM&, JSGlobalObject*, JSObjectCallAsFunctionCallback, const String& name);

    DECLARE_INFO;
    
    // InternalFunction mish-mashes constructor and function behavior -- we should 
    // refactor the code so this override isn't necessary
    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto) 
    { 
        return Structure::create(vm, globalObject, proto, TypeInfo(InternalFunctionType, StructureFlags), info()); 
    }

private:
    JSCallbackFunction(VM&, Structure*, JSObjectCallAsFunctionCallback);
    void finishCreation(VM&, const String& name);

    JSObjectCallAsFunctionCallback functionCallback() { return m_callback.unpoisoned(); }

    Poisoned<NativeCodePoison, JSObjectCallAsFunctionCallback> m_callback;
};

} // namespace JSC

#endif // JSCallbackFunction_h
