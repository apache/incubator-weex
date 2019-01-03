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

#if ENABLE(DFG_JIT)

#include "CCallHelpers.h"
#include "CodeBlock.h"
#include "DFGDisassembler.h"
#include "DFGGraph.h"
#include "DFGInlineCacheWrapper.h"
#include "DFGJITCode.h"
#include "DFGOSRExitCompilationInfo.h"
#include "DFGRegisterBank.h"
#include "FPRInfo.h"
#include "GPRInfo.h"
#include "HandlerInfo.h"
#include "JITCode.h"
#include "JITInlineCacheGenerator.h"
#include "LinkBuffer.h"
#include "MacroAssembler.h"
#include "PCToCodeOriginMap.h"
#include "TempRegisterSet.h"

namespace JSC {

class AbstractSamplingCounter;
class CodeBlock;
class VM;

namespace DFG {

class JITCodeGenerator;
class NodeToRegisterMap;
class OSRExitJumpPlaceholder;
class SlowPathGenerator;
class SpeculativeJIT;
class SpeculationRecovery;

struct EntryLocation;
struct OSRExit;

// === CallLinkRecord ===
//
// A record of a call out from JIT code that needs linking to a helper function.
// Every CallLinkRecord contains a reference to the call instruction & the function
// that it needs to be linked to.
struct CallLinkRecord {
    CallLinkRecord(MacroAssembler::Call call, FunctionPtr function)
        : m_call(call)
        , m_function(function)
    {
    }

    MacroAssembler::Call m_call;
    FunctionPtr m_function;
};

struct InRecord {
    InRecord(
        MacroAssembler::PatchableJump jump, MacroAssembler::Label done,
        SlowPathGenerator* slowPathGenerator, StructureStubInfo* stubInfo)
        : m_jump(jump)
        , m_done(done)
        , m_slowPathGenerator(slowPathGenerator)
        , m_stubInfo(stubInfo)
    {
    }
    
    MacroAssembler::PatchableJump m_jump;
    MacroAssembler::Label m_done;
    SlowPathGenerator* m_slowPathGenerator;
    StructureStubInfo* m_stubInfo;
};

// === JITCompiler ===
//
// DFG::JITCompiler is responsible for generating JIT code from the dataflow graph.
// It does so by delegating to the speculative & non-speculative JITs, which
// generate to a MacroAssembler (which the JITCompiler owns through an inheritance
// relationship). The JITCompiler holds references to information required during
// compilation, and also records information used in linking (e.g. a list of all
// call to be linked).
class JITCompiler : public CCallHelpers {
public:
    JITCompiler(Graph& dfg);
    ~JITCompiler();
    
    void compile();
    void compileFunction();
    
    // Accessors for properties.
    Graph& graph() { return m_graph; }
    
    // Methods to set labels for the disassembler.
    void setStartOfCode()
    {
        m_pcToCodeOriginMapBuilder.appendItem(labelIgnoringWatchpoints(), CodeOrigin(0, nullptr));
        if (LIKELY(!m_disassembler))
            return;
        m_disassembler->setStartOfCode(labelIgnoringWatchpoints());
    }
    
    void setForBlockIndex(BlockIndex blockIndex)
    {
        if (LIKELY(!m_disassembler))
            return;
        m_disassembler->setForBlockIndex(blockIndex, labelIgnoringWatchpoints());
    }
    
    void setForNode(Node* node)
    {
        if (LIKELY(!m_disassembler))
            return;
        m_disassembler->setForNode(node, labelIgnoringWatchpoints());
    }
    
    void setEndOfMainPath();
    void setEndOfCode();
    
    CallSiteIndex addCallSite(CodeOrigin codeOrigin)
    {
        return m_jitCode->common.addCodeOrigin(codeOrigin);
    }

    CallSiteIndex emitStoreCodeOrigin(CodeOrigin codeOrigin)
    {
        CallSiteIndex callSite = addCallSite(codeOrigin);
        emitStoreCallSiteIndex(callSite);
        return callSite;
    }

