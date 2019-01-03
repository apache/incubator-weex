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

#include "BytecodeConventions.h"
#include "CodeSpecializationKind.h"
#include "CodeType.h"
#include "ConstructAbility.h"
#include "ExpressionRangeInfo.h"
#include "HandlerInfo.h"
#include "Identifier.h"
#include "JSCell.h"
#include "JSString.h"
#include "LockDuringMarking.h"
#include "ParserModes.h"
#include "RegExp.h"
#include "SpecialPointer.h"
#include "UnlinkedFunctionExecutable.h"
#include "VariableEnvironment.h"
#include "VirtualRegister.h"
#include <algorithm>
#include <wtf/BitVector.h>
#include <wtf/HashSet.h>
#include <wtf/TriState.h>
#include <wtf/Vector.h>
#include <wtf/text/UniquedStringImpl.h>

namespace JSC {

class BytecodeRewriter;
class Debugger;
class FunctionExecutable;
class ParserError;
class ScriptExecutable;
class SourceCode;
class SourceProvider;
class UnlinkedCodeBlock;
class UnlinkedFunctionCodeBlock;
class UnlinkedFunctionExecutable;
class UnlinkedInstructionStream;
struct ExecutableInfo;

typedef unsigned UnlinkedValueProfile;
typedef unsigned UnlinkedArrayProfile;
typedef unsigned UnlinkedArrayAllocationProfile;
typedef unsigned UnlinkedObjectAllocationProfile;
typedef unsigned UnlinkedLLIntCallLinkInfo;
typedef std::pair<HashSet<UniquedStringImpl*>, unsigned> ConstantIndentifierSetEntry;

struct UnlinkedStringJumpTable {
    struct OffsetLocation {
        int32_t branchOffset;
    };

    typedef HashMap<RefPtr<StringImpl>, OffsetLocation> StringOffsetTable;
    StringOffsetTable offsetTable;

    inline int32_t offsetForValue(StringImpl* value, int32_t defaultOffset)
    {
        StringOffsetTable::const_iterator end = offsetTable.end();
        StringOffsetTable::const_iterator loc = offsetTable.find(value);
        if (loc == end)
            return defaultOffset;
        return loc->value.branchOffset;
    }

};

struct UnlinkedSimpleJumpTable {
    Vector<int32_t> branchOffsets;
    int32_t min;

    int32_t offsetForValue(int32_t value, int32_t defaultOffset);
    void add(int32_t key, int32_t offset)
    {
        if (!branchOffsets[key])
            branchOffsets[key] = offset;
    }
};

struct UnlinkedInstruction {
    UnlinkedInstruction() { u.operand = 0; }
    UnlinkedInstruction(OpcodeID opcode) { u.opcode = opcode; }
    UnlinkedInstruction(int operand) { u.operand = operand; }
    union {
        OpcodeID opcode;
        int32_t operand;
        unsigned unsignedValue;
    } u;
};

class UnlinkedCodeBlock : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags;

    static const bool needsDestruction = true;

    enum { CallFunction, ApplyFunction };

    typedef UnlinkedInstruction Instruction;
    typedef Vector<UnlinkedInstruction, 0, UnsafeVectorOverflow> UnpackedInstructions;

    bool isConstructor() const { return m_isConstructor; }
    bool isStrictMode() const { return m_isStrictMode; }
    bool usesEval() const { return m_usesEval; }
    SourceParseMode parseMode() const { return m_parseMode; }
    bool isArrowFunction() const { return isArrowFunctionParseMode(parseMode()); }
    DerivedContextType derivedContextType() const { return static_cast<DerivedContextType>(m_derivedContextType); }
    EvalContextType evalContextType() const { return static_cast<EvalContextType>(m_evalContextType); }
    bool isArrowFunctionContext() const { return m_isArrowFunctionContext; }
    bool isClassContext() const { return m_isClassContext; }

    void addExpressionInfo(unsigned instructionOffset, int divot,
        int startOffset, int endOffset, unsigned line, unsigned column);

    void addTypeProfilerExpressionInfo(unsigned instructionOffset, unsigned startDivot, unsigned endDivot);

    bool hasExpressionInfo() { return m_expressionInfo.size(); }
    const Vector<ExpressionRangeInfo>& expressionInfo() { return m_expressionInfo; }

