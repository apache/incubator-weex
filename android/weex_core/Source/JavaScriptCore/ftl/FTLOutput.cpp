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
#include "FTLOutput.h"

#if ENABLE(FTL_JIT)

#include "B3ArgumentRegValue.h"
#include "B3BasicBlockInlines.h"
#include "B3CCallValue.h"
#include "B3Const32Value.h"
#include "B3ConstPtrValue.h"
#include "B3FenceValue.h"
#include "B3MathExtras.h"
#include "B3MemoryValue.h"
#include "B3SlotBaseValue.h"
#include "B3StackmapGenerationParams.h"
#include "B3SwitchValue.h"
#include "B3UpsilonValue.h"
#include "B3ValueInlines.h"
#include "SuperSampler.h"

namespace JSC { namespace FTL {

using namespace B3;

Output::Output(State& state)
    : m_proc(*state.proc)
{
}

Output::~Output()
{
}

void Output::initialize(AbstractHeapRepository& heaps)
{
    m_heaps = &heaps;
}

LBasicBlock Output::newBlock()
{
    LBasicBlock result = m_proc.addBlock(m_frequency);

    if (!m_nextBlock)
        m_blockOrder.append(result);
    else
        m_blockOrder.insertBefore(m_nextBlock, result);

    return result;
}

void Output::applyBlockOrder()
{
    m_proc.setBlockOrder(m_blockOrder);
}

LBasicBlock Output::appendTo(LBasicBlock block, LBasicBlock nextBlock)
{
    appendTo(block);
    return insertNewBlocksBefore(nextBlock);
}

void Output::appendTo(LBasicBlock block)
{
    m_block = block;
}

LValue Output::framePointer()
{
    return m_block->appendNew<B3::Value>(m_proc, B3::FramePointer, origin());
}

SlotBaseValue* Output::lockedStackSlot(size_t bytes)
{
    return m_block->appendNew<SlotBaseValue>(m_proc, origin(), m_proc.addStackSlot(bytes));
}

LValue Output::constBool(bool value)
{
    if (value)
        return booleanTrue;
    return booleanFalse;
}

LValue Output::constInt32(int32_t value)
{
    return m_block->appendNew<B3::Const32Value>(m_proc, origin(), value);
}

LValue Output::constInt64(int64_t value)
{
    return m_block->appendNew<B3::Const64Value>(m_proc, origin(), value);
}

LValue Output::constDouble(double value)
{
    return m_block->appendNew<B3::ConstDoubleValue>(m_proc, origin(), value);
}

LValue Output::phi(LType type)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Phi, type, origin());
}

LValue Output::add(LValue left, LValue right)
{
    if (Value* result = left->addConstant(m_proc, right)) {
        m_block->append(result);
        return result;
    }
    return m_block->appendNew<B3::Value>(m_proc, B3::Add, origin(), left, right);
}

LValue Output::sub(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Sub, origin(), left, right);
}

LValue Output::mul(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Mul, origin(), left, right);
}

LValue Output::div(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Div, origin(), left, right);
}

LValue Output::chillDiv(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, chill(B3::Div), origin(), left, right);
}

LValue Output::mod(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Mod, origin(), left, right);
}

LValue Output::chillMod(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, chill(B3::Mod), origin(), left, right);
}

LValue Output::neg(LValue value)
{
    return m_block->appendNew<Value>(m_proc, B3::Neg, origin(), value);
}

LValue Output::doubleAdd(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Add, origin(), left, right);
}

LValue Output::doubleSub(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Sub, origin(), left, right);
}

LValue Output::doubleMul(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Mul, origin(), left, right);
}

LValue Output::doubleDiv(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Div, origin(), left, right);
}

LValue Output::doubleMod(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Mod, origin(), left, right);
}

LValue Output::bitAnd(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::BitAnd, origin(), left, right);
}

LValue Output::bitOr(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::BitOr, origin(), left, right);
}

LValue Output::bitXor(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::BitXor, origin(), left, right);
}

LValue Output::shl(LValue left, LValue right)
{
    right = castToInt32(right);
    if (Value* result = left->shlConstant(m_proc, right)) {
        m_block->append(result);
        return result;
    }
    return m_block->appendNew<B3::Value>(m_proc, B3::Shl, origin(), left, right);
}

LValue Output::aShr(LValue left, LValue right)
{
    right = castToInt32(right);
    if (Value* result = left->sShrConstant(m_proc, right)) {
        m_block->append(result);
        return result;
    }
    return m_block->appendNew<B3::Value>(m_proc, B3::SShr, origin(), left, right);
}

