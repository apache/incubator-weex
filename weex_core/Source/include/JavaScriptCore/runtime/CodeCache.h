/*
 * Copyright (C) 2012, 2013 Apple Inc. All Rights Reserved.
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

#include "BytecodeGenerator.h"
#include "ExecutableInfo.h"
#include "JSCInlines.h"
#include "Parser.h"
#include "ParserModes.h"
#include "SourceCodeKey.h"
#include "Strong.h"
#include "StrongInlines.h"
#include "UnlinkedCodeBlock.h"
#include "UnlinkedEvalCodeBlock.h"
#include "UnlinkedModuleProgramCodeBlock.h"
#include "UnlinkedProgramCodeBlock.h"
#include "UnlinkedSourceCode.h"
#include <wtf/CurrentTime.h>
#include <wtf/Forward.h>
#include <wtf/text/WTFString.h>

namespace JSC {

class EvalExecutable;
class IndirectEvalExecutable;
class Identifier;
class DirectEvalExecutable;
class ModuleProgramExecutable;
class ParserError;
class ProgramExecutable;
class SourceCode;
class UnlinkedCodeBlock;
class UnlinkedEvalCodeBlock;
class UnlinkedFunctionExecutable;
class UnlinkedModuleProgramCodeBlock;
class UnlinkedProgramCodeBlock;
class VM;
class VariableEnvironment;

struct SourceCodeValue {
    SourceCodeValue()
    {
    }

    SourceCodeValue(VM& vm, JSCell* cell, int64_t age)
        : cell(vm, cell)
        , age(age)
    {
    }

    Strong<JSCell> cell;
    int64_t age;
};

class CodeCacheMap {
public:
    typedef HashMap<SourceCodeKey, SourceCodeValue, SourceCodeKey::Hash, SourceCodeKey::HashTraits> MapType;
    typedef MapType::iterator iterator;
    typedef MapType::AddResult AddResult;

    CodeCacheMap()
        : m_size(0)
        , m_sizeAtLastPrune(0)
        , m_timeAtLastPrune(monotonicallyIncreasingTime())
        , m_minCapacity(0)
        , m_capacity(0)
        , m_age(0)
    {
    }

    SourceCodeValue* findCacheAndUpdateAge(const SourceCodeKey& key)
    {
        prune();

        iterator findResult = m_map.find(key);
        if (findResult == m_map.end())
            return nullptr;

        int64_t age = m_age - findResult->value.age;
        if (age > m_capacity) {
            // A requested object is older than the cache's capacity. We can
            // infer that requested objects are subject to high eviction probability,
            // so we grow the cache to improve our hit rate.
            m_capacity += recencyBias * oldObjectSamplingMultiplier * key.length();
        } else if (age < m_capacity / 2) {
            // A requested object is much younger than the cache's capacity. We can
            // infer that requested objects are subject to low eviction probability,
            // so we shrink the cache to save memory.
            m_capacity -= recencyBias * key.length();
            if (m_capacity < m_minCapacity)
                m_capacity = m_minCapacity;
        }

        findResult->value.age = m_age;
        m_age += key.length();

        return &findResult->value;
    }

    AddResult addCache(const SourceCodeKey& key, const SourceCodeValue& value)
    {
        prune();

        AddResult addResult = m_map.add(key, value);
        ASSERT(addResult.isNewEntry);

        m_size += key.length();
        m_age += key.length();
        return addResult;
    }

    void remove(iterator it)
    {
        m_size -= it->key.length();
        m_map.remove(it);
    }

    void clear()
    {
        m_size = 0;
        m_age = 0;
        m_map.clear();
    }

    int64_t age() { return m_age; }

private:
    // This constant factor biases cache capacity toward allowing a minimum
    // working set to enter the cache before it starts evicting.
    static const double workingSetTime;
    static const int64_t workingSetMaxBytes = 16000000;
    static const size_t workingSetMaxEntries = 2000;

    // This constant factor biases cache capacity toward recent activity. We
    // want to adapt to changing workloads.
    static const int64_t recencyBias = 4;

    // This constant factor treats a sampled event for one old object as if it
    // happened for many old objects. Most old objects are evicted before we can
    // sample them, so we need to extrapolate from the ones we do sample.
    static const int64_t oldObjectSamplingMultiplier = 32;

    size_t numberOfEntries() const { return static_cast<size_t>(m_map.size()); }
    bool canPruneQuickly() const { return numberOfEntries() < workingSetMaxEntries; }

    void pruneSlowCase();
    void prune()
    {
        if (m_size <= m_capacity && canPruneQuickly())
            return;

        if (monotonicallyIncreasingTime() - m_timeAtLastPrune < workingSetTime
            && m_size - m_sizeAtLastPrune < workingSetMaxBytes
            && canPruneQuickly())
                return;

        pruneSlowCase();
    }

    MapType m_map;
    int64_t m_size;
    int64_t m_sizeAtLastPrune;
    double m_timeAtLastPrune;
    int64_t m_minCapacity;
    int64_t m_capacity;
    int64_t m_age;
};

// Caches top-level code such as <script>, window.eval(), new Function, and JSEvaluateScript().
class CodeCache {
    WTF_MAKE_FAST_ALLOCATED;
public:
    UnlinkedProgramCodeBlock* getUnlinkedProgramCodeBlock(VM&, ProgramExecutable*, const SourceCode&, JSParserStrictMode, DebuggerMode, ParserError&);
    UnlinkedEvalCodeBlock* getUnlinkedEvalCodeBlock(VM&, IndirectEvalExecutable*, const SourceCode&, JSParserStrictMode, DebuggerMode, ParserError&, EvalContextType);
    UnlinkedModuleProgramCodeBlock* getUnlinkedModuleProgramCodeBlock(VM&, ModuleProgramExecutable*, const SourceCode&, DebuggerMode, ParserError&);
    UnlinkedFunctionExecutable* getUnlinkedGlobalFunctionExecutable(VM&, const Identifier&, const SourceCode&, DebuggerMode, ParserError&);

    void clear() { m_sourceCode.clear(); }

private:
    template <class UnlinkedCodeBlockType, class ExecutableType> 
    UnlinkedCodeBlockType* getUnlinkedGlobalCodeBlock(VM&, ExecutableType*, const SourceCode&, JSParserStrictMode, JSParserScriptMode, DebuggerMode, ParserError&, EvalContextType);

    CodeCacheMap m_sourceCode;
};

template <typename T> struct CacheTypes { };

template <> struct CacheTypes<UnlinkedProgramCodeBlock> {
    typedef JSC::ProgramNode RootNode;
    static const SourceCodeType codeType = SourceCodeType::ProgramType;
    static const SourceParseMode parseMode = SourceParseMode::ProgramMode;
};

template <> struct CacheTypes<UnlinkedEvalCodeBlock> {
    typedef JSC::EvalNode RootNode;
    static const SourceCodeType codeType = SourceCodeType::EvalType;
    static const SourceParseMode parseMode = SourceParseMode::ProgramMode;
};

template <> struct CacheTypes<UnlinkedModuleProgramCodeBlock> {
    typedef JSC::ModuleProgramNode RootNode;
    static const SourceCodeType codeType = SourceCodeType::ModuleType;
    static const SourceParseMode parseMode = SourceParseMode::ModuleEvaluateMode;
};

template <class UnlinkedCodeBlockType, class ExecutableType>
UnlinkedCodeBlockType* generateUnlinkedCodeBlock(VM& vm, ExecutableType* executable, const SourceCode& source, JSParserStrictMode strictMode, JSParserScriptMode scriptMode, DebuggerMode debuggerMode, ParserError& error, EvalContextType evalContextType, const VariableEnvironment* variablesUnderTDZ)
{
    typedef typename CacheTypes<UnlinkedCodeBlockType>::RootNode RootNode;
    DerivedContextType derivedContextType = executable->derivedContextType();
    std::unique_ptr<RootNode> rootNode = parse<RootNode>(
        &vm, source, Identifier(), JSParserBuiltinMode::NotBuiltin, strictMode, scriptMode, CacheTypes<UnlinkedCodeBlockType>::parseMode, SuperBinding::NotNeeded, error, nullptr, ConstructorKind::None, derivedContextType, evalContextType);
    if (!rootNode)
        return nullptr;

    unsigned lineCount = rootNode->lastLine() - rootNode->firstLine();
    unsigned startColumn = rootNode->startColumn() + 1;
    bool endColumnIsOnStartLine = !lineCount;
    unsigned unlinkedEndColumn = rootNode->endColumn();
    unsigned endColumn = unlinkedEndColumn + (endColumnIsOnStartLine ? startColumn : 1);
    unsigned arrowContextFeature = executable->isArrowFunctionContext() ? ArrowFunctionContextFeature : 0;
    executable->recordParse(rootNode->features() | arrowContextFeature, rootNode->hasCapturedVariables(), rootNode->lastLine(), endColumn);

    UnlinkedCodeBlockType* unlinkedCodeBlock = UnlinkedCodeBlockType::create(&vm, executable->executableInfo(), debuggerMode);
    unlinkedCodeBlock->recordParse(rootNode->features(), rootNode->hasCapturedVariables(), lineCount, unlinkedEndColumn);
    unlinkedCodeBlock->setSourceURLDirective(source.provider()->sourceURL());
    unlinkedCodeBlock->setSourceMappingURLDirective(source.provider()->sourceMappingURL());

    error = BytecodeGenerator::generate(vm, rootNode.get(), unlinkedCodeBlock, debuggerMode, variablesUnderTDZ);

    if (error.isValid())
        return nullptr;

    return unlinkedCodeBlock;
}

} // namespace JSC
