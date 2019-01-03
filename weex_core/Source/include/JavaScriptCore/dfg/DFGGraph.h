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

#include "AssemblyHelpers.h"
#include "BytecodeLivenessAnalysisInlines.h"
#include "CodeBlock.h"
#include "DFGArgumentPosition.h"
#include "DFGBasicBlock.h"
#include "DFGFrozenValue.h"
#include "DFGLongLivedState.h"
#include "DFGNode.h"
#include "DFGNodeAllocator.h"
#include "DFGPlan.h"
#include "DFGPropertyTypeKey.h"
#include "DFGScannable.h"
#include "FullBytecodeLiveness.h"
#include "MethodOfGettingAValueProfile.h"
#include <unordered_map>
#include <wtf/BitVector.h>
#include <wtf/HashMap.h>
#include <wtf/Vector.h>
#include <wtf/StdLibExtras.h>

namespace JSC {

class CodeBlock;
class ExecState;

namespace DFG {

class BackwardsCFG;
class BackwardsDominators;
class CFG;
class ControlEquivalenceAnalysis;
class Dominators;
class FlowIndexing;
class NaturalLoops;
class PrePostNumbering;

template<typename> class FlowMap;

#define DFG_NODE_DO_TO_CHILDREN(graph, node, thingToDo) do {            \
        Node* _node = (node);                                           \
        if (_node->flags() & NodeHasVarArgs) {                          \
            for (unsigned _childIdx = _node->firstChild();              \
                _childIdx < _node->firstChild() + _node->numChildren(); \
                _childIdx++) {                                          \
                if (!!(graph).m_varArgChildren[_childIdx])              \
                    thingToDo(_node, (graph).m_varArgChildren[_childIdx]); \
            }                                                           \
        } else {                                                        \
            if (!_node->child1()) {                                     \
                ASSERT(                                                 \
                    !_node->child2()                                    \
                    && !_node->child3());                               \
                break;                                                  \
            }                                                           \
            thingToDo(_node, _node->child1());                          \
                                                                        \
            if (!_node->child2()) {                                     \
                ASSERT(!_node->child3());                               \
                break;                                                  \
            }                                                           \
            thingToDo(_node, _node->child2());                          \
                                                                        \
            if (!_node->child3())                                       \
                break;                                                  \
            thingToDo(_node, _node->child3());                          \
        }                                                               \
    } while (false)

#define DFG_ASSERT(graph, node, assertion) do {                         \
        if (!!(assertion))                                              \
            break;                                                      \
        (graph).handleAssertionFailure(                                 \
            (node), __FILE__, __LINE__, WTF_PRETTY_FUNCTION, #assertion); \
    } while (false)

#define DFG_CRASH(graph, node, reason) do {                             \
        (graph).handleAssertionFailure(                                 \
            (node), __FILE__, __LINE__, WTF_PRETTY_FUNCTION, (reason)); \
    } while (false)

struct InlineVariableData {
    InlineCallFrame* inlineCallFrame;
    unsigned argumentPositionStart;
    VariableAccessData* calleeVariable;
};

enum AddSpeculationMode {
    DontSpeculateInt32,
    SpeculateInt32AndTruncateConstants,
    SpeculateInt32
};

//
// === Graph ===
//
// The order may be significant for nodes with side-effects (property accesses, value conversions).
// Nodes that are 'dead' remain in the vector with refCount 0.
class Graph : public virtual Scannable {
public:
    Graph(VM&, Plan&, LongLivedState&);
    ~Graph();
    
    void changeChild(Edge& edge, Node* newNode)
    {
        edge.setNode(newNode);
    }
    
    void changeEdge(Edge& edge, Edge newEdge)
    {
        edge = newEdge;
    }
    
    void compareAndSwap(Edge& edge, Node* oldNode, Node* newNode)
    {
        if (edge.node() != oldNode)
            return;
        changeChild(edge, newNode);
    }
    
    void compareAndSwap(Edge& edge, Edge oldEdge, Edge newEdge)
    {
        if (edge != oldEdge)
            return;
        changeEdge(edge, newEdge);
    }
    
    void performSubstitution(Node* node)
    {
        if (node->flags() & NodeHasVarArgs) {
            for (unsigned childIdx = node->firstChild(); childIdx < node->firstChild() + node->numChildren(); childIdx++)
                performSubstitutionForEdge(m_varArgChildren[childIdx]);
        } else {
            performSubstitutionForEdge(node->child1());
            performSubstitutionForEdge(node->child2());
            performSubstitutionForEdge(node->child3());
        }
    }
    
