/*
 * Copyright (C) 2011-2017 Apple Inc. All rights reserved.
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
#include "DFGByteCodeParser.h"

#if ENABLE(DFG_JIT)

#include "ArithProfile.h"
#include "Trace.h"
#include "ArrayConstructor.h"
#include "BasicBlockLocation.h"
#include "CallLinkStatus.h"
#include "CodeBlock.h"
#include "CodeBlockWithJITType.h"
#include "DFGAbstractHeap.h"
#include "DFGArrayMode.h"
#include "DFGCapabilities.h"
#include "DFGClobberize.h"
#include "DFGClobbersExitState.h"
#include "DFGGraph.h"
#include "DFGJITCode.h"
#include "FunctionCodeBlock.h"
#include "GetByIdStatus.h"
#include "Heap.h"
#include "JSCInlines.h"
#include "JSModuleEnvironment.h"
#include "JSModuleNamespaceObject.h"
#include "NumberConstructor.h"
#include "ObjectConstructor.h"
#include "PreciseJumpTargets.h"
#include "PutByIdFlags.h"
#include "PutByIdStatus.h"
#include "RegExpPrototype.h"
#include "StackAlignment.h"
#include "StringConstructor.h"
#include "StructureStubInfo.h"
#include "Watchdog.h"
#include <wtf/CommaPrinter.h>
#include <wtf/HashMap.h>
#include <wtf/MathExtras.h>
#include <wtf/StdLibExtras.h>

namespace JSC { namespace DFG {

static const bool verbose = false;

class ConstantBufferKey {
public:
    ConstantBufferKey()
        : m_codeBlock(0)
        , m_index(0)
    {
    }
    
    ConstantBufferKey(WTF::HashTableDeletedValueType)
        : m_codeBlock(0)
        , m_index(1)
    {
    }
    
    ConstantBufferKey(CodeBlock* codeBlock, unsigned index)
        : m_codeBlock(codeBlock)
        , m_index(index)
    {
    }
    
    bool operator==(const ConstantBufferKey& other) const
    {
        return m_codeBlock == other.m_codeBlock
            && m_index == other.m_index;
    }
    
    unsigned hash() const
    {
        return WTF::PtrHash<CodeBlock*>::hash(m_codeBlock) ^ m_index;
    }
    
    bool isHashTableDeletedValue() const
    {
        return !m_codeBlock && m_index;
    }
    
    CodeBlock* codeBlock() const { return m_codeBlock; }
    unsigned index() const { return m_index; }
    
private:
    CodeBlock* m_codeBlock;
    unsigned m_index;
};

struct ConstantBufferKeyHash {
    static unsigned hash(const ConstantBufferKey& key) { return key.hash(); }
    static bool equal(const ConstantBufferKey& a, const ConstantBufferKey& b)
    {
        return a == b;
    }
    
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::DFG

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::DFG::ConstantBufferKey> {
    typedef JSC::DFG::ConstantBufferKeyHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::DFG::ConstantBufferKey> : SimpleClassHashTraits<JSC::DFG::ConstantBufferKey> { };

} // namespace WTF

namespace JSC { namespace DFG {

// === ByteCodeParser ===
//
// This class is used to compile the dataflow graph from a CodeBlock.
class ByteCodeParser {
public:
    ByteCodeParser(Graph& graph)
        : m_vm(&graph.m_vm)
        , m_codeBlock(graph.m_codeBlock)
        , m_profiledBlock(graph.m_profiledBlock)
        , m_graph(graph)
        , m_currentBlock(0)
        , m_currentIndex(0)
        , m_constantUndefined(graph.freeze(jsUndefined()))
        , m_constantNull(graph.freeze(jsNull()))
        , m_constantNaN(graph.freeze(jsNumber(PNaN)))
        , m_constantOne(graph.freeze(jsNumber(1)))
        , m_numArguments(m_codeBlock->numParameters())
        , m_numLocals(m_codeBlock->m_numCalleeLocals)
        , m_parameterSlots(0)
        , m_numPassedVarArgs(0)
        , m_inlineStackTop(0)
        , m_currentInstruction(0)
        , m_hasDebuggerEnabled(graph.hasDebuggerEnabled())
    {
        ASSERT(m_profiledBlock);
    }
    
    // Parse a full CodeBlock of bytecode.
    bool parse();
    
private:
    struct InlineStackEntry;

    // Just parse from m_currentIndex to the end of the current CodeBlock.
    void parseCodeBlock();
    
    void ensureLocals(unsigned newNumLocals)
    {
        if (newNumLocals <= m_numLocals)
            return;
        m_numLocals = newNumLocals;
        for (size_t i = 0; i < m_graph.numBlocks(); ++i)
            m_graph.block(i)->ensureLocals(newNumLocals);
    }

    // Helper for min and max.
    template<typename ChecksFunctor>
    bool handleMinMax(int resultOperand, NodeType op, int registerOffset, int argumentCountIncludingThis, const ChecksFunctor& insertChecks);

    void refineStatically(CallLinkStatus&, Node* callTarget);
    // Handle calls. This resolves issues surrounding inlining and intrinsics.
    enum Terminality { Terminal, NonTerminal };
    Terminality handleCall(
        int result, NodeType op, InlineCallFrame::Kind, unsigned instructionSize,
        Node* callTarget, int argCount, int registerOffset, CallLinkStatus,
        SpeculatedType prediction);
    Terminality handleCall(
        int result, NodeType op, CallMode, unsigned instructionSize,
        Node* callTarget, int argCount, int registerOffset, CallLinkStatus);
    Terminality handleCall(int result, NodeType op, CallMode, unsigned instructionSize, int callee, int argCount, int registerOffset);
    Terminality handleCall(Instruction* pc, NodeType op, CallMode);
    Terminality handleVarargsCall(Instruction* pc, NodeType op, CallMode);
    void emitFunctionChecks(CallVariant, Node* callTarget, VirtualRegister thisArgumnt);
    void emitArgumentPhantoms(int registerOffset, int argumentCountIncludingThis);
    Node* getArgumentCount();
    unsigned inliningCost(CallVariant, int argumentCountIncludingThis, CallMode); // Return UINT_MAX if it's not an inlining candidate. By convention, intrinsics have a cost of 1.
    // Handle inlining. Return true if it succeeded, false if we need to plant a call.
    bool handleInlining(Node* callTargetNode, int resultOperand, const CallLinkStatus&, int registerOffset, VirtualRegister thisArgument, VirtualRegister argumentsArgument, unsigned argumentsOffset, int argumentCountIncludingThis, unsigned nextOffset, NodeType callOp, InlineCallFrame::Kind, SpeculatedType prediction);
    enum CallerLinkability { CallerDoesNormalLinking, CallerLinksManually };
    template<typename ChecksFunctor>
    bool attemptToInlineCall(Node* callTargetNode, int resultOperand, CallVariant, int registerOffset, int argumentCountIncludingThis, unsigned nextOffset, InlineCallFrame::Kind, CallerLinkability, SpeculatedType prediction, unsigned& inliningBalance, const ChecksFunctor& insertChecks);
    template<typename ChecksFunctor>
    void inlineCall(Node* callTargetNode, int resultOperand, CallVariant, int registerOffset, int argumentCountIncludingThis, unsigned nextOffset, InlineCallFrame::Kind, CallerLinkability, const ChecksFunctor& insertChecks);
    void cancelLinkingForBlock(InlineStackEntry*, BasicBlock*); // Only works when the given block is the last one to have been added for that inline stack entry.
    // Handle intrinsic functions. Return true if it succeeded, false if we need to plant a call.
    template<typename ChecksFunctor>
    bool handleIntrinsicCall(Node* callee, int resultOperand, Intrinsic, int registerOffset, int argumentCountIncludingThis, SpeculatedType prediction, const ChecksFunctor& insertChecks);
    template<typename ChecksFunctor>
    bool handleDOMJITCall(Node* callee, int resultOperand, const DOMJIT::Signature*, int registerOffset, int argumentCountIncludingThis, SpeculatedType prediction, const ChecksFunctor& insertChecks);
    template<typename ChecksFunctor>
    bool handleIntrinsicGetter(int resultOperand, const GetByIdVariant& intrinsicVariant, Node* thisNode, const ChecksFunctor& insertChecks);
    template<typename ChecksFunctor>
    bool handleTypedArrayConstructor(int resultOperand, InternalFunction*, int registerOffset, int argumentCountIncludingThis, TypedArrayType, const ChecksFunctor& insertChecks);
    template<typename ChecksFunctor>
    bool handleConstantInternalFunction(Node* callTargetNode, int resultOperand, InternalFunction*, int registerOffset, int argumentCountIncludingThis, CodeSpecializationKind, SpeculatedType, const ChecksFunctor& insertChecks);
    Node* handlePutByOffset(Node* base, unsigned identifier, PropertyOffset, const InferredType::Descriptor&, Node* value);
    Node* handleGetByOffset(SpeculatedType, Node* base, unsigned identifierNumber, PropertyOffset, const InferredType::Descriptor&, NodeType = GetByOffset);
    bool handleDOMJITGetter(int resultOperand, const GetByIdVariant&, Node* thisNode, unsigned identifierNumber, SpeculatedType prediction);
    bool handleModuleNamespaceLoad(int resultOperand, SpeculatedType, Node* base, GetByIdStatus);

    // Create a presence ObjectPropertyCondition based on some known offset and structure set. Does not
    // check the validity of the condition, but it may return a null one if it encounters a contradiction.
    ObjectPropertyCondition presenceLike(
        JSObject* knownBase, UniquedStringImpl*, PropertyOffset, const StructureSet&);
    
    // Attempt to watch the presence of a property. It will watch that the property is present in the same
    // way as in all of the structures in the set. It may emit code instead of just setting a watchpoint.
    // Returns true if this all works out.
    bool checkPresenceLike(JSObject* knownBase, UniquedStringImpl*, PropertyOffset, const StructureSet&);
    void checkPresenceLike(Node* base, UniquedStringImpl*, PropertyOffset, const StructureSet&);
    
    // Works with both GetByIdVariant and the setter form of PutByIdVariant.
    template<typename VariantType>
    Node* load(SpeculatedType, Node* base, unsigned identifierNumber, const VariantType&);

    Node* store(Node* base, unsigned identifier, const PutByIdVariant&, Node* value);

    void handleGetById(
        int destinationOperand, SpeculatedType, Node* base, unsigned identifierNumber, GetByIdStatus, AccessType, unsigned instructionSize);
    void emitPutById(
        Node* base, unsigned identifierNumber, Node* value,  const PutByIdStatus&, bool isDirect);
    void handlePutById(
        Node* base, unsigned identifierNumber, Node* value, const PutByIdStatus&,
        bool isDirect);
    
    // Either register a watchpoint or emit a check for this condition. Returns false if the
    // condition no longer holds, and therefore no reasonable check can be emitted.
    bool check(const ObjectPropertyCondition&);
    
    GetByOffsetMethod promoteToConstant(GetByOffsetMethod);
    
    // Either register a watchpoint or emit a check for this condition. It must be a Presence
    // condition. It will attempt to promote a Presence condition to an Equivalence condition.
    // Emits code for the loaded value that the condition guards, and returns a node containing
    // the loaded value. Returns null if the condition no longer holds.
    GetByOffsetMethod planLoad(const ObjectPropertyCondition&);
    Node* load(SpeculatedType, unsigned identifierNumber, const GetByOffsetMethod&, NodeType = GetByOffset);
    Node* load(SpeculatedType, const ObjectPropertyCondition&, NodeType = GetByOffset);
    
    // Calls check() for each condition in the set: that is, it either emits checks or registers
    // watchpoints (or a combination of the two) to make the conditions hold. If any of those
    // conditions are no longer checkable, returns false.
    bool check(const ObjectPropertyConditionSet&);
    
    // Calls check() for those conditions that aren't the slot base, and calls load() for the slot
    // base. Does a combination of watchpoint registration and check emission to guard the
    // conditions, and emits code to load the value from the slot base. Returns a node containing
    // the loaded value. Returns null if any of the conditions were no longer checkable.
    GetByOffsetMethod planLoad(const ObjectPropertyConditionSet&);
    Node* load(SpeculatedType, const ObjectPropertyConditionSet&, NodeType = GetByOffset);

    void prepareToParseBlock();
    void clearCaches();

    // Parse a single basic block of bytecode instructions.
    bool parseBlock(unsigned limit);
    // Link block successors.
    void linkBlock(BasicBlock*, Vector<BasicBlock*>& possibleTargets);
    void linkBlocks(Vector<UnlinkedBlock>& unlinkedBlocks, Vector<BasicBlock*>& possibleTargets);
    
    VariableAccessData* newVariableAccessData(VirtualRegister operand)
    {
        ASSERT(!operand.isConstant());
        
        m_graph.m_variableAccessData.append(VariableAccessData(operand));
        return &m_graph.m_variableAccessData.last();
    }
    
    // Get/Set the operands/result of a bytecode instruction.
    Node* getDirect(VirtualRegister operand)
    {
        ASSERT(!operand.isConstant());

        // Is this an argument?
        if (operand.isArgument())
            return getArgument(operand);

        // Must be a local.
        return getLocal(operand);
    }

    Node* get(VirtualRegister operand)
    {
        if (operand.isConstant()) {
            unsigned constantIndex = operand.toConstantIndex();
            unsigned oldSize = m_constants.size();
            if (constantIndex >= oldSize || !m_constants[constantIndex]) {
                const CodeBlock& codeBlock = *m_inlineStackTop->m_codeBlock;
                JSValue value = codeBlock.getConstant(operand.offset());
                SourceCodeRepresentation sourceCodeRepresentation = codeBlock.constantSourceCodeRepresentation(operand.offset());
                if (constantIndex >= oldSize) {
                    m_constants.grow(constantIndex + 1);
                    for (unsigned i = oldSize; i < m_constants.size(); ++i)
                        m_constants[i] = nullptr;
                }

                Node* constantNode = nullptr;
                if (sourceCodeRepresentation == SourceCodeRepresentation::Double)
                    constantNode = addToGraph(DoubleConstant, OpInfo(m_graph.freezeStrong(jsDoubleNumber(value.asNumber()))));
                else
                    constantNode = addToGraph(JSConstant, OpInfo(m_graph.freezeStrong(value)));
                m_constants[constantIndex] = constantNode;
            }
            ASSERT(m_constants[constantIndex]);
            return m_constants[constantIndex];
        }
        
        if (inlineCallFrame()) {
            if (!inlineCallFrame()->isClosureCall) {
                JSFunction* callee = inlineCallFrame()->calleeConstant();
                if (operand.offset() == CallFrameSlot::callee)
                    return weakJSConstant(callee);
            }
        } else if (operand.offset() == CallFrameSlot::callee) {
            // We have to do some constant-folding here because this enables CreateThis folding. Note
            // that we don't have such watchpoint-based folding for inlined uses of Callee, since in that
            // case if the function is a singleton then we already know it.
            if (FunctionExecutable* executable = jsDynamicCast<FunctionExecutable*>(*m_vm, m_codeBlock->ownerExecutable())) {
                InferredValue* singleton = executable->singletonFunction();
                if (JSValue value = singleton->inferredValue()) {
                    m_graph.watchpoints().addLazily(singleton);
                    JSFunction* function = jsCast<JSFunction*>(value);
                    return weakJSConstant(function);
                }
            }
            return addToGraph(GetCallee);
        }
        
        return getDirect(m_inlineStackTop->remapOperand(operand));
    }
    
    enum SetMode {
        // A normal set which follows a two-phase commit that spans code origins. During
        // the current code origin it issues a MovHint, and at the start of the next
        // code origin there will be a SetLocal. If the local needs flushing, the second
        // SetLocal will be preceded with a Flush.
        NormalSet,
        
        // A set where the SetLocal happens immediately and there is still a Flush. This
        // is relevant when assigning to a local in tricky situations for the delayed
        // SetLocal logic but where we know that we have not performed any side effects
        // within this code origin. This is a safe replacement for NormalSet anytime we
        // know that we have not yet performed side effects in this code origin.
        ImmediateSetWithFlush,
        
        // A set where the SetLocal happens immediately and we do not Flush it even if
        // this is a local that is marked as needing it. This is relevant when
        // initializing locals at the top of a function.
        ImmediateNakedSet
    };
    Node* setDirect(VirtualRegister operand, Node* value, SetMode setMode = NormalSet)
    {
        addToGraph(MovHint, OpInfo(operand.offset()), value);

        // We can't exit anymore because our OSR exit state has changed.
        m_exitOK = false;

        DelayedSetLocal delayed(currentCodeOrigin(), operand, value);
        
        if (setMode == NormalSet) {
            m_setLocalQueue.append(delayed);
            return 0;
        }
        
        return delayed.execute(this, setMode);
    }
    
    void processSetLocalQueue()
    {
        for (unsigned i = 0; i < m_setLocalQueue.size(); ++i)
            m_setLocalQueue[i].execute(this);
        m_setLocalQueue.resize(0);
    }

    Node* set(VirtualRegister operand, Node* value, SetMode setMode = NormalSet)
    {
        return setDirect(m_inlineStackTop->remapOperand(operand), value, setMode);
    }
    
    Node* injectLazyOperandSpeculation(Node* node)
    {
        ASSERT(node->op() == GetLocal);
        ASSERT(node->origin.semantic.bytecodeIndex == m_currentIndex);
        ConcurrentJSLocker locker(m_inlineStackTop->m_profiledBlock->m_lock);
        LazyOperandValueProfileKey key(m_currentIndex, node->local());
        SpeculatedType prediction = m_inlineStackTop->m_lazyOperands.prediction(locker, key);
        node->variableAccessData()->predict(prediction);
        return node;
    }

    // Used in implementing get/set, above, where the operand is a local variable.
    Node* getLocal(VirtualRegister operand)
    {
        unsigned local = operand.toLocal();

        Node* node = m_currentBlock->variablesAtTail.local(local);
        
        // This has two goals: 1) link together variable access datas, and 2)
        // try to avoid creating redundant GetLocals. (1) is required for
        // correctness - no other phase will ensure that block-local variable
        // access data unification is done correctly. (2) is purely opportunistic
        // and is meant as an compile-time optimization only.
        
        VariableAccessData* variable;
        
        if (node) {
            variable = node->variableAccessData();
            
            switch (node->op()) {
            case GetLocal:
                return node;
            case SetLocal:
                return node->child1().node();
            default:
                break;
            }
        } else
            variable = newVariableAccessData(operand);
        
        node = injectLazyOperandSpeculation(addToGraph(GetLocal, OpInfo(variable)));
        m_currentBlock->variablesAtTail.local(local) = node;
        return node;
    }
    Node* setLocal(const CodeOrigin& semanticOrigin, VirtualRegister operand, Node* value, SetMode setMode = NormalSet)
    {
        CodeOrigin oldSemanticOrigin = m_currentSemanticOrigin;
        m_currentSemanticOrigin = semanticOrigin;

        unsigned local = operand.toLocal();
        
        if (setMode != ImmediateNakedSet) {
            ArgumentPosition* argumentPosition = findArgumentPositionForLocal(operand);
            if (argumentPosition)
                flushDirect(operand, argumentPosition);
            else if (m_graph.needsScopeRegister() && operand == m_codeBlock->scopeRegister())
                flush(operand);
        }

        VariableAccessData* variableAccessData = newVariableAccessData(operand);
        variableAccessData->mergeStructureCheckHoistingFailed(
            m_inlineStackTop->m_exitProfile.hasExitSite(semanticOrigin.bytecodeIndex, BadCache));
        variableAccessData->mergeCheckArrayHoistingFailed(
            m_inlineStackTop->m_exitProfile.hasExitSite(semanticOrigin.bytecodeIndex, BadIndexingType));
        Node* node = addToGraph(SetLocal, OpInfo(variableAccessData), value);
        m_currentBlock->variablesAtTail.local(local) = node;

        m_currentSemanticOrigin = oldSemanticOrigin;
        return node;
    }

    // Used in implementing get/set, above, where the operand is an argument.
    Node* getArgument(VirtualRegister operand)
    {
        unsigned argument = operand.toArgument();
        ASSERT(argument < m_numArguments);
        
        Node* node = m_currentBlock->variablesAtTail.argument(argument);

        VariableAccessData* variable;
        
        if (node) {
            variable = node->variableAccessData();
            
            switch (node->op()) {
            case GetLocal:
                return node;
            case SetLocal:
                return node->child1().node();
            default:
                break;
            }
        } else
            variable = newVariableAccessData(operand);
        
        node = injectLazyOperandSpeculation(addToGraph(GetLocal, OpInfo(variable)));
        m_currentBlock->variablesAtTail.argument(argument) = node;
        return node;
    }
    Node* setArgument(const CodeOrigin& semanticOrigin, VirtualRegister operand, Node* value, SetMode setMode = NormalSet)
    {
        CodeOrigin oldSemanticOrigin = m_currentSemanticOrigin;
        m_currentSemanticOrigin = semanticOrigin;

        unsigned argument = operand.toArgument();
        ASSERT(argument < m_numArguments);
        
        VariableAccessData* variableAccessData = newVariableAccessData(operand);

        // Always flush arguments, except for 'this'. If 'this' is created by us,
        // then make sure that it's never unboxed.
        if (argument || m_graph.needsFlushedThis()) {
            if (setMode != ImmediateNakedSet)
                flushDirect(operand);
        }
        
        if (!argument && m_codeBlock->specializationKind() == CodeForConstruct)
            variableAccessData->mergeShouldNeverUnbox(true);
        
        variableAccessData->mergeStructureCheckHoistingFailed(
            m_inlineStackTop->m_exitProfile.hasExitSite(semanticOrigin.bytecodeIndex, BadCache));
        variableAccessData->mergeCheckArrayHoistingFailed(
            m_inlineStackTop->m_exitProfile.hasExitSite(semanticOrigin.bytecodeIndex, BadIndexingType));
        Node* node = addToGraph(SetLocal, OpInfo(variableAccessData), value);
        m_currentBlock->variablesAtTail.argument(argument) = node;

        m_currentSemanticOrigin = oldSemanticOrigin;
        return node;
    }
    
    ArgumentPosition* findArgumentPositionForArgument(int argument)
    {
        InlineStackEntry* stack = m_inlineStackTop;
        while (stack->m_inlineCallFrame)
            stack = stack->m_caller;
        return stack->m_argumentPositions[argument];
    }
    
    ArgumentPosition* findArgumentPositionForLocal(VirtualRegister operand)
    {
        for (InlineStackEntry* stack = m_inlineStackTop; ; stack = stack->m_caller) {
            InlineCallFrame* inlineCallFrame = stack->m_inlineCallFrame;
            if (!inlineCallFrame)
                break;
            if (operand.offset() < static_cast<int>(inlineCallFrame->stackOffset + CallFrame::headerSizeInRegisters))
                continue;
            if (operand.offset() == inlineCallFrame->stackOffset + CallFrame::thisArgumentOffset())
                continue;
            if (operand.offset() >= static_cast<int>(inlineCallFrame->stackOffset + CallFrame::thisArgumentOffset() + inlineCallFrame->arguments.size()))
                continue;
            int argument = VirtualRegister(operand.offset() - inlineCallFrame->stackOffset).toArgument();
            return stack->m_argumentPositions[argument];
        }
        return 0;
    }
    
    ArgumentPosition* findArgumentPosition(VirtualRegister operand)
    {
        if (operand.isArgument())
            return findArgumentPositionForArgument(operand.toArgument());
        return findArgumentPositionForLocal(operand);
    }

    void flush(VirtualRegister operand)
    {
        flushDirect(m_inlineStackTop->remapOperand(operand));
    }
    
    void flushDirect(VirtualRegister operand)
    {
        flushDirect(operand, findArgumentPosition(operand));
    }

    void flushDirect(VirtualRegister operand, ArgumentPosition* argumentPosition)
    {
        addFlushOrPhantomLocal<Flush>(operand, argumentPosition);
    }

    template<NodeType nodeType>
    void addFlushOrPhantomLocal(VirtualRegister operand, ArgumentPosition* argumentPosition)
    {
        ASSERT(!operand.isConstant());
        
        Node* node = m_currentBlock->variablesAtTail.operand(operand);
        
        VariableAccessData* variable;
        
        if (node)
            variable = node->variableAccessData();
        else
            variable = newVariableAccessData(operand);
        
        node = addToGraph(nodeType, OpInfo(variable));
        m_currentBlock->variablesAtTail.operand(operand) = node;
        if (argumentPosition)
            argumentPosition->addVariable(variable);
    }

    void phantomLocalDirect(VirtualRegister operand)
    {
        addFlushOrPhantomLocal<PhantomLocal>(operand, findArgumentPosition(operand));
    }

    void flush(InlineStackEntry* inlineStackEntry)
    {
        int numArguments;
        if (InlineCallFrame* inlineCallFrame = inlineStackEntry->m_inlineCallFrame) {
            ASSERT(!m_hasDebuggerEnabled);
            numArguments = inlineCallFrame->arguments.size();
            if (inlineCallFrame->isClosureCall)
                flushDirect(inlineStackEntry->remapOperand(VirtualRegister(CallFrameSlot::callee)));
            if (inlineCallFrame->isVarargs())
                flushDirect(inlineStackEntry->remapOperand(VirtualRegister(CallFrameSlot::argumentCount)));
        } else
            numArguments = inlineStackEntry->m_codeBlock->numParameters();
        for (unsigned argument = numArguments; argument-- > 1;)
            flushDirect(inlineStackEntry->remapOperand(virtualRegisterForArgument(argument)));
        if (!inlineStackEntry->m_inlineCallFrame && m_graph.needsFlushedThis())
            flushDirect(virtualRegisterForArgument(0));
        if (m_graph.needsScopeRegister())
            flush(m_codeBlock->scopeRegister());
    }

    void flushForTerminal()
    {
        CodeOrigin origin = currentCodeOrigin();
        unsigned bytecodeIndex = origin.bytecodeIndex;

        for (InlineStackEntry* inlineStackEntry = m_inlineStackTop; inlineStackEntry; inlineStackEntry = inlineStackEntry->m_caller) {
            flush(inlineStackEntry);

            ASSERT(origin.inlineCallFrame == inlineStackEntry->m_inlineCallFrame);
            InlineCallFrame* inlineCallFrame = inlineStackEntry->m_inlineCallFrame;
            CodeBlock* codeBlock = m_graph.baselineCodeBlockFor(inlineCallFrame);
            FullBytecodeLiveness& fullLiveness = m_graph.livenessFor(codeBlock);
            const FastBitVector& livenessAtBytecode = fullLiveness.getLiveness(bytecodeIndex);

            for (unsigned local = codeBlock->m_numCalleeLocals; local--;) {
                if (livenessAtBytecode[local]) {
                    VirtualRegister reg = virtualRegisterForLocal(local);
                    if (inlineCallFrame)
                        reg = inlineStackEntry->remapOperand(reg);
                    phantomLocalDirect(reg);
                }
            }

            if (inlineCallFrame) {
                bytecodeIndex = inlineCallFrame->directCaller.bytecodeIndex;
                origin = inlineCallFrame->directCaller;
            }
        }
    }

    void flushForReturn()
    {
        flush(m_inlineStackTop);
    }
    
    void flushIfTerminal(SwitchData& data)
    {
        if (data.fallThrough.bytecodeIndex() > m_currentIndex)
            return;
        
        for (unsigned i = data.cases.size(); i--;) {
            if (data.cases[i].target.bytecodeIndex() > m_currentIndex)
                return;
        }
        
        flushForTerminal();
    }

    // Assumes that the constant should be strongly marked.
    Node* jsConstant(JSValue constantValue)
    {
        return addToGraph(JSConstant, OpInfo(m_graph.freezeStrong(constantValue)));
    }

    Node* weakJSConstant(JSValue constantValue)
    {
        return addToGraph(JSConstant, OpInfo(m_graph.freeze(constantValue)));
    }

    // Helper functions to get/set the this value.
    Node* getThis()
    {
        return get(m_inlineStackTop->m_codeBlock->thisRegister());
    }

    void setThis(Node* value)
    {
        set(m_inlineStackTop->m_codeBlock->thisRegister(), value);
    }

    InlineCallFrame* inlineCallFrame()
    {
        return m_inlineStackTop->m_inlineCallFrame;
    }

    bool allInlineFramesAreTailCalls()
    {
        return !inlineCallFrame() || !inlineCallFrame()->getCallerSkippingTailCalls();
    }

    CodeOrigin currentCodeOrigin()
    {
        return CodeOrigin(m_currentIndex, inlineCallFrame());
    }

    NodeOrigin currentNodeOrigin()
    {
        CodeOrigin semantic;
        CodeOrigin forExit;

        if (m_currentSemanticOrigin.isSet())
            semantic = m_currentSemanticOrigin;
        else
            semantic = currentCodeOrigin();

        forExit = currentCodeOrigin();

        return NodeOrigin(semantic, forExit, m_exitOK);
    }
    
    BranchData* branchData(unsigned taken, unsigned notTaken)
    {
        // We assume that branches originating from bytecode always have a fall-through. We
        // use this assumption to avoid checking for the creation of terminal blocks.
        ASSERT((taken > m_currentIndex) || (notTaken > m_currentIndex));
        BranchData* data = m_graph.m_branchData.add();
        *data = BranchData::withBytecodeIndices(taken, notTaken);
        return data;
    }
    
    Node* addToGraph(Node* node)
    {
        if (Options::verboseDFGByteCodeParsing())
            dataLog("        appended ", node, " ", Graph::opName(node->op()), "\n");
        m_currentBlock->append(node);
        if (clobbersExitState(m_graph, node))
            m_exitOK = false;
        return node;
    }
    
    Node* addToGraph(NodeType op, Node* child1 = 0, Node* child2 = 0, Node* child3 = 0)
    {
        Node* result = m_graph.addNode(
            op, currentNodeOrigin(), Edge(child1), Edge(child2),
            Edge(child3));
        return addToGraph(result);
    }
    Node* addToGraph(NodeType op, Edge child1, Edge child2 = Edge(), Edge child3 = Edge())
    {
        Node* result = m_graph.addNode(
            op, currentNodeOrigin(), child1, child2, child3);
        return addToGraph(result);
    }
    Node* addToGraph(NodeType op, OpInfo info, Node* child1 = 0, Node* child2 = 0, Node* child3 = 0)
    {
        Node* result = m_graph.addNode(
            op, currentNodeOrigin(), info, Edge(child1), Edge(child2),
            Edge(child3));
        return addToGraph(result);
    }
    Node* addToGraph(NodeType op, OpInfo info, Edge child1, Edge child2 = Edge(), Edge child3 = Edge())
    {
        Node* result = m_graph.addNode(op, currentNodeOrigin(), info, child1, child2, child3);
        return addToGraph(result);
    }
    Node* addToGraph(NodeType op, OpInfo info1, OpInfo info2, Node* child1 = 0, Node* child2 = 0, Node* child3 = 0)
    {
        Node* result = m_graph.addNode(
            op, currentNodeOrigin(), info1, info2,
            Edge(child1), Edge(child2), Edge(child3));
        return addToGraph(result);
    }
    Node* addToGraph(NodeType op, OpInfo info1, OpInfo info2, Edge child1, Edge child2 = Edge(), Edge child3 = Edge())
    {
        Node* result = m_graph.addNode(
            op, currentNodeOrigin(), info1, info2, child1, child2, child3);
        return addToGraph(result);
    }
    
    Node* addToGraph(Node::VarArgTag, NodeType op, OpInfo info1, OpInfo info2 = OpInfo())
    {
        Node* result = m_graph.addNode(
            Node::VarArg, op, currentNodeOrigin(), info1, info2,
            m_graph.m_varArgChildren.size() - m_numPassedVarArgs, m_numPassedVarArgs);
        addToGraph(result);
        
        m_numPassedVarArgs = 0;
        
        return result;
    }
    
    void addVarArgChild(Node* child)
    {
        m_graph.m_varArgChildren.append(Edge(child));
        m_numPassedVarArgs++;
    }
    
    Node* addCallWithoutSettingResult(
        NodeType op, OpInfo opInfo, Node* callee, int argCount, int registerOffset,
        OpInfo prediction)
    {
        addVarArgChild(callee);
        size_t parameterSlots = Graph::parameterSlotsForArgCount(argCount);

        if (parameterSlots > m_parameterSlots)
            m_parameterSlots = parameterSlots;

        for (int i = 0; i < argCount; ++i)
            addVarArgChild(get(virtualRegisterForArgument(i, registerOffset)));

        return addToGraph(Node::VarArg, op, opInfo, prediction);
    }
    
    Node* addCall(
        int result, NodeType op, const DOMJIT::Signature* signature, Node* callee, int argCount, int registerOffset,
        SpeculatedType prediction)
    {
        if (op == TailCall) {
            if (allInlineFramesAreTailCalls())
                return addCallWithoutSettingResult(op, OpInfo(signature), callee, argCount, registerOffset, OpInfo());
            op = TailCallInlinedCaller;
        }


        Node* call = addCallWithoutSettingResult(
            op, OpInfo(signature), callee, argCount, registerOffset, OpInfo(prediction));
        VirtualRegister resultReg(result);
        if (resultReg.isValid())
            set(resultReg, call);
        return call;
    }
    
    Node* cellConstantWithStructureCheck(JSCell* object, Structure* structure)
    {
        // FIXME: This should route to emitPropertyCheck, not the other way around. But currently,
        // this gets no profit from using emitPropertyCheck() since we'll non-adaptively watch the
        // object's structure as soon as we make it a weakJSCosntant.
        Node* objectNode = weakJSConstant(object);
        addToGraph(CheckStructure, OpInfo(m_graph.addStructureSet(structure)), objectNode);
        return objectNode;
    }
    
    SpeculatedType getPredictionWithoutOSRExit(unsigned bytecodeIndex)
    {
        SpeculatedType prediction;
        {
            ConcurrentJSLocker locker(m_inlineStackTop->m_profiledBlock->m_lock);
            prediction = m_inlineStackTop->m_profiledBlock->valueProfilePredictionForBytecodeOffset(locker, bytecodeIndex);
        }

        if (prediction != SpecNone)
            return prediction;

        // If we have no information about the values this
        // node generates, we check if by any chance it is
        // a tail call opcode. In that case, we walk up the
        // inline frames to find a call higher in the call
        // chain and use its prediction. If we only have
        // inlined tail call frames, we use SpecFullTop
        // to avoid a spurious OSR exit.
        Instruction* instruction = m_inlineStackTop->m_profiledBlock->instructions().begin() + bytecodeIndex;
        OpcodeID opcodeID = m_vm->interpreter->getOpcodeID(instruction->u.opcode);

        switch (opcodeID) {
        case op_tail_call:
        case op_tail_call_varargs:
        case op_tail_call_forward_arguments: {
            // Things should be more permissive to us returning BOTTOM instead of TOP here.
            // Currently, this will cause us to Force OSR exit. This is bad because returning
            // TOP will cause anything that transitively touches this speculated type to
            // also become TOP during prediction propagation.
            // https://bugs.webkit.org/show_bug.cgi?id=164337
            if (!inlineCallFrame())
                return SpecFullTop;

            CodeOrigin* codeOrigin = inlineCallFrame()->getCallerSkippingTailCalls();
            if (!codeOrigin)
                return SpecFullTop;

            InlineStackEntry* stack = m_inlineStackTop;
            while (stack->m_inlineCallFrame != codeOrigin->inlineCallFrame)
                stack = stack->m_caller;

            bytecodeIndex = codeOrigin->bytecodeIndex;
            CodeBlock* profiledBlock = stack->m_profiledBlock;
            ConcurrentJSLocker locker(profiledBlock->m_lock);
            return profiledBlock->valueProfilePredictionForBytecodeOffset(locker, bytecodeIndex);
        }

        default:
            return SpecNone;
        }

        RELEASE_ASSERT_NOT_REACHED();
        return SpecNone;
    }

    SpeculatedType getPrediction(unsigned bytecodeIndex)
    {
        SpeculatedType prediction = getPredictionWithoutOSRExit(bytecodeIndex);

        if (prediction == SpecNone) {
            // We have no information about what values this node generates. Give up
            // on executing this code, since we're likely to do more damage than good.
            addToGraph(ForceOSRExit);
        }
        
        return prediction;
    }
    
    SpeculatedType getPredictionWithoutOSRExit()
    {
        return getPredictionWithoutOSRExit(m_currentIndex);
    }
    
    SpeculatedType getPrediction()
    {
        return getPrediction(m_currentIndex);
    }
    
    ArrayMode getArrayMode(ArrayProfile* profile, Array::Action action)
    {
        ConcurrentJSLocker locker(m_inlineStackTop->m_profiledBlock->m_lock);
        profile->computeUpdatedPrediction(locker, m_inlineStackTop->m_profiledBlock);
        bool makeSafe = profile->outOfBounds(locker);
        return ArrayMode::fromObserved(locker, profile, action, makeSafe);
    }
    
    ArrayMode getArrayMode(ArrayProfile* profile)
    {
        return getArrayMode(profile, Array::Read);
    }
    
    Node* makeSafe(Node* node)
    {
        if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, Overflow))
            node->mergeFlags(NodeMayOverflowInt32InDFG);
        if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, NegativeZero))
            node->mergeFlags(NodeMayNegZeroInDFG);
        
        if (!isX86() && node->op() == ArithMod)
            return node;

        {
            ArithProfile* arithProfile = m_inlineStackTop->m_profiledBlock->arithProfileForBytecodeOffset(m_currentIndex);
            if (arithProfile) {
                switch (node->op()) {
                case ArithAdd:
                case ArithSub:
                case ValueAdd:
                    if (arithProfile->didObserveDouble())
                        node->mergeFlags(NodeMayHaveDoubleResult);
                    if (arithProfile->didObserveNonNumber())
                        node->mergeFlags(NodeMayHaveNonNumberResult);
                    break;
                
                case ArithMul: {
                    if (arithProfile->didObserveInt52Overflow())
                        node->mergeFlags(NodeMayOverflowInt52);
                    if (arithProfile->didObserveInt32Overflow() || m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, Overflow))
                        node->mergeFlags(NodeMayOverflowInt32InBaseline);
                    if (arithProfile->didObserveNegZeroDouble() || m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, NegativeZero))
                        node->mergeFlags(NodeMayNegZeroInBaseline);
                    if (arithProfile->didObserveDouble())
                        node->mergeFlags(NodeMayHaveDoubleResult);
                    if (arithProfile->didObserveNonNumber())
                        node->mergeFlags(NodeMayHaveNonNumberResult);
                    break;
                }
                case ArithNegate: {
                    ASSERT_WITH_MESSAGE(!arithProfile->didObserveNonNumber(), "op_negate starts with a toNumber() on the argument, it should only produce numbers.");

                    if (arithProfile->lhsObservedType().sawNumber() || arithProfile->didObserveDouble())
                        node->mergeFlags(NodeMayHaveDoubleResult);
                    if (arithProfile->didObserveNegZeroDouble() || m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, NegativeZero))
                        node->mergeFlags(NodeMayNegZeroInBaseline);
                    if (arithProfile->didObserveInt32Overflow() || m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, Overflow))
                        node->mergeFlags(NodeMayOverflowInt32InBaseline);
                    break;
                }
                
                default:
                    break;
                }
            }
        }
        
        if (m_inlineStackTop->m_profiledBlock->likelyToTakeSlowCase(m_currentIndex)) {
            switch (node->op()) {
            case UInt32ToNumber:
            case ArithAdd:
            case ArithSub:
            case ValueAdd:
            case ArithMod: // for ArithMod "MayOverflow" means we tried to divide by zero, or we saw double.
                node->mergeFlags(NodeMayOverflowInt32InBaseline);
                break;
                
            default:
                break;
            }
        }
        
        return node;
    }
    
    Node* makeDivSafe(Node* node)
    {
        ASSERT(node->op() == ArithDiv);
        
        if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, Overflow))
            node->mergeFlags(NodeMayOverflowInt32InDFG);
        if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, NegativeZero))
            node->mergeFlags(NodeMayNegZeroInDFG);
        
        // The main slow case counter for op_div in the old JIT counts only when
        // the operands are not numbers. We don't care about that since we already
        // have speculations in place that take care of that separately. We only
        // care about when the outcome of the division is not an integer, which
        // is what the special fast case counter tells us.
        
        if (!m_inlineStackTop->m_profiledBlock->couldTakeSpecialFastCase(m_currentIndex))
            return node;
        
        // FIXME: It might be possible to make this more granular.
        node->mergeFlags(NodeMayOverflowInt32InBaseline | NodeMayNegZeroInBaseline);
        
        return node;
    }
    
    void noticeArgumentsUse()
    {
        // All of the arguments in this function need to be formatted as JSValues because we will
        // load from them in a random-access fashion and we don't want to have to switch on
        // format.
        
        for (ArgumentPosition* argument : m_inlineStackTop->m_argumentPositions)
            argument->mergeShouldNeverUnbox(true);
    }

    bool needsDynamicLookup(ResolveType, OpcodeID);
    
    VM* m_vm;
    CodeBlock* m_codeBlock;
    CodeBlock* m_profiledBlock;
    Graph& m_graph;

    // The current block being generated.
    BasicBlock* m_currentBlock;
    // The bytecode index of the current instruction being generated.
    unsigned m_currentIndex;
    // The semantic origin of the current node if different from the current Index.
    CodeOrigin m_currentSemanticOrigin;
    // True if it's OK to OSR exit right now.
    bool m_exitOK { false };

    FrozenValue* m_constantUndefined;
    FrozenValue* m_constantNull;
    FrozenValue* m_constantNaN;
    FrozenValue* m_constantOne;
    Vector<Node*, 16> m_constants;

    // The number of arguments passed to the function.
    unsigned m_numArguments;
    // The number of locals (vars + temporaries) used in the function.
    unsigned m_numLocals;
    // The number of slots (in units of sizeof(Register)) that we need to
    // preallocate for arguments to outgoing calls from this frame. This
    // number includes the CallFrame slots that we initialize for the callee
    // (but not the callee-initialized CallerFrame and ReturnPC slots).
    // This number is 0 if and only if this function is a leaf.
    unsigned m_parameterSlots;
    // The number of var args passed to the next var arg node.
    unsigned m_numPassedVarArgs;

    HashMap<ConstantBufferKey, unsigned> m_constantBufferCache;
    
    struct InlineStackEntry {
        ByteCodeParser* m_byteCodeParser;
        
        CodeBlock* m_codeBlock;
        CodeBlock* m_profiledBlock;
        InlineCallFrame* m_inlineCallFrame;
        
        ScriptExecutable* executable() { return m_codeBlock->ownerScriptExecutable(); }
        
        QueryableExitProfile m_exitProfile;
        
        // Remapping of identifier and constant numbers from the code block being
        // inlined (inline callee) to the code block that we're inlining into
        // (the machine code block, which is the transitive, though not necessarily
        // direct, caller).
        Vector<unsigned> m_identifierRemap;
        Vector<unsigned> m_constantBufferRemap;
        Vector<unsigned> m_switchRemap;
        
        // Blocks introduced by this code block, which need successor linking.
        // May include up to one basic block that includes the continuation after
        // the callsite in the caller. These must be appended in the order that they
        // are created, but their bytecodeBegin values need not be in order as they
        // are ignored.
        Vector<UnlinkedBlock> m_unlinkedBlocks;
        
        // Potential block linking targets. Must be sorted by bytecodeBegin, and
        // cannot have two blocks that have the same bytecodeBegin.
        Vector<BasicBlock*> m_blockLinkingTargets;
        
        // If the callsite's basic block was split into two, then this will be
        // the head of the callsite block. It needs its successors linked to the
        // m_unlinkedBlocks, but not the other way around: there's no way for
        // any blocks in m_unlinkedBlocks to jump back into this block.
        BasicBlock* m_callsiteBlockHead;
        
        // Does the callsite block head need linking? This is typically true
        // but will be false for the machine code block's inline stack entry
        // (since that one is not inlined) and for cases where an inline callee
        // did the linking for us.
        bool m_callsiteBlockHeadNeedsLinking;
        
        VirtualRegister m_returnValue;
        
        // Speculations about variable types collected from the profiled code block,
        // which are based on OSR exit profiles that past DFG compilatins of this
        // code block had gathered.
        LazyOperandValueProfileParser m_lazyOperands;
        
        CallLinkInfoMap m_callLinkInfos;
        StubInfoMap m_stubInfos;
        ByValInfoMap m_byValInfos;
        
        // Did we see any returns? We need to handle the (uncommon but necessary)
        // case where a procedure that does not return was inlined.
        bool m_didReturn;
        
        // Did we have any early returns?
        bool m_didEarlyReturn;
        
        // Pointers to the argument position trackers for this slice of code.
        Vector<ArgumentPosition*> m_argumentPositions;
        
        InlineStackEntry* m_caller;
        
        InlineStackEntry(
            ByteCodeParser*,
            CodeBlock*,
            CodeBlock* profiledBlock,
            BasicBlock* callsiteBlockHead,
            JSFunction* callee, // Null if this is a closure call.
            VirtualRegister returnValueVR,
            VirtualRegister inlineCallFrameStart,
            int argumentCountIncludingThis,
            InlineCallFrame::Kind);
        
        ~InlineStackEntry()
        {
            m_byteCodeParser->m_inlineStackTop = m_caller;
        }
        
        VirtualRegister remapOperand(VirtualRegister operand) const
        {
            if (!m_inlineCallFrame)
                return operand;
            
            ASSERT(!operand.isConstant());

            return VirtualRegister(operand.offset() + m_inlineCallFrame->stackOffset);
        }
    };
    
    InlineStackEntry* m_inlineStackTop;
    
    struct DelayedSetLocal {
        CodeOrigin m_origin;
        VirtualRegister m_operand;
        Node* m_value;
        
        DelayedSetLocal() { }
        DelayedSetLocal(const CodeOrigin& origin, VirtualRegister operand, Node* value)
            : m_origin(origin)
            , m_operand(operand)
            , m_value(value)
        {
        }
        
        Node* execute(ByteCodeParser* parser, SetMode setMode = NormalSet)
        {
            if (m_operand.isArgument())
                return parser->setArgument(m_origin, m_operand, m_value, setMode);
            return parser->setLocal(m_origin, m_operand, m_value, setMode);
        }
    };
    
    Vector<DelayedSetLocal, 2> m_setLocalQueue;

    CodeBlock* m_dfgCodeBlock;
    CallLinkStatus::ContextMap m_callContextMap;
    StubInfoMap m_dfgStubInfos;
    
    Instruction* m_currentInstruction;
    bool m_hasDebuggerEnabled;
};

// The idiom:
//     if (true) { ...; goto label; } else label: continue
// Allows using NEXT_OPCODE as a statement, even in unbraced if+else, while containing a `continue`.
// The more common idiom:
//     do { ...; } while (false)
// Doesn't allow using `continue`.
#define NEXT_OPCODE(name) \
    if (true) { \
        m_currentIndex += OPCODE_LENGTH(name); \
        goto WTF_CONCAT(NEXT_OPCODE_, __LINE__); /* Need a unique label: usable more than once per function. */ \
    } else \
    WTF_CONCAT(NEXT_OPCODE_, __LINE__): \
        continue

