/*
 * Copyright (C) 2012-2013, 2015-2016 Apple Inc. All Rights Reserved.
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

#include "UnlinkedCodeBlock.h"

#include "BytecodeGenerator.h"
#include "BytecodeRewriter.h"
#include "ClassInfo.h"
#include "CodeCache.h"
#include "ExecutableInfo.h"
#include "FunctionOverrides.h"
#include "JSCInlines.h"
#include "JSString.h"
#include "Parser.h"
#include "PreciseJumpTargetsInlines.h"
#include "SourceProvider.h"
#include "Structure.h"
#include "SymbolTable.h"
#include "UnlinkedEvalCodeBlock.h"
#include "UnlinkedFunctionCodeBlock.h"
#include "UnlinkedInstructionStream.h"
#include "UnlinkedModuleProgramCodeBlock.h"
#include "UnlinkedProgramCodeBlock.h"
#include <wtf/DataLog.h>

namespace JSC {

const ClassInfo UnlinkedCodeBlock::s_info = { "UnlinkedCodeBlock", 0, 0, CREATE_METHOD_TABLE(UnlinkedCodeBlock) };

UnlinkedCodeBlock::UnlinkedCodeBlock(VM* vm, Structure* structure, CodeType codeType, const ExecutableInfo& info, DebuggerMode debuggerMode)
    : Base(*vm, structure)
    , m_numVars(0)
    , m_numCalleeLocals(0)
    , m_numParameters(0)
    , m_globalObjectRegister(VirtualRegister())
    , m_usesEval(info.usesEval())
    , m_isStrictMode(info.isStrictMode())
    , m_isConstructor(info.isConstructor())
    , m_hasCapturedVariables(false)
    , m_isBuiltinFunction(info.isBuiltinFunction())
    , m_superBinding(static_cast<unsigned>(info.superBinding()))
    , m_scriptMode(static_cast<unsigned>(info.scriptMode()))
    , m_isArrowFunctionContext(info.isArrowFunctionContext())
    , m_isClassContext(info.isClassContext())
    , m_wasCompiledWithDebuggingOpcodes(debuggerMode == DebuggerMode::DebuggerOn || Options::forceDebuggerBytecodeGeneration())
    , m_constructorKind(static_cast<unsigned>(info.constructorKind()))
    , m_derivedContextType(static_cast<unsigned>(info.derivedContextType()))
    , m_evalContextType(static_cast<unsigned>(info.evalContextType()))
    , m_lineCount(0)
    , m_endColumn(UINT_MAX)
    , m_didOptimize(MixedTriState)
    , m_parseMode(info.parseMode())
    , m_features(0)
    , m_codeType(codeType)
    , m_arrayProfileCount(0)
    , m_arrayAllocationProfileCount(0)
    , m_objectAllocationProfileCount(0)
    , m_valueProfileCount(0)
    , m_llintCallLinkInfoCount(0)
{
    for (auto& constantRegisterIndex : m_linkTimeConstants)
        constantRegisterIndex = 0;
    ASSERT(m_constructorKind == static_cast<unsigned>(info.constructorKind()));
}

void UnlinkedCodeBlock::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    UnlinkedCodeBlock* thisObject = jsCast<UnlinkedCodeBlock*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);
    auto locker = holdLock(*thisObject);
    for (FunctionExpressionVector::iterator ptr = thisObject->m_functionDecls.begin(), end = thisObject->m_functionDecls.end(); ptr != end; ++ptr)
        visitor.append(*ptr);
    for (FunctionExpressionVector::iterator ptr = thisObject->m_functionExprs.begin(), end = thisObject->m_functionExprs.end(); ptr != end; ++ptr)
        visitor.append(*ptr);
    visitor.appendValues(thisObject->m_constantRegisters.data(), thisObject->m_constantRegisters.size());
    if (thisObject->m_unlinkedInstructions)
        visitor.reportExtraMemoryVisited(thisObject->m_unlinkedInstructions->sizeInBytes());
    if (thisObject->m_rareData) {
        for (size_t i = 0, end = thisObject->m_rareData->m_regexps.size(); i != end; i++)
            visitor.append(thisObject->m_rareData->m_regexps[i]);
    }
}

size_t UnlinkedCodeBlock::estimatedSize(JSCell* cell)
{
    UnlinkedCodeBlock* thisObject = jsCast<UnlinkedCodeBlock*>(cell);
    size_t extraSize = thisObject->m_unlinkedInstructions ? thisObject->m_unlinkedInstructions->sizeInBytes() : 0;
    return Base::estimatedSize(cell) + extraSize;
}

int UnlinkedCodeBlock::lineNumberForBytecodeOffset(unsigned bytecodeOffset)
{
    ASSERT(bytecodeOffset < instructions().count());
    int divot;
    int startOffset;
    int endOffset;
    unsigned line;
    unsigned column;
    expressionRangeForBytecodeOffset(bytecodeOffset, divot, startOffset, endOffset, line, column);
    return line;
}

inline void UnlinkedCodeBlock::getLineAndColumn(const ExpressionRangeInfo& info,
    unsigned& line, unsigned& column) const
{
    switch (info.mode) {
    case ExpressionRangeInfo::FatLineMode:
        info.decodeFatLineMode(line, column);
        break;
    case ExpressionRangeInfo::FatColumnMode:
        info.decodeFatColumnMode(line, column);
        break;
    case ExpressionRangeInfo::FatLineAndColumnMode: {
        unsigned fatIndex = info.position;
        ExpressionRangeInfo::FatPosition& fatPos = m_rareData->m_expressionInfoFatPositions[fatIndex];
        line = fatPos.line;
        column = fatPos.column;
        break;
    }
    } // switch
}

#ifndef NDEBUG
static void dumpLineColumnEntry(size_t index, const UnlinkedInstructionStream& instructionStream, unsigned instructionOffset, unsigned line, unsigned column)
{
    const auto& instructions = instructionStream.unpackForDebugging();
    OpcodeID opcode = instructions[instructionOffset].u.opcode;
    const char* event = "";
    if (opcode == op_debug) {
        switch (instructions[instructionOffset + 1].u.operand) {
        case WillExecuteProgram: event = " WillExecuteProgram"; break;
        case DidExecuteProgram: event = " DidExecuteProgram"; break;
        case DidEnterCallFrame: event = " DidEnterCallFrame"; break;
        case DidReachBreakpoint: event = " DidReachBreakpoint"; break;
        case WillLeaveCallFrame: event = " WillLeaveCallFrame"; break;
        case WillExecuteStatement: event = " WillExecuteStatement"; break;
        case WillExecuteExpression: event = " WillExecuteExpression"; break;
        }
    }
    dataLogF("  [%zu] pc %u @ line %u col %u : %s%s\n", index, instructionOffset, line, column, opcodeNames[opcode], event);
}

void UnlinkedCodeBlock::dumpExpressionRangeInfo()
{
    Vector<ExpressionRangeInfo>& expressionInfo = m_expressionInfo;

    size_t size = m_expressionInfo.size();
    dataLogF("UnlinkedCodeBlock %p expressionRangeInfo[%zu] {\n", this, size);
    for (size_t i = 0; i < size; i++) {
        ExpressionRangeInfo& info = expressionInfo[i];
        unsigned line;
        unsigned column;
        getLineAndColumn(info, line, column);
        dumpLineColumnEntry(i, instructions(), info.instructionOffset, line, column);
    }
    dataLog("}\n");
}
#endif

void UnlinkedCodeBlock::expressionRangeForBytecodeOffset(unsigned bytecodeOffset,
    int& divot, int& startOffset, int& endOffset, unsigned& line, unsigned& column) const
{
    ASSERT(bytecodeOffset < instructions().count());

    if (!m_expressionInfo.size()) {
        startOffset = 0;
        endOffset = 0;
        divot = 0;
        line = 0;
        column = 0;
        return;
    }

    const Vector<ExpressionRangeInfo>& expressionInfo = m_expressionInfo;

    int low = 0;
    int high = expressionInfo.size();
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (expressionInfo[mid].instructionOffset <= bytecodeOffset)
            low = mid + 1;
        else
            high = mid;
    }

    if (!low)
        low = 1;

    const ExpressionRangeInfo& info = expressionInfo[low - 1];
    startOffset = info.startOffset;
    endOffset = info.endOffset;
    divot = info.divotPoint;
    getLineAndColumn(info, line, column);
}

void UnlinkedCodeBlock::addExpressionInfo(unsigned instructionOffset,
    int divot, int startOffset, int endOffset, unsigned line, unsigned column)
{
    if (divot > ExpressionRangeInfo::MaxDivot) {
        // Overflow has occurred, we can only give line number info for errors for this region
        divot = 0;
        startOffset = 0;
        endOffset = 0;
    } else if (startOffset > ExpressionRangeInfo::MaxOffset) {
        // If the start offset is out of bounds we clear both offsets
        // so we only get the divot marker. Error message will have to be reduced
        // to line and charPosition number.
        startOffset = 0;
        endOffset = 0;
    } else if (endOffset > ExpressionRangeInfo::MaxOffset) {
        // The end offset is only used for additional context, and is much more likely
        // to overflow (eg. function call arguments) so we are willing to drop it without
        // dropping the rest of the range.
        endOffset = 0;
    }

    unsigned positionMode =
        (line <= ExpressionRangeInfo::MaxFatLineModeLine && column <= ExpressionRangeInfo::MaxFatLineModeColumn) 
        ? ExpressionRangeInfo::FatLineMode
        : (line <= ExpressionRangeInfo::MaxFatColumnModeLine && column <= ExpressionRangeInfo::MaxFatColumnModeColumn)
        ? ExpressionRangeInfo::FatColumnMode
        : ExpressionRangeInfo::FatLineAndColumnMode;

    ExpressionRangeInfo info;
    info.instructionOffset = instructionOffset;
    info.divotPoint = divot;
    info.startOffset = startOffset;
    info.endOffset = endOffset;

    info.mode = positionMode;
    switch (positionMode) {
    case ExpressionRangeInfo::FatLineMode:
        info.encodeFatLineMode(line, column);
        break;
    case ExpressionRangeInfo::FatColumnMode:
        info.encodeFatColumnMode(line, column);
        break;
    case ExpressionRangeInfo::FatLineAndColumnMode: {
        createRareDataIfNecessary();
        unsigned fatIndex = m_rareData->m_expressionInfoFatPositions.size();
        ExpressionRangeInfo::FatPosition fatPos = { line, column };
        m_rareData->m_expressionInfoFatPositions.append(fatPos);
        info.position = fatIndex;
    }
    } // switch

    m_expressionInfo.append(info);
}

bool UnlinkedCodeBlock::typeProfilerExpressionInfoForBytecodeOffset(unsigned bytecodeOffset, unsigned& startDivot, unsigned& endDivot)
{
    static const bool verbose = false;
    if (!m_rareData) {
        if (verbose)
            dataLogF("Don't have assignment info for offset:%u\n", bytecodeOffset);
        startDivot = UINT_MAX;
        endDivot = UINT_MAX;
        return false;
    }

    auto iter = m_rareData->m_typeProfilerInfoMap.find(bytecodeOffset);
    if (iter == m_rareData->m_typeProfilerInfoMap.end()) {
        if (verbose)
            dataLogF("Don't have assignment info for offset:%u\n", bytecodeOffset);
        startDivot = UINT_MAX;
        endDivot = UINT_MAX;
        return false;
    }
    
    RareData::TypeProfilerExpressionRange& range = iter->value;
    startDivot = range.m_startDivot;
    endDivot = range.m_endDivot;
    return true;
}

void UnlinkedCodeBlock::addTypeProfilerExpressionInfo(unsigned instructionOffset, unsigned startDivot, unsigned endDivot)
{
    createRareDataIfNecessary();
    RareData::TypeProfilerExpressionRange range;
    range.m_startDivot = startDivot;
    range.m_endDivot = endDivot;
    m_rareData->m_typeProfilerInfoMap.set(instructionOffset, range);
}

UnlinkedCodeBlock::~UnlinkedCodeBlock()
{
}

void UnlinkedCodeBlock::setInstructions(std::unique_ptr<UnlinkedInstructionStream> instructions)
{
    ASSERT(instructions);
    {
        auto locker = holdLock(*this);
        m_unlinkedInstructions = WTFMove(instructions);
    }
    Heap::heap(this)->reportExtraMemoryAllocated(m_unlinkedInstructions->sizeInBytes());
}

const UnlinkedInstructionStream& UnlinkedCodeBlock::instructions() const
{
    ASSERT(m_unlinkedInstructions.get());
    return *m_unlinkedInstructions;
}

UnlinkedHandlerInfo* UnlinkedCodeBlock::handlerForBytecodeOffset(unsigned bytecodeOffset, RequiredHandler requiredHandler)
{
    return handlerForIndex(bytecodeOffset, requiredHandler);
}

UnlinkedHandlerInfo* UnlinkedCodeBlock::handlerForIndex(unsigned index, RequiredHandler requiredHandler)
{
    if (!m_rareData)
        return nullptr;
    return UnlinkedHandlerInfo::handlerForIndex(m_rareData->m_exceptionHandlers, index, requiredHandler);
}

void UnlinkedCodeBlock::applyModification(BytecodeRewriter& rewriter)
{
    // Before applying the changes, we adjust the jumps based on the original bytecode offset, the offset to the jump target, and
    // the insertion information.

    BytecodeGraph<UnlinkedCodeBlock>& graph = rewriter.graph();
    UnlinkedInstruction* instructionsBegin = graph.instructions().begin();

    for (int bytecodeOffset = 0, instructionCount = graph.instructions().size(); bytecodeOffset < instructionCount;) {
        UnlinkedInstruction* current = instructionsBegin + bytecodeOffset;
        OpcodeID opcodeID = current[0].u.opcode;
        extractStoredJumpTargetsForBytecodeOffset(this, vm()->interpreter, instructionsBegin, bytecodeOffset, [&](int32_t& relativeOffset) {
            relativeOffset = rewriter.adjustJumpTarget(bytecodeOffset, bytecodeOffset + relativeOffset);
        });
        bytecodeOffset += opcodeLength(opcodeID);
    }

    // Then, exception handlers should be adjusted.
    if (m_rareData) {
        for (UnlinkedHandlerInfo& handler : m_rareData->m_exceptionHandlers) {
            handler.target = rewriter.adjustAbsoluteOffset(handler.target);
            handler.start = rewriter.adjustAbsoluteOffset(handler.start);
            handler.end = rewriter.adjustAbsoluteOffset(handler.end);
        }

        for (size_t i = 0; i < m_rareData->m_opProfileControlFlowBytecodeOffsets.size(); ++i)
            m_rareData->m_opProfileControlFlowBytecodeOffsets[i] = rewriter.adjustAbsoluteOffset(m_rareData->m_opProfileControlFlowBytecodeOffsets[i]);

        if (!m_rareData->m_typeProfilerInfoMap.isEmpty()) {
            HashMap<unsigned, RareData::TypeProfilerExpressionRange> adjustedTypeProfilerInfoMap;
            for (auto& entry : m_rareData->m_typeProfilerInfoMap)
                adjustedTypeProfilerInfoMap.set(rewriter.adjustAbsoluteOffset(entry.key), entry.value);
            m_rareData->m_typeProfilerInfoMap.swap(adjustedTypeProfilerInfoMap);
        }
    }

    for (size_t i = 0; i < m_propertyAccessInstructions.size(); ++i)
        m_propertyAccessInstructions[i] = rewriter.adjustAbsoluteOffset(m_propertyAccessInstructions[i]);

    for (size_t i = 0; i < m_expressionInfo.size(); ++i)
        m_expressionInfo[i].instructionOffset = rewriter.adjustAbsoluteOffset(m_expressionInfo[i].instructionOffset);

    // Then, modify the unlinked instructions.
    rewriter.applyModification();

    // And recompute the jump target based on the modified unlinked instructions.
    m_jumpTargets.clear();
    recomputePreciseJumpTargets(this, graph.instructions().begin(), graph.instructions().size(), m_jumpTargets);
}

void UnlinkedCodeBlock::shrinkToFit()
{
    auto locker = holdLock(*this);
    
    m_jumpTargets.shrinkToFit();
    m_identifiers.shrinkToFit();
    m_bitVectors.shrinkToFit();
    m_constantRegisters.shrinkToFit();
    m_constantsSourceCodeRepresentation.shrinkToFit();
    m_functionDecls.shrinkToFit();
    m_functionExprs.shrinkToFit();
    m_propertyAccessInstructions.shrinkToFit();
    m_expressionInfo.shrinkToFit();

    if (m_rareData) {
        m_rareData->m_exceptionHandlers.shrinkToFit();
        m_rareData->m_regexps.shrinkToFit();
        m_rareData->m_constantBuffers.shrinkToFit();
        m_rareData->m_switchJumpTables.shrinkToFit();
        m_rareData->m_stringSwitchJumpTables.shrinkToFit();
        m_rareData->m_expressionInfoFatPositions.shrinkToFit();
    }
}

void UnlinkedCodeBlock::dump(PrintStream&) const
{
}

} // namespace JSC