    void performSubstitutionForEdge(Edge& child)
    {
        // Check if this operand is actually unused.
        if (!child)
            return;
        
        // Check if there is any replacement.
        Node* replacement = child->replacement();
        if (!replacement)
            return;
        
        child.setNode(replacement);
        
        // There is definitely a replacement. Assert that the replacement does not
        // have a replacement.
        ASSERT(!child->replacement());
    }
    
    template<typename... Params>
    Node* addNode(Params... params)
    {
        Node* node = new (m_allocator) Node(params...);
        addNodeToMapByIndex(node);
        return node;
    }
    template<typename... Params>
    Node* addNode(SpeculatedType type, Params... params)
    {
        Node* node = new (m_allocator) Node(params...);
        node->predict(type);
        addNodeToMapByIndex(node);
        return node;
    }

    void deleteNode(Node*);
    unsigned maxNodeCount() const { return m_nodesByIndex.size(); }
    Node* nodeAt(unsigned index) const { return m_nodesByIndex[index]; }
    void packNodeIndices();

    void dethread();
    
    FrozenValue* freeze(JSValue); // We use weak freezing by default.
    FrozenValue* freezeStrong(JSValue); // Shorthand for freeze(value)->strengthenTo(StrongValue).
    
    void convertToConstant(Node* node, FrozenValue* value);
    void convertToConstant(Node* node, JSValue value);
    void convertToStrongConstant(Node* node, JSValue value);
    
    RegisteredStructure registerStructure(Structure* structure)
    {
        StructureRegistrationResult ignored;
        return registerStructure(structure, ignored);
    }
    RegisteredStructure registerStructure(Structure*, StructureRegistrationResult&);
    void assertIsRegistered(Structure* structure);
    
    // CodeBlock is optional, but may allow additional information to be dumped (e.g. Identifier names).
    void dump(PrintStream& = WTF::dataFile(), DumpContext* = 0);
    
    bool terminalsAreValid();
    
    enum PhiNodeDumpMode { DumpLivePhisOnly, DumpAllPhis };
    void dumpBlockHeader(PrintStream&, const char* prefix, BasicBlock*, PhiNodeDumpMode, DumpContext*);
    void dump(PrintStream&, Edge);
    void dump(PrintStream&, const char* prefix, Node*, DumpContext* = 0);
    static int amountOfNodeWhiteSpace(Node*);
    static void printNodeWhiteSpace(PrintStream&, Node*);

    // Dump the code origin of the given node as a diff from the code origin of the
    // preceding node. Returns true if anything was printed.
    bool dumpCodeOrigin(PrintStream&, const char* prefix, Node*& previousNode, Node* currentNode, DumpContext*);

    AddSpeculationMode addSpeculationMode(Node* add, bool leftShouldSpeculateInt32, bool rightShouldSpeculateInt32, PredictionPass pass)
    {
        ASSERT(add->op() == ValueAdd || add->op() == ArithAdd || add->op() == ArithSub);
        
        RareCaseProfilingSource source = add->sourceFor(pass);
        
        Node* left = add->child1().node();
        Node* right = add->child2().node();
        
        if (left->hasConstant())
            return addImmediateShouldSpeculateInt32(add, rightShouldSpeculateInt32, right, left, source);
        if (right->hasConstant())
            return addImmediateShouldSpeculateInt32(add, leftShouldSpeculateInt32, left, right, source);
        
        return (leftShouldSpeculateInt32 && rightShouldSpeculateInt32 && add->canSpeculateInt32(source)) ? SpeculateInt32 : DontSpeculateInt32;
    }
    
    AddSpeculationMode valueAddSpeculationMode(Node* add, PredictionPass pass)
    {
        return addSpeculationMode(
            add,
            add->child1()->shouldSpeculateInt32OrBooleanExpectingDefined(),
            add->child2()->shouldSpeculateInt32OrBooleanExpectingDefined(),
            pass);
    }
    
    AddSpeculationMode arithAddSpeculationMode(Node* add, PredictionPass pass)
    {
        return addSpeculationMode(
            add,
            add->child1()->shouldSpeculateInt32OrBooleanForArithmetic(),
            add->child2()->shouldSpeculateInt32OrBooleanForArithmetic(),
            pass);
    }
    
    AddSpeculationMode addSpeculationMode(Node* add, PredictionPass pass)
    {
        if (add->op() == ValueAdd)
            return valueAddSpeculationMode(add, pass);
        
        return arithAddSpeculationMode(add, pass);
    }
    
    bool addShouldSpeculateInt32(Node* add, PredictionPass pass)
    {
        return addSpeculationMode(add, pass) != DontSpeculateInt32;
    }
    
    bool addShouldSpeculateAnyInt(Node* add)
    {
        if (!enableInt52())
            return false;
        
        Node* left = add->child1().node();
        Node* right = add->child2().node();

        bool speculation = Node::shouldSpeculateAnyInt(left, right);
        return speculation && !hasExitSite(add, Int52Overflow);
    }
    