    void emitStoreCallSiteIndex(CallSiteIndex callSite)
    {
        store32(TrustedImm32(callSite.bits()), tagFor(static_cast<VirtualRegister>(CallFrameSlot::argumentCount)));
    }

    // Add a call out from JIT code, without an exception check.
    Call appendCall(const FunctionPtr& function)
    {
        Call functionCall = call();
        m_calls.append(CallLinkRecord(functionCall, function));
        return functionCall;
    }
    
    void exceptionCheck();

    void exceptionCheckWithCallFrameRollback()
    {
        m_exceptionChecksWithCallFrameRollback.append(emitExceptionCheck());
    }

    // Add a call out from JIT code, with a fast exception check that tests if the return value is zero.
    void fastExceptionCheck()
    {
        callExceptionFuzz();
        m_exceptionChecks.append(branchTestPtr(Zero, GPRInfo::returnValueGPR));
    }
    
    OSRExitCompilationInfo& appendExitInfo(MacroAssembler::JumpList jumpsToFail = MacroAssembler::JumpList())
    {
        OSRExitCompilationInfo info;
        info.m_failureJumps = jumpsToFail;
        m_exitCompilationInfo.append(info);
        return m_exitCompilationInfo.last();
    }

#if USE(JSVALUE32_64)
    void* addressOfDoubleConstant(Node*);
#endif

    void addGetById(const JITGetByIdGenerator& gen, SlowPathGenerator* slowPath)
    {
        m_getByIds.append(InlineCacheWrapper<JITGetByIdGenerator>(gen, slowPath));
    }
    
    void addGetByIdWithThis(const JITGetByIdWithThisGenerator& gen, SlowPathGenerator* slowPath)
    {
        m_getByIdsWithThis.append(InlineCacheWrapper<JITGetByIdWithThisGenerator>(gen, slowPath));
    }
    
    void addPutById(const JITPutByIdGenerator& gen, SlowPathGenerator* slowPath)
    {
        m_putByIds.append(InlineCacheWrapper<JITPutByIdGenerator>(gen, slowPath));
    }

    void addIn(const InRecord& record)
    {
        m_ins.append(record);
    }
    
    void addJSCall(Call fastCall, Call slowCall, DataLabelPtr targetToCheck, CallLinkInfo* info)
    {
        m_jsCalls.append(JSCallRecord(fastCall, slowCall, targetToCheck, info));
    }
    
    void addJSDirectCall(Call call, Label slowPath, CallLinkInfo* info)
    {
        m_jsDirectCalls.append(JSDirectCallRecord(call, slowPath, info));
    }
    
    void addJSDirectTailCall(PatchableJump patchableJump, Call call, Label slowPath, CallLinkInfo* info)
    {
        m_jsDirectTailCalls.append(JSDirectTailCallRecord(patchableJump, call, slowPath, info));
    }
    
    void addWeakReference(JSCell* target)
    {
        m_graph.m_plan.weakReferences.addLazily(target);
    }
    
    void addWeakReferences(const StructureSet& structureSet)
    {
        for (unsigned i = structureSet.size(); i--;)
            addWeakReference(structureSet[i]);
    }
    
    template<typename T>
    Jump branchWeakPtr(RelationalCondition cond, T left, JSCell* weakPtr)
    {
        Jump result = branchPtr(cond, left, TrustedImmPtr(weakPtr));
        addWeakReference(weakPtr);
        return result;
    }

    template<typename T>
    Jump branchWeakStructure(RelationalCondition cond, T left, RegisteredStructure weakStructure)
    {
        Structure* structure = weakStructure.get();
#if USE(JSVALUE64)
        Jump result = branch32(cond, left, TrustedImm32(structure->id()));
        return result;
#else
        return branchPtr(cond, left, TrustedImmPtr(structure));
#endif
    }

    void noticeOSREntry(BasicBlock&, JITCompiler::Label blockHead, LinkBuffer&);
    
    RefPtr<JITCode> jitCode() { return m_jitCode; }
    
    Vector<Label>& blockHeads() { return m_blockHeads; }

