/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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
#include "B3ReduceStrength.h"

#if ENABLE(B3_JIT)

#include "B3AtomicValue.h"
#include "B3BasicBlockInlines.h"
#include "B3BlockInsertionSet.h"
#include "B3ComputeDivisionMagic.h"
#include "B3Dominators.h"
#include "B3InsertionSetInlines.h"
#include "B3MemoryValueInlines.h"
#include "B3PhaseScope.h"
#include "B3PhiChildren.h"
#include "B3ProcedureInlines.h"
#include "B3PureCSE.h"
#include "B3SlotBaseValue.h"
#include "B3StackSlot.h"
#include "B3UpsilonValue.h"
#include "B3ValueKeyInlines.h"
#include "B3ValueInlines.h"
#include "B3Variable.h"
#include "B3VariableValue.h"
#include <wtf/GraphNodeWorklist.h>
#include <wtf/HashMap.h>
#include <wtf/IndexSet.h>

namespace JSC { namespace B3 {

namespace {

// The goal of this phase is to:
//
// - Replace operations with less expensive variants. This includes constant folding and classic
//   strength reductions like turning Mul(x, 1 << k) into Shl(x, k).
//
// - Reassociate constant operations. For example, Load(Add(x, c)) is turned into Load(x, offset = c)
//   and Add(Add(x, c), d) is turned into Add(x, c + d).
//
// - Canonicalize operations. There are some cases where it's not at all obvious which kind of
//   operation is less expensive, but it's useful for subsequent phases - particularly LowerToAir -
//   to have only one way of representing things.
//
// This phase runs to fixpoint. Therefore, the canonicalizations must be designed to be monotonic.
// For example, if we had a canonicalization that said that Add(x, -c) should be Sub(x, c) and
// another canonicalization that said that Sub(x, d) should be Add(x, -d), then this phase would end
// up running forever. We don't want that.
//
// Therefore, we need to prioritize certain canonical forms over others. Naively, we want strength
// reduction to reduce the number of values, and so a form involving fewer total values is more
// canonical. But we might break this, for example when reducing strength of Mul(x, 9). This could be
// better written as Add(Shl(x, 3), x), which also happens to be representable using a single
// instruction on x86.
//
// Here are some of the rules we have:
//
// Canonical form of logical not: BitXor(value, 1). We may have to avoid using this form if we don't
// know for sure that 'value' is 0-or-1 (i.e. returnsBool). In that case we fall back on
// Equal(value, 0).
//
// Canonical form of commutative operations: if the operation involves a constant, the constant must
// come second. Add(x, constant) is canonical, while Add(constant, x) is not. If there are no
// constants then the canonical form involves the lower-indexed value first. Given Add(x, y), it's
// canonical if x->index() <= y->index().

bool verbose = false;

// FIXME: This IntRange stuff should be refactored into a general constant propagator. It's weird
// that it's just sitting here in this file.
class IntRange {
public:
    IntRange()
    {
    }

    IntRange(int64_t min, int64_t max)
        : m_min(min)
        , m_max(max)
    {
    }

    template<typename T>
    static IntRange top()
    {
        return IntRange(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }

    static IntRange top(Type type)
    {
        switch (type) {
        case Int32:
            return top<int32_t>();
        case Int64:
            return top<int64_t>();
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return IntRange();
        }
    }

    template<typename T>
    static IntRange rangeForMask(T mask)
    {
        if (!(mask + 1))
            return top<T>();
        return IntRange(0, mask);
    }

    static IntRange rangeForMask(int64_t mask, Type type)
    {
        switch (type) {
        case Int32:
            return rangeForMask<int32_t>(static_cast<int32_t>(mask));
        case Int64:
            return rangeForMask<int64_t>(mask);
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return IntRange();
        }
    }

    template<typename T>
    static IntRange rangeForZShr(int32_t shiftAmount)
    {
        typename std::make_unsigned<T>::type mask = 0;
        mask--;
        mask >>= shiftAmount;
        return rangeForMask<T>(static_cast<T>(mask));
    }

    static IntRange rangeForZShr(int32_t shiftAmount, Type type)
    {
        switch (type) {
        case Int32:
            return rangeForZShr<int32_t>(shiftAmount);
        case Int64:
            return rangeForZShr<int64_t>(shiftAmount);
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return IntRange();
        }
    }

    int64_t min() const { return m_min; }
    int64_t max() const { return m_max; }

    void dump(PrintStream& out) const
    {
        out.print("[", m_min, ",", m_max, "]");
    }

    template<typename T>
    bool couldOverflowAdd(const IntRange& other)
    {
        return sumOverflows<T>(m_min, other.m_min)
            || sumOverflows<T>(m_min, other.m_max)
            || sumOverflows<T>(m_max, other.m_min)
            || sumOverflows<T>(m_max, other.m_max);
    }

    bool couldOverflowAdd(const IntRange& other, Type type)
    {
        switch (type) {
        case Int32:
            return couldOverflowAdd<int32_t>(other);
        case Int64:
            return couldOverflowAdd<int64_t>(other);
        default:
            return true;
        }
    }

    template<typename T>
    bool couldOverflowSub(const IntRange& other)
    {
        return differenceOverflows<T>(m_min, other.m_min)
            || differenceOverflows<T>(m_min, other.m_max)
            || differenceOverflows<T>(m_max, other.m_min)
            || differenceOverflows<T>(m_max, other.m_max);
    }

    bool couldOverflowSub(const IntRange& other, Type type)
    {
        switch (type) {
        case Int32:
            return couldOverflowSub<int32_t>(other);
        case Int64:
            return couldOverflowSub<int64_t>(other);
        default:
            return true;
        }
    }

    template<typename T>
    bool couldOverflowMul(const IntRange& other)
    {
        return productOverflows<T>(m_min, other.m_min)
            || productOverflows<T>(m_min, other.m_max)
            || productOverflows<T>(m_max, other.m_min)
            || productOverflows<T>(m_max, other.m_max);
    }

    bool couldOverflowMul(const IntRange& other, Type type)
    {
        switch (type) {
        case Int32:
            return couldOverflowMul<int32_t>(other);
        case Int64:
            return couldOverflowMul<int64_t>(other);
        default:
            return true;
        }
    }

    template<typename T>
    IntRange shl(int32_t shiftAmount)
    {
        T newMin = static_cast<T>(m_min) << static_cast<T>(shiftAmount);
        T newMax = static_cast<T>(m_max) << static_cast<T>(shiftAmount);

        if ((newMin >> shiftAmount) != static_cast<T>(m_min))
            newMin = std::numeric_limits<T>::min();
        if ((newMax >> shiftAmount) != static_cast<T>(m_max))
            newMax = std::numeric_limits<T>::max();

        return IntRange(newMin, newMax);
    }

    IntRange shl(int32_t shiftAmount, Type type)
    {
        switch (type) {
        case Int32:
            return shl<int32_t>(shiftAmount);
        case Int64:
            return shl<int64_t>(shiftAmount);
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return IntRange();
        }
    }

    template<typename T>
    IntRange sShr(int32_t shiftAmount)
    {
        T newMin = static_cast<T>(m_min) >> static_cast<T>(shiftAmount);
        T newMax = static_cast<T>(m_max) >> static_cast<T>(shiftAmount);

        return IntRange(newMin, newMax);
    }

    IntRange sShr(int32_t shiftAmount, Type type)
    {
        switch (type) {
        case Int32:
            return sShr<int32_t>(shiftAmount);
        case Int64:
            return sShr<int64_t>(shiftAmount);
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return IntRange();
        }
    }

    template<typename T>
    IntRange zShr(int32_t shiftAmount)
    {
        // This is an awkward corner case for all of the other logic.
        if (!shiftAmount)
            return *this;

        // If the input range may be negative, then all we can say about the output range is that it
        // will be masked. That's because -1 right shifted just produces that mask.
        if (m_min < 0)
            return rangeForZShr<T>(shiftAmount);

        // If the input range is non-negative, then this just brings the range closer to zero.
        typedef typename std::make_unsigned<T>::type UnsignedT;
        UnsignedT newMin = static_cast<UnsignedT>(m_min) >> static_cast<UnsignedT>(shiftAmount);
        UnsignedT newMax = static_cast<UnsignedT>(m_max) >> static_cast<UnsignedT>(shiftAmount);
        
        return IntRange(newMin, newMax);
    }

    IntRange zShr(int32_t shiftAmount, Type type)
    {
        switch (type) {
        case Int32:
            return zShr<int32_t>(shiftAmount);
        case Int64:
            return zShr<int64_t>(shiftAmount);
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return IntRange();
        }
    }

    template<typename T>
    IntRange add(const IntRange& other)
    {
        if (couldOverflowAdd<T>(other))
            return top<T>();
        return IntRange(m_min + other.m_min, m_max + other.m_max);
    }

    IntRange add(const IntRange& other, Type type)
    {
        switch (type) {
        case Int32:
            return add<int32_t>(other);
        case Int64:
            return add<int64_t>(other);
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return IntRange();
        }
    }

    template<typename T>
    IntRange sub(const IntRange& other)
    {
        if (couldOverflowSub<T>(other))
            return top<T>();
        return IntRange(m_min - other.m_max, m_max - other.m_min);
    }

    IntRange sub(const IntRange& other, Type type)
    {
        switch (type) {
        case Int32:
            return sub<int32_t>(other);
        case Int64:
            return sub<int64_t>(other);
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return IntRange();
        }
    }

    template<typename T>
    IntRange mul(const IntRange& other)
    {
        if (couldOverflowMul<T>(other))
            return top<T>();
        return IntRange(
            std::min(
                std::min(m_min * other.m_min, m_min * other.m_max),
                std::min(m_max * other.m_min, m_max * other.m_max)),
            std::max(
                std::max(m_min * other.m_min, m_min * other.m_max),
                std::max(m_max * other.m_min, m_max * other.m_max)));
    }

    IntRange mul(const IntRange& other, Type type)
    {
        switch (type) {
        case Int32:
            return mul<int32_t>(other);
        case Int64:
            return mul<int64_t>(other);
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return IntRange();
        }
    }

private:
    int64_t m_min { 0 };
    int64_t m_max { 0 };
};

class ReduceStrength {
public:
    ReduceStrength(Procedure& proc)
        : m_proc(proc)
        , m_insertionSet(proc)
        , m_blockInsertionSet(proc)
    {
    }