LValue Output::lShr(LValue left, LValue right)
{
    right = castToInt32(right);
    if (Value* result = left->zShrConstant(m_proc, right)) {
        m_block->append(result);
        return result;
    }
    return m_block->appendNew<B3::Value>(m_proc, B3::ZShr, origin(), left, right);
}

LValue Output::bitNot(LValue value)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::BitXor, origin(),
        value,
        m_block->appendIntConstant(m_proc, origin(), value->type(), -1));
}

LValue Output::logicalNot(LValue value)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Equal, origin(), value, int32Zero);
}

LValue Output::ctlz32(LValue operand)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Clz, origin(), operand);
}

LValue Output::doubleAbs(LValue value)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Abs, origin(), value);
}

LValue Output::doubleCeil(LValue operand)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Ceil, origin(), operand);
}

LValue Output::doubleFloor(LValue operand)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Floor, origin(), operand);
}

LValue Output::doubleTrunc(LValue value)
{
    if (MacroAssembler::supportsFloatingPointRounding()) {
        PatchpointValue* result = patchpoint(Double);
        result->append(value, ValueRep::SomeRegister);
        result->setGenerator(
            [] (CCallHelpers& jit, const StackmapGenerationParams& params) {
                jit.roundTowardZeroDouble(params[1].fpr(), params[0].fpr());
            });
        result->effects = Effects::none();
        return result;
    }
    double (*truncDouble)(double) = trunc;
    return callWithoutSideEffects(Double, truncDouble, value);
}

LValue Output::doubleSin(LValue value)
{
    double (*sinDouble)(double) = sin;
    return callWithoutSideEffects(B3::Double, sinDouble, value);
}

LValue Output::doubleCos(LValue value)
{
    double (*cosDouble)(double) = cos;
    return callWithoutSideEffects(B3::Double, cosDouble, value);
}

LValue Output::doubleTan(LValue value)
{
    double (*tanDouble)(double) = tan;
    return callWithoutSideEffects(B3::Double, tanDouble, value);
}

LValue Output::doublePow(LValue xOperand, LValue yOperand)
{
    double (*powDouble)(double, double) = pow;
    return callWithoutSideEffects(B3::Double, powDouble, xOperand, yOperand);
}

LValue Output::doublePowi(LValue x, LValue y)
{
    // FIXME: powDoubleInt32() should be inlined here since Output knows about block layout and
    // should be involved in any operation that creates blocks.
    // https://bugs.webkit.org/show_bug.cgi?id=152223
    auto result = powDoubleInt32(m_proc, m_block, origin(), x, y);
    m_block = result.first;
    return result.second;
}

LValue Output::doubleSqrt(LValue value)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Sqrt, origin(), value);
}

LValue Output::doubleLog(LValue value)
{
    double (*logDouble)(double) = log;
    return callWithoutSideEffects(B3::Double, logDouble, value);
}

LValue Output::doubleToInt(LValue value)
{
    PatchpointValue* result = patchpoint(Int32);
    result->append(value, ValueRep::SomeRegister);
    result->setGenerator(
        [] (CCallHelpers& jit, const StackmapGenerationParams& params) {
            jit.truncateDoubleToInt32(params[1].fpr(), params[0].gpr());
        });
    result->effects = Effects::none();
    return result;
}

LValue Output::doubleToUInt(LValue value)
{
    PatchpointValue* result = patchpoint(Int32);
    result->append(value, ValueRep::SomeRegister);
    result->setGenerator(
        [] (CCallHelpers& jit, const StackmapGenerationParams& params) {
            jit.truncateDoubleToUint32(params[1].fpr(), params[0].gpr());
        });
    result->effects = Effects::none();
    return result;
}

LValue Output::signExt32To64(LValue value)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::SExt32, origin(), value);
}

LValue Output::signExt32ToPtr(LValue value)
{
    return signExt32To64(value);
}

LValue Output::zeroExt(LValue value, LType type)
{
    if (value->type() == type)
        return value;
    if (value->hasInt32())
        return m_block->appendIntConstant(m_proc, origin(), Int64, static_cast<uint64_t>(static_cast<uint32_t>(value->asInt32())));
    return m_block->appendNew<B3::Value>(m_proc, B3::ZExt32, origin(), value);
}

LValue Output::intToDouble(LValue value)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::IToD, origin(), value);
}

LValue Output::unsignedToDouble(LValue value)
{
    return intToDouble(zeroExt(value, Int64));
}

