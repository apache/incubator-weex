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

#include "ExecutableBase.h"

namespace JSC {

class ScriptExecutable : public ExecutableBase {
public:
    typedef ExecutableBase Base;
    static const unsigned StructureFlags = Base::StructureFlags;

    static void destroy(JSCell*);
        
    CodeBlockHash hashFor(CodeSpecializationKind) const;

    const SourceCode& source() const { return m_source; }
    intptr_t sourceID() const { return m_source.providerID(); }
    const SourceOrigin& sourceOrigin() const { return m_source.provider()->sourceOrigin(); }
    const String& sourceURL() const { return m_source.provider()->url(); }
    int firstLine() const { return m_source.firstLine().oneBasedInt(); }
    void setOverrideLineNumber(int overrideLineNumber) { m_overrideLineNumber = overrideLineNumber; }
    bool hasOverrideLineNumber() const { return m_overrideLineNumber != -1; }
    int overrideLineNumber() const { return m_overrideLineNumber; }
    int lastLine() const { return m_lastLine; }
    unsigned startColumn() const { return m_source.startColumn().oneBasedInt(); }
    unsigned endColumn() const { return m_endColumn; }
    unsigned typeProfilingStartOffset() const { return m_typeProfilingStartOffset; }
    unsigned typeProfilingEndOffset() const { return m_typeProfilingEndOffset; }

    bool usesEval() const { return m_features & EvalFeature; }
    bool usesArguments() const { return m_features & ArgumentsFeature; }
    bool isArrowFunctionContext() const { return m_isArrowFunctionContext; }
    bool isStrictMode() const { return m_features & StrictModeFeature; }
    DerivedContextType derivedContextType() const { return static_cast<DerivedContextType>(m_derivedContextType); }
    EvalContextType evalContextType() const { return static_cast<EvalContextType>(m_evalContextType); }

    ECMAMode ecmaMode() const { return isStrictMode() ? StrictMode : NotStrictMode; }
        
    void setNeverInline(bool value) { m_neverInline = value; }
    void setNeverOptimize(bool value) { m_neverOptimize = value; }
    void setNeverFTLOptimize(bool value) { m_neverFTLOptimize = value; }
    void setDidTryToEnterInLoop(bool value) { m_didTryToEnterInLoop = value; }
    void setCanUseOSRExitFuzzing(bool value) { m_canUseOSRExitFuzzing = value; }
    bool neverInline() const { return m_neverInline; }
    bool neverOptimize() const { return m_neverOptimize; }
    bool neverFTLOptimize() const { return m_neverFTLOptimize; }
    bool didTryToEnterInLoop() const { return m_didTryToEnterInLoop; }
    bool isInliningCandidate() const { return !neverInline(); }
    bool isOkToOptimize() const { return !neverOptimize(); }
    bool canUseOSRExitFuzzing() const { return m_canUseOSRExitFuzzing; }
    
    bool* addressOfDidTryToEnterInLoop() { return &m_didTryToEnterInLoop; }

    CodeFeatures features() const { return m_features; }
        
    DECLARE_EXPORT_INFO;

    void recordParse(CodeFeatures features, bool hasCapturedVariables, int lastLine, unsigned endColumn)
    {
        m_features = features;
        m_hasCapturedVariables = hasCapturedVariables;
        m_lastLine = lastLine;
        ASSERT(endColumn != UINT_MAX);
        m_endColumn = endColumn;
    }

    void installCode(CodeBlock*);
    void installCode(VM&, CodeBlock*, CodeType, CodeSpecializationKind);
    CodeBlock* newCodeBlockFor(CodeSpecializationKind, JSFunction*, JSScope*, JSObject*& exception);
    CodeBlock* newReplacementCodeBlockFor(CodeSpecializationKind);

    // This function has an interesting GC story. Callers of this function are asking us to create a CodeBlock
    // that is not jettisoned before this function returns. Callers are essentially asking for a strong reference
    // to the CodeBlock. Because the Executable may be allocating the CodeBlock, we require callers to pass in
    // their CodeBlock*& reference because it's safe for CodeBlock to be jettisoned if Executable is the only thing
    // to point to it. This forces callers to have a CodeBlock* in a register or on the stack that will be marked
    // by conservative GC if a GC happens after we create the CodeBlock.
    template <typename ExecutableType>
    JSObject* prepareForExecution(VM&, JSFunction*, JSScope*, CodeSpecializationKind, CodeBlock*& resultCodeBlock);

private:
    friend class ExecutableBase;
    JSObject* prepareForExecutionImpl(VM&, JSFunction*, JSScope*, CodeSpecializationKind, CodeBlock*&);

protected:
    ScriptExecutable(Structure*, VM&, const SourceCode&, bool isInStrictContext, DerivedContextType, bool isInArrowFunctionContext, EvalContextType, Intrinsic);

    void finishCreation(VM& vm)
    {
        Base::finishCreation(vm);
        vm.heap.addExecutable(this); // Balanced by Heap::deleteUnmarkedCompiledCode().

#if ENABLE(CODEBLOCK_SAMPLING)
        if (SamplingTool* sampler = vm.interpreter->sampler())
            sampler->notifyOfScope(vm, this);
#endif
    }

    CodeFeatures m_features;
    bool m_didTryToEnterInLoop;
    bool m_hasCapturedVariables : 1;
    bool m_neverInline : 1;
    bool m_neverOptimize : 1;
    bool m_neverFTLOptimize : 1;
    bool m_isArrowFunctionContext : 1;
    bool m_canUseOSRExitFuzzing : 1;
    unsigned m_derivedContextType : 2; // DerivedContextType
    unsigned m_evalContextType : 2; // EvalContextType

    int m_overrideLineNumber;
    int m_lastLine;
    unsigned m_endColumn;
    unsigned m_typeProfilingStartOffset;
    unsigned m_typeProfilingEndOffset;
    SourceCode m_source;
};

} // namespace JSC
