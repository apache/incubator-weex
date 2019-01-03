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

#if ENABLE(JIT)

#include "CodeOrigin.h"
#include "JITOperations.h"
#include "JSCJSValue.h"
#include "PutKind.h"
#include "RegisterSet.h"

namespace JSC {

class CodeBlock;
class StructureStubInfo;

struct CallSiteIndex;

enum class AccessType : int8_t;

class JITInlineCacheGenerator {
protected:
    JITInlineCacheGenerator() { }
    JITInlineCacheGenerator(CodeBlock*, CodeOrigin, CallSiteIndex, AccessType);
    
public:
    StructureStubInfo* stubInfo() const { return m_stubInfo; }

protected:
    CodeBlock* m_codeBlock;
    StructureStubInfo* m_stubInfo;
};

class JITByIdGenerator : public JITInlineCacheGenerator {
protected:
    JITByIdGenerator() { }

    JITByIdGenerator(
        CodeBlock*, CodeOrigin, CallSiteIndex, AccessType, const RegisterSet&, JSValueRegs base,
        JSValueRegs value);
    
public:
    void reportSlowPathCall(MacroAssembler::Label slowPathBegin, MacroAssembler::Call call)
    {
        m_slowPathBegin = slowPathBegin;
        m_slowPathCall = call;
    }
    
    MacroAssembler::Label slowPathBegin() const { return m_slowPathBegin; }
    MacroAssembler::Jump slowPathJump() const
    {
        ASSERT(m_slowPathJump.isSet());
        return m_slowPathJump;
    }

    void finalize(LinkBuffer& fastPathLinkBuffer, LinkBuffer& slowPathLinkBuffer);
    void finalize(LinkBuffer&);
    
protected:
    void generateFastCommon(MacroAssembler&, size_t size);
    
    JSValueRegs m_base;
    JSValueRegs m_value;
    
    MacroAssembler::Label m_start;
    MacroAssembler::Label m_done;
    MacroAssembler::Label m_slowPathBegin;
    MacroAssembler::Call m_slowPathCall;
    MacroAssembler::Jump m_slowPathJump;
};

class JITGetByIdGenerator : public JITByIdGenerator {
public:
    JITGetByIdGenerator() { }

    JITGetByIdGenerator(
        CodeBlock*, CodeOrigin, CallSiteIndex, const RegisterSet& usedRegisters, UniquedStringImpl* propertyName,
        JSValueRegs base, JSValueRegs value, AccessType);
    
    void generateFastPath(MacroAssembler&);

private:
    bool m_isLengthAccess;
};

class JITGetByIdWithThisGenerator : public JITByIdGenerator {
public:
    JITGetByIdWithThisGenerator() { }

    JITGetByIdWithThisGenerator(
        CodeBlock*, CodeOrigin, CallSiteIndex, const RegisterSet& usedRegisters, UniquedStringImpl* propertyName,
        JSValueRegs value, JSValueRegs base, JSValueRegs thisRegs, AccessType);

    void generateFastPath(MacroAssembler&);
};

class JITPutByIdGenerator : public JITByIdGenerator {
public:
    JITPutByIdGenerator() { }

    JITPutByIdGenerator(
        CodeBlock*, CodeOrigin, CallSiteIndex, const RegisterSet& usedRegisters, JSValueRegs base,
        JSValueRegs, GPRReg scratch, ECMAMode, PutKind);
    
    void generateFastPath(MacroAssembler&);
    
    V_JITOperation_ESsiJJI slowPathFunction();

private:
    ECMAMode m_ecmaMode;
    PutKind m_putKind;
};

} // namespace JSC

#endif // ENABLE(JIT)