// Chain expressions with comma-operator so LAST_OPCODE can be used as a statement.
#define LAST_OPCODE(name) \
    return \
        m_currentIndex += OPCODE_LENGTH(name), \
        m_exitOK = false, \
        shouldContinueParsing

ByteCodeParser::Terminality ByteCodeParser::handleCall(Instruction* pc, NodeType op, CallMode callMode)
{
    ASSERT(OPCODE_LENGTH(op_call) == OPCODE_LENGTH(op_construct));
    ASSERT(OPCODE_LENGTH(op_call) == OPCODE_LENGTH(op_tail_call));
    return handleCall(
        pc[1].u.operand, op, callMode, OPCODE_LENGTH(op_call),
        pc[2].u.operand, pc[3].u.operand, -pc[4].u.operand);
}

ByteCodeParser::Terminality ByteCodeParser::handleCall(
    int result, NodeType op, CallMode callMode, unsigned instructionSize,
    int callee, int argumentCountIncludingThis, int registerOffset)
{
    Node* callTarget = get(VirtualRegister(callee));
    
    CallLinkStatus callLinkStatus = CallLinkStatus::computeFor(
        m_inlineStackTop->m_profiledBlock, currentCodeOrigin(),
        m_inlineStackTop->m_callLinkInfos, m_callContextMap);
    
    return handleCall(
        result, op, callMode, instructionSize, callTarget,
        argumentCountIncludingThis, registerOffset, callLinkStatus);
}
    
ByteCodeParser::Terminality ByteCodeParser::handleCall(
    int result, NodeType op, CallMode callMode, unsigned instructionSize,
    Node* callTarget, int argumentCountIncludingThis, int registerOffset,
    CallLinkStatus callLinkStatus)
{
    return handleCall(
        result, op, InlineCallFrame::kindFor(callMode), instructionSize, callTarget, argumentCountIncludingThis,
        registerOffset, callLinkStatus, getPrediction());
}

void ByteCodeParser::refineStatically(CallLinkStatus& callLinkStatus, Node* callTarget)
{
    if (callTarget->isCellConstant()) {
        callLinkStatus.setProvenConstantCallee(CallVariant(callTarget->asCell()));
        return;
    }
}

ByteCodeParser::Terminality ByteCodeParser::handleCall(
    int result, NodeType op, InlineCallFrame::Kind kind, unsigned instructionSize,
    Node* callTarget, int argumentCountIncludingThis, int registerOffset,
    CallLinkStatus callLinkStatus, SpeculatedType prediction)
{
    ASSERT(registerOffset <= 0);

    refineStatically(callLinkStatus, callTarget);
    
    if (Options::verboseDFGByteCodeParsing())
        dataLog("    Handling call at ", currentCodeOrigin(), ": ", callLinkStatus, "\n");
    
    if (!callLinkStatus.canOptimize()) {
        // Oddly, this conflates calls that haven't executed with calls that behaved sufficiently polymorphically
        // that we cannot optimize them.

        Node* callNode = addCall(result, op, nullptr, callTarget, argumentCountIncludingThis, registerOffset, prediction);
        if (callNode->op() == TailCall)
            return Terminal;
        ASSERT(callNode->op() != TailCallVarargs && callNode->op() != TailCallForwardVarargs);
        return NonTerminal;
    }
    
    unsigned nextOffset = m_currentIndex + instructionSize;
    
    if (handleInlining(callTarget, result, callLinkStatus, registerOffset, virtualRegisterForArgument(0, registerOffset), VirtualRegister(), 0, argumentCountIncludingThis, nextOffset, op, kind, prediction)) {
        if (m_graph.compilation())
            m_graph.compilation()->noticeInlinedCall();
        return NonTerminal;
    }
    
    Node* callNode = addCall(result, op, nullptr, callTarget, argumentCountIncludingThis, registerOffset, prediction);
    if (callNode->op() == TailCall)
        return Terminal;
    ASSERT(callNode->op() != TailCallVarargs && callNode->op() != TailCallForwardVarargs);
    return NonTerminal;
}

ByteCodeParser::Terminality ByteCodeParser::handleVarargsCall(Instruction* pc, NodeType op, CallMode callMode)
{
    ASSERT(OPCODE_LENGTH(op_call_varargs) == OPCODE_LENGTH(op_construct_varargs));
    ASSERT(OPCODE_LENGTH(op_call_varargs) == OPCODE_LENGTH(op_tail_call_varargs));

    int result = pc[1].u.operand;
    int callee = pc[2].u.operand;
    int thisReg = pc[3].u.operand;
    int arguments = pc[4].u.operand;
    int firstFreeReg = pc[5].u.operand;
    int firstVarArgOffset = pc[6].u.operand;
    
    SpeculatedType prediction = getPrediction();
    
    Node* callTarget = get(VirtualRegister(callee));
    
    CallLinkStatus callLinkStatus = CallLinkStatus::computeFor(
        m_inlineStackTop->m_profiledBlock, currentCodeOrigin(),
        m_inlineStackTop->m_callLinkInfos, m_callContextMap);
    refineStatically(callLinkStatus, callTarget);
    
    if (Options::verboseDFGByteCodeParsing())
        dataLog("    Varargs call link status at ", currentCodeOrigin(), ": ", callLinkStatus, "\n");
    
    if (callLinkStatus.canOptimize()
        && handleInlining(callTarget, result, callLinkStatus, firstFreeReg, VirtualRegister(thisReg), VirtualRegister(arguments), firstVarArgOffset, 0, m_currentIndex + OPCODE_LENGTH(op_call_varargs), op, InlineCallFrame::varargsKindFor(callMode), prediction)) {
        if (m_graph.compilation())
            m_graph.compilation()->noticeInlinedCall();
        return NonTerminal;
    }
    
    CallVarargsData* data = m_graph.m_callVarargsData.add();
    data->firstVarArgOffset = firstVarArgOffset;
    
    Node* thisChild = get(VirtualRegister(thisReg));
    Node* argumentsChild = nullptr;
    if (op != TailCallForwardVarargs)
        argumentsChild = get(VirtualRegister(arguments));

    if (op == TailCallVarargs || op == TailCallForwardVarargs) {
        if (allInlineFramesAreTailCalls()) {
            addToGraph(op, OpInfo(data), OpInfo(), callTarget, thisChild, argumentsChild);
            return Terminal;
        }
        op = op == TailCallVarargs ? TailCallVarargsInlinedCaller : TailCallForwardVarargsInlinedCaller;
    }

    Node* call = addToGraph(op, OpInfo(data), OpInfo(prediction), callTarget, thisChild, argumentsChild);
    VirtualRegister resultReg(result);
    if (resultReg.isValid())
        set(resultReg, call);
    return NonTerminal;
}

void ByteCodeParser::emitFunctionChecks(CallVariant callee, Node* callTarget, VirtualRegister thisArgumentReg)
{
    Node* thisArgument;
    if (thisArgumentReg.isValid())
        thisArgument = get(thisArgumentReg);
    else
        thisArgument = 0;

    JSCell* calleeCell;
    Node* callTargetForCheck;
    if (callee.isClosureCall()) {
        calleeCell = callee.executable();
        callTargetForCheck = addToGraph(GetExecutable, callTarget);
    } else {
        calleeCell = callee.nonExecutableCallee();
        callTargetForCheck = callTarget;
    }
    
    ASSERT(calleeCell);
    addToGraph(CheckCell, OpInfo(m_graph.freeze(calleeCell)), callTargetForCheck, thisArgument);
}

Node* ByteCodeParser::getArgumentCount()
{
    Node* argumentCount;
    if (m_inlineStackTop->m_inlineCallFrame) {
        if (m_inlineStackTop->m_inlineCallFrame->isVarargs())
            argumentCount = get(VirtualRegister(CallFrameSlot::argumentCount));
        else
            argumentCount = jsConstant(m_graph.freeze(jsNumber(m_inlineStackTop->m_inlineCallFrame->arguments.size()))->value());
    } else
        argumentCount = addToGraph(GetArgumentCountIncludingThis, OpInfo(0), OpInfo(SpecInt32Only));
    return argumentCount;
}

void ByteCodeParser::emitArgumentPhantoms(int registerOffset, int argumentCountIncludingThis)
{
    for (int i = 0; i < argumentCountIncludingThis; ++i)
        addToGraph(Phantom, get(virtualRegisterForArgument(i, registerOffset)));
}

unsigned ByteCodeParser::inliningCost(CallVariant callee, int argumentCountIncludingThis, CallMode callMode)
{
    CodeSpecializationKind kind = specializationKindFor(callMode);
    if (verbose)
        dataLog("Considering inlining ", callee, " into ", currentCodeOrigin(), "\n");
    
    if (m_hasDebuggerEnabled) {
        if (verbose)
            dataLog("    Failing because the debugger is in use.\n");
        return UINT_MAX;
    }

    FunctionExecutable* executable = callee.functionExecutable();
    if (!executable) {
        if (verbose)
            dataLog("    Failing because there is no function executable.\n");
        return UINT_MAX;
    }
    
    // Do we have a code block, and does the code block's size match the heuristics/requirements for
    // being an inline candidate? We might not have a code block (1) if code was thrown away,
    // (2) if we simply hadn't actually made this call yet or (3) code is a builtin function and
    // specialization kind is construct. In the former 2 cases, we could still theoretically attempt
    // to inline it if we had a static proof of what was being called; this might happen for example
    // if you call a global function, where watchpointing gives us static information. Overall,
    // it's a rare case because we expect that any hot callees would have already been compiled.
    CodeBlock* codeBlock = executable->baselineCodeBlockFor(kind);
    if (!codeBlock) {
        if (verbose)
            dataLog("    Failing because no code block available.\n");
        return UINT_MAX;
    }

    // Does the number of arguments we're passing match the arity of the target? We currently
    // inline only if the number of arguments passed is greater than or equal to the number
    // arguments expected.
    if (codeBlock->numParameters() > argumentCountIncludingThis) {
        if (verbose)
            dataLog("    Failing because of arity mismatch.\n");
        return UINT_MAX;
    }

    CapabilityLevel capabilityLevel = inlineFunctionForCapabilityLevel(
        codeBlock, kind, callee.isClosureCall());
    if (verbose) {
        dataLog("    Call mode: ", callMode, "\n");
        dataLog("    Is closure call: ", callee.isClosureCall(), "\n");
        dataLog("    Capability level: ", capabilityLevel, "\n");
        dataLog("    Might inline function: ", mightInlineFunctionFor(codeBlock, kind), "\n");
        dataLog("    Might compile function: ", mightCompileFunctionFor(codeBlock, kind), "\n");
        dataLog("    Is supported for inlining: ", isSupportedForInlining(codeBlock), "\n");
        dataLog("    Is inlining candidate: ", codeBlock->ownerScriptExecutable()->isInliningCandidate(), "\n");
    }
    if (!canInline(capabilityLevel)) {
        if (verbose)
            dataLog("    Failing because the function is not inlineable.\n");
        return UINT_MAX;
    }
    
    // Check if the caller is already too large. We do this check here because that's just
    // where we happen to also have the callee's code block, and we want that for the
    // purpose of unsetting SABI.
    if (!isSmallEnoughToInlineCodeInto(m_codeBlock)) {
        codeBlock->m_shouldAlwaysBeInlined = false;
        if (verbose)
            dataLog("    Failing because the caller is too large.\n");
        return UINT_MAX;
    }
    
    // FIXME: this should be better at predicting how much bloat we will introduce by inlining
    // this function.
    // https://bugs.webkit.org/show_bug.cgi?id=127627
    
    // FIXME: We currently inline functions that have run in LLInt but not in Baseline. These
    // functions have very low fidelity profiling, and presumably they weren't very hot if they
    // haven't gotten to Baseline yet. Consider not inlining these functions.
    // https://bugs.webkit.org/show_bug.cgi?id=145503
    
    // Have we exceeded inline stack depth, or are we trying to inline a recursive call to
    // too many levels? If either of these are detected, then don't inline. We adjust our
    // heuristics if we are dealing with a function that cannot otherwise be compiled.
    
    unsigned depth = 0;
    unsigned recursion = 0;
    
    for (InlineStackEntry* entry = m_inlineStackTop; entry; entry = entry->m_caller) {
        ++depth;
        if (depth >= Options::maximumInliningDepth()) {
            if (verbose)
                dataLog("    Failing because depth exceeded.\n");
            return UINT_MAX;
        }
        
        if (entry->executable() == executable) {
            ++recursion;
            if (recursion >= Options::maximumInliningRecursion()) {
                if (verbose)
                    dataLog("    Failing because recursion detected.\n");
                return UINT_MAX;
            }
        }
    }
    
    if (verbose)
        dataLog("    Inlining should be possible.\n");
    
    // It might be possible to inline.
    return codeBlock->instructionCount();
}

