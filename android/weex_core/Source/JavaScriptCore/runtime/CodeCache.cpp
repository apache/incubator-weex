/*
 * Copyright (C) 2012, 2016 Apple Inc. All Rights Reserved.
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

#include "config.h"
#include "CodeCache.h"

#include "IndirectEvalExecutable.h"
#include "Trace.h"

namespace JSC {

const double CodeCacheMap::workingSetTime = 10.0;

void CodeCacheMap::pruneSlowCase()
{
    m_minCapacity = std::max(m_size - m_sizeAtLastPrune, static_cast<int64_t>(0));
    m_sizeAtLastPrune = m_size;
    m_timeAtLastPrune = monotonicallyIncreasingTime();

    if (m_capacity < m_minCapacity)
        m_capacity = m_minCapacity;

    while (m_size > m_capacity || !canPruneQuickly()) {
        MapType::iterator it = m_map.begin();
        m_size -= it->key.length();
        m_map.remove(it);
    }
}

template <class UnlinkedCodeBlockType, class ExecutableType>
UnlinkedCodeBlockType* CodeCache::getUnlinkedGlobalCodeBlock(VM& vm, ExecutableType* executable, const SourceCode& source, JSParserStrictMode strictMode, JSParserScriptMode scriptMode, DebuggerMode debuggerMode, ParserError& error, EvalContextType evalContextType)
{
    DerivedContextType derivedContextType = executable->derivedContextType();
    bool isArrowFunctionContext = executable->isArrowFunctionContext();
    SourceCodeKey key(
        source, String(), CacheTypes<UnlinkedCodeBlockType>::codeType, strictMode, scriptMode, 
        derivedContextType, evalContextType, isArrowFunctionContext, debuggerMode, 
        vm.typeProfiler() ? TypeProfilerEnabled::Yes : TypeProfilerEnabled::No, 
        vm.controlFlowProfiler() ? ControlFlowProfilerEnabled::Yes : ControlFlowProfilerEnabled::No);
    SourceCodeValue* cache = m_sourceCode.findCacheAndUpdateAge(key);
    if (cache && Options::useCodeCache()) {
        UnlinkedCodeBlockType* unlinkedCodeBlock = jsCast<UnlinkedCodeBlockType*>(cache->cell.get());
        unsigned lineCount = unlinkedCodeBlock->lineCount();
        unsigned startColumn = unlinkedCodeBlock->startColumn() + source.startColumn().oneBasedInt();
        bool endColumnIsOnStartLine = !lineCount;
        unsigned endColumn = unlinkedCodeBlock->endColumn() + (endColumnIsOnStartLine ? startColumn : 1);
        executable->recordParse(unlinkedCodeBlock->codeFeatures(), unlinkedCodeBlock->hasCapturedVariables(), source.firstLine().oneBasedInt() + lineCount, endColumn);
        source.provider()->setSourceURLDirective(unlinkedCodeBlock->sourceURLDirective());
        source.provider()->setSourceMappingURLDirective(unlinkedCodeBlock->sourceMappingURLDirective());
        return unlinkedCodeBlock;
    }
    
    VariableEnvironment variablesUnderTDZ;
    UnlinkedCodeBlockType* unlinkedCodeBlock = generateUnlinkedCodeBlock<UnlinkedCodeBlockType, ExecutableType>(vm, executable, source, strictMode, scriptMode, debuggerMode, error, evalContextType, &variablesUnderTDZ);

    if (unlinkedCodeBlock && Options::useCodeCache())
        m_sourceCode.addCache(key, SourceCodeValue(vm, unlinkedCodeBlock, m_sourceCode.age()));

    return unlinkedCodeBlock;
}

UnlinkedProgramCodeBlock* CodeCache::getUnlinkedProgramCodeBlock(VM& vm, ProgramExecutable* executable, const SourceCode& source, JSParserStrictMode strictMode, DebuggerMode debuggerMode, ParserError& error)
{
    return getUnlinkedGlobalCodeBlock<UnlinkedProgramCodeBlock>(vm, executable, source, strictMode, JSParserScriptMode::Classic, debuggerMode, error, EvalContextType::None);
}

UnlinkedEvalCodeBlock* CodeCache::getUnlinkedEvalCodeBlock(VM& vm, IndirectEvalExecutable* executable, const SourceCode& source, JSParserStrictMode strictMode, DebuggerMode debuggerMode, ParserError& error, EvalContextType evalContextType)
{
    return getUnlinkedGlobalCodeBlock<UnlinkedEvalCodeBlock>(vm, executable, source, strictMode, JSParserScriptMode::Classic, debuggerMode, error, evalContextType);
}

UnlinkedModuleProgramCodeBlock* CodeCache::getUnlinkedModuleProgramCodeBlock(VM& vm, ModuleProgramExecutable* executable, const SourceCode& source, DebuggerMode debuggerMode, ParserError& error)
{
    return getUnlinkedGlobalCodeBlock<UnlinkedModuleProgramCodeBlock>(vm, executable, source, JSParserStrictMode::Strict, JSParserScriptMode::Module, debuggerMode, error, EvalContextType::None);
}

UnlinkedFunctionExecutable* CodeCache::getUnlinkedGlobalFunctionExecutable(VM& vm, const Identifier& name, const SourceCode& source, DebuggerMode debuggerMode, ParserError& error)
{
    bool isArrowFunctionContext = false;
    SourceCodeKey key(
        source, name.string(), SourceCodeType::FunctionType,
        JSParserStrictMode::NotStrict,
        JSParserScriptMode::Classic,
        DerivedContextType::None,
        EvalContextType::None,
        isArrowFunctionContext,
        debuggerMode, 
        vm.typeProfiler() ? TypeProfilerEnabled::Yes : TypeProfilerEnabled::No, 
        vm.controlFlowProfiler() ? ControlFlowProfilerEnabled::Yes : ControlFlowProfilerEnabled::No);
    SourceCodeValue* cache = m_sourceCode.findCacheAndUpdateAge(key);
    if (cache && Options::useCodeCache()) {
        UnlinkedFunctionExecutable* executable = jsCast<UnlinkedFunctionExecutable*>(cache->cell.get());
        source.provider()->setSourceURLDirective(executable->sourceURLDirective());
        source.provider()->setSourceMappingURLDirective(executable->sourceMappingURLDirective());
        return executable;
    }

    JSTextPosition positionBeforeLastNewline;
    std::unique_ptr<ProgramNode> program;
    {
        base::debug::TraceScope traceScope("jsc", "parse Program");
        program = parse<ProgramNode>(
            &vm, source, Identifier(), JSParserBuiltinMode::NotBuiltin,
            JSParserStrictMode::NotStrict, JSParserScriptMode::Classic, SourceParseMode::ProgramMode, SuperBinding::NotNeeded,
            error, &positionBeforeLastNewline);
        if (!program) {
            RELEASE_ASSERT(error.isValid());
            return nullptr;
        }
    }

    // This function assumes an input string that would result in a single function declaration.
    StatementNode* statement = program->singleStatement();
    if (UNLIKELY(!statement)) {
        JSToken token;
        error = ParserError(ParserError::SyntaxError, ParserError::SyntaxErrorIrrecoverable, token, "Parser error", -1);
        return nullptr;
    }
    ASSERT(statement->isBlock());

    StatementNode* funcDecl = static_cast<BlockNode*>(statement)->singleStatement();
    if (UNLIKELY(!funcDecl)) {
        JSToken token;
        error = ParserError(ParserError::SyntaxError, ParserError::SyntaxErrorIrrecoverable, token, "Parser error", -1);
        return nullptr;
    }
    ASSERT(funcDecl->isFuncDeclNode());

    FunctionMetadataNode* metadata = static_cast<FuncDeclNode*>(funcDecl)->metadata();
    ASSERT(metadata);
    if (!metadata)
        return nullptr;
    
    metadata->overrideName(name);
    metadata->setEndPosition(positionBeforeLastNewline);
    // The Function constructor only has access to global variables, so no variables will be under TDZ.
    VariableEnvironment emptyTDZVariables;
    ConstructAbility constructAbility = constructAbilityForParseMode(metadata->parseMode());
    UnlinkedFunctionExecutable* functionExecutable = UnlinkedFunctionExecutable::create(&vm, source, metadata, UnlinkedNormalFunction, constructAbility, JSParserScriptMode::Classic, emptyTDZVariables, DerivedContextType::None);

    functionExecutable->setSourceURLDirective(source.provider()->sourceURL());
    functionExecutable->setSourceMappingURLDirective(source.provider()->sourceMappingURL());

    m_sourceCode.addCache(key, SourceCodeValue(vm, functionExecutable, m_sourceCode.age()));
    return functionExecutable;
}

}