    bool run()
    {
        bool result = false;
        bool first = true;
        unsigned index = 0;
        do {
            m_changed = false;
            m_changedCFG = false;
            ++index;

            if (first)
                first = false;
            else if (verbose) {
                dataLog("B3 after iteration #", index - 1, " of reduceStrength:\n");
                dataLog(m_proc);
            }
            
            simplifyCFG();

            if (m_changedCFG) {
                m_proc.resetReachability();
                m_proc.invalidateCFG();
                m_changed = true;
            }

            // We definitely want to do DCE before we do CSE so that we don't hoist things. For
            // example:
            //
            // @dead = Mul(@a, @b)
            // ... lots of control flow and stuff
            // @thing = Mul(@a, @b)
            //
            // If we do CSE before DCE, we will remove @thing and keep @dead. Effectively, we will
            // "hoist" @thing. On the other hand, if we run DCE before CSE, we will kill @dead and
            // keep @thing. That's better, since we usually want things to stay wherever the client
            // put them. We're not actually smart enough to move things around at random.
            killDeadCode();
            
            simplifySSA();
            
            m_proc.resetValueOwners();
            m_dominators = &m_proc.dominators(); // Recompute if necessary.
            m_pureCSE.clear();

            for (BasicBlock* block : m_proc.blocksInPreOrder()) {
                m_block = block;
                
                for (m_index = 0; m_index < block->size(); ++m_index) {
                    if (verbose) {
                        dataLog(
                            "Looking at ", *block, " #", m_index, ": ",
                            deepDump(m_proc, block->at(m_index)), "\n");
                    }
                    m_value = m_block->at(m_index);
                    m_value->performSubstitution();
                    
                    reduceValueStrength();
                    replaceIfRedundant();
                }
                m_insertionSet.execute(m_block);
            }

            m_changedCFG |= m_blockInsertionSet.execute();
            if (m_changedCFG) {
                m_proc.resetReachability();
                m_proc.invalidateCFG();
                m_dominators = nullptr; // Dominators are not valid anymore, and we don't need them yet.
                m_changed = true;
            }
            
            result |= m_changed;
        } while (m_changed);
        return result;
    }
    
private:
    void reduceValueStrength()
    {
        switch (m_value->opcode()) {
        case Add:
            handleCommutativity();
            
            if (m_value->child(0)->opcode() == Add && isInt(m_value->type())) {
                // Turn this: Add(Add(value, constant1), constant2)
                // Into this: Add(value, constant1 + constant2)
                Value* newSum = m_value->child(1)->addConstant(m_proc, m_value->child(0)->child(1));
                if (newSum) {
                    m_insertionSet.insertValue(m_index, newSum);
                    m_value->child(0) = m_value->child(0)->child(0);
                    m_value->child(1) = newSum;
                    m_changed = true;
                    break;
                }
                
                // Turn this: Add(Add(value, constant), otherValue)
                // Into this: Add(Add(value, otherValue), constant)
                if (!m_value->child(1)->hasInt() && m_value->child(0)->child(1)->hasInt()) {
                    Value* value = m_value->child(0)->child(0);
                    Value* constant = m_value->child(0)->child(1);
                    Value* otherValue = m_value->child(1);
                    // This could create duplicate code if Add(value, constant) is used elsewhere.
                    // However, we already model adding a constant as if it was free in other places
                    // so let's just roll with it. The alternative would mean having to do good use
                    // counts, which reduceStrength() currently doesn't have.
                    m_value->child(0) =
                        m_insertionSet.insert<Value>(
                            m_index, Add, m_value->origin(), value, otherValue);
                    m_value->child(1) = constant;
                    m_changed = true;
                    break;
                }
            }
            
            // Turn this: Add(otherValue, Add(value, constant))
            // Into this: Add(Add(value, otherValue), constant)
            if (isInt(m_value->type())
                && !m_value->child(0)->hasInt()
                && m_value->child(1)->opcode() == Add
                && m_value->child(1)->child(1)->hasInt()) {
                Value* value = m_value->child(1)->child(0);
                Value* constant = m_value->child(1)->child(1);
                Value* otherValue = m_value->child(0);
                // This creates a duplicate add. That's dangerous but probably fine, see above.
                m_value->child(0) =
                    m_insertionSet.insert<Value>(
                        m_index, Add, m_value->origin(), value, otherValue);
                m_value->child(1) = constant;
                m_changed = true;
                break;
            }
            
            // Turn this: Add(constant1, constant2)
            // Into this: constant1 + constant2
            if (Value* constantAdd = m_value->child(0)->addConstant(m_proc, m_value->child(1))) {
                replaceWithNewValue(constantAdd);
                break;
            }

            // Turn this: Integer Add(value, value)
            // Into this: Shl(value, 1)
            // This is a useful canonicalization. It's not meant to be a strength reduction.
            if (m_value->isInteger() && m_value->child(0) == m_value->child(1)) {
                replaceWithNewValue(
                    m_proc.add<Value>(
                        Shl, m_value->origin(), m_value->child(0),
                        m_insertionSet.insert<Const32Value>(m_index, m_value->origin(), 1)));
                break;
            }

            // Turn this: Add(value, zero)
            // Into an Identity.
            //
            // Addition is subtle with doubles. Zero is not the neutral value, negative zero is:
            //    0 + 0 = 0
            //    0 + -0 = 0
            //    -0 + 0 = 0
            //    -0 + -0 = -0
            if (m_value->child(1)->isInt(0) || m_value->child(1)->isNegativeZero()) {
                replaceWithIdentity(m_value->child(0));
                break;
            }

            // Turn this: Integer Add(Sub(0, value), -1)
            // Into this: BitXor(value, -1)
            if (m_value->isInteger()
                && m_value->child(0)->opcode() == Sub
                && m_value->child(1)->isInt(-1)
                && m_value->child(0)->child(0)->isInt(0)) {
                replaceWithNewValue(m_proc.add<Value>(BitXor, m_value->origin(), m_value->child(0)->child(1), m_value->child(1)));
                break;
            }

            break;

        case Sub:
            // Turn this: Sub(constant1, constant2)
            // Into this: constant1 - constant2
            if (Value* constantSub = m_value->child(0)->subConstant(m_proc, m_value->child(1))) {
                replaceWithNewValue(constantSub);
                break;
            }

            if (isInt(m_value->type())) {
                // Turn this: Sub(value, constant)
                // Into this: Add(value, -constant)
                if (Value* negatedConstant = m_value->child(1)->negConstant(m_proc)) {
                    m_insertionSet.insertValue(m_index, negatedConstant);
                    replaceWithNew<Value>(
                        Add, m_value->origin(), m_value->child(0), negatedConstant);
                    break;
                }
                
                // Turn this: Sub(0, value)
                // Into this: Neg(value)
                if (m_value->child(0)->isInt(0)) {
                    replaceWithNew<Value>(Neg, m_value->origin(), m_value->child(1));
                    break;
                }
            }

            break;
            
        case Neg:
            // Turn this: Neg(constant)
            // Into this: -constant
            if (Value* constant = m_value->child(0)->negConstant(m_proc)) {
                replaceWithNewValue(constant);
                break;
            }
            
            // Turn this: Neg(Neg(value))
            // Into this: value
            if (m_value->child(0)->opcode() == Neg) {
                replaceWithIdentity(m_value->child(0)->child(0));
                break;
            }
            
            break;

        case Mul:
            handleCommutativity();

            // Turn this: Mul(constant1, constant2)
            // Into this: constant1 * constant2
            if (Value* value = m_value->child(0)->mulConstant(m_proc, m_value->child(1))) {
                replaceWithNewValue(value);
                break;
            }

            if (m_value->child(1)->hasInt()) {
                int64_t factor = m_value->child(1)->asInt();

                // Turn this: Mul(value, 0)
                // Into this: 0
                // Note that we don't do this for doubles because that's wrong. For example, -1 * 0
                // and 1 * 0 yield different results.
                if (!factor) {
                    replaceWithIdentity(m_value->child(1));
                    break;
                }

                // Turn this: Mul(value, 1)
                // Into this: value
                if (factor == 1) {
                    replaceWithIdentity(m_value->child(0));
                    break;
                }

                // Turn this: Mul(value, -1)
                // Into this: Sub(0, value)
                if (factor == -1) {
                    replaceWithNewValue(
                        m_proc.add<Value>(
                            Sub, m_value->origin(),
                            m_insertionSet.insertIntConstant(m_index, m_value, 0),
                            m_value->child(0)));
                    break;
                }
                
                // Turn this: Mul(value, constant)
                // Into this: Shl(value, log2(constant))
                if (hasOneBitSet(factor)) {
                    unsigned shiftAmount = WTF::fastLog2(static_cast<uint64_t>(factor));
                    replaceWithNewValue(
                        m_proc.add<Value>(
                            Shl, m_value->origin(), m_value->child(0),
                            m_insertionSet.insert<Const32Value>(
                                m_index, m_value->origin(), shiftAmount)));
                    break;
                }
            } else if (m_value->child(1)->hasDouble()) {
                double factor = m_value->child(1)->asDouble();

                // Turn this: Mul(value, 1)
                // Into this: value
                if (factor == 1) {
                    replaceWithIdentity(m_value->child(0));
                    break;
                }
            }

            break;

        case Div:
            // Turn this: Div(constant1, constant2)
            // Into this: constant1 / constant2
            // Note that this uses Div<Chill> semantics. That's fine, because the rules for Div
            // are strictly weaker: it has corner cases where it's allowed to do anything it
            // likes.
            if (replaceWithNewValue(m_value->child(0)->divConstant(m_proc, m_value->child(1))))
                break;

            if (m_value->child(1)->hasInt()) {
                switch (m_value->child(1)->asInt()) {
                case -1:
                    // Turn this: Div(value, -1)
                    // Into this: Neg(value)
                    replaceWithNewValue(
                        m_proc.add<Value>(Neg, m_value->origin(), m_value->child(0)));
                    break;

                case 0:
                    // Turn this: Div(value, 0)
                    // Into this: 0
                    // We can do this because it's precisely correct for ChillDiv and for Div we
                    // are allowed to do whatever we want.
                    replaceWithIdentity(m_value->child(1));
                    break;

                case 1:
                    // Turn this: Div(value, 1)
                    // Into this: value
                    replaceWithIdentity(m_value->child(0));
                    break;

                default:
                    // Perform super comprehensive strength reduction of division. Currently we
                    // only do this for 32-bit divisions, since we need a high multiply
                    // operation. We emulate it using 64-bit multiply. We can't emulate 64-bit
                    // high multiply with a 128-bit multiply because we don't have a 128-bit
                    // multiply. We could do it with a patchpoint if we cared badly enough.

                    if (m_value->type() != Int32)
                        break;

                    int32_t divisor = m_value->child(1)->asInt32();
                    DivisionMagic<int32_t> magic = computeDivisionMagic(divisor);

                    // Perform the "high" multiplication. We do it just to get the high bits.
                    // This is sort of like multiplying by the reciprocal, just more gnarly. It's
                    // from Hacker's Delight and I don't claim to understand it.
                    Value* magicQuotient = m_insertionSet.insert<Value>(
                        m_index, Trunc, m_value->origin(),
                        m_insertionSet.insert<Value>(
                            m_index, ZShr, m_value->origin(),
                            m_insertionSet.insert<Value>(
                                m_index, Mul, m_value->origin(),
                                m_insertionSet.insert<Value>(
                                    m_index, SExt32, m_value->origin(), m_value->child(0)),
                                m_insertionSet.insert<Const64Value>(
                                    m_index, m_value->origin(), magic.magicMultiplier)),
                            m_insertionSet.insert<Const32Value>(
                                m_index, m_value->origin(), 32)));

                    if (divisor > 0 && magic.magicMultiplier < 0) {
                        magicQuotient = m_insertionSet.insert<Value>(
                            m_index, Add, m_value->origin(), magicQuotient, m_value->child(0));
                    }
                    if (divisor < 0 && magic.magicMultiplier > 0) {
                        magicQuotient = m_insertionSet.insert<Value>(
                            m_index, Sub, m_value->origin(), magicQuotient, m_value->child(0));
                    }
                    if (magic.shift > 0) {
                        magicQuotient = m_insertionSet.insert<Value>(
                            m_index, SShr, m_value->origin(), magicQuotient,
                            m_insertionSet.insert<Const32Value>(
                                m_index, m_value->origin(), magic.shift));
                    }
                    replaceWithIdentity(
                        m_insertionSet.insert<Value>(
                            m_index, Add, m_value->origin(), magicQuotient,
                            m_insertionSet.insert<Value>(
                                m_index, ZShr, m_value->origin(), magicQuotient,
                                m_insertionSet.insert<Const32Value>(
                                    m_index, m_value->origin(), 31))));
                    break;
                }
                break;
            }
            break;

        case UDiv:
            // Turn this: UDiv(constant1, constant2)
            // Into this: constant1 / constant2
            if (replaceWithNewValue(m_value->child(0)->uDivConstant(m_proc, m_value->child(1))))
                break;

            if (m_value->child(1)->hasInt()) {
                switch (m_value->child(1)->asInt()) {
                case 0:
                    // Turn this: UDiv(value, 0)
                    // Into this: 0
                    // We can do whatever we want here so we might as well do the chill thing,
                    // in case we add chill versions of UDiv in the future.
                    replaceWithIdentity(m_value->child(1));
                    break;

                case 1:
                    // Turn this: UDiv(value, 1)
                    // Into this: value
                    replaceWithIdentity(m_value->child(0));
                    break;
                default:
                    // FIXME: We should do comprehensive strength reduction for unsigned numbers. Likely,
                    // we will just want copy what llvm does. https://bugs.webkit.org/show_bug.cgi?id=164809
                    break;
                }
            }
            break;

        case Mod:
            // Turn this: Mod(constant1, constant2)
            // Into this: constant1 / constant2
            // Note that this uses Mod<Chill> semantics.
            if (replaceWithNewValue(m_value->child(0)->modConstant(m_proc, m_value->child(1))))
                break;

            // Modulo by constant is more efficient if we turn it into Div, and then let Div get
            // optimized.
            if (m_value->child(1)->hasInt()) {
                switch (m_value->child(1)->asInt()) {
                case 0:
                    // Turn this: Mod(value, 0)
                    // Into this: 0
                    // This is correct according to ChillMod semantics.
                    replaceWithIdentity(m_value->child(1));
                    break;

                default:
                    // Turn this: Mod(N, D)
                    // Into this: Sub(N, Mul(Div(N, D), D))
                    //
                    // This is a speed-up because we use our existing Div optimizations.
                    //
                    // Here's an easier way to look at it:
                    //     N % D = N - N / D * D
                    //
                    // Note that this does not work for D = 0 and ChillMod. The expected result is 0.
                    // That's why we have a special-case above.
                    //     X % 0 = X - X / 0 * 0 = X     (should be 0)
                    //
                    // This does work for the D = -1 special case.
                    //     -2^31 % -1 = -2^31 - -2^31 / -1 * -1
                    //                = -2^31 - -2^31 * -1
                    //                = -2^31 - -2^31
                    //                = 0

                    Kind divKind = Div;
                    divKind.setIsChill(m_value->isChill());

                    replaceWithIdentity(
                        m_insertionSet.insert<Value>(
                            m_index, Sub, m_value->origin(),
                            m_value->child(0),
                            m_insertionSet.insert<Value>(
                                m_index, Mul, m_value->origin(),
                                m_insertionSet.insert<Value>(
                                    m_index, divKind, m_value->origin(),
                                    m_value->child(0), m_value->child(1)),
                                m_value->child(1))));
                    break;
                }
                break;
            }
            
            break;

        case UMod:
            // Turn this: UMod(constant1, constant2)
            // Into this: constant1 / constant2
            replaceWithNewValue(m_value->child(0)->uModConstant(m_proc, m_value->child(1)));
            // FIXME: We should do what we do for Mod since the same principle applies here.
            // https://bugs.webkit.org/show_bug.cgi?id=164809
            break;

        case BitAnd:
            handleCommutativity();

            // Turn this: BitAnd(constant1, constant2)
            // Into this: constant1 & constant2
            if (Value* constantBitAnd = m_value->child(0)->bitAndConstant(m_proc, m_value->child(1))) {
                replaceWithNewValue(constantBitAnd);
                break;
            }

            // Turn this: BitAnd(BitAnd(value, constant1), constant2)
            // Into this: BitAnd(value, constant1 & constant2).
            if (m_value->child(0)->opcode() == BitAnd) {
                Value* newConstant = m_value->child(1)->bitAndConstant(m_proc, m_value->child(0)->child(1));
                if (newConstant) {
                    m_insertionSet.insertValue(m_index, newConstant);
                    m_value->child(0) = m_value->child(0)->child(0);
                    m_value->child(1) = newConstant;
                    m_changed = true;
                }
            }

            // Turn this: BitAnd(valueX, valueX)
            // Into this: valueX.
            if (m_value->child(0) == m_value->child(1)) {
                replaceWithIdentity(m_value->child(0));
                break;
            }

            // Turn this: BitAnd(value, zero-constant)
            // Into this: zero-constant.
            if (m_value->child(1)->isInt(0)) {
                replaceWithIdentity(m_value->child(1));
                break;
            }

            // Turn this: BitAnd(value, all-ones)
            // Into this: value.
            if ((m_value->type() == Int64 && m_value->child(1)->isInt(0xffffffffffffffff))
                || (m_value->type() == Int32 && m_value->child(1)->isInt(0xffffffff))) {
                replaceWithIdentity(m_value->child(0));
                break;
            }

            // Turn this: BitAnd(64-bit value, 32 ones)
            // Into this: ZExt32(Trunc(64-bit value))
            if (m_value->child(1)->isInt64(0xffffffffllu)) {
                Value* newValue = m_insertionSet.insert<Value>(
                    m_index, ZExt32, m_value->origin(),
                    m_insertionSet.insert<Value>(m_index, Trunc, m_value->origin(), m_value->child(0)));
                replaceWithIdentity(newValue);
                break;
            }

            // Turn this: BitAnd(SExt8(value), mask) where (mask & 0xffffff00) == 0
            // Into this: BitAnd(value, mask)
            if (m_value->child(0)->opcode() == SExt8 && m_value->child(1)->hasInt32()
                && !(m_value->child(1)->asInt32() & 0xffffff00)) {
                m_value->child(0) = m_value->child(0)->child(0);
                m_changed = true;
            }

            // Turn this: BitAnd(SExt16(value), mask) where (mask & 0xffff0000) == 0
            // Into this: BitAnd(value, mask)
            if (m_value->child(0)->opcode() == SExt16 && m_value->child(1)->hasInt32()
                && !(m_value->child(1)->asInt32() & 0xffff0000)) {
                m_value->child(0) = m_value->child(0)->child(0);
                m_changed = true;
            }

            // Turn this: BitAnd(SExt32(value), mask) where (mask & 0xffffffff00000000) == 0
            // Into this: BitAnd(ZExt32(value), mask)
            if (m_value->child(0)->opcode() == SExt32 && m_value->child(1)->hasInt32()
                && !(m_value->child(1)->asInt32() & 0xffffffff00000000llu)) {
                m_value->child(0) = m_insertionSet.insert<Value>(
                    m_index, ZExt32, m_value->origin(),
                    m_value->child(0)->child(0), m_value->child(0)->child(1));
                m_changed = true;
            }

            // Turn this: BitAnd(Op(value, constant1), constant2)
            //     where !(constant1 & constant2)
            //       and Op is BitOr or BitXor
            // into this: BitAnd(value, constant2)
            if (m_value->child(1)->hasInt()) {
                int64_t constant2 = m_value->child(1)->asInt();
                switch (m_value->child(0)->opcode()) {
                case BitOr:
                case BitXor:
                    if (m_value->child(0)->child(1)->hasInt()
                        && !(m_value->child(0)->child(1)->asInt() & constant2)) {
                        m_value->child(0) = m_value->child(0)->child(0);
                        m_changed = true;
                        break;
                    }
                    break;
                default:
                    break;
                }
            }
            break;

        case BitOr:
            handleCommutativity();

            // Turn this: BitOr(constant1, constant2)
            // Into this: constant1 | constant2
            if (Value* constantBitOr = m_value->child(0)->bitOrConstant(m_proc, m_value->child(1))) {
                replaceWithNewValue(constantBitOr);
                break;
            }

            // Turn this: BitOr(BitOr(value, constant1), constant2)
            // Into this: BitOr(value, constant1 & constant2).
            if (m_value->child(0)->opcode() == BitOr) {
                Value* newConstant = m_value->child(1)->bitOrConstant(m_proc, m_value->child(0)->child(1));
                if (newConstant) {
                    m_insertionSet.insertValue(m_index, newConstant);
                    m_value->child(0) = m_value->child(0)->child(0);
                    m_value->child(1) = newConstant;
                    m_changed = true;
                }
            }

            // Turn this: BitOr(valueX, valueX)
            // Into this: valueX.
            if (m_value->child(0) == m_value->child(1)) {
                replaceWithIdentity(m_value->child(0));
                break;
            }

            // Turn this: BitOr(value, zero-constant)
            // Into this: value.
            if (m_value->child(1)->isInt(0)) {
                replaceWithIdentity(m_value->child(0));
                break;
            }

            // Turn this: BitOr(value, all-ones)
            // Into this: all-ones.
            if ((m_value->type() == Int64 && m_value->child(1)->isInt(0xffffffffffffffff))
                || (m_value->type() == Int32 && m_value->child(1)->isInt(0xffffffff))) {
                replaceWithIdentity(m_value->child(1));
                break;
            }

            break;

        case BitXor:
            handleCommutativity();

            // Turn this: BitXor(constant1, constant2)
            // Into this: constant1 ^ constant2
            if (Value* constantBitXor = m_value->child(0)->bitXorConstant(m_proc, m_value->child(1))) {
                replaceWithNewValue(constantBitXor);
                break;
            }

            // Turn this: BitXor(BitXor(value, constant1), constant2)
            // Into this: BitXor(value, constant1 ^ constant2).
            if (m_value->child(0)->opcode() == BitXor) {
                Value* newConstant = m_value->child(1)->bitXorConstant(m_proc, m_value->child(0)->child(1));
                if (newConstant) {
                    m_insertionSet.insertValue(m_index, newConstant);
                    m_value->child(0) = m_value->child(0)->child(0);
                    m_value->child(1) = newConstant;
                    m_changed = true;
                }
            }

            // Turn this: BitXor(compare, 1)
            // Into this: invertedCompare
            if (m_value->child(1)->isInt32(1)) {
                if (Value* invertedCompare = m_value->child(0)->invertedCompare(m_proc)) {
                    replaceWithNewValue(invertedCompare);
                    break;
                }
            }

            // Turn this: BitXor(valueX, valueX)
            // Into this: zero-constant.
            if (m_value->child(0) == m_value->child(1)) {
                replaceWithNewValue(m_proc.addIntConstant(m_value, 0));
                break;
            }

            // Turn this: BitXor(value, zero-constant)
            // Into this: value.
            if (m_value->child(1)->isInt(0)) {
                replaceWithIdentity(m_value->child(0));
                break;
            }

            break;

        case Shl:
            // Turn this: Shl(constant1, constant2)
            // Into this: constant1 << constant2
            if (Value* constant = m_value->child(0)->shlConstant(m_proc, m_value->child(1))) {
                replaceWithNewValue(constant);
                break;
            }

            handleShiftAmount();
            break;

        case SShr:
            // Turn this: SShr(constant1, constant2)
            // Into this: constant1 >> constant2
            if (Value* constant = m_value->child(0)->sShrConstant(m_proc, m_value->child(1))) {
                replaceWithNewValue(constant);
                break;
            }

            if (m_value->child(1)->hasInt32()
                && m_value->child(0)->opcode() == Shl
                && m_value->child(0)->child(1)->hasInt32()
                && m_value->child(1)->asInt32() == m_value->child(0)->child(1)->asInt32()) {
                switch (m_value->child(1)->asInt32()) {
                case 16:
                    if (m_value->type() == Int32) {
                        // Turn this: SShr(Shl(value, 16), 16)
                        // Into this: SExt16(value)
                        replaceWithNewValue(
                            m_proc.add<Value>(
                                SExt16, m_value->origin(), m_value->child(0)->child(0)));
                    }
                    break;

                case 24:
                    if (m_value->type() == Int32) {
                        // Turn this: SShr(Shl(value, 24), 24)
                        // Into this: SExt8(value)
                        replaceWithNewValue(
                            m_proc.add<Value>(
                                SExt8, m_value->origin(), m_value->child(0)->child(0)));
                    }
                    break;

                case 32:
                    if (m_value->type() == Int64) {
                        // Turn this: SShr(Shl(value, 32), 32)
                        // Into this: SExt32(Trunc(value))
                        replaceWithNewValue(
                            m_proc.add<Value>(
                                SExt32, m_value->origin(),
                                m_insertionSet.insert<Value>(
                                    m_index, Trunc, m_value->origin(),
                                    m_value->child(0)->child(0))));
                    }
                    break;

                // FIXME: Add cases for 48 and 56, but that would translate to SExt32(SExt8) or
                // SExt32(SExt16), which we don't currently lower efficiently.

                default:
                    break;
                }

                if (m_value->opcode() != SShr)
                    break;
            }

            handleShiftAmount();
            break;

        case ZShr:
            // Turn this: ZShr(constant1, constant2)
            // Into this: (unsigned)constant1 >> constant2
            if (Value* constant = m_value->child(0)->zShrConstant(m_proc, m_value->child(1))) {
                replaceWithNewValue(constant);
                break;
            }

            handleShiftAmount();
            break;

        case RotR:
            // Turn this: RotR(constant1, constant2)
            // Into this: (constant1 >> constant2) | (constant1 << sizeof(constant1) * 8 - constant2)
            if (Value* constant = m_value->child(0)->rotRConstant(m_proc, m_value->child(1))) {
                replaceWithNewValue(constant);
                break;
            }

            handleShiftAmount();
            break;

        case RotL:
            // Turn this: RotL(constant1, constant2)
            // Into this: (constant1 << constant2) | (constant1 >> sizeof(constant1) * 8 - constant2)
            if (Value* constant = m_value->child(0)->rotLConstant(m_proc, m_value->child(1))) {
                replaceWithNewValue(constant);
                break;
            }

            handleShiftAmount();
            break;

        case Abs:
            // Turn this: Abs(constant)
            // Into this: fabs<value->type()>(constant)
            if (Value* constant = m_value->child(0)->absConstant(m_proc)) {
                replaceWithNewValue(constant);
                break;
            }

            // Turn this: Abs(Abs(value))
            // Into this: Abs(value)
            if (m_value->child(0)->opcode() == Abs) {
                replaceWithIdentity(m_value->child(0));
                break;
            }

            // Turn this: Abs(BitwiseCast(value))
            // Into this: BitwiseCast(And(value, mask-top-bit))
            if (m_value->child(0)->opcode() == BitwiseCast) {
                Value* mask;
                if (m_value->type() == Double)
                    mask = m_insertionSet.insert<Const64Value>(m_index, m_value->origin(), ~(1ll << 63));
                else
                    mask = m_insertionSet.insert<Const32Value>(m_index, m_value->origin(), ~(1l << 31));

                Value* bitAnd = m_insertionSet.insert<Value>(m_index, BitAnd, m_value->origin(),
                    m_value->child(0)->child(0),
                    mask);
                Value* cast = m_insertionSet.insert<Value>(m_index, BitwiseCast, m_value->origin(), bitAnd);
                replaceWithIdentity(cast);
                break;
            }
            break;

        case Ceil:
            // Turn this: Ceil(constant)
            // Into this: ceil<value->type()>(constant)
            if (Value* constant = m_value->child(0)->ceilConstant(m_proc)) {
                replaceWithNewValue(constant);
                break;
            }

            // Turn this: Ceil(roundedValue)
            // Into this: roundedValue
            if (m_value->child(0)->isRounded()) {
                replaceWithIdentity(m_value->child(0));
                break;
            }
            break;

        case Floor:
            // Turn this: Floor(constant)
            // Into this: floor<value->type()>(constant)
            if (Value* constant = m_value->child(0)->floorConstant(m_proc)) {
                replaceWithNewValue(constant);
                break;
            }

            // Turn this: Floor(roundedValue)
            // Into this: roundedValue
            if (m_value->child(0)->isRounded()) {
                replaceWithIdentity(m_value->child(0));
                break;
            }
            break;

        case Sqrt:
            // Turn this: Sqrt(constant)
            // Into this: sqrt<value->type()>(constant)
            if (Value* constant = m_value->child(0)->sqrtConstant(m_proc)) {
                replaceWithNewValue(constant);
                break;
            }
            break;

        case BitwiseCast:
            // Turn this: BitwiseCast(constant)
            // Into this: bitwise_cast<value->type()>(constant)
            if (Value* constant = m_value->child(0)->bitwiseCastConstant(m_proc)) {
                replaceWithNewValue(constant);
                break;
            }

            // Turn this: BitwiseCast(BitwiseCast(value))
            // Into this: value
            if (m_value->child(0)->opcode() == BitwiseCast) {
                replaceWithIdentity(m_value->child(0)->child(0));
                break;
            }
            break;

        case SExt8:
            // Turn this: SExt8(constant)
            // Into this: static_cast<int8_t>(constant)
            if (m_value->child(0)->hasInt32()) {
                int32_t result = static_cast<int8_t>(m_value->child(0)->asInt32());
                replaceWithNewValue(m_proc.addIntConstant(m_value, result));
                break;
            }

            // Turn this: SExt8(SExt8(value))
            //   or this: SExt8(SExt16(value))
            // Into this: SExt8(value)
            if (m_value->child(0)->opcode() == SExt8 || m_value->child(0)->opcode() == SExt16) {
                m_value->child(0) = m_value->child(0)->child(0);
                m_changed = true;
            }

            if (m_value->child(0)->opcode() == BitAnd && m_value->child(0)->child(1)->hasInt32()) {
                Value* input = m_value->child(0)->child(0);
                int32_t mask = m_value->child(0)->child(1)->asInt32();
                
                // Turn this: SExt8(BitAnd(input, mask)) where (mask & 0xff) == 0xff
                // Into this: SExt8(input)
                if ((mask & 0xff) == 0xff) {
                    m_value->child(0) = input;
                    m_changed = true;
                    break;
                }
                
                // Turn this: SExt8(BitAnd(input, mask)) where (mask & 0x80) == 0
                // Into this: BitAnd(input, const & 0x7f)
                if (!(mask & 0x80)) {
                    replaceWithNewValue(
                        m_proc.add<Value>(
                            BitAnd, m_value->origin(), input,
                            m_insertionSet.insert<Const32Value>(
                                m_index, m_value->origin(), mask & 0x7f)));
                    break;
                }
            }
            
            if (!m_proc.hasQuirks()) {
                // Turn this: SExt8(AtomicXchg___)
                // Into this: AtomicXchg___
                if (isAtomicXchg(m_value->child(0)->opcode())
                    && m_value->child(0)->as<AtomicValue>()->accessWidth() == Width8) {
                    replaceWithIdentity(m_value->child(0));
                    break;
                }
            }
            break;

        case SExt16:
            // Turn this: SExt16(constant)
            // Into this: static_cast<int16_t>(constant)
            if (m_value->child(0)->hasInt32()) {
                int32_t result = static_cast<int16_t>(m_value->child(0)->asInt32());
                replaceWithNewValue(m_proc.addIntConstant(m_value, result));
                break;
            }

            // Turn this: SExt16(SExt16(value))
            // Into this: SExt16(value)
            if (m_value->child(0)->opcode() == SExt16) {
                m_value->child(0) = m_value->child(0)->child(0);
                m_changed = true;
            }

            // Turn this: SExt16(SExt8(value))
            // Into this: SExt8(value)
            if (m_value->child(0)->opcode() == SExt8) {
                replaceWithIdentity(m_value->child(0));
                break;
            }

            if (m_value->child(0)->opcode() == BitAnd && m_value->child(0)->child(1)->hasInt32()) {
                Value* input = m_value->child(0)->child(0);
                int32_t mask = m_value->child(0)->child(1)->asInt32();
                
                // Turn this: SExt16(BitAnd(input, mask)) where (mask & 0xffff) == 0xffff
                // Into this: SExt16(input)
                if ((mask & 0xffff) == 0xffff) {
                    m_value->child(0) = input;
                    m_changed = true;
                    break;
                }
                
                // Turn this: SExt16(BitAnd(input, mask)) where (mask & 0x8000) == 0
                // Into this: BitAnd(input, const & 0x7fff)
                if (!(mask & 0x8000)) {
                    replaceWithNewValue(
                        m_proc.add<Value>(
                            BitAnd, m_value->origin(), input,
                            m_insertionSet.insert<Const32Value>(
                                m_index, m_value->origin(), mask & 0x7fff)));
                    break;
                }
            }

            if (!m_proc.hasQuirks()) {
                // Turn this: SExt16(AtomicXchg___)
                // Into this: AtomicXchg___
                if (isAtomicXchg(m_value->child(0)->opcode())
                    && m_value->child(0)->as<AtomicValue>()->accessWidth() == Width16) {
                    replaceWithIdentity(m_value->child(0));
                    break;
                }
            }
            break;

        case SExt32:
            // Turn this: SExt32(constant)
            // Into this: static_cast<int64_t>(constant)
            if (m_value->child(0)->hasInt32()) {
                replaceWithNewValue(m_proc.addIntConstant(m_value, m_value->child(0)->asInt32()));
                break;
            }

            // Turn this: SExt32(BitAnd(input, mask)) where (mask & 0x80000000) == 0
            // Into this: ZExt32(BitAnd(input, mask))
            if (m_value->child(0)->opcode() == BitAnd && m_value->child(0)->child(1)->hasInt32()
                && !(m_value->child(0)->child(1)->asInt32() & 0x80000000)) {
                replaceWithNewValue(
                    m_proc.add<Value>(
                        ZExt32, m_value->origin(), m_value->child(0)));
                break;
            }
            break;

        case ZExt32:
            // Turn this: ZExt32(constant)
            // Into this: static_cast<uint64_t>(static_cast<uint32_t>(constant))
            if (m_value->child(0)->hasInt32()) {
                replaceWithNewValue(
                    m_proc.addIntConstant(
                        m_value,
                        static_cast<uint64_t>(static_cast<uint32_t>(m_value->child(0)->asInt32()))));
                break;
            }
            break;

        case Trunc:
            // Turn this: Trunc(constant)
            // Into this: static_cast<int32_t>(constant)
            if (m_value->child(0)->hasInt64() || m_value->child(0)->hasDouble()) {
                replaceWithNewValue(
                    m_proc.addIntConstant(m_value, static_cast<int32_t>(m_value->child(0)->asInt64())));
                break;
            }

            // Turn this: Trunc(SExt32(value)) or Trunc(ZExt32(value))
            // Into this: value
            if (m_value->child(0)->opcode() == SExt32 || m_value->child(0)->opcode() == ZExt32) {
                replaceWithIdentity(m_value->child(0)->child(0));
                break;
            }

            // Turn this: Trunc(Op(value, constant))
            //     where !(constant & 0xffffffff)
            //       and Op is Add, Sub, BitOr, or BitXor
            // into this: Trunc(value)
            switch (m_value->child(0)->opcode()) {
            case Add:
            case Sub:
            case BitOr:
            case BitXor:
                if (m_value->child(0)->child(1)->hasInt64()
                    && !(m_value->child(0)->child(1)->asInt64() & 0xffffffffll)) {
                    m_value->child(0) = m_value->child(0)->child(0);
                    m_changed = true;
                    break;
                }
                break;
            default:
                break;
            }
            break;

        case IToD:
            // Turn this: IToD(constant)
            // Into this: ConstDouble(constant)
            if (Value* constant = m_value->child(0)->iToDConstant(m_proc)) {
                replaceWithNewValue(constant);
                break;
            }
            break;

        case IToF:
            // Turn this: IToF(constant)
            // Into this: ConstFloat(constant)
            if (Value* constant = m_value->child(0)->iToFConstant(m_proc)) {
                replaceWithNewValue(constant);
                break;
            }
            break;

        case FloatToDouble:
            // Turn this: FloatToDouble(constant)
            // Into this: ConstDouble(constant)
            if (Value* constant = m_value->child(0)->floatToDoubleConstant(m_proc)) {
                replaceWithNewValue(constant);
                break;
            }
            break;

        case DoubleToFloat:
            // Turn this: DoubleToFloat(FloatToDouble(value))
            // Into this: value
            if (m_value->child(0)->opcode() == FloatToDouble) {
                replaceWithIdentity(m_value->child(0)->child(0));
                break;
            }

            // Turn this: DoubleToFloat(constant)
            // Into this: ConstFloat(constant)
            if (Value* constant = m_value->child(0)->doubleToFloatConstant(m_proc)) {
                replaceWithNewValue(constant);
                break;
            }
            break;

        case Select:
            // Turn this: Select(constant, a, b)
            // Into this: constant ? a : b
            if (m_value->child(0)->hasInt32()) {
                replaceWithIdentity(
                    m_value->child(0)->asInt32() ? m_value->child(1) : m_value->child(2));
                break;
            }

            // Turn this: Select(Equal(x, 0), a, b)
            // Into this: Select(x, b, a)
            if (m_value->child(0)->opcode() == Equal && m_value->child(0)->child(1)->isInt(0)) {
                m_value->child(0) = m_value->child(0)->child(0);
                std::swap(m_value->child(1), m_value->child(2));
                m_changed = true;
                break;
            }

            // Turn this: Select(BitXor(bool, 1), a, b)
            // Into this: Select(bool, b, a)
            if (m_value->child(0)->opcode() == BitXor
                && m_value->child(0)->child(1)->isInt32(1)
                && m_value->child(0)->child(0)->returnsBool()) {
                m_value->child(0) = m_value->child(0)->child(0);
                std::swap(m_value->child(1), m_value->child(2));
                m_changed = true;
                break;
            }

            // Turn this: Select(BitAnd(bool, xyz1), a, b)
            // Into this: Select(bool, a, b)
            if (m_value->child(0)->opcode() == BitAnd
                && m_value->child(0)->child(1)->hasInt()
                && m_value->child(0)->child(1)->asInt() & 1
                && m_value->child(0)->child(0)->returnsBool()) {
                m_value->child(0) = m_value->child(0)->child(0);
                m_changed = true;
                break;
            }

            // Turn this: Select(stuff, x, x)
            // Into this: x
            if (m_value->child(1) == m_value->child(2)) {
                replaceWithIdentity(m_value->child(1));
                break;
            }
            break;

        case Load8Z:
        case Load8S:
        case Load16Z:
        case Load16S:
        case Load:
        case Store8:
        case Store16:
        case Store: {
            Value* address = m_value->lastChild();
            MemoryValue* memory = m_value->as<MemoryValue>();

            // Turn this: Load(Add(address, offset1), offset = offset2)
            // Into this: Load(address, offset = offset1 + offset2)
            //
            // Also turns this: Store(value, Add(address, offset1), offset = offset2)
            // Into this: Store(value, address, offset = offset1 + offset2)
            if (address->opcode() == Add && address->child(1)->hasIntPtr()) {
                intptr_t offset = address->child(1)->asIntPtr();
                if (!sumOverflows<intptr_t>(offset, memory->offset())) {
                    offset += memory->offset();
                    int32_t smallOffset = static_cast<int32_t>(offset);
                    if (smallOffset == offset) {
                        address = address->child(0);
                        memory->lastChild() = address;
                        memory->setOffset(smallOffset);
                        m_changed = true;
                    }
                }
            }

            // Turn this: Load(constant1, offset = constant2)
            // Into this: Load(constant1 + constant2)
            //
            // This is a fun canonicalization. It purely regresses naively generated code. We rely
            // on constant materialization to be smart enough to materialize this constant the smart
            // way. We want this canonicalization because we want to know if two memory accesses see
            // the same address.
            if (memory->offset()) {
                if (Value* newAddress = address->addConstant(m_proc, memory->offset())) {
                    m_insertionSet.insertValue(m_index, newAddress);
                    address = newAddress;
                    memory->lastChild() = newAddress;
                    memory->setOffset(0);
                    m_changed = true;
                }
            }
            
            break;
        }

        case CCall: {
            // Turn this: Call(fmod, constant1, constant2)
            // Into this: fcall-constant(constant1, constant2)
            double(*fmodDouble)(double, double) = fmod;
            if (m_value->type() == Double
                && m_value->numChildren() == 3
                && m_value->child(0)->isIntPtr(reinterpret_cast<intptr_t>(fmodDouble))
                && m_value->child(1)->type() == Double
                && m_value->child(2)->type() == Double) {
                replaceWithNewValue(m_value->child(1)->modConstant(m_proc, m_value->child(2)));
            }
            break;
        }
        case Equal:
            handleCommutativity();

            // Turn this: Equal(bool, 0)
            // Into this: BitXor(bool, 1)
            if (m_value->child(0)->returnsBool() && m_value->child(1)->isInt32(0)) {
                replaceWithNew<Value>(
                    BitXor, m_value->origin(), m_value->child(0),
                    m_insertionSet.insert<Const32Value>(m_index, m_value->origin(), 1));
                break;
            }
            
            // Turn this Equal(bool, 1)
            // Into this: bool
            if (m_value->child(0)->returnsBool() && m_value->child(1)->isInt32(1)) {
                replaceWithIdentity(m_value->child(0));
                break;
            }

            // Turn this: Equal(const1, const2)
            // Into this: const1 == const2
            replaceWithNewValue(
                m_proc.addBoolConstant(
                    m_value->origin(),
                    m_value->child(0)->equalConstant(m_value->child(1))));
            break;
            
        case NotEqual:
            handleCommutativity();

            if (m_value->child(0)->returnsBool()) {
                // Turn this: NotEqual(bool, 0)
                // Into this: bool
                if (m_value->child(1)->isInt32(0)) {
                    replaceWithIdentity(m_value->child(0));
                    break;
                }
                
                // Turn this: NotEqual(bool, 1)
                // Into this: Equal(bool, 0)
                if (m_value->child(1)->isInt32(1)) {
                    replaceWithNew<Value>(
                        Equal, m_value->origin(), m_value->child(0),
                        m_insertionSet.insertIntConstant(m_index, m_value->origin(), Int32, 0));
                    break;
                }
            }

            // Turn this: NotEqual(const1, const2)
            // Into this: const1 != const2
            replaceWithNewValue(
                m_proc.addBoolConstant(
                    m_value->origin(),
                    m_value->child(0)->notEqualConstant(m_value->child(1))));
            break;

        case LessThan:
            // FIXME: We could do a better job of canonicalizing integer comparisons.
            // https://bugs.webkit.org/show_bug.cgi?id=150958

            replaceWithNewValue(
                m_proc.addBoolConstant(
                    m_value->origin(),
                    m_value->child(0)->lessThanConstant(m_value->child(1))));
            break;

        case GreaterThan:
            replaceWithNewValue(
                m_proc.addBoolConstant(
                    m_value->origin(),
                    m_value->child(0)->greaterThanConstant(m_value->child(1))));
            break;

        case LessEqual:
            replaceWithNewValue(
                m_proc.addBoolConstant(
                    m_value->origin(),
                    m_value->child(0)->lessEqualConstant(m_value->child(1))));
            break;

        case GreaterEqual:
            replaceWithNewValue(
                m_proc.addBoolConstant(
                    m_value->origin(),
                    m_value->child(0)->greaterEqualConstant(m_value->child(1))));
            break;

        case Above:
            replaceWithNewValue(
                m_proc.addBoolConstant(
                    m_value->origin(),
                    m_value->child(0)->aboveConstant(m_value->child(1))));
            break;

        case Below:
            replaceWithNewValue(
                m_proc.addBoolConstant(
                    m_value->origin(),
                    m_value->child(0)->belowConstant(m_value->child(1))));
            break;

        case AboveEqual:
            replaceWithNewValue(
                m_proc.addBoolConstant(
                    m_value->origin(),
                    m_value->child(0)->aboveEqualConstant(m_value->child(1))));
            break;

        case BelowEqual:
            replaceWithNewValue(
                m_proc.addBoolConstant(
                    m_value->origin(),
                    m_value->child(0)->belowEqualConstant(m_value->child(1))));
            break;

        case EqualOrUnordered:
            handleCommutativity();

            // Turn this: Equal(const1, const2)
            // Into this: isunordered(const1, const2) || const1 == const2.
            // Turn this: Equal(value, const_NaN)
            // Into this: 1.
            replaceWithNewValue(
                m_proc.addBoolConstant(
                    m_value->origin(),
                    m_value->child(1)->equalOrUnorderedConstant(m_value->child(0))));
            break;

        case CheckAdd: {
            if (replaceWithNewValue(m_value->child(0)->checkAddConstant(m_proc, m_value->child(1))))
                break;

            handleCommutativity();
            
            if (m_value->child(1)->isInt(0)) {
                replaceWithIdentity(m_value->child(0));
                break;
            }

            IntRange leftRange = rangeFor(m_value->child(0));
            IntRange rightRange = rangeFor(m_value->child(1));
            if (!leftRange.couldOverflowAdd(rightRange, m_value->type())) {
                replaceWithNewValue(
                    m_proc.add<Value>(Add, m_value->origin(), m_value->child(0), m_value->child(1)));
                break;
            }
            break;
        }

        case CheckSub: {
            if (replaceWithNewValue(m_value->child(0)->checkSubConstant(m_proc, m_value->child(1))))
                break;

            if (m_value->child(1)->isInt(0)) {
                replaceWithIdentity(m_value->child(0));
                break;
            }

            if (Value* negatedConstant = m_value->child(1)->checkNegConstant(m_proc)) {
                m_insertionSet.insertValue(m_index, negatedConstant);
                m_value->as<CheckValue>()->convertToAdd();
                m_value->child(1) = negatedConstant;
                m_changed = true;
                break;
            }

            IntRange leftRange = rangeFor(m_value->child(0));
            IntRange rightRange = rangeFor(m_value->child(1));
            if (!leftRange.couldOverflowSub(rightRange, m_value->type())) {
                replaceWithNewValue(
                    m_proc.add<Value>(Sub, m_value->origin(), m_value->child(0), m_value->child(1)));
                break;
            }
            break;
        }

        case CheckMul: {
            if (replaceWithNewValue(m_value->child(0)->checkMulConstant(m_proc, m_value->child(1))))
                break;

            handleCommutativity();

            if (m_value->child(1)->hasInt()) {
                bool modified = true;
                switch (m_value->child(1)->asInt()) {
                case 0:
                    replaceWithNewValue(m_proc.addIntConstant(m_value, 0));
                    break;
                case 1:
                    replaceWithIdentity(m_value->child(0));
                    break;
                case 2:
                    m_value->as<CheckValue>()->convertToAdd();
                    m_value->child(1) = m_value->child(0);
                    m_changed = true;
                    break;
                default:
                    modified = false;
                    break;
                }
                if (modified)
                    break;
            }

            IntRange leftRange = rangeFor(m_value->child(0));
            IntRange rightRange = rangeFor(m_value->child(1));
            if (!leftRange.couldOverflowMul(rightRange, m_value->type())) {
                replaceWithNewValue(
                    m_proc.add<Value>(Mul, m_value->origin(), m_value->child(0), m_value->child(1)));
                break;
            }
            break;
        }

        case Check: {
            CheckValue* checkValue = m_value->as<CheckValue>();
            
            if (checkValue->child(0)->isLikeZero()) {
                checkValue->replaceWithNop();
                m_changed = true;
                break;
            }

            if (checkValue->child(0)->isLikeNonZero()) {
                PatchpointValue* patchpoint =
                    m_insertionSet.insert<PatchpointValue>(m_index, Void, checkValue->origin());

                patchpoint->effects = Effects();
                patchpoint->effects.reads = HeapRange::top();
                patchpoint->effects.exitsSideways = true;

                for (unsigned i = 1; i < checkValue->numChildren(); ++i)
                    patchpoint->append(checkValue->constrainedChild(i));

                patchpoint->setGenerator(checkValue->generator());

                // Replace the rest of the block with an Oops.
                for (unsigned i = m_index + 1; i < m_block->size() - 1; ++i)
                    m_block->at(i)->replaceWithBottom(m_insertionSet, m_index);
                m_block->last()->replaceWithOops(m_block);
                m_block->last()->setOrigin(checkValue->origin());

                // Replace ourselves last.
                checkValue->replaceWithNop();
                m_changedCFG = true;
                break;
            }

            if (checkValue->child(0)->opcode() == NotEqual
                && checkValue->child(0)->child(1)->isInt(0)) {
                checkValue->child(0) = checkValue->child(0)->child(0);
                m_changed = true;
            }

            // If we are checking some bounded-size SSA expression that leads to a Select that
            // has a constant as one of its results, then turn the Select into a Branch and split
            // the code between the Check and the Branch. For example, this:
            //
            //     @a = Select(@p, @x, 42)
            //     @b = Add(@a, 35)
            //     Check(@b)
            //
            // becomes this:
            //
            //     Branch(@p, #truecase, #falsecase)
            //
            //   BB#truecase:
            //     @b_truecase = Add(@x, 35)
            //     Check(@b_truecase)
            //     Upsilon(@x, ^a)
            //     Upsilon(@b_truecase, ^b)
            //     Jump(#continuation)
            //
            //   BB#falsecase:
            //     @b_falsecase = Add(42, 35)
            //     Check(@b_falsecase)
            //     Upsilon(42, ^a)
            //     Upsilon(@b_falsecase, ^b)
            //     Jump(#continuation)
            //
            //   BB#continuation:
            //     @a = Phi()
            //     @b = Phi()
            //
            // The goal of this optimization is to kill a lot of code in one of those basic
            // blocks. This is pretty much guaranteed since one of those blocks will replace all
            // uses of the Select with a constant, and that constant will be transitively used
            // from the check.
            static const unsigned selectSpecializationBound = 3;
            Value* select = findRecentNodeMatching(
                m_value->child(0), selectSpecializationBound,
                [&] (Value* value) -> bool {
                    return value->opcode() == Select
                        && (value->child(1)->isConstant() && value->child(2)->isConstant());
                });
            
            if (select) {
                specializeSelect(select);
                break;
            }
            break;
        }

        case Branch: {
            // Turn this: Branch(NotEqual(x, 0))
            // Into this: Branch(x)
            if (m_value->child(0)->opcode() == NotEqual && m_value->child(0)->child(1)->isInt(0)) {
                m_value->child(0) = m_value->child(0)->child(0);
                m_changed = true;
            }

            // Turn this: Branch(Equal(x, 0), then, else)
            // Into this: Branch(x, else, then)
            if (m_value->child(0)->opcode() == Equal && m_value->child(0)->child(1)->isInt(0)) {
                m_value->child(0) = m_value->child(0)->child(0);
                std::swap(m_block->taken(), m_block->notTaken());
                m_changed = true;
            }
            
            // Turn this: Branch(BitXor(bool, 1), then, else)
            // Into this: Branch(bool, else, then)
            if (m_value->child(0)->opcode() == BitXor
                && m_value->child(0)->child(1)->isInt32(1)
                && m_value->child(0)->child(0)->returnsBool()) {
                m_value->child(0) = m_value->child(0)->child(0);
                std::swap(m_block->taken(), m_block->notTaken());
                m_changed = true;
            }

            // Turn this: Branch(BitAnd(bool, xyb1), then, else)
            // Into this: Branch(bool, then, else)
            if (m_value->child(0)->opcode() == BitAnd
                && m_value->child(0)->child(1)->hasInt()
                && m_value->child(0)->child(1)->asInt() & 1
                && m_value->child(0)->child(0)->returnsBool()) {
                m_value->child(0) = m_value->child(0)->child(0);
                m_changed = true;
            }

            TriState triState = m_value->child(0)->asTriState();

            // Turn this: Branch(0, then, else)
            // Into this: Jump(else)
            if (triState == FalseTriState) {
                m_block->taken().block()->removePredecessor(m_block);
                m_value->replaceWithJump(m_block, m_block->notTaken());
                m_changedCFG = true;
                break;
            }

            // Turn this: Branch(not 0, then, else)
            // Into this: Jump(then)
            if (triState == TrueTriState) {
                m_block->notTaken().block()->removePredecessor(m_block);
                m_value->replaceWithJump(m_block, m_block->taken());
                m_changedCFG = true;
                break;
            }

            // If a check for the same property dominates us, we can kill the branch. This sort
            // of makes sense here because it's cheap, but hacks like this show that we're going
            // to need SCCP.
            Value* check = m_pureCSE.findMatch(
                ValueKey(Check, Void, m_value->child(0)), m_block, *m_dominators);
            if (check) {
                // The Check would have side-exited if child(0) was non-zero. So, it must be
                // zero here.
                m_block->taken().block()->removePredecessor(m_block);
                m_value->replaceWithJump(m_block, m_block->notTaken());
                m_changedCFG = true;
            }
            break;
        }
            
        default:
            break;
        }
    }