    // Special registers
    void setThisRegister(VirtualRegister thisRegister) { m_thisRegister = thisRegister; }
    void setScopeRegister(VirtualRegister scopeRegister) { m_scopeRegister = scopeRegister; }

    bool usesGlobalObject() const { return m_globalObjectRegister.isValid(); }
    void setGlobalObjectRegister(VirtualRegister globalObjectRegister) { m_globalObjectRegister = globalObjectRegister; }
    VirtualRegister globalObjectRegister() const { return m_globalObjectRegister; }

    // Parameter information
    void setNumParameters(int newValue) { m_numParameters = newValue; }
    void addParameter() { m_numParameters++; }
    unsigned numParameters() const { return m_numParameters; }

    unsigned addRegExp(RegExp* r)
    {
        createRareDataIfNecessary();
        VM& vm = *this->vm();
        auto locker = lockDuringMarking(vm.heap, *this);
        unsigned size = m_rareData->m_regexps.size();
        m_rareData->m_regexps.append(WriteBarrier<RegExp>(vm, this, r));
        return size;
    }
    unsigned numberOfRegExps() const
    {
        if (!m_rareData)
            return 0;
        return m_rareData->m_regexps.size();
    }
    RegExp* regexp(int index) const { ASSERT(m_rareData); return m_rareData->m_regexps[index].get(); }

    // Constant Pools

    size_t numberOfIdentifiers() const { return m_identifiers.size(); }
    void addIdentifier(const Identifier& i) { return m_identifiers.append(i); }
    const Identifier& identifier(int index) const { return m_identifiers[index]; }
    const Vector<Identifier>& identifiers() const { return m_identifiers; }

    const Vector<BitVector>& bitVectors() const { return m_bitVectors; }
    BitVector& bitVector(size_t i) { return m_bitVectors[i]; }
    unsigned addBitVector(BitVector&& bitVector)
    {
        m_bitVectors.append(WTFMove(bitVector));
        return m_bitVectors.size() - 1;
    }
    
    void addSetConstant(HashSet<UniquedStringImpl*>& set)
    {
        VM& vm = *this->vm();
        auto locker = lockDuringMarking(vm.heap, *this);
        unsigned result = m_constantRegisters.size();
        m_constantRegisters.append(WriteBarrier<Unknown>());
        m_constantsSourceCodeRepresentation.append(SourceCodeRepresentation::Other);
        m_constantIdentifierSets.append(ConstantIndentifierSetEntry(set, result));
    }

    unsigned addConstant(JSValue v, SourceCodeRepresentation sourceCodeRepresentation = SourceCodeRepresentation::Other)
    {
        VM& vm = *this->vm();
        auto locker = lockDuringMarking(vm.heap, *this);
        unsigned result = m_constantRegisters.size();
        m_constantRegisters.append(WriteBarrier<Unknown>());
        m_constantRegisters.last().set(vm, this, v);
        m_constantsSourceCodeRepresentation.append(sourceCodeRepresentation);
        return result;
    }
    unsigned addConstant(LinkTimeConstant type)
    {
        VM& vm = *this->vm();
        auto locker = lockDuringMarking(vm.heap, *this);
        unsigned result = m_constantRegisters.size();
        ASSERT(result);
        unsigned index = static_cast<unsigned>(type);
        ASSERT(index < LinkTimeConstantCount);
        m_linkTimeConstants[index] = result;
        m_constantRegisters.append(WriteBarrier<Unknown>());
        m_constantsSourceCodeRepresentation.append(SourceCodeRepresentation::Other);
        return result;
    }
    unsigned registerIndexForLinkTimeConstant(LinkTimeConstant type)
    {
        unsigned index = static_cast<unsigned>(type);
        ASSERT(index < LinkTimeConstantCount);
        return m_linkTimeConstants[index];
    }
    const Vector<WriteBarrier<Unknown>>& constantRegisters() { return m_constantRegisters; }
    const Vector<ConstantIndentifierSetEntry>& constantIdentifierSets() { return m_constantIdentifierSets; }
    const WriteBarrier<Unknown>& constantRegister(int index) const { return m_constantRegisters[index - FirstConstantRegisterIndex]; }
    ALWAYS_INLINE bool isConstantRegisterIndex(int index) const { return index >= FirstConstantRegisterIndex; }
    ALWAYS_INLINE JSValue getConstant(int index) const { return m_constantRegisters[index - FirstConstantRegisterIndex].get(); }
    const Vector<SourceCodeRepresentation>& constantsSourceCodeRepresentation() { return m_constantsSourceCodeRepresentation; }

