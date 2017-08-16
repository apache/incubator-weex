/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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
#include "DFGBackwardsPropagationPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGGraph.h"
#include "DFGPhase.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class BackwardsPropagationPhase : public Phase {
public:
    BackwardsPropagationPhase(Graph& graph)
        : Phase(graph, "backwards propagation")
    {
    }
    
    bool run()
    {
        m_changed = true;
        while (m_changed) {
            m_changed = false;
            for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
                BasicBlock* block = m_graph.block(blockIndex);
                if (!block)
                    continue;
            
                // Prevent a tower of overflowing additions from creating a value that is out of the
                // safe 2^48 range.
                m_allowNestedOverflowingAdditions = block->size() < (1 << 16);
            
                for (unsigned indexInBlock = block->size(); indexInBlock--;)
                    propagate(block->at(indexInBlock));
            }
        }
        
        return true;
    }

private:
    bool isNotNegZero(Node* node)
    {
        if (!node->isNumberConstant())
            return false;
        double value = node->asNumber();
        return (value || 1.0 / value > 0.0);
    }
    
    bool isNotPosZero(Node* node)
    {
        if (!node->isNumberConstant())
            return false;
        double value = node->asNumber();
        return (value || 1.0 / value < 0.0);
    }

    // Tests if the absolute value is strictly less than the power of two.
    template<int power>
    bool isWithinPowerOfTwoForConstant(Node* node)
    {
        JSValue immediateValue = node->asJSValue();
        if (!immediateValue.isNumber())
            return false;
        double immediate = immediateValue.asNumber();
        return immediate > -(static_cast<int64_t>(1) << power) && immediate < (static_cast<int64_t>(1) << power);
    }
    
    template<int power>
    bool isWithinPowerOfTwoNonRecursive(Node* node)
    {
        if (!node->isNumberConstant())
            return false;
        return isWithinPowerOfTwoForConstant<power>(node);
    }
    
    template<int power>
    bool isWithinPowerOfTwo(Node* node)
    {
        switch (node->op()) {
        case DoubleConstant:
        case JSConstant:
        case Int52Constant: {
            return isWithinPowerOfTwoForConstant<power>(node);
        }
            
        case BitAnd: {
            if (power > 31)
                return true;
            
            return isWithinPowerOfTwoNonRecursive<power>(node->child1().node())
                || isWithinPowerOfTwoNonRecursive<power>(node->child2().node());
        }
            
        case BitOr:
        case BitXor:
        case BitLShift: {
            return power > 31;
        }
            
        case BitRShift:
        case BitURShift: {
            if (power > 31)
                return true;
            
            Node* shiftAmount = node->child2().node();
            if (!node->isNumberConstant())
                return false;
            JSValue immediateValue = shiftAmount->asJSValue();
            if (!immediateValue.isInt32())
                return false;
            return immediateValue.asInt32() > 32 - power;
        }
            
        default:
            return false;
        }
    }

    template<int power>
    bool isWithinPowerOfTwo(Edge edge)
    {
        return isWithinPowerOfTwo<power>(edge.node());
    }

    bool mergeDefaultFlags(Node* node)
    {
        bool changed = false;
        if (node->flags() & NodeHasVarArgs) {
            for (unsigned childIdx = node->firstChild();
                childIdx < node->firstChild() + node->numChildren();
                childIdx++) {
                if (!!m_graph.m_varArgChildren[childIdx])
                    changed |= m_graph.m_varArgChildren[childIdx]->mergeFlags(NodeBytecodeUsesAsValue);
            }
        } else {
            if (!node->child1())
                return changed;
            changed |= node->child1()->mergeFlags(NodeBytecodeUsesAsValue);
            if (!node->child2())
                return changed;
            changed |= node->child2()->mergeFlags(NodeBytecodeUsesAsValue);
            if (!node->child3())
                return changed;
            changed |= node->child3()->mergeFlags(NodeBytecodeUsesAsValue);
        }
        return changed;
    }
    
    void propagate(Node* node)
    {
        NodeFlags flags = node->flags() & NodeBytecodeBackPropMask;
        
        switch (node->op()) {
        case GetLocal: {
            VariableAccessData* variableAccessData = node->variableAccessData();
            flags &= ~NodeBytecodeUsesAsInt; // We don't care about cross-block uses-as-int.
            m_changed |= variableAccessData->mergeFlags(flags);
            break;
        }
            
        case SetLocal: {
            VariableAccessData* variableAccessData = node->variableAccessData();
            if (!variableAccessData->isLoadedFrom())
                break;
            flags = variableAccessData->flags();
            RELEASE_ASSERT(!(flags & ~NodeBytecodeBackPropMask));
            flags |= NodeBytecodeUsesAsNumber; // Account for the fact that control flow may cause overflows that our modeling can't handle.
            node->child1()->mergeFlags(flags);
            break;
        }
            
        case Flush: {
            VariableAccessData* variableAccessData = node->variableAccessData();
            m_changed |= variableAccessData->mergeFlags(NodeBytecodeUsesAsValue);
            break;
        }
            
        case MovHint:
        case Check:
            break;
            
        case BitAnd:
        case BitOr:
        case BitXor:
        case BitRShift:
        case BitLShift:
        case BitURShift:
        case ArithIMul: {
            flags |= NodeBytecodeUsesAsInt;
            flags &= ~(NodeBytecodeUsesAsNumber | NodeBytecodeNeedsNegZero | NodeBytecodeUsesAsOther);
            flags &= ~NodeBytecodeUsesAsArrayIndex;
            node->child1()->mergeFlags(flags);
            node->child2()->mergeFlags(flags);
            break;
        }
            
        case StringCharCodeAt: {
            node->child1()->mergeFlags(NodeBytecodeUsesAsValue);
            node->child2()->mergeFlags(NodeBytecodeUsesAsValue | NodeBytecodeUsesAsInt | NodeBytecodeUsesAsArrayIndex);
            break;
        }
            
        case UInt32ToNumber: {
            node->child1()->mergeFlags(flags);
            break;
        }

        case ValueAdd: {
            if (isNotNegZero(node->child1().node()) || isNotNegZero(node->child2().node()))
                flags &= ~NodeBytecodeNeedsNegZero;
            if (node->child1()->hasNumberResult() || node->child2()->hasNumberResult())
                flags &= ~NodeBytecodeUsesAsOther;
            if (!isWithinPowerOfTwo<32>(node->child1()) && !isWithinPowerOfTwo<32>(node->child2()))
                flags |= NodeBytecodeUsesAsNumber;
            if (!m_allowNestedOverflowingAdditions)
                flags |= NodeBytecodeUsesAsNumber;
            
            node->child1()->mergeFlags(flags);
            node->child2()->mergeFlags(flags);
            break;
        }

        case ArithAdd: {
            flags &= ~NodeBytecodeUsesAsOther;
            if (isNotNegZero(node->child1().node()) || isNotNegZero(node->child2().node()))
                flags &= ~NodeBytecodeNeedsNegZero;
            if (!isWithinPowerOfTwo<32>(node->child1()) && !isWithinPowerOfTwo<32>(node->child2()))
                flags |= NodeBytecodeUsesAsNumber;
            if (!m_allowNestedOverflowingAdditions)
                flags |= NodeBytecodeUsesAsNumber;
            
            node->child1()->mergeFlags(flags);
            node->child2()->mergeFlags(flags);
            break;
        }

        case ArithClz32: {
            flags &= ~(NodeBytecodeUsesAsNumber | NodeBytecodeNeedsNegZero | NodeBytecodeUsesAsOther | ~NodeBytecodeUsesAsArrayIndex);
            flags |= NodeBytecodeUsesAsInt;
            node->child1()->mergeFlags(flags);
            break;
        }

        case ArithSub: {
            flags &= ~NodeBytecodeUsesAsOther;
            if (isNotNegZero(node->child1().node()) || isNotPosZero(node->child2().node()))
                flags &= ~NodeBytecodeNeedsNegZero;
            if (!isWithinPowerOfTwo<32>(node->child1()) && !isWithinPowerOfTwo<32>(node->child2()))
                flags |= NodeBytecodeUsesAsNumber;
            if (!m_allowNestedOverflowingAdditions)
                flags |= NodeBytecodeUsesAsNumber;
            
            node->child1()->mergeFlags(flags);
            node->child2()->mergeFlags(flags);
            break;
        }
            
        case ArithNegate: {
            flags &= ~NodeBytecodeUsesAsOther;

            node->child1()->mergeFlags(flags);
            break;
        }
            
        case ArithMul: {
            // As soon as a multiply happens, we can easily end up in the part
            // of the double domain where the point at which you do truncation
            // can change the outcome. So, ArithMul always forces its inputs to
            // check for overflow. Additionally, it will have to check for overflow
            // itself unless we can prove that there is no way for the values
            // produced to cause double rounding.
            
            if (!isWithinPowerOfTwo<22>(node->child1().node())
                && !isWithinPowerOfTwo<22>(node->child2().node()))
                flags |= NodeBytecodeUsesAsNumber;
            
            node->mergeFlags(flags);
            
            flags |= NodeBytecodeUsesAsNumber | NodeBytecodeNeedsNegZero;
            flags &= ~NodeBytecodeUsesAsOther;

            node->child1()->mergeFlags(flags);
            node->child2()->mergeFlags(flags);
            break;
        }
            
        case ArithDiv: {
            flags |= NodeBytecodeUsesAsNumber | NodeBytecodeNeedsNegZero;
            flags &= ~NodeBytecodeUsesAsOther;

            node->child1()->mergeFlags(flags);
            node->child2()->mergeFlags(flags);
            break;
        }
            
        case ArithMod: {
            flags |= NodeBytecodeUsesAsNumber;
            flags &= ~NodeBytecodeUsesAsOther;

            node->child1()->mergeFlags(flags);
            node->child2()->mergeFlags(flags & ~NodeBytecodeNeedsNegZero);
            break;
        }
            
        case GetByVal: {
            node->child1()->mergeFlags(NodeBytecodeUsesAsValue);
            node->child2()->mergeFlags(NodeBytecodeUsesAsNumber | NodeBytecodeUsesAsOther | NodeBytecodeUsesAsInt | NodeBytecodeUsesAsArrayIndex);
            break;
        }
            
        case NewArrayWithSize: {
            node->child1()->mergeFlags(NodeBytecodeUsesAsValue | NodeBytecodeUsesAsInt | NodeBytecodeUsesAsArrayIndex);
            break;
        }
            
        case NewTypedArray: {
            // Negative zero is not observable. NaN versus undefined are only observable
            // in that you would get a different exception message. So, like, whatever: we
            // claim here that NaN v. undefined is observable.
            node->child1()->mergeFlags(NodeBytecodeUsesAsInt | NodeBytecodeUsesAsNumber | NodeBytecodeUsesAsOther | NodeBytecodeUsesAsArrayIndex);
            break;
        }
            
        case StringCharAt: {
            node->child1()->mergeFlags(NodeBytecodeUsesAsValue);
            node->child2()->mergeFlags(NodeBytecodeUsesAsValue | NodeBytecodeUsesAsInt | NodeBytecodeUsesAsArrayIndex);
            break;
        }
            
        case ToString:
        case CallStringConstructor: {
            node->child1()->mergeFlags(NodeBytecodeUsesAsNumber | NodeBytecodeUsesAsOther);
            break;
        }
            
        case ToPrimitive:
        case ToNumber: {
            node->child1()->mergeFlags(flags);
            break;
        }

        case PutByValDirect:
        case PutByVal: {
            m_graph.varArgChild(node, 0)->mergeFlags(NodeBytecodeUsesAsValue);
            m_graph.varArgChild(node, 1)->mergeFlags(NodeBytecodeUsesAsNumber | NodeBytecodeUsesAsOther | NodeBytecodeUsesAsInt | NodeBytecodeUsesAsArrayIndex);
            m_graph.varArgChild(node, 2)->mergeFlags(NodeBytecodeUsesAsValue);
            break;
        }
            
        case Switch: {
            SwitchData* data = node->switchData();
            switch (data->kind) {
            case SwitchImm:
                // We don't need NodeBytecodeNeedsNegZero because if the cases are all integers
                // then -0 and 0 are treated the same.  We don't need NodeBytecodeUsesAsOther
                // because if all of the cases are integers then NaN and undefined are
                // treated the same (i.e. they will take default).
                node->child1()->mergeFlags(NodeBytecodeUsesAsNumber | NodeBytecodeUsesAsInt);
                break;
            case SwitchChar: {
                // We don't need NodeBytecodeNeedsNegZero because if the cases are all strings
                // then -0 and 0 are treated the same.  We don't need NodeBytecodeUsesAsOther
                // because if all of the cases are single-character strings then NaN
                // and undefined are treated the same (i.e. they will take default).
                node->child1()->mergeFlags(NodeBytecodeUsesAsNumber);
                break;
            }
            case SwitchString:
                // We don't need NodeBytecodeNeedsNegZero because if the cases are all strings
                // then -0 and 0 are treated the same.
                node->child1()->mergeFlags(NodeBytecodeUsesAsNumber | NodeBytecodeUsesAsOther);
                break;
            case SwitchCell:
                // There is currently no point to being clever here since this is used for switching
                // on objects.
                mergeDefaultFlags(node);
                break;
            }
            break;
        }

        case Identity: 
            // This would be trivial to handle but we just assert that we cannot see these yet.
            RELEASE_ASSERT_NOT_REACHED();
            break;
            
        // Note: ArithSqrt, ArithSin, and ArithCos and other math intrinsics don't have special
        // rules in here because they are always followed by Phantoms to signify that if the
        // method call speculation fails, the bytecode may use the arguments in arbitrary ways.
        // This corresponds to that possibility of someone doing something like:
        // Math.sin = function(x) { doArbitraryThingsTo(x); }
            
        default:
            mergeDefaultFlags(node);
            break;
        }
    }
    
    bool m_allowNestedOverflowingAdditions;
    bool m_changed;
};

bool performBackwardsPropagation(Graph& graph)
{
    return runPhase<BackwardsPropagationPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