    // Find a node that:
    //     - functor(node) returns true.
    //     - it's reachable from the given node via children.
    //     - it's in the last "bound" slots in the current basic block.
    // This algorithm is optimized under the assumption that the bound is small.
    template<typename Functor>
    Value* findRecentNodeMatching(Value* start, unsigned bound, const Functor& functor)
    {
        unsigned startIndex = bound < m_index ? m_index - bound : 0;
        Value* result = nullptr;
        start->walk(
            [&] (Value* value) -> Value::WalkStatus {
                bool found = false;
                for (unsigned i = startIndex; i <= m_index; ++i) {
                    if (m_block->at(i) == value)
                        found = true;
                }
                if (!found)
                    return Value::IgnoreChildren;

                if (functor(value)) {
                    result = value;
                    return Value::Stop;
                }

                return Value::Continue;
            });
        return result;
    }

    // This specializes a sequence of code up to a Select. This doesn't work when we're at a
    // terminal. It would be cool to fix that eventually. The main problem is that instead of
    // splitting the block, we should just insert the then/else blocks. We'll have to create
    // double the Phis and double the Upsilons. It'll probably be the sort of optimization that
    // we want to do only after we've done loop optimizations, since this will *definitely*
    // obscure things. In fact, even this simpler form of select specialization will possibly
    // obscure other optimizations. It would be great to have two modes of strength reduction,
    // one that does obscuring optimizations and runs late, and another that does not do
    // obscuring optimizations and runs early.
    // FIXME: Make select specialization handle branches.
    // FIXME: Have a form of strength reduction that does no obscuring optimizations and runs
    // early.
    void specializeSelect(Value* source)
    {
        if (verbose)
            dataLog("Specializing select: ", deepDump(m_proc, source), "\n");

        // This mutates startIndex to account for the fact that m_block got the front of it
        // chopped off.
        BasicBlock* predecessor =
            m_blockInsertionSet.splitForward(m_block, m_index, &m_insertionSet);

        // Splitting will commit the insertion set, which changes the exact position of the
        // source. That's why we do the search after splitting.
        unsigned startIndex = UINT_MAX;
        for (unsigned i = predecessor->size(); i--;) {
            if (predecessor->at(i) == source) {
                startIndex = i;
                break;
            }
        }
        
        RELEASE_ASSERT(startIndex != UINT_MAX);

        // By BasicBlock convention, caseIndex == 0 => then, caseIndex == 1 => else.
        static const unsigned numCases = 2;
        BasicBlock* cases[numCases];
        for (unsigned i = 0; i < numCases; ++i)
            cases[i] = m_blockInsertionSet.insertBefore(m_block);

        HashMap<Value*, Value*> mappings[2];

        // Save things we want to know about the source.
        Value* predicate = source->child(0);

        for (unsigned i = 0; i < numCases; ++i)
            mappings[i].add(source, source->child(1 + i));

        auto cloneValue = [&] (Value* value) {
            ASSERT(value != source);

            for (unsigned i = 0; i < numCases; ++i) {
                Value* clone = m_proc.clone(value);
                for (Value*& child : clone->children()) {
                    if (Value* newChild = mappings[i].get(child))
                        child = newChild;
                }
                if (value->type() != Void)
                    mappings[i].add(value, clone);

                cases[i]->append(clone);
                if (value->type() != Void)
                    cases[i]->appendNew<UpsilonValue>(m_proc, value->origin(), clone, value);
            }

            value->replaceWithPhi();
        };

        // The jump that the splitter inserted is of no use to us.
        predecessor->removeLast(m_proc);

        // Hance the source, it's special.
        for (unsigned i = 0; i < numCases; ++i) {
            cases[i]->appendNew<UpsilonValue>(
                m_proc, source->origin(), source->child(1 + i), source);
        }
        source->replaceWithPhi();
        m_insertionSet.insertValue(m_index, source);

        // Now handle all values between the source and the check.
        for (unsigned i = startIndex + 1; i < predecessor->size(); ++i) {
            Value* value = predecessor->at(i);
            value->owner = nullptr;

            cloneValue(value);

            if (value->type() != Void)
                m_insertionSet.insertValue(m_index, value);
            else
                m_proc.deleteValue(value);
        }

        // Finally, deal with the check.
        cloneValue(m_value);

        // Remove the values from the predecessor.
        predecessor->values().resize(startIndex);
        
        predecessor->appendNew<Value>(m_proc, Branch, source->origin(), predicate);
        predecessor->setSuccessors(FrequentedBlock(cases[0]), FrequentedBlock(cases[1]));

        for (unsigned i = 0; i < numCases; ++i) {
            cases[i]->appendNew<Value>(m_proc, Jump, m_value->origin());
            cases[i]->setSuccessors(FrequentedBlock(m_block));
        }

        m_changed = true;

        predecessor->updatePredecessorsAfter();
    }