    // Jumps
    size_t numberOfJumpTargets() const { return m_jumpTargets.size(); }
    void addJumpTarget(unsigned jumpTarget) { m_jumpTargets.append(jumpTarget); }
    unsigned jumpTarget(int index) const { return m_jumpTargets[index]; }
    unsigned lastJumpTarget() const { return m_jumpTargets.last(); }

    UnlinkedHandlerInfo* handlerForBytecodeOffset(unsigned bytecodeOffset, RequiredHandler = RequiredHandler::AnyHandler);
    UnlinkedHandlerInfo* handlerForIndex(unsigned, RequiredHandler = RequiredHandler::AnyHandler);

    bool isBuiltinFunction() const { return m_isBuiltinFunction; }

    ConstructorKind constructorKind() const { return static_cast<ConstructorKind>(m_constructorKind); }
    SuperBinding superBinding() const { return static_cast<SuperBinding>(m_superBinding); }
    JSParserScriptMode scriptMode() const { return static_cast<JSParserScriptMode>(m_scriptMode); }

    void shrinkToFit();

    void setInstructions(std::unique_ptr<UnlinkedInstructionStream>);
    const UnlinkedInstructionStream& instructions() const;

    int numCalleeLocals() const { return m_numCalleeLocals; }

    int m_numVars;
    int m_numCapturedVars;
    int m_numCalleeLocals;

    // Jump Tables

    size_t numberOfSwitchJumpTables() const { return m_rareData ? m_rareData->m_switchJumpTables.size() : 0; }
    UnlinkedSimpleJumpTable& addSwitchJumpTable() { createRareDataIfNecessary(); m_rareData->m_switchJumpTables.append(UnlinkedSimpleJumpTable()); return m_rareData->m_switchJumpTables.last(); }
    UnlinkedSimpleJumpTable& switchJumpTable(int tableIndex) { ASSERT(m_rareData); return m_rareData->m_switchJumpTables[tableIndex]; }

    size_t numberOfStringSwitchJumpTables() const { return m_rareData ? m_rareData->m_stringSwitchJumpTables.size() : 0; }
    UnlinkedStringJumpTable& addStringSwitchJumpTable() { createRareDataIfNecessary(); m_rareData->m_stringSwitchJumpTables.append(UnlinkedStringJumpTable()); return m_rareData->m_stringSwitchJumpTables.last(); }
    UnlinkedStringJumpTable& stringSwitchJumpTable(int tableIndex) { ASSERT(m_rareData); return m_rareData->m_stringSwitchJumpTables[tableIndex]; }

    unsigned addFunctionDecl(UnlinkedFunctionExecutable* n)
    {
        VM& vm = *this->vm();
        auto locker = lockDuringMarking(vm.heap, *this);
        unsigned size = m_functionDecls.size();
        m_functionDecls.append(WriteBarrier<UnlinkedFunctionExecutable>());
        m_functionDecls.last().set(vm, this, n);
        return size;
    }
    UnlinkedFunctionExecutable* functionDecl(int index) { return m_functionDecls[index].get(); }
    size_t numberOfFunctionDecls() { return m_functionDecls.size(); }
    unsigned addFunctionExpr(UnlinkedFunctionExecutable* n)
    {
        VM& vm = *this->vm();
        auto locker = lockDuringMarking(vm.heap, *this);
        unsigned size = m_functionExprs.size();
        m_functionExprs.append(WriteBarrier<UnlinkedFunctionExecutable>());
        m_functionExprs.last().set(vm, this, n);
        return size;
    }
    UnlinkedFunctionExecutable* functionExpr(int index) { return m_functionExprs[index].get(); }
    size_t numberOfFunctionExprs() { return m_functionExprs.size(); }

    // Exception handling support
    size_t numberOfExceptionHandlers() const { return m_rareData ? m_rareData->m_exceptionHandlers.size() : 0; }
    void addExceptionHandler(const UnlinkedHandlerInfo& handler) { createRareDataIfNecessary(); return m_rareData->m_exceptionHandlers.append(handler); }
    UnlinkedHandlerInfo& exceptionHandler(int index) { ASSERT(m_rareData); return m_rareData->m_exceptionHandlers[index]; }

