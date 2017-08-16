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

#pragma once

#include "DFGCommon.h"

#if ENABLE(FTL_JIT)

#include "B3BasicBlockInlines.h"
#include "B3CCallValue.h"
#include "B3Compilation.h"
#include "B3FrequentedBlock.h"
#include "B3Procedure.h"
#include "B3SwitchValue.h"
#include "FTLAbbreviatedTypes.h"
#include "FTLAbstractHeapRepository.h"
#include "FTLCommonValues.h"
#include "FTLState.h"
#include "FTLSwitchCase.h"
#include "FTLTypedPointer.h"
#include "FTLValueFromBlock.h"
#include "FTLWeight.h"
#include "FTLWeightedTarget.h"
#include "HeapCell.h"
#include <wtf/OrderMaker.h>
#include <wtf/StringPrintStream.h>

// FIXME: remove this once everything can be generated through B3.
#if COMPILER(GCC_OR_CLANG)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif // COMPILER(GCC_OR_CLANG)

namespace JSC {

namespace DFG {
struct Node;
} // namespace DFG

namespace B3 {
class FenceValue;
class SlotBaseValue;
} // namespace B3

namespace FTL {

enum Scale { ScaleOne, ScaleTwo, ScaleFour, ScaleEight, ScalePtr };

class Output : public CommonValues {
public:
    Output(State&);
    ~Output();

    void initialize(AbstractHeapRepository&);

    void setFrequency(double value)
    {
        m_frequency = value;
    }

    LBasicBlock newBlock();

    LBasicBlock insertNewBlocksBefore(LBasicBlock nextBlock)
    {
        LBasicBlock lastNextBlock = m_nextBlock;
        m_nextBlock = nextBlock;
        return lastNextBlock;
    }

    void applyBlockOrder();

    LBasicBlock appendTo(LBasicBlock, LBasicBlock nextBlock);
    void appendTo(LBasicBlock);

    void setOrigin(DFG::Node* node) { m_origin = node; }
    B3::Origin origin() { return B3::Origin(m_origin); }

    LValue framePointer();

    B3::SlotBaseValue* lockedStackSlot(size_t bytes);

    LValue constBool(bool value);
    LValue constInt32(int32_t value);

    LValue weakPointer(DFG::Graph& graph, JSCell* cell)
    {
        ASSERT(graph.m_plan.weakReferences.contains(cell));

        if (sizeof(void*) == 8)
            return constInt64(bitwise_cast<intptr_t>(cell));
        return constInt32(bitwise_cast<intptr_t>(cell));
    }

    LValue weakPointer(DFG::FrozenValue* value)
    {
        RELEASE_ASSERT(value->value().isCell());

        if (sizeof(void*) == 8)
            return constInt64(bitwise_cast<intptr_t>(value->cell()));
        return constInt32(bitwise_cast<intptr_t>(value->cell()));
    }

    template<typename T>
    LValue constIntPtr(T* value)
    {
        static_assert(!std::is_base_of<HeapCell, T>::value, "To use a GC pointer, the graph must be aware of it. Use gcPointer instead and make sure the graph is aware of this reference.");
        if (sizeof(void*) == 8)
            return constInt64(bitwise_cast<intptr_t>(value));
        return constInt32(bitwise_cast<intptr_t>(value));
    }
    template<typename T>
    LValue constIntPtr(T value)
    {
        if (sizeof(void*) == 8)
            return constInt64(static_cast<intptr_t>(value));
        return constInt32(static_cast<intptr_t>(value));
    }
    LValue constInt64(int64_t value);
    LValue constDouble(double value);

    LValue phi(LType);
    template<typename... Params>
    LValue phi(LType, ValueFromBlock, Params... theRest);
    template<typename VectorType>
    LValue phi(LType, const VectorType&);
    void addIncomingToPhi(LValue phi, ValueFromBlock);
    template<typename... Params>
    void addIncomingToPhi(LValue phi, ValueFromBlock, Params... theRest);

    LValue add(LValue, LValue);
    LValue sub(LValue, LValue);
    LValue mul(LValue, LValue);
    LValue div(LValue, LValue);
    LValue chillDiv(LValue, LValue);
    LValue mod(LValue, LValue);
    LValue chillMod(LValue, LValue);
    LValue neg(LValue);

    LValue doubleAdd(LValue, LValue);
    LValue doubleSub(LValue, LValue);
    LValue doubleMul(LValue, LValue);
    LValue doubleDiv(LValue, LValue);
    LValue doubleMod(LValue, LValue);
    LValue doubleNeg(LValue value) { return neg(value); }