    // Turn this: Add(constant, value)
    // Into this: Add(value, constant)
    //
    // Also:
    // Turn this: Add(value1, value2)
    // Into this: Add(value2, value1)
    // If we decide that value2 coming first is the canonical ordering.
    void handleCommutativity()
    {
        // Note that we have commutative operations that take more than two children. Those operations may
        // commute their first two children while leaving the rest unaffected.
        ASSERT(m_value->numChildren() >= 2);
        
        // Leave it alone if the right child is a constant.
        if (m_value->child(1)->isConstant()
            || m_value->child(0)->opcode() == AtomicStrongCAS)
            return;
        
        auto swap = [&] () {
            std::swap(m_value->child(0), m_value->child(1));
            m_changed = true;
        };
        
        if (m_value->child(0)->isConstant())
            return swap();
        
        if (m_value->child(1)->opcode() == AtomicStrongCAS)
            return swap();
        
        // Sort the operands. This is an important canonicalization. We use the index instead of
        // the address to make this at least slightly deterministic.
        if (m_value->child(0)->index() > m_value->child(1)->index())
            return swap();
    }

    // FIXME: This should really be a forward analysis. Instead, we uses a bounded-search backwards
    // analysis.
    IntRange rangeFor(Value* value, unsigned timeToLive = 5)
    {
        if (!timeToLive)
            return IntRange::top(value->type());
        
        switch (value->opcode()) {
        case Const32:
        case Const64: {
            int64_t intValue = value->asInt();
            return IntRange(intValue, intValue);
        }

        case BitAnd:
            if (value->child(1)->hasInt())
                return IntRange::rangeForMask(value->child(1)->asInt(), value->type());
            break;

        case SShr:
            if (value->child(1)->hasInt32()) {
                return rangeFor(value->child(0), timeToLive - 1).sShr(
                    value->child(1)->asInt32(), value->type());
            }
            break;

        case ZShr:
            if (value->child(1)->hasInt32()) {
                return rangeFor(value->child(0), timeToLive - 1).zShr(
                    value->child(1)->asInt32(), value->type());
            }
            break;

        case Shl:
            if (value->child(1)->hasInt32()) {
                return rangeFor(value->child(0), timeToLive - 1).shl(
                    value->child(1)->asInt32(), value->type());
            }
            break;

        case Add:
            return rangeFor(value->child(0), timeToLive - 1).add(
                rangeFor(value->child(1), timeToLive - 1), value->type());

        case Sub:
            return rangeFor(value->child(0), timeToLive - 1).sub(
                rangeFor(value->child(1), timeToLive - 1), value->type());

        case Mul:
            return rangeFor(value->child(0), timeToLive - 1).mul(
                rangeFor(value->child(1), timeToLive - 1), value->type());

        default:
            break;
        }

        return IntRange::top(value->type());
    }