template<typename ChecksFunctor>
void ByteCodeParser::inlineCall(Node* callTargetNode, int resultOperand, CallVariant callee, int registerOffset, int argumentCountIncludingThis, unsigned nextOffset, InlineCallFrame::Kind kind, CallerLinkability callerLinkability, const ChecksFunctor& insertChecks)
{
    CodeSpecializationKind specializationKind = InlineCallFrame::specializationKindFor(kind);
    
    ASSERT(inliningCost(callee, argumentCountIncludingThis, InlineCallFrame::callModeFor(kind)) != UINT_MAX);
    
    CodeBlock* codeBlock = callee.functionExecutable()->baselineCodeBlockFor(specializationKind);
    insertChecks(codeBlock);

    // FIXME: Don't flush constants!
    
    int inlineCallFrameStart = m_inlineStackTop->remapOperand(VirtualRegister(registerOffset)).offset() + CallFrame::headerSizeInRegisters;
    
    ensureLocals(
        VirtualRegister(inlineCallFrameStart).toLocal() + 1 +
        CallFrame::headerSizeInRegisters + codeBlock->m_numCalleeLocals);
    
    size_t argumentPositionStart = m_graph.m_argumentPositions.size();

    VirtualRegister resultReg(resultOperand);
    if (resultReg.isValid())
        resultReg = m_inlineStackTop->remapOperand(resultReg);

    VariableAccessData* calleeVariable = nullptr;
    if (callee.isClosureCall()) {
        Node* calleeSet = set(
            VirtualRegister(registerOffset + CallFrameSlot::callee), callTargetNode, ImmediateNakedSet);
        
        calleeVariable = calleeSet->variableAccessData();
        calleeVariable->mergeShouldNeverUnbox(true);
    }
    
    InlineStackEntry inlineStackEntry(
        this, codeBlock, codeBlock, m_graph.lastBlock(), callee.function(), resultReg,
        (VirtualRegister)inlineCallFrameStart, argumentCountIncludingThis, kind);
    
    // This is where the actual inlining really happens.
    unsigned oldIndex = m_currentIndex;
    m_currentIndex = 0;

    // At this point, it's again OK to OSR exit.
    m_exitOK = true;

    InlineVariableData inlineVariableData;
    inlineVariableData.inlineCallFrame = m_inlineStackTop->m_inlineCallFrame;
    inlineVariableData.argumentPositionStart = argumentPositionStart;
    inlineVariableData.calleeVariable = 0;
    
    RELEASE_ASSERT(
        m_inlineStackTop->m_inlineCallFrame->isClosureCall
        == callee.isClosureCall());
    if (callee.isClosureCall()) {
        RELEASE_ASSERT(calleeVariable);
        inlineVariableData.calleeVariable = calleeVariable;
    }
    
    m_graph.m_inlineVariableData.append(inlineVariableData);

    parseCodeBlock();
    clearCaches(); // Reset our state now that we're back to the outer code.
    
    m_currentIndex = oldIndex;
    m_exitOK = false;
    
    // If the inlined code created some new basic blocks, then we have linking to do.
    if (inlineStackEntry.m_callsiteBlockHead != m_graph.lastBlock()) {
        
        ASSERT(!inlineStackEntry.m_unlinkedBlocks.isEmpty());
        if (inlineStackEntry.m_callsiteBlockHeadNeedsLinking)
            linkBlock(inlineStackEntry.m_callsiteBlockHead, inlineStackEntry.m_blockLinkingTargets);
        else
            ASSERT(inlineStackEntry.m_callsiteBlockHead->isLinked);
        
        if (callerLinkability == CallerDoesNormalLinking)
            cancelLinkingForBlock(inlineStackEntry.m_caller, inlineStackEntry.m_callsiteBlockHead);
        
        linkBlocks(inlineStackEntry.m_unlinkedBlocks, inlineStackEntry.m_blockLinkingTargets);
    } else
        ASSERT(inlineStackEntry.m_unlinkedBlocks.isEmpty());
    
    BasicBlock* lastBlock = m_graph.lastBlock();
    // If there was a return, but no early returns, then we're done. We allow parsing of
    // the caller to continue in whatever basic block we're in right now.
    if (!inlineStackEntry.m_didEarlyReturn && inlineStackEntry.m_didReturn) {
        if (Options::verboseDFGByteCodeParsing())
            dataLog("    Allowing parsing to continue in last inlined block.\n");
        
        ASSERT(lastBlock->isEmpty() || !lastBlock->terminal());
        
        // If we created new blocks then the last block needs linking, but in the
        // caller. It doesn't need to be linked to, but it needs outgoing links.
        if (!inlineStackEntry.m_unlinkedBlocks.isEmpty()) {
            // For debugging purposes, set the bytecodeBegin. Note that this doesn't matter
            // for release builds because this block will never serve as a potential target
            // in the linker's binary search.
            if (Options::verboseDFGByteCodeParsing())
                dataLog("        Repurposing last block from ", lastBlock->bytecodeBegin, " to ", m_currentIndex, "\n");
            lastBlock->bytecodeBegin = m_currentIndex;
            if (callerLinkability == CallerDoesNormalLinking) {
                if (verbose)
                    dataLog("Adding unlinked block ", RawPointer(m_graph.lastBlock()), " (one return)\n");
                m_inlineStackTop->m_caller->m_unlinkedBlocks.append(UnlinkedBlock(m_graph.lastBlock()));
            }
        }
        
        m_currentBlock = m_graph.lastBlock();
        return;
    }

    if (Options::verboseDFGByteCodeParsing())
        dataLog("    Creating new block after inlining.\n");

    // If we get to this point then all blocks must end in some sort of terminals.
    ASSERT(lastBlock->terminal());

    // Need to create a new basic block for the continuation at the caller.
    Ref<BasicBlock> block = adoptRef(*new BasicBlock(nextOffset, m_numArguments, m_numLocals, 1));

    // Link the early returns to the basic block we're about to create.
    for (size_t i = 0; i < inlineStackEntry.m_unlinkedBlocks.size(); ++i) {
        if (!inlineStackEntry.m_unlinkedBlocks[i].m_needsEarlyReturnLinking)
            continue;
        BasicBlock* blockToLink = inlineStackEntry.m_unlinkedBlocks[i].m_block;
        ASSERT(!blockToLink->isLinked);
        Node* node = blockToLink->terminal();
        ASSERT(node->op() == Jump);
        ASSERT(!node->targetBlock());
        node->targetBlock() = block.ptr();
        inlineStackEntry.m_unlinkedBlocks[i].m_needsEarlyReturnLinking = false;
        if (verbose)
            dataLog("Marking ", RawPointer(blockToLink), " as linked (jumps to return)\n");
        blockToLink->didLink();
    }
    
    m_currentBlock = block.ptr();
    ASSERT(m_inlineStackTop->m_caller->m_blockLinkingTargets.isEmpty() || m_inlineStackTop->m_caller->m_blockLinkingTargets.last()->bytecodeBegin < nextOffset);
    if (verbose)
        dataLog("Adding unlinked block ", RawPointer(block.ptr()), " (many returns)\n");
    if (callerLinkability == CallerDoesNormalLinking) {
        m_inlineStackTop->m_caller->m_unlinkedBlocks.append(UnlinkedBlock(block.ptr()));
        m_inlineStackTop->m_caller->m_blockLinkingTargets.append(block.ptr());
    }
    m_graph.appendBlock(WTFMove(block));
    prepareToParseBlock();
}

void ByteCodeParser::cancelLinkingForBlock(InlineStackEntry* inlineStackEntry, BasicBlock* block)
{
    // It's possible that the callsite block head is not owned by the caller.
    if (!inlineStackEntry->m_unlinkedBlocks.isEmpty()) {
        // It's definitely owned by the caller, because the caller created new blocks.
        // Assert that this all adds up.
        ASSERT_UNUSED(block, inlineStackEntry->m_unlinkedBlocks.last().m_block == block);
        ASSERT(inlineStackEntry->m_unlinkedBlocks.last().m_needsNormalLinking);
        inlineStackEntry->m_unlinkedBlocks.last().m_needsNormalLinking = false;
    } else {
        // It's definitely not owned by the caller. Tell the caller that he does not
        // need to link his callsite block head, because we did it for him.
        ASSERT(inlineStackEntry->m_callsiteBlockHeadNeedsLinking);
        ASSERT_UNUSED(block, inlineStackEntry->m_callsiteBlockHead == block);
        inlineStackEntry->m_callsiteBlockHeadNeedsLinking = false;
    }
}

template<typename ChecksFunctor>
bool ByteCodeParser::attemptToInlineCall(Node* callTargetNode, int resultOperand, CallVariant callee, int registerOffset, int argumentCountIncludingThis, unsigned nextOffset, InlineCallFrame::Kind kind, CallerLinkability callerLinkability, SpeculatedType prediction, unsigned& inliningBalance, const ChecksFunctor& insertChecks)
{
    CodeSpecializationKind specializationKind = InlineCallFrame::specializationKindFor(kind);
    
    if (!inliningBalance)
        return false;
    
    if (verbose)
        dataLog("    Considering callee ", callee, "\n");
    
    // Intrinsics and internal functions can only be inlined if we're not doing varargs. This is because
    // we currently don't have any way of getting profiling information for arguments to non-JS varargs
    // calls. The prediction propagator won't be of any help because LoadVarargs obscures the data flow,
    // and there are no callsite value profiles and native function won't have callee value profiles for
    // those arguments. Even worse, if the intrinsic decides to exit, it won't really have anywhere to
    // exit to: LoadVarargs is effectful and it's part of the op_call_varargs, so we can't exit without
    // calling LoadVarargs twice.
    if (!InlineCallFrame::isVarargs(kind)) {

        bool didInsertChecks = false;
        auto insertChecksWithAccounting = [&] () {
            insertChecks(nullptr);
            didInsertChecks = true;
        };
    
        if (InternalFunction* function = callee.internalFunction()) {
            if (handleConstantInternalFunction(callTargetNode, resultOperand, function, registerOffset, argumentCountIncludingThis, specializationKind, prediction, insertChecksWithAccounting)) {
                RELEASE_ASSERT(didInsertChecks);
                addToGraph(Phantom, callTargetNode);
                emitArgumentPhantoms(registerOffset, argumentCountIncludingThis);
                inliningBalance--;
                return true;
            }
            RELEASE_ASSERT(!didInsertChecks);
            return false;
        }
    
        Intrinsic intrinsic = callee.intrinsicFor(specializationKind);
        if (intrinsic != NoIntrinsic) {
            if (handleIntrinsicCall(callTargetNode, resultOperand, intrinsic, registerOffset, argumentCountIncludingThis, prediction, insertChecksWithAccounting)) {
                RELEASE_ASSERT(didInsertChecks);
                addToGraph(Phantom, callTargetNode);
                emitArgumentPhantoms(registerOffset, argumentCountIncludingThis);
                inliningBalance--;
                return true;
            }

            RELEASE_ASSERT(!didInsertChecks);
            // We might still try to inline the Intrinsic because it might be a builtin JS function.
        }

        if (Options::useDOMJIT()) {
            if (const DOMJIT::Signature* signature = callee.signatureFor(specializationKind)) {
                if (handleDOMJITCall(callTargetNode, resultOperand, signature, registerOffset, argumentCountIncludingThis, prediction, insertChecksWithAccounting)) {
                    RELEASE_ASSERT(didInsertChecks);
                    addToGraph(Phantom, callTargetNode);
                    emitArgumentPhantoms(registerOffset, argumentCountIncludingThis);
                    inliningBalance--;
                    return true;
                }
                RELEASE_ASSERT(!didInsertChecks);
            }
        }
    }
    
    unsigned myInliningCost = inliningCost(callee, argumentCountIncludingThis, InlineCallFrame::callModeFor(kind));
    if (myInliningCost > inliningBalance)
        return false;

    Instruction* savedCurrentInstruction = m_currentInstruction;
    inlineCall(callTargetNode, resultOperand, callee, registerOffset, argumentCountIncludingThis, nextOffset, kind, callerLinkability, insertChecks);
    inliningBalance -= myInliningCost;
    m_currentInstruction = savedCurrentInstruction;
    return true;
}

bool ByteCodeParser::handleInlining(
    Node* callTargetNode, int resultOperand, const CallLinkStatus& callLinkStatus,
    int registerOffsetOrFirstFreeReg, VirtualRegister thisArgument,
    VirtualRegister argumentsArgument, unsigned argumentsOffset, int argumentCountIncludingThis,
    unsigned nextOffset, NodeType callOp, InlineCallFrame::Kind kind, SpeculatedType prediction)
{
    if (verbose) {
        dataLog("Handling inlining...\n");
        dataLog("Stack: ", currentCodeOrigin(), "\n");
    }
    CodeSpecializationKind specializationKind = InlineCallFrame::specializationKindFor(kind);
    
    if (!callLinkStatus.size()) {
        if (verbose)
            dataLog("Bailing inlining.\n");
        return false;
    }
    
    if (InlineCallFrame::isVarargs(kind)
        && callLinkStatus.maxNumArguments() > Options::maximumVarargsForInlining()) {
        if (verbose)
            dataLog("Bailing inlining because of varargs.\n");
        return false;
    }
        
    unsigned inliningBalance = Options::maximumFunctionForCallInlineCandidateInstructionCount();
    if (specializationKind == CodeForConstruct)
        inliningBalance = std::min(inliningBalance, Options::maximumFunctionForConstructInlineCandidateInstructionCount());
    if (callLinkStatus.isClosureCall())
        inliningBalance = std::min(inliningBalance, Options::maximumFunctionForClosureCallInlineCandidateInstructionCount());
    
    // First check if we can avoid creating control flow. Our inliner does some CFG
    // simplification on the fly and this helps reduce compile times, but we can only leverage
    // this in cases where we don't need control flow diamonds to check the callee.
    if (!callLinkStatus.couldTakeSlowPath() && callLinkStatus.size() == 1) {
        int registerOffset;
        
        // Only used for varargs calls.
        unsigned mandatoryMinimum = 0;
        unsigned maxNumArguments = 0;

        if (InlineCallFrame::isVarargs(kind)) {
            if (FunctionExecutable* functionExecutable = callLinkStatus[0].functionExecutable())
                mandatoryMinimum = functionExecutable->parameterCount();
            else
                mandatoryMinimum = 0;
            
            // includes "this"
            maxNumArguments = std::max(
                callLinkStatus.maxNumArguments(),
                mandatoryMinimum + 1);
            
            // We sort of pretend that this *is* the number of arguments that were passed.
            argumentCountIncludingThis = maxNumArguments;
            
            registerOffset = registerOffsetOrFirstFreeReg + 1;
            registerOffset -= maxNumArguments; // includes "this"
            registerOffset -= CallFrame::headerSizeInRegisters;
            registerOffset = -WTF::roundUpToMultipleOf(
                stackAlignmentRegisters(),
                -registerOffset);
        } else
            registerOffset = registerOffsetOrFirstFreeReg;
        
        bool result = attemptToInlineCall(
            callTargetNode, resultOperand, callLinkStatus[0], registerOffset,
            argumentCountIncludingThis, nextOffset, kind, CallerDoesNormalLinking, prediction,
            inliningBalance, [&] (CodeBlock* codeBlock) {
                emitFunctionChecks(callLinkStatus[0], callTargetNode, thisArgument);

                // If we have a varargs call, we want to extract the arguments right now.
                if (InlineCallFrame::isVarargs(kind)) {
                    int remappedRegisterOffset =
                        m_inlineStackTop->remapOperand(VirtualRegister(registerOffset)).offset();
                    
                    ensureLocals(VirtualRegister(remappedRegisterOffset).toLocal());
                    
                    int argumentStart = registerOffset + CallFrame::headerSizeInRegisters;
                    int remappedArgumentStart =
                        m_inlineStackTop->remapOperand(VirtualRegister(argumentStart)).offset();

                    LoadVarargsData* data = m_graph.m_loadVarargsData.add();
                    data->start = VirtualRegister(remappedArgumentStart + 1);
                    data->count = VirtualRegister(remappedRegisterOffset + CallFrameSlot::argumentCount);
                    data->offset = argumentsOffset;
                    data->limit = maxNumArguments;
                    data->mandatoryMinimum = mandatoryMinimum;

                    if (callOp == TailCallForwardVarargs)
                        addToGraph(ForwardVarargs, OpInfo(data));
                    else
                        addToGraph(LoadVarargs, OpInfo(data), get(argumentsArgument));

                    // LoadVarargs may OSR exit. Hence, we need to keep alive callTargetNode, thisArgument
                    // and argumentsArgument for the baseline JIT. However, we only need a Phantom for
                    // callTargetNode because the other 2 are still in use and alive at this point.
                    addToGraph(Phantom, callTargetNode);

                    // In DFG IR before SSA, we cannot insert control flow between after the
                    // LoadVarargs and the last SetArgument. This isn't a problem once we get to DFG
                    // SSA. Fortunately, we also have other reasons for not inserting control flow
                    // before SSA.
            
                    VariableAccessData* countVariable = newVariableAccessData(
                        VirtualRegister(remappedRegisterOffset + CallFrameSlot::argumentCount));
                    // This is pretty lame, but it will force the count to be flushed as an int. This doesn't
                    // matter very much, since our use of a SetArgument and Flushes for this local slot is
                    // mostly just a formality.
                    countVariable->predict(SpecInt32Only);
                    countVariable->mergeIsProfitableToUnbox(true);
                    Node* setArgumentCount = addToGraph(SetArgument, OpInfo(countVariable));
                    m_currentBlock->variablesAtTail.setOperand(countVariable->local(), setArgumentCount);

                    set(VirtualRegister(argumentStart), get(thisArgument), ImmediateNakedSet);
                    for (unsigned argument = 1; argument < maxNumArguments; ++argument) {
                        VariableAccessData* variable = newVariableAccessData(
                            VirtualRegister(remappedArgumentStart + argument));
                        variable->mergeShouldNeverUnbox(true); // We currently have nowhere to put the type check on the LoadVarargs. LoadVarargs is effectful, so after it finishes, we cannot exit.
                        
                        // For a while it had been my intention to do things like this inside the
                        // prediction injection phase. But in this case it's really best to do it here,
                        // because it's here that we have access to the variable access datas for the
                        // inlining we're about to do.
                        //
                        // Something else that's interesting here is that we'd really love to get
                        // predictions from the arguments loaded at the callsite, rather than the
                        // arguments received inside the callee. But that probably won't matter for most
                        // calls.
                        if (codeBlock && argument < static_cast<unsigned>(codeBlock->numParameters())) {
                            ConcurrentJSLocker locker(codeBlock->m_lock);
                            if (ValueProfile* profile = codeBlock->valueProfileForArgument(argument))
                                variable->predict(profile->computeUpdatedPrediction(locker));
                        }
                        
                        Node* setArgument = addToGraph(SetArgument, OpInfo(variable));
                        m_currentBlock->variablesAtTail.setOperand(variable->local(), setArgument);
                    }
                }
            });
        if (verbose) {
            dataLog("Done inlining (simple).\n");
            dataLog("Stack: ", currentCodeOrigin(), "\n");
            dataLog("Result: ", result, "\n");
        }
        return result;
    }
    
    // We need to create some kind of switch over callee. For now we only do this if we believe that
    // we're in the top tier. We have two reasons for this: first, it provides us an opportunity to
    // do more detailed polyvariant/polymorphic profiling; and second, it reduces compile times in
    // the DFG. And by polyvariant profiling we mean polyvariant profiling of *this* call. Note that
    // we could improve that aspect of this by doing polymorphic inlining but having the profiling
    // also.
    if (!isFTL(m_graph.m_plan.mode) || !Options::usePolymorphicCallInlining()
        || InlineCallFrame::isVarargs(kind)) {
        if (verbose) {
            dataLog("Bailing inlining (hard).\n");
            dataLog("Stack: ", currentCodeOrigin(), "\n");
        }
        return false;
    }
    
    // If the claim is that this did not originate from a stub, then we don't want to emit a switch
    // statement. Whenever the non-stub profiling says that it could take slow path, it really means that
    // it has no idea.
    if (!Options::usePolymorphicCallInliningForNonStubStatus()
        && !callLinkStatus.isBasedOnStub()) {
        if (verbose) {
            dataLog("Bailing inlining (non-stub polymorphism).\n");
            dataLog("Stack: ", currentCodeOrigin(), "\n");
        }
        return false;
    }
    
    unsigned oldOffset = m_currentIndex;
    
    bool allAreClosureCalls = true;
    bool allAreDirectCalls = true;
    for (unsigned i = callLinkStatus.size(); i--;) {
        if (callLinkStatus[i].isClosureCall())
            allAreDirectCalls = false;
        else
            allAreClosureCalls = false;
    }
    
    Node* thingToSwitchOn;
    if (allAreDirectCalls)
        thingToSwitchOn = callTargetNode;
    else if (allAreClosureCalls)
        thingToSwitchOn = addToGraph(GetExecutable, callTargetNode);
    else {
        // FIXME: We should be able to handle this case, but it's tricky and we don't know of cases
        // where it would be beneficial. It might be best to handle these cases as if all calls were
        // closure calls.
        // https://bugs.webkit.org/show_bug.cgi?id=136020
        if (verbose) {
            dataLog("Bailing inlining (mix).\n");
            dataLog("Stack: ", currentCodeOrigin(), "\n");
        }
        return false;
    }
    
    if (verbose) {
        dataLog("Doing hard inlining...\n");
        dataLog("Stack: ", currentCodeOrigin(), "\n");
    }
    
    int registerOffset = registerOffsetOrFirstFreeReg;
    
    // This makes me wish that we were in SSA all the time. We need to pick a variable into which to
    // store the callee so that it will be accessible to all of the blocks we're about to create. We
    // get away with doing an immediate-set here because we wouldn't have performed any side effects
    // yet.
    if (verbose)
        dataLog("Register offset: ", registerOffset);
    VirtualRegister calleeReg(registerOffset + CallFrameSlot::callee);
    calleeReg = m_inlineStackTop->remapOperand(calleeReg);
    if (verbose)
        dataLog("Callee is going to be ", calleeReg, "\n");
    setDirect(calleeReg, callTargetNode, ImmediateSetWithFlush);

    // It's OK to exit right now, even though we set some locals. That's because those locals are not
    // user-visible.
    m_exitOK = true;
    addToGraph(ExitOK);
    
    SwitchData& data = *m_graph.m_switchData.add();
    data.kind = SwitchCell;
    addToGraph(Switch, OpInfo(&data), thingToSwitchOn);
    
    BasicBlock* originBlock = m_currentBlock;
    if (verbose)
        dataLog("Marking ", RawPointer(originBlock), " as linked (origin of poly inline)\n");
    originBlock->didLink();
    cancelLinkingForBlock(m_inlineStackTop, originBlock);
    
    // Each inlined callee will have a landing block that it returns at. They should all have jumps
    // to the continuation block, which we create last.
    Vector<BasicBlock*> landingBlocks;
    
    // We may force this true if we give up on inlining any of the edges.
    bool couldTakeSlowPath = callLinkStatus.couldTakeSlowPath();
    
    if (verbose)
        dataLog("About to loop over functions at ", currentCodeOrigin(), ".\n");
    
    for (unsigned i = 0; i < callLinkStatus.size(); ++i) {
        m_currentIndex = oldOffset;
        Ref<BasicBlock> block = adoptRef(*new BasicBlock(UINT_MAX, m_numArguments, m_numLocals, 1));
        m_currentBlock = block.ptr();
        m_graph.appendBlock(block.copyRef());
        prepareToParseBlock();
        
        Node* myCallTargetNode = getDirect(calleeReg);
        
        bool inliningResult = attemptToInlineCall(
            myCallTargetNode, resultOperand, callLinkStatus[i], registerOffset,
            argumentCountIncludingThis, nextOffset, kind, CallerLinksManually, prediction,
            inliningBalance, [&] (CodeBlock*) { });
        
        if (!inliningResult) {
            // That failed so we let the block die. Nothing interesting should have been added to
            // the block. We also give up on inlining any of the (less frequent) callees.
            ASSERT(m_currentBlock == block.ptr());
            ASSERT(m_graph.m_blocks.last() == block.ptr());
            m_graph.killBlockAndItsContents(block.ptr());
            m_graph.m_blocks.removeLast();
            
            // The fact that inlining failed means we need a slow path.
            couldTakeSlowPath = true;
            break;
        }
        
        JSCell* thingToCaseOn;
        if (allAreDirectCalls)
            thingToCaseOn = callLinkStatus[i].nonExecutableCallee();
        else {
            ASSERT(allAreClosureCalls);
            thingToCaseOn = callLinkStatus[i].executable();
        }
        data.cases.append(SwitchCase(m_graph.freeze(thingToCaseOn), block.ptr()));
        m_currentIndex = nextOffset;
        m_exitOK = true;
        processSetLocalQueue(); // This only comes into play for intrinsics, since normal inlined code will leave an empty queue.
        if (Node* terminal = m_currentBlock->terminal())
            ASSERT_UNUSED(terminal, terminal->op() == TailCall || terminal->op() == TailCallVarargs || terminal->op() == TailCallForwardVarargs);
        else {
            addToGraph(Jump);
            landingBlocks.append(m_currentBlock);
        }
        if (verbose)
            dataLog("Marking ", RawPointer(m_currentBlock), " as linked (tail of poly inlinee)\n");
        m_currentBlock->didLink();

        if (verbose)
            dataLog("Finished inlining ", callLinkStatus[i], " at ", currentCodeOrigin(), ".\n");
    }
    
    Ref<BasicBlock> slowPathBlock = adoptRef(
        *new BasicBlock(UINT_MAX, m_numArguments, m_numLocals, 1));
    m_currentIndex = oldOffset;
    m_exitOK = true;
    data.fallThrough = BranchTarget(slowPathBlock.ptr());
    m_graph.appendBlock(slowPathBlock.copyRef());
    if (verbose)
        dataLog("Marking ", RawPointer(slowPathBlock.ptr()), " as linked (slow path block)\n");
    slowPathBlock->didLink();
    prepareToParseBlock();
    m_currentBlock = slowPathBlock.ptr();
    Node* myCallTargetNode = getDirect(calleeReg);
    if (couldTakeSlowPath) {
        addCall(
            resultOperand, callOp, nullptr, myCallTargetNode, argumentCountIncludingThis,
            registerOffset, prediction);
    } else {
        addToGraph(CheckBadCell);
        addToGraph(Phantom, myCallTargetNode);
        emitArgumentPhantoms(registerOffset, argumentCountIncludingThis);
        
        set(VirtualRegister(resultOperand), addToGraph(BottomValue));
    }

    m_currentIndex = nextOffset;
    m_exitOK = true; // Origin changed, so it's fine to exit again.
    processSetLocalQueue();
    if (Node* terminal = m_currentBlock->terminal())
        ASSERT_UNUSED(terminal, terminal->op() == TailCall || terminal->op() == TailCallVarargs || terminal->op() == TailCallForwardVarargs);
    else {
        addToGraph(Jump);
        landingBlocks.append(m_currentBlock);
    }
    
    Ref<BasicBlock> continuationBlock = adoptRef(
        *new BasicBlock(UINT_MAX, m_numArguments, m_numLocals, 1));
    m_graph.appendBlock(continuationBlock.copyRef());
    if (verbose)
        dataLog("Adding unlinked block ", RawPointer(continuationBlock.ptr()), " (continuation)\n");
    m_inlineStackTop->m_unlinkedBlocks.append(UnlinkedBlock(continuationBlock.ptr()));
    prepareToParseBlock();
    m_currentBlock = continuationBlock.ptr();
    
    for (unsigned i = landingBlocks.size(); i--;)
        landingBlocks[i]->terminal()->targetBlock() = continuationBlock.ptr();
    
    m_currentIndex = oldOffset;
    m_exitOK = true;
    
    if (verbose) {
        dataLog("Done inlining (hard).\n");
        dataLog("Stack: ", currentCodeOrigin(), "\n");
    }
    return true;
}

template<typename ChecksFunctor>
bool ByteCodeParser::handleMinMax(int resultOperand, NodeType op, int registerOffset, int argumentCountIncludingThis, const ChecksFunctor& insertChecks)
{
    ASSERT(op == ArithMin || op == ArithMax);

    if (argumentCountIncludingThis == 1) {
        insertChecks();
        double result = op == ArithMax ? -std::numeric_limits<double>::infinity() : +std::numeric_limits<double>::infinity();
        set(VirtualRegister(resultOperand), addToGraph(JSConstant, OpInfo(m_graph.freeze(jsDoubleNumber(result)))));
        return true;
    }
     
    if (argumentCountIncludingThis == 2) {
        insertChecks();
        Node* result = get(VirtualRegister(virtualRegisterForArgument(1, registerOffset)));
        addToGraph(Phantom, Edge(result, NumberUse));
        set(VirtualRegister(resultOperand), result);
        return true;
    }
    
    if (argumentCountIncludingThis == 3) {
        insertChecks();
        set(VirtualRegister(resultOperand), addToGraph(op, get(virtualRegisterForArgument(1, registerOffset)), get(virtualRegisterForArgument(2, registerOffset))));
        return true;
    }
    
    // Don't handle >=3 arguments for now.
    return false;
}