    LValue bitAnd(LValue, LValue);
    LValue bitOr(LValue, LValue);
    LValue bitXor(LValue, LValue);
    LValue shl(LValue, LValue shiftAmount);
    LValue aShr(LValue, LValue shiftAmount);
    LValue lShr(LValue, LValue shiftAmount);
    LValue bitNot(LValue);
    LValue logicalNot(LValue);

    LValue ctlz32(LValue);
    LValue doubleAbs(LValue);
    LValue doubleCeil(LValue);
    LValue doubleFloor(LValue);
    LValue doubleTrunc(LValue);

    LValue doubleSin(LValue);
    LValue doubleCos(LValue);
    LValue doubleTan(LValue);

    LValue doublePow(LValue base, LValue exponent);
    LValue doublePowi(LValue base, LValue exponent);

    LValue doubleSqrt(LValue);

    LValue doubleLog(LValue);

    LValue doubleToInt(LValue);
    LValue doubleToUInt(LValue);

    LValue signExt32To64(LValue);
    LValue signExt32ToPtr(LValue);
    LValue zeroExt(LValue, LType);
    LValue zeroExtPtr(LValue value) { return zeroExt(value, B3::Int64); }
    LValue intToDouble(LValue);
    LValue unsignedToDouble(LValue);
    LValue castToInt32(LValue);
    LValue doubleToFloat(LValue);
    LValue floatToDouble(LValue);
    LValue bitCast(LValue, LType);
    LValue fround(LValue);

    LValue load(TypedPointer, LType);
    void store(LValue, TypedPointer);
    B3::FenceValue* fence(const AbstractHeap* read, const AbstractHeap* write);

    LValue load8SignExt32(TypedPointer);
    LValue load8ZeroExt32(TypedPointer);
    LValue load16SignExt32(TypedPointer);
    LValue load16ZeroExt32(TypedPointer);
    LValue load32(TypedPointer pointer) { return load(pointer, B3::Int32); }
    LValue load64(TypedPointer pointer) { return load(pointer, B3::Int64); }
    LValue loadPtr(TypedPointer pointer) { return load(pointer, B3::pointerType()); }
    LValue loadFloat(TypedPointer pointer) { return load(pointer, B3::Float); }
    LValue loadDouble(TypedPointer pointer) { return load(pointer, B3::Double); }
    void store32As8(LValue, TypedPointer);
    void store32As16(LValue, TypedPointer);
    void store32(LValue value, TypedPointer pointer)
    {
        ASSERT(value->type() == B3::Int32);
        store(value, pointer);
    }
    void store64(LValue value, TypedPointer pointer)
    {
        ASSERT(value->type() == B3::Int64);
        store(value, pointer);
    }
    void storePtr(LValue value, TypedPointer pointer)
    {
        ASSERT(value->type() == B3::pointerType());
        store(value, pointer);
    }
    void storeFloat(LValue value, TypedPointer pointer)
    {
        ASSERT(value->type() == B3::Float);
        store(value, pointer);
    }
    void storeDouble(LValue value, TypedPointer pointer)
    {
        ASSERT(value->type() == B3::Double);
        store(value, pointer);
    }

    enum LoadType {
        Load8SignExt32,
        Load8ZeroExt32,
        Load16SignExt32,
        Load16ZeroExt32,
        Load32,
        Load64,
        LoadPtr,
        LoadFloat,
        LoadDouble
    };

    LValue load(TypedPointer, LoadType);
    
    enum StoreType {
        Store32As8,
        Store32As16,
        Store32,
        Store64,
        StorePtr,
        StoreFloat,
        StoreDouble
    };

    void store(LValue, TypedPointer, StoreType);

    LValue addPtr(LValue value, ptrdiff_t immediate = 0)
    {
        if (!immediate)
            return value;
        return add(value, constIntPtr(immediate));
    }

    // Construct an address by offsetting base by the requested amount and ascribing
    // the requested abstract heap to it.
    TypedPointer address(const AbstractHeap& heap, LValue base, ptrdiff_t offset = 0)
    {
        return TypedPointer(heap, addPtr(base, offset));
    }
    // Construct an address by offsetting base by the amount specified by the field,
    // and optionally an additional amount (use this with care), and then creating
    // a TypedPointer with the given field as the heap.
    TypedPointer address(LValue base, const AbstractHeap& field, ptrdiff_t offset = 0)
    {
        return address(field, base, offset + field.offset());
    }

    LValue baseIndex(LValue base, LValue index, Scale, ptrdiff_t offset = 0);

