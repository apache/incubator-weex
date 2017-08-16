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
#include "DFGPredictionPropagationPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGGraph.h"
#include "DFGPhase.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

namespace {

bool verboseFixPointLoops = false;

class PredictionPropagationPhase : public Phase {
public:
    PredictionPropagationPhase(Graph& graph)
        : Phase(graph, "prediction propagation")
    {
    }
    
    bool run()
    {
        ASSERT(m_graph.m_form == ThreadedCPS);
        ASSERT(m_graph.m_unificationState == GloballyUnified);

        propagateThroughArgumentPositions();

        processInvariants();

        m_pass = PrimaryPass;
        propagateToFixpoint();
        
        m_pass = RareCasePass;
        propagateToFixpoint();
        
        m_pass = DoubleVotingPass;
        unsigned counter = 0;
        do {
            if (verboseFixPointLoops)
                ++counter;

            m_changed = false;
            doRoundOfDoubleVoting();
            if (!m_changed)
                break;
            m_changed = false;
            propagateForward();
        } while (m_changed);

        if (verboseFixPointLoops)
            dataLog("Iterated ", counter, " times in double voting fixpoint.\n");
        
        return true;
    }
    
private:
    void propagateToFixpoint()
    {
        unsigned counter = 0;
        do {
            if (verboseFixPointLoops)
                ++counter;

            m_changed = false;

            // Forward propagation is near-optimal for both topologically-sorted and
            // DFS-sorted code.
            propagateForward();
            if (!m_changed)
                break;
            
            // Backward propagation reduces the likelihood that pathological code will
            // cause slowness. Loops (especially nested ones) resemble backward flow.
            // This pass captures two cases: (1) it detects if the forward fixpoint
            // found a sound solution and (2) short-circuits backward flow.
            m_changed = false;
            propagateBackward();
        } while (m_changed);

        if (verboseFixPointLoops)
            dataLog("Iterated ", counter, " times in propagateToFixpoint.\n");
    }
    
    bool setPrediction(SpeculatedType prediction)
    {
        ASSERT(m_currentNode->hasResult());
        
        // setPrediction() is used when we know that there is no way that we can change
        // our minds about what the prediction is going to be. There is no semantic
        // difference between setPrediction() and mergeSpeculation() other than the
        // increased checking to validate this property.
        ASSERT(m_currentNode->prediction() == SpecNone || m_currentNode->prediction() == prediction);
        
        return m_currentNode->predict(prediction);
    }
    
    bool mergePrediction(SpeculatedType prediction)
    {
        ASSERT(m_currentNode->hasResult());
        
        return m_currentNode->predict(prediction);
    }
    
    SpeculatedType speculatedDoubleTypeForPrediction(SpeculatedType value)
    {
        SpeculatedType result = SpecDoubleReal;
        if (value & SpecDoubleImpureNaN)
            result |= SpecDoubleImpureNaN;
        if (value & SpecDoublePureNaN)
            result |= SpecDoublePureNaN;
        if (!isFullNumberOrBooleanSpeculation(value))
            result |= SpecDoublePureNaN;
        return result;
    }

    SpeculatedType speculatedDoubleTypeForPredictions(SpeculatedType left, SpeculatedType right)
    {
        return speculatedDoubleTypeForPrediction(mergeSpeculations(left, right));
    }