LValue Output::castToInt32(LValue value)
{
    if (value->type() == Int32)
        return value;
    if (value->hasInt64())
        return constInt32(static_cast<int32_t>(value->asInt64()));
    return m_block->appendNew<B3::Value>(m_proc, B3::Trunc, origin(), value);
}

LValue Output::doubleToFloat(LValue value)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::DoubleToFloat, origin(), value);
}

LValue Output::floatToDouble(LValue value)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::FloatToDouble, origin(), value);
}

LValue Output::load(TypedPointer pointer, LType type)
{
    LValue load = m_block->appendNew<MemoryValue>(m_proc, Load, type, origin(), pointer.value());
    m_heaps->decorateMemory(pointer.heap(), load);
    return load;
}

LValue Output::load8SignExt32(TypedPointer pointer)
{
    LValue load = m_block->appendNew<MemoryValue>(m_proc, Load8S, Int32, origin(), pointer.value());
    m_heaps->decorateMemory(pointer.heap(), load);
    return load;
}

LValue Output::load8ZeroExt32(TypedPointer pointer)
{
    LValue load = m_block->appendNew<MemoryValue>(m_proc, Load8Z, Int32, origin(), pointer.value());
    m_heaps->decorateMemory(pointer.heap(), load);
    return load;
}

LValue Output::load16SignExt32(TypedPointer pointer)
{
    LValue load = m_block->appendNew<MemoryValue>(m_proc, Load16S, Int32, origin(), pointer.value());
    m_heaps->decorateMemory(pointer.heap(), load);
    return load;
}

LValue Output::load16ZeroExt32(TypedPointer pointer)
{
    LValue load = m_block->appendNew<MemoryValue>(m_proc, Load16Z, Int32, origin(), pointer.value());
    m_heaps->decorateMemory(pointer.heap(), load);
    return load;
}

void Output::store(LValue value, TypedPointer pointer)
{
    LValue store = m_block->appendNew<MemoryValue>(m_proc, Store, origin(), value, pointer.value());
    m_heaps->decorateMemory(pointer.heap(), store);
}

FenceValue* Output::fence(const AbstractHeap* read, const AbstractHeap* write)
{
    FenceValue* result = m_block->appendNew<FenceValue>(m_proc, origin());
    m_heaps->decorateFenceRead(read, result);
    m_heaps->decorateFenceWrite(write, result);
    return result;
}

void Output::store32As8(LValue value, TypedPointer pointer)
{
    LValue store = m_block->appendNew<MemoryValue>(m_proc, Store8, origin(), value, pointer.value());
    m_heaps->decorateMemory(pointer.heap(), store);
}

void Output::store32As16(LValue value, TypedPointer pointer)
{
    LValue store = m_block->appendNew<MemoryValue>(m_proc, Store16, origin(), value, pointer.value());
    m_heaps->decorateMemory(pointer.heap(), store);
}

LValue Output::baseIndex(LValue base, LValue index, Scale scale, ptrdiff_t offset)
{
    LValue accumulatedOffset;
        
    switch (scale) {
    case ScaleOne:
        accumulatedOffset = index;
        break;
    case ScaleTwo:
        accumulatedOffset = shl(index, intPtrOne);
        break;
    case ScaleFour:
        accumulatedOffset = shl(index, intPtrTwo);
        break;
    case ScaleEight:
    case ScalePtr:
        accumulatedOffset = shl(index, intPtrThree);
        break;
    }
        
    if (offset)
        accumulatedOffset = add(accumulatedOffset, constIntPtr(offset));
        
    return add(base, accumulatedOffset);
}

LValue Output::equal(LValue left, LValue right)
{
    TriState result = left->equalConstant(right);
    if (result != MixedTriState)
        return constBool(result == TrueTriState);
    return m_block->appendNew<B3::Value>(m_proc, B3::Equal, origin(), left, right);
}

LValue Output::notEqual(LValue left, LValue right)
{
    TriState result = left->notEqualConstant(right);
    if (result != MixedTriState)
        return constBool(result == TrueTriState);
    return m_block->appendNew<B3::Value>(m_proc, B3::NotEqual, origin(), left, right);
}

LValue Output::above(LValue left, LValue right)
{
    TriState result = left->aboveConstant(right);
    if (result != MixedTriState)
        return constBool(result == TrueTriState);
    return m_block->appendNew<B3::Value>(m_proc, B3::Above, origin(), left, right);
}