    TypedPointer baseIndex(const AbstractHeap& heap, LValue base, LValue index, Scale scale, ptrdiff_t offset = 0)
    {
        return TypedPointer(heap, baseIndex(base, index, scale, offset));
    }
    TypedPointer baseIndex(IndexedAbstractHeap& heap, LValue base, LValue index, JSValue indexAsConstant = JSValue(), ptrdiff_t offset = 0)
    {
        return heap.baseIndex(*this, base, index, indexAsConstant, offset);
    }

    TypedPointer absolute(const void* address);

    LValue load8SignExt32(LValue base, const AbstractHeap& field) { return load8SignExt32(address(base, field)); }
    LValue load8ZeroExt32(LValue base, const AbstractHeap& field) { return load8ZeroExt32(address(base, field)); }
    LValue load16SignExt32(LValue base, const AbstractHeap& field) { return load16SignExt32(address(base, field)); }
    LValue load16ZeroExt32(LValue base, const AbstractHeap& field) { return load16ZeroExt32(address(base, field)); }
    LValue load32(LValue base, const AbstractHeap& field) { return load32(address(base, field)); }
    LValue load64(LValue base, const AbstractHeap& field) { return load64(address(base, field)); }
    LValue loadPtr(LValue base, const AbstractHeap& field) { return loadPtr(address(base, field)); }
    LValue loadDouble(LValue base, const AbstractHeap& field) { return loadDouble(address(base, field)); }
    void store32(LValue value, LValue base, const AbstractHeap& field) { store32(value, address(base, field)); }
    void store64(LValue value, LValue base, const AbstractHeap& field) { store64(value, address(base, field)); }
    void storePtr(LValue value, LValue base, const AbstractHeap& field) { storePtr(value, address(base, field)); }
    void storeDouble(LValue value, LValue base, const AbstractHeap& field) { storeDouble(value, address(base, field)); }

    // FIXME: Explore adding support for value range constraints to B3. Maybe it could be as simple as having
    // a load instruction that guarantees that its result is non-negative.
    // https://bugs.webkit.org/show_bug.cgi?id=151458
    void ascribeRange(LValue, const ValueRange&) { }
    LValue nonNegative32(LValue loadInstruction) { return loadInstruction; }
    LValue load32NonNegative(TypedPointer pointer) { return load32(pointer); }
    LValue load32NonNegative(LValue base, const AbstractHeap& field) { return load32(base, field); }

    LValue equal(LValue, LValue);
    LValue notEqual(LValue, LValue);
    LValue above(LValue, LValue);
    LValue aboveOrEqual(LValue, LValue);
    LValue below(LValue, LValue);
    LValue belowOrEqual(LValue, LValue);
    LValue greaterThan(LValue, LValue);
    LValue greaterThanOrEqual(LValue, LValue);
    LValue lessThan(LValue, LValue);
    LValue lessThanOrEqual(LValue, LValue);

    LValue doubleEqual(LValue, LValue);
    LValue doubleEqualOrUnordered(LValue, LValue);
    LValue doubleNotEqualOrUnordered(LValue, LValue);
    LValue doubleLessThan(LValue, LValue);
    LValue doubleLessThanOrEqual(LValue, LValue);
    LValue doubleGreaterThan(LValue, LValue);
    LValue doubleGreaterThanOrEqual(LValue, LValue);
    LValue doubleNotEqualAndOrdered(LValue, LValue);
    LValue doubleLessThanOrUnordered(LValue, LValue);
    LValue doubleLessThanOrEqualOrUnordered(LValue, LValue);
    LValue doubleGreaterThanOrUnordered(LValue, LValue);
    LValue doubleGreaterThanOrEqualOrUnordered(LValue, LValue);

    LValue isZero32(LValue);
    LValue notZero32(LValue);
    LValue isZero64(LValue);
    LValue notZero64(LValue);
    LValue isNull(LValue value) { return isZero64(value); }
    LValue notNull(LValue value) { return notZero64(value); }

    LValue testIsZero32(LValue value, LValue mask) { return isZero32(bitAnd(value, mask)); }
    LValue testNonZero32(LValue value, LValue mask) { return notZero32(bitAnd(value, mask)); }
    LValue testIsZero64(LValue value, LValue mask) { return isZero64(bitAnd(value, mask)); }
    LValue testNonZero64(LValue value, LValue mask) { return notZero64(bitAnd(value, mask)); }
    LValue testIsZeroPtr(LValue value, LValue mask) { return isNull(bitAnd(value, mask)); }
    LValue testNonZeroPtr(LValue value, LValue mask) { return notNull(bitAnd(value, mask)); }

