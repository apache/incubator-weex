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

#include "CallFrameClosure.h"
#include "ExceptionHelpers.h"
#include "JSFunction.h"
#include "JSGlobalObject.h"
#include "Interpreter.h"
#include "ProtoCallFrame.h"
#include "VMEntryScope.h"
#include "VMInlines.h"
#include <wtf/ForbidHeapAllocation.h>

namespace JSC {
    class CachedCall {
        WTF_MAKE_NONCOPYABLE(CachedCall);
        WTF_FORBID_HEAP_ALLOCATION;
    public:
        CachedCall(CallFrame* callFrame, JSFunction* function, int argumentCount)
            : m_valid(false)
            , m_interpreter(callFrame->interpreter())
            , m_vm(callFrame->vm())
            , m_entryScope(m_vm, function->scope()->globalObject(m_vm))
        {
            VM& vm = m_entryScope.vm();
            auto scope = DECLARE_THROW_SCOPE(vm);

            ASSERT(!function->isHostFunctionNonInline());
            if (UNLIKELY(vm.isSafeToRecurseSoft())) {
                m_arguments.ensureCapacity(argumentCount);
                m_closure = m_interpreter->prepareForRepeatCall(function->jsExecutable(), callFrame, &m_protoCallFrame, function, argumentCount + 1, function->scope(), m_arguments);
            } else
                throwStackOverflowError(callFrame, scope);
            m_valid = !scope.exception();
        }
        
        JSValue call()
        { 
            ASSERT(m_valid);
            ASSERT(m_arguments.size() == static_cast<size_t>(m_protoCallFrame.argumentCount()));
            return m_interpreter->execute(m_closure);
        }
        void setThis(JSValue v) { m_protoCallFrame.setThisValue(v); }

        void clearArguments() { m_arguments.clear(); }
        void appendArgument(JSValue v) { m_arguments.append(v); }

    private:
        bool m_valid;
        Interpreter* m_interpreter;
        VM& m_vm;
        VMEntryScope m_entryScope;
        ProtoCallFrame m_protoCallFrame;
        MarkedArgumentBuffer m_arguments;
        CallFrameClosure m_closure;
    };
}