    CallSiteIndex recordCallSiteAndGenerateExceptionHandlingOSRExitIfNeeded(const CodeOrigin&, unsigned eventStreamIndex);

    PCToCodeOriginMapBuilder& pcToCodeOriginMapBuilder() { return m_pcToCodeOriginMapBuilder; }

private:
    friend class OSRExitJumpPlaceholder;
    
    // Internal implementation to compile.
    void compileEntry();
    void compileSetupRegistersForEntry();
    void compileEntryExecutionFlag();
    void compileBody();
    void link(LinkBuffer&);
    
    void exitSpeculativeWithOSR(const OSRExit&, SpeculationRecovery*);
    void compileExceptionHandlers();
    void linkOSRExits();
    void disassemble(LinkBuffer&);

    void appendExceptionHandlingOSRExit(ExitKind, unsigned eventStreamIndex, CodeOrigin, HandlerInfo* exceptionHandler, CallSiteIndex, MacroAssembler::JumpList jumpsToFail = MacroAssembler::JumpList());

    // The dataflow graph currently being generated.
    Graph& m_graph;

    std::unique_ptr<Disassembler> m_disassembler;
    
    RefPtr<JITCode> m_jitCode;
    
    // Vector of calls out from JIT code, including exception handler information.
    // Count of the number of CallRecords with exception handlers.
    Vector<CallLinkRecord> m_calls;
    JumpList m_exceptionChecks;
    JumpList m_exceptionChecksWithCallFrameRollback;
    
    Vector<Label> m_blockHeads;

    struct JSCallRecord {
        JSCallRecord(Call fastCall, Call slowCall, DataLabelPtr targetToCheck, CallLinkInfo* info)
            : fastCall(fastCall)
            , slowCall(slowCall)
            , targetToCheck(targetToCheck)
            , info(info)
        {
        }
        
        Call fastCall;
        Call slowCall;
        DataLabelPtr targetToCheck;
        CallLinkInfo* info;
    };
    
    struct JSDirectCallRecord {
        JSDirectCallRecord(Call call, Label slowPath, CallLinkInfo* info)
            : call(call)
            , slowPath(slowPath)
            , info(info)
        {
        }
        
        Call call;
        Label slowPath;
        CallLinkInfo* info;
    };
    
    struct JSDirectTailCallRecord {
        JSDirectTailCallRecord(PatchableJump patchableJump, Call call, Label slowPath, CallLinkInfo* info)
            : patchableJump(patchableJump)
            , call(call)
            , slowPath(slowPath)
            , info(info)
        {
        }
        
        PatchableJump patchableJump;
        Call call;
        Label slowPath;
        CallLinkInfo* info;
    };
    
    Vector<InlineCacheWrapper<JITGetByIdGenerator>, 4> m_getByIds;
    Vector<InlineCacheWrapper<JITGetByIdWithThisGenerator>, 4> m_getByIdsWithThis;
    Vector<InlineCacheWrapper<JITPutByIdGenerator>, 4> m_putByIds;
    Vector<InRecord, 4> m_ins;
    Vector<JSCallRecord, 4> m_jsCalls;
    Vector<JSDirectCallRecord, 4> m_jsDirectCalls;
    Vector<JSDirectTailCallRecord, 4> m_jsDirectTailCalls;
    SegmentedVector<OSRExitCompilationInfo, 4> m_exitCompilationInfo;
    Vector<Vector<Label>> m_exitSiteLabels;
    
    struct ExceptionHandlingOSRExitInfo {
        OSRExitCompilationInfo& exitInfo;
        HandlerInfo baselineExceptionHandler;
        CallSiteIndex callSiteIndex;
    };
    Vector<ExceptionHandlingOSRExitInfo> m_exceptionHandlerOSRExitCallSites;
    
    Call m_callArityFixup;
    Label m_arityCheck;
    std::unique_ptr<SpeculativeJIT> m_speculative;
    PCToCodeOriginMapBuilder m_pcToCodeOriginMapBuilder;
};

} } // namespace JSC::DFG

#endif