    bool binaryArithShouldSpeculateInt32(Node* node, PredictionPass pass)
    {
        Node* left = node->child1().node();
        Node* right = node->child2().node();
        
        return Node::shouldSpeculateInt32OrBooleanForArithmetic(left, right)
            && node->canSpeculateInt32(node->sourceFor(pass));
    }
    
    bool binaryArithShouldSpeculateAnyInt(Node* node, PredictionPass pass)
    {
        if (!enableInt52())
            return false;
        
        Node* left = node->child1().node();
        Node* right = node->child2().node();

        return Node::shouldSpeculateAnyInt(left, right)
            && node->canSpeculateInt52(pass)
            && !hasExitSite(node, Int52Overflow);
    }
    
    bool unaryArithShouldSpeculateInt32(Node* node, PredictionPass pass)
    {
        return node->child1()->shouldSpeculateInt32OrBooleanForArithmetic()
            && node->canSpeculateInt32(pass);
    }
    
    bool unaryArithShouldSpeculateAnyInt(Node* node, PredictionPass pass)
    {
        if (!enableInt52())
            return false;
        return node->child1()->shouldSpeculateAnyInt()
            && node->canSpeculateInt52(pass)
            && !hasExitSite(node, Int52Overflow);
    }

    bool canOptimizeStringObjectAccess(const CodeOrigin&);

    bool getRegExpPrototypeProperty(JSObject* regExpPrototype, Structure* regExpPrototypeStructure, UniquedStringImpl* uid, JSValue& returnJSValue);

    bool roundShouldSpeculateInt32(Node* arithRound, PredictionPass pass)
    {
        ASSERT(arithRound->op() == ArithRound || arithRound->op() == ArithFloor || arithRound->op() == ArithCeil || arithRound->op() == ArithTrunc);
        return arithRound->canSpeculateInt32(pass) && !hasExitSite(arithRound->origin.semantic, Overflow) && !hasExitSite(arithRound->origin.semantic, NegativeZero);
    }
    
    static const char *opName(NodeType);
    
    RegisteredStructureSet* addStructureSet(const StructureSet& structureSet)
    {
        m_structureSets.append();
        RegisteredStructureSet* result = &m_structureSets.last();

        for (Structure* structure : structureSet)
            result->add(registerStructure(structure));

        return result;
    }

    RegisteredStructureSet* addStructureSet(const RegisteredStructureSet& structureSet)
    {
        m_structureSets.append();
        RegisteredStructureSet* result = &m_structureSets.last();

        for (RegisteredStructure structure : structureSet)
            result->add(structure);

        return result;
    }
    
    JSGlobalObject* globalObjectFor(CodeOrigin codeOrigin)
    {
        return m_codeBlock->globalObjectFor(codeOrigin);
    }
    
    JSObject* globalThisObjectFor(CodeOrigin codeOrigin)
    {
        JSGlobalObject* object = globalObjectFor(codeOrigin);
        return jsCast<JSObject*>(object->methodTable()->toThis(object, object->globalExec(), NotStrictMode));
    }
    
    ScriptExecutable* executableFor(InlineCallFrame* inlineCallFrame)
    {
        if (!inlineCallFrame)
            return m_codeBlock->ownerScriptExecutable();
        
        return inlineCallFrame->baselineCodeBlock->ownerScriptExecutable();
    }
    
    ScriptExecutable* executableFor(const CodeOrigin& codeOrigin)
    {
        return executableFor(codeOrigin.inlineCallFrame);
    }
    
    CodeBlock* baselineCodeBlockFor(InlineCallFrame* inlineCallFrame)
    {
        if (!inlineCallFrame)
            return m_profiledBlock;
        return baselineCodeBlockForInlineCallFrame(inlineCallFrame);
    }
    
    CodeBlock* baselineCodeBlockFor(const CodeOrigin& codeOrigin)
    {
        return baselineCodeBlockForOriginAndBaselineCodeBlock(codeOrigin, m_profiledBlock);
    }
    
    bool isStrictModeFor(CodeOrigin codeOrigin)
    {
        if (!codeOrigin.inlineCallFrame)
            return m_codeBlock->isStrictMode();
        return codeOrigin.inlineCallFrame->isStrictMode();
    }
    
    ECMAMode ecmaModeFor(CodeOrigin codeOrigin)
    {
        return isStrictModeFor(codeOrigin) ? StrictMode : NotStrictMode;
    }
    
    bool masqueradesAsUndefinedWatchpointIsStillValid(const CodeOrigin& codeOrigin)
    {
        return globalObjectFor(codeOrigin)->masqueradesAsUndefinedWatchpoint()->isStillValid();
    }
    