    template<typename ValueType, typename... Arguments>
    void replaceWithNew(Arguments... arguments)
    {
        replaceWithNewValue(m_proc.add<ValueType>(arguments...));
    }

    bool replaceWithNewValue(Value* newValue)
    {
        if (!newValue)
            return false;
        m_insertionSet.insertValue(m_index, newValue);
        m_value->replaceWithIdentity(newValue);
        m_changed = true;
        return true;
    }

    void replaceWithIdentity(Value* newValue)
    {
        m_value->replaceWithIdentity(newValue);
        m_changed = true;
    }

    void handleShiftAmount()
    {
        // Shift anything by zero is identity.
        if (m_value->child(1)->isInt32(0)) {
            replaceWithIdentity(m_value->child(0));
            return;
        }

        // The shift already masks its shift amount. If the shift amount is being masked by a
        // redundant amount, then remove the mask. For example,
        // Turn this: Shl(@x, BitAnd(@y, 63))
        // Into this: Shl(@x, @y)
        unsigned mask = sizeofType(m_value->type()) * 8 - 1;
        if (m_value->child(1)->opcode() == BitAnd
            && m_value->child(1)->child(1)->hasInt32()
            && (m_value->child(1)->child(1)->asInt32() & mask) == mask) {
            m_value->child(1) = m_value->child(1)->child(0);
            m_changed = true;
        }
    }