    LValue select(LValue value, LValue taken, LValue notTaken);

    template<typename VectorType>
    LValue call(LType type, LValue function, const VectorType& vector)
    {
        B3::CCallValue* result = m_block->appendNew<B3::CCallValue>(m_proc, type, origin(), function);
        result->children().appendVector(vector);
        return result;
    }
    LValue call(LType type, LValue function) { return m_block->appendNew<B3::CCallValue>(m_proc, type, origin(), function); }
    LValue call(LType type, LValue function, LValue arg1) { return m_block->appendNew<B3::CCallValue>(m_proc, type, origin(), function, arg1); }
    template<typename... Args>
    LValue call(LType type, LValue function, LValue arg1, Args... args) { return m_block->appendNew<B3::CCallValue>(m_proc, type, origin(), function, arg1, args...); }

    template<typename Function, typename... Args>
    LValue callWithoutSideEffects(B3::Type type, Function function, LValue arg1, Args... args)
    {
        return m_block->appendNew<B3::CCallValue>(m_proc, type, origin(), B3::Effects::none(),
            constIntPtr(bitwise_cast<void*>(function)), arg1, args...);
    }

    template<typename FunctionType>
    LValue operation(FunctionType function) { return constIntPtr(bitwise_cast<void*>(function)); }

    void jump(LBasicBlock);
    void branch(LValue condition, LBasicBlock taken, Weight takenWeight, LBasicBlock notTaken, Weight notTakenWeight);
    void branch(LValue condition, WeightedTarget taken, WeightedTarget notTaken)
    {
        branch(condition, taken.target(), taken.weight(), notTaken.target(), notTaken.weight());
    }

    // Branches to an already-created handler if true, "falls through" if false. Fall-through is
    // simulated by creating a continuation for you.
    void check(LValue condition, WeightedTarget taken, Weight notTakenWeight);
    
    // Same as check(), but uses Weight::inverse() to compute the notTakenWeight.
    void check(LValue condition, WeightedTarget taken);
    
    template<typename VectorType>
    void switchInstruction(LValue value, const VectorType& cases, LBasicBlock fallThrough, Weight fallThroughWeight)
    {
        B3::SwitchValue* switchValue = m_block->appendNew<B3::SwitchValue>(m_proc, origin(), value);
        switchValue->setFallThrough(B3::FrequentedBlock(fallThrough));
        for (const SwitchCase& switchCase : cases) {
            int64_t value = switchCase.value()->asInt();
            B3::FrequentedBlock target(switchCase.target(), switchCase.weight().frequencyClass());
            switchValue->appendCase(B3::SwitchCase(value, target));
        }
    }

    void ret(LValue);

    void unreachable();
    
    void appendSuccessor(WeightedTarget);

    B3::CheckValue* speculate(LValue);
    B3::CheckValue* speculateAdd(LValue, LValue);
    B3::CheckValue* speculateSub(LValue, LValue);
    B3::CheckValue* speculateMul(LValue, LValue);

    B3::PatchpointValue* patchpoint(LType);

    void trap();

    ValueFromBlock anchor(LValue);

    void incrementSuperSamplerCount();
    void decrementSuperSamplerCount();

#if PLATFORM(COCOA)
#pragma mark - States
#endif
    B3::Procedure& m_proc;

    DFG::Node* m_origin { nullptr };
    LBasicBlock m_block { nullptr };
    LBasicBlock m_nextBlock { nullptr };

    AbstractHeapRepository* m_heaps;

    double m_frequency { 1 };

private:
    OrderMaker<LBasicBlock> m_blockOrder;
};

template<typename... Params>
inline LValue Output::phi(LType type, ValueFromBlock value, Params... theRest)
{
    LValue phiNode = phi(type);
    addIncomingToPhi(phiNode, value, theRest...);
    return phiNode;
}

template<typename VectorType>
inline LValue Output::phi(LType type, const VectorType& vector)
{
    LValue phiNode = phi(type);
    for (const ValueFromBlock& valueFromBlock : vector)
        addIncomingToPhi(phiNode, valueFromBlock);
    return phiNode;
}

template<typename... Params>
inline void Output::addIncomingToPhi(LValue phi, ValueFromBlock value, Params... theRest)
{
    addIncomingToPhi(phi, value);
    addIncomingToPhi(phi, theRest...);
}

#if COMPILER(GCC_OR_CLANG)
#pragma GCC diagnostic pop
#endif // COMPILER(GCC_OR_CLANG)

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)