    bool hasGlobalExitSite(const CodeOrigin& codeOrigin, ExitKind exitKind)
    {
        return baselineCodeBlockFor(codeOrigin)->hasExitSite(FrequentExitSite(exitKind));
    }
    
    bool hasExitSite(const CodeOrigin& codeOrigin, ExitKind exitKind)
    {
        return baselineCodeBlockFor(codeOrigin)->hasExitSite(FrequentExitSite(codeOrigin.bytecodeIndex, exitKind));
    }
    
    bool hasExitSite(Node* node, ExitKind exitKind)
    {
        return hasExitSite(node->origin.semantic, exitKind);
    }
    
    MethodOfGettingAValueProfile methodOfGettingAValueProfileFor(Node* currentNode, Node* operandNode);
    
    BlockIndex numBlocks() const { return m_blocks.size(); }
    BasicBlock* block(BlockIndex blockIndex) const { return m_blocks[blockIndex].get(); }
    BasicBlock* lastBlock() const { return block(numBlocks() - 1); }

    void appendBlock(Ref<BasicBlock>&& basicBlock)
    {
        basicBlock->index = m_blocks.size();
        m_blocks.append(WTFMove(basicBlock));
    }
    
    void killBlock(BlockIndex blockIndex)
    {
        m_blocks[blockIndex] = nullptr;
    }
    
    void killBlock(BasicBlock* basicBlock)
    {
        killBlock(basicBlock->index);
    }
    
    void killBlockAndItsContents(BasicBlock*);
    
    void killUnreachableBlocks();
    
    void determineReachability();
    void resetReachability();
    
    void computeRefCounts();
    
    unsigned varArgNumChildren(Node* node)
    {
        ASSERT(node->flags() & NodeHasVarArgs);
        return node->numChildren();
    }
    
    unsigned numChildren(Node* node)
    {
        if (node->flags() & NodeHasVarArgs)
            return varArgNumChildren(node);
        return AdjacencyList::Size;
    }
    
    Edge& varArgChild(Node* node, unsigned index)
    {
        ASSERT(node->flags() & NodeHasVarArgs);
        return m_varArgChildren[node->firstChild() + index];
    }
    
    Edge& child(Node* node, unsigned index)
    {
        if (node->flags() & NodeHasVarArgs)
            return varArgChild(node, index);
        return node->children.child(index);
    }
    
    void voteNode(Node* node, unsigned ballot, float weight = 1)
    {
        switch (node->op()) {
        case ValueToInt32:
        case UInt32ToNumber:
            node = node->child1().node();
            break;
        default:
            break;
        }
        
        if (node->op() == GetLocal)
            node->variableAccessData()->vote(ballot, weight);
    }
    
    void voteNode(Edge edge, unsigned ballot, float weight = 1)
    {
        voteNode(edge.node(), ballot, weight);
    }
    
    void voteChildren(Node* node, unsigned ballot, float weight = 1)
    {
        if (node->flags() & NodeHasVarArgs) {
            for (unsigned childIdx = node->firstChild();
                childIdx < node->firstChild() + node->numChildren();
                childIdx++) {
                if (!!m_varArgChildren[childIdx])
                    voteNode(m_varArgChildren[childIdx], ballot, weight);
            }
            return;
        }
        
        if (!node->child1())
            return;
        voteNode(node->child1(), ballot, weight);
        if (!node->child2())
            return;
        voteNode(node->child2(), ballot, weight);
        if (!node->child3())
            return;
        voteNode(node->child3(), ballot, weight);
    }
    
    template<typename T> // T = Node* or Edge
    void substitute(BasicBlock& block, unsigned startIndexInBlock, T oldThing, T newThing)
    {
        for (unsigned indexInBlock = startIndexInBlock; indexInBlock < block.size(); ++indexInBlock) {
            Node* node = block[indexInBlock];
            if (node->flags() & NodeHasVarArgs) {
                for (unsigned childIdx = node->firstChild(); childIdx < node->firstChild() + node->numChildren(); ++childIdx) {
                    if (!!m_varArgChildren[childIdx])
                        compareAndSwap(m_varArgChildren[childIdx], oldThing, newThing);
                }
                continue;
            }
            if (!node->child1())
                continue;
            compareAndSwap(node->children.child1(), oldThing, newThing);
            if (!node->child2())
                continue;
            compareAndSwap(node->children.child2(), oldThing, newThing);
            if (!node->child3())
                continue;
            compareAndSwap(node->children.child3(), oldThing, newThing);
        }
    }
    
    // Use this if you introduce a new GetLocal and you know that you introduced it *before*
    // any GetLocals in the basic block.
    // FIXME: it may be appropriate, in the future, to generalize this to handle GetLocals
    // introduced anywhere in the basic block.
    void substituteGetLocal(BasicBlock& block, unsigned startIndexInBlock, VariableAccessData* variableAccessData, Node* newGetLocal);
    