    void propagate(Node* node)
    {
        NodeType op = node->op();

        bool changed = false;
        
        switch (op) {
        case GetLocal: {
            VariableAccessData* variable = node->variableAccessData();
            SpeculatedType prediction = variable->prediction();
            if (!variable->couldRepresentInt52() && (prediction & SpecInt52Only))
                prediction = (prediction | SpecAnyIntAsDouble) & ~SpecInt52Only;
            if (prediction)
                changed |= mergePrediction(prediction);
            break;
        }
            
        case SetLocal: {
            VariableAccessData* variableAccessData = node->variableAccessData();
            changed |= variableAccessData->predict(node->child1()->prediction());
            break;
        }

        case UInt32ToNumber: {
            if (node->canSpeculateInt32(m_pass))
                changed |= mergePrediction(SpecInt32Only);
            else if (enableInt52())
                changed |= mergePrediction(SpecAnyInt);
            else
                changed |= mergePrediction(SpecBytecodeNumber);
            break;
        }

        case ValueAdd: {
            SpeculatedType left = node->child1()->prediction();
            SpeculatedType right = node->child2()->prediction();
            
            if (left && right) {
                if (isFullNumberOrBooleanSpeculationExpectingDefined(left)
                    && isFullNumberOrBooleanSpeculationExpectingDefined(right)) {
                    if (m_graph.addSpeculationMode(node, m_pass) != DontSpeculateInt32)
                        changed |= mergePrediction(SpecInt32Only);
                    else if (m_graph.addShouldSpeculateAnyInt(node))
                        changed |= mergePrediction(SpecInt52Only);
                    else
                        changed |= mergePrediction(speculatedDoubleTypeForPredictions(left, right));
                } else if (isStringOrStringObjectSpeculation(left) && isStringOrStringObjectSpeculation(right)) {
                    // left or right is definitely something other than a number.
                    changed |= mergePrediction(SpecString);
                } else {
                    changed |= mergePrediction(SpecInt32Only);
                    if (node->mayHaveDoubleResult())
                        changed |= mergePrediction(SpecBytecodeDouble);
                    if (node->mayHaveNonNumberResult())
                        changed |= mergePrediction(SpecString);
                }
            }
            break;
        }

        case ArithAdd: {
            SpeculatedType left = node->child1()->prediction();
            SpeculatedType right = node->child2()->prediction();
            
            if (left && right) {
                if (m_graph.addSpeculationMode(node, m_pass) != DontSpeculateInt32)
                    changed |= mergePrediction(SpecInt32Only);
                else if (m_graph.addShouldSpeculateAnyInt(node))
                    changed |= mergePrediction(SpecInt52Only);
                else if (isFullNumberOrBooleanSpeculation(left) && isFullNumberOrBooleanSpeculation(right))
                    changed |= mergePrediction(speculatedDoubleTypeForPredictions(left, right));
                else if (node->mayHaveNonIntResult() || (left & SpecBytecodeDouble) || (right & SpecBytecodeDouble))
                    changed |= mergePrediction(SpecInt32Only | SpecBytecodeDouble);
                else
                    changed |= mergePrediction(SpecInt32Only);
            }
            break;
        }
            
        case ArithSub: {
            SpeculatedType left = node->child1()->prediction();
            SpeculatedType right = node->child2()->prediction();

            if (left && right) {
                if (isFullNumberOrBooleanSpeculationExpectingDefined(left)
                    && isFullNumberOrBooleanSpeculationExpectingDefined(right)) {
                    if (m_graph.addSpeculationMode(node, m_pass) != DontSpeculateInt32)
                        changed |= mergePrediction(SpecInt32Only);
                    else if (m_graph.addShouldSpeculateAnyInt(node))
                        changed |= mergePrediction(SpecInt52Only);
                    else
                        changed |= mergePrediction(speculatedDoubleTypeForPredictions(left, right));
                } else if (node->mayHaveNonIntResult() || (left & SpecBytecodeDouble) || (right & SpecBytecodeDouble))
                    changed |= mergePrediction(SpecInt32Only | SpecBytecodeDouble);
                else
                    changed |= mergePrediction(SpecInt32Only);
            }
            break;
        }

        case ArithNegate: {
            SpeculatedType prediction = node->child1()->prediction();
            if (prediction) {
                if (isInt32OrBooleanSpeculation(prediction) && node->canSpeculateInt32(m_pass))
                    changed |= mergePrediction(SpecInt32Only);
                else if (m_graph.unaryArithShouldSpeculateAnyInt(node, m_pass))
                    changed |= mergePrediction(SpecInt52Only);
                else if (isBytecodeNumberSpeculation(prediction))
                    changed |= mergePrediction(speculatedDoubleTypeForPrediction(node->child1()->prediction()));
                else {
                    changed |= mergePrediction(SpecInt32Only);
                    if (node->mayHaveDoubleResult())
                        changed |= mergePrediction(SpecBytecodeDouble);
                }
            }
            break;
        }
        case ArithMin:
        case ArithMax: {
            SpeculatedType left = node->child1()->prediction();
            SpeculatedType right = node->child2()->prediction();
            
            if (left && right) {
                if (Node::shouldSpeculateInt32OrBooleanForArithmetic(node->child1().node(), node->child2().node())
                    && node->canSpeculateInt32(m_pass))
                    changed |= mergePrediction(SpecInt32Only);
                else
                    changed |= mergePrediction(speculatedDoubleTypeForPredictions(left, right));
            }
            break;
        }

        case ArithMul: {
            SpeculatedType left = node->child1()->prediction();
            SpeculatedType right = node->child2()->prediction();
            
            if (left && right) {
                // FIXME: We're currently relying on prediction propagation and backwards propagation
                // whenever we can, and only falling back on result flags if that fails. And the result
                // flags logic doesn't know how to use backwards propagation. We should get rid of the
                // prediction propagation logic and rely solely on the result type.
                if (isFullNumberOrBooleanSpeculationExpectingDefined(left)
                    && isFullNumberOrBooleanSpeculationExpectingDefined(right)) {
                    if (m_graph.binaryArithShouldSpeculateInt32(node, m_pass))
                        changed |= mergePrediction(SpecInt32Only);
                    else if (m_graph.binaryArithShouldSpeculateAnyInt(node, m_pass))
                        changed |= mergePrediction(SpecInt52Only);
                    else
                        changed |= mergePrediction(speculatedDoubleTypeForPredictions(left, right));
                } else {
                    if (node->mayHaveNonIntResult()
                        || (left & SpecBytecodeDouble)
                        || (right & SpecBytecodeDouble))
                        changed |= mergePrediction(SpecInt32Only | SpecBytecodeDouble);
                    else
                        changed |= mergePrediction(SpecInt32Only);
                }
            }
            break;
        }

        case ArithDiv:
        case ArithMod: {
            SpeculatedType left = node->child1()->prediction();
            SpeculatedType right = node->child2()->prediction();
            
            if (left && right) {
                if (isFullNumberOrBooleanSpeculationExpectingDefined(left)
                    && isFullNumberOrBooleanSpeculationExpectingDefined(right)) {
                    if (m_graph.binaryArithShouldSpeculateInt32(node, m_pass))
                        changed |= mergePrediction(SpecInt32Only);
                    else
                        changed |= mergePrediction(SpecBytecodeDouble);
                } else
                    changed |= mergePrediction(SpecInt32Only | SpecBytecodeDouble);
            }
            break;
        }

        case ArithAbs: {
            SpeculatedType childPrediction = node->child1()->prediction();
            if (isInt32OrBooleanSpeculation(childPrediction)
                && node->canSpeculateInt32(m_pass))
                changed |= mergePrediction(SpecInt32Only);
            else
                changed |= mergePrediction(SpecBytecodeDouble);
            break;
        }

        case GetByVal: {
            if (!node->child1()->prediction())
                break;
            
            ArrayMode arrayMode = node->arrayMode().refine(
                m_graph, node,
                node->child1()->prediction(),
                node->child2()->prediction(),
                SpecNone);
            
            switch (arrayMode.type()) {
            case Array::Int32:
                if (arrayMode.isOutOfBounds())
                    changed |= mergePrediction(node->getHeapPrediction() | SpecInt32Only);
                else
                    changed |= mergePrediction(SpecInt32Only);
                break;
            case Array::Double:
                if (arrayMode.isOutOfBounds())
                    changed |= mergePrediction(node->getHeapPrediction() | SpecDoubleReal);
                else
                    changed |= mergePrediction(SpecDoubleReal);
                break;
            case Array::Float32Array:
            case Array::Float64Array:
                changed |= mergePrediction(SpecFullDouble);
                break;
            case Array::Uint32Array:
                if (isInt32SpeculationForArithmetic(node->getHeapPrediction()))
                    changed |= mergePrediction(SpecInt32Only);
                else if (enableInt52())
                    changed |= mergePrediction(SpecAnyInt);
                else
                    changed |= mergePrediction(SpecInt32Only | SpecAnyIntAsDouble);
                break;
            case Array::Int8Array:
            case Array::Uint8Array:
            case Array::Int16Array:
            case Array::Uint16Array:
            case Array::Int32Array:
                changed |= mergePrediction(SpecInt32Only);
                break;
            default:
                changed |= mergePrediction(node->getHeapPrediction());
                break;
            }
            break;
        }

        case ToThis: {
            // ToThis in methods for primitive types should speculate primitive types in strict mode.
            ECMAMode ecmaMode = m_graph.executableFor(node->origin.semantic)->isStrictMode() ? StrictMode : NotStrictMode;
            if (ecmaMode == StrictMode) {
                if (node->child1()->shouldSpeculateBoolean()) {
                    changed |= mergePrediction(SpecBoolean);
                    break;
                }

                if (node->child1()->shouldSpeculateInt32()) {
                    changed |= mergePrediction(SpecInt32Only);
                    break;
                }

                if (enableInt52() && node->child1()->shouldSpeculateAnyInt()) {
                    changed |= mergePrediction(SpecAnyInt);
                    break;
                }

                if (node->child1()->shouldSpeculateNumber()) {
                    changed |= mergePrediction(SpecAnyInt);
                    break;
                }

                if (node->child1()->shouldSpeculateSymbol()) {
                    changed |= mergePrediction(SpecSymbol);
                    break;
                }

                if (node->child1()->shouldSpeculateStringIdent()) {
                    changed |= mergePrediction(SpecStringIdent);
                    break;
                }

                if (node->child1()->shouldSpeculateString()) {
                    changed |= mergePrediction(SpecString);
                    break;
                }
            } else {
                if (node->child1()->shouldSpeculateString()) {
                    changed |= mergePrediction(SpecStringObject);
                    break;
                }
            }

            SpeculatedType prediction = node->child1()->prediction();
            if (prediction) {
                if (prediction & ~SpecObject) {
                    // Wrapper objects are created only in sloppy mode.
                    if (ecmaMode != StrictMode) {
                        prediction &= SpecObject;
                        prediction = mergeSpeculations(prediction, SpecObjectOther);
                    }
                }
                changed |= mergePrediction(prediction);
            }
            break;
        }
            
        case ToPrimitive: {
            SpeculatedType child = node->child1()->prediction();
            if (child)
                changed |= mergePrediction(resultOfToPrimitive(child));
            break;
        }

        default:
            break;
        }

        m_changed |= changed;
    }
        