template<typename ChecksFunctor>
bool ByteCodeParser::handleIntrinsicCall(Node* callee, int resultOperand, Intrinsic intrinsic, int registerOffset, int argumentCountIncludingThis, SpeculatedType prediction, const ChecksFunctor& insertChecks)
{
    switch (intrinsic) {

    // Intrinsic Functions:

    case AbsIntrinsic: {
        if (argumentCountIncludingThis == 1) { // Math.abs()
            insertChecks();
            set(VirtualRegister(resultOperand), addToGraph(JSConstant, OpInfo(m_constantNaN)));
            return true;
        }

        if (!MacroAssembler::supportsFloatingPointAbs())
            return false;

        insertChecks();
        Node* node = addToGraph(ArithAbs, get(virtualRegisterForArgument(1, registerOffset)));
        if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, Overflow))
            node->mergeFlags(NodeMayOverflowInt32InDFG);
        set(VirtualRegister(resultOperand), node);
        return true;
    }

    case MinIntrinsic:
        return handleMinMax(resultOperand, ArithMin, registerOffset, argumentCountIncludingThis, insertChecks);
        
    case MaxIntrinsic:
        return handleMinMax(resultOperand, ArithMax, registerOffset, argumentCountIncludingThis, insertChecks);

    case CosIntrinsic:
    case FRoundIntrinsic:
    case LogIntrinsic:
    case SinIntrinsic:
    case SqrtIntrinsic:
    case TanIntrinsic: {
        if (argumentCountIncludingThis == 1) {
            insertChecks();
            set(VirtualRegister(resultOperand), addToGraph(JSConstant, OpInfo(m_constantNaN)));
            return true;
        }

        NodeType nodeType = Unreachable;
        switch (intrinsic) {
        case CosIntrinsic:
            nodeType = ArithCos;
            break;
        case FRoundIntrinsic:
            nodeType = ArithFRound;
            break;
        case LogIntrinsic:
            nodeType = ArithLog;
            break;
        case SinIntrinsic:
            nodeType = ArithSin;
            break;
        case SqrtIntrinsic:
            nodeType = ArithSqrt;
            break;
        case TanIntrinsic:
            nodeType = ArithTan;
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
        insertChecks();
        set(VirtualRegister(resultOperand), addToGraph(nodeType, get(virtualRegisterForArgument(1, registerOffset))));
        return true;
    }

    case PowIntrinsic: {
        if (argumentCountIncludingThis < 3) {
            // Math.pow() and Math.pow(x) return NaN.
            insertChecks();
            set(VirtualRegister(resultOperand), addToGraph(JSConstant, OpInfo(m_constantNaN)));
            return true;
        }
        insertChecks();
        VirtualRegister xOperand = virtualRegisterForArgument(1, registerOffset);
        VirtualRegister yOperand = virtualRegisterForArgument(2, registerOffset);
        set(VirtualRegister(resultOperand), addToGraph(ArithPow, get(xOperand), get(yOperand)));
        return true;
    }
        
    case ArrayPushIntrinsic: {
        if (argumentCountIncludingThis != 2)
            return false;
        
        ArrayMode arrayMode = getArrayMode(m_currentInstruction[OPCODE_LENGTH(op_call) - 2].u.arrayProfile);
        if (!arrayMode.isJSArray())
            return false;
        switch (arrayMode.type()) {
        case Array::Int32:
        case Array::Double:
        case Array::Contiguous:
        case Array::ArrayStorage: {
            insertChecks();
            Node* arrayPush = addToGraph(ArrayPush, OpInfo(arrayMode.asWord()), OpInfo(prediction), get(virtualRegisterForArgument(0, registerOffset)), get(virtualRegisterForArgument(1, registerOffset)));
            set(VirtualRegister(resultOperand), arrayPush);
            
            return true;
        }
            
        default:
            return false;
        }
    }

    case ArraySliceIntrinsic: {
#if USE(JSVALUE32_64)
        if (isX86()) {
            // There aren't enough registers for this to be done easily.
            return false;
        }
#endif
        if (argumentCountIncludingThis < 2)
            return false;

        if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadConstantCache)
            || m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadCache))
            return false;

        ArrayMode arrayMode = getArrayMode(m_currentInstruction[OPCODE_LENGTH(op_call) - 2].u.arrayProfile);
        if (!arrayMode.isJSArray())
            return false;

        if (arrayMode.arrayClass() != Array::OriginalArray)
            return false;

        switch (arrayMode.type()) {
        case Array::Double:
        case Array::Int32:
        case Array::Contiguous: {
            JSGlobalObject* globalObject = m_graph.globalObjectFor(currentNodeOrigin().semantic);

            InlineWatchpointSet& objectPrototypeTransition = globalObject->objectPrototype()->structure()->transitionWatchpointSet();
            InlineWatchpointSet& arrayPrototypeTransition = globalObject->arrayPrototype()->structure()->transitionWatchpointSet();

            // FIXME: We could easily relax the Array/Object.prototype transition as long as we OSR exitted if we saw a hole.
            if (globalObject->arraySpeciesWatchpoint().state() == IsWatched
                && globalObject->havingABadTimeWatchpoint()->isStillValid()
                && arrayPrototypeTransition.isStillValid()
                && objectPrototypeTransition.isStillValid()
                && globalObject->arrayPrototypeChainIsSane()) {

                m_graph.watchpoints().addLazily(globalObject->arraySpeciesWatchpoint());
                m_graph.watchpoints().addLazily(globalObject->havingABadTimeWatchpoint());
                m_graph.watchpoints().addLazily(arrayPrototypeTransition);
                m_graph.watchpoints().addLazily(objectPrototypeTransition);

                insertChecks();

                Node* array = get(virtualRegisterForArgument(0, registerOffset));
                // We do a few things here to prove that we aren't skipping doing side-effects in an observable way:
                // 1. We ensure that the "constructor" property hasn't been changed (because the observable
                // effects of slice require that we perform a Get(array, "constructor") and we can skip
                // that if we're an original array structure. (We can relax this in the future by using
                // TryGetById and CheckCell).
                //
                // 2. We check that the array we're calling slice on has the same global object as the lexical
                // global object that this code is running in. This requirement is necessary because we setup the
                // watchpoints above on the lexical global object. This means that code that calls slice on
                // arrays produced by other global objects won't get this optimization. We could relax this
                // requirement in the future by checking that the watchpoint hasn't fired at runtime in the code
                // we generate instead of registering it as a watchpoint that would invalidate the compilation.
                //
                // 3. By proving we're an original array structure, we guarantee that the incoming array
                // isn't a subclass of Array.

                StructureSet structureSet;
                structureSet.add(globalObject->originalArrayStructureForIndexingType(ArrayWithInt32));
                structureSet.add(globalObject->originalArrayStructureForIndexingType(ArrayWithContiguous));
                structureSet.add(globalObject->originalArrayStructureForIndexingType(ArrayWithDouble));
                addToGraph(CheckStructure, OpInfo(m_graph.addStructureSet(structureSet)), array);

                addVarArgChild(array);
                addVarArgChild(get(virtualRegisterForArgument(1, registerOffset))); // Start index.
                if (argumentCountIncludingThis >= 3)
                    addVarArgChild(get(virtualRegisterForArgument(2, registerOffset))); // End index.
                addVarArgChild(addToGraph(GetButterfly, array));

                Node* arraySlice = addToGraph(Node::VarArg, ArraySlice, OpInfo(), OpInfo());
                set(VirtualRegister(resultOperand), arraySlice);
                return true;
            }

            return false;
        }
        default:
            return false;
        }

        RELEASE_ASSERT_NOT_REACHED();
        return false;
    }
        
    case ArrayPopIntrinsic: {
        if (argumentCountIncludingThis != 1)
            return false;
        
        ArrayMode arrayMode = getArrayMode(m_currentInstruction[OPCODE_LENGTH(op_call) - 2].u.arrayProfile);
        if (!arrayMode.isJSArray())
            return false;
        switch (arrayMode.type()) {
        case Array::Int32:
        case Array::Double:
        case Array::Contiguous:
        case Array::ArrayStorage: {
            insertChecks();
            Node* arrayPop = addToGraph(ArrayPop, OpInfo(arrayMode.asWord()), OpInfo(prediction), get(virtualRegisterForArgument(0, registerOffset)));
            set(VirtualRegister(resultOperand), arrayPop);
            return true;
        }
            
        default:
            return false;
        }
    }

    case ParseIntIntrinsic: {
        if (argumentCountIncludingThis < 2)
            return false;

        if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadCell) || m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadType))
            return false;

        insertChecks();
        VirtualRegister valueOperand = virtualRegisterForArgument(1, registerOffset);
        Node* parseInt;
        if (argumentCountIncludingThis == 2)
            parseInt = addToGraph(ParseInt, OpInfo(), OpInfo(prediction), get(valueOperand));
        else {
            ASSERT(argumentCountIncludingThis > 2);
            VirtualRegister radixOperand = virtualRegisterForArgument(2, registerOffset);
            parseInt = addToGraph(ParseInt, OpInfo(), OpInfo(prediction), get(valueOperand), get(radixOperand));
        }
        set(VirtualRegister(resultOperand), parseInt);
        return true;
    }

    case CharCodeAtIntrinsic: {
        if (argumentCountIncludingThis != 2)
            return false;

        insertChecks();
        VirtualRegister thisOperand = virtualRegisterForArgument(0, registerOffset);
        VirtualRegister indexOperand = virtualRegisterForArgument(1, registerOffset);
        Node* charCode = addToGraph(StringCharCodeAt, OpInfo(ArrayMode(Array::String).asWord()), get(thisOperand), get(indexOperand));

        set(VirtualRegister(resultOperand), charCode);
        return true;
    }

    case CharAtIntrinsic: {
        if (argumentCountIncludingThis != 2)
            return false;

        insertChecks();
        VirtualRegister thisOperand = virtualRegisterForArgument(0, registerOffset);
        VirtualRegister indexOperand = virtualRegisterForArgument(1, registerOffset);
        Node* charCode = addToGraph(StringCharAt, OpInfo(ArrayMode(Array::String).asWord()), get(thisOperand), get(indexOperand));

        set(VirtualRegister(resultOperand), charCode);
        return true;
    }
    case Clz32Intrinsic: {
        insertChecks();
        if (argumentCountIncludingThis == 1)
            set(VirtualRegister(resultOperand), addToGraph(JSConstant, OpInfo(m_graph.freeze(jsNumber(32)))));
        else {
            Node* operand = get(virtualRegisterForArgument(1, registerOffset));
            set(VirtualRegister(resultOperand), addToGraph(ArithClz32, operand));
        }
        return true;
    }
    case FromCharCodeIntrinsic: {
        if (argumentCountIncludingThis != 2)
            return false;

        insertChecks();
        VirtualRegister indexOperand = virtualRegisterForArgument(1, registerOffset);
        Node* charCode = addToGraph(StringFromCharCode, get(indexOperand));

        set(VirtualRegister(resultOperand), charCode);

        return true;
    }

    case RegExpExecIntrinsic: {
        if (argumentCountIncludingThis != 2)
            return false;
        
        insertChecks();
        Node* regExpExec = addToGraph(RegExpExec, OpInfo(0), OpInfo(prediction), addToGraph(GetGlobalObject, callee), get(virtualRegisterForArgument(0, registerOffset)), get(virtualRegisterForArgument(1, registerOffset)));
        set(VirtualRegister(resultOperand), regExpExec);
        
        return true;
    }
        
    case RegExpTestIntrinsic:
    case RegExpTestFastIntrinsic: {
        if (argumentCountIncludingThis != 2)
            return false;

        if (intrinsic == RegExpTestIntrinsic) {
            // Don't inline intrinsic if we exited due to one of the primordial RegExp checks failing.
            if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadCell))
                return false;

            JSGlobalObject* globalObject = m_inlineStackTop->m_codeBlock->globalObject();
            Structure* regExpStructure = globalObject->regExpStructure();
            m_graph.registerStructure(regExpStructure);
            ASSERT(regExpStructure->storedPrototype().isObject());
            ASSERT(regExpStructure->storedPrototype().asCell()->classInfo(*m_vm) == RegExpPrototype::info());

            FrozenValue* regExpPrototypeObjectValue = m_graph.freeze(regExpStructure->storedPrototype());
            Structure* regExpPrototypeStructure = regExpPrototypeObjectValue->structure();

            auto isRegExpPropertySame = [&] (JSValue primordialProperty, UniquedStringImpl* propertyUID) {
                JSValue currentProperty;
                if (!m_graph.getRegExpPrototypeProperty(regExpStructure->storedPrototypeObject(), regExpPrototypeStructure, propertyUID, currentProperty))
                    return false;
                
                return currentProperty == primordialProperty;
            };

            // Check that RegExp.exec is still the primordial RegExp.prototype.exec
            if (!isRegExpPropertySame(globalObject->regExpProtoExecFunction(), m_vm->propertyNames->exec.impl()))
                return false;

            // Check that regExpObject is actually a RegExp object.
            Node* regExpObject = get(virtualRegisterForArgument(0, registerOffset));
            addToGraph(Check, Edge(regExpObject, RegExpObjectUse));

            // Check that regExpObject's exec is actually the primodial RegExp.prototype.exec.
            UniquedStringImpl* execPropertyID = m_vm->propertyNames->exec.impl();
            unsigned execIndex = m_graph.identifiers().ensure(execPropertyID);
            Node* actualProperty = addToGraph(TryGetById, OpInfo(execIndex), OpInfo(SpecFunction), Edge(regExpObject, CellUse));
            FrozenValue* regExpPrototypeExec = m_graph.freeze(globalObject->regExpProtoExecFunction());
            addToGraph(CheckCell, OpInfo(regExpPrototypeExec), Edge(actualProperty, CellUse));
        }

        insertChecks();
        Node* regExpObject = get(virtualRegisterForArgument(0, registerOffset));
        Node* regExpExec = addToGraph(RegExpTest, OpInfo(0), OpInfo(prediction), addToGraph(GetGlobalObject, callee), regExpObject, get(virtualRegisterForArgument(1, registerOffset)));
        set(VirtualRegister(resultOperand), regExpExec);
        
        return true;
    }

    case IsTypedArrayViewIntrinsic: {
        ASSERT(argumentCountIncludingThis == 2);

        insertChecks();
        set(VirtualRegister(resultOperand), addToGraph(IsTypedArrayView, OpInfo(prediction), get(virtualRegisterForArgument(1, registerOffset))));
        return true;
    }

    case StringPrototypeReplaceIntrinsic: {
        if (argumentCountIncludingThis != 3)
            return false;

        // Don't inline intrinsic if we exited due to "search" not being a RegExp or String object.
        if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadType))
            return false;

        // Don't inline intrinsic if we exited due to one of the primordial RegExp checks failing.
        if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadCell))
            return false;

        JSGlobalObject* globalObject = m_inlineStackTop->m_codeBlock->globalObject();
        Structure* regExpStructure = globalObject->regExpStructure();
        m_graph.registerStructure(regExpStructure);
        ASSERT(regExpStructure->storedPrototype().isObject());
        ASSERT(regExpStructure->storedPrototype().asCell()->classInfo(*m_vm) == RegExpPrototype::info());

        FrozenValue* regExpPrototypeObjectValue = m_graph.freeze(regExpStructure->storedPrototype());
        Structure* regExpPrototypeStructure = regExpPrototypeObjectValue->structure();

        auto isRegExpPropertySame = [&] (JSValue primordialProperty, UniquedStringImpl* propertyUID) {
            JSValue currentProperty;
            if (!m_graph.getRegExpPrototypeProperty(regExpStructure->storedPrototypeObject(), regExpPrototypeStructure, propertyUID, currentProperty))
                return false;

            return currentProperty == primordialProperty;
        };

        // Check that searchRegExp.exec is still the primordial RegExp.prototype.exec
        if (!isRegExpPropertySame(globalObject->regExpProtoExecFunction(), m_vm->propertyNames->exec.impl()))
            return false;

        // Check that searchRegExp.global is still the primordial RegExp.prototype.global
        if (!isRegExpPropertySame(globalObject->regExpProtoGlobalGetter(), m_vm->propertyNames->global.impl()))
            return false;

        // Check that searchRegExp.unicode is still the primordial RegExp.prototype.unicode
        if (!isRegExpPropertySame(globalObject->regExpProtoUnicodeGetter(), m_vm->propertyNames->unicode.impl()))
            return false;

        // Check that searchRegExp[Symbol.match] is still the primordial RegExp.prototype[Symbol.replace]
        if (!isRegExpPropertySame(globalObject->regExpProtoSymbolReplaceFunction(), m_vm->propertyNames->replaceSymbol.impl()))
            return false;

        insertChecks();

        Node* result = addToGraph(StringReplace, OpInfo(0), OpInfo(prediction), get(virtualRegisterForArgument(0, registerOffset)), get(virtualRegisterForArgument(1, registerOffset)), get(virtualRegisterForArgument(2, registerOffset)));
        set(VirtualRegister(resultOperand), result);
        return true;
    }
        
    case StringPrototypeReplaceRegExpIntrinsic: {
        if (argumentCountIncludingThis != 3)
            return false;
        
        insertChecks();
        Node* result = addToGraph(StringReplaceRegExp, OpInfo(0), OpInfo(prediction), get(virtualRegisterForArgument(0, registerOffset)), get(virtualRegisterForArgument(1, registerOffset)), get(virtualRegisterForArgument(2, registerOffset)));
        set(VirtualRegister(resultOperand), result);
        return true;
    }
        
    case RoundIntrinsic:
    case FloorIntrinsic:
    case CeilIntrinsic:
    case TruncIntrinsic: {
        if (argumentCountIncludingThis == 1) {
            insertChecks();
            set(VirtualRegister(resultOperand), addToGraph(JSConstant, OpInfo(m_constantNaN)));
            return true;
        }
        insertChecks();
        Node* operand = get(virtualRegisterForArgument(1, registerOffset));
        NodeType op;
        if (intrinsic == RoundIntrinsic)
            op = ArithRound;
        else if (intrinsic == FloorIntrinsic)
            op = ArithFloor;
        else if (intrinsic == CeilIntrinsic)
            op = ArithCeil;
        else {
            ASSERT(intrinsic == TruncIntrinsic);
            op = ArithTrunc;
        }
        Node* roundNode = addToGraph(op, OpInfo(0), OpInfo(prediction), operand);
        set(VirtualRegister(resultOperand), roundNode);
        return true;
    }
    case IMulIntrinsic: {
        if (argumentCountIncludingThis != 3)
            return false;
        insertChecks();
        VirtualRegister leftOperand = virtualRegisterForArgument(1, registerOffset);
        VirtualRegister rightOperand = virtualRegisterForArgument(2, registerOffset);
        Node* left = get(leftOperand);
        Node* right = get(rightOperand);
        set(VirtualRegister(resultOperand), addToGraph(ArithIMul, left, right));
        return true;
    }

    case RandomIntrinsic: {
        if (argumentCountIncludingThis != 1)
            return false;
        insertChecks();
        set(VirtualRegister(resultOperand), addToGraph(ArithRandom));
        return true;
    }
        
    case DFGTrueIntrinsic: {
        insertChecks();
        set(VirtualRegister(resultOperand), jsConstant(jsBoolean(true)));
        return true;
    }
        
    case OSRExitIntrinsic: {
        insertChecks();
        addToGraph(ForceOSRExit);
        set(VirtualRegister(resultOperand), addToGraph(JSConstant, OpInfo(m_constantUndefined)));
        return true;
    }
        
    case IsFinalTierIntrinsic: {
        insertChecks();
        set(VirtualRegister(resultOperand),
            jsConstant(jsBoolean(Options::useFTLJIT() ? isFTL(m_graph.m_plan.mode) : true)));
        return true;
    }
        
    case SetInt32HeapPredictionIntrinsic: {
        insertChecks();
        for (int i = 1; i < argumentCountIncludingThis; ++i) {
            Node* node = get(virtualRegisterForArgument(i, registerOffset));
            if (node->hasHeapPrediction())
                node->setHeapPrediction(SpecInt32Only);
        }
        set(VirtualRegister(resultOperand), addToGraph(JSConstant, OpInfo(m_constantUndefined)));
        return true;
    }
        
    case CheckInt32Intrinsic: {
        insertChecks();
        for (int i = 1; i < argumentCountIncludingThis; ++i) {
            Node* node = get(virtualRegisterForArgument(i, registerOffset));
            addToGraph(Phantom, Edge(node, Int32Use));
        }
        set(VirtualRegister(resultOperand), jsConstant(jsBoolean(true)));
        return true;
    }
        
    case FiatInt52Intrinsic: {
        if (argumentCountIncludingThis != 2)
            return false;
        insertChecks();
        VirtualRegister operand = virtualRegisterForArgument(1, registerOffset);
        if (enableInt52())
            set(VirtualRegister(resultOperand), addToGraph(FiatInt52, get(operand)));
        else
            set(VirtualRegister(resultOperand), get(operand));
        return true;
    }

    case JSMapGetIntrinsic: {
        if (argumentCountIncludingThis != 2)
            return false;

        insertChecks();
        Node* map = get(virtualRegisterForArgument(0, registerOffset));
        Node* key = get(virtualRegisterForArgument(1, registerOffset));
        Node* hash = addToGraph(MapHash, key);
        Node* bucket = addToGraph(GetMapBucket, Edge(map, MapObjectUse), Edge(key), Edge(hash));
        Node* result = addToGraph(LoadFromJSMapBucket, OpInfo(), OpInfo(prediction), bucket);
        set(VirtualRegister(resultOperand), result);
        return true;
    }

    case JSSetHasIntrinsic:
    case JSMapHasIntrinsic: {
        if (argumentCountIncludingThis != 2)
            return false;

        insertChecks();
        Node* mapOrSet = get(virtualRegisterForArgument(0, registerOffset));
        Node* key = get(virtualRegisterForArgument(1, registerOffset));
        Node* hash = addToGraph(MapHash, key);
        UseKind useKind = intrinsic == JSSetHasIntrinsic ? SetObjectUse : MapObjectUse;
        Node* bucket = addToGraph(GetMapBucket, OpInfo(0), Edge(mapOrSet, useKind), Edge(key), Edge(hash));
        Node* result = addToGraph(IsNonEmptyMapBucket, bucket);
        set(VirtualRegister(resultOperand), result);
        return true;
    }

    case HasOwnPropertyIntrinsic: {
        if (argumentCountIncludingThis != 2)
            return false;

        // This can be racy, that's fine. We know that once we observe that this is created,
        // that it will never be destroyed until the VM is destroyed. It's unlikely that
        // we'd ever get to the point where we inline this as an intrinsic without the
        // cache being created, however, it's possible if we always throw exceptions inside
        // hasOwnProperty.
        if (!m_vm->hasOwnPropertyCache())
            return false;

        insertChecks();
        Node* object = get(virtualRegisterForArgument(0, registerOffset));
        Node* key = get(virtualRegisterForArgument(1, registerOffset));
        Node* result = addToGraph(HasOwnProperty, object, key);
        set(VirtualRegister(resultOperand), result);
        return true;
    }

    case ToLowerCaseIntrinsic: {
        if (argumentCountIncludingThis != 1)
            return false;

        if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadType))
            return false;

        insertChecks();
        Node* thisString = get(virtualRegisterForArgument(0, registerOffset));
        Node* result = addToGraph(ToLowerCase, thisString);
        set(VirtualRegister(resultOperand), result);
        return true;
    }

    default:
        return false;
    }
}

template<typename ChecksFunctor>
bool ByteCodeParser::handleDOMJITCall(Node* callTarget, int resultOperand, const DOMJIT::Signature* signature, int registerOffset, int argumentCountIncludingThis, SpeculatedType prediction, const ChecksFunctor& insertChecks)
{
    if (argumentCountIncludingThis != static_cast<int>(1 + signature->argumentCount))
        return false;
    if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadType))
        return false;

    // FIXME: Currently, we only support functions which arguments are up to 2.
    // Eventually, we should extend this. But possibly, 2 or 3 can cover typical use cases.
    // https://bugs.webkit.org/show_bug.cgi?id=164346
    ASSERT_WITH_MESSAGE(argumentCountIncludingThis <= JSC_DOMJIT_SIGNATURE_MAX_ARGUMENTS_INCLUDING_THIS, "Currently CallDOM does not support an arbitrary length arguments.");

    insertChecks();
    addCall(resultOperand, Call, signature, callTarget, argumentCountIncludingThis, registerOffset, prediction);
    return true;
}


template<typename ChecksFunctor>
bool ByteCodeParser::handleIntrinsicGetter(int resultOperand, const GetByIdVariant& variant, Node* thisNode, const ChecksFunctor& insertChecks)
{
    switch (variant.intrinsic()) {
    case TypedArrayByteLengthIntrinsic: {
        insertChecks();

        TypedArrayType type = (*variant.structureSet().begin())->classInfo()->typedArrayStorageType;
        Array::Type arrayType = toArrayType(type);
        size_t logSize = logElementSize(type);

        variant.structureSet().forEach([&] (Structure* structure) {
            TypedArrayType curType = structure->classInfo()->typedArrayStorageType;
            ASSERT(logSize == logElementSize(curType));
            arrayType = refineTypedArrayType(arrayType, curType);
            ASSERT(arrayType != Array::Generic);
        });

        Node* lengthNode = addToGraph(GetArrayLength, OpInfo(ArrayMode(arrayType).asWord()), thisNode);

        if (!logSize) {
            set(VirtualRegister(resultOperand), lengthNode);
            return true;
        }

        // We can use a BitLShift here because typed arrays will never have a byteLength
        // that overflows int32.
        Node* shiftNode = jsConstant(jsNumber(logSize));
        set(VirtualRegister(resultOperand), addToGraph(BitLShift, lengthNode, shiftNode));

        return true;
    }

    case TypedArrayLengthIntrinsic: {
        insertChecks();

        TypedArrayType type = (*variant.structureSet().begin())->classInfo()->typedArrayStorageType;
        Array::Type arrayType = toArrayType(type);

        variant.structureSet().forEach([&] (Structure* structure) {
            TypedArrayType curType = structure->classInfo()->typedArrayStorageType;
            arrayType = refineTypedArrayType(arrayType, curType);
            ASSERT(arrayType != Array::Generic);
        });

        set(VirtualRegister(resultOperand), addToGraph(GetArrayLength, OpInfo(ArrayMode(arrayType).asWord()), thisNode));

        return true;

    }

    case TypedArrayByteOffsetIntrinsic: {
        insertChecks();

        TypedArrayType type = (*variant.structureSet().begin())->classInfo()->typedArrayStorageType;
        Array::Type arrayType = toArrayType(type);

        variant.structureSet().forEach([&] (Structure* structure) {
            TypedArrayType curType = structure->classInfo()->typedArrayStorageType;
            arrayType = refineTypedArrayType(arrayType, curType);
            ASSERT(arrayType != Array::Generic);
        });

        set(VirtualRegister(resultOperand), addToGraph(GetTypedArrayByteOffset, OpInfo(ArrayMode(arrayType).asWord()), thisNode));

        return true;
    }

    default:
        return false;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

static void blessCallDOMGetter(Node* node)
{
    DOMJIT::CallDOMGetterPatchpoint* patchpoint = node->callDOMGetterData()->patchpoint;
    if (!patchpoint->effect.mustGenerate())
        node->clearFlags(NodeMustGenerate);
}

bool ByteCodeParser::handleDOMJITGetter(int resultOperand, const GetByIdVariant& variant, Node* thisNode, unsigned identifierNumber, SpeculatedType prediction)
{
    if (!variant.domJIT())
        return false;

    DOMJIT::GetterSetter* domJIT = variant.domJIT();

    // We do not need to actually look up CustomGetterSetter here. Checking Structures or registering watchpoints are enough,
    // since replacement of CustomGetterSetter always incurs Structure transition.
    if (!check(variant.conditionSet()))
        return false;
    addToGraph(CheckStructure, OpInfo(m_graph.addStructureSet(variant.structureSet())), thisNode);

    Ref<DOMJIT::Patchpoint> checkDOMPatchpoint = domJIT->checkDOM();
    m_graph.m_domJITPatchpoints.append(checkDOMPatchpoint.ptr());
    // We do not need to emit CheckCell thingy here. When the custom accessor is replaced to different one, Structure transition occurs.
    addToGraph(CheckDOM, OpInfo(checkDOMPatchpoint.ptr()), OpInfo(domJIT->thisClassInfo()), thisNode);

    CallDOMGetterData* callDOMGetterData = m_graph.m_callDOMGetterData.add();
    Ref<DOMJIT::CallDOMGetterPatchpoint> callDOMGetterPatchpoint = domJIT->callDOMGetter();
    m_graph.m_domJITPatchpoints.append(callDOMGetterPatchpoint.ptr());

    callDOMGetterData->domJIT = domJIT;
    callDOMGetterData->patchpoint = callDOMGetterPatchpoint.ptr();
    callDOMGetterData->identifierNumber = identifierNumber;

    Node* callDOMGetterNode = nullptr;
    // GlobalObject of thisNode is always used to create a DOMWrapper.
    if (callDOMGetterPatchpoint->requireGlobalObject) {
        Node* globalObject = addToGraph(GetGlobalObject, thisNode);
        callDOMGetterNode = addToGraph(CallDOMGetter, OpInfo(callDOMGetterData), OpInfo(prediction), thisNode, globalObject);
    } else
        callDOMGetterNode = addToGraph(CallDOMGetter, OpInfo(callDOMGetterData), OpInfo(prediction), thisNode);
    blessCallDOMGetter(callDOMGetterNode);
    set(VirtualRegister(resultOperand), callDOMGetterNode);
    return true;
}

bool ByteCodeParser::handleModuleNamespaceLoad(int resultOperand, SpeculatedType prediction, Node* base, GetByIdStatus getById)
{
    if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadCell))
        return false;
    addToGraph(CheckCell, OpInfo(m_graph.freeze(getById.moduleNamespaceObject())), Edge(base, CellUse));

    // Ideally we wouldn't have to do this Phantom. But:
    //
    // For the constant case: we must do it because otherwise we would have no way of knowing
    // that the scope is live at OSR here.
    //
    // For the non-constant case: GetClosureVar could be DCE'd, but baseline's implementation
    // won't be able to handle an Undefined scope.
    addToGraph(Phantom, base);

    // Constant folding in the bytecode parser is important for performance. This may not
    // have executed yet. If it hasn't, then we won't have a prediction. Lacking a
    // prediction, we'd otherwise think that it has to exit. Then when it did execute, we
    // would recompile. But if we can fold it here, we avoid the exit.
    m_graph.freeze(getById.moduleEnvironment());
    if (JSValue value = m_graph.tryGetConstantClosureVar(getById.moduleEnvironment(), getById.scopeOffset())) {
        set(VirtualRegister(resultOperand), weakJSConstant(value));
        return true;
    }
    set(VirtualRegister(resultOperand), addToGraph(GetClosureVar, OpInfo(getById.scopeOffset().offset()), OpInfo(prediction), weakJSConstant(getById.moduleEnvironment())));
    return true;
}

template<typename ChecksFunctor>
bool ByteCodeParser::handleTypedArrayConstructor(
    int resultOperand, InternalFunction* function, int registerOffset,
    int argumentCountIncludingThis, TypedArrayType type, const ChecksFunctor& insertChecks)
{
    if (!isTypedView(type))
        return false;
    
    if (function->classInfo() != constructorClassInfoForType(type))
        return false;
    
    if (function->globalObject() != m_inlineStackTop->m_codeBlock->globalObject())
        return false;
    
    // We only have an intrinsic for the case where you say:
    //
    // new FooArray(blah);
    //
    // Of course, 'blah' could be any of the following:
    //
    // - Integer, indicating that you want to allocate an array of that length.
    //   This is the thing we're hoping for, and what we can actually do meaningful
    //   optimizations for.
    //
    // - Array buffer, indicating that you want to create a view onto that _entire_
    //   buffer.
    //
    // - Non-buffer object, indicating that you want to create a copy of that
    //   object by pretending that it quacks like an array.
    //
    // - Anything else, indicating that you want to have an exception thrown at
    //   you.
    //
    // The intrinsic, NewTypedArray, will behave as if it could do any of these
    // things up until we do Fixup. Thereafter, if child1 (i.e. 'blah') is
    // predicted Int32, then we lock it in as a normal typed array allocation.
    // Otherwise, NewTypedArray turns into a totally opaque function call that
    // may clobber the world - by virtue of it accessing properties on what could
    // be an object.
    //
    // Note that although the generic form of NewTypedArray sounds sort of awful,
    // it is actually quite likely to be more efficient than a fully generic
    // Construct. So, we might want to think about making NewTypedArray variadic,
    // or else making Construct not super slow.
    
    if (argumentCountIncludingThis != 2)
        return false;
    
    if (!function->globalObject()->typedArrayStructureConcurrently(type))
        return false;

    insertChecks();
    set(VirtualRegister(resultOperand),
        addToGraph(NewTypedArray, OpInfo(type), get(virtualRegisterForArgument(1, registerOffset))));
    return true;
}

template<typename ChecksFunctor>
bool ByteCodeParser::handleConstantInternalFunction(
    Node* callTargetNode, int resultOperand, InternalFunction* function, int registerOffset,
    int argumentCountIncludingThis, CodeSpecializationKind kind, SpeculatedType prediction, const ChecksFunctor& insertChecks)
{
    if (verbose)
        dataLog("    Handling constant internal function ", JSValue(function), "\n");

    if (kind == CodeForConstruct) {
        Node* newTargetNode = get(virtualRegisterForArgument(0, registerOffset));
        // We cannot handle the case where new.target != callee (i.e. a construct from a super call) because we
        // don't know what the prototype of the constructed object will be.
        // FIXME: If we have inlined super calls up to the call site, however, we should be able to figure out the structure. https://bugs.webkit.org/show_bug.cgi?id=152700
        if (newTargetNode != callTargetNode)
            return false;
    }

    if (function->classInfo() == ArrayConstructor::info()) {
        if (function->globalObject() != m_inlineStackTop->m_codeBlock->globalObject())
            return false;
        
        insertChecks();
        if (argumentCountIncludingThis == 2) {
            set(VirtualRegister(resultOperand),
                addToGraph(NewArrayWithSize, OpInfo(ArrayWithUndecided), get(virtualRegisterForArgument(1, registerOffset))));
            return true;
        }
        
        for (int i = 1; i < argumentCountIncludingThis; ++i)
            addVarArgChild(get(virtualRegisterForArgument(i, registerOffset)));
        set(VirtualRegister(resultOperand),
            addToGraph(Node::VarArg, NewArray, OpInfo(ArrayWithUndecided), OpInfo(0)));
        return true;
    }

    if (function->classInfo() == NumberConstructor::info()) {
        if (kind == CodeForConstruct)
            return false;

        insertChecks();
        if (argumentCountIncludingThis <= 1)
            set(VirtualRegister(resultOperand), jsConstant(jsNumber(0)));
        else
            set(VirtualRegister(resultOperand), addToGraph(ToNumber, OpInfo(0), OpInfo(prediction), get(virtualRegisterForArgument(1, registerOffset))));

        return true;
    }
    
    if (function->classInfo() == StringConstructor::info()) {
        insertChecks();
        
        Node* result;
        
        if (argumentCountIncludingThis <= 1)
            result = jsConstant(m_vm->smallStrings.emptyString());
        else
            result = addToGraph(CallStringConstructor, get(virtualRegisterForArgument(1, registerOffset)));
        
        if (kind == CodeForConstruct)
            result = addToGraph(NewStringObject, OpInfo(m_graph.registerStructure(function->globalObject()->stringObjectStructure())), result);
        
        set(VirtualRegister(resultOperand), result);
        return true;
    }

    // FIXME: This should handle construction as well. https://bugs.webkit.org/show_bug.cgi?id=155591
    if (function->classInfo() == ObjectConstructor::info() && kind == CodeForCall) {
        insertChecks();

        Node* result;
        if (argumentCountIncludingThis <= 1)
            result = addToGraph(NewObject, OpInfo(m_graph.registerStructure(function->globalObject()->objectStructureForObjectConstructor())));
        else
            result = addToGraph(CallObjectConstructor, get(virtualRegisterForArgument(1, registerOffset)));
        set(VirtualRegister(resultOperand), result);
        return true;
    }

    for (unsigned typeIndex = 0; typeIndex < NUMBER_OF_TYPED_ARRAY_TYPES; ++typeIndex) {
        bool result = handleTypedArrayConstructor(
            resultOperand, function, registerOffset, argumentCountIncludingThis,
            indexToTypedArrayType(typeIndex), insertChecks);
        if (result)
            return true;
    }
    
    return false;
}