    void invalidateCFG();
    void invalidateNodeLiveness();
    
    void clearFlagsOnAllNodes(NodeFlags);
    
    void clearReplacements();
    void clearEpochs();
    void initializeNodeOwners();
    
    BlockList blocksInPreOrder();
    BlockList blocksInPostOrder();
    
    class NaturalBlockIterable {
    public:
        NaturalBlockIterable()
            : m_graph(nullptr)
        {
        }
        
        NaturalBlockIterable(Graph& graph)
            : m_graph(&graph)
        {
        }
        
        class iterator {
        public:
            iterator()
                : m_graph(nullptr)
                , m_index(0)
            {
            }
            
            iterator(Graph& graph, BlockIndex index)
                : m_graph(&graph)
                , m_index(findNext(index))
            {
            }
            
            BasicBlock *operator*()
            {
                return m_graph->block(m_index);
            }
            
            iterator& operator++()
            {
                m_index = findNext(m_index + 1);
                return *this;
            }
            
            bool operator==(const iterator& other) const
            {
                return m_index == other.m_index;
            }
            
            bool operator!=(const iterator& other) const
            {
                return !(*this == other);
            }
            
        private:
            BlockIndex findNext(BlockIndex index)
            {
                while (index < m_graph->numBlocks() && !m_graph->block(index))
                    index++;
                return index;
            }
            
            Graph* m_graph;
            BlockIndex m_index;
        };
        
        iterator begin()
        {
            return iterator(*m_graph, 0);
        }
        
        iterator end()
        {
            return iterator(*m_graph, m_graph->numBlocks());
        }
        
    private:
        Graph* m_graph;
    };
    
    NaturalBlockIterable blocksInNaturalOrder()
    {
        return NaturalBlockIterable(*this);
    }
    
    template<typename ChildFunctor>
    void doToChildrenWithNode(Node* node, const ChildFunctor& functor)
    {
        DFG_NODE_DO_TO_CHILDREN(*this, node, functor);
    }
    
    template<typename ChildFunctor>
    void doToChildren(Node* node, const ChildFunctor& functor)
    {
        doToChildrenWithNode(
            node,
            [&functor] (Node*, Edge& edge) {
                functor(edge);
            });
    }
    
    bool uses(Node* node, Node* child)
    {
        bool result = false;
        doToChildren(node, [&] (Edge edge) { result |= edge == child; });
        return result;
    }

    bool isWatchingHavingABadTimeWatchpoint(Node* node)
    {
        JSGlobalObject* globalObject = globalObjectFor(node->origin.semantic);
        return watchpoints().isWatched(globalObject->havingABadTimeWatchpoint());
    }

    bool isWatchingArrayIteratorProtocolWatchpoint(Node* node)
    {
        JSGlobalObject* globalObject = globalObjectFor(node->origin.semantic);
        InlineWatchpointSet& set = globalObject->arrayIteratorProtocolWatchpoint();
        if (watchpoints().isWatched(set))
            return true;

        if (set.isStillValid()) {
            // Since the global object owns this watchpoint, we make ourselves have a weak pointer to it.
            // If the global object got deallocated, it wouldn't fire the watchpoint. It's unlikely the
            // global object would get deallocated without this code ever getting thrown away, however,
            // it's more sound logically to depend on the global object lifetime weakly.
            freeze(globalObject);
            watchpoints().addLazily(set);
            return true;
        }

        return false;
    }
    
    Profiler::Compilation* compilation() { return m_plan.compilation.get(); }
    
    DesiredIdentifiers& identifiers() { return m_plan.identifiers; }
    DesiredWatchpoints& watchpoints() { return m_plan.watchpoints; }
    
    // Returns false if the key is already invalid or unwatchable. If this is a Presence condition,
    // this also makes it cheap to query if the condition holds. Also makes sure that the GC knows
    // what's going on.
    bool watchCondition(const ObjectPropertyCondition&);
    bool watchConditions(const ObjectPropertyConditionSet&);

    // Checks if it's known that loading from the given object at the given offset is fine. This is
    // computed by tracking which conditions we track with watchCondition().
    bool isSafeToLoad(JSObject* base, PropertyOffset);

    void registerInferredType(const InferredType::Descriptor& type)
    {
        if (type.structure())
            registerStructure(type.structure());
    }

    // Tells us what inferred type we are able to prove the property to have now and in the future.
    InferredType::Descriptor inferredTypeFor(const PropertyTypeKey&);
    InferredType::Descriptor inferredTypeForProperty(Structure* structure, UniquedStringImpl* uid)
    {
        return inferredTypeFor(PropertyTypeKey(structure, uid));
    }