    void replaceIfRedundant()
    {
        m_changed |= m_pureCSE.process(m_value, *m_dominators);
    }

    void simplifyCFG()
    {
        if (verbose) {
            dataLog("Before simplifyCFG:\n");
            dataLog(m_proc);
        }
        
        // We have three easy simplification rules:
        //
        // 1) If a successor is a block that just jumps to another block, then jump directly to
        //    that block.
        //
        // 2) If all successors are the same and the operation has no effects, then use a jump
        //    instead.
        //
        // 3) If you jump to a block that is not you and has one predecessor, then merge.
        //
        // Note that because of the first rule, this phase may introduce critical edges. That's fine.
        // If you need broken critical edges, then you have to break them yourself.

        // Note that this relies on predecessors being at least conservatively correct. It's fine for
        // predecessors to mention a block that isn't actually a predecessor. It's *not* fine for a
        // predecessor to be omitted. We assert as much in the loop. In practice, we precisely preserve
        // predecessors during strength reduction since that minimizes the total number of fixpoint
        // iterations needed to kill a lot of code.

        for (BasicBlock* block : m_proc) {
            if (verbose)
                dataLog("Considering block ", *block, ":\n");

            checkPredecessorValidity();

            // We don't care about blocks that don't have successors.
            if (!block->numSuccessors())
                continue;

            // First check if any of the successors of this block can be forwarded over.
            for (BasicBlock*& successor : block->successorBlocks()) {
                if (successor != block
                    && successor->size() == 1
                    && successor->last()->opcode() == Jump) {
                    BasicBlock* newSuccessor = successor->successorBlock(0);
                    if (newSuccessor != successor) {
                        if (verbose) {
                            dataLog(
                                "Replacing ", pointerDump(block), "->", pointerDump(successor),
                                " with ", pointerDump(block), "->", pointerDump(newSuccessor),
                                "\n");
                        }
                        // Note that we do not do replacePredecessor() because the block we're
                        // skipping will still have newSuccessor as its successor.
                        newSuccessor->addPredecessor(block);
                        successor = newSuccessor;
                        m_changedCFG = true;
                    }
                }
            }

            // Now check if the block's terminal can be replaced with a jump.
            if (block->numSuccessors() > 1) {
                // The terminal must not have weird effects.
                Effects effects = block->last()->effects();
                effects.terminal = false;
                if (!effects.mustExecute()) {
                    // All of the successors must be the same.
                    bool allSame = true;
                    BasicBlock* firstSuccessor = block->successorBlock(0);
                    for (unsigned i = 1; i < block->numSuccessors(); ++i) {
                        if (block->successorBlock(i) != firstSuccessor) {
                            allSame = false;
                            break;
                        }
                    }
                    if (allSame) {
                        if (verbose) {
                            dataLog(
                                "Changing ", pointerDump(block), "'s terminal to a Jump.\n");
                        }
                        block->last()->replaceWithJump(block, FrequentedBlock(firstSuccessor));
                        m_changedCFG = true;
                    }
                }
            }

            // Finally handle jumps to a block with one predecessor.
            if (block->numSuccessors() == 1) {
                BasicBlock* successor = block->successorBlock(0);
                if (successor != block && successor->numPredecessors() == 1) {
                    RELEASE_ASSERT(successor->predecessor(0) == block);
                    
                    // We can merge the two blocks, because the predecessor only jumps to the successor
                    // and the successor is only reachable from the predecessor.
                    
                    // Remove the terminal.
                    Value* value = block->values().takeLast();
                    Origin jumpOrigin = value->origin();
                    RELEASE_ASSERT(value->effects().terminal);
                    m_proc.deleteValue(value);
                    
                    // Append the full contents of the successor to the predecessor.
                    block->values().appendVector(successor->values());
                    block->successors() = successor->successors();
                    
                    // Make sure that the successor has nothing left in it. Make sure that the block
                    // has a terminal so that nobody chokes when they look at it.
                    successor->values().resize(0);
                    successor->appendNew<Value>(m_proc, Oops, jumpOrigin);
                    successor->clearSuccessors();
                    
                    // Ensure that predecessors of block's new successors know what's up.
                    for (BasicBlock* newSuccessor : block->successorBlocks())
                        newSuccessor->replacePredecessor(successor, block);

                    if (verbose) {
                        dataLog(
                            "Merged ", pointerDump(block), "->", pointerDump(successor), "\n");
                    }
                    
                    m_changedCFG = true;
                }
            }
        }

        if (m_changedCFG && verbose) {
            dataLog("B3 after simplifyCFG:\n");
            dataLog(m_proc);
        }
    }