    void propagateForward()
    {
        for (Node* node : m_dependentNodes) {
            m_currentNode = node;
            propagate(m_currentNode);
        }
    }

    void propagateBackward()
    {
        for (unsigned i = m_dependentNodes.size(); i--;) {
            m_currentNode = m_dependentNodes[i];
            propagate(m_currentNode);
        }
    }
    
    void doDoubleVoting(Node* node, float weight)
    {
        // Loop pre-headers created by OSR entrypoint creation may have NaN weight to indicate
        // that we actually don't know they weight. Assume that they execute once. This turns
        // out to be an OK assumption since the pre-header doesn't have any meaningful code.
        if (weight != weight)
            weight = 1;
        
        switch (node->op()) {
        case ValueAdd:
        case ArithAdd:
        case ArithSub: {
            SpeculatedType left = node->child1()->prediction();
            SpeculatedType right = node->child2()->prediction();
                
            DoubleBallot ballot;
                
            if (isFullNumberSpeculation(left)
                && isFullNumberSpeculation(right)
                && !m_graph.addShouldSpeculateInt32(node, m_pass)
                && !m_graph.addShouldSpeculateAnyInt(node))
                ballot = VoteDouble;
            else
                ballot = VoteValue;
                
            m_graph.voteNode(node->child1(), ballot, weight);
            m_graph.voteNode(node->child2(), ballot, weight);
            break;
        }

        case ArithMul: {
            SpeculatedType left = node->child1()->prediction();
            SpeculatedType right = node->child2()->prediction();
                
            DoubleBallot ballot;
                
            if (isFullNumberSpeculation(left)
                && isFullNumberSpeculation(right)
                && !m_graph.binaryArithShouldSpeculateInt32(node, m_pass)
                && !m_graph.binaryArithShouldSpeculateAnyInt(node, m_pass))
                ballot = VoteDouble;
            else
                ballot = VoteValue;
                
            m_graph.voteNode(node->child1(), ballot, weight);
            m_graph.voteNode(node->child2(), ballot, weight);
            break;
        }

        case ArithMin:
        case ArithMax:
        case ArithMod:
        case ArithDiv: {
            SpeculatedType left = node->child1()->prediction();
            SpeculatedType right = node->child2()->prediction();
                
            DoubleBallot ballot;
                
            if (isFullNumberSpeculation(left)
                && isFullNumberSpeculation(right)
                && !m_graph.binaryArithShouldSpeculateInt32(node, m_pass))
                ballot = VoteDouble;
            else
                ballot = VoteValue;
                
            m_graph.voteNode(node->child1(), ballot, weight);
            m_graph.voteNode(node->child2(), ballot, weight);
            break;
        }

        case ArithAbs:
            DoubleBallot ballot;
            if (node->child1()->shouldSpeculateNumber()
                && !m_graph.unaryArithShouldSpeculateInt32(node, m_pass))
                ballot = VoteDouble;
            else
                ballot = VoteValue;
                
            m_graph.voteNode(node->child1(), ballot, weight);
            break;
                
        case ArithSqrt:
        case ArithCos:
        case ArithSin:
        case ArithTan:
        case ArithLog:
            if (node->child1()->shouldSpeculateNumber())
                m_graph.voteNode(node->child1(), VoteDouble, weight);
            else
                m_graph.voteNode(node->child1(), VoteValue, weight);
            break;
                
        case SetLocal: {
            SpeculatedType prediction = node->child1()->prediction();
            if (isDoubleSpeculation(prediction))
                node->variableAccessData()->vote(VoteDouble, weight);
            else if (!isFullNumberSpeculation(prediction)
                || isInt32Speculation(prediction) || isAnyIntSpeculation(prediction))
                node->variableAccessData()->vote(VoteValue, weight);
            break;
        }

        case PutByValDirect:
        case PutByVal:
        case PutByValAlias: {
            Edge child1 = m_graph.varArgChild(node, 0);
            Edge child2 = m_graph.varArgChild(node, 1);
            Edge child3 = m_graph.varArgChild(node, 2);
            m_graph.voteNode(child1, VoteValue, weight);
            m_graph.voteNode(child2, VoteValue, weight);
            switch (node->arrayMode().type()) {
            case Array::Double:
                m_graph.voteNode(child3, VoteDouble, weight);
                break;
            default:
                m_graph.voteNode(child3, VoteValue, weight);
                break;
            }
            break;
        }
            
        case MovHint:
            // Ignore these since they have no effect on in-DFG execution.
            break;
            
        default:
            m_graph.voteChildren(node, VoteValue, weight);
            break;
        }
    }
    