    AbstractValue inferredValueForProperty(
        const RegisteredStructureSet& base, UniquedStringImpl* uid, StructureClobberState = StructuresAreWatched);

    // This uses either constant property inference or property type inference to derive a good abstract
    // value for some property accessed with the given abstract value base.
    AbstractValue inferredValueForProperty(
        const AbstractValue& base, UniquedStringImpl* uid, PropertyOffset, StructureClobberState);
    
    FullBytecodeLiveness& livenessFor(CodeBlock*);
    FullBytecodeLiveness& livenessFor(InlineCallFrame*);
    
    // Quickly query if a single local is live at the given point. This is faster than calling
    // forAllLiveInBytecode() if you will only query one local. But, if you want to know all of the
    // locals live, then calling this for each local is much slower than forAllLiveInBytecode().
    bool isLiveInBytecode(VirtualRegister, CodeOrigin);
    
    // Quickly get all of the non-argument locals live at the given point. This doesn't give you
    // any arguments because those are all presumed live. You can call forAllLiveInBytecode() to
    // also get the arguments. This is much faster than calling isLiveInBytecode() for each local.
    template<typename Functor>
    void forAllLocalsLiveInBytecode(CodeOrigin codeOrigin, const Functor& functor)
    {
        // Support for not redundantly reporting arguments. Necessary because in case of a varargs
        // call, only the callee knows that arguments are live while in the case of a non-varargs
        // call, both callee and caller will see the variables live.
        VirtualRegister exclusionStart;
        VirtualRegister exclusionEnd;

        CodeOrigin* codeOriginPtr = &codeOrigin;
        
        for (;;) {
            InlineCallFrame* inlineCallFrame = codeOriginPtr->inlineCallFrame;
            VirtualRegister stackOffset(inlineCallFrame ? inlineCallFrame->stackOffset : 0);
            
            if (inlineCallFrame) {
                if (inlineCallFrame->isClosureCall)
                    functor(stackOffset + CallFrameSlot::callee);
                if (inlineCallFrame->isVarargs())
                    functor(stackOffset + CallFrameSlot::argumentCount);
            }
            
            CodeBlock* codeBlock = baselineCodeBlockFor(inlineCallFrame);
            FullBytecodeLiveness& fullLiveness = livenessFor(codeBlock);
            const FastBitVector& liveness = fullLiveness.getLiveness(codeOriginPtr->bytecodeIndex);
            for (unsigned relativeLocal = codeBlock->m_numCalleeLocals; relativeLocal--;) {
                VirtualRegister reg = stackOffset + virtualRegisterForLocal(relativeLocal);
                
                // Don't report if our callee already reported.
                if (reg >= exclusionStart && reg < exclusionEnd)
                    continue;
                
                if (liveness[relativeLocal])
                    functor(reg);
            }
            
            if (!inlineCallFrame)
                break;

            // Arguments are always live. This would be redundant if it wasn't for our
            // op_call_varargs inlining. See the comment above.
            exclusionStart = stackOffset + CallFrame::argumentOffsetIncludingThis(0);
            exclusionEnd = stackOffset + CallFrame::argumentOffsetIncludingThis(inlineCallFrame->arguments.size());
            
            // We will always have a "this" argument and exclusionStart should be a smaller stack
            // offset than exclusionEnd.
            ASSERT(exclusionStart < exclusionEnd);

            for (VirtualRegister reg = exclusionStart; reg < exclusionEnd; reg += 1)
                functor(reg);
            
            codeOriginPtr = inlineCallFrame->getCallerSkippingTailCalls();

            // The first inline call frame could be an inline tail call
            if (!codeOriginPtr)
                break;
        }
    }
    
    // Get a BitVector of all of the non-argument locals live right now. This is mostly useful if
    // you want to compare two sets of live locals from two different CodeOrigins.
    BitVector localsLiveInBytecode(CodeOrigin);
    
    // Tells you all of the arguments and locals live at the given CodeOrigin. This is a small
    // extension to forAllLocalsLiveInBytecode(), since all arguments are always presumed live.
    template<typename Functor>
    void forAllLiveInBytecode(CodeOrigin codeOrigin, const Functor& functor)
    {
        forAllLocalsLiveInBytecode(codeOrigin, functor);
        
        // Report all arguments as being live.
        for (unsigned argument = block(0)->variablesAtHead.numberOfArguments(); argument--;)
            functor(virtualRegisterForArgument(argument));
    }
    
    BytecodeKills& killsFor(CodeBlock*);
    BytecodeKills& killsFor(InlineCallFrame*);
    
    static unsigned parameterSlotsForArgCount(unsigned);
    