    void checkPredecessorValidity()
    {
        if (!shouldValidateIRAtEachPhase())
            return;

        for (BasicBlock* block : m_proc) {
            for (BasicBlock* successor : block->successorBlocks())
                RELEASE_ASSERT(successor->containsPredecessor(block));
        }
    }

    void killDeadCode()
    {
        GraphNodeWorklist<Value*, IndexSet<Value>> worklist;
        Vector<UpsilonValue*, 64> upsilons;
        for (BasicBlock* block : m_proc) {
            for (Value* value : *block) {
                Effects effects;
                // We don't care about effects of SSA operations, since we model them more
                // accurately than the effects() method does.
                if (value->opcode() != Phi && value->opcode() != Upsilon)
                    effects = value->effects();
                
                if (effects.mustExecute())
                    worklist.push(value);
                
                if (UpsilonValue* upsilon = value->as<UpsilonValue>())
                    upsilons.append(upsilon);
            }
        }
        for (;;) {
            while (Value* value = worklist.pop()) {
                for (Value* child : value->children())
                    worklist.push(child);
            }
            
            bool didPush = false;
            for (size_t upsilonIndex = 0; upsilonIndex < upsilons.size(); ++upsilonIndex) {
                UpsilonValue* upsilon = upsilons[upsilonIndex];
                if (worklist.saw(upsilon->phi())) {
                    worklist.push(upsilon);
                    upsilons[upsilonIndex--] = upsilons.last();
                    upsilons.takeLast();
                    didPush = true;
                }
            }
            if (!didPush)
                break;
        }

        IndexSet<Variable> liveVariables;
        
        for (BasicBlock* block : m_proc) {
            size_t sourceIndex = 0;
            size_t targetIndex = 0;
            while (sourceIndex < block->size()) {
                Value* value = block->at(sourceIndex++);
                if (worklist.saw(value)) {
                    if (VariableValue* variableValue = value->as<VariableValue>())
                        liveVariables.add(variableValue->variable());
                    block->at(targetIndex++) = value;
                } else {
                    m_proc.deleteValue(value);
                    m_changed = true;
                }
            }
            block->values().resize(targetIndex);
        }

        for (Variable* variable : m_proc.variables()) {
            if (!liveVariables.contains(variable))
                m_proc.deleteVariable(variable);
        }
    }