    UnlinkedArrayProfile addArrayProfile() { return m_arrayProfileCount++; }
    unsigned numberOfArrayProfiles() { return m_arrayProfileCount; }
    UnlinkedArrayAllocationProfile addArrayAllocationProfile() { return m_arrayAllocationProfileCount++; }
    unsigned numberOfArrayAllocationProfiles() { return m_arrayAllocationProfileCount; }
    UnlinkedObjectAllocationProfile addObjectAllocationProfile() { return m_objectAllocationProfileCount++; }
    unsigned numberOfObjectAllocationProfiles() { return m_objectAllocationProfileCount; }
    UnlinkedValueProfile addValueProfile() { return m_valueProfileCount++; }
    unsigned numberOfValueProfiles() { return m_valueProfileCount; }

    UnlinkedLLIntCallLinkInfo addLLIntCallLinkInfo() { return m_llintCallLinkInfoCount++; }
    unsigned numberOfLLintCallLinkInfos() { return m_llintCallLinkInfoCount; }

    CodeType codeType() const { return m_codeType; }

    VirtualRegister thisRegister() const { return m_thisRegister; }
    VirtualRegister scopeRegister() const { return m_scopeRegister; }

    void addPropertyAccessInstruction(unsigned propertyAccessInstruction)
    {
        m_propertyAccessInstructions.append(propertyAccessInstruction);
    }

    size_t numberOfPropertyAccessInstructions() const { return m_propertyAccessInstructions.size(); }
    const Vector<unsigned>& propertyAccessInstructions() const { return m_propertyAccessInstructions; }

    typedef Vector<JSValue> ConstantBuffer;

    size_t constantBufferCount() { ASSERT(m_rareData); return m_rareData->m_constantBuffers.size(); }
    unsigned addConstantBuffer(unsigned length)
    {
        createRareDataIfNecessary();
        unsigned size = m_rareData->m_constantBuffers.size();
        m_rareData->m_constantBuffers.append(Vector<JSValue>(length));
        return size;
    }

    const ConstantBuffer& constantBuffer(unsigned index) const
    {
        ASSERT(m_rareData);
        return m_rareData->m_constantBuffers[index];
    }

    ConstantBuffer& constantBuffer(unsigned index)
    {
        ASSERT(m_rareData);
        return m_rareData->m_constantBuffers[index];
    }

    bool hasRareData() const { return m_rareData.get(); }

    int lineNumberForBytecodeOffset(unsigned bytecodeOffset);

    void expressionRangeForBytecodeOffset(unsigned bytecodeOffset, int& divot,
        int& startOffset, int& endOffset, unsigned& line, unsigned& column) const;

    bool typeProfilerExpressionInfoForBytecodeOffset(unsigned bytecodeOffset, unsigned& startDivot, unsigned& endDivot);

    void recordParse(CodeFeatures features, bool hasCapturedVariables, unsigned lineCount, unsigned endColumn)
    {
        m_features = features;
        m_hasCapturedVariables = hasCapturedVariables;
        m_lineCount = lineCount;
        // For the UnlinkedCodeBlock, startColumn is always 0.
        m_endColumn = endColumn;
    }

    const String& sourceURLDirective() const { return m_sourceURLDirective; }
    const String& sourceMappingURLDirective() const { return m_sourceMappingURLDirective; }
    void setSourceURLDirective(const String& sourceURL) { m_sourceURLDirective = sourceURL; }
    void setSourceMappingURLDirective(const String& sourceMappingURL) { m_sourceMappingURLDirective = sourceMappingURL; }

    CodeFeatures codeFeatures() const { return m_features; }
    bool hasCapturedVariables() const { return m_hasCapturedVariables; }
    unsigned lineCount() const { return m_lineCount; }
    ALWAYS_INLINE unsigned startColumn() const { return 0; }
    unsigned endColumn() const { return m_endColumn; }

    void addOpProfileControlFlowBytecodeOffset(size_t offset)
    {
        createRareDataIfNecessary();
        m_rareData->m_opProfileControlFlowBytecodeOffsets.append(offset);
    }
    const Vector<size_t>& opProfileControlFlowBytecodeOffsets() const
    {
        ASSERT(m_rareData);
        return m_rareData->m_opProfileControlFlowBytecodeOffsets;
    }
    bool hasOpProfileControlFlowBytecodeOffsets() const
    {
        return m_rareData && !m_rareData->m_opProfileControlFlowBytecodeOffsets.isEmpty();
    }

    void dumpExpressionRangeInfo(); // For debugging purpose only.

