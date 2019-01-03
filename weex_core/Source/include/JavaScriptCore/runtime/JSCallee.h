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

#include "JSGlobalObject.h"
#include "JSObject.h"
#include "JSScope.h"

namespace JSC {

class JSGlobalObject;
class LLIntOffsetsExtractor;


class JSCallee : public JSNonFinalObject {
    friend class JIT;
#if ENABLE(DFG_JIT)
    friend class DFG::SpeculativeJIT;
    friend class DFG::JITCompiler;
#endif
    friend class VM;

public:
    typedef JSNonFinalObject Base;
    const static unsigned StructureFlags = Base::StructureFlags | ImplementsHasInstance | ImplementsDefaultHasInstance;

    static JSCallee* create(VM& vm, JSGlobalObject* globalObject, JSScope* scope)
    {
        JSCallee* callee = new (NotNull, allocateCell<JSCallee>(vm.heap)) JSCallee(vm, scope, globalObject->calleeStructure());
        callee->finishCreation(vm);
        return callee;
    }
    
    JSScope* scope()
    {
        return m_scope.get();
    }

    // This method may be called for host functions, in which case it
    // will return an arbitrary value. This should only be used for
    // optimized paths in which the return value does not matter for
    // host functions, and checking whether the function is a host
    // function is deemed too expensive.
    JSScope* scopeUnchecked()
    {
        return m_scope.get();
    }

    void setScope(VM& vm, JSScope* scope)
    {
        m_scope.set(vm, this, scope);
    }

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype) 
    {
        ASSERT(globalObject);
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSCalleeType, StructureFlags), info());
    }

    static inline ptrdiff_t offsetOfScopeChain()
    {
        return OBJECT_OFFSETOF(JSCallee, m_scope);
    }

protected:
    JS_EXPORT_PRIVATE JSCallee(VM&, JSGlobalObject*, Structure*);
    JSCallee(VM&, JSScope*, Structure*);

    void finishCreation(VM&);
    using Base::finishCreation;

    static void visitChildren(JSCell*, SlotVisitor&);

private:
    friend class LLIntOffsetsExtractor;

    WriteBarrier<JSScope> m_scope;
};

} // namespace JSC
