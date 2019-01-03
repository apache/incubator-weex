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

#include "ArityCheckMode.h"
#include "CallFrame.h"
#include "CodeOrigin.h"
#include "Disassembler.h"
#include "JSCJSValue.h"
#include "MacroAssemblerCodeRef.h"
#include "RegisterSet.h"
#include <wtf/Optional.h>

namespace JSC {

namespace DFG {
class CommonData;
class JITCode;
}
namespace FTL {
class ForOSREntryJITCode;
class JITCode;
}

struct ProtoCallFrame;
class TrackedReferences;
class VM;

class JITCode : public ThreadSafeRefCounted<JITCode> {
public:
    typedef MacroAssemblerCodeRef CodeRef;
    typedef MacroAssemblerCodePtr CodePtr;

    enum JITType : uint8_t {
        None,
        HostCallThunk,
        InterpreterThunk,
        BaselineJIT,
        DFGJIT,
        FTLJIT
    };
    
    static const char* typeName(JITType);

    static JITType bottomTierJIT()
    {
        return BaselineJIT;
    }
    
    static JITType topTierJIT()
    {
        return FTLJIT;
    }
    
    static JITType nextTierJIT(JITType jitType)
    {
        switch (jitType) {
        case BaselineJIT:
            return DFGJIT;
        case DFGJIT:
            return FTLJIT;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return None;
        }
    }
    
    static bool isExecutableScript(JITType jitType)
    {
        switch (jitType) {
        case None:
        case HostCallThunk:
            return false;
        default:
            return true;
        }
    }
    
    static bool couldBeInterpreted(JITType jitType)
    {
        switch (jitType) {
        case InterpreterThunk:
        case BaselineJIT:
            return true;
        default:
            return false;
        }
    }
    
    static bool isJIT(JITType jitType)
    {
        switch (jitType) {
        case BaselineJIT:
        case DFGJIT:
        case FTLJIT:
            return true;
        default:
            return false;
        }
    }

    static bool isLowerTier(JITType expectedLower, JITType expectedHigher)
    {
        RELEASE_ASSERT(isExecutableScript(expectedLower));
        RELEASE_ASSERT(isExecutableScript(expectedHigher));
        return expectedLower < expectedHigher;
    }
    
    static bool isHigherTier(JITType expectedHigher, JITType expectedLower)
    {
        return isLowerTier(expectedLower, expectedHigher);
    }
    
    static bool isLowerOrSameTier(JITType expectedLower, JITType expectedHigher)
    {
        return !isHigherTier(expectedLower, expectedHigher);
    }
    
    static bool isHigherOrSameTier(JITType expectedHigher, JITType expectedLower)
    {
        return isLowerOrSameTier(expectedLower, expectedHigher);
    }
    
    static bool isOptimizingJIT(JITType jitType)
    {
        return jitType == DFGJIT || jitType == FTLJIT;
    }
    
    static bool isBaselineCode(JITType jitType)
    {
        return jitType == InterpreterThunk || jitType == BaselineJIT;
    }
    
protected:
    JITCode(JITType);
    
public:
    virtual ~JITCode();
    
    JITType jitType() const
    {
        return m_jitType;
    }
    
    template<typename PointerType>
    static JITType jitTypeFor(PointerType jitCode)
    {
        if (!jitCode)
            return None;
        return jitCode->jitType();
    }
    
    virtual CodePtr addressForCall(ArityCheckMode) = 0;
    virtual void* executableAddressAtOffset(size_t offset) = 0;
    void* executableAddress() { return executableAddressAtOffset(0); }
    virtual void* dataAddressAtOffset(size_t offset) = 0;
    virtual unsigned offsetOf(void* pointerIntoCode) = 0;
    
    virtual DFG::CommonData* dfgCommon();
    virtual DFG::JITCode* dfg();
    virtual FTL::JITCode* ftl();
    virtual FTL::ForOSREntryJITCode* ftlForOSREntry();
    
    virtual void validateReferences(const TrackedReferences&);
    
    JSValue execute(VM*, ProtoCallFrame*);
    
    void* start() { return dataAddressAtOffset(0); }
    virtual size_t size() = 0;
    void* end() { return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(start()) + size()); }
    
    virtual bool contains(void*) = 0;

#if ENABLE(JIT)
    virtual RegisterSet liveRegistersToPreserveAtExceptionHandlingCallSite(CodeBlock*, CallSiteIndex);
    virtual std::optional<CodeOrigin> findPC(CodeBlock*, void* pc) { UNUSED_PARAM(pc); return std::nullopt; }
#endif

private:
    JITType m_jitType;
};

class JITCodeWithCodeRef : public JITCode {
protected:
    JITCodeWithCodeRef(JITType);
    JITCodeWithCodeRef(CodeRef, JITType);

public:
    virtual ~JITCodeWithCodeRef();

    void* executableAddressAtOffset(size_t offset) override;
    void* dataAddressAtOffset(size_t offset) override;
    unsigned offsetOf(void* pointerIntoCode) override;
    size_t size() override;
    bool contains(void*) override;

protected:
    CodeRef m_ref;
};

class DirectJITCode : public JITCodeWithCodeRef {
public:
    DirectJITCode(JITType);
    DirectJITCode(CodeRef, CodePtr withArityCheck, JITType);
    virtual ~DirectJITCode();
    
    void initializeCodeRef(CodeRef, CodePtr withArityCheck);

    CodePtr addressForCall(ArityCheckMode) override;

private:
    CodePtr m_withArityCheck;
};

class NativeJITCode : public JITCodeWithCodeRef {
public:
    NativeJITCode(JITType);
    NativeJITCode(CodeRef, JITType);
    virtual ~NativeJITCode();
    
    void initializeCodeRef(CodeRef);

    CodePtr addressForCall(ArityCheckMode) override;
};

} // namespace JSC

namespace WTF {

class PrintStream;
void printInternal(PrintStream&, JSC::JITCode::JITType);

} // namespace WTF