    bool wasCompiledWithDebuggingOpcodes() const { return m_wasCompiledWithDebuggingOpcodes; }

    TriState didOptimize() const { return m_didOptimize; }
    void setDidOptimize(TriState didOptimize) { m_didOptimize = didOptimize; }

    void dump(PrintStream&) const;

protected:
    UnlinkedCodeBlock(VM*, Structure*, CodeType, const ExecutableInfo&, DebuggerMode);
    ~UnlinkedCodeBlock();

    void finishCreation(VM& vm)
    {
        Base::finishCreation(vm);
    }

private:
    friend class BytecodeRewriter;
    void applyModification(BytecodeRewriter&);

    void createRareDataIfNecessary()
    {
        if (!m_rareData) {
            auto locker = lockDuringMarking(*heap(), *this);
            m_rareData = std::make_unique<RareData>();
        }
    }

    void getLineAndColumn(const ExpressionRangeInfo&, unsigned& line, unsigned& column) const;

    int m_numParameters;

    std::unique_ptr<UnlinkedInstructionStream> m_unlinkedInstructions;

    VirtualRegister m_thisRegister;
    VirtualRegister m_scopeRegister;
    VirtualRegister m_globalObjectRegister;

    String m_sourceURLDirective;
    String m_sourceMappingURLDirective;

    unsigned m_usesEval : 1;
    unsigned m_isStrictMode : 1;
    unsigned m_isConstructor : 1;
    unsigned m_hasCapturedVariables : 1;
    unsigned m_isBuiltinFunction : 1;
    unsigned m_superBinding : 1;
    unsigned m_scriptMode: 1;
    unsigned m_isArrowFunctionContext : 1;
    unsigned m_isClassContext : 1;
    unsigned m_wasCompiledWithDebuggingOpcodes : 1;
    unsigned m_constructorKind : 2;
    unsigned m_derivedContextType : 2;
    unsigned m_evalContextType : 2;
    unsigned m_lineCount;
    unsigned m_endColumn;

    TriState m_didOptimize;
    SourceParseMode m_parseMode;
    CodeFeatures m_features;
    CodeType m_codeType;

    Vector<unsigned> m_jumpTargets;

    Vector<unsigned> m_propertyAccessInstructions;

    // Constant Pools
    Vector<Identifier> m_identifiers;
    Vector<BitVector> m_bitVectors;
    Vector<WriteBarrier<Unknown>> m_constantRegisters;
    Vector<ConstantIndentifierSetEntry> m_constantIdentifierSets;
    Vector<SourceCodeRepresentation> m_constantsSourceCodeRepresentation;
    typedef Vector<WriteBarrier<UnlinkedFunctionExecutable>> FunctionExpressionVector;
    FunctionExpressionVector m_functionDecls;
    FunctionExpressionVector m_functionExprs;
    std::array<unsigned, LinkTimeConstantCount> m_linkTimeConstants;

    unsigned m_arrayProfileCount;
    unsigned m_arrayAllocationProfileCount;
    unsigned m_objectAllocationProfileCount;
    unsigned m_valueProfileCount;
    unsigned m_llintCallLinkInfoCount;

public:
    struct RareData {
        WTF_MAKE_FAST_ALLOCATED;
    public:
        Vector<UnlinkedHandlerInfo> m_exceptionHandlers;

        // Rare Constants
        Vector<WriteBarrier<RegExp>> m_regexps;

        // Buffers used for large array literals
        Vector<ConstantBuffer> m_constantBuffers;

        // Jump Tables
        Vector<UnlinkedSimpleJumpTable> m_switchJumpTables;
        Vector<UnlinkedStringJumpTable> m_stringSwitchJumpTables;

        Vector<ExpressionRangeInfo::FatPosition> m_expressionInfoFatPositions;

        struct TypeProfilerExpressionRange {
            unsigned m_startDivot;
            unsigned m_endDivot;
        };
        HashMap<unsigned, TypeProfilerExpressionRange> m_typeProfilerInfoMap;
        Vector<size_t> m_opProfileControlFlowBytecodeOffsets;
    };

private:
    std::unique_ptr<RareData> m_rareData;
    Vector<ExpressionRangeInfo> m_expressionInfo;

protected:
    static void visitChildren(JSCell*, SlotVisitor&);
    static size_t estimatedSize(JSCell*);

public:
    DECLARE_INFO;
};

}
