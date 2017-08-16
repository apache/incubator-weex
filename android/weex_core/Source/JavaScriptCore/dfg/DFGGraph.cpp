/*
 * Copyright (C) 2011, 2013-2016 Apple Inc. All rights reserved.
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
#include "DFGGraph.h"

#if ENABLE(DFG_JIT)

#include "BytecodeKills.h"
#include "BytecodeLivenessAnalysisInlines.h"
#include "CodeBlock.h"
#include "CodeBlockWithJITType.h"
#include "DFGBackwardsCFG.h"
#include "DFGBackwardsDominators.h"
#include "DFGBlockWorklist.h"
#include "DFGCFG.h"
#include "DFGClobberSet.h"
#include "DFGClobbersExitState.h"
#include "DFGControlEquivalenceAnalysis.h"
#include "DFGDominators.h"
#include "DFGFlowIndexing.h"
#include "DFGFlowMap.h"
#include "DFGJITCode.h"
#include "DFGMayExit.h"
#include "DFGNaturalLoops.h"
#include "DFGPrePostNumbering.h"
#include "DFGVariableAccessDataDump.h"
#include "FullBytecodeLiveness.h"
#include "FunctionExecutableDump.h"
#include "GetterSetter.h"
#include "JIT.h"
#include "JSLexicalEnvironment.h"
#include "MaxFrameExtentForSlowPathCall.h"
#include "OperandsInlines.h"
#include "JSCInlines.h"
#include "StackAlignment.h"
#include <wtf/CommaPrinter.h>
#include <wtf/ListDump.h>

namespace JSC { namespace DFG {

// Creates an array of stringized names.
static const char* dfgOpNames[] = {
#define STRINGIZE_DFG_OP_ENUM(opcode, flags) #opcode ,
    FOR_EACH_DFG_OP(STRINGIZE_DFG_OP_ENUM)
#undef STRINGIZE_DFG_OP_ENUM
};

Graph::Graph(VM& vm, Plan& plan, LongLivedState& longLivedState)
    : m_vm(vm)
    , m_plan(plan)
    , m_codeBlock(m_plan.codeBlock)
    , m_profiledBlock(m_codeBlock->alternative())
    , m_allocator(longLivedState.m_allocator)
    , m_cfg(std::make_unique<CFG>(*this))
    , m_nextMachineLocal(0)
    , m_fixpointState(BeforeFixpoint)
    , m_structureRegistrationState(HaveNotStartedRegistering)
    , m_form(LoadStore)
    , m_unificationState(LocallyUnified)
    , m_refCountState(EverythingIsLive)
{
    ASSERT(m_profiledBlock);
    
    m_hasDebuggerEnabled = m_profiledBlock->wasCompiledWithDebuggingOpcodes() || Options::forceDebuggerBytecodeGeneration();
    
    m_indexingCache = std::make_unique<FlowIndexing>(*this);
    m_abstractValuesCache = std::make_unique<FlowMap<AbstractValue>>(*this);

    registerStructure(vm.structureStructure.get());
    this->stringStructure = registerStructure(vm.stringStructure.get());
    this->symbolStructure = registerStructure(vm.symbolStructure.get());
}

Graph::~Graph()
{
    for (BlockIndex blockIndex = numBlocks(); blockIndex--;) {
        BasicBlock* block = this->block(blockIndex);
        if (!block)
            continue;

        for (unsigned phiIndex = block->phis.size(); phiIndex--;)
            m_allocator.free(block->phis[phiIndex]);
        for (unsigned nodeIndex = block->size(); nodeIndex--;)
            m_allocator.free(block->at(nodeIndex));
    }
    m_allocator.freeAll();
}

const char *Graph::opName(NodeType op)
{
    return dfgOpNames[op];
}

static void printWhiteSpace(PrintStream& out, unsigned amount)
{
    while (amount-- > 0)
        out.print(" ");
}

bool Graph::dumpCodeOrigin(PrintStream& out, const char* prefix, Node*& previousNodeRef, Node* currentNode, DumpContext* context)
{
    if (!currentNode->origin.semantic)
        return false;
    
    Node* previousNode = previousNodeRef;
    previousNodeRef = currentNode;

    if (!previousNode)
        return false;
    
    if (previousNode->origin.semantic.inlineCallFrame == currentNode->origin.semantic.inlineCallFrame)
        return false;
    
    Vector<CodeOrigin> previousInlineStack = previousNode->origin.semantic.inlineStack();
    Vector<CodeOrigin> currentInlineStack = currentNode->origin.semantic.inlineStack();
    unsigned commonSize = std::min(previousInlineStack.size(), currentInlineStack.size());
    unsigned indexOfDivergence = commonSize;
    for (unsigned i = 0; i < commonSize; ++i) {
        if (previousInlineStack[i].inlineCallFrame != currentInlineStack[i].inlineCallFrame) {
            indexOfDivergence = i;
            break;
        }
    }
    
    bool hasPrinted = false;
    
    // Print the pops.
    for (unsigned i = previousInlineStack.size(); i-- > indexOfDivergence;) {
        out.print(prefix);
        printWhiteSpace(out, i * 2);
        out.print("<-- ", inContext(*previousInlineStack[i].inlineCallFrame, context), "\n");
        hasPrinted = true;
    }
    
    // Print the pushes.
    for (unsigned i = indexOfDivergence; i < currentInlineStack.size(); ++i) {
        out.print(prefix);
        printWhiteSpace(out, i * 2);
        out.print("--> ", inContext(*currentInlineStack[i].inlineCallFrame, context), "\n");
        hasPrinted = true;
    }
    
    return hasPrinted;
}

int Graph::amountOfNodeWhiteSpace(Node* node)
{
    return (node->origin.semantic.inlineDepth() - 1) * 2;
}

void Graph::printNodeWhiteSpace(PrintStream& out, Node* node)
{
    printWhiteSpace(out, amountOfNodeWhiteSpace(node));
}

void Graph::dump(PrintStream& out, const char* prefix, Node* node, DumpContext* context)
{
    NodeType op = node->op();

    unsigned refCount = node->refCount();
    bool mustGenerate = node->mustGenerate();
    if (mustGenerate)
        --refCount;

    out.print(prefix);
    printNodeWhiteSpace(out, node);

    // Example/explanation of dataflow dump output
    //
    //   14:   <!2:7>  GetByVal(@3, @13)
    //   ^1     ^2 ^3     ^4       ^5
    //
    // (1) The nodeIndex of this operation.
    // (2) The reference count. The number printed is the 'real' count,
    //     not including the 'mustGenerate' ref. If the node is
    //     'mustGenerate' then the count it prefixed with '!'.
    // (3) The virtual register slot assigned to this node.
    // (4) The name of the operation.
    // (5) The arguments to the operation. The may be of the form:
    //         @#   - a NodeIndex referencing a prior node in the graph.
    //         arg# - an argument number.
    //         id#  - the index in the CodeBlock of an identifier { if codeBlock is passed to dump(), the string representation is displayed }.
    //         var# - the index of a var on the global object, used by GetGlobalVar/GetGlobalLexicalVariable/PutGlobalVariable operations.
    out.printf("% 4d:<%c%u:", (int)node->index(), mustGenerate ? '!' : ' ', refCount);
    if (node->hasResult() && node->hasVirtualRegister() && node->virtualRegister().isValid())
        out.print(node->virtualRegister());
    else
        out.print("-");
    out.print(">\t", opName(op), "(");
    CommaPrinter comma;
    if (node->flags() & NodeHasVarArgs) {
        for (unsigned childIdx = node->firstChild(); childIdx < node->firstChild() + node->numChildren(); childIdx++) {
            if (!m_varArgChildren[childIdx])
                continue;
            out.print(comma, m_varArgChildren[childIdx]);
        }
    } else {
        if (!!node->child1() || !!node->child2() || !!node->child3())
            out.print(comma, node->child1());
        if (!!node->child2() || !!node->child3())
            out.print(comma, node->child2());
        if (!!node->child3())
            out.print(comma, node->child3());
    }

    if (toCString(NodeFlagsDump(node->flags())) != "<empty>")
        out.print(comma, NodeFlagsDump(node->flags()));
    if (node->prediction())
        out.print(comma, SpeculationDump(node->prediction()));
    if (node->hasArrayMode())
        out.print(comma, node->arrayMode());
    if (node->hasArithMode())
        out.print(comma, node->arithMode());
    if (node->hasArithRoundingMode())
        out.print(comma, "Rounding:", node->arithRoundingMode());
    if (node->hasScopeOffset())
        out.print(comma, node->scopeOffset());
    if (node->hasDirectArgumentsOffset())
        out.print(comma, node->capturedArgumentsOffset());
    if (node->hasArgumentIndex())
        out.print(comma, node->argumentIndex());
    if (node->hasRegisterPointer())
        out.print(comma, "global", "(", RawPointer(node->variablePointer()), ")");
    if (node->hasIdentifier())
        out.print(comma, "id", node->identifierNumber(), "{", identifiers()[node->identifierNumber()], "}");
    if (node->hasPromotedLocationDescriptor())
        out.print(comma, node->promotedLocationDescriptor());
    if (node->hasStructureSet())
        out.print(comma, inContext(node->structureSet().toStructureSet(), context));
    if (node->hasStructure())
        out.print(comma, inContext(*node->structure().get(), context));
    if (node->hasTransition()) {
        out.print(comma, pointerDumpInContext(node->transition(), context));
#if USE(JSVALUE64)
        out.print(", ID:", node->transition()->next->id());
#else
        out.print(", ID:", RawPointer(node->transition()->next.get()));
#endif
    }
    if (node->hasCellOperand()) {
        if (!node->cellOperand()->value() || !node->cellOperand()->value().isCell())
            out.print(comma, "invalid cell operand: ", node->cellOperand()->value());
        else {
            out.print(comma, pointerDump(node->cellOperand()->value().asCell()));
            if (node->cellOperand()->value().isCell()) {
                CallVariant variant(node->cellOperand()->value().asCell());
                if (ExecutableBase* executable = variant.executable()) {
                    if (executable->isHostFunction())
                        out.print(comma, "<host function>");
                    else if (FunctionExecutable* functionExecutable = jsDynamicCast<FunctionExecutable*>(m_vm, executable))
                        out.print(comma, FunctionExecutableDump(functionExecutable));
                    else
                        out.print(comma, "<non-function executable>");
                }
            }
        }
    }
    if (node->hasSpeculatedTypeForQuery())
        out.print(comma, SpeculationDump(node->speculatedTypeForQuery()));
    if (node->hasStorageAccessData()) {
        StorageAccessData& storageAccessData = node->storageAccessData();
        out.print(comma, "id", storageAccessData.identifierNumber, "{", identifiers()[storageAccessData.identifierNumber], "}");
        out.print(", ", static_cast<ptrdiff_t>(storageAccessData.offset));
        out.print(", inferredType = ", inContext(storageAccessData.inferredType, context));
    }
    if (node->hasMultiGetByOffsetData()) {
        MultiGetByOffsetData& data = node->multiGetByOffsetData();
        out.print(comma, "id", data.identifierNumber, "{", identifiers()[data.identifierNumber], "}");
        for (unsigned i = 0; i < data.cases.size(); ++i)
            out.print(comma, inContext(data.cases[i], context));
    }
    if (node->hasMultiPutByOffsetData()) {
        MultiPutByOffsetData& data = node->multiPutByOffsetData();
        out.print(comma, "id", data.identifierNumber, "{", identifiers()[data.identifierNumber], "}");
        for (unsigned i = 0; i < data.variants.size(); ++i)
            out.print(comma, inContext(data.variants[i], context));
    }
    ASSERT(node->hasVariableAccessData(*this) == node->accessesStack(*this));
    if (node->hasVariableAccessData(*this)) {
        VariableAccessData* variableAccessData = node->tryGetVariableAccessData();
        if (variableAccessData) {
            VirtualRegister operand = variableAccessData->local();
            out.print(comma, variableAccessData->local(), "(", VariableAccessDataDump(*this, variableAccessData), ")");
            operand = variableAccessData->machineLocal();
            if (operand.isValid())
                out.print(comma, "machine:", operand);
        }
    }
    if (node->hasStackAccessData()) {
        StackAccessData* data = node->stackAccessData();
        out.print(comma, data->local);
        if (data->machineLocal.isValid())
            out.print(comma, "machine:", data->machineLocal);
        out.print(comma, data->format);
    }
    if (node->hasUnlinkedLocal()) 
        out.print(comma, node->unlinkedLocal());
    if (node->hasUnlinkedMachineLocal()) {
        VirtualRegister operand = node->unlinkedMachineLocal();
        if (operand.isValid())
            out.print(comma, "machine:", operand);
    }
    if (node->hasConstantBuffer()) {
        out.print(comma);
        out.print(node->startConstant(), ":[");
        CommaPrinter anotherComma;
        for (unsigned i = 0; i < node->numConstants(); ++i)
            out.print(anotherComma, pointerDumpInContext(freeze(m_codeBlock->constantBuffer(node->startConstant())[i]), context));
        out.print("]");
    }
    if (node->hasLazyJSValue())
        out.print(comma, node->lazyJSValue());
    if (node->hasIndexingType())
        out.print(comma, IndexingTypeDump(node->indexingType()));
    if (node->hasTypedArrayType())
        out.print(comma, node->typedArrayType());
    if (node->hasPhi())
        out.print(comma, "^", node->phi()->index());
    if (node->hasExecutionCounter())
        out.print(comma, RawPointer(node->executionCounter()));
    if (node->hasWatchpointSet())
        out.print(comma, RawPointer(node->watchpointSet()));
    if (node->hasStoragePointer())
        out.print(comma, RawPointer(node->storagePointer()));
    if (node->hasObjectMaterializationData())
        out.print(comma, node->objectMaterializationData());
    if (node->hasCallVarargsData())
        out.print(comma, "firstVarArgOffset = ", node->callVarargsData()->firstVarArgOffset);
    if (node->hasLoadVarargsData()) {
        LoadVarargsData* data = node->loadVarargsData();
        out.print(comma, "start = ", data->start, ", count = ", data->count);
        if (data->machineStart.isValid())
            out.print(", machineStart = ", data->machineStart);
        if (data->machineCount.isValid())
            out.print(", machineCount = ", data->machineCount);
        out.print(", offset = ", data->offset, ", mandatoryMinimum = ", data->mandatoryMinimum);
        out.print(", limit = ", data->limit);
    }
    if (node->hasCallDOMGetterData()) {
        CallDOMGetterData* data = node->callDOMGetterData();
        out.print(comma, "id", data->identifierNumber, "{", identifiers()[data->identifierNumber], "}");
        out.print(", domJIT = ", RawPointer(data->domJIT));
    }
    if (node->isConstant())
        out.print(comma, pointerDumpInContext(node->constant(), context));
    if (node->isJump())
        out.print(comma, "T:", *node->targetBlock());
    if (node->isBranch())
        out.print(comma, "T:", node->branchData()->taken, ", F:", node->branchData()->notTaken);
    if (node->isSwitch()) {
        SwitchData* data = node->switchData();
        out.print(comma, data->kind);
        for (unsigned i = 0; i < data->cases.size(); ++i)
            out.print(comma, inContext(data->cases[i].value, context), ":", data->cases[i].target);
        out.print(comma, "default:", data->fallThrough);
    }
    ClobberSet reads;
    ClobberSet writes;
    addReadsAndWrites(*this, node, reads, writes);
    if (!reads.isEmpty())
        out.print(comma, "R:", sortedListDump(reads.direct(), ","));
    if (!writes.isEmpty())
        out.print(comma, "W:", sortedListDump(writes.direct(), ","));
    ExitMode exitMode = mayExit(*this, node);
    if (exitMode != DoesNotExit)
        out.print(comma, exitMode);
    if (clobbersExitState(*this, node))
        out.print(comma, "ClobbersExit");
    if (node->origin.isSet()) {
        out.print(comma, "bc#", node->origin.semantic.bytecodeIndex);
        if (node->origin.semantic != node->origin.forExit && node->origin.forExit.isSet())
            out.print(comma, "exit: ", node->origin.forExit);
    }
    if (!node->origin.exitOK)
        out.print(comma, "ExitInvalid");
    if (node->origin.wasHoisted)
        out.print(comma, "WasHoisted");
    out.print(")");

    if (node->accessesStack(*this) && node->tryGetVariableAccessData())
        out.print("  predicting ", SpeculationDump(node->tryGetVariableAccessData()->prediction()));
    else if (node->hasHeapPrediction())
        out.print("  predicting ", SpeculationDump(node->getHeapPrediction()));
    
    out.print("\n");
}

bool Graph::terminalsAreValid()
{
    for (BasicBlock* block : blocksInNaturalOrder()) {
        if (!block->terminal())
            return false;
    }
    return true;
}

void Graph::dumpBlockHeader(PrintStream& out, const char* prefix, BasicBlock* block, PhiNodeDumpMode phiNodeDumpMode, DumpContext* context)
{
    out.print(prefix, "Block ", *block, " (", inContext(block->at(0)->origin.semantic, context), "):", block->isReachable ? "" : " (skipped)", block->isOSRTarget ? " (OSR target)" : "", "\n");
    if (block->executionCount == block->executionCount)
        out.print(prefix, "  Execution count: ", block->executionCount, "\n");
    out.print(prefix, "  Predecessors:");
    for (size_t i = 0; i < block->predecessors.size(); ++i)
        out.print(" ", *block->predecessors[i]);
    out.print("\n");
    out.print(prefix, "  Successors:");
    if (block->terminal()) {
        for (BasicBlock* successor : block->successors()) {
            out.print(" ", *successor);
            if (m_prePostNumbering)
                out.print(" (", m_prePostNumbering->edgeKind(block, successor), ")");
        }
    } else
        out.print(" <invalid>");
    out.print("\n");
    if (m_dominators && terminalsAreValid()) {
        out.print(prefix, "  Dominated by: ", m_dominators->dominatorsOf(block), "\n");
        out.print(prefix, "  Dominates: ", m_dominators->blocksDominatedBy(block), "\n");
        out.print(prefix, "  Dominance Frontier: ", m_dominators->dominanceFrontierOf(block), "\n");
        out.print(prefix, "  Iterated Dominance Frontier: ", m_dominators->iteratedDominanceFrontierOf(BlockList(1, block)), "\n");
    }
    if (m_backwardsDominators && terminalsAreValid()) {
        out.print(prefix, "  Backwards dominates by: ", m_backwardsDominators->dominatorsOf(block), "\n");
        out.print(prefix, "  Backwards dominates: ", m_backwardsDominators->blocksDominatedBy(block), "\n");
    }
    if (m_controlEquivalenceAnalysis && terminalsAreValid()) {
        out.print(prefix, "  Control equivalent to:");
        for (BasicBlock* otherBlock : blocksInNaturalOrder()) {
            if (m_controlEquivalenceAnalysis->areEquivalent(block, otherBlock))
                out.print(" ", *otherBlock);
        }
        out.print("\n");
    }
    if (m_prePostNumbering)
        out.print(prefix, "  Pre/Post Numbering: ", m_prePostNumbering->preNumber(block), "/", m_prePostNumbering->postNumber(block), "\n");
    if (m_naturalLoops) {
        if (const NaturalLoop* loop = m_naturalLoops->headerOf(block)) {
            out.print(prefix, "  Loop header, contains:");
            Vector<BlockIndex> sortedBlockList;
            for (unsigned i = 0; i < loop->size(); ++i)
                sortedBlockList.append(loop->at(i)->index);
            std::sort(sortedBlockList.begin(), sortedBlockList.end());
            for (unsigned i = 0; i < sortedBlockList.size(); ++i)
                out.print(" #", sortedBlockList[i]);
            out.print("\n");
        }
        
        Vector<const NaturalLoop*> containingLoops =
            m_naturalLoops->loopsOf(block);
        if (!containingLoops.isEmpty()) {
            out.print(prefix, "  Containing loop headers:");
            for (unsigned i = 0; i < containingLoops.size(); ++i)
                out.print(" ", *containingLoops[i]->header());
            out.print("\n");
        }
    }
    if (!block->phis.isEmpty()) {
        out.print(prefix, "  Phi Nodes:");
        for (size_t i = 0; i < block->phis.size(); ++i) {
            Node* phiNode = block->phis[i];
            if (!phiNode->shouldGenerate() && phiNodeDumpMode == DumpLivePhisOnly)
                continue;
            out.print(" @", phiNode->index(), "<", phiNode->local(), ",", phiNode->refCount(), ">->(");
            if (phiNode->child1()) {
                out.print("@", phiNode->child1()->index());
                if (phiNode->child2()) {
                    out.print(", @", phiNode->child2()->index());
                    if (phiNode->child3())
                        out.print(", @", phiNode->child3()->index());
                }
            }
            out.print(")", i + 1 < block->phis.size() ? "," : "");
        }
        out.print("\n");
    }
}

void Graph::dump(PrintStream& out, DumpContext* context)
{
    DumpContext myContext;
    myContext.graph = this;
    if (!context)
        context = &myContext;
    
    out.print("\n");
    out.print("DFG for ", CodeBlockWithJITType(m_codeBlock, JITCode::DFGJIT), ":\n");
    out.print("  Fixpoint state: ", m_fixpointState, "; Form: ", m_form, "; Unification state: ", m_unificationState, "; Ref count state: ", m_refCountState, "\n");
    if (m_form == SSA)
        out.print("  Argument formats: ", listDump(m_argumentFormats), "\n");
    else
        out.print("  Arguments: ", listDump(m_arguments), "\n");
    out.print("\n");
    
    Node* lastNode = nullptr;
    for (size_t b = 0; b < m_blocks.size(); ++b) {
        BasicBlock* block = m_blocks[b].get();
        if (!block)
            continue;
        dumpBlockHeader(out, "", block, DumpAllPhis, context);
        out.print("  States: ", block->cfaStructureClobberStateAtHead);
        if (!block->cfaHasVisited)
            out.print(", CurrentlyCFAUnreachable");
        if (!block->intersectionOfCFAHasVisited)
            out.print(", CFAUnreachable");
        out.print("\n");
        switch (m_form) {
        case LoadStore:
        case ThreadedCPS: {
            out.print("  Vars Before: ");
            if (block->cfaHasVisited)
                out.print(inContext(block->valuesAtHead, context));
            else
                out.print("<empty>");
            out.print("\n");
            out.print("  Intersected Vars Before: ");
            if (block->intersectionOfCFAHasVisited)
                out.print(inContext(block->intersectionOfPastValuesAtHead, context));
            else
                out.print("<empty>");
            out.print("\n");
            out.print("  Var Links: ", block->variablesAtHead, "\n");
            break;
        }
            
        case SSA: {
            RELEASE_ASSERT(block->ssa);
            out.print("  Availability: ", block->ssa->availabilityAtHead, "\n");
            out.print("  Live: ", nodeListDump(block->ssa->liveAtHead), "\n");
            out.print("  Values: ", nodeValuePairListDump(block->ssa->valuesAtHead, context), "\n");
            break;
        } }
        for (size_t i = 0; i < block->size(); ++i) {
            dumpCodeOrigin(out, "", lastNode, block->at(i), context);
            dump(out, "", block->at(i), context);
        }
        out.print("  States: ", block->cfaBranchDirection, ", ", block->cfaStructureClobberStateAtTail);
        if (!block->cfaDidFinish)
            out.print(", CFAInvalidated");
        out.print("\n");
        switch (m_form) {
        case LoadStore:
        case ThreadedCPS: {
            out.print("  Vars After: ");
            if (block->cfaHasVisited)
                out.print(inContext(block->valuesAtTail, context));
            else
                out.print("<empty>");
            out.print("\n");
            out.print("  Var Links: ", block->variablesAtTail, "\n");
            break;
        }
            
        case SSA: {
            RELEASE_ASSERT(block->ssa);
            out.print("  Availability: ", block->ssa->availabilityAtTail, "\n");
            out.print("  Live: ", nodeListDump(block->ssa->liveAtTail), "\n");
            out.print("  Values: ", nodeValuePairListDump(block->ssa->valuesAtTail, context), "\n");
            break;
        } }
        out.print("\n");
    }
    
    out.print("GC Values:\n");
    for (FrozenValue* value : m_frozenValues) {
        if (value->pointsToHeap())
            out.print("    ", inContext(*value, &myContext), "\n");
    }

    out.print(inContext(watchpoints(), &myContext));
    
    if (!myContext.isEmpty()) {
        myContext.dump(out);
        out.print("\n");
    }
}

void Graph::addNodeToMapByIndex(Node* node)
{
    if (m_nodeIndexFreeList.isEmpty()) {
        node->m_index = m_nodesByIndex.size();
        m_nodesByIndex.append(node);
        return;
    }
    unsigned index = m_nodeIndexFreeList.takeLast();
    node->m_index = index;
    ASSERT(!m_nodesByIndex[index]);
    m_nodesByIndex[index] = node;
}

void Graph::deleteNode(Node* node)
{
    if (validationEnabled() && m_form == SSA) {
        for (BasicBlock* block : blocksInNaturalOrder()) {
            DFG_ASSERT(*this, node, !block->ssa->liveAtHead.contains(node));
            DFG_ASSERT(*this, node, !block->ssa->liveAtTail.contains(node));
        }
    }

    RELEASE_ASSERT(m_nodesByIndex[node->m_index] == node);
    unsigned nodeIndex = node->m_index;
    m_nodesByIndex[nodeIndex] = nullptr;
    m_nodeIndexFreeList.append(nodeIndex);

    m_allocator.free(node);
}

void Graph::packNodeIndices()
{
    if (m_nodeIndexFreeList.isEmpty())
        return;

    unsigned holeIndex = 0;
    unsigned endIndex = m_nodesByIndex.size();

    while (true) {
        while (holeIndex < endIndex && m_nodesByIndex[holeIndex])
            ++holeIndex;

        if (holeIndex == endIndex)
            break;
        ASSERT(holeIndex < m_nodesByIndex.size());
        ASSERT(!m_nodesByIndex[holeIndex]);

        do {
            --endIndex;
        } while (!m_nodesByIndex[endIndex] && endIndex > holeIndex);

        if (holeIndex == endIndex)
            break;
        ASSERT(endIndex > holeIndex);
        ASSERT(m_nodesByIndex[endIndex]);

        auto& value = m_nodesByIndex[endIndex];
        value->m_index = holeIndex;
        m_nodesByIndex[holeIndex] = WTFMove(value);
        ++holeIndex;
    }

    m_nodeIndexFreeList.resize(0);
    m_nodesByIndex.resize(endIndex);
}

void Graph::dethread()
{
    if (m_form == LoadStore || m_form == SSA)
        return;
    
    if (logCompilationChanges())
        dataLog("Dethreading DFG graph.\n");
    
    for (BlockIndex blockIndex = m_blocks.size(); blockIndex--;) {
        BasicBlock* block = m_blocks[blockIndex].get();
        if (!block)
            continue;
        for (unsigned phiIndex = block->phis.size(); phiIndex--;) {
            Node* phi = block->phis[phiIndex];
            phi->children.reset();
        }
    }
    
    m_form = LoadStore;
}

void Graph::handleSuccessor(Vector<BasicBlock*, 16>& worklist, BasicBlock* block, BasicBlock* successor)
{
    if (!successor->isReachable) {
        successor->isReachable = true;
        worklist.append(successor);
    }
    
    successor->predecessors.append(block);
}

void Graph::determineReachability()
{
    Vector<BasicBlock*, 16> worklist;
    worklist.append(block(0));
    block(0)->isReachable = true;
    while (!worklist.isEmpty()) {
        BasicBlock* block = worklist.takeLast();
        for (unsigned i = block->numSuccessors(); i--;)
            handleSuccessor(worklist, block, block->successor(i));
    }
}

void Graph::resetReachability()
{
    for (BlockIndex blockIndex = m_blocks.size(); blockIndex--;) {
        BasicBlock* block = m_blocks[blockIndex].get();
        if (!block)
            continue;
        block->isReachable = false;
        block->predecessors.clear();
    }
    
    determineReachability();
}

namespace {

class RefCountCalculator {
public:
    RefCountCalculator(Graph& graph)
        : m_graph(graph)
    {
    }
    
    void calculate()
    {
        // First reset the counts to 0 for all nodes.
        for (BlockIndex blockIndex = 0; blockIndex < m_graph.numBlocks(); ++blockIndex) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            for (unsigned indexInBlock = block->size(); indexInBlock--;)
                block->at(indexInBlock)->setRefCount(0);
            for (unsigned phiIndex = block->phis.size(); phiIndex--;)
                block->phis[phiIndex]->setRefCount(0);
        }
    
        // Now find the roots:
        // - Nodes that are must-generate.
        // - Nodes that are reachable from type checks.
        // Set their ref counts to 1 and put them on the worklist.
        for (BlockIndex blockIndex = 0; blockIndex < m_graph.numBlocks(); ++blockIndex) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            for (unsigned indexInBlock = block->size(); indexInBlock--;) {
                Node* node = block->at(indexInBlock);
                DFG_NODE_DO_TO_CHILDREN(m_graph, node, findTypeCheckRoot);
                if (!(node->flags() & NodeMustGenerate))
                    continue;
                if (!node->postfixRef())
                    m_worklist.append(node);
            }
        }
        
        while (!m_worklist.isEmpty()) {
            while (!m_worklist.isEmpty()) {
                Node* node = m_worklist.last();
                m_worklist.removeLast();
                ASSERT(node->shouldGenerate()); // It should not be on the worklist unless it's ref'ed.
                DFG_NODE_DO_TO_CHILDREN(m_graph, node, countEdge);
            }
            
            if (m_graph.m_form == SSA) {
                // Find Phi->Upsilon edges, which are represented as meta-data in the
                // Upsilon.
                for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
                    BasicBlock* block = m_graph.block(blockIndex);
                    if (!block)
                        continue;
                    for (unsigned nodeIndex = block->size(); nodeIndex--;) {
                        Node* node = block->at(nodeIndex);
                        if (node->op() != Upsilon)
                            continue;
                        if (node->shouldGenerate())
                            continue;
                        if (node->phi()->shouldGenerate())
                            countNode(node);
                    }
                }
            }
        }
    }
    
private:
    void findTypeCheckRoot(Node*, Edge edge)
    {
        // We may have an "unproved" untyped use for code that is unreachable. The CFA
        // will just not have gotten around to it.
        if (edge.isProved() || edge.willNotHaveCheck())
            return;
        if (!edge->postfixRef())
            m_worklist.append(edge.node());
    }
    
    void countNode(Node* node)
    {
        if (node->postfixRef())
            return;
        m_worklist.append(node);
    }
    
    void countEdge(Node*, Edge edge)
    {
        // Don't count edges that are already counted for their type checks.
        if (!(edge.isProved() || edge.willNotHaveCheck()))
            return;
        countNode(edge.node());
    }
    
    Graph& m_graph;
    Vector<Node*, 128> m_worklist;
};

} // anonymous namespace

void Graph::computeRefCounts()
{
    RefCountCalculator calculator(*this);
    calculator.calculate();
}

void Graph::killBlockAndItsContents(BasicBlock* block)
{
    if (auto& ssaData = block->ssa)
        ssaData->invalidate();
    for (unsigned phiIndex = block->phis.size(); phiIndex--;)
        deleteNode(block->phis[phiIndex]);
    for (Node* node : *block)
        deleteNode(node);
    
    killBlock(block);
}

void Graph::killUnreachableBlocks()
{
    invalidateNodeLiveness();

    for (BlockIndex blockIndex = 0; blockIndex < numBlocks(); ++blockIndex) {
        BasicBlock* block = this->block(blockIndex);
        if (!block)
            continue;
        if (block->isReachable)
            continue;
        
        killBlockAndItsContents(block);
    }
}

void Graph::invalidateCFG()
{
    m_dominators = nullptr;
    m_naturalLoops = nullptr;
    m_prePostNumbering = nullptr;
    m_controlEquivalenceAnalysis = nullptr;
    m_backwardsDominators = nullptr;
    m_backwardsCFG = nullptr;
}

void Graph::invalidateNodeLiveness()
{
    if (m_form != SSA)
        return;

    for (BasicBlock* block : blocksInNaturalOrder())
        block->ssa->invalidate();
}

void Graph::substituteGetLocal(BasicBlock& block, unsigned startIndexInBlock, VariableAccessData* variableAccessData, Node* newGetLocal)
{
    for (unsigned indexInBlock = startIndexInBlock; indexInBlock < block.size(); ++indexInBlock) {
        Node* node = block[indexInBlock];
        bool shouldContinue = true;
        switch (node->op()) {
        case SetLocal: {
            if (node->local() == variableAccessData->local())
                shouldContinue = false;
            break;
        }
                
        case GetLocal: {
            if (node->variableAccessData() != variableAccessData)
                continue;
            substitute(block, indexInBlock, node, newGetLocal);
            Node* oldTailNode = block.variablesAtTail.operand(variableAccessData->local());
            if (oldTailNode == node)
                block.variablesAtTail.operand(variableAccessData->local()) = newGetLocal;
            shouldContinue = false;
            break;
        }
                
        default:
            break;
        }
        if (!shouldContinue)
            break;
    }
}

BlockList Graph::blocksInPreOrder()
{
    BlockList result;
    BlockWorklist worklist;
    worklist.push(block(0));
    while (BasicBlock* block = worklist.pop()) {
        result.append(block);
        for (unsigned i = block->numSuccessors(); i--;)
            worklist.push(block->successor(i));
    }
    return result;
}

BlockList Graph::blocksInPostOrder()
{
    BlockList result;
    PostOrderBlockWorklist worklist;
    worklist.push(block(0));
    while (BlockWithOrder item = worklist.pop()) {
        switch (item.order) {
        case VisitOrder::Pre:
            worklist.pushPost(item.node);
            for (unsigned i = item.node->numSuccessors(); i--;)
                worklist.push(item.node->successor(i));
            break;
        case VisitOrder::Post:
            result.append(item.node);
            break;
        }
    }
    return result;
}

void Graph::clearReplacements()
{
    for (BlockIndex blockIndex = numBlocks(); blockIndex--;) {
        BasicBlock* block = m_blocks[blockIndex].get();
        if (!block)
            continue;
        for (unsigned phiIndex = block->phis.size(); phiIndex--;)
            block->phis[phiIndex]->setReplacement(nullptr);
        for (unsigned nodeIndex = block->size(); nodeIndex--;)
            block->at(nodeIndex)->setReplacement(nullptr);
    }
}

void Graph::clearEpochs()
{
    for (BlockIndex blockIndex = numBlocks(); blockIndex--;) {
        BasicBlock* block = m_blocks[blockIndex].get();
        if (!block)
            continue;
        for (unsigned phiIndex = block->phis.size(); phiIndex--;)
            block->phis[phiIndex]->setEpoch(Epoch());
        for (unsigned nodeIndex = block->size(); nodeIndex--;)
            block->at(nodeIndex)->setEpoch(Epoch());
    }
}

void Graph::initializeNodeOwners()
{
    for (BlockIndex blockIndex = numBlocks(); blockIndex--;) {
        BasicBlock* block = m_blocks[blockIndex].get();
        if (!block)
            continue;
        for (unsigned phiIndex = block->phis.size(); phiIndex--;)
            block->phis[phiIndex]->owner = block;
        for (unsigned nodeIndex = block->size(); nodeIndex--;)
            block->at(nodeIndex)->owner = block;
    }
}

void Graph::clearFlagsOnAllNodes(NodeFlags flags)
{
    for (BlockIndex blockIndex = numBlocks(); blockIndex--;) {
        BasicBlock* block = m_blocks[blockIndex].get();
        if (!block)
            continue;
        for (unsigned phiIndex = block->phis.size(); phiIndex--;)
            block->phis[phiIndex]->clearFlags(flags);
        for (unsigned nodeIndex = block->size(); nodeIndex--;)
            block->at(nodeIndex)->clearFlags(flags);
    }
}

bool Graph::watchCondition(const ObjectPropertyCondition& key)
{
    if (!key.isWatchable())
        return false;
    
    m_plan.weakReferences.addLazily(key.object());
    if (key.hasPrototype())
        m_plan.weakReferences.addLazily(key.prototype());
    if (key.hasRequiredValue())
        m_plan.weakReferences.addLazily(key.requiredValue());
    
    m_plan.watchpoints.addLazily(key);

    if (key.kind() == PropertyCondition::Presence)
        m_safeToLoad.add(std::make_pair(key.object(), key.offset()));
    
    return true;
}

bool Graph::watchConditions(const ObjectPropertyConditionSet& keys)
{
    if (!keys.isValid())
        return false;

    for (const ObjectPropertyCondition& key : keys) {
        if (!watchCondition(key))
            return false;
    }
    return true;
}

bool Graph::isSafeToLoad(JSObject* base, PropertyOffset offset)
{
    return m_safeToLoad.contains(std::make_pair(base, offset));
}

InferredType::Descriptor Graph::inferredTypeFor(const PropertyTypeKey& key)
{
    assertIsRegistered(key.structure());
    
    auto iter = m_inferredTypes.find(key);
    if (iter != m_inferredTypes.end())
        return iter->value;

    InferredType* typeObject = key.structure()->inferredTypeFor(key.uid());
    if (!typeObject) {
        m_inferredTypes.add(key, InferredType::Top);
        return InferredType::Top;
    }

    InferredType::Descriptor typeDescriptor = typeObject->descriptor();
    if (typeDescriptor.kind() == InferredType::Top) {
        m_inferredTypes.add(key, InferredType::Top);
        return InferredType::Top;
    }
    
    m_inferredTypes.add(key, typeDescriptor);

    m_plan.weakReferences.addLazily(typeObject);
    registerInferredType(typeDescriptor);

    // Note that we may already be watching this desired inferred type, because multiple structures may
    // point to the same InferredType instance.
    m_plan.watchpoints.addLazily(DesiredInferredType(typeObject, typeDescriptor));

    return typeDescriptor;
}

FullBytecodeLiveness& Graph::livenessFor(CodeBlock* codeBlock)
{
    HashMap<CodeBlock*, std::unique_ptr<FullBytecodeLiveness>>::iterator iter = m_bytecodeLiveness.find(codeBlock);
    if (iter != m_bytecodeLiveness.end())
        return *iter->value;
    
    std::unique_ptr<FullBytecodeLiveness> liveness = std::make_unique<FullBytecodeLiveness>();
    codeBlock->livenessAnalysis().computeFullLiveness(*liveness);
    FullBytecodeLiveness& result = *liveness;
    m_bytecodeLiveness.add(codeBlock, WTFMove(liveness));
    return result;
}

FullBytecodeLiveness& Graph::livenessFor(InlineCallFrame* inlineCallFrame)
{
    return livenessFor(baselineCodeBlockFor(inlineCallFrame));
}

BytecodeKills& Graph::killsFor(CodeBlock* codeBlock)
{
    HashMap<CodeBlock*, std::unique_ptr<BytecodeKills>>::iterator iter = m_bytecodeKills.find(codeBlock);
    if (iter != m_bytecodeKills.end())
        return *iter->value;
    
    std::unique_ptr<BytecodeKills> kills = std::make_unique<BytecodeKills>();
    codeBlock->livenessAnalysis().computeKills(*kills);
    BytecodeKills& result = *kills;
    m_bytecodeKills.add(codeBlock, WTFMove(kills));
    return result;
}

BytecodeKills& Graph::killsFor(InlineCallFrame* inlineCallFrame)
{
    return killsFor(baselineCodeBlockFor(inlineCallFrame));
}

bool Graph::isLiveInBytecode(VirtualRegister operand, CodeOrigin codeOrigin)
{
    static const bool verbose = false;
    
    if (verbose)
        dataLog("Checking of operand is live: ", operand, "\n");
    CodeOrigin* codeOriginPtr = &codeOrigin;
    for (;;) {
        VirtualRegister reg = VirtualRegister(
            operand.offset() - codeOriginPtr->stackOffset());
        
        if (verbose)
            dataLog("reg = ", reg, "\n");
        
        if (operand.offset() < codeOriginPtr->stackOffset() + CallFrame::headerSizeInRegisters) {
            if (reg.isArgument()) {
                RELEASE_ASSERT(reg.offset() < CallFrame::headerSizeInRegisters);
                
                if (codeOriginPtr->inlineCallFrame->isClosureCall
                    && reg.offset() == CallFrameSlot::callee) {
                    if (verbose)
                        dataLog("Looks like a callee.\n");
                    return true;
                }
                
                if (codeOriginPtr->inlineCallFrame->isVarargs()
                    && reg.offset() == CallFrameSlot::argumentCount) {
                    if (verbose)
                        dataLog("Looks like the argument count.\n");
                    return true;
                }
                
                return false;
            }

            if (verbose)
                dataLog("Asking the bytecode liveness.\n");
            return livenessFor(codeOriginPtr->inlineCallFrame).operandIsLive(
                reg.offset(), codeOriginPtr->bytecodeIndex);
        }
        
        InlineCallFrame* inlineCallFrame = codeOriginPtr->inlineCallFrame;
        if (!inlineCallFrame) {
            if (verbose)
                dataLog("Ran out of stack, returning true.\n");
            return true;
        }

        // Arguments are always live. This would be redundant if it wasn't for our
        // op_call_varargs inlining.
        if (reg.isArgument()
            && static_cast<size_t>(reg.toArgument()) < inlineCallFrame->arguments.size()) {
            if (verbose)
                dataLog("Argument is live.\n");
            return true;
        }
        
        codeOriginPtr = inlineCallFrame->getCallerSkippingTailCalls();

        // The first inline call frame could be an inline tail call
        if (!codeOriginPtr) {
            if (verbose)
                dataLog("Dead because of tail inlining.\n");
            return false;
        }
    }
    
    RELEASE_ASSERT_NOT_REACHED();
}

BitVector Graph::localsLiveInBytecode(CodeOrigin codeOrigin)
{
    BitVector result;
    result.ensureSize(block(0)->variablesAtHead.numberOfLocals());
    forAllLocalsLiveInBytecode(
        codeOrigin,
        [&] (VirtualRegister reg) {
            ASSERT(reg.isLocal());
            result.quickSet(reg.toLocal());
        });
    return result;
}

unsigned Graph::parameterSlotsForArgCount(unsigned argCount)
{
    size_t frameSize = CallFrame::headerSizeInRegisters + argCount;
    size_t alignedFrameSize = WTF::roundUpToMultipleOf(stackAlignmentRegisters(), frameSize);
    return alignedFrameSize - CallerFrameAndPC::sizeInRegisters;
}

unsigned Graph::frameRegisterCount()
{
    unsigned result = m_nextMachineLocal + std::max(m_parameterSlots, static_cast<unsigned>(maxFrameExtentForSlowPathCallInRegisters));
    return roundLocalRegisterCountForFramePointerOffset(result);
}

unsigned Graph::stackPointerOffset()
{
    return virtualRegisterForLocal(frameRegisterCount() - 1).offset();
}

unsigned Graph::requiredRegisterCountForExit()
{
    unsigned count = JIT::frameRegisterCountFor(m_profiledBlock);
    for (InlineCallFrameSet::iterator iter = m_plan.inlineCallFrames->begin(); !!iter; ++iter) {
        InlineCallFrame* inlineCallFrame = *iter;
        CodeBlock* codeBlock = baselineCodeBlockForInlineCallFrame(inlineCallFrame);
        unsigned requiredCount = VirtualRegister(inlineCallFrame->stackOffset).toLocal() + 1 + JIT::frameRegisterCountFor(codeBlock);
        count = std::max(count, requiredCount);
    }
    return count;
}

unsigned Graph::requiredRegisterCountForExecutionAndExit()
{
    return std::max(frameRegisterCount(), requiredRegisterCountForExit());
}

JSValue Graph::tryGetConstantProperty(
    JSValue base, const RegisteredStructureSet& structureSet, PropertyOffset offset)
{
    if (!base || !base.isObject())
        return JSValue();
    
    JSObject* object = asObject(base);
    
    for (unsigned i = structureSet.size(); i--;) {
        RegisteredStructure structure = structureSet[i];
        
        WatchpointSet* set = structure->propertyReplacementWatchpointSet(offset);
        if (!set || !set->isStillValid())
            return JSValue();
        
        ASSERT(structure->isValidOffset(offset));
        ASSERT(!structure->isUncacheableDictionary());
        
        watchpoints().addLazily(set);
    }
    
    // What follows may require some extra thought. We need this load to load a valid JSValue. If
    // our profiling makes sense and we're still on track to generate code that won't be
    // invalidated, then we have nothing to worry about. We do, however, have to worry about
    // loading - and then using - an invalid JSValue in the case that unbeknownst to us our code
    // is doomed.
    //
    // One argument in favor of this code is that it should definitely work because the butterfly
    // is always set before the structure. However, we don't currently have a fence between those
    // stores. It's not clear if this matters, however. We don't ever shrink the property storage.
    // So, for this to fail, you'd need an access on a constant object pointer such that the inline
    // caches told us that the object had a structure that it did not *yet* have, and then later,
    // the object transitioned to that structure that the inline caches had alraedy seen. And then
    // the processor reordered the stores. Seems unlikely and difficult to test. I believe that
    // this is worth revisiting but it isn't worth losing sleep over. Filed:
    // https://bugs.webkit.org/show_bug.cgi?id=134641
    //
    // For now, we just do the minimal thing: defend against the structure right now being
    // incompatible with the getDirect we're trying to do. The easiest way to do that is to
    // determine if the structure belongs to the proven set.
    
    if (!structureSet.toStructureSet().contains(object->structure()))
        return JSValue();
    
    return object->getDirect(offset);
}

JSValue Graph::tryGetConstantProperty(JSValue base, Structure* structure, PropertyOffset offset)
{
    return tryGetConstantProperty(base, RegisteredStructureSet(registerStructure(structure)), offset);
}

JSValue Graph::tryGetConstantProperty(
    JSValue base, const StructureAbstractValue& structure, PropertyOffset offset)
{
    if (structure.isInfinite()) {
        // FIXME: If we just converted the offset to a uid, we could do ObjectPropertyCondition
        // watching to constant-fold the property.
        // https://bugs.webkit.org/show_bug.cgi?id=147271
        return JSValue();
    }
    
    return tryGetConstantProperty(base, structure.set(), offset);
}

JSValue Graph::tryGetConstantProperty(const AbstractValue& base, PropertyOffset offset)
{
    return tryGetConstantProperty(base.m_value, base.m_structure, offset);
}

AbstractValue Graph::inferredValueForProperty(
    const RegisteredStructureSet& base, UniquedStringImpl* uid, StructureClobberState clobberState)
{
    AbstractValue result;
    base.forEach(
        [&] (RegisteredStructure structure) {
            AbstractValue value;
            value.set(*this, inferredTypeForProperty(structure.get(), uid));
            result.merge(value);
        });
    if (clobberState == StructuresAreClobbered)
        result.clobberStructures();
    return result;
}

AbstractValue Graph::inferredValueForProperty(
    const AbstractValue& base, UniquedStringImpl* uid, PropertyOffset offset,
    StructureClobberState clobberState)
{
    if (JSValue value = tryGetConstantProperty(base, offset)) {
        AbstractValue result;
        result.set(*this, *freeze(value), clobberState);
        return result;
    }

    if (base.m_structure.isFinite())
        return inferredValueForProperty(base.m_structure.set(), uid, clobberState);

    return AbstractValue::heapTop();
}

JSValue Graph::tryGetConstantClosureVar(JSValue base, ScopeOffset offset)
{
    // This has an awesome concurrency story. See comment for GetGlobalVar in ByteCodeParser.
    
    if (!base)
        return JSValue();
    
    JSLexicalEnvironment* activation = jsDynamicCast<JSLexicalEnvironment*>(m_vm, base);
    if (!activation)
        return JSValue();
    
    SymbolTable* symbolTable = activation->symbolTable();
    JSValue value;
    WatchpointSet* set;
    {
        ConcurrentJSLocker locker(symbolTable->m_lock);
        
        SymbolTableEntry* entry = symbolTable->entryFor(locker, offset);
        if (!entry)
            return JSValue();
        
        set = entry->watchpointSet();
        if (!set)
            return JSValue();
        
        if (set->state() != IsWatched)
            return JSValue();
        
        ASSERT(entry->scopeOffset() == offset);
        value = activation->variableAt(offset).get();
        if (!value)
            return JSValue();
    }
    
    watchpoints().addLazily(set);
    
    return value;
}

JSValue Graph::tryGetConstantClosureVar(const AbstractValue& value, ScopeOffset offset)
{
    return tryGetConstantClosureVar(value.m_value, offset);
}

JSValue Graph::tryGetConstantClosureVar(Node* node, ScopeOffset offset)
{
    if (!node->hasConstant())
        return JSValue();
    return tryGetConstantClosureVar(node->asJSValue(), offset);
}

JSArrayBufferView* Graph::tryGetFoldableView(JSValue value)
{
    if (!value)
        return nullptr;
    JSArrayBufferView* view = jsDynamicCast<JSArrayBufferView*>(m_vm, value);
    if (!value)
        return nullptr;
    if (!view->length())
        return nullptr;
    WTF::loadLoadFence();
    watchpoints().addLazily(view);
    return view;
}

JSArrayBufferView* Graph::tryGetFoldableView(JSValue value, ArrayMode arrayMode)
{
    if (arrayMode.type() != Array::AnyTypedArray && arrayMode.typedArrayType() == NotTypedArray)
        return nullptr;
    return tryGetFoldableView(value);
}

void Graph::registerFrozenValues()
{
    m_codeBlock->constants().resize(0);
    m_codeBlock->constantsSourceCodeRepresentation().resize(0);
    for (FrozenValue* value : m_frozenValues) {
        if (!value->pointsToHeap())
            continue;
        
        ASSERT(value->structure());
        ASSERT(m_plan.weakReferences.contains(value->structure()));
        
        switch (value->strength()) {
        case WeakValue: {
            m_plan.weakReferences.addLazily(value->value().asCell());
            break;
        }
        case StrongValue: {
            unsigned constantIndex = m_codeBlock->addConstantLazily();
            // We already have a barrier on the code block.
            m_codeBlock->constants()[constantIndex].setWithoutWriteBarrier(value->value());
            break;
        } }
    }
    m_codeBlock->constants().shrinkToFit();
    m_codeBlock->constantsSourceCodeRepresentation().shrinkToFit();
}

void Graph::visitChildren(SlotVisitor& visitor)
{
    for (FrozenValue* value : m_frozenValues) {
        visitor.appendUnbarriered(value->value());
        visitor.appendUnbarriered(value->structure());
    }
}

FrozenValue* Graph::freeze(JSValue value)
{
    if (UNLIKELY(!value))
        return FrozenValue::emptySingleton();

    // There are weird relationships in how optimized CodeBlocks
    // point to other CodeBlocks. We don't want to have them be
    // part of the weak pointer set. For example, an optimized CodeBlock
    // having a weak pointer to itself will cause it to get collected.
    RELEASE_ASSERT(!jsDynamicCast<CodeBlock*>(m_vm, value));
    
    auto result = m_frozenValueMap.add(JSValue::encode(value), nullptr);
    if (LIKELY(!result.isNewEntry))
        return result.iterator->value;

    if (value.isUInt32())
        m_uint32ValuesInUse.append(value.asUInt32());
    
    FrozenValue frozenValue = FrozenValue::freeze(value);
    if (Structure* structure = frozenValue.structure())
        registerStructure(structure);
    
    return result.iterator->value = m_frozenValues.add(frozenValue);
}

FrozenValue* Graph::freezeStrong(JSValue value)
{
    FrozenValue* result = freeze(value);
    result->strengthenTo(StrongValue);
    return result;
}

void Graph::convertToConstant(Node* node, FrozenValue* value)
{
    if (value->structure())
        assertIsRegistered(value->structure());
    node->convertToConstant(value);
}

void Graph::convertToConstant(Node* node, JSValue value)
{
    convertToConstant(node, freeze(value));
}

void Graph::convertToStrongConstant(Node* node, JSValue value)
{
    convertToConstant(node, freezeStrong(value));
}

RegisteredStructure Graph::registerStructure(Structure* structure, StructureRegistrationResult& result)
{
    m_plan.weakReferences.addLazily(structure);
    if (m_plan.watchpoints.consider(structure))
        result = StructureRegisteredAndWatched;
    else
        result = StructureRegisteredNormally;
    return RegisteredStructure::createPrivate(structure);
}

void Graph::assertIsRegistered(Structure* structure)
{
    // It's convenient to be able to call this with a maybe-null structure.
    if (!structure)
        return;
    
    DFG_ASSERT(*this, nullptr, m_plan.weakReferences.contains(structure));
    
    if (!structure->dfgShouldWatch())
        return;
    if (watchpoints().isWatched(structure->transitionWatchpointSet()))
        return;
    
    DFG_CRASH(*this, nullptr, toCString("Structure ", pointerDump(structure), " is watchable but isn't being watched.").data());
}

NO_RETURN_DUE_TO_CRASH static void crash(
    Graph& graph, const CString& whileText, const char* file, int line, const char* function,
    const char* assertion)
{
    startCrashing();
    dataLog("DFG ASSERTION FAILED: ", assertion, "\n");
    dataLog(file, "(", line, ") : ", function, "\n");
    dataLog("\n");
    dataLog(whileText);
    dataLog("Graph at time of failure:\n");
    graph.dump();
    dataLog("\n");
    dataLog("DFG ASSERTION FAILED: ", assertion, "\n");
    dataLog(file, "(", line, ") : ", function, "\n");
    CRASH_WITH_SECURITY_IMPLICATION();
}

void Graph::handleAssertionFailure(
    std::nullptr_t, const char* file, int line, const char* function, const char* assertion)
{
    crash(*this, "", file, line, function, assertion);
}

void Graph::handleAssertionFailure(
    Node* node, const char* file, int line, const char* function, const char* assertion)
{
    crash(*this, toCString("While handling node ", node, "\n\n"), file, line, function, assertion);
}

void Graph::handleAssertionFailure(
    BasicBlock* block, const char* file, int line, const char* function, const char* assertion)
{
    crash(*this, toCString("While handling block ", pointerDump(block), "\n\n"), file, line, function, assertion);
}

Dominators& Graph::ensureDominators()
{
    if (!m_dominators)
        m_dominators = std::make_unique<Dominators>(*this);
    return *m_dominators;
}

PrePostNumbering& Graph::ensurePrePostNumbering()
{
    if (!m_prePostNumbering)
        m_prePostNumbering = std::make_unique<PrePostNumbering>(*this);
    return *m_prePostNumbering;
}

NaturalLoops& Graph::ensureNaturalLoops()
{
    ensureDominators();
    if (!m_naturalLoops)
        m_naturalLoops = std::make_unique<NaturalLoops>(*this);
    return *m_naturalLoops;
}

BackwardsCFG& Graph::ensureBackwardsCFG()
{
    if (!m_backwardsCFG)
        m_backwardsCFG = std::make_unique<BackwardsCFG>(*this);
    return *m_backwardsCFG;
}

BackwardsDominators& Graph::ensureBackwardsDominators()
{
    if (!m_backwardsDominators)
        m_backwardsDominators = std::make_unique<BackwardsDominators>(*this);
    return *m_backwardsDominators;
}

ControlEquivalenceAnalysis& Graph::ensureControlEquivalenceAnalysis()
{
    if (!m_controlEquivalenceAnalysis)
        m_controlEquivalenceAnalysis = std::make_unique<ControlEquivalenceAnalysis>(*this);
    return *m_controlEquivalenceAnalysis;
}

MethodOfGettingAValueProfile Graph::methodOfGettingAValueProfileFor(Node* currentNode, Node* operandNode)
{
    for (Node* node = operandNode; node;) {
        // currentNode is null when we're doing speculation checks for checkArgumentTypes().
        if (!currentNode || node->origin != currentNode->origin) {
            CodeBlock* profiledBlock = baselineCodeBlockFor(node->origin.semantic);

            if (node->accessesStack(*this)) {
                ValueProfile* result = [&] () -> ValueProfile* {
                    if (!node->local().isArgument())
                        return nullptr;
                    int argument = node->local().toArgument();
                    Node* argumentNode = m_arguments[argument];
                    if (!argumentNode)
                        return nullptr;
                    if (node->variableAccessData() != argumentNode->variableAccessData())
                        return nullptr;
                    return profiledBlock->valueProfileForArgument(argument);
                }();
                if (result)
                    return result;

                if (node->op() == GetLocal) {
                    return MethodOfGettingAValueProfile::fromLazyOperand(
                        profiledBlock,
                        LazyOperandValueProfileKey(
                            node->origin.semantic.bytecodeIndex, node->local()));
                }
            }

            if (node->hasHeapPrediction())
                return profiledBlock->valueProfileForBytecodeOffset(node->origin.semantic.bytecodeIndex);

            if (profiledBlock->hasBaselineJITProfiling()) {
                if (ArithProfile* result = profiledBlock->arithProfileForBytecodeOffset(node->origin.semantic.bytecodeIndex))
                    return result;
            }
        }

        switch (node->op()) {
        case BooleanToNumber:
        case Identity:
        case ValueRep:
        case DoubleRep:
        case Int52Rep:
            node = node->child1().node();
            break;
        default:
            node = nullptr;
        }
    }
    
    return MethodOfGettingAValueProfile();
}

bool Graph::getRegExpPrototypeProperty(JSObject* regExpPrototype, Structure* regExpPrototypeStructure, UniquedStringImpl* uid, JSValue& returnJSValue)
{
    unsigned attributesUnused;
    PropertyOffset offset = regExpPrototypeStructure->getConcurrently(uid, attributesUnused);
    if (!isValidOffset(offset))
        return false;

    JSValue value = tryGetConstantProperty(regExpPrototype, regExpPrototypeStructure, offset);
    if (!value)
        return false;

    // We only care about functions and getters at this point. If you want to access other properties
    // you'll have to add code for those types.
    JSFunction* function = jsDynamicCast<JSFunction*>(m_vm, value);
    if (!function) {
        GetterSetter* getterSetter = jsDynamicCast<GetterSetter*>(m_vm, value);

        if (!getterSetter)
            return false;

        returnJSValue = JSValue(getterSetter);
        return true;
    }

    returnJSValue = value;
    return true;
}

bool Graph::isStringPrototypeMethodSane(JSGlobalObject* globalObject, UniquedStringImpl* uid)
{
    ObjectPropertyConditionSet conditions = generateConditionsForPrototypeEquivalenceConcurrently(m_vm, globalObject, globalObject->stringObjectStructure(), globalObject->stringPrototype(), uid);

    if (!conditions.isValid())
        return false;

    ObjectPropertyCondition equivalenceCondition = conditions.slotBaseCondition();
    RELEASE_ASSERT(equivalenceCondition.hasRequiredValue());
    JSFunction* function = jsDynamicCast<JSFunction*>(m_vm, equivalenceCondition.condition().requiredValue());
    if (!function)
        return false;

    if (function->executable()->intrinsicFor(CodeForCall) != StringPrototypeValueOfIntrinsic)
        return false;
    
    return watchConditions(conditions);
}


bool Graph::canOptimizeStringObjectAccess(const CodeOrigin& codeOrigin)
{
    if (hasExitSite(codeOrigin, NotStringObject))
        return false;

    JSGlobalObject* globalObject = globalObjectFor(codeOrigin);
    Structure* stringObjectStructure = globalObjectFor(codeOrigin)->stringObjectStructure();
    registerStructure(stringObjectStructure);
    ASSERT(stringObjectStructure->storedPrototype().isObject());
    ASSERT(stringObjectStructure->storedPrototype().asCell()->classInfo(*stringObjectStructure->storedPrototype().asCell()->vm()) == StringPrototype::info());

    if (!watchConditions(generateConditionsForPropertyMissConcurrently(m_vm, globalObject, stringObjectStructure, m_vm.propertyNames->toPrimitiveSymbol.impl())))
        return false;

    // We're being conservative here. We want DFG's ToString on StringObject to be
    // used in both numeric contexts (that would call valueOf()) and string contexts
    // (that would call toString()). We don't want the DFG to have to distinguish
    // between the two, just because that seems like it would get confusing. So we
    // just require both methods to be sane.
    if (!isStringPrototypeMethodSane(globalObject, m_vm.propertyNames->valueOf.impl()))
        return false;
    return isStringPrototypeMethodSane(globalObject, m_vm.propertyNames->toString.impl());
}

bool Graph::willCatchExceptionInMachineFrame(CodeOrigin codeOrigin, CodeOrigin& opCatchOriginOut, HandlerInfo*& catchHandlerOut)
{
    if (!m_hasExceptionHandlers)
        return false;

    unsigned bytecodeIndexToCheck = codeOrigin.bytecodeIndex;
    while (1) {
        InlineCallFrame* inlineCallFrame = codeOrigin.inlineCallFrame;
        CodeBlock* codeBlock = baselineCodeBlockFor(inlineCallFrame);
        if (HandlerInfo* handler = codeBlock->handlerForBytecodeOffset(bytecodeIndexToCheck)) {
            opCatchOriginOut = CodeOrigin(handler->target, inlineCallFrame);
            catchHandlerOut = handler;
            return true;
        }

        if (!inlineCallFrame)
            return false;

        bytecodeIndexToCheck = inlineCallFrame->directCaller.bytecodeIndex;
        codeOrigin = codeOrigin.inlineCallFrame->directCaller;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