    void doRoundOfDoubleVoting()
    {
        for (unsigned i = 0; i < m_graph.m_variableAccessData.size(); ++i)
            m_graph.m_variableAccessData[i].find()->clearVotes();
        for (BlockIndex blockIndex = 0; blockIndex < m_graph.numBlocks(); ++blockIndex) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            ASSERT(block->isReachable);
            for (unsigned i = 0; i < block->size(); ++i) {
                m_currentNode = block->at(i);
                doDoubleVoting(m_currentNode, block->executionCount);
            }
        }
        for (unsigned i = 0; i < m_graph.m_variableAccessData.size(); ++i) {
            VariableAccessData* variableAccessData = &m_graph.m_variableAccessData[i];
            if (!variableAccessData->isRoot())
                continue;
            m_changed |= variableAccessData->tallyVotesForShouldUseDoubleFormat();
        }
        propagateThroughArgumentPositions();
        for (unsigned i = 0; i < m_graph.m_variableAccessData.size(); ++i) {
            VariableAccessData* variableAccessData = &m_graph.m_variableAccessData[i];
            if (!variableAccessData->isRoot())
                continue;
            m_changed |= variableAccessData->makePredictionForDoubleFormat();
        }
    }
    
    void propagateThroughArgumentPositions()
    {
        for (unsigned i = 0; i < m_graph.m_argumentPositions.size(); ++i)
            m_changed |= m_graph.m_argumentPositions[i].mergeArgumentPredictionAwareness();
    }

