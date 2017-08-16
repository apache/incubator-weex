/*
 * Copyright (C) 2009, 2010, 2013-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#pragma once

#include "ArityCheckMode.h"
#include "CallData.h"
#include "CodeBlockHash.h"
#include "CodeSpecializationKind.h"
#include "CompilationResult.h"
#include "ExecutableInfo.h"
#include "HandlerInfo.h"
#include "InferredValue.h"
#include "JITCode.h"
#include "JSGlobalObject.h"
#include "SourceCode.h"
#include "TypeSet.h"
#include "UnlinkedCodeBlock.h"
#include "UnlinkedFunctionExecutable.h"

namespace JSC {

class CodeBlock;
class EvalCodeBlock;
class FunctionCodeBlock;
class JSScope;
class JSWebAssemblyModule;
class LLIntOffsetsExtractor;
class ModuleProgramCodeBlock;
class ProgramCodeBlock;

enum CompilationKind { FirstCompilation, OptimizingCompilation };

inline bool isCall(CodeSpecializationKind kind)
{
    if (kind == CodeForCall)
        return true;
    ASSERT(kind == CodeForConstruct);
    return false;
}

class ExecutableBase : public JSCell {
    friend class JIT;

protected:
    static const int NUM_PARAMETERS_IS_HOST = 0;
    static const int NUM_PARAMETERS_NOT_COMPILED = -1;

    ExecutableBase(VM& vm, Structure* structure, int numParameters, Intrinsic intrinsic)
        : JSCell(vm, structure)
        , m_numParametersForCall(numParameters)
        , m_numParametersForConstruct(numParameters)
        , m_intrinsic(intrinsic)
    {
    }

    void finishCreation(VM& vm)
    {
        Base::finishCreation(vm);
    }

public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags;

    static const bool needsDestruction = true;
    static void destroy(JSCell*);
        
    CodeBlockHash hashFor(CodeSpecializationKind) const;

    bool isEvalExecutable() const
    {
        return type() == EvalExecutableType;
    }
    bool isFunctionExecutable() const
    {
        return type() == FunctionExecutableType;
    }
    bool isProgramExecutable() const
    {
        return type() == ProgramExecutableType;
    }
    bool isModuleProgramExecutable()
    {
        return type() == ModuleProgramExecutableType;
    }


    bool isHostFunction() const
    {
        ASSERT((m_numParametersForCall == NUM_PARAMETERS_IS_HOST) == (m_numParametersForConstruct == NUM_PARAMETERS_IS_HOST));
        return m_numParametersForCall == NUM_PARAMETERS_IS_HOST;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto) { return Structure::create(vm, globalObject, proto, TypeInfo(CellType, StructureFlags), info()); }
        
    void clearCode();

    DECLARE_EXPORT_INFO;

protected:
    int m_numParametersForCall;
    int m_numParametersForConstruct;

public:
    Ref<JITCode> generatedJITCodeForCall()
    {
        ASSERT(m_jitCodeForCall);
        return *m_jitCodeForCall;
    }

    Ref<JITCode> generatedJITCodeForConstruct()
    {
        ASSERT(m_jitCodeForConstruct);
        return *m_jitCodeForConstruct;
    }
        
    Ref<JITCode> generatedJITCodeFor(CodeSpecializationKind kind)
    {
        if (kind == CodeForCall)
            return generatedJITCodeForCall();
        ASSERT(kind == CodeForConstruct);
        return generatedJITCodeForConstruct();
    }
    
    MacroAssemblerCodePtr entrypointFor(CodeSpecializationKind kind, ArityCheckMode arity)
    {
        // Check if we have a cached result. We only have it for arity check because we use the
        // no-arity entrypoint in non-virtual calls, which will "cache" this value directly in
        // machine code.
        if (arity == MustCheckArity) {
            switch (kind) {
            case CodeForCall:
                if (MacroAssemblerCodePtr result = m_jitCodeForCallWithArityCheck)
                    return result;
                break;
            case CodeForConstruct:
                if (MacroAssemblerCodePtr result = m_jitCodeForConstructWithArityCheck)
                    return result;
                break;
            }
        }
        MacroAssemblerCodePtr result =
            generatedJITCodeFor(kind)->addressForCall(arity);
        if (arity == MustCheckArity) {
            // Cache the result; this is necessary for the JIT's virtual call optimizations.
            switch (kind) {
            case CodeForCall:
                m_jitCodeForCallWithArityCheck = result;
                break;
            case CodeForConstruct:
                m_jitCodeForConstructWithArityCheck = result;
                break;
            }
        }
        return result;
    }

    static ptrdiff_t offsetOfJITCodeWithArityCheckFor(
        CodeSpecializationKind kind)
    {
        switch (kind) {
        case CodeForCall:
            return OBJECT_OFFSETOF(ExecutableBase, m_jitCodeForCallWithArityCheck);
        case CodeForConstruct:
            return OBJECT_OFFSETOF(ExecutableBase, m_jitCodeForConstructWithArityCheck);
        }
        RELEASE_ASSERT_NOT_REACHED();
        return 0;
    }
    
    static ptrdiff_t offsetOfNumParametersFor(CodeSpecializationKind kind)
    {
        if (kind == CodeForCall)
            return OBJECT_OFFSETOF(ExecutableBase, m_numParametersForCall);
        ASSERT(kind == CodeForConstruct);
        return OBJECT_OFFSETOF(ExecutableBase, m_numParametersForConstruct);
    }

    bool hasJITCodeForCall() const
    {
        return m_numParametersForCall >= 0;
    }
        
    bool hasJITCodeForConstruct() const
    {
        return m_numParametersForConstruct >= 0;
    }
        
    bool hasJITCodeFor(CodeSpecializationKind kind) const
    {
        if (kind == CodeForCall)
            return hasJITCodeForCall();
        ASSERT(kind == CodeForConstruct);
        return hasJITCodeForConstruct();
    }

    // Intrinsics are only for calls, currently.
    Intrinsic intrinsic() const { return m_intrinsic; }
        
    Intrinsic intrinsicFor(CodeSpecializationKind kind) const
    {
        if (isCall(kind))
            return intrinsic();
        return NoIntrinsic;
    }
    
    void dump(PrintStream&) const;
        
protected:
    Intrinsic m_intrinsic;
    RefPtr<JITCode> m_jitCodeForCall;
    RefPtr<JITCode> m_jitCodeForConstruct;
    MacroAssemblerCodePtr m_jitCodeForCallWithArityCheck;
    MacroAssemblerCodePtr m_jitCodeForConstructWithArityCheck;
};

} // namespace JSC
