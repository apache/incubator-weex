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

#include "GPRInfo.h"

namespace JSC {

struct VMEntryFrame;
class ExecState;
class VM;

struct VMEntryRecord {
    /*
     * This record stored in a vmEntryTo{JavaScript,Host} allocated frame. It is allocated on the stack
     * after callee save registers where local variables would go.
     */
    VM* m_vm;
    ExecState* m_prevTopCallFrame;
    VMEntryFrame* m_prevTopVMEntryFrame;

#if ENABLE(JIT) && NUMBER_OF_CALLEE_SAVES_REGISTERS > 0
    intptr_t calleeSaveRegistersBuffer[NUMBER_OF_CALLEE_SAVES_REGISTERS];
#endif

    ExecState* prevTopCallFrame() { return m_prevTopCallFrame; }
    SUPPRESS_ASAN ExecState* unsafePrevTopCallFrame() { return m_prevTopCallFrame; }

    VMEntryFrame* prevTopVMEntryFrame() { return m_prevTopVMEntryFrame; }
    SUPPRESS_ASAN VMEntryFrame* unsafePrevTopVMEntryFrame() { return m_prevTopVMEntryFrame; }
};

extern "C" VMEntryRecord* vmEntryRecord(VMEntryFrame*);

struct VMEntryFrame {
#if ENABLE(JIT) && NUMBER_OF_CALLEE_SAVES_REGISTERS > 0
    static ptrdiff_t vmEntryRecordOffset()
    {
        VMEntryFrame* fakeVMEntryFrame = reinterpret_cast<VMEntryFrame*>(0x1000);
        VMEntryRecord* record = vmEntryRecord(fakeVMEntryFrame);
        return static_cast<ptrdiff_t>(
            reinterpret_cast<char*>(record) - reinterpret_cast<char*>(fakeVMEntryFrame));
    }

    static ptrdiff_t calleeSaveRegistersBufferOffset()
    {
        return vmEntryRecordOffset() + OBJECT_OFFSETOF(VMEntryRecord, calleeSaveRegistersBuffer);
    }
#endif
};

} // namespace JSC