    unsigned frameRegisterCount();
    unsigned stackPointerOffset();
    unsigned requiredRegisterCountForExit();
    unsigned requiredRegisterCountForExecutionAndExit();
    
    JSValue tryGetConstantProperty(JSValue base, const RegisteredStructureSet&, PropertyOffset);
    JSValue tryGetConstantProperty(JSValue base, Structure*, PropertyOffset);
    JSValue tryGetConstantProperty(JSValue base, const StructureAbstractValue&, PropertyOffset);
    JSValue tryGetConstantProperty(const AbstractValue&, PropertyOffset);
    
    JSValue tryGetConstantClosureVar(JSValue base, ScopeOffset);
    JSValue tryGetConstantClosureVar(const AbstractValue&, ScopeOffset);
    JSValue tryGetConstantClosureVar(Node*, ScopeOffset);
    
    JSArrayBufferView* tryGetFoldableView(JSValue);
    JSArrayBufferView* tryGetFoldableView(JSValue, ArrayMode arrayMode);
    
    void registerFrozenValues();
    
    void visitChildren(SlotVisitor&) override;
    
    NO_RETURN_DUE_TO_CRASH void handleAssertionFailure(
        std::nullptr_t, const char* file, int line, const char* function,
        const char* assertion);
    NO_RETURN_DUE_TO_CRASH void handleAssertionFailure(
        Node*, const char* file, int line, const char* function,
        const char* assertion);
    NO_RETURN_DUE_TO_CRASH void handleAssertionFailure(
        BasicBlock*, const char* file, int line, const char* function,
        const char* assertion);

    bool hasDebuggerEnabled() const { return m_hasDebuggerEnabled; }

    Dominators& ensureDominators();
    PrePostNumbering& ensurePrePostNumbering();
    NaturalLoops& ensureNaturalLoops();
    BackwardsCFG& ensureBackwardsCFG();
    BackwardsDominators& ensureBackwardsDominators();
    ControlEquivalenceAnalysis& ensureControlEquivalenceAnalysis();

    // This function only makes sense to call after bytecode parsing
    // because it queries the m_hasExceptionHandlers boolean whose value
    // is only fully determined after bytcode parsing.
    bool willCatchExceptionInMachineFrame(CodeOrigin, CodeOrigin& opCatchOriginOut, HandlerInfo*& catchHandlerOut);
    
    bool needsScopeRegister() const { return m_hasDebuggerEnabled || m_codeBlock->usesEval(); }
    bool needsFlushedThis() const { return m_codeBlock->usesEval(); }

    VM& m_vm;
    Plan& m_plan;
    CodeBlock* m_codeBlock;
    CodeBlock* m_profiledBlock;
    
    NodeAllocator& m_allocator;

    Vector< RefPtr<BasicBlock> , 8> m_blocks;
    Vector<Edge, 16> m_varArgChildren;

    HashMap<EncodedJSValue, FrozenValue*, EncodedJSValueHash, EncodedJSValueHashTraits> m_frozenValueMap;
    Bag<FrozenValue> m_frozenValues;

    Vector<uint32_t> m_uint32ValuesInUse;
    
    Bag<StorageAccessData> m_storageAccessData;
    
    // In CPS, this is all of the SetArgument nodes for the arguments in the machine code block
    // that survived DCE. All of them except maybe "this" will survive DCE, because of the Flush
    // nodes.
    //
    // In SSA, this is all of the GetStack nodes for the arguments in the machine code block that
    // may have some speculation in the prologue and survived DCE. Note that to get the speculation
    // for an argument in SSA, you must use m_argumentFormats, since we still have to speculate
    // even if the argument got killed. For example:
    //
    //     function foo(x) {
    //        var tmp = x + 1;
    //     }
    //
    // Assume that x is always int during profiling. The ArithAdd for "x + 1" will be dead and will
    // have a proven check for the edge to "x". So, we will not insert a Check node and we will
    // kill the GetStack for "x". But, we must do the int check in the progolue, because that's the
    // thing we used to allow DCE of ArithAdd. Otherwise the add could be impure:
    //
    //     var o = {
    //         valueOf: function() { do side effects; }
    //     };
    //     foo(o);
    //
    // If we DCE the ArithAdd and we remove the int check on x, then this won't do the side
    // effects.
    Vector<Node*, 8> m_arguments;
    
    // In CPS, this is meaningless. In SSA, this is the argument speculation that we've locked in.
    Vector<FlushFormat> m_argumentFormats;
    