Node* ByteCodeParser::handleGetByOffset(
    SpeculatedType prediction, Node* base, unsigned identifierNumber, PropertyOffset offset,
    const InferredType::Descriptor& inferredType, NodeType op)
{
    Node* propertyStorage;
    if (isInlineOffset(offset))
        propertyStorage = base;
    else
        propertyStorage = addToGraph(GetButterfly, base);
    
    StorageAccessData* data = m_graph.m_storageAccessData.add();
    data->offset = offset;
    data->identifierNumber = identifierNumber;
    data->inferredType = inferredType;
    m_graph.registerInferredType(inferredType);
    
    Node* getByOffset = addToGraph(op, OpInfo(data), OpInfo(prediction), propertyStorage, base);

    return getByOffset;
}

Node* ByteCodeParser::handlePutByOffset(
    Node* base, unsigned identifier, PropertyOffset offset, const InferredType::Descriptor& inferredType,
    Node* value)
{
    Node* propertyStorage;
    if (isInlineOffset(offset))
        propertyStorage = base;
    else
        propertyStorage = addToGraph(GetButterfly, base);
    
    StorageAccessData* data = m_graph.m_storageAccessData.add();
    data->offset = offset;
    data->identifierNumber = identifier;
    data->inferredType = inferredType;
    m_graph.registerInferredType(inferredType);
    
    Node* result = addToGraph(PutByOffset, OpInfo(data), propertyStorage, base, value);
    
    return result;
}

bool ByteCodeParser::check(const ObjectPropertyCondition& condition)
{
    if (!condition)
        return false;
    
    if (m_graph.watchCondition(condition))
        return true;
    
    Structure* structure = condition.object()->structure();
    if (!condition.structureEnsuresValidity(structure))
        return false;
    
    addToGraph(
        CheckStructure,
        OpInfo(m_graph.addStructureSet(structure)),
        weakJSConstant(condition.object()));
    return true;
}

GetByOffsetMethod ByteCodeParser::promoteToConstant(GetByOffsetMethod method)
{
    if (method.kind() == GetByOffsetMethod::LoadFromPrototype
        && method.prototype()->structure()->dfgShouldWatch()) {
        if (JSValue constant = m_graph.tryGetConstantProperty(method.prototype()->value(), method.prototype()->structure(), method.offset()))
            return GetByOffsetMethod::constant(m_graph.freeze(constant));
    }
    
    return method;
}

bool ByteCodeParser::needsDynamicLookup(ResolveType type, OpcodeID opcode)
{
    ASSERT(opcode == op_resolve_scope || opcode == op_get_from_scope || opcode == op_put_to_scope);

    JSGlobalObject* globalObject = m_inlineStackTop->m_codeBlock->globalObject();
    if (needsVarInjectionChecks(type) && globalObject->varInjectionWatchpoint()->hasBeenInvalidated())
        return true;

    switch (type) {
    case GlobalProperty:
    case GlobalVar:
    case GlobalLexicalVar:
    case ClosureVar:
    case LocalClosureVar:
    case ModuleVar:
        return false;

    case UnresolvedProperty:
    case UnresolvedPropertyWithVarInjectionChecks: {
        // The heuristic for UnresolvedProperty scope accesses is we will ForceOSRExit if we
        // haven't exited from from this access before to let the baseline JIT try to better
        // cache the access. If we've already exited from this operation, it's unlikely that
        // the baseline will come up with a better ResolveType and instead we will compile
        // this as a dynamic scope access.

        // We only track our heuristic through resolve_scope since resolve_scope will
        // dominate unresolved gets/puts on that scope.
        if (opcode != op_resolve_scope)
            return true;

        if (m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, InadequateCoverage)) {
            // We've already exited so give up on getting better ResolveType information.
            return true;
        }

        // We have not exited yet, so let's have the baseline get better ResolveType information for us.
        // This type of code is often seen when we tier up in a loop but haven't executed the part
        // of a function that comes after the loop.
        return false;
    }

    case Dynamic:
        return true;

    case GlobalPropertyWithVarInjectionChecks:
    case GlobalVarWithVarInjectionChecks:
    case GlobalLexicalVarWithVarInjectionChecks:
    case ClosureVarWithVarInjectionChecks:
        return false;
    }

    ASSERT_NOT_REACHED();
    return false;
}

GetByOffsetMethod ByteCodeParser::planLoad(const ObjectPropertyCondition& condition)
{
    if (verbose)
        dataLog("Planning a load: ", condition, "\n");
    
    // We might promote this to Equivalence, and a later DFG pass might also do such promotion
    // even if we fail, but for simplicity this cannot be asked to load an equivalence condition.
    // None of the clients of this method will request a load of an Equivalence condition anyway,
    // and supporting it would complicate the heuristics below.
    RELEASE_ASSERT(condition.kind() == PropertyCondition::Presence);
    
    // Here's the ranking of how to handle this, from most preferred to least preferred:
    //
    // 1) Watchpoint on an equivalence condition and return a constant node for the loaded value.
    //    No other code is emitted, and the structure of the base object is never registered.
    //    Hence this results in zero code and we won't jettison this compilation if the object
    //    transitions, even if the structure is watchable right now.
    //
    // 2) Need to emit a load, and the current structure of the base is going to be watched by the
    //    DFG anyway (i.e. dfgShouldWatch). Watch the structure and emit the load. Don't watch the
    //    condition, since the act of turning the base into a constant in IR will cause the DFG to
    //    watch the structure anyway and doing so would subsume watching the condition.
    //
    // 3) Need to emit a load, and the current structure of the base is watchable but not by the
    //    DFG (i.e. transitionWatchpointSetIsStillValid() and !dfgShouldWatchIfPossible()). Watch
    //    the condition, and emit a load.
    //
    // 4) Need to emit a load, and the current structure of the base is not watchable. Emit a
    //    structure check, and emit a load.
    //
    // 5) The condition does not hold. Give up and return null.
    
    // First, try to promote Presence to Equivalence. We do this before doing anything else
    // because it's the most profitable. Also, there are cases where the presence is watchable but
    // we don't want to watch it unless it became an equivalence (see the relationship between
    // (1), (2), and (3) above).
    ObjectPropertyCondition equivalenceCondition = condition.attemptToMakeEquivalenceWithoutBarrier(*m_vm);
    if (m_graph.watchCondition(equivalenceCondition))
        return GetByOffsetMethod::constant(m_graph.freeze(equivalenceCondition.requiredValue()));
    
    // At this point, we'll have to materialize the condition's base as a constant in DFG IR. Once
    // we do this, the frozen value will have its own idea of what the structure is. Use that from
    // now on just because it's less confusing.
    FrozenValue* base = m_graph.freeze(condition.object());
    Structure* structure = base->structure();
    
    // Check if the structure that we've registered makes the condition hold. If not, just give
    // up. This is case (5) above.
    if (!condition.structureEnsuresValidity(structure))
        return GetByOffsetMethod();
    
    // If the structure is watched by the DFG already, then just use this fact to emit the load.
    // This is case (2) above.
    if (structure->dfgShouldWatch())
        return promoteToConstant(GetByOffsetMethod::loadFromPrototype(base, condition.offset()));
    
    // If we can watch the condition right now, then we can emit the load after watching it. This
    // is case (3) above.
    if (m_graph.watchCondition(condition))
        return promoteToConstant(GetByOffsetMethod::loadFromPrototype(base, condition.offset()));
    
    // We can't watch anything but we know that the current structure satisfies the condition. So,
    // check for that structure and then emit the load.
    addToGraph(
        CheckStructure, 
        OpInfo(m_graph.addStructureSet(structure)),
        addToGraph(JSConstant, OpInfo(base)));
    return promoteToConstant(GetByOffsetMethod::loadFromPrototype(base, condition.offset()));
}