LValue Output::aboveOrEqual(LValue left, LValue right)
{
    TriState result = left->aboveEqualConstant(right);
    if (result != MixedTriState)
        return constBool(result == TrueTriState);
    return m_block->appendNew<B3::Value>(m_proc, B3::AboveEqual, origin(), left, right);
}

LValue Output::below(LValue left, LValue right)
{
    TriState result = left->belowConstant(right);
    if (result != MixedTriState)
        return constBool(result == TrueTriState);
    return m_block->appendNew<B3::Value>(m_proc, B3::Below, origin(), left, right);
}

LValue Output::belowOrEqual(LValue left, LValue right)
{
    TriState result = left->belowEqualConstant(right);
    if (result != MixedTriState)
        return constBool(result == TrueTriState);
    return m_block->appendNew<B3::Value>(m_proc, B3::BelowEqual, origin(), left, right);
}

LValue Output::greaterThan(LValue left, LValue right)
{
    TriState result = left->greaterThanConstant(right);
    if (result != MixedTriState)
        return constBool(result == TrueTriState);
    return m_block->appendNew<B3::Value>(m_proc, B3::GreaterThan, origin(), left, right);
}

LValue Output::greaterThanOrEqual(LValue left, LValue right)
{
    TriState result = left->greaterEqualConstant(right);
    if (result != MixedTriState)
        return constBool(result == TrueTriState);
    return m_block->appendNew<B3::Value>(m_proc, B3::GreaterEqual, origin(), left, right);
}

LValue Output::lessThan(LValue left, LValue right)
{
    TriState result = left->lessThanConstant(right);
    if (result != MixedTriState)
        return constBool(result == TrueTriState);
    return m_block->appendNew<B3::Value>(m_proc, B3::LessThan, origin(), left, right);
}

LValue Output::lessThanOrEqual(LValue left, LValue right)
{
    TriState result = left->lessEqualConstant(right);
    if (result != MixedTriState)
        return constBool(result == TrueTriState);
    return m_block->appendNew<B3::Value>(m_proc, B3::LessEqual, origin(), left, right);
}

LValue Output::doubleEqual(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Equal, origin(), left, right);
}

LValue Output::doubleEqualOrUnordered(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::EqualOrUnordered, origin(), left, right);
}

LValue Output::doubleNotEqualOrUnordered(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::NotEqual, origin(), left, right);
}

LValue Output::doubleLessThan(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::LessThan, origin(), left, right);
}

LValue Output::doubleLessThanOrEqual(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::LessEqual, origin(), left, right);
}

LValue Output::doubleGreaterThan(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::GreaterThan, origin(), left, right);
}

LValue Output::doubleGreaterThanOrEqual(LValue left, LValue right)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::GreaterEqual, origin(), left, right);
}

LValue Output::doubleNotEqualAndOrdered(LValue left, LValue right)
{
    return logicalNot(doubleEqualOrUnordered(left, right));
}

LValue Output::doubleLessThanOrUnordered(LValue left, LValue right)
{
    return logicalNot(doubleGreaterThanOrEqual(left, right));
}

LValue Output::doubleLessThanOrEqualOrUnordered(LValue left, LValue right)
{
    return logicalNot(doubleGreaterThan(left, right));
}

LValue Output::doubleGreaterThanOrUnordered(LValue left, LValue right)
{
    return logicalNot(doubleLessThanOrEqual(left, right));
}

LValue Output::doubleGreaterThanOrEqualOrUnordered(LValue left, LValue right)
{
    return logicalNot(doubleLessThan(left, right));
}

LValue Output::isZero32(LValue value)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Equal, origin(), value, int32Zero);
}

LValue Output::notZero32(LValue value)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::NotEqual, origin(), value, int32Zero);
}

LValue Output::isZero64(LValue value)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::Equal, origin(), value, int64Zero);
}

LValue Output::notZero64(LValue value)
{
    return m_block->appendNew<B3::Value>(m_proc, B3::NotEqual, origin(), value, int64Zero);
}

LValue Output::select(LValue value, LValue taken, LValue notTaken)
{
    if (value->hasInt32()) {
        if (value->asInt32())
            return taken;
        else
            return notTaken;
    }
    return m_block->appendNew<B3::Value>(m_proc, B3::Select, origin(), value, taken, notTaken);
}

void Output::jump(LBasicBlock destination)
{
    m_block->appendNewControlValue(m_proc, B3::Jump, origin(), B3::FrequentedBlock(destination));
}

