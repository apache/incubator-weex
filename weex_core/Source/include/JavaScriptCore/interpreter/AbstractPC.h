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

#include "MacroAssemblerCodeRef.h"

namespace JSC {

class VM;
class ExecState;
struct Instruction;

class AbstractPC {
public:
    AbstractPC()
        : m_pointer(0)
        , m_mode(None)
    {
    }
    
    AbstractPC(VM&, ExecState*);
    
#if ENABLE(JIT)
    AbstractPC(ReturnAddressPtr ptr)
        : m_pointer(ptr.value())
        , m_mode(JIT)
    {
    }
    
    bool hasJITReturnAddress() const { return m_mode == JIT; }
    ReturnAddressPtr jitReturnAddress() const
    {
        ASSERT(hasJITReturnAddress());
        return ReturnAddressPtr(m_pointer);
    }
#endif

    bool isSet() const { return m_mode != None; }
    bool operator!() const { return !isSet(); }

private:
    void* m_pointer;
    
    enum Mode { None, JIT, Interpreter };
    Mode m_mode;
};

} // namespace JSC