Node* ByteCodeParser::load(
    SpeculatedType prediction, unsigned identifierNumber, const GetByOffsetMethod& method,
    NodeType op)
{
    switch (method.kind()) {
    case GetByOffsetMethod::Invalid:
        return nullptr;
    case GetByOffsetMethod::Constant:
        return addToGraph(JSConstant, OpInfo(method.constant()));
    case GetByOffsetMethod::LoadFromPrototype: {
        Node* baseNode = addToGraph(JSConstant, OpInfo(method.prototype()));
        return handleGetByOffset(
            prediction, baseNode, identifierNumber, method.offset(), InferredType::Top, op);
    }
    case GetByOffsetMethod::Load:
        // Will never see this from planLoad().
        RELEASE_ASSERT_NOT_REACHED();
        return nullptr;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
    return nullptr;
}

Node* ByteCodeParser::load(
    SpeculatedType prediction, const ObjectPropertyCondition& condition, NodeType op)
{
    GetByOffsetMethod method = planLoad(condition);
    return load(prediction, m_graph.identifiers().ensure(condition.uid()), method, op);
}

bool ByteCodeParser::check(const ObjectPropertyConditionSet& conditionSet)
{
    for (const ObjectPropertyCondition condition : conditionSet) {
        if (!check(condition))
            return false;
    }
    return true;
}

GetByOffsetMethod ByteCodeParser::planLoad(const ObjectPropertyConditionSet& conditionSet)
{
    if (verbose)
        dataLog("conditionSet = ", conditionSet, "\n");
    
    GetByOffsetMethod result;
    for (const ObjectPropertyCondition condition : conditionSet) {
        switch (condition.kind()) {
        case PropertyCondition::Presence:
            RELEASE_ASSERT(!result); // Should only see exactly one of these.
            result = planLoad(condition);
            if (!result)
                return GetByOffsetMethod();
            break;
        default:
            if (!check(condition))
                return GetByOffsetMethod();
            break;
        }
    }
    if (!result) {
        // We have a unset property.
        ASSERT(!conditionSet.numberOfConditionsWithKind(PropertyCondition::Presence));
        return GetByOffsetMethod::constant(m_constantUndefined);
    }
    return result;
}

Node* ByteCodeParser::load(
    SpeculatedType prediction, const ObjectPropertyConditionSet& conditionSet, NodeType op)
{
    GetByOffsetMethod method = planLoad(conditionSet);
    return load(
        prediction,
        m_graph.identifiers().ensure(conditionSet.slotBaseCondition().uid()),
        method, op);
}

ObjectPropertyCondition ByteCodeParser::presenceLike(
    JSObject* knownBase, UniquedStringImpl* uid, PropertyOffset offset, const StructureSet& set)
{
    if (set.isEmpty())
        return ObjectPropertyCondition();
    unsigned attributes;
    PropertyOffset firstOffset = set[0]->getConcurrently(uid, attributes);
    if (firstOffset != offset)
        return ObjectPropertyCondition();
    for (unsigned i = 1; i < set.size(); ++i) {
        unsigned otherAttributes;
        PropertyOffset otherOffset = set[i]->getConcurrently(uid, otherAttributes);
        if (otherOffset != offset || otherAttributes != attributes)
            return ObjectPropertyCondition();
    }
    return ObjectPropertyCondition::presenceWithoutBarrier(knownBase, uid, offset, attributes);
}

bool ByteCodeParser::checkPresenceLike(
    JSObject* knownBase, UniquedStringImpl* uid, PropertyOffset offset, const StructureSet& set)
{
    return check(presenceLike(knownBase, uid, offset, set));
}

void ByteCodeParser::checkPresenceLike(
    Node* base, UniquedStringImpl* uid, PropertyOffset offset, const StructureSet& set)
{
    if (JSObject* knownBase = base->dynamicCastConstant<JSObject*>(*m_vm)) {
        if (checkPresenceLike(knownBase, uid, offset, set))
            return;
    }

    addToGraph(CheckStructure, OpInfo(m_graph.addStructureSet(set)), base);
}

template<typename VariantType>
Node* ByteCodeParser::load(
    SpeculatedType prediction, Node* base, unsigned identifierNumber, const VariantType& variant)
{
    // Make sure backwards propagation knows that we've used base.
    addToGraph(Phantom, base);
    
    bool needStructureCheck = true;
    
    UniquedStringImpl* uid = m_graph.identifiers()[identifierNumber];
    
    if (JSObject* knownBase = base->dynamicCastConstant<JSObject*>(*m_vm)) {
        // Try to optimize away the structure check. Note that it's not worth doing anything about this
        // if the base's structure is watched.
        Structure* structure = base->constant()->structure();
        if (!structure->dfgShouldWatch()) {
            if (!variant.conditionSet().isEmpty()) {
                // This means that we're loading from a prototype or we have a property miss. We expect
                // the base not to have the property. We can only use ObjectPropertyCondition if all of
                // the structures in the variant.structureSet() agree on the prototype (it would be
                // hilariously rare if they didn't). Note that we are relying on structureSet() having
                // at least one element. That will always be true here because of how GetByIdStatus/PutByIdStatus work.
                JSObject* prototype = variant.structureSet()[0]->storedPrototypeObject();
                bool allAgree = true;
                for (unsigned i = 1; i < variant.structureSet().size(); ++i) {
                    if (variant.structureSet()[i]->storedPrototypeObject() != prototype) {
                        allAgree = false;
                        break;
                    }
                }
                if (allAgree) {
                    ObjectPropertyCondition condition = ObjectPropertyCondition::absenceWithoutBarrier(
                        knownBase, uid, prototype);
                    if (check(condition))
                        needStructureCheck = false;
                }
            } else {
                // This means we're loading directly from base. We can avoid all of the code that follows
                // if we can prove that the property is a constant. Otherwise, we try to prove that the
                // property is watchably present, in which case we get rid of the structure check.

                ObjectPropertyCondition presenceCondition =
                    presenceLike(knownBase, uid, variant.offset(), variant.structureSet());
                if (presenceCondition) {
                    ObjectPropertyCondition equivalenceCondition =
                        presenceCondition.attemptToMakeEquivalenceWithoutBarrier(*m_vm);
                    if (m_graph.watchCondition(equivalenceCondition))
                        return weakJSConstant(equivalenceCondition.requiredValue());
                    
                    if (check(presenceCondition))
                        needStructureCheck = false;
                }
            }
        }
    }

    if (needStructureCheck)
        addToGraph(CheckStructure, OpInfo(m_graph.addStructureSet(variant.structureSet())), base);

    if (variant.isPropertyUnset()) {
        if (m_graph.watchConditions(variant.conditionSet()))
            return jsConstant(jsUndefined());
        return nullptr;
    }

    SpeculatedType loadPrediction;
    NodeType loadOp;
    if (variant.callLinkStatus() || variant.intrinsic() != NoIntrinsic) {
        loadPrediction = SpecCellOther;
        loadOp = GetGetterSetterByOffset;
    } else {
        loadPrediction = prediction;
        loadOp = GetByOffset;
    }
    
    Node* loadedValue;
    if (!variant.conditionSet().isEmpty())
        loadedValue = load(loadPrediction, variant.conditionSet(), loadOp);
    else {
        if (needStructureCheck && base->hasConstant()) {
            // We did emit a structure check. That means that we have an opportunity to do constant folding
            // here, since we didn't do it above.
            JSValue constant = m_graph.tryGetConstantProperty(
                base->asJSValue(), *m_graph.addStructureSet(variant.structureSet()), variant.offset());
            if (constant)
                return weakJSConstant(constant);
        }

        InferredType::Descriptor inferredType;
        if (needStructureCheck) {
            for (Structure* structure : variant.structureSet()) {
                InferredType::Descriptor thisType = m_graph.inferredTypeForProperty(structure, uid);
                inferredType.merge(thisType);
            }
        } else
            inferredType = InferredType::Top;
        
        loadedValue = handleGetByOffset(
            loadPrediction, base, identifierNumber, variant.offset(), inferredType, loadOp);
    }

    return loadedValue;
}

Node* ByteCodeParser::store(Node* base, unsigned identifier, const PutByIdVariant& variant, Node* value)
{
    RELEASE_ASSERT(variant.kind() == PutByIdVariant::Replace);

    checkPresenceLike(base, m_graph.identifiers()[identifier], variant.offset(), variant.structure());
    return handlePutByOffset(base, identifier, variant.offset(), variant.requiredType(), value);
}

void ByteCodeParser::handleGetById(
    int destinationOperand, SpeculatedType prediction, Node* base, unsigned identifierNumber,
    GetByIdStatus getByIdStatus, AccessType type, unsigned instructionSize)
{
    // Attempt to reduce the set of things in the GetByIdStatus.
    if (base->op() == NewObject) {
        bool ok = true;
        for (unsigned i = m_currentBlock->size(); i--;) {
            Node* node = m_currentBlock->at(i);
            if (node == base)
                break;
            if (writesOverlap(m_graph, node, JSCell_structureID)) {
                ok = false;
                break;
            }
        }
        if (ok)
            getByIdStatus.filter(base->structure().get());
    }
    
    NodeType getById;
    if (type == AccessType::Get)
        getById = getByIdStatus.makesCalls() ? GetByIdFlush : GetById;
    else
        getById = TryGetById;

    if (getById != TryGetById && getByIdStatus.isModuleNamespace()) {
        if (handleModuleNamespaceLoad(destinationOperand, prediction, base, getByIdStatus)) {
            if (m_graph.compilation())
                m_graph.compilation()->noticeInlinedGetById();
            return;
        }
    }

    // Special path for custom accessors since custom's offset does not have any meanings.
    // So, this is completely different from Simple one. But we have a chance to optimize it when we use DOMJIT.
    if (Options::useDOMJIT() && getByIdStatus.isCustom()) {
        ASSERT(getByIdStatus.numVariants() == 1);
        ASSERT(!getByIdStatus.makesCalls());
        GetByIdVariant variant = getByIdStatus[0];
        ASSERT(variant.domJIT());
        if (handleDOMJITGetter(destinationOperand, variant, base, identifierNumber, prediction)) {
            if (m_graph.compilation())
                m_graph.compilation()->noticeInlinedGetById();
            return;
        }
    }

    ASSERT(type == AccessType::Get || !getByIdStatus.makesCalls());
    if (!getByIdStatus.isSimple() || !getByIdStatus.numVariants() || !Options::useAccessInlining()) {
        set(VirtualRegister(destinationOperand),
            addToGraph(getById, OpInfo(identifierNumber), OpInfo(prediction), base));
        return;
    }
    
    if (getByIdStatus.numVariants() > 1) {
        if (getByIdStatus.makesCalls() || !isFTL(m_graph.m_plan.mode)
            || !Options::usePolymorphicAccessInlining()) {
            set(VirtualRegister(destinationOperand),
                addToGraph(getById, OpInfo(identifierNumber), OpInfo(prediction), base));
            return;
        }
        
        Vector<MultiGetByOffsetCase, 2> cases;
        
        // 1) Emit prototype structure checks for all chains. This could sort of maybe not be
        //    optimal, if there is some rarely executed case in the chain that requires a lot
        //    of checks and those checks are not watchpointable.
        for (const GetByIdVariant& variant : getByIdStatus.variants()) {
            if (variant.intrinsic() != NoIntrinsic) {
                set(VirtualRegister(destinationOperand),
                    addToGraph(getById, OpInfo(identifierNumber), OpInfo(prediction), base));
                return;
            }

            if (variant.conditionSet().isEmpty()) {
                cases.append(
                    MultiGetByOffsetCase(
                        *m_graph.addStructureSet(variant.structureSet()),
                        GetByOffsetMethod::load(variant.offset())));
                continue;
            }

            GetByOffsetMethod method = planLoad(variant.conditionSet());
            if (!method) {
                set(VirtualRegister(destinationOperand),
                    addToGraph(getById, OpInfo(identifierNumber), OpInfo(prediction), base));
                return;
            }
            
            cases.append(MultiGetByOffsetCase(*m_graph.addStructureSet(variant.structureSet()), method));
        }

        if (m_graph.compilation())
            m_graph.compilation()->noticeInlinedGetById();
    
        // 2) Emit a MultiGetByOffset
        MultiGetByOffsetData* data = m_graph.m_multiGetByOffsetData.add();
        data->cases = cases;
        data->identifierNumber = identifierNumber;
        set(VirtualRegister(destinationOperand),
            addToGraph(MultiGetByOffset, OpInfo(data), OpInfo(prediction), base));
        return;
    }
    
    ASSERT(getByIdStatus.numVariants() == 1);
    GetByIdVariant variant = getByIdStatus[0];
                
    Node* loadedValue = load(prediction, base, identifierNumber, variant);
    if (!loadedValue) {
        set(VirtualRegister(destinationOperand),
            addToGraph(getById, OpInfo(identifierNumber), OpInfo(prediction), base));
        return;
    }

    if (m_graph.compilation())
        m_graph.compilation()->noticeInlinedGetById();

    ASSERT(type == AccessType::Get || !variant.callLinkStatus());
    if (!variant.callLinkStatus() && variant.intrinsic() == NoIntrinsic) {
        set(VirtualRegister(destinationOperand), loadedValue);
        return;
    }
    
    Node* getter = addToGraph(GetGetter, loadedValue);

    if (handleIntrinsicGetter(destinationOperand, variant, base,
            [&] () {
                addToGraph(CheckCell, OpInfo(m_graph.freeze(variant.intrinsicFunction())), getter, base);
            })) {
        addToGraph(Phantom, getter);
        return;
    }

    ASSERT(variant.intrinsic() == NoIntrinsic);

    // Make a call. We don't try to get fancy with using the smallest operand number because
    // the stack layout phase should compress the stack anyway.
    
    unsigned numberOfParameters = 0;
    numberOfParameters++; // The 'this' argument.
    numberOfParameters++; // True return PC.
    
    // Start with a register offset that corresponds to the last in-use register.
    int registerOffset = virtualRegisterForLocal(
        m_inlineStackTop->m_profiledBlock->m_numCalleeLocals - 1).offset();
    registerOffset -= numberOfParameters;
    registerOffset -= CallFrame::headerSizeInRegisters;
    
    // Get the alignment right.
    registerOffset = -WTF::roundUpToMultipleOf(
        stackAlignmentRegisters(),
        -registerOffset);
    
    ensureLocals(
        m_inlineStackTop->remapOperand(
            VirtualRegister(registerOffset)).toLocal());
    
    // Issue SetLocals. This has two effects:
    // 1) That's how handleCall() sees the arguments.
    // 2) If we inline then this ensures that the arguments are flushed so that if you use
    //    the dreaded arguments object on the getter, the right things happen. Well, sort of -
    //    since we only really care about 'this' in this case. But we're not going to take that
    //    shortcut.
    int nextRegister = registerOffset + CallFrame::headerSizeInRegisters;
    set(VirtualRegister(nextRegister++), base, ImmediateNakedSet);

    // We've set some locals, but they are not user-visible. It's still OK to exit from here.
    m_exitOK = true;
    addToGraph(ExitOK);
    
    handleCall(
        destinationOperand, Call, InlineCallFrame::GetterCall, instructionSize,
        getter, numberOfParameters - 1, registerOffset, *variant.callLinkStatus(), prediction);
}

void ByteCodeParser::emitPutById(
    Node* base, unsigned identifierNumber, Node* value, const PutByIdStatus& putByIdStatus, bool isDirect)
{
    if (isDirect)
        addToGraph(PutByIdDirect, OpInfo(identifierNumber), base, value);
    else
        addToGraph(putByIdStatus.makesCalls() ? PutByIdFlush : PutById, OpInfo(identifierNumber), base, value);
}

void ByteCodeParser::handlePutById(
    Node* base, unsigned identifierNumber, Node* value,
    const PutByIdStatus& putByIdStatus, bool isDirect)
{
    if (!putByIdStatus.isSimple() || !putByIdStatus.numVariants() || !Options::useAccessInlining()) {
        if (!putByIdStatus.isSet())
            addToGraph(ForceOSRExit);
        emitPutById(base, identifierNumber, value, putByIdStatus, isDirect);
        return;
    }
    
    if (putByIdStatus.numVariants() > 1) {
        if (!isFTL(m_graph.m_plan.mode) || putByIdStatus.makesCalls()
            || !Options::usePolymorphicAccessInlining()) {
            emitPutById(base, identifierNumber, value, putByIdStatus, isDirect);
            return;
        }
        
        if (!isDirect) {
            for (unsigned variantIndex = putByIdStatus.numVariants(); variantIndex--;) {
                if (putByIdStatus[variantIndex].kind() != PutByIdVariant::Transition)
                    continue;
                if (!check(putByIdStatus[variantIndex].conditionSet())) {
                    emitPutById(base, identifierNumber, value, putByIdStatus, isDirect);
                    return;
                }
            }
        }
        
        if (m_graph.compilation())
            m_graph.compilation()->noticeInlinedPutById();

        for (const PutByIdVariant& variant : putByIdStatus.variants()) {
            m_graph.registerInferredType(variant.requiredType());
            for (Structure* structure : variant.oldStructure())
                m_graph.registerStructure(structure);
            if (variant.kind() == PutByIdVariant::Transition)
                m_graph.registerStructure(variant.newStructure());
        }
        
        MultiPutByOffsetData* data = m_graph.m_multiPutByOffsetData.add();
        data->variants = putByIdStatus.variants();
        data->identifierNumber = identifierNumber;
        addToGraph(MultiPutByOffset, OpInfo(data), base, value);
        return;
    }
    
    ASSERT(putByIdStatus.numVariants() == 1);
    const PutByIdVariant& variant = putByIdStatus[0];
    
    switch (variant.kind()) {
    case PutByIdVariant::Replace: {
        store(base, identifierNumber, variant, value);
        if (m_graph.compilation())
            m_graph.compilation()->noticeInlinedPutById();
        return;
    }
    
    case PutByIdVariant::Transition: {
        addToGraph(CheckStructure, OpInfo(m_graph.addStructureSet(variant.oldStructure())), base);
        if (!check(variant.conditionSet())) {
            emitPutById(base, identifierNumber, value, putByIdStatus, isDirect);
            return;
        }

        ASSERT(variant.oldStructureForTransition()->transitionWatchpointSetHasBeenInvalidated());
    
        Node* propertyStorage;
        Transition* transition = m_graph.m_transitions.add(
            m_graph.registerStructure(variant.oldStructureForTransition()), m_graph.registerStructure(variant.newStructure()));

        if (variant.reallocatesStorage()) {

            // If we're growing the property storage then it must be because we're
            // storing into the out-of-line storage.
            ASSERT(!isInlineOffset(variant.offset()));

            if (!variant.oldStructureForTransition()->outOfLineCapacity()) {
                propertyStorage = addToGraph(
                    AllocatePropertyStorage, OpInfo(transition), base);
            } else {
                propertyStorage = addToGraph(
                    ReallocatePropertyStorage, OpInfo(transition),
                    base, addToGraph(GetButterfly, base));
            }
        } else {
            if (isInlineOffset(variant.offset()))
                propertyStorage = base;
            else
                propertyStorage = addToGraph(GetButterfly, base);
        }

        StorageAccessData* data = m_graph.m_storageAccessData.add();
        data->offset = variant.offset();
        data->identifierNumber = identifierNumber;
        data->inferredType = variant.requiredType();
        m_graph.registerInferredType(data->inferredType);
        
        // NOTE: We could GC at this point because someone could insert an operation that GCs.
        // That's fine because:
        // - Things already in the structure will get scanned because we haven't messed with
        //   the object yet.
        // - The value we are fixing to put is going to be kept live by OSR exit handling. So
        //   if the GC does a conservative scan here it will see the new value.
        
        addToGraph(
            PutByOffset,
            OpInfo(data),
            propertyStorage,
            base,
            value);
        
        if (variant.reallocatesStorage())
            addToGraph(NukeStructureAndSetButterfly, base, propertyStorage);

        // FIXME: PutStructure goes last until we fix either
        // https://bugs.webkit.org/show_bug.cgi?id=142921 or
        // https://bugs.webkit.org/show_bug.cgi?id=142924.
        addToGraph(PutStructure, OpInfo(transition), base);

        if (m_graph.compilation())
            m_graph.compilation()->noticeInlinedPutById();
        return;
    }
        
    case PutByIdVariant::Setter: {
        Node* loadedValue = load(SpecCellOther, base, identifierNumber, variant);
        if (!loadedValue) {
            emitPutById(base, identifierNumber, value, putByIdStatus, isDirect);
            return;
        }
        
        Node* setter = addToGraph(GetSetter, loadedValue);
        
        // Make a call. We don't try to get fancy with using the smallest operand number because
        // the stack layout phase should compress the stack anyway.
    
        unsigned numberOfParameters = 0;
        numberOfParameters++; // The 'this' argument.
        numberOfParameters++; // The new value.
        numberOfParameters++; // True return PC.
    
        // Start with a register offset that corresponds to the last in-use register.
        int registerOffset = virtualRegisterForLocal(
            m_inlineStackTop->m_profiledBlock->m_numCalleeLocals - 1).offset();
        registerOffset -= numberOfParameters;
        registerOffset -= CallFrame::headerSizeInRegisters;
    
        // Get the alignment right.
        registerOffset = -WTF::roundUpToMultipleOf(
            stackAlignmentRegisters(),
            -registerOffset);
    
        ensureLocals(
            m_inlineStackTop->remapOperand(
                VirtualRegister(registerOffset)).toLocal());
    
        int nextRegister = registerOffset + CallFrame::headerSizeInRegisters;
        set(VirtualRegister(nextRegister++), base, ImmediateNakedSet);
        set(VirtualRegister(nextRegister++), value, ImmediateNakedSet);

        // We've set some locals, but they are not user-visible. It's still OK to exit from here.
        m_exitOK = true;
        addToGraph(ExitOK);
    
        handleCall(
            VirtualRegister().offset(), Call, InlineCallFrame::SetterCall,
            OPCODE_LENGTH(op_put_by_id), setter, numberOfParameters - 1, registerOffset,
            *variant.callLinkStatus(), SpecOther);
        return;
    }
    
    default: {
        emitPutById(base, identifierNumber, value, putByIdStatus, isDirect);
        return;
    } }
}

void ByteCodeParser::prepareToParseBlock()
{
    clearCaches();
    ASSERT(m_setLocalQueue.isEmpty());
}

void ByteCodeParser::clearCaches()
{
    m_constants.resize(0);
}

bool ByteCodeParser::parseBlock(unsigned limit)
{
    bool shouldContinueParsing = true;

    Interpreter* interpreter = m_vm->interpreter;
    Instruction* instructionsBegin = m_inlineStackTop->m_codeBlock->instructions().begin();
    unsigned blockBegin = m_currentIndex;

    // If we are the first basic block, introduce markers for arguments. This allows
    // us to track if a use of an argument may use the actual argument passed, as
    // opposed to using a value we set explicitly.
    if (m_currentBlock == m_graph.block(0) && !inlineCallFrame()) {
        m_graph.m_arguments.resize(m_numArguments);
        // We will emit SetArgument nodes. They don't exit, but we're at the top of an op_enter so
        // exitOK = true.
        m_exitOK = true;
        for (unsigned argument = 0; argument < m_numArguments; ++argument) {
            VariableAccessData* variable = newVariableAccessData(
                virtualRegisterForArgument(argument));
            variable->mergeStructureCheckHoistingFailed(
                m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadCache));
            variable->mergeCheckArrayHoistingFailed(
                m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadIndexingType));
            
            Node* setArgument = addToGraph(SetArgument, OpInfo(variable));
            m_graph.m_arguments[argument] = setArgument;
            m_currentBlock->variablesAtTail.setArgumentFirstTime(argument, setArgument);
        }
    }

    while (true) {
        // We're staring a new bytecode instruction. Hence, we once again have a place that we can exit
        // to.
        m_exitOK = true;
        
        processSetLocalQueue();
        
        // Don't extend over jump destinations.
        if (m_currentIndex == limit) {
            // Ordinarily we want to plant a jump. But refuse to do this if the block is
            // empty. This is a special case for inlining, which might otherwise create
            // some empty blocks in some cases. When parseBlock() returns with an empty
            // block, it will get repurposed instead of creating a new one. Note that this
            // logic relies on every bytecode resulting in one or more nodes, which would
            // be true anyway except for op_loop_hint, which emits a Phantom to force this
            // to be true.
            // We also don't insert a jump if the block already has a terminal,
            // which could happen after a tail call.
            ASSERT(m_currentBlock->isEmpty() || !m_currentBlock->terminal());
            if (!m_currentBlock->isEmpty())
                addToGraph(Jump, OpInfo(m_currentIndex));
            return shouldContinueParsing;
        }
        
        // Switch on the current bytecode opcode.
        Instruction* currentInstruction = instructionsBegin + m_currentIndex;
        m_currentInstruction = currentInstruction; // Some methods want to use this, and we'd rather not thread it through calls.
        OpcodeID opcodeID = interpreter->getOpcodeID(currentInstruction->u.opcode);
        
        if (Options::verboseDFGByteCodeParsing())
            dataLog("    parsing ", currentCodeOrigin(), ": ", opcodeID, "\n");
        
        if (m_graph.compilation()) {
            addToGraph(CountExecution, OpInfo(m_graph.compilation()->executionCounterFor(
                Profiler::OriginStack(*m_vm->m_perBytecodeProfiler, m_codeBlock, currentCodeOrigin()))));
        }
        
        switch (opcodeID) {

        // === Function entry opcodes ===

        case op_enter: {
            Node* undefined = addToGraph(JSConstant, OpInfo(m_constantUndefined));
            // Initialize all locals to undefined.
            for (int i = 0; i < m_inlineStackTop->m_codeBlock->m_numVars; ++i)
                set(virtualRegisterForLocal(i), undefined, ImmediateNakedSet);
            NEXT_OPCODE(op_enter);
        }
            
        case op_to_this: {
            Node* op1 = getThis();
            if (op1->op() != ToThis) {
                Structure* cachedStructure = currentInstruction[2].u.structure.get();
                if (currentInstruction[2].u.toThisStatus != ToThisOK
                    || !cachedStructure
                    || cachedStructure->classInfo()->methodTable.toThis != JSObject::info()->methodTable.toThis
                    || m_inlineStackTop->m_profiledBlock->couldTakeSlowCase(m_currentIndex)
                    || m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadCache)
                    || (op1->op() == GetLocal && op1->variableAccessData()->structureCheckHoistingFailed())) {
                    setThis(addToGraph(ToThis, op1));
                } else {
                    addToGraph(
                        CheckStructure,
                        OpInfo(m_graph.addStructureSet(cachedStructure)),
                        op1);
                }
            }
            NEXT_OPCODE(op_to_this);
        }

        case op_create_this: {
            int calleeOperand = currentInstruction[2].u.operand;
            Node* callee = get(VirtualRegister(calleeOperand));

            JSFunction* function = callee->dynamicCastConstant<JSFunction*>(*m_vm);
            if (!function) {
                JSCell* cachedFunction = currentInstruction[4].u.jsCell.unvalidatedGet();
                if (cachedFunction
                    && cachedFunction != JSCell::seenMultipleCalleeObjects()
                    && !m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadCell)) {
                    ASSERT(cachedFunction->inherits(*m_vm, JSFunction::info()));

                    FrozenValue* frozen = m_graph.freeze(cachedFunction);
                    addToGraph(CheckCell, OpInfo(frozen), callee);

                    function = static_cast<JSFunction*>(cachedFunction);
                }
            }

            bool alreadyEmitted = false;
            if (function) {
                if (FunctionRareData* rareData = function->rareData()) {
                    if (Structure* structure = rareData->objectAllocationStructure()) {
                        m_graph.freeze(rareData);
                        m_graph.watchpoints().addLazily(rareData->allocationProfileWatchpointSet());
                        // The callee is still live up to this point.
                        addToGraph(Phantom, callee);
                        set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(NewObject, OpInfo(m_graph.registerStructure(structure))));
                        alreadyEmitted = true;
                    }
                }
            }
            if (!alreadyEmitted) {
                set(VirtualRegister(currentInstruction[1].u.operand),
                    addToGraph(CreateThis, OpInfo(currentInstruction[3].u.operand), callee));
            }
            NEXT_OPCODE(op_create_this);
        }

        case op_new_object: {
            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(NewObject,
                    OpInfo(m_graph.registerStructure(currentInstruction[3].u.objectAllocationProfile->structure()))));
            NEXT_OPCODE(op_new_object);
        }
            
        case op_new_array: {
            int startOperand = currentInstruction[2].u.operand;
            int numOperands = currentInstruction[3].u.operand;
            ArrayAllocationProfile* profile = currentInstruction[4].u.arrayAllocationProfile;
            for (int operandIdx = startOperand; operandIdx > startOperand - numOperands; --operandIdx)
                addVarArgChild(get(VirtualRegister(operandIdx)));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(Node::VarArg, NewArray, OpInfo(profile->selectIndexingType()), OpInfo(0)));
            NEXT_OPCODE(op_new_array);
        }

        case op_new_array_with_spread: {
            int startOperand = currentInstruction[2].u.operand;
            int numOperands = currentInstruction[3].u.operand;
            const BitVector& bitVector = m_inlineStackTop->m_profiledBlock->unlinkedCodeBlock()->bitVector(currentInstruction[4].u.unsignedValue);
            for (int operandIdx = startOperand; operandIdx > startOperand - numOperands; --operandIdx)
                addVarArgChild(get(VirtualRegister(operandIdx)));

            BitVector* copy = m_graph.m_bitVectors.add(bitVector);
            ASSERT(*copy == bitVector);

            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(Node::VarArg, NewArrayWithSpread, OpInfo(copy)));
            NEXT_OPCODE(op_new_array_with_spread);
        }

        case op_spread: {
            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(Spread, get(VirtualRegister(currentInstruction[2].u.operand))));
            NEXT_OPCODE(op_spread);
        }
            
        case op_new_array_with_size: {
            int lengthOperand = currentInstruction[2].u.operand;
            ArrayAllocationProfile* profile = currentInstruction[3].u.arrayAllocationProfile;
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(NewArrayWithSize, OpInfo(profile->selectIndexingType()), get(VirtualRegister(lengthOperand))));
            NEXT_OPCODE(op_new_array_with_size);
        }
            
        case op_new_array_buffer: {
            int startConstant = currentInstruction[2].u.operand;
            int numConstants = currentInstruction[3].u.operand;
            ArrayAllocationProfile* profile = currentInstruction[4].u.arrayAllocationProfile;
            NewArrayBufferData data;
            data.startConstant = m_inlineStackTop->m_constantBufferRemap[startConstant];
            data.numConstants = numConstants;
            data.indexingType = profile->selectIndexingType();

            // If this statement has never executed, we'll have the wrong indexing type in the profile.
            for (int i = 0; i < numConstants; ++i) {
                data.indexingType =
                    leastUpperBoundOfIndexingTypeAndValue(
                        data.indexingType,
                        m_codeBlock->constantBuffer(data.startConstant)[i]);
            }
            
            m_graph.m_newArrayBufferData.append(data);
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(NewArrayBuffer, OpInfo(&m_graph.m_newArrayBufferData.last())));
            NEXT_OPCODE(op_new_array_buffer);
        }
            
        case op_new_regexp: {
            RegExp* regexp = m_inlineStackTop->m_codeBlock->regexp(currentInstruction[2].u.operand);
            FrozenValue* frozen = m_graph.freezeStrong(regexp);
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(NewRegexp, OpInfo(frozen)));
            NEXT_OPCODE(op_new_regexp);
        }

        case op_get_rest_length: {
            InlineCallFrame* inlineCallFrame = this->inlineCallFrame();
            Node* length;
            if (inlineCallFrame && !inlineCallFrame->isVarargs()) {
                unsigned argumentsLength = inlineCallFrame->arguments.size() - 1;
                unsigned numParamsToSkip = currentInstruction[2].u.unsignedValue;
                JSValue restLength;
                if (argumentsLength <= numParamsToSkip)
                    restLength = jsNumber(0);
                else
                    restLength = jsNumber(argumentsLength - numParamsToSkip);

                length = jsConstant(restLength);
            } else
                length = addToGraph(GetRestLength, OpInfo(currentInstruction[2].u.unsignedValue));
            set(VirtualRegister(currentInstruction[1].u.operand), length);
            NEXT_OPCODE(op_get_rest_length);
        }

        case op_create_rest: {
            noticeArgumentsUse();
            Node* arrayLength = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(CreateRest, OpInfo(currentInstruction[3].u.unsignedValue), arrayLength));
            NEXT_OPCODE(op_create_rest);
        }
            
        // === Bitwise operations ===

        case op_bitand: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(BitAnd, op1, op2));
            NEXT_OPCODE(op_bitand);
        }

        case op_bitor: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(BitOr, op1, op2));
            NEXT_OPCODE(op_bitor);
        }

        case op_bitxor: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(BitXor, op1, op2));
            NEXT_OPCODE(op_bitxor);
        }

        case op_rshift: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(BitRShift, op1, op2));
            NEXT_OPCODE(op_rshift);
        }

        case op_lshift: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(BitLShift, op1, op2));
            NEXT_OPCODE(op_lshift);
        }

        case op_urshift: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(BitURShift, op1, op2));
            NEXT_OPCODE(op_urshift);
        }
            
        case op_unsigned: {
            set(VirtualRegister(currentInstruction[1].u.operand),
                makeSafe(addToGraph(UInt32ToNumber, get(VirtualRegister(currentInstruction[2].u.operand)))));
            NEXT_OPCODE(op_unsigned);
        }

        // === Increment/Decrement opcodes ===

        case op_inc: {
            int srcDst = currentInstruction[1].u.operand;
            VirtualRegister srcDstVirtualRegister = VirtualRegister(srcDst);
            Node* op = get(srcDstVirtualRegister);
            set(srcDstVirtualRegister, makeSafe(addToGraph(ArithAdd, op, addToGraph(JSConstant, OpInfo(m_constantOne)))));
            NEXT_OPCODE(op_inc);
        }

        case op_dec: {
            int srcDst = currentInstruction[1].u.operand;
            VirtualRegister srcDstVirtualRegister = VirtualRegister(srcDst);
            Node* op = get(srcDstVirtualRegister);
            set(srcDstVirtualRegister, makeSafe(addToGraph(ArithSub, op, addToGraph(JSConstant, OpInfo(m_constantOne)))));
            NEXT_OPCODE(op_dec);
        }

        // === Arithmetic operations ===

        case op_add: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            if (op1->hasNumberResult() && op2->hasNumberResult())
                set(VirtualRegister(currentInstruction[1].u.operand), makeSafe(addToGraph(ArithAdd, op1, op2)));
            else
                set(VirtualRegister(currentInstruction[1].u.operand), makeSafe(addToGraph(ValueAdd, op1, op2)));
            NEXT_OPCODE(op_add);
        }

        case op_sub: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), makeSafe(addToGraph(ArithSub, op1, op2)));
            NEXT_OPCODE(op_sub);
        }

        case op_negate: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), makeSafe(addToGraph(ArithNegate, op1)));
            NEXT_OPCODE(op_negate);
        }

        case op_mul: {
            // Multiply requires that the inputs are not truncated, unfortunately.
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), makeSafe(addToGraph(ArithMul, op1, op2)));
            NEXT_OPCODE(op_mul);
        }

        case op_mod: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), makeSafe(addToGraph(ArithMod, op1, op2)));
            NEXT_OPCODE(op_mod);
        }

        case op_pow: {
            // FIXME: ArithPow(Untyped, Untyped) should be supported as the same to ArithMul, ArithSub etc.
            // https://bugs.webkit.org/show_bug.cgi?id=160012
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(ArithPow, op1, op2));
            NEXT_OPCODE(op_pow);
        }

        case op_div: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), makeDivSafe(addToGraph(ArithDiv, op1, op2)));
            NEXT_OPCODE(op_div);
        }

        // === Misc operations ===

        case op_debug: {
            // This is a nop in the DFG/FTL because when we set a breakpoint in the debugger,
            // we will jettison all optimized CodeBlocks that contains the breakpoint.
            addToGraph(Check); // We add a nop here so that basic block linking doesn't break.
            NEXT_OPCODE(op_debug);
        }

        case op_mov: {
            Node* op = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), op);
            NEXT_OPCODE(op_mov);
        }

        case op_check_tdz: {
            addToGraph(CheckNotEmpty, get(VirtualRegister(currentInstruction[1].u.operand)));
            NEXT_OPCODE(op_check_tdz);
        }

        case op_overrides_has_instance: {
            JSFunction* defaultHasInstanceSymbolFunction = m_inlineStackTop->m_codeBlock->globalObjectFor(currentCodeOrigin())->functionProtoHasInstanceSymbolFunction();

            Node* constructor = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* hasInstanceValue = get(VirtualRegister(currentInstruction[3].u.operand));

            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(OverridesHasInstance, OpInfo(m_graph.freeze(defaultHasInstanceSymbolFunction)), constructor, hasInstanceValue));
            NEXT_OPCODE(op_overrides_has_instance);
        }

        case op_instanceof: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* prototype = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(InstanceOf, value, prototype));
            NEXT_OPCODE(op_instanceof);
        }

        case op_instanceof_custom: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* constructor = get(VirtualRegister(currentInstruction[3].u.operand));
            Node* hasInstanceValue = get(VirtualRegister(currentInstruction[4].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(InstanceOfCustom, value, constructor, hasInstanceValue));
            NEXT_OPCODE(op_instanceof_custom);
        }
        case op_is_empty: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(IsEmpty, value));
            NEXT_OPCODE(op_is_empty);
        }
        case op_is_undefined: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(IsUndefined, value));
            NEXT_OPCODE(op_is_undefined);
        }

        case op_is_boolean: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(IsBoolean, value));
            NEXT_OPCODE(op_is_boolean);
        }

        case op_is_number: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(IsNumber, value));
            NEXT_OPCODE(op_is_number);
        }

        case op_is_cell_with_type: {
            JSType type = static_cast<JSType>(currentInstruction[3].u.operand);
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(IsCellWithType, OpInfo(type), value));
            NEXT_OPCODE(op_is_cell_with_type);
        }

        case op_is_object: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(IsObject, value));
            NEXT_OPCODE(op_is_object);
        }

        case op_is_object_or_null: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(IsObjectOrNull, value));
            NEXT_OPCODE(op_is_object_or_null);
        }

        case op_is_function: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(IsFunction, value));
            NEXT_OPCODE(op_is_function);
        }

        case op_not: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(LogicalNot, value));
            NEXT_OPCODE(op_not);
        }
            
        case op_to_primitive: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(ToPrimitive, value));
            NEXT_OPCODE(op_to_primitive);
        }
            
        case op_strcat: {
            int startOperand = currentInstruction[2].u.operand;
            int numOperands = currentInstruction[3].u.operand;
#if CPU(X86)
            // X86 doesn't have enough registers to compile MakeRope with three arguments. The
            // StrCat we emit here may be turned into a MakeRope. Rather than try to be clever,
            // we just make StrCat dumber on this processor.
            const unsigned maxArguments = 2;
#else
            const unsigned maxArguments = 3;
#endif
            Node* operands[AdjacencyList::Size];
            unsigned indexInOperands = 0;
            for (unsigned i = 0; i < AdjacencyList::Size; ++i)
                operands[i] = 0;
            for (int operandIdx = 0; operandIdx < numOperands; ++operandIdx) {
                if (indexInOperands == maxArguments) {
                    operands[0] = addToGraph(StrCat, operands[0], operands[1], operands[2]);
                    for (unsigned i = 1; i < AdjacencyList::Size; ++i)
                        operands[i] = 0;
                    indexInOperands = 1;
                }
                
                ASSERT(indexInOperands < AdjacencyList::Size);
                ASSERT(indexInOperands < maxArguments);
                operands[indexInOperands++] = get(VirtualRegister(startOperand - operandIdx));
            }
            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(StrCat, operands[0], operands[1], operands[2]));
            NEXT_OPCODE(op_strcat);
        }

        case op_less: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(CompareLess, op1, op2));
            NEXT_OPCODE(op_less);
        }

        case op_lesseq: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(CompareLessEq, op1, op2));
            NEXT_OPCODE(op_lesseq);
        }

        case op_greater: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(CompareGreater, op1, op2));
            NEXT_OPCODE(op_greater);
        }

        case op_greatereq: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(CompareGreaterEq, op1, op2));
            NEXT_OPCODE(op_greatereq);
        }

        case op_eq: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(CompareEq, op1, op2));
            NEXT_OPCODE(op_eq);
        }

        case op_eq_null: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* nullConstant = addToGraph(JSConstant, OpInfo(m_constantNull));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(CompareEq, value, nullConstant));
            NEXT_OPCODE(op_eq_null);
        }

        case op_stricteq: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(CompareStrictEq, op1, op2));
            NEXT_OPCODE(op_stricteq);
        }

        case op_neq: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(LogicalNot, addToGraph(CompareEq, op1, op2)));
            NEXT_OPCODE(op_neq);
        }

        case op_neq_null: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* nullConstant = addToGraph(JSConstant, OpInfo(m_constantNull));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(LogicalNot, addToGraph(CompareEq, value, nullConstant)));
            NEXT_OPCODE(op_neq_null);
        }

        case op_nstricteq: {
            Node* op1 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[3].u.operand));
            Node* invertedResult;
            invertedResult = addToGraph(CompareStrictEq, op1, op2);
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(LogicalNot, invertedResult));
            NEXT_OPCODE(op_nstricteq);
        }

        // === Property access operations ===

        case op_get_by_val: {
            SpeculatedType prediction = getPredictionWithoutOSRExit();

            Node* base = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* property = get(VirtualRegister(currentInstruction[3].u.operand));
            bool compiledAsGetById = false;
            GetByIdStatus getByIdStatus;
            unsigned identifierNumber = 0;
            {
                ConcurrentJSLocker locker(m_inlineStackTop->m_profiledBlock->m_lock);
                ByValInfo* byValInfo = m_inlineStackTop->m_byValInfos.get(CodeOrigin(currentCodeOrigin().bytecodeIndex));
                // FIXME: When the bytecode is not compiled in the baseline JIT, byValInfo becomes null.
                // At that time, there is no information.
                if (byValInfo && byValInfo->stubInfo && !byValInfo->tookSlowPath && !m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadIdent) && !m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadCell)) {
                    compiledAsGetById = true;
                    identifierNumber = m_graph.identifiers().ensure(byValInfo->cachedId.impl());
                    UniquedStringImpl* uid = m_graph.identifiers()[identifierNumber];

                    if (Symbol* symbol = byValInfo->cachedSymbol.get()) {
                        FrozenValue* frozen = m_graph.freezeStrong(symbol);
                        addToGraph(CheckCell, OpInfo(frozen), property);
                    } else {
                        ASSERT(!uid->isSymbol());
                        addToGraph(CheckStringIdent, OpInfo(uid), property);
                    }

                    getByIdStatus = GetByIdStatus::computeForStubInfo(
                        locker, m_inlineStackTop->m_profiledBlock,
                        byValInfo->stubInfo, currentCodeOrigin(), uid);
                }
            }

            if (compiledAsGetById)
                handleGetById(currentInstruction[1].u.operand, prediction, base, identifierNumber, getByIdStatus, AccessType::Get, OPCODE_LENGTH(op_get_by_val));
            else {
                ArrayMode arrayMode = getArrayMode(currentInstruction[4].u.arrayProfile, Array::Read);
                Node* getByVal = addToGraph(GetByVal, OpInfo(arrayMode.asWord()), OpInfo(prediction), base, property);
                m_exitOK = false; // GetByVal must be treated as if it clobbers exit state, since FixupPhase may make it generic.
                set(VirtualRegister(currentInstruction[1].u.operand), getByVal);
            }

            NEXT_OPCODE(op_get_by_val);
        }

        case op_get_by_val_with_this: {
            SpeculatedType prediction = getPrediction();

            Node* base = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* thisValue = get(VirtualRegister(currentInstruction[3].u.operand));
            Node* property = get(VirtualRegister(currentInstruction[4].u.operand));
            Node* getByValWithThis = addToGraph(GetByValWithThis, OpInfo(), OpInfo(prediction), base, thisValue, property);
            set(VirtualRegister(currentInstruction[1].u.operand), getByValWithThis);

            NEXT_OPCODE(op_get_by_val_with_this);
        }

        case op_put_by_val_direct:
        case op_put_by_val: {
            Node* base = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* property = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* value = get(VirtualRegister(currentInstruction[3].u.operand));
            bool isDirect = opcodeID == op_put_by_val_direct;
            bool compiledAsPutById = false;
            {
                unsigned identifierNumber = std::numeric_limits<unsigned>::max();
                PutByIdStatus putByIdStatus;
                {
                    ConcurrentJSLocker locker(m_inlineStackTop->m_profiledBlock->m_lock);
                    ByValInfo* byValInfo = m_inlineStackTop->m_byValInfos.get(CodeOrigin(currentCodeOrigin().bytecodeIndex));
                    // FIXME: When the bytecode is not compiled in the baseline JIT, byValInfo becomes null.
                    // At that time, there is no information.
                    if (byValInfo 
                        && byValInfo->stubInfo
                        && !byValInfo->tookSlowPath
                        && !m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadIdent)
                        && !m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadType)
                        && !m_inlineStackTop->m_exitProfile.hasExitSite(m_currentIndex, BadCell)) {
                        compiledAsPutById = true;
                        identifierNumber = m_graph.identifiers().ensure(byValInfo->cachedId.impl());
                        UniquedStringImpl* uid = m_graph.identifiers()[identifierNumber];

                        if (Symbol* symbol = byValInfo->cachedSymbol.get()) {
                            FrozenValue* frozen = m_graph.freezeStrong(symbol);
                            addToGraph(CheckCell, OpInfo(frozen), property);
                        } else {
                            ASSERT(!uid->isSymbol());
                            addToGraph(CheckStringIdent, OpInfo(uid), property);
                        }

                        putByIdStatus = PutByIdStatus::computeForStubInfo(
                            locker, m_inlineStackTop->m_profiledBlock,
                            byValInfo->stubInfo, currentCodeOrigin(), uid);

                    }
                }

                if (compiledAsPutById)
                    handlePutById(base, identifierNumber, value, putByIdStatus, isDirect);
            }

            if (!compiledAsPutById) {
                ArrayMode arrayMode = getArrayMode(currentInstruction[4].u.arrayProfile, Array::Write);

                addVarArgChild(base);
                addVarArgChild(property);
                addVarArgChild(value);
                addVarArgChild(0); // Leave room for property storage.
                addVarArgChild(0); // Leave room for length.
                addToGraph(Node::VarArg, isDirect ? PutByValDirect : PutByVal, OpInfo(arrayMode.asWord()), OpInfo(0));
            }

            NEXT_OPCODE(op_put_by_val);
        }

        case op_put_by_val_with_this: {
            Node* base = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* thisValue = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* property = get(VirtualRegister(currentInstruction[3].u.operand));
            Node* value = get(VirtualRegister(currentInstruction[4].u.operand));

            addVarArgChild(base);
            addVarArgChild(thisValue);
            addVarArgChild(property);
            addVarArgChild(value);
            addToGraph(Node::VarArg, PutByValWithThis, OpInfo(0), OpInfo(0));

            NEXT_OPCODE(op_put_by_val_with_this);
        }

        case op_define_data_property: {
            Node* base = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* property = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* value = get(VirtualRegister(currentInstruction[3].u.operand));
            Node* attributes = get(VirtualRegister(currentInstruction[4].u.operand));

            addVarArgChild(base);
            addVarArgChild(property);
            addVarArgChild(value);
            addVarArgChild(attributes);
            addToGraph(Node::VarArg, DefineDataProperty, OpInfo(0), OpInfo(0));

            NEXT_OPCODE(op_define_data_property);
        }

        case op_define_accessor_property: {
            Node* base = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* property = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* getter = get(VirtualRegister(currentInstruction[3].u.operand));
            Node* setter = get(VirtualRegister(currentInstruction[4].u.operand));
            Node* attributes = get(VirtualRegister(currentInstruction[5].u.operand));

            addVarArgChild(base);
            addVarArgChild(property);
            addVarArgChild(getter);
            addVarArgChild(setter);
            addVarArgChild(attributes);
            addToGraph(Node::VarArg, DefineAccessorProperty, OpInfo(0), OpInfo(0));

            NEXT_OPCODE(op_define_accessor_property);
        }

        case op_try_get_by_id:
        case op_get_by_id:
        case op_get_by_id_proto_load:
        case op_get_by_id_unset:
        case op_get_array_length: {
            SpeculatedType prediction = getPrediction();
            
            Node* base = get(VirtualRegister(currentInstruction[2].u.operand));
            unsigned identifierNumber = m_inlineStackTop->m_identifierRemap[currentInstruction[3].u.operand];
            
            UniquedStringImpl* uid = m_graph.identifiers()[identifierNumber];
            GetByIdStatus getByIdStatus = GetByIdStatus::computeFor(
                m_inlineStackTop->m_profiledBlock, m_dfgCodeBlock,
                m_inlineStackTop->m_stubInfos, m_dfgStubInfos,
                currentCodeOrigin(), uid);
            AccessType type = op_try_get_by_id == opcodeID ? AccessType::TryGet : AccessType::Get;

            unsigned opcodeLength = opcodeID == op_try_get_by_id ? OPCODE_LENGTH(op_try_get_by_id) : OPCODE_LENGTH(op_get_by_id);

            handleGetById(
                currentInstruction[1].u.operand, prediction, base, identifierNumber, getByIdStatus, type, opcodeLength);

            if (op_try_get_by_id == opcodeID)
                NEXT_OPCODE(op_try_get_by_id); // Opcode's length is different from others in this case.
            else
                NEXT_OPCODE(op_get_by_id);
        }
        case op_get_by_id_with_this: {
            SpeculatedType prediction = getPrediction();

            Node* base = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* thisValue = get(VirtualRegister(currentInstruction[3].u.operand));
            unsigned identifierNumber = m_inlineStackTop->m_identifierRemap[currentInstruction[4].u.operand];

            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(GetByIdWithThis, OpInfo(identifierNumber), OpInfo(prediction), base, thisValue));

            NEXT_OPCODE(op_get_by_id_with_this);
        }
        case op_put_by_id: {
            Node* value = get(VirtualRegister(currentInstruction[3].u.operand));
            Node* base = get(VirtualRegister(currentInstruction[1].u.operand));
            unsigned identifierNumber = m_inlineStackTop->m_identifierRemap[currentInstruction[2].u.operand];
            bool direct = currentInstruction[8].u.putByIdFlags & PutByIdIsDirect;

            PutByIdStatus putByIdStatus = PutByIdStatus::computeFor(
                m_inlineStackTop->m_profiledBlock, m_dfgCodeBlock,
                m_inlineStackTop->m_stubInfos, m_dfgStubInfos,
                currentCodeOrigin(), m_graph.identifiers()[identifierNumber]);
            
            handlePutById(base, identifierNumber, value, putByIdStatus, direct);
            NEXT_OPCODE(op_put_by_id);
        }

        case op_put_by_id_with_this: {
            Node* base = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* thisValue = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* value = get(VirtualRegister(currentInstruction[4].u.operand));
            unsigned identifierNumber = m_inlineStackTop->m_identifierRemap[currentInstruction[3].u.operand];

            addToGraph(PutByIdWithThis, OpInfo(identifierNumber), base, thisValue, value);
            NEXT_OPCODE(op_put_by_id_with_this);
        }

        case op_put_getter_by_id:
        case op_put_setter_by_id: {
            Node* base = get(VirtualRegister(currentInstruction[1].u.operand));
            unsigned identifierNumber = m_inlineStackTop->m_identifierRemap[currentInstruction[2].u.operand];
            unsigned attributes = currentInstruction[3].u.operand;
            Node* accessor = get(VirtualRegister(currentInstruction[4].u.operand));
            NodeType op = (opcodeID == op_put_getter_by_id) ? PutGetterById : PutSetterById;
            addToGraph(op, OpInfo(identifierNumber), OpInfo(attributes), base, accessor);
            NEXT_OPCODE(op_put_getter_by_id);
        }

        case op_put_getter_setter_by_id: {
            Node* base = get(VirtualRegister(currentInstruction[1].u.operand));
            unsigned identifierNumber = m_inlineStackTop->m_identifierRemap[currentInstruction[2].u.operand];
            unsigned attributes = currentInstruction[3].u.operand;
            Node* getter = get(VirtualRegister(currentInstruction[4].u.operand));
            Node* setter = get(VirtualRegister(currentInstruction[5].u.operand));
            addToGraph(PutGetterSetterById, OpInfo(identifierNumber), OpInfo(attributes), base, getter, setter);
            NEXT_OPCODE(op_put_getter_setter_by_id);
        }

        case op_put_getter_by_val:
        case op_put_setter_by_val: {
            Node* base = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* subscript = get(VirtualRegister(currentInstruction[2].u.operand));
            unsigned attributes = currentInstruction[3].u.operand;
            Node* accessor = get(VirtualRegister(currentInstruction[4].u.operand));
            NodeType op = (opcodeID == op_put_getter_by_val) ? PutGetterByVal : PutSetterByVal;
            addToGraph(op, OpInfo(attributes), base, subscript, accessor);
            NEXT_OPCODE(op_put_getter_by_val);
        }

        case op_del_by_id: {
            Node* base = get(VirtualRegister(currentInstruction[2].u.operand));
            unsigned identifierNumber = m_inlineStackTop->m_identifierRemap[currentInstruction[3].u.operand];
            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(DeleteById, OpInfo(identifierNumber), base));
            NEXT_OPCODE(op_del_by_id);
        }

        case op_del_by_val: {
            int dst = currentInstruction[1].u.operand;
            Node* base = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* key = get(VirtualRegister(currentInstruction[3].u.operand));
            set(VirtualRegister(dst), addToGraph(DeleteByVal, base, key));
            NEXT_OPCODE(op_del_by_val);
        }

        case op_profile_type: {
            Node* valueToProfile = get(VirtualRegister(currentInstruction[1].u.operand));
            addToGraph(ProfileType, OpInfo(currentInstruction[2].u.location), valueToProfile);
            NEXT_OPCODE(op_profile_type);
        }

        case op_profile_control_flow: {
            BasicBlockLocation* basicBlockLocation = currentInstruction[1].u.basicBlockLocation;
            addToGraph(ProfileControlFlow, OpInfo(basicBlockLocation));
            NEXT_OPCODE(op_profile_control_flow);
        }

        // === Block terminators. ===

        case op_jmp: {
            ASSERT(!m_currentBlock->terminal());
            int relativeOffset = currentInstruction[1].u.operand;
            addToGraph(Jump, OpInfo(m_currentIndex + relativeOffset));
            if (relativeOffset <= 0)
                flushForTerminal();
            LAST_OPCODE(op_jmp);
        }

        case op_jtrue: {
            unsigned relativeOffset = currentInstruction[2].u.operand;
            Node* condition = get(VirtualRegister(currentInstruction[1].u.operand));
            addToGraph(Branch, OpInfo(branchData(m_currentIndex + relativeOffset, m_currentIndex + OPCODE_LENGTH(op_jtrue))), condition);
            LAST_OPCODE(op_jtrue);
        }

        case op_jfalse: {
            unsigned relativeOffset = currentInstruction[2].u.operand;
            Node* condition = get(VirtualRegister(currentInstruction[1].u.operand));
            addToGraph(Branch, OpInfo(branchData(m_currentIndex + OPCODE_LENGTH(op_jfalse), m_currentIndex + relativeOffset)), condition);
            LAST_OPCODE(op_jfalse);
        }

        case op_jeq_null: {
            unsigned relativeOffset = currentInstruction[2].u.operand;
            Node* value = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* nullConstant = addToGraph(JSConstant, OpInfo(m_constantNull));
            Node* condition = addToGraph(CompareEq, value, nullConstant);
            addToGraph(Branch, OpInfo(branchData(m_currentIndex + relativeOffset, m_currentIndex + OPCODE_LENGTH(op_jeq_null))), condition);
            LAST_OPCODE(op_jeq_null);
        }

        case op_jneq_null: {
            unsigned relativeOffset = currentInstruction[2].u.operand;
            Node* value = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* nullConstant = addToGraph(JSConstant, OpInfo(m_constantNull));
            Node* condition = addToGraph(CompareEq, value, nullConstant);
            addToGraph(Branch, OpInfo(branchData(m_currentIndex + OPCODE_LENGTH(op_jneq_null), m_currentIndex + relativeOffset)), condition);
            LAST_OPCODE(op_jneq_null);
        }

        case op_jless: {
            unsigned relativeOffset = currentInstruction[3].u.operand;
            Node* op1 = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* condition = addToGraph(CompareLess, op1, op2);
            addToGraph(Branch, OpInfo(branchData(m_currentIndex + relativeOffset, m_currentIndex + OPCODE_LENGTH(op_jless))), condition);
            LAST_OPCODE(op_jless);
        }

        case op_jlesseq: {
            unsigned relativeOffset = currentInstruction[3].u.operand;
            Node* op1 = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* condition = addToGraph(CompareLessEq, op1, op2);
            addToGraph(Branch, OpInfo(branchData(m_currentIndex + relativeOffset, m_currentIndex + OPCODE_LENGTH(op_jlesseq))), condition);
            LAST_OPCODE(op_jlesseq);
        }

        case op_jgreater: {
            unsigned relativeOffset = currentInstruction[3].u.operand;
            Node* op1 = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* condition = addToGraph(CompareGreater, op1, op2);
            addToGraph(Branch, OpInfo(branchData(m_currentIndex + relativeOffset, m_currentIndex + OPCODE_LENGTH(op_jgreater))), condition);
            LAST_OPCODE(op_jgreater);
        }

        case op_jgreatereq: {
            unsigned relativeOffset = currentInstruction[3].u.operand;
            Node* op1 = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* condition = addToGraph(CompareGreaterEq, op1, op2);
            addToGraph(Branch, OpInfo(branchData(m_currentIndex + relativeOffset, m_currentIndex + OPCODE_LENGTH(op_jgreatereq))), condition);
            LAST_OPCODE(op_jgreatereq);
        }

        case op_jnless: {
            unsigned relativeOffset = currentInstruction[3].u.operand;
            Node* op1 = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* condition = addToGraph(CompareLess, op1, op2);
            addToGraph(Branch, OpInfo(branchData(m_currentIndex + OPCODE_LENGTH(op_jnless), m_currentIndex + relativeOffset)), condition);
            LAST_OPCODE(op_jnless);
        }

        case op_jnlesseq: {
            unsigned relativeOffset = currentInstruction[3].u.operand;
            Node* op1 = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* condition = addToGraph(CompareLessEq, op1, op2);
            addToGraph(Branch, OpInfo(branchData(m_currentIndex + OPCODE_LENGTH(op_jnlesseq), m_currentIndex + relativeOffset)), condition);
            LAST_OPCODE(op_jnlesseq);
        }

        case op_jngreater: {
            unsigned relativeOffset = currentInstruction[3].u.operand;
            Node* op1 = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* condition = addToGraph(CompareGreater, op1, op2);
            addToGraph(Branch, OpInfo(branchData(m_currentIndex + OPCODE_LENGTH(op_jngreater), m_currentIndex + relativeOffset)), condition);
            LAST_OPCODE(op_jngreater);
        }

        case op_jngreatereq: {
            unsigned relativeOffset = currentInstruction[3].u.operand;
            Node* op1 = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* op2 = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* condition = addToGraph(CompareGreaterEq, op1, op2);
            addToGraph(Branch, OpInfo(branchData(m_currentIndex + OPCODE_LENGTH(op_jngreatereq), m_currentIndex + relativeOffset)), condition);
            LAST_OPCODE(op_jngreatereq);
        }
            
        case op_switch_imm: {
            SwitchData& data = *m_graph.m_switchData.add();
            data.kind = SwitchImm;
            data.switchTableIndex = m_inlineStackTop->m_switchRemap[currentInstruction[1].u.operand];
            data.fallThrough.setBytecodeIndex(m_currentIndex + currentInstruction[2].u.operand);
            SimpleJumpTable& table = m_codeBlock->switchJumpTable(data.switchTableIndex);
            for (unsigned i = 0; i < table.branchOffsets.size(); ++i) {
                if (!table.branchOffsets[i])
                    continue;
                unsigned target = m_currentIndex + table.branchOffsets[i];
                if (target == data.fallThrough.bytecodeIndex())
                    continue;
                data.cases.append(SwitchCase::withBytecodeIndex(m_graph.freeze(jsNumber(static_cast<int32_t>(table.min + i))), target));
            }
            addToGraph(Switch, OpInfo(&data), get(VirtualRegister(currentInstruction[3].u.operand)));
            flushIfTerminal(data);
            LAST_OPCODE(op_switch_imm);
        }
            
        case op_switch_char: {
            SwitchData& data = *m_graph.m_switchData.add();
            data.kind = SwitchChar;
            data.switchTableIndex = m_inlineStackTop->m_switchRemap[currentInstruction[1].u.operand];
            data.fallThrough.setBytecodeIndex(m_currentIndex + currentInstruction[2].u.operand);
            SimpleJumpTable& table = m_codeBlock->switchJumpTable(data.switchTableIndex);
            for (unsigned i = 0; i < table.branchOffsets.size(); ++i) {
                if (!table.branchOffsets[i])
                    continue;
                unsigned target = m_currentIndex + table.branchOffsets[i];
                if (target == data.fallThrough.bytecodeIndex())
                    continue;
                data.cases.append(
                    SwitchCase::withBytecodeIndex(LazyJSValue::singleCharacterString(table.min + i), target));
            }
            addToGraph(Switch, OpInfo(&data), get(VirtualRegister(currentInstruction[3].u.operand)));
            flushIfTerminal(data);
            LAST_OPCODE(op_switch_char);
        }

        case op_switch_string: {
            SwitchData& data = *m_graph.m_switchData.add();
            data.kind = SwitchString;
            data.switchTableIndex = currentInstruction[1].u.operand;
            data.fallThrough.setBytecodeIndex(m_currentIndex + currentInstruction[2].u.operand);
            StringJumpTable& table = m_codeBlock->stringSwitchJumpTable(data.switchTableIndex);
            StringJumpTable::StringOffsetTable::iterator iter;
            StringJumpTable::StringOffsetTable::iterator end = table.offsetTable.end();
            for (iter = table.offsetTable.begin(); iter != end; ++iter) {
                unsigned target = m_currentIndex + iter->value.branchOffset;
                if (target == data.fallThrough.bytecodeIndex())
                    continue;
                data.cases.append(
                    SwitchCase::withBytecodeIndex(LazyJSValue::knownStringImpl(iter->key.get()), target));
            }
            addToGraph(Switch, OpInfo(&data), get(VirtualRegister(currentInstruction[3].u.operand)));
            flushIfTerminal(data);
            LAST_OPCODE(op_switch_string);
        }

        case op_ret:
            ASSERT(!m_currentBlock->terminal());
            if (inlineCallFrame()) {
                flushForReturn();
                if (m_inlineStackTop->m_returnValue.isValid())
                    setDirect(m_inlineStackTop->m_returnValue, get(VirtualRegister(currentInstruction[1].u.operand)), ImmediateSetWithFlush);
                m_inlineStackTop->m_didReturn = true;
                if (m_inlineStackTop->m_unlinkedBlocks.isEmpty()) {
                    // If we're returning from the first block, then we're done parsing.
                    ASSERT(m_inlineStackTop->m_callsiteBlockHead == m_graph.lastBlock());
                    shouldContinueParsing = false;
                    LAST_OPCODE(op_ret);
                } else {
                    // If inlining created blocks, and we're doing a return, then we need some
                    // special linking.
                    ASSERT(m_inlineStackTop->m_unlinkedBlocks.last().m_block == m_graph.lastBlock());
                    m_inlineStackTop->m_unlinkedBlocks.last().m_needsNormalLinking = false;
                }
                if (m_currentIndex + OPCODE_LENGTH(op_ret) != m_inlineStackTop->m_codeBlock->instructions().size() || m_inlineStackTop->m_didEarlyReturn) {
                    ASSERT(m_currentIndex + OPCODE_LENGTH(op_ret) <= m_inlineStackTop->m_codeBlock->instructions().size());
                    addToGraph(Jump, OpInfo(0));
                    m_inlineStackTop->m_unlinkedBlocks.last().m_needsEarlyReturnLinking = true;
                    m_inlineStackTop->m_didEarlyReturn = true;
                }
                LAST_OPCODE(op_ret);
            }
            addToGraph(Return, get(VirtualRegister(currentInstruction[1].u.operand)));
            flushForReturn();
            LAST_OPCODE(op_ret);
            
        case op_end:
            ASSERT(!inlineCallFrame());
            addToGraph(Return, get(VirtualRegister(currentInstruction[1].u.operand)));
            flushForReturn();
            LAST_OPCODE(op_end);

        case op_throw:
            addToGraph(Throw, get(VirtualRegister(currentInstruction[1].u.operand)));
            flushForTerminal();
            addToGraph(Unreachable);
            LAST_OPCODE(op_throw);
            
        case op_throw_static_error:
            addToGraph(Phantom, get(VirtualRegister(currentInstruction[1].u.operand))); // Keep argument live.
            addToGraph(ThrowStaticError);
            flushForTerminal();
            addToGraph(Unreachable);
            LAST_OPCODE(op_throw_static_error);

        case op_catch:
            m_graph.m_hasExceptionHandlers = true;
            NEXT_OPCODE(op_catch);
            
        case op_call:
            handleCall(currentInstruction, Call, CallMode::Regular);
            ASSERT_WITH_MESSAGE(m_currentInstruction == currentInstruction, "handleCall, which may have inlined the callee, trashed m_currentInstruction");
            NEXT_OPCODE(op_call);

        case op_tail_call: {
            flushForReturn();
            Terminality terminality = handleCall(currentInstruction, TailCall, CallMode::Tail);
            ASSERT_WITH_MESSAGE(m_currentInstruction == currentInstruction, "handleCall, which may have inlined the callee, trashed m_currentInstruction");
            // If the call is terminal then we should not parse any further bytecodes as the TailCall will exit the function.
            // If the call is not terminal, however, then we want the subsequent op_ret/op_jump to update metadata and clean
            // things up.
            if (terminality == NonTerminal)
                NEXT_OPCODE(op_tail_call);
            else
                LAST_OPCODE(op_tail_call);
        }

        case op_construct:
            handleCall(currentInstruction, Construct, CallMode::Construct);
            ASSERT_WITH_MESSAGE(m_currentInstruction == currentInstruction, "handleCall, which may have inlined the callee, trashed m_currentInstruction");
            NEXT_OPCODE(op_construct);
            
        case op_call_varargs: {
            handleVarargsCall(currentInstruction, CallVarargs, CallMode::Regular);
            ASSERT_WITH_MESSAGE(m_currentInstruction == currentInstruction, "handleVarargsCall, which may have inlined the callee, trashed m_currentInstruction");
            NEXT_OPCODE(op_call_varargs);
        }

        case op_tail_call_varargs: {
            flushForReturn();
            Terminality terminality = handleVarargsCall(currentInstruction, TailCallVarargs, CallMode::Tail);
            ASSERT_WITH_MESSAGE(m_currentInstruction == currentInstruction, "handleVarargsCall, which may have inlined the callee, trashed m_currentInstruction");
            // If the call is terminal then we should not parse any further bytecodes as the TailCall will exit the function.
            // If the call is not terminal, however, then we want the subsequent op_ret/op_jump to update metadata and clean
            // things up.
            if (terminality == NonTerminal)
                NEXT_OPCODE(op_tail_call_varargs);
            else
                LAST_OPCODE(op_tail_call_varargs);
        }

        case op_tail_call_forward_arguments: {
            // We need to make sure that we don't unbox our arguments here since that won't be
            // done by the arguments object creation node as that node may not exist.
            noticeArgumentsUse();
            flushForReturn();
            Terminality terminality = handleVarargsCall(currentInstruction, TailCallForwardVarargs, CallMode::Tail);
            ASSERT_WITH_MESSAGE(m_currentInstruction == currentInstruction, "handleVarargsCall, which may have inlined the callee, trashed m_currentInstruction");
            // If the call is terminal then we should not parse any further bytecodes as the TailCall will exit the function.
            // If the call is not terminal, however, then we want the subsequent op_ret/op_jump to update metadata and clean
            // things up.
            if (terminality == NonTerminal)
                NEXT_OPCODE(op_tail_call);
            else
                LAST_OPCODE(op_tail_call);
        }
            
        case op_construct_varargs: {
            handleVarargsCall(currentInstruction, ConstructVarargs, CallMode::Construct);
            ASSERT_WITH_MESSAGE(m_currentInstruction == currentInstruction, "handleVarargsCall, which may have inlined the callee, trashed m_currentInstruction");
            NEXT_OPCODE(op_construct_varargs);
        }
            
        case op_call_eval: {
            int result = currentInstruction[1].u.operand;
            int callee = currentInstruction[2].u.operand;
            int argumentCountIncludingThis = currentInstruction[3].u.operand;
            int registerOffset = -currentInstruction[4].u.operand;
            addCall(result, CallEval, nullptr, get(VirtualRegister(callee)), argumentCountIncludingThis, registerOffset, getPrediction());
            NEXT_OPCODE(op_call_eval);
        }
            
        case op_jneq_ptr: {
            Special::Pointer specialPointer = currentInstruction[2].u.specialPointer;
            ASSERT(pointerIsCell(specialPointer));
            JSCell* actualPointer = static_cast<JSCell*>(
                actualPointerFor(m_inlineStackTop->m_codeBlock, specialPointer));
            FrozenValue* frozenPointer = m_graph.freeze(actualPointer);
            int operand = currentInstruction[1].u.operand;
            unsigned relativeOffset = currentInstruction[3].u.operand;
            Node* child = get(VirtualRegister(operand));
            if (currentInstruction[4].u.operand) {
                Node* condition = addToGraph(CompareEqPtr, OpInfo(frozenPointer), child);
                addToGraph(Branch, OpInfo(branchData(m_currentIndex + OPCODE_LENGTH(op_jneq_ptr), m_currentIndex + relativeOffset)), condition);
                LAST_OPCODE(op_jneq_ptr);
            }
            addToGraph(CheckCell, OpInfo(frozenPointer), child);
            NEXT_OPCODE(op_jneq_ptr);
        }

        case op_resolve_scope: {
            int dst = currentInstruction[1].u.operand;
            ResolveType resolveType = static_cast<ResolveType>(currentInstruction[4].u.operand);
            unsigned depth = currentInstruction[5].u.operand;
            int scope = currentInstruction[2].u.operand;

            if (needsDynamicLookup(resolveType, op_resolve_scope)) {
                unsigned identifierNumber = m_inlineStackTop->m_identifierRemap[currentInstruction[3].u.operand];
                set(VirtualRegister(dst), addToGraph(ResolveScope, OpInfo(identifierNumber), get(VirtualRegister(scope))));
                NEXT_OPCODE(op_resolve_scope);
            }

            // get_from_scope and put_to_scope depend on this watchpoint forcing OSR exit, so they don't add their own watchpoints.
            if (needsVarInjectionChecks(resolveType))
                m_graph.watchpoints().addLazily(m_inlineStackTop->m_codeBlock->globalObject()->varInjectionWatchpoint());

            switch (resolveType) {
            case GlobalProperty:
            case GlobalVar:
            case GlobalPropertyWithVarInjectionChecks:
            case GlobalVarWithVarInjectionChecks:
            case GlobalLexicalVar:
            case GlobalLexicalVarWithVarInjectionChecks: {
                JSScope* constantScope = JSScope::constantScopeForCodeBlock(resolveType, m_inlineStackTop->m_codeBlock);
                RELEASE_ASSERT(constantScope);
                RELEASE_ASSERT(static_cast<JSScope*>(currentInstruction[6].u.pointer) == constantScope);
                set(VirtualRegister(dst), weakJSConstant(constantScope));
                addToGraph(Phantom, get(VirtualRegister(scope)));
                break;
            }
            case ModuleVar: {
                // Since the value of the "scope" virtual register is not used in LLInt / baseline op_resolve_scope with ModuleVar,
                // we need not to keep it alive by the Phantom node.
                JSModuleEnvironment* moduleEnvironment = jsCast<JSModuleEnvironment*>(currentInstruction[6].u.jsCell.get());
                // Module environment is already strongly referenced by the CodeBlock.
                set(VirtualRegister(dst), weakJSConstant(moduleEnvironment));
                break;
            }
            case LocalClosureVar:
            case ClosureVar:
            case ClosureVarWithVarInjectionChecks: {
                Node* localBase = get(VirtualRegister(scope));
                addToGraph(Phantom, localBase); // OSR exit cannot handle resolve_scope on a DCE'd scope.
                
                // We have various forms of constant folding here. This is necessary to avoid
                // spurious recompiles in dead-but-foldable code.
                if (SymbolTable* symbolTable = currentInstruction[6].u.symbolTable.get()) {
                    InferredValue* singleton = symbolTable->singletonScope();
                    if (JSValue value = singleton->inferredValue()) {
                        m_graph.watchpoints().addLazily(singleton);
                        set(VirtualRegister(dst), weakJSConstant(value));
                        break;
                    }
                }
                if (JSScope* scope = localBase->dynamicCastConstant<JSScope*>(*m_vm)) {
                    for (unsigned n = depth; n--;)
                        scope = scope->next();
                    set(VirtualRegister(dst), weakJSConstant(scope));
                    break;
                }
                for (unsigned n = depth; n--;)
                    localBase = addToGraph(SkipScope, localBase);
                set(VirtualRegister(dst), localBase);
                break;
            }
            case UnresolvedProperty:
            case UnresolvedPropertyWithVarInjectionChecks: {
                addToGraph(Phantom, get(VirtualRegister(scope)));
                addToGraph(ForceOSRExit);
                set(VirtualRegister(dst), addToGraph(JSConstant, OpInfo(m_constantNull)));
                break;
            }
            case Dynamic:
                RELEASE_ASSERT_NOT_REACHED();
                break;
            }
            NEXT_OPCODE(op_resolve_scope);
        }

        case op_get_from_scope: {
            int dst = currentInstruction[1].u.operand;
            int scope = currentInstruction[2].u.operand;
            unsigned identifierNumber = m_inlineStackTop->m_identifierRemap[currentInstruction[3].u.operand];
            UniquedStringImpl* uid = m_graph.identifiers()[identifierNumber];
            ResolveType resolveType = GetPutInfo(currentInstruction[4].u.operand).resolveType();

            Structure* structure = 0;
            WatchpointSet* watchpoints = 0;
            uintptr_t operand;
            {
                ConcurrentJSLocker locker(m_inlineStackTop->m_profiledBlock->m_lock);
                if (resolveType == GlobalVar || resolveType == GlobalVarWithVarInjectionChecks || resolveType == GlobalLexicalVar || resolveType == GlobalLexicalVarWithVarInjectionChecks)
                    watchpoints = currentInstruction[5].u.watchpointSet;
                else if (resolveType != UnresolvedProperty && resolveType != UnresolvedPropertyWithVarInjectionChecks)
                    structure = currentInstruction[5].u.structure.get();
                operand = reinterpret_cast<uintptr_t>(currentInstruction[6].u.pointer);
            }

            if (needsDynamicLookup(resolveType, op_get_from_scope)) {
                set(VirtualRegister(dst),
                    addToGraph(GetDynamicVar, OpInfo(identifierNumber), OpInfo(currentInstruction[4].u.operand), get(VirtualRegister(scope))));
                NEXT_OPCODE(op_get_from_scope);
            }

            UNUSED_PARAM(watchpoints); // We will use this in the future. For now we set it as a way of documenting the fact that that's what index 5 is in GlobalVar mode.

            JSGlobalObject* globalObject = m_inlineStackTop->m_codeBlock->globalObject();

            switch (resolveType) {
            case GlobalProperty:
            case GlobalPropertyWithVarInjectionChecks: {
                SpeculatedType prediction = getPrediction();

                GetByIdStatus status = GetByIdStatus::computeFor(structure, uid);
                if (status.state() != GetByIdStatus::Simple
                    || status.numVariants() != 1
                    || status[0].structureSet().size() != 1) {
                    set(VirtualRegister(dst), addToGraph(GetByIdFlush, OpInfo(identifierNumber), OpInfo(prediction), get(VirtualRegister(scope))));
                    break;
                }

                Node* base = weakJSConstant(globalObject);
                Node* result = load(prediction, base, identifierNumber, status[0]);
                addToGraph(Phantom, get(VirtualRegister(scope)));
                set(VirtualRegister(dst), result);
                break;
            }
            case GlobalVar:
            case GlobalVarWithVarInjectionChecks:
            case GlobalLexicalVar:
            case GlobalLexicalVarWithVarInjectionChecks: {
                addToGraph(Phantom, get(VirtualRegister(scope)));
                WatchpointSet* watchpointSet;
                ScopeOffset offset;
                JSSegmentedVariableObject* scopeObject = jsCast<JSSegmentedVariableObject*>(JSScope::constantScopeForCodeBlock(resolveType, m_inlineStackTop->m_codeBlock));
                {
                    ConcurrentJSLocker locker(scopeObject->symbolTable()->m_lock);
                    SymbolTableEntry entry = scopeObject->symbolTable()->get(locker, uid);
                    watchpointSet = entry.watchpointSet();
                    offset = entry.scopeOffset();
                }
                if (watchpointSet && watchpointSet->state() == IsWatched) {
                    // This has a fun concurrency story. There is the possibility of a race in two
                    // directions:
                    //
                    // We see that the set IsWatched, but in the meantime it gets invalidated: this is
                    // fine because if we saw that it IsWatched then we add a watchpoint. If it gets
                    // invalidated, then this compilation is invalidated. Note that in the meantime we
                    // may load an absurd value from the global object. It's fine to load an absurd
                    // value if the compilation is invalidated anyway.
                    //
                    // We see that the set IsWatched, but the value isn't yet initialized: this isn't
                    // possible because of the ordering of operations.
                    //
                    // Here's how we order operations:
                    //
                    // Main thread stores to the global object: always store a value first, and only
                    // after that do we touch the watchpoint set. There is a fence in the touch, that
                    // ensures that the store to the global object always happens before the touch on the
                    // set.
                    //
                    // Compilation thread: always first load the state of the watchpoint set, and then
                    // load the value. The WatchpointSet::state() method does fences for us to ensure
                    // that the load of the state happens before our load of the value.
                    //
                    // Finalizing compilation: this happens on the main thread and synchronously checks
                    // validity of all watchpoint sets.
                    //
                    // We will only perform optimizations if the load of the state yields IsWatched. That
                    // means that at least one store would have happened to initialize the original value
                    // of the variable (that is, the value we'd like to constant fold to). There may be
                    // other stores that happen after that, but those stores will invalidate the
                    // watchpoint set and also the compilation.
                    
                    // Note that we need to use the operand, which is a direct pointer at the global,
                    // rather than looking up the global by doing variableAt(offset). That's because the
                    // internal data structures of JSSegmentedVariableObject are not thread-safe even
                    // though accessing the global itself is. The segmentation involves a vector spine
                    // that resizes with malloc/free, so if new globals unrelated to the one we are
                    // reading are added, we might access freed memory if we do variableAt().
                    WriteBarrier<Unknown>* pointer = bitwise_cast<WriteBarrier<Unknown>*>(operand);
                    
                    ASSERT(scopeObject->findVariableIndex(pointer) == offset);
                    
                    JSValue value = pointer->get();
                    if (value) {
                        m_graph.watchpoints().addLazily(watchpointSet);
                        set(VirtualRegister(dst), weakJSConstant(value));
                        break;
                    }
                }
                
                SpeculatedType prediction = getPrediction();
                NodeType nodeType;
                if (resolveType == GlobalVar || resolveType == GlobalVarWithVarInjectionChecks)
                    nodeType = GetGlobalVar;
                else
                    nodeType = GetGlobalLexicalVariable;
                Node* value = addToGraph(nodeType, OpInfo(operand), OpInfo(prediction));
                if (resolveType == GlobalLexicalVar || resolveType == GlobalLexicalVarWithVarInjectionChecks)
                    addToGraph(CheckNotEmpty, value);
                set(VirtualRegister(dst), value);
                break;
            }
            case LocalClosureVar:
            case ClosureVar:
            case ClosureVarWithVarInjectionChecks: {
                Node* scopeNode = get(VirtualRegister(scope));
                
                // Ideally we wouldn't have to do this Phantom. But:
                //
                // For the constant case: we must do it because otherwise we would have no way of knowing
                // that the scope is live at OSR here.
                //
                // For the non-constant case: GetClosureVar could be DCE'd, but baseline's implementation
                // won't be able to handle an Undefined scope.
                addToGraph(Phantom, scopeNode);
                
                // Constant folding in the bytecode parser is important for performance. This may not
                // have executed yet. If it hasn't, then we won't have a prediction. Lacking a
                // prediction, we'd otherwise think that it has to exit. Then when it did execute, we
                // would recompile. But if we can fold it here, we avoid the exit.
                if (JSValue value = m_graph.tryGetConstantClosureVar(scopeNode, ScopeOffset(operand))) {
                    set(VirtualRegister(dst), weakJSConstant(value));
                    break;
                }
                SpeculatedType prediction = getPrediction();
                set(VirtualRegister(dst),
                    addToGraph(GetClosureVar, OpInfo(operand), OpInfo(prediction), scopeNode));
                break;
            }
            case UnresolvedProperty:
            case UnresolvedPropertyWithVarInjectionChecks:
            case ModuleVar:
            case Dynamic:
                RELEASE_ASSERT_NOT_REACHED();
                break;
            }
            NEXT_OPCODE(op_get_from_scope);
        }

        case op_put_to_scope: {
            unsigned scope = currentInstruction[1].u.operand;
            unsigned identifierNumber = currentInstruction[2].u.operand;
            if (identifierNumber != UINT_MAX)
                identifierNumber = m_inlineStackTop->m_identifierRemap[identifierNumber];
            unsigned value = currentInstruction[3].u.operand;
            GetPutInfo getPutInfo = GetPutInfo(currentInstruction[4].u.operand);
            ResolveType resolveType = getPutInfo.resolveType();
            UniquedStringImpl* uid;
            if (identifierNumber != UINT_MAX)
                uid = m_graph.identifiers()[identifierNumber];
            else
                uid = nullptr;
            
            Structure* structure = nullptr;
            WatchpointSet* watchpoints = nullptr;
            uintptr_t operand;
            {
                ConcurrentJSLocker locker(m_inlineStackTop->m_profiledBlock->m_lock);
                if (resolveType == GlobalVar || resolveType == GlobalVarWithVarInjectionChecks || resolveType == LocalClosureVar || resolveType == GlobalLexicalVar || resolveType == GlobalLexicalVarWithVarInjectionChecks)
                    watchpoints = currentInstruction[5].u.watchpointSet;
                else if (resolveType != UnresolvedProperty && resolveType != UnresolvedPropertyWithVarInjectionChecks)
                    structure = currentInstruction[5].u.structure.get();
                operand = reinterpret_cast<uintptr_t>(currentInstruction[6].u.pointer);
            }

            JSGlobalObject* globalObject = m_inlineStackTop->m_codeBlock->globalObject();

            if (needsDynamicLookup(resolveType, op_put_to_scope)) {
                ASSERT(identifierNumber != UINT_MAX);
                addToGraph(PutDynamicVar, OpInfo(identifierNumber), OpInfo(currentInstruction[4].u.operand), get(VirtualRegister(scope)), get(VirtualRegister(value)));
                NEXT_OPCODE(op_put_to_scope);
            }

            switch (resolveType) {
            case GlobalProperty:
            case GlobalPropertyWithVarInjectionChecks: {
                PutByIdStatus status;
                if (uid)
                    status = PutByIdStatus::computeFor(globalObject, structure, uid, false);
                else
                    status = PutByIdStatus(PutByIdStatus::TakesSlowPath);
                if (status.numVariants() != 1
                    || status[0].kind() != PutByIdVariant::Replace
                    || status[0].structure().size() != 1) {
                    addToGraph(PutById, OpInfo(identifierNumber), get(VirtualRegister(scope)), get(VirtualRegister(value)));
                    break;
                }
                Node* base = weakJSConstant(globalObject);
                store(base, identifierNumber, status[0], get(VirtualRegister(value)));
                // Keep scope alive until after put.
                addToGraph(Phantom, get(VirtualRegister(scope)));
                break;
            }
            case GlobalLexicalVar:
            case GlobalLexicalVarWithVarInjectionChecks:
            case GlobalVar:
            case GlobalVarWithVarInjectionChecks: {
                if (!isInitialization(getPutInfo.initializationMode()) && (resolveType == GlobalLexicalVar || resolveType == GlobalLexicalVarWithVarInjectionChecks)) {
                    SpeculatedType prediction = SpecEmpty;
                    Node* value = addToGraph(GetGlobalLexicalVariable, OpInfo(operand), OpInfo(prediction));
                    addToGraph(CheckNotEmpty, value);
                }

                JSSegmentedVariableObject* scopeObject = jsCast<JSSegmentedVariableObject*>(JSScope::constantScopeForCodeBlock(resolveType, m_inlineStackTop->m_codeBlock));
                if (watchpoints) {
                    SymbolTableEntry entry = scopeObject->symbolTable()->get(uid);
                    ASSERT_UNUSED(entry, watchpoints == entry.watchpointSet());
                }
                Node* valueNode = get(VirtualRegister(value));
                addToGraph(PutGlobalVariable, OpInfo(operand), weakJSConstant(scopeObject), valueNode);
                if (watchpoints && watchpoints->state() != IsInvalidated) {
                    // Must happen after the store. See comment for GetGlobalVar.
                    addToGraph(NotifyWrite, OpInfo(watchpoints));
                }
                // Keep scope alive until after put.
                addToGraph(Phantom, get(VirtualRegister(scope)));
                break;
            }
            case LocalClosureVar:
            case ClosureVar:
            case ClosureVarWithVarInjectionChecks: {
                Node* scopeNode = get(VirtualRegister(scope));
                Node* valueNode = get(VirtualRegister(value));

                addToGraph(PutClosureVar, OpInfo(operand), scopeNode, valueNode);

                if (watchpoints && watchpoints->state() != IsInvalidated) {
                    // Must happen after the store. See comment for GetGlobalVar.
                    addToGraph(NotifyWrite, OpInfo(watchpoints));
                }
                break;
            }

            case ModuleVar:
                // Need not to keep "scope" and "value" register values here by Phantom because
                // they are not used in LLInt / baseline op_put_to_scope with ModuleVar.
                addToGraph(ForceOSRExit);
                break;

            case Dynamic:
            case UnresolvedProperty:
            case UnresolvedPropertyWithVarInjectionChecks:
                RELEASE_ASSERT_NOT_REACHED();
                break;
            }
            NEXT_OPCODE(op_put_to_scope);
        }

        case op_loop_hint: {
            // Baseline->DFG OSR jumps between loop hints. The DFG assumes that Baseline->DFG
            // OSR can only happen at basic block boundaries. Assert that these two statements
            // are compatible.
            RELEASE_ASSERT(m_currentIndex == blockBegin);
            
            // We never do OSR into an inlined code block. That could not happen, since OSR
            // looks up the code block that is the replacement for the baseline JIT code
            // block. Hence, machine code block = true code block = not inline code block.
            if (!m_inlineStackTop->m_caller)
                m_currentBlock->isOSRTarget = true;

            addToGraph(LoopHint);
            NEXT_OPCODE(op_loop_hint);
        }
        
        case op_check_traps: {
            addToGraph(CheckTraps);
            NEXT_OPCODE(op_check_traps);
        }
            
        case op_create_lexical_environment: {
            VirtualRegister symbolTableRegister(currentInstruction[3].u.operand);
            VirtualRegister initialValueRegister(currentInstruction[4].u.operand);
            ASSERT(symbolTableRegister.isConstant() && initialValueRegister.isConstant());
            FrozenValue* symbolTable = m_graph.freezeStrong(m_inlineStackTop->m_codeBlock->getConstant(symbolTableRegister.offset()));
            FrozenValue* initialValue = m_graph.freezeStrong(m_inlineStackTop->m_codeBlock->getConstant(initialValueRegister.offset()));
            Node* scope = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* lexicalEnvironment = addToGraph(CreateActivation, OpInfo(symbolTable), OpInfo(initialValue), scope);
            set(VirtualRegister(currentInstruction[1].u.operand), lexicalEnvironment);
            NEXT_OPCODE(op_create_lexical_environment);
        }

        case op_get_parent_scope: {
            Node* currentScope = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* newScope = addToGraph(SkipScope, currentScope);
            set(VirtualRegister(currentInstruction[1].u.operand), newScope);
            addToGraph(Phantom, currentScope);
            NEXT_OPCODE(op_get_parent_scope);
        }

        case op_get_scope: {
            // Help the later stages a bit by doing some small constant folding here. Note that this
            // only helps for the first basic block. It's extremely important not to constant fold
            // loads from the scope register later, as that would prevent the DFG from tracking the
            // bytecode-level liveness of the scope register.
            Node* callee = get(VirtualRegister(CallFrameSlot::callee));
            Node* result;
            if (JSFunction* function = callee->dynamicCastConstant<JSFunction*>(*m_vm))
                result = weakJSConstant(function->scope());
            else
                result = addToGraph(GetScope, callee);
            set(VirtualRegister(currentInstruction[1].u.operand), result);
            NEXT_OPCODE(op_get_scope);
        }

        case op_argument_count: {
            Node* sub = addToGraph(ArithSub, OpInfo(Arith::Unchecked), OpInfo(SpecInt32Only), getArgumentCount(), addToGraph(JSConstant, OpInfo(m_constantOne)));

            set(VirtualRegister(currentInstruction[1].u.operand), sub);
            NEXT_OPCODE(op_argument_count);
        }

        case op_create_direct_arguments: {
            noticeArgumentsUse();
            Node* createArguments = addToGraph(CreateDirectArguments);
            set(VirtualRegister(currentInstruction[1].u.operand), createArguments);
            NEXT_OPCODE(op_create_direct_arguments);
        }
            
        case op_create_scoped_arguments: {
            noticeArgumentsUse();
            Node* createArguments = addToGraph(CreateScopedArguments, get(VirtualRegister(currentInstruction[2].u.operand)));
            set(VirtualRegister(currentInstruction[1].u.operand), createArguments);
            NEXT_OPCODE(op_create_scoped_arguments);
        }

        case op_create_cloned_arguments: {
            noticeArgumentsUse();
            Node* createArguments = addToGraph(CreateClonedArguments);
            set(VirtualRegister(currentInstruction[1].u.operand), createArguments);
            NEXT_OPCODE(op_create_cloned_arguments);
        }
            
        case op_get_from_arguments: {
            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(
                    GetFromArguments,
                    OpInfo(currentInstruction[3].u.operand),
                    OpInfo(getPrediction()),
                    get(VirtualRegister(currentInstruction[2].u.operand))));
            NEXT_OPCODE(op_get_from_arguments);
        }
            
        case op_put_to_arguments: {
            addToGraph(
                PutToArguments,
                OpInfo(currentInstruction[2].u.operand),
                get(VirtualRegister(currentInstruction[1].u.operand)),
                get(VirtualRegister(currentInstruction[3].u.operand)));
            NEXT_OPCODE(op_put_to_arguments);
        }

        case op_get_argument: {
            InlineCallFrame* inlineCallFrame = this->inlineCallFrame();
            Node* argument;
            int32_t argumentIndexIncludingThis = currentInstruction[2].u.operand;
            if (inlineCallFrame && !inlineCallFrame->isVarargs()) {
                int32_t argumentCountIncludingThis = inlineCallFrame->arguments.size();
                if (argumentIndexIncludingThis < argumentCountIncludingThis)
                    argument = get(virtualRegisterForArgument(argumentIndexIncludingThis));
                else
                    argument = addToGraph(JSConstant, OpInfo(m_constantUndefined));
            } else
                argument = addToGraph(GetArgument, OpInfo(argumentIndexIncludingThis), OpInfo(getPrediction()));
            set(VirtualRegister(currentInstruction[1].u.operand), argument);
            NEXT_OPCODE(op_get_argument);
        }
            
        case op_new_func:
        case op_new_generator_func:
        case op_new_async_func: {
            FunctionExecutable* decl = m_inlineStackTop->m_profiledBlock->functionDecl(currentInstruction[3].u.operand);
            FrozenValue* frozen = m_graph.freezeStrong(decl);
            NodeType op = (opcodeID == op_new_generator_func) ? NewGeneratorFunction :
                (opcodeID == op_new_async_func) ? NewAsyncFunction : NewFunction;
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(op, OpInfo(frozen), get(VirtualRegister(currentInstruction[2].u.operand))));
            static_assert(OPCODE_LENGTH(op_new_func) == OPCODE_LENGTH(op_new_generator_func), "The length of op_new_func should eqaual to one of op_new_generator_func");
            static_assert(OPCODE_LENGTH(op_new_func) == OPCODE_LENGTH(op_new_async_func), "The length of op_new_func should eqaual to one of op_new_async_func");
            NEXT_OPCODE(op_new_func);
        }

        case op_new_func_exp:
        case op_new_generator_func_exp:
        case op_new_async_func_exp: {
            FunctionExecutable* expr = m_inlineStackTop->m_profiledBlock->functionExpr(currentInstruction[3].u.operand);
            FrozenValue* frozen = m_graph.freezeStrong(expr);
            NodeType op = (opcodeID == op_new_generator_func_exp) ? NewGeneratorFunction :
                (opcodeID == op_new_async_func_exp) ? NewAsyncFunction : NewFunction;
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(op, OpInfo(frozen), get(VirtualRegister(currentInstruction[2].u.operand))));
    
            static_assert(OPCODE_LENGTH(op_new_func_exp) == OPCODE_LENGTH(op_new_generator_func_exp), "The length of op_new_func_exp should eqaual to one of op_new_generator_func_exp");
            static_assert(OPCODE_LENGTH(op_new_func_exp) == OPCODE_LENGTH(op_new_async_func_exp), "The length of op_new_func_exp should eqaual to one of op_new_async_func_exp");
            NEXT_OPCODE(op_new_func_exp);
        }

        case op_set_function_name: {
            Node* func = get(VirtualRegister(currentInstruction[1].u.operand));
            Node* name = get(VirtualRegister(currentInstruction[2].u.operand));
            addToGraph(SetFunctionName, func, name);
            NEXT_OPCODE(op_set_function_name);
        }

        case op_typeof: {
            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(TypeOf, get(VirtualRegister(currentInstruction[2].u.operand))));
            NEXT_OPCODE(op_typeof);
        }

        case op_to_number: {
            SpeculatedType prediction = getPrediction();
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(ToNumber, OpInfo(0), OpInfo(prediction), value));
            NEXT_OPCODE(op_to_number);
        }

        case op_to_string: {
            Node* value = get(VirtualRegister(currentInstruction[2].u.operand));
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(ToString, value));
            NEXT_OPCODE(op_to_string);
        }

        case op_in: {
            ArrayMode arrayMode = getArrayMode(currentInstruction[OPCODE_LENGTH(op_in) - 1].u.arrayProfile);
            set(VirtualRegister(currentInstruction[1].u.operand),
                addToGraph(In, OpInfo(arrayMode.asWord()), get(VirtualRegister(currentInstruction[2].u.operand)), get(VirtualRegister(currentInstruction[3].u.operand))));
            NEXT_OPCODE(op_in);
        }

        case op_get_enumerable_length: {
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(GetEnumerableLength, 
                get(VirtualRegister(currentInstruction[2].u.operand))));
            NEXT_OPCODE(op_get_enumerable_length);
        }

        case op_has_generic_property: {
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(HasGenericProperty, 
                get(VirtualRegister(currentInstruction[2].u.operand)),
                get(VirtualRegister(currentInstruction[3].u.operand))));
            NEXT_OPCODE(op_has_generic_property);
        }

        case op_has_structure_property: {
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(HasStructureProperty, 
                get(VirtualRegister(currentInstruction[2].u.operand)),
                get(VirtualRegister(currentInstruction[3].u.operand)),
                get(VirtualRegister(currentInstruction[4].u.operand))));
            NEXT_OPCODE(op_has_structure_property);
        }

        case op_has_indexed_property: {
            Node* base = get(VirtualRegister(currentInstruction[2].u.operand));
            ArrayMode arrayMode = getArrayMode(currentInstruction[4].u.arrayProfile, Array::Read);
            Node* property = get(VirtualRegister(currentInstruction[3].u.operand));
            Node* hasIterableProperty = addToGraph(HasIndexedProperty, OpInfo(arrayMode.asWord()), OpInfo(static_cast<uint32_t>(PropertySlot::InternalMethodType::GetOwnProperty)), base, property);
            set(VirtualRegister(currentInstruction[1].u.operand), hasIterableProperty);
            NEXT_OPCODE(op_has_indexed_property);
        }

        case op_get_direct_pname: {
            SpeculatedType prediction = getPredictionWithoutOSRExit();
            
            Node* base = get(VirtualRegister(currentInstruction[2].u.operand));
            Node* property = get(VirtualRegister(currentInstruction[3].u.operand));
            Node* index = get(VirtualRegister(currentInstruction[4].u.operand));
            Node* enumerator = get(VirtualRegister(currentInstruction[5].u.operand));

            addVarArgChild(base);
            addVarArgChild(property);
            addVarArgChild(index);
            addVarArgChild(enumerator);
            set(VirtualRegister(currentInstruction[1].u.operand), 
                addToGraph(Node::VarArg, GetDirectPname, OpInfo(0), OpInfo(prediction)));

            NEXT_OPCODE(op_get_direct_pname);
        }

        case op_get_property_enumerator: {
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(GetPropertyEnumerator, 
                get(VirtualRegister(currentInstruction[2].u.operand))));
            NEXT_OPCODE(op_get_property_enumerator);
        }

        case op_enumerator_structure_pname: {
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(GetEnumeratorStructurePname,
                get(VirtualRegister(currentInstruction[2].u.operand)),
                get(VirtualRegister(currentInstruction[3].u.operand))));
            NEXT_OPCODE(op_enumerator_structure_pname);
        }

        case op_enumerator_generic_pname: {
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(GetEnumeratorGenericPname,
                get(VirtualRegister(currentInstruction[2].u.operand)),
                get(VirtualRegister(currentInstruction[3].u.operand))));
            NEXT_OPCODE(op_enumerator_generic_pname);
        }
            
        case op_to_index_string: {
            set(VirtualRegister(currentInstruction[1].u.operand), addToGraph(ToIndexString, 
                get(VirtualRegister(currentInstruction[2].u.operand))));
            NEXT_OPCODE(op_to_index_string);
        }
            
        case op_log_shadow_chicken_prologue: {
            if (!m_inlineStackTop->m_inlineCallFrame)
                addToGraph(LogShadowChickenPrologue, get(VirtualRegister(currentInstruction[1].u.operand)));
            NEXT_OPCODE(op_log_shadow_chicken_prologue);
        }

        case op_log_shadow_chicken_tail: {
            if (!m_inlineStackTop->m_inlineCallFrame) {
                // FIXME: The right solution for inlining is to elide these whenever the tail call
                // ends up being inlined.
                // https://bugs.webkit.org/show_bug.cgi?id=155686
                addToGraph(LogShadowChickenTail, get(VirtualRegister(currentInstruction[1].u.operand)), get(VirtualRegister(currentInstruction[2].u.operand)));
            }
            NEXT_OPCODE(op_log_shadow_chicken_tail);
        }

        default:
            // Parse failed! This should not happen because the capabilities checker
            // should have caught it.
            RELEASE_ASSERT_NOT_REACHED();
            return false;
        }
    }
}

