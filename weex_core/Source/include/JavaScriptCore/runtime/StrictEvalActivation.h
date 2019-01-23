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

#include "JSScope.h"

namespace JSC {

class StrictEvalActivation : public JSScope {
public:
    typedef JSScope Base;
    static const unsigned StructureFlags = Base::StructureFlags | OverridesToThis;

    static StrictEvalActivation* create(ExecState* exec, JSScope* currentScope)
    {
        StrictEvalActivation* lexicalEnvironment = new (NotNull, allocateCell<StrictEvalActivation>(*exec->heap())) StrictEvalActivation(exec, currentScope);
        lexicalEnvironment->finishCreation(exec->vm());
        return lexicalEnvironment;
    }

    static bool deleteProperty(JSCell*, ExecState*, PropertyName);
    static JSValue toThis(JSCell*, ExecState*, ECMAMode);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(StrictEvalActivationType, StructureFlags), info());
    }
    
    DECLARE_INFO;

private:
    StrictEvalActivation(ExecState*, JSScope*);
};

} // namespace JSC
