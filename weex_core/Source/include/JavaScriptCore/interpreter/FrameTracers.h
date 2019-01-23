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

#include "CatchScope.h"
#include "VM.h"

namespace JSC {

struct VMEntryFrame;

class SuspendExceptionScope {
public:
    SuspendExceptionScope(VM* vm)
        : m_vm(vm)
    {
        auto scope = DECLARE_CATCH_SCOPE(*vm);
        oldException = scope.exception();
        scope.clearException();
    }
    ~SuspendExceptionScope()
    {
        m_vm->restorePreviousException(oldException);
    }
private:
    Exception* oldException;
    VM* m_vm;
};

class TopCallFrameSetter {
public:
    TopCallFrameSetter(VM& currentVM, CallFrame* callFrame)
        : vm(currentVM)
        , oldCallFrame(currentVM.topCallFrame)
    {
        currentVM.topCallFrame = callFrame;
    }

    ~TopCallFrameSetter()
    {
        vm.topCallFrame = oldCallFrame;
    }
private:
    VM& vm;
    CallFrame* oldCallFrame;
};

class NativeCallFrameTracer {
public:
    ALWAYS_INLINE NativeCallFrameTracer(VM* vm, CallFrame* callFrame)
    {
        ASSERT(vm);
        ASSERT(callFrame);
        ASSERT(reinterpret_cast<void*>(callFrame) < reinterpret_cast<void*>(vm->topVMEntryFrame));
        vm->topCallFrame = callFrame;
    }
};

class NativeCallFrameTracerWithRestore {
public:
    ALWAYS_INLINE NativeCallFrameTracerWithRestore(VM* vm, VMEntryFrame* vmEntryFrame, CallFrame* callFrame)
        : m_vm(vm)
    {
        ASSERT(vm);
        ASSERT(callFrame);
        m_savedTopVMEntryFrame = vm->topVMEntryFrame;
        m_savedTopCallFrame = vm->topCallFrame;
        vm->topVMEntryFrame = vmEntryFrame;
        vm->topCallFrame = callFrame;
    }

    ALWAYS_INLINE ~NativeCallFrameTracerWithRestore()
    {
        m_vm->topVMEntryFrame = m_savedTopVMEntryFrame;
        m_vm->topCallFrame = m_savedTopCallFrame;
    }

private:
    VM* m_vm;
    VMEntryFrame* m_savedTopVMEntryFrame;
    CallFrame* m_savedTopCallFrame;
};

}
