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

#include "ArrayConstructor.h"
#include "DFGAbstractInterpreter.h"
#include "DOMJITGetterSetter.h"
#include "DOMJITSignature.h"
#include "GetByIdStatus.h"
#include "GetterSetter.h"
#include "HashMapImpl.h"
#include "JITOperations.h"
#include "MathCommon.h"
#include "Operations.h"
#include "PutByIdStatus.h"
#include "StringObject.h"

namespace JSC { namespace DFG {

template<typename AbstractStateType>
AbstractInterpreter<AbstractStateType>::AbstractInterpreter(Graph& graph, AbstractStateType& state)
    : m_codeBlock(graph.m_codeBlock)
    , m_graph(graph)
    , m_vm(m_graph.m_vm)
    , m_state(state)
{
    if (m_graph.m_form == SSA)
        m_phiChildren = std::make_unique<PhiChildren>(m_graph);
}

template<typename AbstractStateType>
AbstractInterpreter<AbstractStateType>::~AbstractInterpreter()
{
}

template<typename AbstractStateType>
typename AbstractInterpreter<AbstractStateType>::BooleanResult
AbstractInterpreter<AbstractStateType>::booleanResult(
    Node* node, AbstractValue& value)
{
    JSValue childConst = value.value();
    if (childConst) {
        if (childConst.toBoolean(m_codeBlock->globalObjectFor(node->origin.semantic)->globalExec()))
            return DefinitelyTrue;
        return DefinitelyFalse;
    }

    // Next check if we can fold because we know that the source is an object or string and does not equal undefined.
    if (isCellSpeculation(value.m_type) && !value.m_structure.isTop()) {
        bool allTrue = true;
        for (unsigned i = value.m_structure.size(); i--;) {
            RegisteredStructure structure = value.m_structure[i];
            if (structure->masqueradesAsUndefined(m_codeBlock->globalObjectFor(node->origin.semantic))
                || structure->typeInfo().type() == StringType) {
                allTrue = false;
                break;
            }
        }
        if (allTrue)
            return DefinitelyTrue;
    }
    
    return UnknownBooleanResult;
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::startExecuting()
{
    ASSERT(m_state.block());
    ASSERT(m_state.isValid());
    
    m_state.setDidClobber(false);
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::executeEdges(Node* node)
{
    m_graph.doToChildren(
        node,
        [&] (Edge& edge) {
            filterEdgeByUse(edge);
        });
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::executeKnownEdgeTypes(Node* node)
{
    // Some use kinds are required to not have checks, because we know somehow that the incoming
    // value will already have the type we want. In those cases, AI may not be smart enough to
    // prove that this is indeed the case. But the existance of the edge is enough to prove that
    // it is indeed the case. Taking advantage of this is not optional, since otherwise the DFG
    // and FTL backends may emit checks in a node that lacks a valid exit origin.
    m_graph.doToChildren(
        node,
        [&] (Edge& edge) {
            if (mayHaveTypeCheck(edge.useKind()))
                return;
            
            filterEdgeByUse(edge);
        });
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::verifyEdge(Node* node, Edge edge)
{
    if (!(forNode(edge).m_type & ~typeFilterFor(edge.useKind())))
        return;
    
    DFG_CRASH(m_graph, node, toCString("Edge verification error: ", node, "->", edge, " was expected to have type ", SpeculationDump(typeFilterFor(edge.useKind())), " but has type ", SpeculationDump(forNode(edge).m_type), " (", forNode(edge).m_type, ")").data());
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::verifyEdges(Node* node)
{
    DFG_NODE_DO_TO_CHILDREN(m_graph, node, verifyEdge);
}

inline bool isToThisAnIdentity(bool isStrictMode, AbstractValue& valueForNode)
{
    // We look at the type first since that will cover most cases and does not require iterating all the structures.
    if (isStrictMode) {
        if (valueForNode.m_type && !(valueForNode.m_type & SpecObjectOther))
            return true;
    } else {
        if (valueForNode.m_type && !(valueForNode.m_type & (~SpecObject | SpecObjectOther)))
            return true;
    }

    if ((isStrictMode || (valueForNode.m_type && !(valueForNode.m_type & ~SpecObject))) && valueForNode.m_structure.isFinite()) {
        bool overridesToThis = false;
        valueForNode.m_structure.forEach([&](RegisteredStructure structure) {
            TypeInfo type = structure->typeInfo();
            ASSERT(type.isObject() || type.type() == StringType || type.type() == SymbolType);
            if (!isStrictMode)
                ASSERT(type.isObject());
            // We don't need to worry about strings/symbols here since either:
            // 1) We are in strict mode and strings/symbols are not wrapped
            // 2) The AI has proven that the type of this is a subtype of object
            if (type.isObject() && type.overridesToThis())
                overridesToThis = true;
        });
        return !overridesToThis;
    }

    return false;
}

template<typename AbstractStateType>
bool AbstractInterpreter<AbstractStateType>::executeEffects(unsigned clobberLimit, Node* node)
{
    verifyEdges(node);
    
    m_state.createValueForNode(node);
    
    switch (node->op()) {
    case JSConstant:
    case DoubleConstant:
    case Int52Constant: {
        setBuiltInConstant(node, *node->constant());
        break;
    }

    case LazyJSConstant: {
        LazyJSValue value = node->lazyJSValue();
        switch (value.kind()) {
        case LazyJSValue::KnownValue:
            setConstant(node, value.value()->value());
            break;
        case LazyJSValue::SingleCharacterString:
        case LazyJSValue::KnownStringImpl:
        case LazyJSValue::NewStringImpl:
            forNode(node).setType(m_graph, SpecString);
            break;
        }
        break;
    }
        
    case Identity: {
        forNode(node) = forNode(node->child1());
        if (forNode(node).value())
            m_state.setFoundConstants(true);
        break;
    }
        
    case ExtractOSREntryLocal: {
        forNode(node).makeBytecodeTop();
        break;
    }
            
    case GetLocal: {
        VariableAccessData* variableAccessData = node->variableAccessData();
        AbstractValue value = m_state.variables().operand(variableAccessData->local().offset());
        // The value in the local should already be checked.
        DFG_ASSERT(m_graph, node, value.isType(typeFilterFor(variableAccessData->flushFormat())));
        if (value.value())
            m_state.setFoundConstants(true);
        forNode(node) = value;
        break;
    }
        
    case GetStack: {
        StackAccessData* data = node->stackAccessData();
        AbstractValue value = m_state.variables().operand(data->local);
        // The value in the local should already be checked.
        DFG_ASSERT(m_graph, node, value.isType(typeFilterFor(data->format)));
        if (value.value())
            m_state.setFoundConstants(true);
        forNode(node) = value;
        break;
    }
        
    case GetLocalUnlinked: {
        AbstractValue value = m_state.variables().operand(node->unlinkedLocal().offset());
        if (value.value())
            m_state.setFoundConstants(true);
        forNode(node) = value;
        break;
    }
        
    case SetLocal: {
        m_state.variables().operand(node->local()) = forNode(node->child1());
        break;
    }
        
    case PutStack: {
        m_state.variables().operand(node->stackAccessData()->local) = forNode(node->child1());
        break;
    }
        
    case MovHint: {
        // Don't need to do anything. A MovHint only informs us about what would have happened
        // in bytecode, but this code is just concerned with what is actually happening during
        // DFG execution.
        break;
    }
        
    case KillStack: {
        // This is just a hint telling us that the OSR state of the local is no longer inside the
        // flushed data.
        break;
    }
        
    case SetArgument:
        // Assert that the state of arguments has been set. SetArgument means that someone set
        // the argument values out-of-band, and currently this always means setting to a
        // non-clear value.
        ASSERT(!m_state.variables().operand(node->local()).isClear());
        break;
        
    case LoadVarargs:
    case ForwardVarargs: {
        // FIXME: ForwardVarargs should check if the count becomes known, and if it does, it should turn
        // itself into a straight-line sequence of GetStack/PutStack.
        // https://bugs.webkit.org/show_bug.cgi?id=143071
        clobberWorld(node->origin.semantic, clobberLimit);
        LoadVarargsData* data = node->loadVarargsData();
        m_state.variables().operand(data->count).setType(SpecInt32Only);
        for (unsigned i = data->limit - 1; i--;)
            m_state.variables().operand(data->start.offset() + i).makeHeapTop();
        break;
    }
            
    case BitAnd:
    case BitOr:
    case BitXor:
    case BitRShift:
    case BitLShift:
    case BitURShift: {
        if (node->child1().useKind() == UntypedUse || node->child2().useKind() == UntypedUse) {
            clobberWorld(node->origin.semantic, clobberLimit);
            forNode(node).setType(m_graph, SpecInt32Only);
            break;
        }

        JSValue left = forNode(node->child1()).value();
        JSValue right = forNode(node->child2()).value();
        if (left && right && left.isInt32() && right.isInt32()) {
            int32_t a = left.asInt32();
            int32_t b = right.asInt32();
            switch (node->op()) {
            case BitAnd:
                setConstant(node, JSValue(a & b));
                break;
            case BitOr:
                setConstant(node, JSValue(a | b));
                break;
            case BitXor:
                setConstant(node, JSValue(a ^ b));
                break;
            case BitRShift:
                setConstant(node, JSValue(a >> static_cast<uint32_t>(b)));
                break;
            case BitLShift:
                setConstant(node, JSValue(a << static_cast<uint32_t>(b)));
                break;
            case BitURShift:
                setConstant(node, JSValue(static_cast<uint32_t>(a) >> static_cast<uint32_t>(b)));
                break;
            default:
                RELEASE_ASSERT_NOT_REACHED();
                break;
            }
            break;
        }
        
        if (node->op() == BitAnd
            && (isBoolInt32Speculation(forNode(node->child1()).m_type) ||
                isBoolInt32Speculation(forNode(node->child2()).m_type))) {
            forNode(node).setType(SpecBoolInt32);
            break;
        }
        
        forNode(node).setType(SpecInt32Only);
        break;
    }
        
    case UInt32ToNumber: {
        JSValue child = forNode(node->child1()).value();
        if (doesOverflow(node->arithMode())) {
            if (enableInt52()) {
                if (child && child.isAnyInt()) {
                    int64_t machineInt = child.asAnyInt();
                    setConstant(node, jsNumber(static_cast<uint32_t>(machineInt)));
                    break;
                }
                forNode(node).setType(SpecAnyInt);
                break;
            }
            if (child && child.isInt32()) {
                uint32_t value = child.asInt32();
                setConstant(node, jsNumber(value));
                break;
            }
            forNode(node).setType(SpecAnyIntAsDouble);
            break;
        }
        if (child && child.isInt32()) {
            int32_t value = child.asInt32();
            if (value >= 0) {
                setConstant(node, jsNumber(value));
                break;
            }
        }
        forNode(node).setType(SpecInt32Only);
        break;
    }
        
    case BooleanToNumber: {
        JSValue concreteValue = forNode(node->child1()).value();
        if (concreteValue) {
            if (concreteValue.isBoolean())
                setConstant(node, jsNumber(concreteValue.asBoolean()));
            else
                setConstant(node, *m_graph.freeze(concreteValue));
            break;
        }
        AbstractValue& value = forNode(node);
        value = forNode(node->child1());
        if (node->child1().useKind() == UntypedUse && !(value.m_type & ~SpecBoolean))
            m_state.setFoundConstants(true);
        if (value.m_type & SpecBoolean) {
            value.merge(SpecBoolInt32);
            value.filter(~SpecBoolean);
        }
        break;
    }
            
    case DoubleAsInt32: {
        JSValue child = forNode(node->child1()).value();
        if (child && child.isNumber()) {
            double asDouble = child.asNumber();
            int32_t asInt = JSC::toInt32(asDouble);
            if (bitwise_cast<int64_t>(static_cast<double>(asInt)) == bitwise_cast<int64_t>(asDouble)) {
                setConstant(node, JSValue(asInt));
                break;
            }
        }
        forNode(node).setType(SpecInt32Only);
        break;
    }
            
    case ValueToInt32: {
        JSValue child = forNode(node->child1()).value();
        if (child) {
            if (child.isNumber()) {
                if (child.isInt32())
                    setConstant(node, child);
                else
                    setConstant(node, JSValue(JSC::toInt32(child.asDouble())));
                break;
            }
            if (child.isBoolean()) {
                setConstant(node, jsNumber(child.asBoolean()));
                break;
            }
            if (child.isUndefinedOrNull()) {
                setConstant(node, jsNumber(0));
                break;
            }
        }
        
        if (isBooleanSpeculation(forNode(node->child1()).m_type)) {
            forNode(node).setType(SpecBoolInt32);
            break;
        }
        
        forNode(node).setType(SpecInt32Only);
        break;
    }
        
    case DoubleRep: {
        JSValue child = forNode(node->child1()).value();
        if (std::optional<double> number = child.toNumberFromPrimitive()) {
            setConstant(node, jsDoubleNumber(*number));
            break;
        }

        SpeculatedType type = forNode(node->child1()).m_type;
        switch (node->child1().useKind()) {
        case NotCellUse: {
            if (type & SpecOther) {
                type &= ~SpecOther;
                type |= SpecDoublePureNaN | SpecBoolInt32; // Null becomes zero, undefined becomes NaN.
            }
            if (type & SpecBoolean) {
                type &= ~SpecBoolean;
                type |= SpecBoolInt32; // True becomes 1, false becomes 0.
            }
            type &= SpecBytecodeNumber;
            break;
        }

        case Int52RepUse:
        case NumberUse:
        case RealNumberUse:
            break;

        default:
            RELEASE_ASSERT_NOT_REACHED();
        }
        forNode(node).setType(type);
        forNode(node).fixTypeForRepresentation(m_graph, node);
        break;
    }
        
    case Int52Rep: {
        JSValue child = forNode(node->child1()).value();
        if (child && child.isAnyInt()) {
            setConstant(node, child);
            break;
        }
        
        forNode(node).setType(SpecInt32Only);
        break;
    }
        
    case ValueRep: {
        JSValue value = forNode(node->child1()).value();
        if (value) {
            setConstant(node, value);
            break;
        }
        
        forNode(node).setType(m_graph, forNode(node->child1()).m_type & ~SpecDoubleImpureNaN);
        forNode(node).fixTypeForRepresentation(m_graph, node);
        break;
    }
        
    case ValueAdd: {
        ASSERT(node->binaryUseKind() == UntypedUse);
        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).setType(m_graph, SpecString | SpecBytecodeNumber);
        break;
    }

    case StrCat: {
        forNode(node).setType(m_graph, SpecString);
        break;
    }
        
    case ArithAdd: {
        JSValue left = forNode(node->child1()).value();
        JSValue right = forNode(node->child2()).value();
        switch (node->binaryUseKind()) {
        case Int32Use:
            if (left && right && left.isInt32() && right.isInt32()) {
                if (!shouldCheckOverflow(node->arithMode())) {
                    setConstant(node, jsNumber(left.asInt32() + right.asInt32()));
                    break;
                }
                JSValue result = jsNumber(left.asNumber() + right.asNumber());
                if (result.isInt32()) {
                    setConstant(node, result);
                    break;
                }
            }
            forNode(node).setType(SpecInt32Only);
            break;
        case Int52RepUse:
            if (left && right && left.isAnyInt() && right.isAnyInt()) {
                JSValue result = jsNumber(left.asAnyInt() + right.asAnyInt());
                if (result.isAnyInt()) {
                    setConstant(node, result);
                    break;
                }
            }
            forNode(node).setType(SpecAnyInt);
            break;
        case DoubleRepUse:
            if (left && right && left.isNumber() && right.isNumber()) {
                setConstant(node, jsDoubleNumber(left.asNumber() + right.asNumber()));
                break;
            }
            forNode(node).setType(
                typeOfDoubleSum(
                    forNode(node->child1()).m_type, forNode(node->child2()).m_type));
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }

    case ArithClz32: {
        JSValue operand = forNode(node->child1()).value();
        if (std::optional<double> number = operand.toNumberFromPrimitive()) {
            uint32_t value = toUInt32(*number);
            setConstant(node, jsNumber(clz32(value)));
            break;
        }
        forNode(node).setType(SpecInt32Only);
        break;
    }

    case MakeRope: {
        forNode(node).set(m_graph, m_vm.stringStructure.get());
        break;
    }
            
    case ArithSub: {
        JSValue left = forNode(node->child1()).value();
        JSValue right = forNode(node->child2()).value();
        switch (node->binaryUseKind()) {
        case Int32Use:
            if (left && right && left.isInt32() && right.isInt32()) {
                if (!shouldCheckOverflow(node->arithMode())) {
                    setConstant(node, jsNumber(left.asInt32() - right.asInt32()));
                    break;
                }
                JSValue result = jsNumber(left.asNumber() - right.asNumber());
                if (result.isInt32()) {
                    setConstant(node, result);
                    break;
                }
            }
            forNode(node).setType(SpecInt32Only);
            break;
        case Int52RepUse:
            if (left && right && left.isAnyInt() && right.isAnyInt()) {
                JSValue result = jsNumber(left.asAnyInt() - right.asAnyInt());
                if (result.isAnyInt() || !shouldCheckOverflow(node->arithMode())) {
                    setConstant(node, result);
                    break;
                }
            }
            forNode(node).setType(SpecAnyInt);
            break;
        case DoubleRepUse:
            if (left && right && left.isNumber() && right.isNumber()) {
                setConstant(node, jsDoubleNumber(left.asNumber() - right.asNumber()));
                break;
            }
            forNode(node).setType(
                typeOfDoubleDifference(
                    forNode(node->child1()).m_type, forNode(node->child2()).m_type));
            break;
        case UntypedUse:
            clobberWorld(node->origin.semantic, clobberLimit);
            forNode(node).setType(m_graph, SpecBytecodeNumber);
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }
        
    case ArithNegate: {
        JSValue child = forNode(node->child1()).value();
        switch (node->child1().useKind()) {
        case Int32Use:
            if (child && child.isInt32()) {
                if (!shouldCheckOverflow(node->arithMode())) {
                    setConstant(node, jsNumber(-child.asInt32()));
                    break;
                }
                double doubleResult;
                if (shouldCheckNegativeZero(node->arithMode()))
                    doubleResult = -child.asNumber();
                else
                    doubleResult = 0 - child.asNumber();
                JSValue valueResult = jsNumber(doubleResult);
                if (valueResult.isInt32()) {
                    setConstant(node, valueResult);
                    break;
                }
            }
            forNode(node).setType(SpecInt32Only);
            break;
        case Int52RepUse:
            if (child && child.isAnyInt()) {
                double doubleResult;
                if (shouldCheckNegativeZero(node->arithMode()))
                    doubleResult = -child.asNumber();
                else
                    doubleResult = 0 - child.asNumber();
                JSValue valueResult = jsNumber(doubleResult);
                if (valueResult.isAnyInt()) {
                    setConstant(node, valueResult);
                    break;
                }
            }
            forNode(node).setType(SpecAnyInt);
            break;
        case DoubleRepUse:
            if (child && child.isNumber()) {
                setConstant(node, jsDoubleNumber(-child.asNumber()));
                break;
            }
            forNode(node).setType(
                typeOfDoubleNegation(
                    forNode(node->child1()).m_type));
            break;
        default:
            DFG_ASSERT(m_graph, node, node->child1().useKind() == UntypedUse);
            forNode(node).setType(SpecBytecodeNumber);
            break;
        }
        break;
    }
        
    case ArithMul: {
        JSValue left = forNode(node->child1()).value();
        JSValue right = forNode(node->child2()).value();
        switch (node->binaryUseKind()) {
        case Int32Use:
            if (left && right && left.isInt32() && right.isInt32()) {
                if (!shouldCheckOverflow(node->arithMode())) {
                    setConstant(node, jsNumber(left.asInt32() * right.asInt32()));
                    break;
                }
                double doubleResult = left.asNumber() * right.asNumber();
                if (!shouldCheckNegativeZero(node->arithMode()))
                    doubleResult += 0; // Sanitizes zero.
                JSValue valueResult = jsNumber(doubleResult);
                if (valueResult.isInt32()) {
                    setConstant(node, valueResult);
                    break;
                }
            }
            forNode(node).setType(SpecInt32Only);
            break;
        case Int52RepUse:
            if (left && right && left.isAnyInt() && right.isAnyInt()) {
                double doubleResult = left.asNumber() * right.asNumber();
                if (!shouldCheckNegativeZero(node->arithMode()))
                    doubleResult += 0;
                JSValue valueResult = jsNumber(doubleResult);
                if (valueResult.isAnyInt()) {
                    setConstant(node, valueResult);
                    break;
                }
            }
            forNode(node).setType(SpecAnyInt);
            break;
        case DoubleRepUse:
            if (left && right && left.isNumber() && right.isNumber()) {
                setConstant(node, jsDoubleNumber(left.asNumber() * right.asNumber()));
                break;
            }
            forNode(node).setType(
                typeOfDoubleProduct(
                    forNode(node->child1()).m_type, forNode(node->child2()).m_type));
            break;
        case UntypedUse:
            clobberWorld(node->origin.semantic, clobberLimit);
            forNode(node).setType(m_graph, SpecBytecodeNumber);
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }
        
    case ArithDiv: {
        JSValue left = forNode(node->child1()).value();
        JSValue right = forNode(node->child2()).value();
        switch (node->binaryUseKind()) {
        case Int32Use:
            if (left && right && left.isInt32() && right.isInt32()) {
                double doubleResult = left.asNumber() / right.asNumber();
                if (!shouldCheckOverflow(node->arithMode()))
                    doubleResult = toInt32(doubleResult);
                else if (!shouldCheckNegativeZero(node->arithMode()))
                    doubleResult += 0; // Sanitizes zero.
                JSValue valueResult = jsNumber(doubleResult);
                if (valueResult.isInt32()) {
                    setConstant(node, valueResult);
                    break;
                }
            }
            forNode(node).setType(SpecInt32Only);
            break;
        case DoubleRepUse:
            if (left && right && left.isNumber() && right.isNumber()) {
                setConstant(node, jsDoubleNumber(left.asNumber() / right.asNumber()));
                break;
            }
            forNode(node).setType(
                typeOfDoubleQuotient(
                    forNode(node->child1()).m_type, forNode(node->child2()).m_type));
            break;
        case UntypedUse:
            clobberWorld(node->origin.semantic, clobberLimit);
            forNode(node).setType(m_graph, SpecBytecodeNumber);
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }

    case ArithMod: {
        JSValue left = forNode(node->child1()).value();
        JSValue right = forNode(node->child2()).value();
        switch (node->binaryUseKind()) {
        case Int32Use:
            if (left && right && left.isInt32() && right.isInt32()) {
                double doubleResult = fmod(left.asNumber(), right.asNumber());
                if (!shouldCheckOverflow(node->arithMode()))
                    doubleResult = toInt32(doubleResult);
                else if (!shouldCheckNegativeZero(node->arithMode()))
                    doubleResult += 0; // Sanitizes zero.
                JSValue valueResult = jsNumber(doubleResult);
                if (valueResult.isInt32()) {
                    setConstant(node, valueResult);
                    break;
                }
            }
            forNode(node).setType(SpecInt32Only);
            break;
        case DoubleRepUse:
            if (left && right && left.isNumber() && right.isNumber()) {
                setConstant(node, jsDoubleNumber(fmod(left.asNumber(), right.asNumber())));
                break;
            }
            forNode(node).setType(
                typeOfDoubleBinaryOp(
                    forNode(node->child1()).m_type, forNode(node->child2()).m_type));
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }

    case ArithMin: {
        JSValue left = forNode(node->child1()).value();
        JSValue right = forNode(node->child2()).value();
        switch (node->binaryUseKind()) {
        case Int32Use:
            if (left && right && left.isInt32() && right.isInt32()) {
                setConstant(node, jsNumber(std::min(left.asInt32(), right.asInt32())));
                break;
            }
            forNode(node).setType(SpecInt32Only);
            break;
        case DoubleRepUse:
            if (left && right && left.isNumber() && right.isNumber()) {
                double a = left.asNumber();
                double b = right.asNumber();
                setConstant(node, jsDoubleNumber(a < b ? a : (b <= a ? b : a + b)));
                break;
            }
            forNode(node).setType(
                typeOfDoubleMinMax(
                    forNode(node->child1()).m_type, forNode(node->child2()).m_type));
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }
            
    case ArithMax: {
        JSValue left = forNode(node->child1()).value();
        JSValue right = forNode(node->child2()).value();
        switch (node->binaryUseKind()) {
        case Int32Use:
            if (left && right && left.isInt32() && right.isInt32()) {
                setConstant(node, jsNumber(std::max(left.asInt32(), right.asInt32())));
                break;
            }
            forNode(node).setType(SpecInt32Only);
            break;
        case DoubleRepUse:
            if (left && right && left.isNumber() && right.isNumber()) {
                double a = left.asNumber();
                double b = right.asNumber();
                setConstant(node, jsDoubleNumber(a > b ? a : (b >= a ? b : a + b)));
                break;
            }
            forNode(node).setType(
                typeOfDoubleMinMax(
                    forNode(node->child1()).m_type, forNode(node->child2()).m_type));
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }
            
    case ArithAbs: {
        JSValue child = forNode(node->child1()).value();
        switch (node->child1().useKind()) {
        case Int32Use:
            if (std::optional<double> number = child.toNumberFromPrimitive()) {
                JSValue result = jsNumber(fabs(*number));
                if (result.isInt32()) {
                    setConstant(node, result);
                    break;
                }
            }
            forNode(node).setType(SpecInt32Only);
            break;
        case DoubleRepUse:
            if (std::optional<double> number = child.toNumberFromPrimitive()) {
                setConstant(node, jsDoubleNumber(fabs(*number)));
                break;
            }
            forNode(node).setType(typeOfDoubleAbs(forNode(node->child1()).m_type));
            break;
        default:
            DFG_ASSERT(m_graph, node, node->child1().useKind() == UntypedUse);
            forNode(node).setType(SpecFullNumber);
            break;
        }
        break;
    }

    case ArithPow: {
        JSValue childY = forNode(node->child2()).value();
        if (childY && childY.isNumber()) {
            if (!childY.asNumber()) {
                setConstant(node, jsDoubleNumber(1));
                break;
            }

            JSValue childX = forNode(node->child1()).value();
            if (childX && childX.isNumber()) {
                setConstant(node, jsDoubleNumber(operationMathPow(childX.asNumber(), childY.asNumber())));
                break;
            }
        }
        forNode(node).setType(typeOfDoublePow(forNode(node->child1()).m_type, forNode(node->child2()).m_type));
        break;
    }

    case ArithRandom: {
        forNode(node).setType(m_graph, SpecDoubleReal);
        break;
    }

    case ArithRound:
    case ArithFloor:
    case ArithCeil:
    case ArithTrunc: {
        JSValue operand = forNode(node->child1()).value();
        if (std::optional<double> number = operand.toNumberFromPrimitive()) {
            double roundedValue = 0;
            if (node->op() == ArithRound)
                roundedValue = jsRound(*number);
            else if (node->op() == ArithFloor)
                roundedValue = floor(*number);
            else if (node->op() == ArithCeil)
                roundedValue = ceil(*number);
            else {
                ASSERT(node->op() == ArithTrunc);
                roundedValue = trunc(*number);
            }

            if (node->child1().useKind() == UntypedUse) {
                setConstant(node, jsNumber(roundedValue));
                break;
            }
            if (producesInteger(node->arithRoundingMode())) {
                int32_t roundedValueAsInt32 = static_cast<int32_t>(roundedValue);
                if (roundedValueAsInt32 == roundedValue) {
                    if (shouldCheckNegativeZero(node->arithRoundingMode())) {
                        if (roundedValueAsInt32 || !std::signbit(roundedValue)) {
                            setConstant(node, jsNumber(roundedValueAsInt32));
                            break;
                        }
                    } else {
                        setConstant(node, jsNumber(roundedValueAsInt32));
                        break;
                    }
                }
            } else {
                setConstant(node, jsDoubleNumber(roundedValue));
                break;
            }
        }
        if (node->child1().useKind() == DoubleRepUse) {
            if (producesInteger(node->arithRoundingMode()))
                forNode(node).setType(SpecInt32Only);
            else if (node->child1().useKind() == DoubleRepUse)
                forNode(node).setType(typeOfDoubleRounding(forNode(node->child1()).m_type));
        } else {
            DFG_ASSERT(m_graph, node, node->child1().useKind() == UntypedUse);
            forNode(node).setType(SpecFullNumber);
        }
        break;
    }
            
    case ArithSqrt:
        executeDoubleUnaryOpEffects(node, sqrt);
        break;

    case ArithFRound:
        executeDoubleUnaryOpEffects(node, [](double value) -> double { return static_cast<float>(value); });
        break;
        
    case ArithSin:
        executeDoubleUnaryOpEffects(node, sin);
        break;
    
    case ArithCos:
        executeDoubleUnaryOpEffects(node, cos);
        break;

    case ArithTan:
        executeDoubleUnaryOpEffects(node, tan);
        break;

    case ArithLog:
        executeDoubleUnaryOpEffects(node, log);
        break;
            
    case LogicalNot: {
        switch (booleanResult(node, forNode(node->child1()))) {
        case DefinitelyTrue:
            setConstant(node, jsBoolean(false));
            break;
        case DefinitelyFalse:
            setConstant(node, jsBoolean(true));
            break;
        default:
            forNode(node).setType(SpecBoolean);
            break;
        }
        break;
    }

    case MapHash: {
        if (JSValue key = forNode(node->child1()).value()) {
            if (std::optional<uint32_t> hash = concurrentJSMapHash(key)) {
                // Although C++ code uses uint32_t for the hash, the closest type in DFG IR is Int32
                // and that's what MapHash returns. So, we have to cast to int32_t to avoid large
                // unsigned values becoming doubles. This casting between signed and unsigned
                // happens in the assembly code we emit when we don't constant fold this node.
                setConstant(node, jsNumber(static_cast<int32_t>(*hash)));
                break;
            }
        }
        forNode(node).setType(SpecInt32Only);
        break;
    }

    case ToLowerCase: {
        forNode(node).setType(m_graph, SpecString);
        break;
    }

    case LoadFromJSMapBucket:
        forNode(node).makeHeapTop();
        break;

    case GetMapBucket:
        forNode(node).setType(m_graph, SpecCellOther);
        break;

    case IsNonEmptyMapBucket:
        forNode(node).setType(SpecBoolean);
        break;

    case IsEmpty:
    case IsUndefined:
    case IsBoolean:
    case IsNumber:
    case IsObject:
    case IsObjectOrNull:
    case IsFunction:
    case IsCellWithType:
    case IsTypedArrayView: {
        AbstractValue child = forNode(node->child1());
        if (child.value()) {
            bool constantWasSet = true;
            switch (node->op()) {
            case IsCellWithType:
                setConstant(node, jsBoolean(child.value().isCell() && child.value().asCell()->type() == node->queriedType()));
                break;
            case IsUndefined:
                setConstant(node, jsBoolean(
                    child.value().isCell()
                    ? child.value().asCell()->structure()->masqueradesAsUndefined(m_codeBlock->globalObjectFor(node->origin.semantic))
                    : child.value().isUndefined()));
                break;
            case IsBoolean:
                setConstant(node, jsBoolean(child.value().isBoolean()));
                break;
            case IsNumber:
                setConstant(node, jsBoolean(child.value().isNumber()));
                break;
            case IsObject:
                setConstant(node, jsBoolean(child.value().isObject()));
                break;
            case IsObjectOrNull:
                if (child.value().isObject()) {
                    JSObject* object = asObject(child.value());
                    if (object->type() == JSFunctionType)
                        setConstant(node, jsBoolean(false));
                    else if (!(object->inlineTypeFlags() & TypeOfShouldCallGetCallData))
                        setConstant(node, jsBoolean(!child.value().asCell()->structure()->masqueradesAsUndefined(m_codeBlock->globalObjectFor(node->origin.semantic))));
                    else {
                        // FIXME: This could just call getCallData.
                        // https://bugs.webkit.org/show_bug.cgi?id=144457
                        constantWasSet = false;
                    }
                } else
                    setConstant(node, jsBoolean(child.value().isNull()));
                break;
            case IsFunction:
                if (child.value().isObject()) {
                    JSObject* object = asObject(child.value());
                    if (object->type() == JSFunctionType)
                        setConstant(node, jsBoolean(true));
                    else if (!(object->inlineTypeFlags() & TypeOfShouldCallGetCallData))
                        setConstant(node, jsBoolean(false));
                    else {
                        // FIXME: This could just call getCallData.
                        // https://bugs.webkit.org/show_bug.cgi?id=144457
                        constantWasSet = false;
                    }
                } else
                    setConstant(node, jsBoolean(false));
                break;
            case IsEmpty:
                setConstant(node, jsBoolean(child.value().isEmpty()));
                break;
            case IsTypedArrayView:
                setConstant(node, jsBoolean(child.value().isObject() && isTypedView(child.value().getObject()->classInfo(m_vm)->typedArrayStorageType)));
                break;
            default:
                constantWasSet = false;
                break;
            }
            if (constantWasSet)
                break;
        }
        
        // FIXME: This code should really use AbstractValue::isType() and
        // AbstractValue::couldBeType().
        // https://bugs.webkit.org/show_bug.cgi?id=146870
        
        bool constantWasSet = false;
        switch (node->op()) {
        case IsEmpty: {
            if (child.m_type && !(child.m_type & SpecEmpty)) {
                setConstant(node, jsBoolean(false));
                constantWasSet = true;
                break;
            }

            if (child.m_type && !(child.m_type & ~SpecEmpty)) {
                setConstant(node, jsBoolean(true));
                constantWasSet = true;
                break;
            }

            break;
        }
        case IsUndefined:
            // FIXME: Use the masquerades-as-undefined watchpoint thingy.
            // https://bugs.webkit.org/show_bug.cgi?id=144456
            
            if (!(child.m_type & (SpecOther | SpecObjectOther))) {
                setConstant(node, jsBoolean(false));
                constantWasSet = true;
                break;
            }
            
            break;
        case IsBoolean:
            if (!(child.m_type & ~SpecBoolean)) {
                setConstant(node, jsBoolean(true));
                constantWasSet = true;
                break;
            }
            
            if (!(child.m_type & SpecBoolean)) {
                setConstant(node, jsBoolean(false));
                constantWasSet = true;
                break;
            }
            
            break;
        case IsNumber:
            if (!(child.m_type & ~SpecFullNumber)) {
                setConstant(node, jsBoolean(true));
                constantWasSet = true;
                break;
            }
            
            if (!(child.m_type & SpecFullNumber)) {
                setConstant(node, jsBoolean(false));
                constantWasSet = true;
                break;
            }
            
            break;
        case IsObject:
            if (!(child.m_type & ~SpecObject)) {
                setConstant(node, jsBoolean(true));
                constantWasSet = true;
                break;
            }
            
            if (!(child.m_type & SpecObject)) {
                setConstant(node, jsBoolean(false));
                constantWasSet = true;
                break;
            }
            
            break;
        case IsObjectOrNull:
            // FIXME: Use the masquerades-as-undefined watchpoint thingy.
            // https://bugs.webkit.org/show_bug.cgi?id=144456
            
            // These expressions are complicated to parse. A helpful way to parse this is that
            // "!(T & ~S)" means "T is a subset of S". Conversely, "!(T & S)" means "T is a
            // disjoint set from S". Things like "T - S" means that, provided that S is a
            // subset of T, it's the "set of all things in T but not in S". Things like "T | S"
            // mean the "union of T and S".
            
            // Is the child's type an object that isn't an other-object (i.e. object that could
            // have masquaredes-as-undefined traps) and isn't a function?  Then: we should fold
            // this to true.
            if (!(child.m_type & ~(SpecObject - SpecObjectOther - SpecFunction))) {
                setConstant(node, jsBoolean(true));
                constantWasSet = true;
                break;
            }
            
            // Is the child's type definitely not either of: an object that isn't a function,
            // or either undefined or null?  Then: we should fold this to false.  This means
            // for example that if it's any non-function object, including those that have
            // masquerades-as-undefined traps, then we don't fold. It also means we won't fold
            // if it's undefined-or-null, since the type bits don't distinguish between
            // undefined (which should fold to false) and null (which should fold to true).
            if (!(child.m_type & ((SpecObject - SpecFunction) | SpecOther))) {
                setConstant(node, jsBoolean(false));
                constantWasSet = true;
                break;
            }
            
            break;
        case IsFunction:
            if (!(child.m_type & ~SpecFunction)) {
                setConstant(node, jsBoolean(true));
                constantWasSet = true;
                break;
            }
            
            if (!(child.m_type & (SpecFunction | SpecObjectOther | SpecProxyObject))) {
                setConstant(node, jsBoolean(false));
                constantWasSet = true;
                break;
            }
            break;

        case IsCellWithType:
            if (!(child.m_type & ~node->speculatedTypeForQuery())) {
                setConstant(node, jsBoolean(true));
                constantWasSet = true;
                break;
            }
            if (!(child.m_type & node->speculatedTypeForQuery())) {
                setConstant(node, jsBoolean(false));
                constantWasSet = true;
                break;
            }
            break;

        case IsTypedArrayView:
            if (!(child.m_type & ~SpecTypedArrayView)) {
                setConstant(node, jsBoolean(true));
                constantWasSet = true;
                break;
            }
            if (!(child.m_type & SpecTypedArrayView)) {
                setConstant(node, jsBoolean(false));
                constantWasSet = true;
                break;
            }
            break;

        default:
            break;
        }
        if (constantWasSet)
            break;
        
        forNode(node).setType(SpecBoolean);
        break;
    }

    case TypeOf: {
        JSValue child = forNode(node->child1()).value();
        AbstractValue& abstractChild = forNode(node->child1());
        if (child) {
            JSValue typeString = jsTypeStringForValue(m_vm, m_codeBlock->globalObjectFor(node->origin.semantic), child);
            setConstant(node, *m_graph.freeze(typeString));
            break;
        }
        
        if (isFullNumberSpeculation(abstractChild.m_type)) {
            setConstant(node, *m_graph.freeze(m_vm.smallStrings.numberString()));
            break;
        }
        
        if (isStringSpeculation(abstractChild.m_type)) {
            setConstant(node, *m_graph.freeze(m_vm.smallStrings.stringString()));
            break;
        }

        // FIXME: We could use the masquerades-as-undefined watchpoint here.
        // https://bugs.webkit.org/show_bug.cgi?id=144456
        if (!(abstractChild.m_type & ~(SpecObject - SpecObjectOther - SpecFunction))) {
            setConstant(node, *m_graph.freeze(m_vm.smallStrings.objectString()));
            break;
        }
        
        if (isFunctionSpeculation(abstractChild.m_type)) {
            setConstant(node, *m_graph.freeze(m_vm.smallStrings.functionString()));
            break;
        }
        
        if (isBooleanSpeculation(abstractChild.m_type)) {
            setConstant(node, *m_graph.freeze(m_vm.smallStrings.booleanString()));
            break;
        }

        if (isSymbolSpeculation(abstractChild.m_type)) {
            setConstant(node, *m_graph.freeze(m_vm.smallStrings.symbolString()));
            break;
        }

        forNode(node).setType(m_graph, SpecStringIdent);
        break;
    }
            
    case CompareLess:
    case CompareLessEq:
    case CompareGreater:
    case CompareGreaterEq:
    case CompareEq: {
        JSValue leftConst = forNode(node->child1()).value();
        JSValue rightConst = forNode(node->child2()).value();
        if (leftConst && rightConst) {
            if (leftConst.isNumber() && rightConst.isNumber()) {
                double a = leftConst.asNumber();
                double b = rightConst.asNumber();
                switch (node->op()) {
                case CompareLess:
                    setConstant(node, jsBoolean(a < b));
                    break;
                case CompareLessEq:
                    setConstant(node, jsBoolean(a <= b));
                    break;
                case CompareGreater:
                    setConstant(node, jsBoolean(a > b));
                    break;
                case CompareGreaterEq:
                    setConstant(node, jsBoolean(a >= b));
                    break;
                case CompareEq:
                    setConstant(node, jsBoolean(a == b));
                    break;
                default:
                    RELEASE_ASSERT_NOT_REACHED();
                    break;
                }
                break;
            }
            
            if (leftConst.isString() && rightConst.isString()) {
                const StringImpl* a = asString(leftConst)->tryGetValueImpl();
                const StringImpl* b = asString(rightConst)->tryGetValueImpl();
                if (a && b) {
                    bool result;
                    if (node->op() == CompareEq)
                        result = WTF::equal(a, b);
                    else if (node->op() == CompareLess)
                        result = codePointCompare(a, b) < 0;
                    else if (node->op() == CompareLessEq)
                        result = codePointCompare(a, b) <= 0;
                    else if (node->op() == CompareGreater)
                        result = codePointCompare(a, b) > 0;
                    else if (node->op() == CompareGreaterEq)
                        result = codePointCompare(a, b) >= 0;
                    else
                        RELEASE_ASSERT_NOT_REACHED();
                    setConstant(node, jsBoolean(result));
                    break;
                }
            }

            if (node->op() == CompareEq && leftConst.isSymbol() && rightConst.isSymbol()) {
                setConstant(node, jsBoolean(asSymbol(leftConst) == asSymbol(rightConst)));
                break;
            }
        }
        
        if (node->op() == CompareEq) {
            SpeculatedType leftType = forNode(node->child1()).m_type;
            SpeculatedType rightType = forNode(node->child2()).m_type;
            if (!valuesCouldBeEqual(leftType, rightType)) {
                setConstant(node, jsBoolean(false));
                break;
            }

            if (leftType == SpecOther)
                std::swap(leftType, rightType);
            if (rightType == SpecOther) {
                // Undefined and Null are always equal when compared to eachother.
                if (!(leftType & ~SpecOther)) {
                    setConstant(node, jsBoolean(true));
                    break;
                }

                // Any other type compared to Null or Undefined is always false
                // as long as the MasqueradesAsUndefined watchpoint is valid.
                //
                // MasqueradesAsUndefined only matters for SpecObjectOther, other
                // cases are always "false".
                if (!(leftType & (SpecObjectOther | SpecOther))) {
                    setConstant(node, jsBoolean(false));
                    break;
                }

                if (!(leftType & SpecOther) && m_graph.masqueradesAsUndefinedWatchpointIsStillValid(node->origin.semantic)) {
                    JSGlobalObject* globalObject = m_graph.globalObjectFor(node->origin.semantic);
                    m_graph.watchpoints().addLazily(globalObject->masqueradesAsUndefinedWatchpoint());
                    setConstant(node, jsBoolean(false));
                    break;
                }
            }
        }
        
        if (node->child1() == node->child2()) {
            if (node->isBinaryUseKind(Int32Use) ||
                node->isBinaryUseKind(Int52RepUse) ||
                node->isBinaryUseKind(StringUse) ||
                node->isBinaryUseKind(BooleanUse) ||
                node->isBinaryUseKind(SymbolUse) ||
                node->isBinaryUseKind(StringIdentUse) ||
                node->isBinaryUseKind(ObjectUse) ||
                node->isBinaryUseKind(ObjectUse, ObjectOrOtherUse) ||
                node->isBinaryUseKind(ObjectOrOtherUse, ObjectUse)) {
                switch (node->op()) {
                case CompareLess:
                case CompareGreater:
                    setConstant(node, jsBoolean(false));
                    break;
                case CompareLessEq:
                case CompareGreaterEq:
                case CompareEq:
                    setConstant(node, jsBoolean(true));
                    break;
                default:
                    DFG_CRASH(m_graph, node, "Unexpected node type");
                    break;
                }
                break;
            }
        }
        
        forNode(node).setType(SpecBoolean);
        break;
    }
            
    case CompareStrictEq: {
        Node* leftNode = node->child1().node();
        Node* rightNode = node->child2().node();
        JSValue left = forNode(leftNode).value();
        JSValue right = forNode(rightNode).value();
        if (left && right) {
            if (left.isString() && right.isString()) {
                // We need this case because JSValue::strictEqual is otherwise too racy for
                // string comparisons.
                const StringImpl* a = asString(left)->tryGetValueImpl();
                const StringImpl* b = asString(right)->tryGetValueImpl();
                if (a && b) {
                    setConstant(node, jsBoolean(WTF::equal(a, b)));
                    break;
                }
            } else {
                setConstant(node, jsBoolean(JSValue::strictEqual(0, left, right)));
                break;
            }
        }
        
        SpeculatedType leftLUB = leastUpperBoundOfStrictlyEquivalentSpeculations(forNode(leftNode).m_type);
        SpeculatedType rightLUB = leastUpperBoundOfStrictlyEquivalentSpeculations(forNode(rightNode).m_type);
        if (!(leftLUB & rightLUB)) {
            setConstant(node, jsBoolean(false));
            break;
        }
        
        if (node->child1() == node->child2()) {
            if (node->isBinaryUseKind(BooleanUse) ||
                node->isBinaryUseKind(Int32Use) ||
                node->isBinaryUseKind(Int52RepUse) ||
                node->isBinaryUseKind(StringUse) ||
                node->isBinaryUseKind(StringIdentUse) ||
                node->isBinaryUseKind(SymbolUse) ||
                node->isBinaryUseKind(ObjectUse) ||
                node->isBinaryUseKind(MiscUse, UntypedUse) ||
                node->isBinaryUseKind(UntypedUse, MiscUse) ||
                node->isBinaryUseKind(StringIdentUse, NotStringVarUse) ||
                node->isBinaryUseKind(NotStringVarUse, StringIdentUse) ||
                node->isBinaryUseKind(StringUse, UntypedUse) ||
                node->isBinaryUseKind(UntypedUse, StringUse)) {
                setConstant(node, jsBoolean(true));
                break;
            }
        }

        forNode(node).setType(SpecBoolean);
        break;
    }
        
    case CompareEqPtr: {
        Node* childNode = node->child1().node();
        JSValue childValue = forNode(childNode).value();
        if (childValue) {
            setConstant(node, jsBoolean(childValue.isCell() && childValue.asCell() == node->cellOperand()->cell()));
            break;
        }
        
        forNode(node).setType(SpecBoolean);
        break;
    }
        
    case StringCharCodeAt:
        forNode(node).setType(SpecInt32Only);
        break;
        
    case StringFromCharCode:
        forNode(node).setType(m_graph, SpecString);
        break;

    case StringCharAt:
        forNode(node).set(m_graph, m_vm.stringStructure.get());
        break;
            
    case GetByVal: {
        switch (node->arrayMode().type()) {
        case Array::SelectUsingPredictions:
        case Array::Unprofiled:
        case Array::SelectUsingArguments:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        case Array::ForceExit:
            m_state.setIsValid(false);
            break;
        case Array::Undecided: {
            JSValue index = forNode(node->child2()).value();
            if (index && index.isInt32() && index.asInt32() >= 0) {
                setConstant(node, jsUndefined());
                break;
            }
            forNode(node).setType(SpecOther);
            break;
        }
        case Array::Generic:
            clobberWorld(node->origin.semantic, clobberLimit);
            forNode(node).makeHeapTop();
            break;
        case Array::String:
            if (node->arrayMode().isOutOfBounds()) {
                // If the watchpoint was still valid we could totally set this to be
                // SpecString | SpecOther. Except that we'd have to be careful. If we
                // tested the watchpoint state here then it could change by the time
                // we got to the backend. So to do this right, we'd have to get the
                // fixup phase to check the watchpoint state and then bake into the
                // GetByVal operation the fact that we're using a watchpoint, using
                // something like Array::SaneChain (except not quite, because that
                // implies an in-bounds access). None of this feels like it's worth it,
                // so we're going with TOP for now. The same thing applies to
                // clobbering the world.
                clobberWorld(node->origin.semantic, clobberLimit);
                forNode(node).makeHeapTop();
            } else
                forNode(node).set(m_graph, m_vm.stringStructure.get());
            break;
        case Array::DirectArguments:
        case Array::ScopedArguments:
            forNode(node).makeHeapTop();
            break;
        case Array::Int32:
            if (node->arrayMode().isOutOfBounds()) {
                clobberWorld(node->origin.semantic, clobberLimit);
                forNode(node).makeHeapTop();
            } else
                forNode(node).setType(SpecInt32Only);
            break;
        case Array::Double:
            if (node->arrayMode().isOutOfBounds()) {
                clobberWorld(node->origin.semantic, clobberLimit);
                forNode(node).makeHeapTop();
            } else if (node->arrayMode().isSaneChain())
                forNode(node).setType(SpecBytecodeDouble);
            else
                forNode(node).setType(SpecDoubleReal);
            break;
        case Array::Contiguous:
        case Array::ArrayStorage:
        case Array::SlowPutArrayStorage:
            if (node->arrayMode().isOutOfBounds())
                clobberWorld(node->origin.semantic, clobberLimit);
            forNode(node).makeHeapTop();
            break;
        case Array::Int8Array:
            forNode(node).setType(SpecInt32Only);
            break;
        case Array::Int16Array:
            forNode(node).setType(SpecInt32Only);
            break;
        case Array::Int32Array:
            forNode(node).setType(SpecInt32Only);
            break;
        case Array::Uint8Array:
            forNode(node).setType(SpecInt32Only);
            break;
        case Array::Uint8ClampedArray:
            forNode(node).setType(SpecInt32Only);
            break;
        case Array::Uint16Array:
            forNode(node).setType(SpecInt32Only);
            break;
        case Array::Uint32Array:
            if (node->shouldSpeculateInt32())
                forNode(node).setType(SpecInt32Only);
            else if (enableInt52() && node->shouldSpeculateAnyInt())
                forNode(node).setType(SpecAnyInt);
            else
                forNode(node).setType(SpecAnyIntAsDouble);
            break;
        case Array::Float32Array:
            forNode(node).setType(SpecFullDouble);
            break;
        case Array::Float64Array:
            forNode(node).setType(SpecFullDouble);
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        break;
    }
            
    case PutByValDirect:
    case PutByVal:
    case PutByValAlias: {
        switch (node->arrayMode().modeForPut().type()) {
        case Array::ForceExit:
            m_state.setIsValid(false);
            break;
        case Array::Generic:
            clobberWorld(node->origin.semantic, clobberLimit);
            break;
        case Array::Int32:
            if (node->arrayMode().isOutOfBounds())
                clobberWorld(node->origin.semantic, clobberLimit);
            break;
        case Array::Double:
            if (node->arrayMode().isOutOfBounds())
                clobberWorld(node->origin.semantic, clobberLimit);
            break;
        case Array::Contiguous:
        case Array::ArrayStorage:
            if (node->arrayMode().isOutOfBounds())
                clobberWorld(node->origin.semantic, clobberLimit);
            break;
        case Array::SlowPutArrayStorage:
            if (node->arrayMode().mayStoreToHole())
                clobberWorld(node->origin.semantic, clobberLimit);
            break;
        default:
            break;
        }
        break;
    }
            
    case ArrayPush:
        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).setType(SpecBytecodeNumber);
        break;

    case ArraySlice: {
        JSGlobalObject* globalObject = m_graph.globalObjectFor(node->origin.semantic);

        // FIXME: We could do better here if we prove that the
        // incoming value has only a single structure.
        RegisteredStructureSet structureSet;
        structureSet.add(m_graph.registerStructure(globalObject->originalArrayStructureForIndexingType(ArrayWithInt32)));
        structureSet.add(m_graph.registerStructure(globalObject->originalArrayStructureForIndexingType(ArrayWithContiguous)));
        structureSet.add(m_graph.registerStructure(globalObject->originalArrayStructureForIndexingType(ArrayWithDouble)));

        forNode(node).set(m_graph, structureSet);
        break;
    }
            
    case ArrayPop:
        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).makeHeapTop();
        break;
        
    case GetMyArgumentByVal:
    case GetMyArgumentByValOutOfBounds: {
        JSValue index = forNode(node->child2()).m_value;
        InlineCallFrame* inlineCallFrame = node->child1()->origin.semantic.inlineCallFrame;

        if (index && index.isInt32()) {
            // This pretends to return TOP for accesses that are actually proven out-of-bounds because
            // that's the conservative thing to do. Otherwise we'd need to write more code to mark such
            // paths as unreachable, or to return undefined. We could implement that eventually.
            
            unsigned argumentIndex = index.asUInt32() + node->numberOfArgumentsToSkip();
            if (inlineCallFrame) {
                if (argumentIndex < inlineCallFrame->arguments.size() - 1) {
                    forNode(node) = m_state.variables().operand(
                        virtualRegisterForArgument(argumentIndex + 1) + inlineCallFrame->stackOffset);
                    m_state.setFoundConstants(true);
                    break;
                }
            } else {
                if (argumentIndex < m_state.variables().numberOfArguments() - 1) {
                    forNode(node) = m_state.variables().argument(argumentIndex + 1);
                    m_state.setFoundConstants(true);
                    break;
                }
            }
        }
        
        if (inlineCallFrame) {
            // We have a bound on the types even though it's random access. Take advantage of this.
            
            AbstractValue result;
            for (unsigned i = 1 + node->numberOfArgumentsToSkip(); i < inlineCallFrame->arguments.size(); ++i) {
                result.merge(
                    m_state.variables().operand(
                        virtualRegisterForArgument(i) + inlineCallFrame->stackOffset));
            }
            
            if (node->op() == GetMyArgumentByValOutOfBounds)
                result.merge(SpecOther);
            
            if (result.value())
                m_state.setFoundConstants(true);
            
            forNode(node) = result;
            break;
        }
        
        forNode(node).makeHeapTop();
        break;
    }
            
    case RegExpExec:
        if (node->child2().useKind() == RegExpObjectUse
            && node->child3().useKind() == StringUse) {
            // This doesn't clobber the world since there are no conversions to perform.
        } else
            clobberWorld(node->origin.semantic, clobberLimit);
        if (JSValue globalObjectValue = forNode(node->child1()).m_value) {
            if (JSGlobalObject* globalObject = jsDynamicCast<JSGlobalObject*>(m_vm, globalObjectValue)) {
                if (!globalObject->isHavingABadTime()) {
                    m_graph.watchpoints().addLazily(globalObject->havingABadTimeWatchpoint());
                    Structure* structure = globalObject->regExpMatchesArrayStructure();
                    m_graph.registerStructure(structure);
                    forNode(node).set(m_graph, structure);
                    forNode(node).merge(SpecOther);
                    break;
                }
            }
        }
        forNode(node).setType(m_graph, SpecOther | SpecArray);
        break;

    case RegExpTest:
        if (node->child2().useKind() == RegExpObjectUse
            && node->child3().useKind() == StringUse) {
            // This doesn't clobber the world since there are no conversions to perform.
        } else
            clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).setType(SpecBoolean);
        break;
            
    case StringReplace:
    case StringReplaceRegExp:
        if (node->child1().useKind() == StringUse
            && node->child2().useKind() == RegExpObjectUse
            && node->child3().useKind() == StringUse) {
            // This doesn't clobber the world. It just reads and writes regexp state.
        } else
            clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).set(m_graph, m_vm.stringStructure.get());
        break;

    case Jump:
        break;
            
    case Branch: {
        Node* child = node->child1().node();
        BooleanResult result = booleanResult(node, forNode(child));
        if (result == DefinitelyTrue) {
            m_state.setBranchDirection(TakeTrue);
            break;
        }
        if (result == DefinitelyFalse) {
            m_state.setBranchDirection(TakeFalse);
            break;
        }
        // FIXME: The above handles the trivial cases of sparse conditional
        // constant propagation, but we can do better:
        // We can specialize the source variable's value on each direction of
        // the branch.
        m_state.setBranchDirection(TakeBoth);
        break;
    }
        
    case Switch: {
        // Nothing to do for now.
        // FIXME: Do sparse conditional things.
        break;
    }
            
    case Return:
        m_state.setIsValid(false);
        break;

    case TailCall:
    case DirectTailCall:
    case TailCallVarargs:
    case TailCallForwardVarargs:
        clobberWorld(node->origin.semantic, clobberLimit);
        m_state.setIsValid(false);
        break;
        
    case Throw:
    case ThrowStaticError:
        m_state.setIsValid(false);
        break;
            
    case ToPrimitive: {
        JSValue childConst = forNode(node->child1()).value();
        if (childConst && childConst.isNumber()) {
            setConstant(node, childConst);
            break;
        }
        
        ASSERT(node->child1().useKind() == UntypedUse);
        
        if (!(forNode(node->child1()).m_type & ~(SpecFullNumber | SpecBoolean | SpecString | SpecSymbol))) {
            m_state.setFoundConstants(true);
            forNode(node) = forNode(node->child1());
            break;
        }
        
        clobberWorld(node->origin.semantic, clobberLimit);
        
        forNode(node).setType(m_graph, SpecHeapTop & ~SpecObject);
        break;
    }

    case ToNumber: {
        JSValue childConst = forNode(node->child1()).value();
        if (childConst && childConst.isNumber()) {
            setConstant(node, childConst);
            break;
        }

        ASSERT(node->child1().useKind() == UntypedUse);

        if (!(forNode(node->child1()).m_type & ~SpecBytecodeNumber)) {
            m_state.setFoundConstants(true);
            forNode(node) = forNode(node->child1());
            break;
        }

        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).setType(m_graph, SpecBytecodeNumber);
        break;
    }
        
    case ToString:
    case CallStringConstructor: {
        switch (node->child1().useKind()) {
        case StringObjectUse:
            // This also filters that the StringObject has the primordial StringObject
            // structure.
            filter(
                node->child1(),
                m_graph.registerStructure(m_graph.globalObjectFor(node->origin.semantic)->stringObjectStructure()));
            break;
        case StringOrStringObjectUse:
        case NotCellUse:
            break;
        case CellUse:
        case UntypedUse:
            clobberWorld(node->origin.semantic, clobberLimit);
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        forNode(node).set(m_graph, m_vm.stringStructure.get());
        break;
    }
        
    case NewStringObject: {
        ASSERT(node->structure()->classInfo() == StringObject::info());
        forNode(node).set(m_graph, node->structure());
        break;
    }
            
    case NewArray:
        forNode(node).set(
            m_graph,
            m_graph.globalObjectFor(node->origin.semantic)->arrayStructureForIndexingTypeDuringAllocation(node->indexingType()));
        break;

    case NewArrayWithSpread:
        if (m_graph.isWatchingHavingABadTimeWatchpoint(node)) {
            // We've compiled assuming we're not having a bad time, so to be consistent
            // with StructureRegisterationPhase we must say we produce an original array
            // allocation structure.
            forNode(node).set(
                m_graph,
                m_graph.globalObjectFor(node->origin.semantic)->originalArrayStructureForIndexingType(ArrayWithContiguous));
        } else {
            forNode(node).set(
                m_graph,
                m_graph.globalObjectFor(node->origin.semantic)->arrayStructureForIndexingTypeDuringAllocation(ArrayWithContiguous));
        }

        break;

    case Spread:
        forNode(node).set(
            m_graph, m_vm.fixedArrayStructure.get());
        break;
        
    case NewArrayBuffer:
        forNode(node).set(
            m_graph,
            m_graph.globalObjectFor(node->origin.semantic)->arrayStructureForIndexingTypeDuringAllocation(node->indexingType()));
        break;

    case NewArrayWithSize:
        forNode(node).setType(m_graph, SpecArray);
        break;
        
    case NewTypedArray:
        switch (node->child1().useKind()) {
        case Int32Use:
            break;
        case UntypedUse:
            clobberWorld(node->origin.semantic, clobberLimit);
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        forNode(node).set(
            m_graph,
            m_graph.globalObjectFor(node->origin.semantic)->typedArrayStructureConcurrently(
                node->typedArrayType()));
        break;
        
    case NewRegexp:
        forNode(node).set(m_graph, m_graph.globalObjectFor(node->origin.semantic)->regExpStructure());
        break;
            
    case ToThis: {
        AbstractValue& source = forNode(node->child1());
        AbstractValue& destination = forNode(node);
        bool strictMode = m_graph.executableFor(node->origin.semantic)->isStrictMode();

        if (isToThisAnIdentity(strictMode, source)) {
            m_state.setFoundConstants(true);
            destination = source;
            break;
        }

        if (strictMode)
            destination.makeHeapTop();
        else {
            destination = source;
            destination.merge(SpecObject);
        }
        break;
    }

    case CreateThis: {
        // FIXME: We can fold this to NewObject if the incoming callee is a constant.
        forNode(node).setType(m_graph, SpecFinalObject);
        break;
    }
        
    case NewObject:
        ASSERT(!!node->structure().get());
        forNode(node).set(m_graph, node->structure());
        break;

    case CallObjectConstructor: {
        AbstractValue& source = forNode(node->child1());
        AbstractValue& destination = forNode(node);

        if (!(source.m_type & ~SpecObject)) {
            m_state.setFoundConstants(true);
            destination = source;
            break;
        }

        forNode(node).setType(m_graph, SpecObject);
        break;
    }

    case PhantomNewObject:
    case PhantomNewFunction:
    case PhantomNewGeneratorFunction:
    case PhantomNewAsyncFunction:
    case PhantomCreateActivation:
    case PhantomDirectArguments:
    case PhantomClonedArguments:
    case PhantomCreateRest:
    case PhantomSpread:
    case PhantomNewArrayWithSpread:
    case BottomValue:
        m_state.setDidClobber(true); // Prevent constant folding.
        // This claims to return bottom.
        break;
        
    case PutHint:
        break;
        
    case MaterializeNewObject: {
        forNode(node).set(m_graph, node->structureSet());
        break;
    }

    case CreateActivation:
    case MaterializeCreateActivation:
        forNode(node).set(
            m_graph, m_codeBlock->globalObjectFor(node->origin.semantic)->activationStructure());
        break;
        
    case CreateDirectArguments:
        forNode(node).set(m_graph, m_codeBlock->globalObjectFor(node->origin.semantic)->directArgumentsStructure());
        break;
        
    case CreateScopedArguments:
        forNode(node).set(m_graph, m_codeBlock->globalObjectFor(node->origin.semantic)->scopedArgumentsStructure());
        break;
        
    case CreateClonedArguments:
        if (!m_graph.isWatchingHavingABadTimeWatchpoint(node)) {
            forNode(node).setType(m_graph, SpecObject);
            break;
        }
        forNode(node).set(m_graph, m_codeBlock->globalObjectFor(node->origin.semantic)->clonedArgumentsStructure());
        break;

    case NewGeneratorFunction:
        forNode(node).set(
            m_graph, m_codeBlock->globalObjectFor(node->origin.semantic)->generatorFunctionStructure());
        break;

    case NewAsyncFunction:
        forNode(node).set(
            m_graph, m_codeBlock->globalObjectFor(node->origin.semantic)->asyncFunctionStructure());
        break;

    case NewFunction:
        forNode(node).set(
            m_graph, m_codeBlock->globalObjectFor(node->origin.semantic)->functionStructure());
        break;
        
    case GetCallee:
        if (FunctionExecutable* executable = jsDynamicCast<FunctionExecutable*>(m_vm, m_codeBlock->ownerExecutable())) {
            InferredValue* singleton = executable->singletonFunction();
            if (JSValue value = singleton->inferredValue()) {
                m_graph.watchpoints().addLazily(singleton);
                JSFunction* function = jsCast<JSFunction*>(value);
                setConstant(node, *m_graph.freeze(function));
                break;
            }
        }
        forNode(node).setType(m_graph, SpecFunction);
        break;
        
    case GetArgumentCountIncludingThis:
        forNode(node).setType(SpecInt32Only);
        break;
        
    case GetRestLength:
        forNode(node).setType(SpecInt32Only);
        break;
        
    case GetGetter: {
        JSValue base = forNode(node->child1()).m_value;
        if (base) {
            GetterSetter* getterSetter = jsCast<GetterSetter*>(base);
            if (!getterSetter->isGetterNull()) {
                setConstant(node, *m_graph.freeze(getterSetter->getterConcurrently()));
                break;
            }
        }
        
        forNode(node).setType(m_graph, SpecObject);
        break;
    }
        
    case GetSetter: {
        JSValue base = forNode(node->child1()).m_value;
        if (base) {
            GetterSetter* getterSetter = jsCast<GetterSetter*>(base);
            if (!getterSetter->isSetterNull()) {
                setConstant(node, *m_graph.freeze(getterSetter->setterConcurrently()));
                break;
            }
        }
        
        forNode(node).setType(m_graph, SpecObject);
        break;
    }
        
    case GetScope:
        if (JSValue base = forNode(node->child1()).m_value) {
            if (JSFunction* function = jsDynamicCast<JSFunction*>(m_vm, base)) {
                setConstant(node, *m_graph.freeze(function->scope()));
                break;
            }
        }
        forNode(node).setType(m_graph, SpecObjectOther);
        break;

    case SkipScope: {
        JSValue child = forNode(node->child1()).value();
        if (child) {
            setConstant(node, *m_graph.freeze(JSValue(jsCast<JSScope*>(child.asCell())->next())));
            break;
        }
        forNode(node).setType(m_graph, SpecObjectOther);
        break;
    }

    case GetGlobalObject: {
        JSValue child = forNode(node->child1()).value();
        if (child) {
            setConstant(node, *m_graph.freeze(JSValue(asObject(child)->globalObject())));
            break;
        }

        if (forNode(node->child1()).m_structure.isFinite()) {
            JSGlobalObject* globalObject = nullptr;
            bool ok = true;
            forNode(node->child1()).m_structure.forEach(
                [&] (RegisteredStructure structure) {
                    if (!globalObject)
                        globalObject = structure->globalObject();
                    else if (globalObject != structure->globalObject())
                        ok = false;
                });
            if (globalObject && ok) {
                setConstant(node, *m_graph.freeze(JSValue(globalObject)));
                break;
            }
        }

        forNode(node).setType(m_graph, SpecObjectOther);
        break;
    }

    case GetClosureVar:
        if (JSValue value = m_graph.tryGetConstantClosureVar(forNode(node->child1()), node->scopeOffset())) {
            setConstant(node, *m_graph.freeze(value));
            break;
        }
        forNode(node).makeBytecodeTop();
        break;
            
    case PutClosureVar:
        break;

    case GetRegExpObjectLastIndex:
        forNode(node).makeHeapTop();
        break;

    case SetRegExpObjectLastIndex:
    case RecordRegExpCachedResult:
        break;
        
    case GetFromArguments:
        forNode(node).makeHeapTop();
        break;
        
    case PutToArguments:
        break;

    case GetArgument:
        forNode(node).makeHeapTop();
        break;

    case TryGetById:
        // FIXME: This should constant fold at least as well as the normal GetById case.
        // https://bugs.webkit.org/show_bug.cgi?id=156422
        forNode(node).makeHeapTop();
        break;

    case GetById:
    case GetByIdFlush: {
        if (!node->prediction()) {
            m_state.setIsValid(false);
            break;
        }
        
        AbstractValue& value = forNode(node->child1());
        if (value.m_structure.isFinite()
            && (node->child1().useKind() == CellUse || !(value.m_type & ~SpecCell))) {
            UniquedStringImpl* uid = m_graph.identifiers()[node->identifierNumber()];
            GetByIdStatus status = GetByIdStatus::computeFor(value.m_structure.toStructureSet(), uid);
            if (status.isSimple()) {
                // Figure out what the result is going to be - is it TOP, a constant, or maybe
                // something more subtle?
                AbstractValue result;
                for (unsigned i = status.numVariants(); i--;) {
                    // This thing won't give us a variant that involves prototypes. If it did, we'd
                    // have more work to do here.
                    DFG_ASSERT(m_graph, node, status[i].conditionSet().isEmpty());

                    result.merge(
                        m_graph.inferredValueForProperty(
                            value, uid, status[i].offset(), m_state.structureClobberState()));
                }
                m_state.setFoundConstants(true);
                forNode(node) = result;
                break;
            }
        }

        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).makeHeapTop();
        break;
    }

    case GetByValWithThis:
    case GetByIdWithThis:
        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).makeHeapTop();
        break;
            
    case GetArrayLength: {
        JSArrayBufferView* view = m_graph.tryGetFoldableView(
            forNode(node->child1()).m_value, node->arrayMode());
        if (view) {
            setConstant(node, jsNumber(view->length()));
            break;
        }
        forNode(node).setType(SpecInt32Only);
        break;
    }

    case DeleteById:
    case DeleteByVal: {
        // FIXME: This could decide if the delete will be successful based on the set of structures that
        // we get from our base value. https://bugs.webkit.org/show_bug.cgi?id=156611
        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).setType(SpecBoolean);
        break;
    }
        
    case CheckStructure: {
        AbstractValue& value = forNode(node->child1());

        const RegisteredStructureSet& set = node->structureSet();
        
        // It's interesting that we could have proven that the object has a larger structure set
        // that includes the set we're testing. In that case we could make the structure check
        // more efficient. We currently don't.
        
        if (value.m_structure.isSubsetOf(set))
            m_state.setFoundConstants(true);

        SpeculatedType admittedTypes = SpecNone;
        switch (node->child1().useKind()) {
        case CellUse:
        case KnownCellUse:
            admittedTypes = SpecNone;
            break;
        case CellOrOtherUse:
            admittedTypes = SpecOther;
            break;
        default:
            DFG_CRASH(m_graph, node, "Bad use kind");
            break;
        }
        
        filter(value, set, admittedTypes);
        break;
    }
        
    case CheckStructureImmediate: {
        // FIXME: This currently can only reason about one structure at a time.
        // https://bugs.webkit.org/show_bug.cgi?id=136988
        
        AbstractValue& value = forNode(node->child1());
        const RegisteredStructureSet& set = node->structureSet();
        
        if (value.value()) {
            if (Structure* structure = jsDynamicCast<Structure*>(m_vm, value.value())) {
                if (set.contains(m_graph.registerStructure(structure))) {
                    m_state.setFoundConstants(true);
                    break;
                }
            }
            m_state.setIsValid(false);
            break;
        }
        
        if (m_phiChildren) {
            bool allGood = true;
            m_phiChildren->forAllTransitiveIncomingValues(
                node,
                [&] (Node* incoming) {
                    if (Structure* structure = incoming->dynamicCastConstant<Structure*>(m_vm)) {
                        if (set.contains(m_graph.registerStructure(structure)))
                            return;
                    }
                    allGood = false;
                });
            if (allGood) {
                m_state.setFoundConstants(true);
                break;
            }
        }
            
        if (RegisteredStructure structure = set.onlyStructure()) {
            filterByValue(node->child1(), *m_graph.freeze(structure.get()));
            break;
        }
        
        // Aw shucks, we can't do anything!
        break;
    }
        
    case PutStructure:
        if (!forNode(node->child1()).m_structure.isClear()) {
            if (forNode(node->child1()).m_structure.onlyStructure() == node->transition()->next)
                m_state.setFoundConstants(true);
            else {
                observeTransition(
                    clobberLimit, node->transition()->previous, node->transition()->next);
                forNode(node->child1()).changeStructure(m_graph, node->transition()->next);
            }
        }
        break;
    case GetButterfly:
    case AllocatePropertyStorage:
    case ReallocatePropertyStorage:
    case NukeStructureAndSetButterfly:
        // FIXME: We don't model the fact that the structureID is nuked, simply because currently
        // nobody would currently benefit from having that information. But it's a bug nonetheless.
        forNode(node).clear(); // The result is not a JS value.
        break;
    case CheckDOM: {
        JSValue constant = forNode(node->child1()).value();
        if (constant) {
            if (constant.isCell() && constant.asCell()->inherits(m_vm, node->classInfo())) {
                m_state.setFoundConstants(true);
                ASSERT(constant);
                break;
            }
        }

        AbstractValue& value = forNode(node->child1());

        if (value.m_structure.isSubClassOf(node->classInfo()))
            m_state.setFoundConstants(true);

        filterClassInfo(value, node->classInfo());
        break;
    }
    case CallDOMGetter: {
        CallDOMGetterData* callDOMGetterData = node->callDOMGetterData();
        DOMJIT::CallDOMGetterPatchpoint* patchpoint = callDOMGetterData->patchpoint;
        if (patchpoint->effect.writes)
            clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).setType(m_graph, callDOMGetterData->domJIT->resultType());
        break;
    }
    case CallDOM: {
        const DOMJIT::Signature* signature = node->signature();
        if (signature->effect.writes)
            clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).setType(m_graph, signature->result);
        break;
    }
    case CheckArray: {
        if (node->arrayMode().alreadyChecked(m_graph, node, forNode(node->child1()))) {
            m_state.setFoundConstants(true);
            break;
        }
        switch (node->arrayMode().type()) {
        case Array::String:
            filter(node->child1(), SpecString);
            break;
        case Array::Int32:
        case Array::Double:
        case Array::Contiguous:
        case Array::Undecided:
        case Array::ArrayStorage:
        case Array::SlowPutArrayStorage:
            break;
        case Array::DirectArguments:
            filter(node->child1(), SpecDirectArguments);
            break;
        case Array::ScopedArguments:
            filter(node->child1(), SpecScopedArguments);
            break;
        case Array::Int8Array:
            filter(node->child1(), SpecInt8Array);
            break;
        case Array::Int16Array:
            filter(node->child1(), SpecInt16Array);
            break;
        case Array::Int32Array:
            filter(node->child1(), SpecInt32Array);
            break;
        case Array::Uint8Array:
            filter(node->child1(), SpecUint8Array);
            break;
        case Array::Uint8ClampedArray:
            filter(node->child1(), SpecUint8ClampedArray);
            break;
        case Array::Uint16Array:
            filter(node->child1(), SpecUint16Array);
            break;
        case Array::Uint32Array:
            filter(node->child1(), SpecUint32Array);
            break;
        case Array::Float32Array:
            filter(node->child1(), SpecFloat32Array);
            break;
        case Array::Float64Array:
            filter(node->child1(), SpecFloat64Array);
            break;
        case Array::AnyTypedArray:
            filter(node->child1(), SpecTypedArrayView);
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        filterArrayModes(node->child1(), node->arrayMode().arrayModesThatPassFiltering());
        break;
    }
    case Arrayify: {
        if (node->arrayMode().alreadyChecked(m_graph, node, forNode(node->child1()))) {
            m_state.setFoundConstants(true);
            break;
        }
        ASSERT(node->arrayMode().conversion() == Array::Convert);
        clobberStructures(clobberLimit);
        filterArrayModes(node->child1(), node->arrayMode().arrayModesThatPassFiltering());
        break;
    }
    case ArrayifyToStructure: {
        AbstractValue& value = forNode(node->child1());
        if (value.m_structure.isSubsetOf(RegisteredStructureSet(node->structure())))
            m_state.setFoundConstants(true);
        clobberStructures(clobberLimit);
        
        // We have a bunch of options of how to express the abstract set at this point. Let set S
        // be the set of structures that the value had before clobbering and assume that all of
        // them are watchable. The new value should be the least expressible upper bound of the
        // intersection of "values that currently have structure = node->structure()" and "values
        // that have structure in S plus any structure transition-reachable from S". Assume that
        // node->structure() is not in S but it is transition-reachable from S. Then we would
        // like to say that the result is "values that have structure = node->structure() until
        // we invalidate", but there is no way to express this using the AbstractValue syntax. So
        // we must choose between:
        //
        // 1) "values that currently have structure = node->structure()". This is a valid
        //    superset of the value that we really want, and it's specific enough to satisfy the
        //    preconditions of the array access that this is guarding. It's also specific enough
        //    to allow relevant optimizations in the case that we didn't have a contradiction
        //    like in this example. Notice that in the abscence of any contradiction, this result
        //    is precise rather than being a conservative LUB.
        //
        // 2) "values that currently hava structure in S plus any structure transition-reachable
        //    from S". This is also a valid superset of the value that we really want, but it's
        //    not specific enough to satisfy the preconditions of the array access that this is
        //    guarding - so playing such shenanigans would preclude us from having assertions on
        //    the typing preconditions of any array accesses. This would also not be a desirable
        //    answer in the absence of a contradiction.
        //
        // Note that it's tempting to simply say that the resulting value is BOTTOM because of
        // the contradiction. That would be wrong, since we haven't hit an invalidation point,
        // yet.
        value.set(m_graph, node->structure());
        break;
    }
    case GetIndexedPropertyStorage: {
        JSArrayBufferView* view = m_graph.tryGetFoldableView(
            forNode(node->child1()).m_value, node->arrayMode());
        if (view)
            m_state.setFoundConstants(true);
        forNode(node).clear();
        break;
    }
    case ConstantStoragePointer: {
        forNode(node).clear();
        break; 
    }
        
    case GetTypedArrayByteOffset: {
        JSArrayBufferView* view = m_graph.tryGetFoldableView(forNode(node->child1()).m_value);
        if (view) {
            setConstant(node, jsNumber(view->byteOffset()));
            break;
        }
        forNode(node).setType(SpecInt32Only);
        break;
    }
        
    case GetByOffset: {
        StorageAccessData& data = node->storageAccessData();
        UniquedStringImpl* uid = m_graph.identifiers()[data.identifierNumber];

        // FIXME: The part of this that handles inferred property types relies on AI knowing the structure
        // right now. That's probably not optimal. In some cases, we may perform an optimization (usually
        // by something other than AI, maybe by CSE for example) that obscures AI's view of the structure
        // at the point where GetByOffset runs. Currently, when that happens, we'll have to rely entirely
        // on the type that ByteCodeParser was able to prove.
        AbstractValue value = m_graph.inferredValueForProperty(
            forNode(node->child2()), uid, data.offset, m_state.structureClobberState());

        // It's possible that the type that ByteCodeParser came up with is better.
        AbstractValue typeFromParsing;
        typeFromParsing.set(m_graph, data.inferredType, m_state.structureClobberState());
        value.filter(typeFromParsing);

        // If we decide that there does not exist any value that this can return, then it's probably
        // because the compilation was already invalidated.
        if (value.isClear())
            m_state.setIsValid(false);

        forNode(node) = value;
        if (value.m_value)
            m_state.setFoundConstants(true);
        break;
    }
        
    case GetGetterSetterByOffset: {
        StorageAccessData& data = node->storageAccessData();
        JSValue result = m_graph.tryGetConstantProperty(forNode(node->child2()), data.offset);
        if (result && jsDynamicCast<GetterSetter*>(m_vm, result)) {
            setConstant(node, *m_graph.freeze(result));
            break;
        }
        
        forNode(node).set(m_graph, m_graph.globalObjectFor(node->origin.semantic)->getterSetterStructure());
        break;
    }
        
    case MultiGetByOffset: {
        // This code will filter the base value in a manner that is possibly different (either more
        // or less precise) than the way it would be filtered if this was strength-reduced to a
        // CheckStructure. This is fine. It's legal for different passes over the code to prove
        // different things about the code, so long as all of them are sound. That even includes
        // one guy proving that code should never execute (due to a contradiction) and another guy
        // not finding that contradiction. If someone ever proved that there would be a
        // contradiction then there must always be a contradiction even if subsequent passes don't
        // realize it. This is the case here.
        
        // Ordinarily you have to be careful with calling setFoundConstants()
        // because of the effect on compile times, but this node is FTL-only.
        m_state.setFoundConstants(true);
        
        UniquedStringImpl* uid = m_graph.identifiers()[node->multiGetByOffsetData().identifierNumber];

        AbstractValue base = forNode(node->child1());
        RegisteredStructureSet baseSet;
        AbstractValue result;
        for (const MultiGetByOffsetCase& getCase : node->multiGetByOffsetData().cases) {
            RegisteredStructureSet set = getCase.set();
            set.filter(base);
            if (set.isEmpty())
                continue;
            baseSet.merge(set);

            switch (getCase.method().kind()) {
            case GetByOffsetMethod::Constant: {
                AbstractValue thisResult;
                thisResult.set(
                    m_graph,
                    *getCase.method().constant(),
                    m_state.structureClobberState());
                result.merge(thisResult);
                break;
            }

            case GetByOffsetMethod::Load: {
                result.merge(
                    m_graph.inferredValueForProperty(
                        set, uid, m_state.structureClobberState()));
                break;
            }

            default: {
                result.makeHeapTop();
                break;
            } }
        }
        
        if (forNode(node->child1()).changeStructure(m_graph, baseSet) == Contradiction)
            m_state.setIsValid(false);
        
        forNode(node) = result;
        break;
    }
            
    case PutByOffset: {
        break;
    }
        
    case MultiPutByOffset: {
        RegisteredStructureSet newSet;
        TransitionVector transitions;
        
        // Ordinarily you have to be careful with calling setFoundConstants()
        // because of the effect on compile times, but this node is FTL-only.
        m_state.setFoundConstants(true);
        
        AbstractValue base = forNode(node->child1());
        AbstractValue originalValue = forNode(node->child2());
        AbstractValue resultingValue;
        
        for (unsigned i = node->multiPutByOffsetData().variants.size(); i--;) {
            const PutByIdVariant& variant = node->multiPutByOffsetData().variants[i];
            RegisteredStructureSet thisSet = *m_graph.addStructureSet(variant.oldStructure());
            thisSet.filter(base);
            if (thisSet.isEmpty())
                continue;

            AbstractValue thisValue = originalValue;
            thisValue.filter(m_graph, variant.requiredType());
            resultingValue.merge(thisValue);
            
            if (variant.kind() == PutByIdVariant::Transition) {
                RegisteredStructure newStructure = m_graph.registerStructure(variant.newStructure());
                if (thisSet.onlyStructure() != newStructure) {
                    transitions.append(
                        Transition(m_graph.registerStructure(variant.oldStructureForTransition()), newStructure));
                } // else this is really a replace.
                newSet.add(newStructure);
            } else {
                ASSERT(variant.kind() == PutByIdVariant::Replace);
                newSet.merge(thisSet);
            }
        }
        
        observeTransitions(clobberLimit, transitions);
        if (forNode(node->child1()).changeStructure(m_graph, newSet) == Contradiction)
            m_state.setIsValid(false);
        forNode(node->child2()) = resultingValue;
        if (!!originalValue && !resultingValue)
            m_state.setIsValid(false);
        break;
    }
        
    case GetExecutable: {
        JSValue value = forNode(node->child1()).value();
        if (value) {
            JSFunction* function = jsDynamicCast<JSFunction*>(m_vm, value);
            if (function) {
                setConstant(node, *m_graph.freeze(function->executable()));
                break;
            }
        }
        forNode(node).setType(m_graph, SpecCellOther);
        break;
    }
    
    case CheckCell: {
        JSValue value = forNode(node->child1()).value();
        if (value == node->cellOperand()->value()) {
            m_state.setFoundConstants(true);
            ASSERT(value);
            break;
        }
        filterByValue(node->child1(), *node->cellOperand());
        break;
    }

    case CheckNotEmpty: {
        AbstractValue& value = forNode(node->child1());
        if (!(value.m_type & SpecEmpty)) {
            m_state.setFoundConstants(true);
            break;
        }
        
        filter(value, ~SpecEmpty);
        break;
    }

    case CheckStringIdent: {
        AbstractValue& value = forNode(node->child1());
        UniquedStringImpl* uid = node->uidOperand();
        ASSERT(!(value.m_type & ~SpecStringIdent)); // Edge filtering should have already ensured this.

        JSValue childConstant = value.value();
        if (childConstant) {
            ASSERT(childConstant.isString());
            if (asString(childConstant)->tryGetValueImpl() == uid) {
                m_state.setFoundConstants(true);
                break;
            }
        }

        filter(value, SpecStringIdent);
        break;
    }

    case CheckInBounds: {
        JSValue left = forNode(node->child1()).value();
        JSValue right = forNode(node->child2()).value();
        if (left && right && left.isInt32() && right.isInt32()
            && static_cast<uint32_t>(left.asInt32()) < static_cast<uint32_t>(right.asInt32())) {
            m_state.setFoundConstants(true);
            break;
        }
        break;
    }
        
    case PutById:
    case PutByIdFlush:
    case PutByIdDirect: {
        AbstractValue& value = forNode(node->child1());
        if (value.m_structure.isFinite()) {
            PutByIdStatus status = PutByIdStatus::computeFor(
                m_graph.globalObjectFor(node->origin.semantic),
                value.m_structure.toStructureSet(),
                m_graph.identifiers()[node->identifierNumber()],
                node->op() == PutByIdDirect);

            if (status.isSimple()) {
                RegisteredStructureSet newSet;
                TransitionVector transitions;
                
                for (unsigned i = status.numVariants(); i--;) {
                    const PutByIdVariant& variant = status[i];
                    if (variant.kind() == PutByIdVariant::Transition) {
                        RegisteredStructure newStructure = m_graph.registerStructure(variant.newStructure());
                        transitions.append(
                            Transition(
                                m_graph.registerStructure(variant.oldStructureForTransition()), newStructure));
                        newSet.add(newStructure);
                    } else {
                        ASSERT(variant.kind() == PutByIdVariant::Replace);
                        newSet.merge(*m_graph.addStructureSet(variant.oldStructure()));
                    }
                }
                
                if (status.numVariants() == 1 || isFTL(m_graph.m_plan.mode))
                    m_state.setFoundConstants(true);
                
                observeTransitions(clobberLimit, transitions);
                if (forNode(node->child1()).changeStructure(m_graph, newSet) == Contradiction)
                    m_state.setIsValid(false);
                break;
            }
        }
        
        clobberWorld(node->origin.semantic, clobberLimit);
        break;
    }

    case PutByValWithThis:
    case PutByIdWithThis:
        clobberWorld(node->origin.semantic, clobberLimit);
        break;

    case PutGetterById:
    case PutSetterById:
    case PutGetterSetterById:
    case PutGetterByVal:
    case PutSetterByVal: {
        clobberWorld(node->origin.semantic, clobberLimit);
        break;
    }

    case DefineDataProperty:
    case DefineAccessorProperty:
        clobberWorld(node->origin.semantic, clobberLimit);
        break;
        
    case In: {
        // FIXME: We can determine when the property definitely exists based on abstract
        // value information.
        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).setType(SpecBoolean);
        break;
    }

    case HasOwnProperty: {
        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).setType(SpecBoolean);
        break;
    }
            
    case GetEnumerableLength: {
        forNode(node).setType(SpecInt32Only);
        break;
    }
    case HasGenericProperty: {
        forNode(node).setType(SpecBoolean);
        break;
    }
    case HasStructureProperty: {
        forNode(node).setType(SpecBoolean);
        break;
    }
    case HasIndexedProperty: {
        ArrayMode mode = node->arrayMode();
        switch (mode.type()) {
        case Array::Int32:
        case Array::Double:
        case Array::Contiguous:
        case Array::ArrayStorage: {
            break;
        }
        default: {
            clobberWorld(node->origin.semantic, clobberLimit);
            break;
        }
        }
        forNode(node).setType(SpecBoolean);
        break;
    }
    case GetDirectPname: {
        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).makeHeapTop();
        break;
    }
    case GetPropertyEnumerator: {
        forNode(node).setType(m_graph, SpecCell);
        break;
    }
    case GetEnumeratorStructurePname: {
        forNode(node).setType(m_graph, SpecString | SpecOther);
        break;
    }
    case GetEnumeratorGenericPname: {
        forNode(node).setType(m_graph, SpecString | SpecOther);
        break;
    }
    case ToIndexString: {
        forNode(node).setType(m_graph, SpecString);
        break;
    }

    case GetGlobalVar:
        forNode(node).makeHeapTop();
        break;

    case GetGlobalLexicalVariable:
        forNode(node).makeBytecodeTop();
        break;

    case GetDynamicVar:
        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).makeBytecodeTop();
        break;

    case PutDynamicVar:
        clobberWorld(node->origin.semantic, clobberLimit);
        break;

    case ResolveScope:
        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).setType(m_graph, SpecObject);
        break;
        
    case PutGlobalVariable:
    case NotifyWrite:
        break;
            
    case OverridesHasInstance:
        forNode(node).setType(SpecBoolean);
        break;
            
    case InstanceOf:
        // Sadly, we don't propagate the fact that we've done InstanceOf
        forNode(node).setType(SpecBoolean);
        break;

    case InstanceOfCustom:
        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).setType(SpecBoolean);
        break;
            
    case Phi:
        RELEASE_ASSERT(m_graph.m_form == SSA);
        forNode(node) = forNode(NodeFlowProjection(node, NodeFlowProjection::Shadow));
        // The state of this node would have already been decided, but it may have become a
        // constant, in which case we'd like to know.
        if (forNode(node).m_value)
            m_state.setFoundConstants(true);
        break;
        
    case Upsilon: {
        NodeFlowProjection shadow(node->phi(), NodeFlowProjection::Shadow);
        if (shadow.isStillValid()) {
            m_state.createValueForNode(shadow);
            forNode(shadow) = forNode(node->child1());
        }
        break;
    }
        
    case Flush:
    case PhantomLocal:
        break;
            
    case Call:
    case TailCallInlinedCaller:
    case Construct:
    case CallVarargs:
    case CallForwardVarargs:
    case TailCallVarargsInlinedCaller:
    case ConstructVarargs:
    case ConstructForwardVarargs:
    case TailCallForwardVarargsInlinedCaller:
    case CallEval:
    case DirectCall:
    case DirectConstruct:
    case DirectTailCallInlinedCaller:
        clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).makeHeapTop();
        break;

