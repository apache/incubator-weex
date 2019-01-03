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

#include "CallMode.h"
#include "CodeLocation.h"
#include "CodeSpecializationKind.h"
#include "PolymorphicCallStubRoutine.h"
#include "WriteBarrier.h"
#include <wtf/SentinelLinkedList.h>

namespace JSC {

#if ENABLE(JIT)

class FunctionCodeBlock;
class JSFunction;
enum OpcodeID : unsigned;
struct CallFrameShuffleData;

class CallLinkInfo : public BasicRawSentinelNode<CallLinkInfo> {
public:
    enum CallType {
        None,
        Call,
        CallVarargs,
        Construct,
        ConstructVarargs,
        TailCall,
        TailCallVarargs,
        DirectCall,
        DirectConstruct,
        DirectTailCall
    };
    
    static CallType callTypeFor(OpcodeID opcodeID);

    static bool isVarargsCallType(CallType callType)
    {
        switch (callType) {
        case CallVarargs:
        case ConstructVarargs:
        case TailCallVarargs:
            return true;

        default:
            return false;
        }
    }

    CallLinkInfo();
        
    ~CallLinkInfo();
    
    static CodeSpecializationKind specializationKindFor(CallType callType)
    {
        return specializationFromIsConstruct(callType == Construct || callType == ConstructVarargs || callType == DirectConstruct);
    }
    CodeSpecializationKind specializationKind() const
    {
        return specializationKindFor(static_cast<CallType>(m_callType));
    }
    
    static CallMode callModeFor(CallType callType)
    {
        switch (callType) {
        case Call:
        case CallVarargs:
        case DirectCall:
            return CallMode::Regular;
        case TailCall:
        case TailCallVarargs:
        case DirectTailCall:
            return CallMode::Tail;
        case Construct:
        case ConstructVarargs:
        case DirectConstruct:
            return CallMode::Construct;
        case None:
            RELEASE_ASSERT_NOT_REACHED();
        }

        RELEASE_ASSERT_NOT_REACHED();
    }
    
    static bool isDirect(CallType callType)
    {
        switch (callType) {
        case DirectCall:
        case DirectTailCall:
        case DirectConstruct:
            return true;
        case Call:
        case CallVarargs:
        case TailCall:
        case TailCallVarargs:
        case Construct:
        case ConstructVarargs:
            return false;
        case None:
            RELEASE_ASSERT_NOT_REACHED();
            return false;
        }

        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }
    
    CallMode callMode() const
    {
        return callModeFor(static_cast<CallType>(m_callType));
    }

    bool isDirect()
    {
        return isDirect(static_cast<CallType>(m_callType));
    }

    bool isTailCall() const
    {
        return callMode() == CallMode::Tail;
    }
    
    NearCallMode nearCallMode() const
    {
        return isTailCall() ? Tail : Regular;
    }

    bool isVarargs() const
    {
        return isVarargsCallType(static_cast<CallType>(m_callType));
    }

    bool isLinked() { return m_stub || m_calleeOrCodeBlock; }
    void unlink(VM&);

    void setUpCall(CallType callType, CodeOrigin codeOrigin, unsigned calleeGPR)
    {
        m_callType = callType;
        m_codeOrigin = codeOrigin;
        m_calleeGPR = calleeGPR;
    }

    void setCallLocations(
        CodeLocationLabel callReturnLocationOrPatchableJump,
        CodeLocationLabel hotPathBeginOrSlowPathStart,
        CodeLocationNearCall hotPathOther)
    {
        m_callReturnLocationOrPatchableJump = callReturnLocationOrPatchableJump;
        m_hotPathBeginOrSlowPathStart = hotPathBeginOrSlowPathStart;
        m_hotPathOther = hotPathOther;
    }

    bool allowStubs() const { return m_allowStubs; }

    void disallowStubs()
    {
        m_allowStubs = false;
    }

    CodeLocationNearCall callReturnLocation();
    CodeLocationJump patchableJump();
    CodeLocationDataLabelPtr hotPathBegin();
    CodeLocationLabel slowPathStart();

    CodeLocationNearCall hotPathOther()
    {
        return m_hotPathOther;
    }

    void setCallee(VM&, JSCell*, JSFunction* callee);
    void clearCallee();
    JSFunction* callee();