void ByteCodeParser::linkBlock(BasicBlock* block, Vector<BasicBlock*>& possibleTargets)
{
    ASSERT(!block->isLinked);
    ASSERT(!block->isEmpty());
    Node* node = block->terminal();
    ASSERT(node->isTerminal());
    
    switch (node->op()) {
    case Jump:
        node->targetBlock() = blockForBytecodeOffset(possibleTargets, node->targetBytecodeOffsetDuringParsing());
        break;
        
    case Branch: {
        BranchData* data = node->branchData();
        data->taken.block = blockForBytecodeOffset(possibleTargets, data->takenBytecodeIndex());
        data->notTaken.block = blockForBytecodeOffset(possibleTargets, data->notTakenBytecodeIndex());
        break;
    }
        
    case Switch: {
        SwitchData* data = node->switchData();
        for (unsigned i = node->switchData()->cases.size(); i--;)
            data->cases[i].target.block = blockForBytecodeOffset(possibleTargets, data->cases[i].target.bytecodeIndex());
        data->fallThrough.block = blockForBytecodeOffset(possibleTargets, data->fallThrough.bytecodeIndex());
        break;
    }
        
    default:
        break;
    }
    
    if (verbose)
        dataLog("Marking ", RawPointer(block), " as linked (actually did linking)\n");
    block->didLink();
}