    SegmentedVector<VariableAccessData, 16> m_variableAccessData;
    SegmentedVector<ArgumentPosition, 8> m_argumentPositions;
    Bag<Transition> m_transitions;
    SegmentedVector<NewArrayBufferData, 4> m_newArrayBufferData;
    Bag<BranchData> m_branchData;
    Bag<SwitchData> m_switchData;
    Bag<MultiGetByOffsetData> m_multiGetByOffsetData;
    Bag<MultiPutByOffsetData> m_multiPutByOffsetData;
    Bag<ObjectMaterializationData> m_objectMaterializationData;
    Bag<CallVarargsData> m_callVarargsData;
    Bag<LoadVarargsData> m_loadVarargsData;
    Bag<StackAccessData> m_stackAccessData;
    Bag<LazyJSValue> m_lazyJSValues;
    Bag<CallDOMGetterData> m_callDOMGetterData;
    Bag<BitVector> m_bitVectors;
    Vector<InlineVariableData, 4> m_inlineVariableData;
    HashMap<CodeBlock*, std::unique_ptr<FullBytecodeLiveness>> m_bytecodeLiveness;
    HashMap<CodeBlock*, std::unique_ptr<BytecodeKills>> m_bytecodeKills;
    HashSet<std::pair<JSObject*, PropertyOffset>> m_safeToLoad;
    HashMap<PropertyTypeKey, InferredType::Descriptor> m_inferredTypes;
    Vector<RefPtr<DOMJIT::Patchpoint>> m_domJITPatchpoints;
    std::unique_ptr<Dominators> m_dominators;
    std::unique_ptr<PrePostNumbering> m_prePostNumbering;
    std::unique_ptr<NaturalLoops> m_naturalLoops;
    std::unique_ptr<CFG> m_cfg;
    std::unique_ptr<BackwardsCFG> m_backwardsCFG;
    std::unique_ptr<BackwardsDominators> m_backwardsDominators;
    std::unique_ptr<ControlEquivalenceAnalysis> m_controlEquivalenceAnalysis;
    unsigned m_localVars;
    unsigned m_nextMachineLocal;
    unsigned m_parameterSlots;
    
    HashSet<String> m_localStrings;
    HashMap<const StringImpl*, String> m_copiedStrings;

#if USE(JSVALUE32_64)
    std::unordered_map<int64_t, double*> m_doubleConstantsMap;
    std::unique_ptr<Bag<double>> m_doubleConstants;
#endif
    
    OptimizationFixpointState m_fixpointState;
    StructureRegistrationState m_structureRegistrationState;
    GraphForm m_form;
    UnificationState m_unificationState;
    PlanStage m_planStage { PlanStage::Initial };
    RefCountState m_refCountState;
    bool m_hasDebuggerEnabled;
    bool m_hasExceptionHandlers { false };
    std::unique_ptr<FlowIndexing> m_indexingCache;
    std::unique_ptr<FlowMap<AbstractValue>> m_abstractValuesCache;

    RegisteredStructure stringStructure;
    RegisteredStructure symbolStructure;

private:
    void addNodeToMapByIndex(Node*);

    bool isStringPrototypeMethodSane(JSGlobalObject*, UniquedStringImpl*);

    void handleSuccessor(Vector<BasicBlock*, 16>& worklist, BasicBlock*, BasicBlock* successor);
    
    AddSpeculationMode addImmediateShouldSpeculateInt32(Node* add, bool variableShouldSpeculateInt32, Node* operand, Node*immediate, RareCaseProfilingSource source)
    {
        ASSERT(immediate->hasConstant());
        
        JSValue immediateValue = immediate->asJSValue();
        if (!immediateValue.isNumber() && !immediateValue.isBoolean())
            return DontSpeculateInt32;
        
        if (!variableShouldSpeculateInt32)
            return DontSpeculateInt32;

        // Integer constants can be typed Double if they are written like a double in the source code (e.g. 42.0).
        // In that case, we stay conservative unless the other operand was explicitly typed as integer.
        NodeFlags operandResultType = operand->result();
        if (operandResultType != NodeResultInt32 && immediateValue.isDouble())
            return DontSpeculateInt32;
        
        if (immediateValue.isBoolean() || jsNumber(immediateValue.asNumber()).isInt32())
            return add->canSpeculateInt32(source) ? SpeculateInt32 : DontSpeculateInt32;
        
        double doubleImmediate = immediateValue.asDouble();
        const double twoToThe48 = 281474976710656.0;
        if (doubleImmediate < -twoToThe48 || doubleImmediate > twoToThe48)
            return DontSpeculateInt32;
        
        return bytecodeCanTruncateInteger(add->arithNodeFlags()) ? SpeculateInt32AndTruncateConstants : DontSpeculateInt32;
    }

    Vector<Node*, 0, UnsafeVectorOverflow> m_nodesByIndex;
    Vector<unsigned, 0, UnsafeVectorOverflow> m_nodeIndexFreeList;
    SegmentedVector<RegisteredStructureSet, 16> m_structureSets;
};

} } // namespace JSC::DFG

#endif