    // Sets any predictions that do not depends on other nodes.
    void processInvariants()
    {
        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            for (Node* node : *block) {
                m_currentNode = node;
                processInvariantsForNode();
            }
        }
    }

    void processInvariantsForNode()
    {
        switch (m_currentNode->op()) {
        case JSConstant: {
            SpeculatedType type = speculationFromValue(m_currentNode->asJSValue());
            if (type == SpecAnyIntAsDouble && enableInt52())
                type = SpecInt52Only;
            setPrediction(type);
            break;
        }
        case DoubleConstant: {
            SpeculatedType type = speculationFromValue(m_currentNode->asJSValue());
            setPrediction(type);
            break;
        }
        case BitAnd:
        case BitOr:
        case BitXor:
        case BitRShift:
        case BitLShift:
        case BitURShift:
        case ArithIMul:
        case ArithClz32: {
            setPrediction(SpecInt32Only);
            break;
        }

        case ArrayPop:
        case ArrayPush:
        case RegExpExec:
        case RegExpTest:
        case StringReplace:
        case StringReplaceRegExp:
        case GetById:
        case GetByIdFlush:
        case GetByIdWithThis:
        case TryGetById:
        case GetByValWithThis:
        case GetByOffset:
        case MultiGetByOffset:
        case GetDirectPname:
        case Call:
        case DirectCall:
        case TailCallInlinedCaller:
        case DirectTailCallInlinedCaller:
        case Construct:
        case DirectConstruct:
        case CallVarargs:
        case CallEval:
        case TailCallVarargsInlinedCaller:
        case ConstructVarargs:
        case CallForwardVarargs:
        case ConstructForwardVarargs:
        case TailCallForwardVarargsInlinedCaller:
        case GetGlobalVar:
        case GetGlobalLexicalVariable:
        case GetClosureVar:
        case GetFromArguments:
        case LoadFromJSMapBucket:
        case ToNumber:
        case GetArgument:
        case CallDOMGetter: {
            setPrediction(m_currentNode->getHeapPrediction());
            break;
        }

        case GetDynamicVar: {
            setPrediction(SpecBytecodeTop);
            break;
        }
            
        case GetGetterSetterByOffset:
        case GetExecutable: {
            setPrediction(SpecCellOther);
            break;
        }

        case GetGetter:
        case GetSetter:
        case GetCallee:
        case NewFunction:
        case NewGeneratorFunction:
        case NewAsyncFunction: {
            setPrediction(SpecFunction);
            break;
        }
            
        case GetArgumentCountIncludingThis: {
            setPrediction(SpecInt32Only);
            break;
        }

        case MapHash:
            setPrediction(SpecInt32Only);
            break;
        case GetMapBucket:
            setPrediction(SpecCellOther);
            break;
        case IsNonEmptyMapBucket:
            setPrediction(SpecBoolean);
            break;

        case GetRestLength: {
            setPrediction(SpecInt32Only);
            break;
        }

        case GetTypedArrayByteOffset:
        case GetArrayLength: {
            setPrediction(SpecInt32Only);
            break;
        }

        case StringCharCodeAt: {
            setPrediction(SpecInt32Only);
            break;
        }

        case ToLowerCase:
            setPrediction(SpecString);
            break;

        case ArithPow:
        case ArithSqrt:
        case ArithFRound:
        case ArithSin:
        case ArithCos:
        case ArithTan:
        case ArithLog: {
            setPrediction(SpecBytecodeDouble);
            break;
        }

        case ArithRound:
        case ArithFloor:
        case ArithCeil:
        case ArithTrunc: {
            if (isInt32OrBooleanSpeculation(m_currentNode->getHeapPrediction())
                && m_graph.roundShouldSpeculateInt32(m_currentNode, m_pass))
                setPrediction(SpecInt32Only);
            else
                setPrediction(SpecBytecodeDouble);
            break;
        }

        case ArithRandom: {
            setPrediction(SpecDoubleReal);
            break;
        }
        case DeleteByVal:
        case DeleteById:
        case LogicalNot:
        case CompareLess:
        case CompareLessEq:
        case CompareGreater:
        case CompareGreaterEq:
        case CompareEq:
        case CompareStrictEq:
        case CompareEqPtr:
        case OverridesHasInstance:
        case InstanceOf:
        case InstanceOfCustom:
        case IsEmpty:
        case IsUndefined:
        case IsBoolean:
        case IsNumber:
        case IsObject:
        case IsObjectOrNull:
        case IsFunction:
        case IsCellWithType:
        case IsTypedArrayView: {
            setPrediction(SpecBoolean);
            break;
        }

        case TypeOf: {
            setPrediction(SpecStringIdent);
            break;
        }
        case GetButterfly:
        case GetIndexedPropertyStorage:
        case AllocatePropertyStorage:
        case ReallocatePropertyStorage: {
            setPrediction(SpecOther);
            break;
        }

        case CheckDOM:
            break;

        case CallObjectConstructor: {
            setPrediction(SpecObject);
            break;
        }
        case SkipScope:
        case GetGlobalObject: {
            setPrediction(SpecObjectOther);
            break;
        }

        case ResolveScope: {
            setPrediction(SpecObjectOther);
            break;
        }
            
        case CreateThis:
        case NewObject: {
            setPrediction(SpecFinalObject);
            break;
        }
            
        case ArraySlice:
        case NewArrayWithSpread:
        case NewArray:
        case NewArrayWithSize:
        case CreateRest:
        case NewArrayBuffer: {
            setPrediction(SpecArray);
            break;
        }

        case Spread:
            setPrediction(SpecCellOther);
            break;
            
        case NewTypedArray: {
            setPrediction(speculationFromTypedArrayType(m_currentNode->typedArrayType()));
            break;
        }
            
        case NewRegexp: {
            setPrediction(SpecRegExpObject);
            break;
        }
            
        case CreateActivation: {
            setPrediction(SpecObjectOther);
            break;
        }
        
        case StringFromCharCode: {
            setPrediction(SpecString);
            m_currentNode->child1()->mergeFlags(NodeBytecodeUsesAsNumber | NodeBytecodeUsesAsInt);
            break;
        }
        case StringCharAt:
        case CallStringConstructor:
        case ToString:
        case MakeRope:
        case StrCat: {
            setPrediction(SpecString);
            break;
        }
        case NewStringObject: {
            setPrediction(SpecStringObject);
            break;
        }
            
        case CreateDirectArguments: {
            setPrediction(SpecDirectArguments);
            break;
        }
            
        case CreateScopedArguments: {
            setPrediction(SpecScopedArguments);
            break;
        }
            
        case CreateClonedArguments: {
            setPrediction(SpecObjectOther);
            break;
        }
            
        case FiatInt52: {
            RELEASE_ASSERT(enableInt52());
            setPrediction(SpecAnyInt);
            break;
        }

        case GetScope:
            setPrediction(SpecObjectOther);
            break;

        case In:
            setPrediction(SpecBoolean);
            break;

        case HasOwnProperty:
            setPrediction(SpecBoolean);
            break;

        case GetEnumerableLength: {
            setPrediction(SpecInt32Only);
            break;
        }
        case HasGenericProperty:
        case HasStructureProperty:
        case HasIndexedProperty: {
            setPrediction(SpecBoolean);
            break;
        }
        case GetPropertyEnumerator: {
            setPrediction(SpecCell);
            break;
        }
        case GetEnumeratorStructurePname: {
            setPrediction(SpecCell | SpecOther);
            break;
        }
        case GetEnumeratorGenericPname: {
            setPrediction(SpecCell | SpecOther);
            break;
        }
        case ToIndexString: {
            setPrediction(SpecString);
            break;
        }
        case ParseInt: {
            // We expect this node to almost always produce an int32. However,
            // it's possible it produces NaN or integers out of int32 range. We
            // rely on the heap prediction since the parseInt() call profiled
            // its result.
            setPrediction(m_currentNode->getHeapPrediction());
            break;
        }

        case GetLocal:
        case SetLocal:
        case UInt32ToNumber:
        case ValueAdd:
        case ArithAdd:
        case ArithSub:
        case ArithNegate:
        case ArithMin:
        case ArithMax:
        case ArithMul:
        case ArithDiv:
        case ArithMod:
        case ArithAbs:
        case GetByVal:
        case ToThis:
        case ToPrimitive: {
            m_dependentNodes.append(m_currentNode);
            break;
        }

        case PutByValAlias:
        case DoubleAsInt32:
        case GetLocalUnlinked:
        case CheckArray:
        case CheckTypeInfoFlags:
        case Arrayify:
        case ArrayifyToStructure:
        case CheckTierUpInLoop:
        case CheckTierUpAtReturn:
        case CheckTierUpAndOSREnter:
        case InvalidationPoint:
        case CheckInBounds:
        case ValueToInt32:
        case DoubleRep:
        case ValueRep:
        case Int52Rep:
        case Int52Constant:
        case Identity:
        case BooleanToNumber:
        case PhantomNewObject:
        case PhantomNewFunction:
        case PhantomNewGeneratorFunction:
        case PhantomNewAsyncFunction:
        case PhantomCreateActivation:
        case PhantomDirectArguments:
        case PhantomCreateRest:
        case PhantomSpread:
        case PhantomNewArrayWithSpread:
        case PhantomClonedArguments:
        case GetMyArgumentByVal:
        case GetMyArgumentByValOutOfBounds:
        case PutHint:
        case CheckStructureImmediate:
        case MaterializeNewObject:
        case MaterializeCreateActivation:
        case PutStack:
        case KillStack:
        case StoreBarrier:
        case FencedStoreBarrier:
        case GetStack:
        case GetRegExpObjectLastIndex:
        case SetRegExpObjectLastIndex:
        case RecordRegExpCachedResult:
        case LazyJSConstant:
        case CallDOM: {
            // This node should never be visible at this stage of compilation. It is
            // inserted by fixup(), which follows this phase.
            DFG_CRASH(m_graph, m_currentNode, "Unexpected node during prediction propagation");
            break;
        }
        
        case Phi:
            // Phis should not be visible here since we're iterating the all-but-Phi's
            // part of basic blocks.
            RELEASE_ASSERT_NOT_REACHED();
            break;
            
        case Upsilon:
            // These don't get inserted until we go into SSA.
            RELEASE_ASSERT_NOT_REACHED();
            break;

#ifndef NDEBUG
        // These get ignored because they don't return anything.
        case PutByValDirect:
        case PutByValWithThis:
        case PutByIdWithThis:
        case PutByVal:
        case PutClosureVar:
        case PutToArguments:
        case Return:
        case TailCall:
        case DirectTailCall:
        case TailCallVarargs:
        case TailCallForwardVarargs:
        case Throw:
        case PutById:
        case PutByIdFlush:
        case PutByIdDirect:
        case PutByOffset:
        case MultiPutByOffset:
        case PutGetterById:
        case PutSetterById:
        case PutGetterSetterById:
        case PutGetterByVal:
        case PutSetterByVal:
        case DefineDataProperty:
        case DefineAccessorProperty:
        case DFG::Jump:
        case Branch:
        case Switch:
        case ProfileType:
        case ProfileControlFlow:
        case ThrowStaticError:
        case ForceOSRExit:
        case SetArgument:
        case SetFunctionName:
        case CheckStructure:
        case CheckCell:
        case CheckNotEmpty:
        case CheckStringIdent:
        case CheckBadCell:
        case PutStructure:
        case Phantom:
        case Check:
        case PutGlobalVariable:
        case CheckTraps:
        case LogShadowChickenPrologue:
        case LogShadowChickenTail:
        case Unreachable:
        case LoopHint:
        case NotifyWrite:
        case ConstantStoragePointer:
        case MovHint:
        case ZombieHint:
        case ExitOK:
        case LoadVarargs:
        case ForwardVarargs:
        case PutDynamicVar:
        case NukeStructureAndSetButterfly:
            break;
            
        // This gets ignored because it only pretends to produce a value.
        case BottomValue:
            break;
            
        // This gets ignored because it already has a prediction.
        case ExtractOSREntryLocal:
            break;
            
        // These gets ignored because it doesn't do anything.
        case CountExecution:
        case PhantomLocal:
        case Flush:
            break;
            
        case LastNodeType:
            RELEASE_ASSERT_NOT_REACHED();
            break;
#else
        default:
            break;
#endif
        }
    }

    SpeculatedType resultOfToPrimitive(SpeculatedType type)
    {
        if (type & SpecObject) {
            // We try to be optimistic here about StringObjects since it's unlikely that
            // someone overrides the valueOf or toString methods.
            if (type & SpecStringObject && m_graph.canOptimizeStringObjectAccess(m_currentNode->origin.semantic))
                return mergeSpeculations(type & ~SpecObject, SpecString);

            return mergeSpeculations(type & ~SpecObject, SpecPrimitive);
        }

        return type;
    }

    Vector<Node*> m_dependentNodes;
    Node* m_currentNode;
    bool m_changed;
    PredictionPass m_pass; // We use different logic for considering predictions depending on how far along we are in propagation.
};

} // Anonymous namespace.
    
bool performPredictionPropagation(Graph& graph)
{
    return runPhase<PredictionPropagationPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