void ByteCodeParser::linkBlocks(Vector<UnlinkedBlock>& unlinkedBlocks, Vector<BasicBlock*>& possibleTargets)
{
    for (size_t i = 0; i < unlinkedBlocks.size(); ++i) {
        if (verbose)
            dataLog("Attempting to link ", RawPointer(unlinkedBlocks[i].m_block), "\n");
        if (unlinkedBlocks[i].m_needsNormalLinking) {
            if (verbose)
                dataLog("    Does need normal linking.\n");
            linkBlock(unlinkedBlocks[i].m_block, possibleTargets);
            unlinkedBlocks[i].m_needsNormalLinking = false;
        }
    }
}

ByteCodeParser::InlineStackEntry::InlineStackEntry(
    ByteCodeParser* byteCodeParser,
    CodeBlock* codeBlock,
    CodeBlock* profiledBlock,
    BasicBlock* callsiteBlockHead,
    JSFunction* callee, // Null if this is a closure call.
    VirtualRegister returnValueVR,
    VirtualRegister inlineCallFrameStart,
    int argumentCountIncludingThis,
    InlineCallFrame::Kind kind)
    : m_byteCodeParser(byteCodeParser)
    , m_codeBlock(codeBlock)
    , m_profiledBlock(profiledBlock)
    , m_callsiteBlockHead(callsiteBlockHead)
    , m_returnValue(returnValueVR)
    , m_didReturn(false)
    , m_didEarlyReturn(false)
    , m_caller(byteCodeParser->m_inlineStackTop)
{
    {
        ConcurrentJSLocker locker(m_profiledBlock->m_lock);
        m_lazyOperands.initialize(locker, m_profiledBlock->lazyOperandValueProfiles());
        m_exitProfile.initialize(locker, profiledBlock->exitProfile());
        
        // We do this while holding the lock because we want to encourage StructureStubInfo's
        // to be potentially added to operations and because the profiled block could be in the
        // middle of LLInt->JIT tier-up in which case we would be adding the info's right now.
        if (m_profiledBlock->hasBaselineJITProfiling()) {
            m_profiledBlock->getStubInfoMap(locker, m_stubInfos);
            m_profiledBlock->getCallLinkInfoMap(locker, m_callLinkInfos);
            m_profiledBlock->getByValInfoMap(locker, m_byValInfos);
        }
    }
    
    m_argumentPositions.resize(argumentCountIncludingThis);
    for (int i = 0; i < argumentCountIncludingThis; ++i) {
        byteCodeParser->m_graph.m_argumentPositions.append(ArgumentPosition());
        ArgumentPosition* argumentPosition = &byteCodeParser->m_graph.m_argumentPositions.last();
        m_argumentPositions[i] = argumentPosition;
    }
    
    if (m_caller) {
        // Inline case.
        ASSERT(codeBlock != byteCodeParser->m_codeBlock);
        ASSERT(inlineCallFrameStart.isValid());
        ASSERT(callsiteBlockHead);
        
        m_inlineCallFrame = byteCodeParser->m_graph.m_plan.inlineCallFrames->add();

        // The owner is the machine code block, and we already have a barrier on that when the
        // plan finishes.
        m_inlineCallFrame->baselineCodeBlock.setWithoutWriteBarrier(codeBlock->baselineVersion());
        m_inlineCallFrame->setStackOffset(inlineCallFrameStart.offset() - CallFrame::headerSizeInRegisters);
        if (callee) {
            m_inlineCallFrame->calleeRecovery = ValueRecovery::constant(callee);
            m_inlineCallFrame->isClosureCall = false;
        } else
            m_inlineCallFrame->isClosureCall = true;
        m_inlineCallFrame->directCaller = byteCodeParser->currentCodeOrigin();
        m_inlineCallFrame->arguments.resizeToFit(argumentCountIncludingThis); // Set the number of arguments including this, but don't configure the value recoveries, yet.
        m_inlineCallFrame->kind = kind;
        
        m_identifierRemap.resize(codeBlock->numberOfIdentifiers());
        m_constantBufferRemap.resize(codeBlock->numberOfConstantBuffers());
        m_switchRemap.resize(codeBlock->numberOfSwitchJumpTables());

        for (size_t i = 0; i < codeBlock->numberOfIdentifiers(); ++i) {
            UniquedStringImpl* rep = codeBlock->identifier(i).impl();
            unsigned index = byteCodeParser->m_graph.identifiers().ensure(rep);
            m_identifierRemap[i] = index;
        }
        for (unsigned i = 0; i < codeBlock->numberOfConstantBuffers(); ++i) {
            // If we inline the same code block multiple times, we don't want to needlessly
            // duplicate its constant buffers.
            HashMap<ConstantBufferKey, unsigned>::iterator iter =
                byteCodeParser->m_constantBufferCache.find(ConstantBufferKey(codeBlock, i));
            if (iter != byteCodeParser->m_constantBufferCache.end()) {
                m_constantBufferRemap[i] = iter->value;
                continue;
            }
            Vector<JSValue>& buffer = codeBlock->constantBufferAsVector(i);
            unsigned newIndex = byteCodeParser->m_codeBlock->addConstantBuffer(buffer);
            m_constantBufferRemap[i] = newIndex;
            byteCodeParser->m_constantBufferCache.add(ConstantBufferKey(codeBlock, i), newIndex);
        }
        for (unsigned i = 0; i < codeBlock->numberOfSwitchJumpTables(); ++i) {
            m_switchRemap[i] = byteCodeParser->m_codeBlock->numberOfSwitchJumpTables();
            byteCodeParser->m_codeBlock->addSwitchJumpTable() = codeBlock->switchJumpTable(i);
        }
        m_callsiteBlockHeadNeedsLinking = true;
    } else {
        // Machine code block case.
        ASSERT(codeBlock == byteCodeParser->m_codeBlock);
        ASSERT(!callee);
        ASSERT(!returnValueVR.isValid());
        ASSERT(!inlineCallFrameStart.isValid());
        ASSERT(!callsiteBlockHead);

        m_inlineCallFrame = 0;

        m_identifierRemap.resize(codeBlock->numberOfIdentifiers());
        m_constantBufferRemap.resize(codeBlock->numberOfConstantBuffers());
        m_switchRemap.resize(codeBlock->numberOfSwitchJumpTables());
        for (size_t i = 0; i < codeBlock->numberOfIdentifiers(); ++i)
            m_identifierRemap[i] = i;
        for (size_t i = 0; i < codeBlock->numberOfConstantBuffers(); ++i)
            m_constantBufferRemap[i] = i;
        for (size_t i = 0; i < codeBlock->numberOfSwitchJumpTables(); ++i)
            m_switchRemap[i] = i;
        m_callsiteBlockHeadNeedsLinking = false;
    }
    
    byteCodeParser->m_inlineStackTop = this;
}

void ByteCodeParser::parseCodeBlock()
{
    clearCaches();
    
    CodeBlock* codeBlock = m_inlineStackTop->m_codeBlock;
    
    if (m_graph.compilation()) {
        m_graph.compilation()->addProfiledBytecodes(
            *m_vm->m_perBytecodeProfiler, m_inlineStackTop->m_profiledBlock);
    }
    
    if (UNLIKELY(Options::dumpSourceAtDFGTime())) {
        Vector<DeferredSourceDump>& deferredSourceDump = m_graph.m_plan.callback->ensureDeferredSourceDump();
        if (inlineCallFrame()) {
            DeferredSourceDump dump(codeBlock->baselineVersion(), m_codeBlock, JITCode::DFGJIT, inlineCallFrame()->directCaller);
            deferredSourceDump.append(dump);
        } else
            deferredSourceDump.append(DeferredSourceDump(codeBlock->baselineVersion()));
    }

    if (Options::dumpBytecodeAtDFGTime()) {
        dataLog("Parsing ", *codeBlock);
        if (inlineCallFrame()) {
            dataLog(
                " for inlining at ", CodeBlockWithJITType(m_codeBlock, JITCode::DFGJIT),
                " ", inlineCallFrame()->directCaller);
        }
        dataLog(
            ", isStrictMode = ", codeBlock->ownerScriptExecutable()->isStrictMode(), "\n");
        codeBlock->baselineVersion()->dumpBytecode();
    }
    
    Vector<unsigned, 32> jumpTargets;
    computePreciseJumpTargets(codeBlock, jumpTargets);
    if (Options::dumpBytecodeAtDFGTime()) {
        dataLog("Jump targets: ");
        CommaPrinter comma;
        for (unsigned i = 0; i < jumpTargets.size(); ++i)
            dataLog(comma, jumpTargets[i]);
        dataLog("\n");
    }
    
    for (unsigned jumpTargetIndex = 0; jumpTargetIndex <= jumpTargets.size(); ++jumpTargetIndex) {
        // The maximum bytecode offset to go into the current basicblock is either the next jump target, or the end of the instructions.
        unsigned limit = jumpTargetIndex < jumpTargets.size() ? jumpTargets[jumpTargetIndex] : codeBlock->instructions().size();
        ASSERT(m_currentIndex < limit);

        // Loop until we reach the current limit (i.e. next jump target).
        do {
            if (!m_currentBlock) {
                // Check if we can use the last block.
                if (m_graph.numBlocks() && m_graph.lastBlock()->isEmpty()) {
                    // This must be a block belonging to us.
                    ASSERT(m_inlineStackTop->m_unlinkedBlocks.last().m_block == m_graph.lastBlock());
                    // Either the block is linkable or it isn't. If it's linkable then it's the last
                    // block in the blockLinkingTargets list. If it's not then the last block will
                    // have a lower bytecode index that the one we're about to give to this block.
                    if (m_inlineStackTop->m_blockLinkingTargets.isEmpty() || m_inlineStackTop->m_blockLinkingTargets.last()->bytecodeBegin != m_currentIndex) {
                        // Make the block linkable.
                        ASSERT(m_inlineStackTop->m_blockLinkingTargets.isEmpty() || m_inlineStackTop->m_blockLinkingTargets.last()->bytecodeBegin < m_currentIndex);
                        m_inlineStackTop->m_blockLinkingTargets.append(m_graph.lastBlock());
                    }
                    // Change its bytecode begin and continue.
                    m_currentBlock = m_graph.lastBlock();
                    m_currentBlock->bytecodeBegin = m_currentIndex;
                } else {
                    Ref<BasicBlock> block = adoptRef(*new BasicBlock(m_currentIndex, m_numArguments, m_numLocals, 1));
                    m_currentBlock = block.ptr();
                    // This assertion checks two things:
                    // 1) If the bytecodeBegin is greater than currentIndex, then something has gone
                    //    horribly wrong. So, we're probably generating incorrect code.
                    // 2) If the bytecodeBegin is equal to the currentIndex, then we failed to do
                    //    a peephole coalescing of this block in the if statement above. So, we're
                    //    generating suboptimal code and leaving more work for the CFG simplifier.
                    if (!m_inlineStackTop->m_unlinkedBlocks.isEmpty()) {
                        unsigned lastBegin =
                            m_inlineStackTop->m_unlinkedBlocks.last().m_block->bytecodeBegin;
                        ASSERT_UNUSED(
                            lastBegin, lastBegin == UINT_MAX || lastBegin < m_currentIndex);
                    }
                    m_inlineStackTop->m_unlinkedBlocks.append(UnlinkedBlock(block.ptr()));
                    m_inlineStackTop->m_blockLinkingTargets.append(block.ptr());
                    // The first block is definitely an OSR target.
                    if (!m_graph.numBlocks())
                        block->isOSRTarget = true;
                    m_graph.appendBlock(WTFMove(block));
                    prepareToParseBlock();
                }
            }

            bool shouldContinueParsing = parseBlock(limit);

            // We should not have gone beyond the limit.
            ASSERT(m_currentIndex <= limit);
            
            // We should have planted a terminal, or we just gave up because
            // we realized that the jump target information is imprecise, or we
            // are at the end of an inline function, or we realized that we
            // should stop parsing because there was a return in the first
            // basic block.
            ASSERT(m_currentBlock->isEmpty() || m_currentBlock->terminal() || (m_currentIndex == codeBlock->instructions().size() && inlineCallFrame()) || !shouldContinueParsing);

            if (!shouldContinueParsing) {
                if (Options::verboseDFGByteCodeParsing())
                    dataLog("Done parsing ", *codeBlock, "\n");
                return;
            }
            
            m_currentBlock = nullptr;
        } while (m_currentIndex < limit);
    }

    // Should have reached the end of the instructions.
    ASSERT(m_currentIndex == codeBlock->instructions().size());
    
    if (Options::verboseDFGByteCodeParsing())
        dataLog("Done parsing ", *codeBlock, " (fell off end)\n");
}

bool ByteCodeParser::parse()
{
    // Set during construction.
    ASSERT(!m_currentIndex);
    
    if (Options::verboseDFGByteCodeParsing())
        dataLog("Parsing ", *m_codeBlock, "\n");
    
    m_dfgCodeBlock = m_graph.m_plan.profiledDFGCodeBlock;
    if (isFTL(m_graph.m_plan.mode) && m_dfgCodeBlock
        && Options::usePolyvariantDevirtualization()) {
        if (Options::usePolyvariantCallInlining())
            CallLinkStatus::computeDFGStatuses(m_dfgCodeBlock, m_callContextMap);
        if (Options::usePolyvariantByIdInlining())
            m_dfgCodeBlock->getStubInfoMap(m_dfgStubInfos);
    }
    
    InlineStackEntry inlineStackEntry(
        this, m_codeBlock, m_profiledBlock, 0, 0, VirtualRegister(), VirtualRegister(),
        m_codeBlock->numParameters(), InlineCallFrame::Call);
    
    parseCodeBlock();

    linkBlocks(inlineStackEntry.m_unlinkedBlocks, inlineStackEntry.m_blockLinkingTargets);
    m_graph.determineReachability();
    m_graph.killUnreachableBlocks();
    
    for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
        BasicBlock* block = m_graph.block(blockIndex);
        if (!block)
            continue;
        ASSERT(block->variablesAtHead.numberOfLocals() == m_graph.block(0)->variablesAtHead.numberOfLocals());
        ASSERT(block->variablesAtHead.numberOfArguments() == m_graph.block(0)->variablesAtHead.numberOfArguments());
        ASSERT(block->variablesAtTail.numberOfLocals() == m_graph.block(0)->variablesAtHead.numberOfLocals());
        ASSERT(block->variablesAtTail.numberOfArguments() == m_graph.block(0)->variablesAtHead.numberOfArguments());
    }
    
    m_graph.m_localVars = m_numLocals;
    m_graph.m_parameterSlots = m_parameterSlots;

    return true;
}

bool parse(Graph& graph)
{
    base::debug::TraceScope traceScope("jsc", "DFG parse");
    return ByteCodeParser(graph).parse();
}

} } // namespace JSC::DFG

#endif
