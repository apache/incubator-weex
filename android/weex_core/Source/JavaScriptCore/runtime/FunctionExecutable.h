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

#include "ScriptExecutable.h"

namespace JSC {

class FunctionExecutable final : public ScriptExecutable {
    friend class JIT;
    friend class LLIntOffsetsExtractor;
public:
    typedef ScriptExecutable Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static FunctionExecutable* create(
        VM& vm, const SourceCode& source, UnlinkedFunctionExecutable* unlinkedExecutable, 
        unsigned lastLine, unsigned endColumn, Intrinsic intrinsic)
    {
        FunctionExecutable* executable = new (NotNull, allocateCell<FunctionExecutable>(vm.heap)) FunctionExecutable(vm, source, unlinkedExecutable, lastLine, endColumn, intrinsic);
        executable->finishCreation(vm);
        return executable;
    }
    static FunctionExecutable* fromGlobalCode(
        const Identifier& name, ExecState&, const SourceCode&, 
        JSObject*& exception, int overrideLineNumber);

    static void destroy(JSCell*);
        
    UnlinkedFunctionExecutable* unlinkedExecutable() const
    {
        return m_unlinkedExecutable.get();
    }

    // Returns either call or construct bytecode. This can be appropriate
    // for answering questions that that don't vary between call and construct --
    // for example, argumentsRegister().
    FunctionCodeBlock* eitherCodeBlock()
    {
        if (m_codeBlockForCall)
            return m_codeBlockForCall.get();
        return m_codeBlockForConstruct.get();
    }
        
    bool isGeneratedForCall() const
    {
        return !!m_codeBlockForCall;
    }

    FunctionCodeBlock* codeBlockForCall()
    {
        return m_codeBlockForCall.get();
    }

    bool isGeneratedForConstruct() const
    {
        return m_codeBlockForConstruct.get();
    }

    FunctionCodeBlock* codeBlockForConstruct()
    {
        return m_codeBlockForConstruct.get();
    }
        
    bool isGeneratedFor(CodeSpecializationKind kind)
    {
        if (kind == CodeForCall)
            return isGeneratedForCall();
        ASSERT(kind == CodeForConstruct);
        return isGeneratedForConstruct();
    }
        
    FunctionCodeBlock* codeBlockFor(CodeSpecializationKind kind)
    {
        if (kind == CodeForCall)
            return codeBlockForCall();
        ASSERT(kind == CodeForConstruct);
        return codeBlockForConstruct();
    }

    FunctionCodeBlock* baselineCodeBlockFor(CodeSpecializationKind);
        
    FunctionCodeBlock* profiledCodeBlockFor(CodeSpecializationKind kind)
    {
        return baselineCodeBlockFor(kind);
    }

    RefPtr<TypeSet> returnStatementTypeSet() 
    {
        if (!m_returnStatementTypeSet)
            m_returnStatementTypeSet = TypeSet::create();

        return m_returnStatementTypeSet;
    }
        
    FunctionMode functionMode() { return m_unlinkedExecutable->functionMode(); }
    bool isBuiltinFunction() const { return m_unlinkedExecutable->isBuiltinFunction(); }
    ConstructAbility constructAbility() const { return m_unlinkedExecutable->constructAbility(); }
    bool isClass() const { return !classSource().isNull(); }
    bool isArrowFunction() const { return parseMode() == SourceParseMode::ArrowFunctionMode; }
    bool isGetter() const { return parseMode() == SourceParseMode::GetterMode; }
    bool isSetter() const { return parseMode() == SourceParseMode::SetterMode; }
    bool isGenerator() const { return SourceParseModeSet(SourceParseMode::GeneratorBodyMode, SourceParseMode::GeneratorWrapperFunctionMode).contains(parseMode()); }
    bool isMethod() const { return parseMode() == SourceParseMode::MethodMode; }
    bool hasCallerAndArgumentsProperties() const
    {
        // Per https://tc39.github.io/ecma262/#sec-forbidden-extensions, only sloppy-mode non-builtin functions in old-style (pre-ES6) syntactic forms can contain
        // "caller" and "arguments".
        return !isStrictMode() && parseMode() == SourceParseMode::NormalFunctionMode && !isClassConstructorFunction();
    }
    bool hasPrototypeProperty() const
    {
        return SourceParseModeSet(
            SourceParseMode::NormalFunctionMode,
            SourceParseMode::GeneratorBodyMode,
            SourceParseMode::GeneratorWrapperFunctionMode
        ).contains(parseMode()) || isClass();
    }
    DerivedContextType derivedContextType() const { return m_unlinkedExecutable->derivedContextType(); }
    bool isClassConstructorFunction() const { return m_unlinkedExecutable->isClassConstructorFunction(); }
    const Identifier& name() { return m_unlinkedExecutable->name(); }
    const Identifier& ecmaName() { return m_unlinkedExecutable->ecmaName(); }
    const Identifier& inferredName() { return m_unlinkedExecutable->inferredName(); }
    unsigned parameterCount() const { return m_unlinkedExecutable->parameterCount(); } // Excluding 'this'!
    SourceParseMode parseMode() const { return m_unlinkedExecutable->parseMode(); }
    JSParserScriptMode scriptMode() const { return m_unlinkedExecutable->scriptMode(); }
    const SourceCode& classSource() const { return m_unlinkedExecutable->classSource(); }

    static void visitChildren(JSCell*, SlotVisitor&);
    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
    {
        return Structure::create(vm, globalObject, proto, TypeInfo(FunctionExecutableType, StructureFlags), info());
    }

    unsigned parametersStartOffset() const { return m_parametersStartOffset; }

    void overrideParameterAndTypeProfilingStartEndOffsets(unsigned parametersStartOffset, unsigned typeProfilingStartOffset, unsigned typeProfilingEndOffset)
    {
        m_parametersStartOffset = parametersStartOffset;
        m_typeProfilingStartOffset = typeProfilingStartOffset;
        m_typeProfilingEndOffset = typeProfilingEndOffset;
    }

    DECLARE_INFO;

    InferredValue* singletonFunction() { return m_singletonFunction.get(); }

private:
    friend class ExecutableBase;
    FunctionExecutable(
        VM&, const SourceCode&, UnlinkedFunctionExecutable*,
        unsigned lastLine, unsigned endColumn, Intrinsic);
    
    void finishCreation(VM&);

    friend class ScriptExecutable;
    
    unsigned m_parametersStartOffset;
    WriteBarrier<UnlinkedFunctionExecutable> m_unlinkedExecutable;
    WriteBarrier<FunctionCodeBlock> m_codeBlockForCall;
    WriteBarrier<FunctionCodeBlock> m_codeBlockForConstruct;
    RefPtr<TypeSet> m_returnStatementTypeSet;
    WriteBarrier<InferredValue> m_singletonFunction;
};

} // namespace JSC