    void setCodeBlock(VM&, JSCell*, FunctionCodeBlock*);
    void clearCodeBlock();
    FunctionCodeBlock* codeBlock();

    void setLastSeenCallee(VM& vm, const JSCell* owner, JSFunction* callee);
    void clearLastSeenCallee();
    JSFunction* lastSeenCallee();
    bool haveLastSeenCallee();
    
    void setExecutableDuringCompilation(ExecutableBase*);
    ExecutableBase* executable();
    
    void setStub(Ref<PolymorphicCallStubRoutine>&& newStub)
    {
        clearStub();
        m_stub = WTFMove(newStub);
    }

    void clearStub();

    PolymorphicCallStubRoutine* stub()
    {
        return m_stub.get();
    }

    void setSlowStub(Ref<JITStubRoutine>&& newSlowStub)
    {
        m_slowStub = WTFMove(newSlowStub);
    }

    void clearSlowStub()
    {
        m_slowStub = nullptr;
    }

    JITStubRoutine* slowStub()
    {
        return m_slowStub.get();
    }

    bool seenOnce()
    {
        return m_hasSeenShouldRepatch;
    }

    void clearSeen()
    {
        m_hasSeenShouldRepatch = false;
    }

    void setSeen()
    {
        m_hasSeenShouldRepatch = true;
    }

    bool hasSeenClosure()
    {
        return m_hasSeenClosure;
    }

    void setHasSeenClosure()
    {
        m_hasSeenClosure = true;
    }

    bool clearedByGC()
    {
        return m_clearedByGC;
    }

    void setCallType(CallType callType)
    {
        m_callType = callType;
    }

    CallType callType()
    {
        return static_cast<CallType>(m_callType);
    }

    uint32_t* addressOfMaxNumArguments()
    {
        return &m_maxNumArguments;
    }

    uint32_t maxNumArguments()
    {
        return m_maxNumArguments;
    }
    
    void setMaxNumArguments(unsigned);

    static ptrdiff_t offsetOfSlowPathCount()
    {
        return OBJECT_OFFSETOF(CallLinkInfo, m_slowPathCount);
    }

    void setCalleeGPR(unsigned calleeGPR)
    {
        m_calleeGPR = calleeGPR;
    }

    unsigned calleeGPR()
    {
        return m_calleeGPR;
    }

    uint32_t slowPathCount()
    {
        return m_slowPathCount;
    }

    void setCodeOrigin(CodeOrigin codeOrigin)
    {
        m_codeOrigin = codeOrigin;
    }

    CodeOrigin codeOrigin()
    {
        return m_codeOrigin;
    }

    void visitWeak(VM&);

    void setFrameShuffleData(const CallFrameShuffleData&);

    const CallFrameShuffleData* frameShuffleData()
    {
        return m_frameShuffleData.get();
    }

private:
    CodeLocationLabel m_callReturnLocationOrPatchableJump;
    CodeLocationLabel m_hotPathBeginOrSlowPathStart;
    CodeLocationNearCall m_hotPathOther;
    WriteBarrier<JSCell> m_calleeOrCodeBlock;
    WriteBarrier<JSCell> m_lastSeenCalleeOrExecutable;
    RefPtr<PolymorphicCallStubRoutine> m_stub;
    RefPtr<JITStubRoutine> m_slowStub;
    std::unique_ptr<CallFrameShuffleData> m_frameShuffleData;
    bool m_hasSeenShouldRepatch : 1;
    bool m_hasSeenClosure : 1;
    bool m_clearedByGC : 1;
    bool m_allowStubs : 1;
    bool m_isLinked : 1;
    unsigned m_callType : 4; // CallType
    unsigned m_calleeGPR : 8;
    uint32_t m_maxNumArguments; // For varargs: the profiled maximum number of arguments. For direct: the number of stack slots allocated for arguments.
    uint32_t m_slowPathCount;
    CodeOrigin m_codeOrigin;
};

inline CodeOrigin getCallLinkInfoCodeOrigin(CallLinkInfo& callLinkInfo)
{
    return callLinkInfo.codeOrigin();
}

typedef HashMap<CodeOrigin, CallLinkInfo*, CodeOriginApproximateHash> CallLinkInfoMap;

#else // ENABLE(JIT)

typedef HashMap<int, void*> CallLinkInfoMap;

#endif // ENABLE(JIT)

} // namespace JSC