    case ForceOSRExit:
    case CheckBadCell:
        m_state.setIsValid(false);
        break;
        
    case InvalidationPoint:
        forAllValues(clobberLimit, AbstractValue::observeInvalidationPointFor);
        m_state.setStructureClobberState(StructuresAreWatched);
        break;

    case CheckTraps:
    case LogShadowChickenPrologue:
    case LogShadowChickenTail:
        break;

    case ProfileType:
    case ProfileControlFlow:
    case Phantom:
    case CountExecution:
    case CheckTierUpInLoop:
    case CheckTierUpAtReturn:
    case CheckTypeInfoFlags:
        break;

    case ParseInt: {
        AbstractValue value = forNode(node->child1());
        if (value.m_type && !(value.m_type & ~SpecInt32Only)) {
            JSValue radix;
            if (!node->child2())
                radix = jsNumber(0);
            else
                radix = forNode(node->child2()).m_value;

            if (radix.isNumber()
                && (radix.asNumber() == 0 || radix.asNumber() == 10)) {
                m_state.setFoundConstants(true);
                forNode(node).setType(SpecInt32Only);
                break;
            }
        }

        if (node->child1().useKind() == UntypedUse)
            clobberWorld(node->origin.semantic, clobberLimit);
        forNode(node).setType(m_graph, SpecBytecodeNumber);
        break;
    }

