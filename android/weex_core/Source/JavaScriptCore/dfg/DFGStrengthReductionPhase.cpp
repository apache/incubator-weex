/*
 * Copyright (C) 2013-2016 Apple Inc. All rights reserved.
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
#include "DFGStrengthReductionPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGAbstractHeap.h"
#include "DFGClobberize.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGPhase.h"
#include "DFGPredictionPropagationPhase.h"
#include "DFGVariableAccessDataDump.h"
#include "JSCInlines.h"
#include "MathCommon.h"
#include "RegExpConstructor.h"
#include "StringPrototype.h"
#include <cstdlib>
#include <wtf/text/StringBuilder.h>

namespace JSC { namespace DFG {

class StrengthReductionPhase : public Phase {
    static const bool verbose = false;
    
public:
    StrengthReductionPhase(Graph& graph)
        : Phase(graph, "strength reduction")
        , m_insertionSet(graph)
    {
    }
    
    bool run()
    {
        ASSERT(m_graph.m_fixpointState == FixpointNotConverged);
        
        m_changed = false;
        
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            m_block = m_graph.block(blockIndex);
            if (!m_block)
                continue;
            for (m_nodeIndex = 0; m_nodeIndex < m_block->size(); ++m_nodeIndex) {
                m_node = m_block->at(m_nodeIndex);
                handleNode();
            }
            m_insertionSet.execute(m_block);
        }
        
        return m_changed;
    }

private:
    void handleNode()
    {
        switch (m_node->op()) {
        case BitOr:
            handleCommutativity();

            if (m_node->child1().useKind() != UntypedUse && m_node->child2()->isInt32Constant() && !m_node->child2()->asInt32()) {
                convertToIdentityOverChild1();
                break;
            }
            break;
            
        case BitXor:
        case BitAnd:
            handleCommutativity();
            break;
            
        case BitLShift:
        case BitRShift:
        case BitURShift:
            if (m_node->child1().useKind() != UntypedUse && m_node->child2()->isInt32Constant() && !(m_node->child2()->asInt32() & 0x1f)) {
                convertToIdentityOverChild1();
                break;
            }
            break;
            
        case UInt32ToNumber:
            if (m_node->child1()->op() == BitURShift
                && m_node->child1()->child2()->isInt32Constant()
                && (m_node->child1()->child2()->asInt32() & 0x1f)
                && m_node->arithMode() != Arith::DoOverflow) {
                m_node->convertToIdentity();
                m_changed = true;
                break;
            }
            break;
            
        case ArithAdd:
            handleCommutativity();
            
            if (m_node->child2()->isInt32Constant() && !m_node->child2()->asInt32()) {
                convertToIdentityOverChild1();
                break;
            }
            break;
            
        case ArithMul: {
            handleCommutativity();
            Edge& child2 = m_node->child2();
            if (child2->isNumberConstant() && child2->asNumber() == 2) {
                switch (m_node->binaryUseKind()) {
                case DoubleRepUse:
                    // It is always valuable to get rid of a double multiplication by 2.
                    // We won't have half-register dependencies issues on x86 and we won't have to load the constants.
                    m_node->setOp(ArithAdd);
                    child2.setNode(m_node->child1().node());
                    m_changed = true;
                    break;
#if USE(JSVALUE64)
                case Int52RepUse:
#endif
                case Int32Use:
                    // For integers, we can only convert compatible modes.
                    // ArithAdd does handle do negative zero check for example.
                    if (m_node->arithMode() == Arith::CheckOverflow || m_node->arithMode() == Arith::Unchecked) {
                        m_node->setOp(ArithAdd);
                        child2.setNode(m_node->child1().node());
                        m_changed = true;
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        }
        case ArithSub:
            if (m_node->child2()->isInt32Constant()
                && m_node->isBinaryUseKind(Int32Use)) {
                int32_t value = m_node->child2()->asInt32();
                if (-value != value) {
                    m_node->setOp(ArithAdd);
                    m_node->child2().setNode(
                        m_insertionSet.insertConstant(
                            m_nodeIndex, m_node->origin, jsNumber(-value)));
                    m_changed = true;
                    break;
                }
            }
            break;

        case ArithPow:
            if (m_node->child2()->isNumberConstant()) {
                double yOperandValue = m_node->child2()->asNumber();
                if (yOperandValue == 1) {
                    convertToIdentityOverChild1();
                    m_changed = true;
                } else if (yOperandValue == 2) {
                    m_node->setOp(ArithMul);
                    m_node->child2() = m_node->child1();
                    m_changed = true;
                }
            }
            break;

        case ArithMod:
            // On Integers
            // In: ArithMod(ArithMod(x, const1), const2)
            // Out: Identity(ArithMod(x, const1))
            //     if const1 <= const2.
            if (m_node->binaryUseKind() == Int32Use
                && m_node->child2()->isInt32Constant()
                && m_node->child1()->op() == ArithMod
                && m_node->child1()->binaryUseKind() == Int32Use
                && m_node->child1()->child2()->isInt32Constant()
                && std::abs(m_node->child1()->child2()->asInt32()) <= std::abs(m_node->child2()->asInt32())) {
                    convertToIdentityOverChild1();
            }
            break;

        case ArithDiv:
            // Transform
            //    ArithDiv(x, constant)
            // Into
            //    ArithMul(x, 1 / constant)
            // if the operation has the same result.
            if (m_node->isBinaryUseKind(DoubleRepUse)
                && m_node->child2()->isNumberConstant()) {

                if (std::optional<double> reciprocal = safeReciprocalForDivByConst(m_node->child2()->asNumber())) {
                    Node* reciprocalNode = m_insertionSet.insertConstant(m_nodeIndex, m_node->origin, jsDoubleNumber(*reciprocal), DoubleConstant);
                    m_node->setOp(ArithMul);
                    m_node->child2() = Edge(reciprocalNode, DoubleRepUse);
                    m_changed = true;
                    break;
                }
            }
            break;

        case ValueRep:
        case Int52Rep:
        case DoubleRep: {
            // This short-circuits circuitous conversions, like ValueRep(DoubleRep(value)) or
            // even more complicated things. Like, it can handle a beast like
            // ValueRep(DoubleRep(Int52Rep(value))).
            
            // The only speculation that we would do beyond validating that we have a type that
            // can be represented a certain way is an Int32 check that would appear on Int52Rep
            // nodes. For now, if we see this and the final type we want is an Int52, we use it
            // as an excuse not to fold. The only thing we would need is a Int52RepInt32Use kind.
            bool hadInt32Check = false;
            if (m_node->op() == Int52Rep) {
                if (m_node->child1().useKind() != Int32Use)
                    break;
                hadInt32Check = true;
            }
            for (Node* node = m_node->child1().node(); ; node = node->child1().node()) {
                if (canonicalResultRepresentation(node->result()) ==
                    canonicalResultRepresentation(m_node->result())) {
                    m_insertionSet.insertCheck(m_nodeIndex, m_node);
                    if (hadInt32Check) {
                        // FIXME: Consider adding Int52RepInt32Use or even DoubleRepInt32Use,
                        // which would be super weird. The latter would only arise in some
                        // seriously circuitous conversions.
                        if (canonicalResultRepresentation(node->result()) != NodeResultJS)
                            break;
                        
                        m_insertionSet.insertCheck(
                            m_nodeIndex, m_node->origin, Edge(node, Int32Use));
                    }
                    m_node->child1() = node->defaultEdge();
                    m_node->convertToIdentity();
                    m_changed = true;
                    break;
                }
                
                switch (node->op()) {
                case Int52Rep:
                    if (node->child1().useKind() != Int32Use)
                        break;
                    hadInt32Check = true;
                    continue;
                    
                case DoubleRep:
                case ValueRep:
                    continue;
                    
                default:
                    break;
                }
                break;
            }
            break;
        }
            
        case Flush: {
            ASSERT(m_graph.m_form != SSA);
            
            Node* setLocal = nullptr;
            VirtualRegister local = m_node->local();
            
            for (unsigned i = m_nodeIndex; i--;) {
                Node* node = m_block->at(i);
                if (node->op() == SetLocal && node->local() == local) {
                    setLocal = node;
                    break;
                }
                if (accessesOverlap(m_graph, node, AbstractHeap(Stack, local)))
                    break;
            }
            
            if (!setLocal)
                break;
            
            // The Flush should become a PhantomLocal at this point. This means that we want the
            // local's value during OSR, but we don't care if the value is stored to the stack. CPS
            // rethreading can canonicalize PhantomLocals for us.
            m_node->convertFlushToPhantomLocal();
            m_graph.dethread();
            m_changed = true;
            break;
        }

        // FIXME: we should probably do this in constant folding but this currently relies on an OSR exit rule.
        // https://bugs.webkit.org/show_bug.cgi?id=154832
        case OverridesHasInstance: {
            if (!m_node->child2().node()->isCellConstant())
                break;

            if (m_node->child2().node()->asCell() != m_graph.globalObjectFor(m_node->origin.semantic)->functionProtoHasInstanceSymbolFunction()) {
                m_graph.convertToConstant(m_node, jsBoolean(true));
                m_changed = true;

            } else if (!m_graph.hasExitSite(m_node->origin.semantic, BadTypeInfoFlags)) {
                // We optimistically assume that we will not see a function that has a custom instanceof operation as they should be rare.
                m_insertionSet.insertNode(m_nodeIndex, SpecNone, CheckTypeInfoFlags, m_node->origin, OpInfo(ImplementsDefaultHasInstance), Edge(m_node->child1().node(), CellUse));
                m_graph.convertToConstant(m_node, jsBoolean(false));
                m_changed = true;
            }

            break;
        }

        // FIXME: We have a lot of string constant-folding rules here. It would be great to
        // move these to the abstract interpreter once AbstractValue can support LazyJSValue.
        // https://bugs.webkit.org/show_bug.cgi?id=155204

        case ValueAdd: {
            if (m_node->child1()->isConstant()
                && m_node->child2()->isConstant()
                && (!!m_node->child1()->tryGetString(m_graph) || !!m_node->child2()->tryGetString(m_graph))) {
                auto tryGetConstantString = [&] (Node* node) -> String {
                    String string = node->tryGetString(m_graph);
                    if (!string.isEmpty())
                        return string;
                    JSValue value = node->constant()->value();
                    if (!value)
                        return String();
                    if (value.isInt32())
                        return String::number(value.asInt32());
                    if (value.isNumber())
                        return String::numberToStringECMAScript(value.asNumber());
                    if (value.isBoolean())
                        return value.asBoolean() ? ASCIILiteral("true") : ASCIILiteral("false");
                    if (value.isNull())
                        return ASCIILiteral("null");
                    if (value.isUndefined())
                        return ASCIILiteral("undefined");
                    return String();
                };

                String leftString = tryGetConstantString(m_node->child1().node());
                if (!leftString)
                    break;
                String rightString = tryGetConstantString(m_node->child2().node());
                if (!rightString)
                    break;

                StringBuilder builder;
                builder.append(leftString);
                builder.append(rightString);
                m_node->convertToLazyJSConstant(
                    m_graph, LazyJSValue::newString(m_graph, builder.toString()));
                m_changed = true;
            }
            break;
        }

        case MakeRope:
        case StrCat: {
            String leftString = m_node->child1()->tryGetString(m_graph);
            if (!leftString)
                break;
            String rightString = m_node->child2()->tryGetString(m_graph);
            if (!rightString)
                break;
            String extraString;
            if (m_node->child3()) {
                extraString = m_node->child3()->tryGetString(m_graph);
                if (!extraString)
                    break;
            }

            StringBuilder builder;
            builder.append(leftString);
            builder.append(rightString);
            if (!!extraString)
                builder.append(extraString);

            m_node->convertToLazyJSConstant(
                m_graph, LazyJSValue::newString(m_graph, builder.toString()));
            m_changed = true;
            break;
        }

        case GetArrayLength: {
            if (m_node->arrayMode().type() == Array::Generic
                || m_node->arrayMode().type() == Array::String) {
                String string = m_node->child1()->tryGetString(m_graph);
                if (!!string) {
                    m_graph.convertToConstant(m_node, jsNumber(string.length()));
                    m_changed = true;
                    break;
                }
            }
            break;
        }

        case GetGlobalObject: {
            if (JSObject* object = m_node->child1()->dynamicCastConstant<JSObject*>(vm())) {
                m_graph.convertToConstant(m_node, object->globalObject());
                m_changed = true;
                break;
            }
            break;
        }

        case RegExpExec:
        case RegExpTest: {
            JSGlobalObject* globalObject = m_node->child1()->dynamicCastConstant<JSGlobalObject*>(vm());
            if (!globalObject) {
                if (verbose)
                    dataLog("Giving up because no global object.\n");
                break;
            }

            if (globalObject->isHavingABadTime()) {
                if (verbose)
                    dataLog("Giving up because bad time.\n");
                break;
            }

            Node* regExpObjectNode = m_node->child2().node();
            RegExp* regExp;
            if (RegExpObject* regExpObject = regExpObjectNode->dynamicCastConstant<RegExpObject*>(vm()))
                regExp = regExpObject->regExp();
            else if (regExpObjectNode->op() == NewRegexp)
                regExp = regExpObjectNode->castOperand<RegExp*>();
            else {
                if (verbose)
                    dataLog("Giving up because the regexp is unknown.\n");
                break;
            }

            Node* stringNode = m_node->child3().node();
            
            // NOTE: This mostly already protects us from having the compiler execute a regexp
            // operation on a ginormous string by preventing us from getting our hands on ginormous
            // strings in the first place.
            String string = m_node->child3()->tryGetString(m_graph);
            if (!string) {
                if (verbose)
                    dataLog("Giving up because the string is unknown.\n");
                break;
            }

            FrozenValue* regExpFrozenValue = m_graph.freeze(regExp);

            // Refuse to do things with regular expressions that have a ginormous number of
            // subpatterns.
            unsigned ginormousNumberOfSubPatterns = 1000;
            if (regExp->numSubpatterns() > ginormousNumberOfSubPatterns) {
                if (verbose)
                    dataLog("Giving up because of pattern limit.\n");
                break;
            }
            
            unsigned lastIndex;
            if (regExp->globalOrSticky()) {
                // This will only work if we can prove what the value of lastIndex is. To do this
                // safely, we need to execute the insertion set so that we see any previous strength
                // reductions. This is needed for soundness since otherwise the effectfulness of any
                // previous strength reductions would be invisible to us.
                executeInsertionSet();
                lastIndex = UINT_MAX;
                for (unsigned otherNodeIndex = m_nodeIndex; otherNodeIndex--;) {
                    Node* otherNode = m_block->at(otherNodeIndex);
                    if (otherNode == regExpObjectNode) {
                        lastIndex = 0;
                        break;
                    }
                    if (otherNode->op() == SetRegExpObjectLastIndex
                        && otherNode->child1() == regExpObjectNode
                        && otherNode->child2()->isInt32Constant()
                        && otherNode->child2()->asInt32() >= 0) {
                        lastIndex = static_cast<unsigned>(otherNode->child2()->asInt32());
                        break;
                    }
                    if (writesOverlap(m_graph, otherNode, RegExpObject_lastIndex))
                        break;
                }
                if (lastIndex == UINT_MAX) {
                    if (verbose)
                        dataLog("Giving up because the last index is not known.\n");
                    break;
                }
            } else
                lastIndex = 0;

            m_graph.watchpoints().addLazily(globalObject->havingABadTimeWatchpoint());
            
            Structure* structure = globalObject->regExpMatchesArrayStructure();
            if (structure->indexingType() != ArrayWithContiguous) {
                // This is further protection against a race with haveABadTime.
                if (verbose)
                    dataLog("Giving up because the structure has the wrong indexing type.\n");
                break;
            }
            m_graph.registerStructure(structure);

            RegExpConstructor* constructor = globalObject->regExpConstructor();
            FrozenValue* constructorFrozenValue = m_graph.freeze(constructor);

            MatchResult result;
            Vector<int> ovector;
            // We have to call the kind of match function that the main thread would have called.
            // Otherwise, we might not have the desired Yarr code compiled, and the match will fail.
            if (m_node->op() == RegExpExec) {
                int position;
                if (!regExp->matchConcurrently(vm(), string, lastIndex, position, ovector)) {
                    if (verbose)
                        dataLog("Giving up because match failed.\n");
                    break;
                }
                result.start = position;
                result.end = ovector[1];
            } else {
                if (!regExp->matchConcurrently(vm(), string, lastIndex, result)) {
                    if (verbose)
                        dataLog("Giving up because match failed.\n");
                    break;
                }
            }

            // We've constant-folded the regexp. Now we're committed to replacing RegExpExec/Test.

            m_changed = true;

            NodeOrigin origin = m_node->origin;

            m_insertionSet.insertNode(
                m_nodeIndex, SpecNone, Check, origin, m_node->children.justChecks());

            if (m_node->op() == RegExpExec) {
                if (result) {
                    RegisteredStructureSet* structureSet = m_graph.addStructureSet(structure);

                    // Create an array modeling the JS array that we will try to allocate. This is
                    // basically createRegExpMatchesArray but over C++ strings instead of JSStrings.
                    Vector<String> resultArray;
                    resultArray.append(string.substring(result.start, result.end - result.start));
                    for (unsigned i = 1; i <= regExp->numSubpatterns(); ++i) {
                        int start = ovector[2 * i];
                        if (start >= 0)
                            resultArray.append(string.substring(start, ovector[2 * i + 1] - start));
                        else
                            resultArray.append(String());
                    }

                    unsigned publicLength = resultArray.size();
                    unsigned vectorLength =
                        Butterfly::optimalContiguousVectorLength(structure, publicLength);

                    UniquedStringImpl* indexUID = vm().propertyNames->index.impl();
                    UniquedStringImpl* inputUID = vm().propertyNames->input.impl();
                    unsigned indexIndex = m_graph.identifiers().ensure(indexUID);
                    unsigned inputIndex = m_graph.identifiers().ensure(inputUID);

                    unsigned firstChild = m_graph.m_varArgChildren.size();
                    m_graph.m_varArgChildren.append(
                        m_insertionSet.insertConstantForUse(
                            m_nodeIndex, origin, structure, KnownCellUse));
                    ObjectMaterializationData* data = m_graph.m_objectMaterializationData.add();
            
                    m_graph.m_varArgChildren.append(
                        m_insertionSet.insertConstantForUse(
                            m_nodeIndex, origin, jsNumber(publicLength), KnownInt32Use));
                    data->m_properties.append(PublicLengthPLoc);
            
                    m_graph.m_varArgChildren.append(
                        m_insertionSet.insertConstantForUse(
                            m_nodeIndex, origin, jsNumber(vectorLength), KnownInt32Use));
                    data->m_properties.append(VectorLengthPLoc);

                    m_graph.m_varArgChildren.append(
                        m_insertionSet.insertConstantForUse(
                            m_nodeIndex, origin, jsNumber(result.start), UntypedUse));
                    data->m_properties.append(
                        PromotedLocationDescriptor(NamedPropertyPLoc, indexIndex));

                    m_graph.m_varArgChildren.append(Edge(stringNode, UntypedUse));
                    data->m_properties.append(
                        PromotedLocationDescriptor(NamedPropertyPLoc, inputIndex));

                    auto materializeString = [&] (const String& string) -> Node* {
                        if (string.isNull())
                            return nullptr;
                        if (string.isEmpty()) {
                            return m_insertionSet.insertConstant(
                                m_nodeIndex, origin, vm().smallStrings.emptyString());
                        }
                        LazyJSValue value = LazyJSValue::newString(m_graph, string);
                        return m_insertionSet.insertNode(
                            m_nodeIndex, SpecNone, LazyJSConstant, origin,
                            OpInfo(m_graph.m_lazyJSValues.add(value)));
                    };

                    for (unsigned i = 0; i < resultArray.size(); ++i) {
                        if (Node* node = materializeString(resultArray[i])) {
                            m_graph.m_varArgChildren.append(Edge(node, UntypedUse));
                            data->m_properties.append(
                                PromotedLocationDescriptor(IndexedPropertyPLoc, i));
                        }
                    }
            
                    Node* resultNode = m_insertionSet.insertNode(
                        m_nodeIndex, SpecArray, Node::VarArg, MaterializeNewObject, origin,
                        OpInfo(structureSet), OpInfo(data), firstChild,
                        m_graph.m_varArgChildren.size() - firstChild);
                
                    m_node->convertToIdentityOn(resultNode);
                } else
                    m_graph.convertToConstant(m_node, jsNull());
            } else
                m_graph.convertToConstant(m_node, jsBoolean(!!result));

            // Whether it's Exec or Test, we need to tell the constructor and RegExpObject what's up.
            // Because SetRegExpObjectLastIndex may exit and it clobbers exit state, we do that
            // first.
            
            if (regExp->globalOrSticky()) {
                m_insertionSet.insertNode(
                    m_nodeIndex, SpecNone, SetRegExpObjectLastIndex, origin,
                    Edge(regExpObjectNode, RegExpObjectUse),
                    m_insertionSet.insertConstantForUse(
                        m_nodeIndex, origin, jsNumber(result ? result.end : 0), UntypedUse));
                
                origin = origin.withInvalidExit();
            }

            if (result) {
                unsigned firstChild = m_graph.m_varArgChildren.size();
                m_graph.m_varArgChildren.append(
                    m_insertionSet.insertConstantForUse(
                        m_nodeIndex, origin, constructorFrozenValue, KnownCellUse));
                m_graph.m_varArgChildren.append(
                    m_insertionSet.insertConstantForUse(
                        m_nodeIndex, origin, regExpFrozenValue, KnownCellUse));
                m_graph.m_varArgChildren.append(Edge(stringNode, KnownCellUse));
                m_graph.m_varArgChildren.append(
                    m_insertionSet.insertConstantForUse(
                        m_nodeIndex, origin, jsNumber(result.start), KnownInt32Use));
                m_graph.m_varArgChildren.append(
                    m_insertionSet.insertConstantForUse(
                        m_nodeIndex, origin, jsNumber(result.end), KnownInt32Use));
                m_insertionSet.insertNode(
                    m_nodeIndex, SpecNone, Node::VarArg, RecordRegExpCachedResult, origin,
                    OpInfo(), OpInfo(), firstChild, m_graph.m_varArgChildren.size() - firstChild);

                origin = origin.withInvalidExit();
            }
            
            m_node->origin = origin;
            break;
        }

        case StringReplace:
        case StringReplaceRegExp: {
            Node* stringNode = m_node->child1().node();
            String string = stringNode->tryGetString(m_graph);
            if (!string)
                break;
            
            Node* regExpObjectNode = m_node->child2().node();
            RegExp* regExp;
            if (RegExpObject* regExpObject = regExpObjectNode->dynamicCastConstant<RegExpObject*>(vm()))
                regExp = regExpObject->regExp();
            else if (regExpObjectNode->op() == NewRegexp)
                regExp = regExpObjectNode->castOperand<RegExp*>();
            else {
                if (verbose)
                    dataLog("Giving up because the regexp is unknown.\n");
                break;
            }

            String replace = m_node->child3()->tryGetString(m_graph);
            if (!replace)
                break;

            StringBuilder builder;

            unsigned lastIndex = 0;
            unsigned startPosition = 0;
            bool ok = true;
            do {
                MatchResult result;
                Vector<int> ovector;
                // Model which version of match() is called by the main thread.
                if (replace.isEmpty() && regExp->global()) {
                    if (!regExp->matchConcurrently(vm(), string, startPosition, result)) {
                        ok = false;
                        break;
                    }
                } else {
                    int position;
                    if (!regExp->matchConcurrently(vm(), string, startPosition, position, ovector)) {
                        ok = false;
                        break;
                    }
                    
                    result.start = position;
                    result.end = ovector[1];
                }
                
                if (!result)
                    break;

                unsigned replLen = replace.length();
                if (lastIndex < result.start || replLen) {
                    builder.append(string, lastIndex, result.start - lastIndex);
                    if (replLen)
                        builder.append(substituteBackreferences(replace, string, ovector.data(), regExp));
                }

                lastIndex = result.end;
                startPosition = lastIndex;

                // special case of empty match
                if (result.empty()) {
                    startPosition++;
                    if (startPosition > string.length())
                        break;
                }
            } while (regExp->global());
            if (!ok)
                break;

            // We are committed at this point.
            m_changed = true;

            NodeOrigin origin = m_node->origin;

            if (regExp->global()) {
                m_insertionSet.insertNode(
                    m_nodeIndex, SpecNone, SetRegExpObjectLastIndex, origin,
                    Edge(regExpObjectNode, RegExpObjectUse),
                    m_insertionSet.insertConstantForUse(
                        m_nodeIndex, origin, jsNumber(0), UntypedUse));
                
                origin = origin.withInvalidExit();
            }

            if (!lastIndex && builder.isEmpty())
                m_node->convertToIdentityOn(stringNode);
            else {
                if (lastIndex < string.length())
                    builder.append(string, lastIndex, string.length() - lastIndex);
                
                m_node->convertToLazyJSConstant(
                    m_graph, LazyJSValue::newString(m_graph, builder.toString()));
            }

            m_node->origin = origin;
            break;
        }
            
        case Call:
        case Construct:
        case TailCallInlinedCaller:
        case TailCall: {
            ExecutableBase* executable = nullptr;
            Edge callee = m_graph.varArgChild(m_node, 0);
            if (JSFunction* function = callee->dynamicCastConstant<JSFunction*>(vm()))
                executable = function->executable();
            else if (callee->isFunctionAllocation())
                executable = callee->castOperand<FunctionExecutable*>();
            
            if (!executable)
                break;
            
            if (FunctionExecutable* functionExecutable = jsDynamicCast<FunctionExecutable*>(vm(), executable)) {
                // We need to update m_parameterSlots before we get to the backend, but we don't
                // want to do too much of this.
                unsigned numAllocatedArgs =
                    static_cast<unsigned>(functionExecutable->parameterCount()) + 1;
                
                if (numAllocatedArgs <= Options::maximumDirectCallStackSize()) {
                    m_graph.m_parameterSlots = std::max(
                        m_graph.m_parameterSlots,
                        Graph::parameterSlotsForArgCount(numAllocatedArgs));
                }
            }
            
            m_node->convertToDirectCall(m_graph.freeze(executable));
            m_changed = true;
            break;
        }

        default:
            break;
        }
    }
            
    void convertToIdentityOverChild(unsigned childIndex)
    {
        m_insertionSet.insertCheck(m_nodeIndex, m_node);
        m_node->children.removeEdge(childIndex ^ 1);
        m_node->convertToIdentity();
        m_changed = true;
    }
    
    void convertToIdentityOverChild1()
    {
        convertToIdentityOverChild(0);
    }
    
    void convertToIdentityOverChild2()
    {
        convertToIdentityOverChild(1);
    }
    
    void handleCommutativity()
    {
        // It's definitely not sound to swap the lhs and rhs when we may be performing effectful
        // calls on the lhs/rhs for valueOf.
        if (m_node->child1().useKind() == UntypedUse || m_node->child2().useKind() == UntypedUse)
            return;

        // If the right side is a constant then there is nothing left to do.
        if (m_node->child2()->hasConstant())
            return;
        
        // This case ensures that optimizations that look for x + const don't also have
        // to look for const + x.
        if (m_node->child1()->hasConstant() && !m_node->child1()->asJSValue().isCell()) {
            std::swap(m_node->child1(), m_node->child2());
            m_changed = true;
            return;
        }
        
        // This case ensures that CSE is commutativity-aware.
        if (m_node->child1().node() > m_node->child2().node()) {
            std::swap(m_node->child1(), m_node->child2());
            m_changed = true;
            return;
        }
    }

    void executeInsertionSet()
    {
        m_nodeIndex += m_insertionSet.execute(m_block);
    }
    
    InsertionSet m_insertionSet;
    BasicBlock* m_block;
    unsigned m_nodeIndex;
    Node* m_node;
    bool m_changed;
};
    
bool performStrengthReduction(Graph& graph)
{
    return runPhase<StrengthReductionPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