void Output::branch(LValue condition, LBasicBlock taken, Weight takenWeight, LBasicBlock notTaken, Weight notTakenWeight)
{
    m_block->appendNewControlValue(
        m_proc, B3::Branch, origin(), condition,
        FrequentedBlock(taken, takenWeight.frequencyClass()),
        FrequentedBlock(notTaken, notTakenWeight.frequencyClass()));
}

void Output::check(LValue condition, WeightedTarget taken, Weight notTakenWeight)
{
    LBasicBlock continuation = newBlock();
    branch(condition, taken, WeightedTarget(continuation, notTakenWeight));
    appendTo(continuation);
}

void Output::check(LValue condition, WeightedTarget taken)
{
    check(condition, taken, taken.weight().inverse());
}

void Output::ret(LValue value)
{
    m_block->appendNewControlValue(m_proc, B3::Return, origin(), value);
}

void Output::unreachable()
{
    m_block->appendNewControlValue(m_proc, B3::Oops, origin());
}

void Output::appendSuccessor(WeightedTarget target)
{
    m_block->appendSuccessor(target.frequentedBlock());
}

CheckValue* Output::speculate(LValue value)
{
    return m_block->appendNew<B3::CheckValue>(m_proc, B3::Check, origin(), value);
}

CheckValue* Output::speculateAdd(LValue left, LValue right)
{
    return m_block->appendNew<B3::CheckValue>(m_proc, B3::CheckAdd, origin(), left, right);
}

CheckValue* Output::speculateSub(LValue left, LValue right)
{
    return m_block->appendNew<B3::CheckValue>(m_proc, B3::CheckSub, origin(), left, right);
}

CheckValue* Output::speculateMul(LValue left, LValue right)
{
    return m_block->appendNew<B3::CheckValue>(m_proc, B3::CheckMul, origin(), left, right);
}

PatchpointValue* Output::patchpoint(LType type)
{
    return m_block->appendNew<B3::PatchpointValue>(m_proc, type, origin());
}

void Output::trap()
{
    m_block->appendNewControlValue(m_proc, B3::Oops, origin());
}

ValueFromBlock Output::anchor(LValue value)
{
    B3::UpsilonValue* upsilon = m_block->appendNew<B3::UpsilonValue>(m_proc, origin(), value);
    return ValueFromBlock(upsilon, m_block);
}

LValue Output::bitCast(LValue value, LType type)
{
    ASSERT_UNUSED(type, type == Int64 || type == Double);
    return m_block->appendNew<B3::Value>(m_proc, B3::BitwiseCast, origin(), value);
}

LValue Output::fround(LValue doubleValue)
{
    return floatToDouble(doubleToFloat(doubleValue));
}

LValue Output::load(TypedPointer pointer, LoadType type)
{
    switch (type) {
    case Load8SignExt32:
        return load8SignExt32(pointer);
    case Load8ZeroExt32:
        return load8ZeroExt32(pointer);
    case Load16SignExt32:
        return load8SignExt32(pointer);
    case Load16ZeroExt32:
        return load8ZeroExt32(pointer);
    case Load32:
        return load32(pointer);
    case Load64:
        return load64(pointer);
    case LoadPtr:
        return loadPtr(pointer);
    case LoadFloat:
        return loadFloat(pointer);
    case LoadDouble:
        return loadDouble(pointer);
    }
    RELEASE_ASSERT_NOT_REACHED();
    return nullptr;
}

void Output::store(LValue value, TypedPointer pointer, StoreType type)
{
    switch (type) {
    case Store32As8:
        store32As8(value, pointer);
        return;
    case Store32As16:
        store32As16(value, pointer);
        return;
    case Store32:
        store32(value, pointer);
        return;
    case Store64:
        store64(value, pointer);
        return;
    case StorePtr:
        storePtr(value, pointer);
        return;
    case StoreFloat:
        storeFloat(value, pointer);
        return;
    case StoreDouble:
        storeDouble(value, pointer);
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

TypedPointer Output::absolute(const void* address)
{
    return TypedPointer(m_heaps->absolute[address], constIntPtr(address));
}

void Output::incrementSuperSamplerCount()
{
    TypedPointer counter = absolute(bitwise_cast<void*>(&g_superSamplerCount));
    store32(add(load32(counter), int32One), counter);
}

void Output::decrementSuperSamplerCount()
{
    TypedPointer counter = absolute(bitwise_cast<void*>(&g_superSamplerCount));
    store32(sub(load32(counter), int32One), counter);
}

void Output::addIncomingToPhi(LValue phi, ValueFromBlock value)
{
    if (value)
        value.value()->as<B3::UpsilonValue>()->setPhi(phi);
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)