    case CreateRest:
        if (!m_graph.isWatchingHavingABadTimeWatchpoint(node)) {
            // This means we're already having a bad time.
            clobberWorld(node->origin.semantic, clobberLimit);
            forNode(node).setType(m_graph, SpecArray);
            break;
        }
        forNode(node).set(
            m_graph,
            m_graph.globalObjectFor(node->origin.semantic)->restParameterStructure());
        break;
            
    case Check: {
        // Simplify out checks that don't actually do checking.
        for (unsigned i = 0; i < AdjacencyList::Size; ++i) {
            Edge edge = node->children.child(i);
            if (!edge)
                break;
            if (edge.isProved() || edge.willNotHaveCheck()) {
                m_state.setFoundConstants(true);
                break;
            }
        }
        break;
    }

    case SetFunctionName: {
        clobberWorld(node->origin.semantic, clobberLimit);
        break;
    }

    case StoreBarrier:
    case FencedStoreBarrier: {
        filter(node->child1(), SpecCell);
        break;
    }
        
    case CheckTierUpAndOSREnter:
    case LoopHint:
    case ZombieHint:
    case ExitOK:
        break;

    case Unreachable:
        // It may be that during a previous run of AI we proved that something was unreachable, but
        // during this run of AI we forget that it's unreachable. AI's proofs don't have to get
        // monotonically stronger over time. So, we don't assert that AI doesn't reach the
        // Unreachable. We have no choice but to take our past proof at face value. Otherwise we'll
        // crash whenever AI fails to be as powerful on run K as it was on run K-1.
        m_state.setIsValid(false);
        break;
        