    void simplifySSA()
    {
        // This runs Aycock and Horspool's algorithm on our Phi functions [1]. For most CFG patterns,
        // this can take a suboptimal arrangement of Phi functions and make it optimal, as if you had
        // run Cytron, Ferrante, Rosen, Wegman, and Zadeck. It's only suboptimal for irreducible
        // CFGs. In practice, that doesn't matter, since we expect clients of B3 to run their own SSA
        // conversion before lowering to B3, and in the case of the DFG, that conversion uses Cytron
        // et al. In that context, this algorithm is intended to simplify Phi functions that were
        // made redundant by prior CFG simplification. But according to Aycock and Horspool's paper,
        // this algorithm is good enough that a B3 client could just give us maximal Phi's (i.e. Phi
        // for each variable at each basic block) and we will make them optimal.
        // [1] http://pages.cpsc.ucalgary.ca/~aycock/papers/ssa.ps

        // Aycock and Horspool prescribe two rules that are to be run to fixpoint:
        //
        // 1) If all of the Phi's children are the same (i.e. it's one child referenced from one or
        //    more Upsilons), then replace all uses of the Phi with the one child.
        //
        // 2) If all of the Phi's children are either the Phi itself or exactly one other child, then
        //    replace all uses of the Phi with the one other child.
        //
        // Rule (2) subsumes rule (1), so we can just run (2). We only run one fixpoint iteration
        // here. This premise is that in common cases, this will only find optimization opportunities
        // as a result of CFG simplification and usually CFG simplification will only do one round
        // of block merging per ReduceStrength fixpoint iteration, so it's OK for this to only do one
        // round of Phi merging - since Phis are the value analogue of blocks.

        PhiChildren phiChildren(m_proc);

        for (Value* phi : phiChildren.phis()) {
            Value* otherChild = nullptr;
            bool ok = true;
            for (Value* child : phiChildren[phi].values()) {
                if (child == phi)
                    continue;
                if (child == otherChild)
                    continue;
                if (!otherChild) {
                    otherChild = child;
                    continue;
                }
                ok = false;
                break;
            }
            if (!ok)
                continue;
            if (!otherChild) {
                // Wow, this would be super weird. It probably won't happen, except that things could
                // get weird as a consequence of stepwise simplifications in the strength reduction
                // fixpoint.
                continue;
            }
            
            // Turn the Phi into an Identity and turn the Upsilons into Nops.
            m_changed = true;
            for (Value* upsilon : phiChildren[phi])
                upsilon->replaceWithNop();
            phi->replaceWithIdentity(otherChild);
        }
    }

    Procedure& m_proc;
    InsertionSet m_insertionSet;
    BlockInsertionSet m_blockInsertionSet;
    BasicBlock* m_block { nullptr };
    unsigned m_index { 0 };
    Value* m_value { nullptr };
    Dominators* m_dominators { nullptr };
    PureCSE m_pureCSE;
    bool m_changed { false };
    bool m_changedCFG { false };
};

} // anonymous namespace

bool reduceStrength(Procedure& proc)
{
    PhaseScope phaseScope(proc, "reduceStrength");
    ReduceStrength reduceStrength(proc);
    return reduceStrength.run();
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