    case LastNodeType:
    case ArithIMul:
    case FiatInt52:
        DFG_CRASH(m_graph, node, "Unexpected node type");
        break;
    }
    
    return m_state.isValid();
}

template<typename AbstractStateType>
bool AbstractInterpreter<AbstractStateType>::executeEffects(unsigned indexInBlock)
{
    return executeEffects(indexInBlock, m_state.block()->at(indexInBlock));
}

template<typename AbstractStateType>
bool AbstractInterpreter<AbstractStateType>::execute(unsigned indexInBlock)
{
    Node* node = m_state.block()->at(indexInBlock);

    startExecuting();
    executeEdges(node);
    return executeEffects(indexInBlock, node);
}

template<typename AbstractStateType>
bool AbstractInterpreter<AbstractStateType>::execute(Node* node)
{
    startExecuting();
    executeEdges(node);
    return executeEffects(UINT_MAX, node);
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::clobberWorld(
    const CodeOrigin&, unsigned clobberLimit)
{
    clobberStructures(clobberLimit);
}

template<typename AbstractStateType>
template<typename Functor>
void AbstractInterpreter<AbstractStateType>::forAllValues(
    unsigned clobberLimit, Functor& functor)
{
    if (clobberLimit >= m_state.block()->size())
        clobberLimit = m_state.block()->size();
    else
        clobberLimit++;
    ASSERT(clobberLimit <= m_state.block()->size());
    for (size_t i = clobberLimit; i--;) {
        NodeFlowProjection::forEach(
            m_state.block()->at(i),
            [&] (NodeFlowProjection nodeProjection) {
                functor(forNode(nodeProjection));
            });
    }
    if (m_graph.m_form == SSA) {
        for (NodeFlowProjection node : m_state.block()->ssa->liveAtHead) {
            if (node.isStillValid())
                functor(forNode(node));
        }
    }
    for (size_t i = m_state.variables().numberOfArguments(); i--;)
        functor(m_state.variables().argument(i));
    for (size_t i = m_state.variables().numberOfLocals(); i--;)
        functor(m_state.variables().local(i));
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::clobberStructures(unsigned clobberLimit)
{
    forAllValues(clobberLimit, AbstractValue::clobberStructuresFor);
    setDidClobber();
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::observeTransition(
    unsigned clobberLimit, RegisteredStructure from, RegisteredStructure to)
{
    AbstractValue::TransitionObserver transitionObserver(from, to);
    forAllValues(clobberLimit, transitionObserver);
    
    ASSERT(!from->dfgShouldWatch()); // We don't need to claim to be in a clobbered state because 'from' was never watchable (during the time we were compiling), hence no constants ever introduced into the DFG IR that ever had a watchable structure would ever have the same structure as from.
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::observeTransitions(
    unsigned clobberLimit, const TransitionVector& vector)
{
    AbstractValue::TransitionsObserver transitionsObserver(vector);
    forAllValues(clobberLimit, transitionsObserver);
    
    if (!ASSERT_DISABLED) {
        // We don't need to claim to be in a clobbered state because none of the Transition::previous structures are watchable.
        for (unsigned i = vector.size(); i--;)
            ASSERT(!vector[i].previous->dfgShouldWatch());
    }
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::setDidClobber()
{
    m_state.setDidClobber(true);
    m_state.setStructureClobberState(StructuresAreClobbered);
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::dump(PrintStream& out) const
{
    const_cast<AbstractInterpreter<AbstractStateType>*>(this)->dump(out);
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::dump(PrintStream& out)
{
    CommaPrinter comma(" ");
    HashSet<NodeFlowProjection> seen;
    if (m_graph.m_form == SSA) {
        for (NodeFlowProjection node : m_state.block()->ssa->liveAtHead) {
            seen.add(node);
            AbstractValue& value = forNode(node);
            if (value.isClear())
                continue;
            out.print(comma, node, ":", value);
        }
    }
    for (size_t i = 0; i < m_state.block()->size(); ++i) {
        NodeFlowProjection::forEach(
            m_state.block()->at(i), [&] (NodeFlowProjection nodeProjection) {
                seen.add(nodeProjection);
                AbstractValue& value = forNode(nodeProjection);
                if (value.isClear())
                    return;
                out.print(comma, nodeProjection, ":", value);
            });
    }
    if (m_graph.m_form == SSA) {
        for (NodeFlowProjection node : m_state.block()->ssa->liveAtTail) {
            if (seen.contains(node))
                continue;
            AbstractValue& value = forNode(node);
            if (value.isClear())
                continue;
            out.print(comma, node, ":", value);
        }
    }
}

template<typename AbstractStateType>
FiltrationResult AbstractInterpreter<AbstractStateType>::filter(
    AbstractValue& value, const RegisteredStructureSet& set, SpeculatedType admittedTypes)
{
    if (value.filter(m_graph, set, admittedTypes) == FiltrationOK)
        return FiltrationOK;
    m_state.setIsValid(false);
    return Contradiction;
}

template<typename AbstractStateType>
FiltrationResult AbstractInterpreter<AbstractStateType>::filterArrayModes(
    AbstractValue& value, ArrayModes arrayModes)
{
    if (value.filterArrayModes(arrayModes) == FiltrationOK)
        return FiltrationOK;
    m_state.setIsValid(false);
    return Contradiction;
}

template<typename AbstractStateType>
FiltrationResult AbstractInterpreter<AbstractStateType>::filter(
    AbstractValue& value, SpeculatedType type)
{
    if (value.filter(type) == FiltrationOK)
        return FiltrationOK;
    m_state.setIsValid(false);
    return Contradiction;
}

template<typename AbstractStateType>
FiltrationResult AbstractInterpreter<AbstractStateType>::filterByValue(
    AbstractValue& abstractValue, FrozenValue concreteValue)
{
    if (abstractValue.filterByValue(concreteValue) == FiltrationOK)
        return FiltrationOK;
    m_state.setIsValid(false);
    return Contradiction;
}

template<typename AbstractStateType>
FiltrationResult AbstractInterpreter<AbstractStateType>::filterClassInfo(
    AbstractValue& value, const ClassInfo* classInfo)
{
    if (value.filterClassInfo(m_graph, classInfo) == FiltrationOK)
        return FiltrationOK;
    m_state.setIsValid(false);
    return Contradiction;
}

template<typename AbstractStateType>
void AbstractInterpreter<AbstractStateType>::executeDoubleUnaryOpEffects(Node* node, double(*equivalentFunction)(double))
{
    JSValue child = forNode(node->child1()).value();
    if (std::optional<double> number = child.toNumberFromPrimitive()) {
        setConstant(node, jsDoubleNumber(equivalentFunction(*number)));
        return;
    }
    SpeculatedType type = SpecFullNumber;
    if (node->child1().useKind() == DoubleRepUse)
        type = typeOfDoubleUnaryOp(forNode(node->child1()).m_type);
    forNode(node).setType(type);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
