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
#include "B3Value.h"

#if ENABLE(B3_JIT)

#include "B3ArgumentRegValue.h"
#include "B3AtomicValue.h"
#include "B3BasicBlockInlines.h"
#include "B3BottomProvider.h"
#include "B3CCallValue.h"
#include "B3FenceValue.h"
#include "B3MemoryValue.h"
#include "B3OriginDump.h"
#include "B3ProcedureInlines.h"
#include "B3SlotBaseValue.h"
#include "B3StackSlot.h"
#include "B3UpsilonValue.h"
#include "B3ValueInlines.h"
#include "B3ValueKeyInlines.h"
#include "B3VariableValue.h"
#include <wtf/CommaPrinter.h>
#include <wtf/ListDump.h>
#include <wtf/StringPrintStream.h>

namespace JSC { namespace B3 {

const char* const Value::dumpPrefix = "@";

Value::~Value()
{
}

void Value::replaceWithIdentity(Value* value)
{
    // This is a bit crazy. It does an in-place replacement of whatever Value subclass this is with
    // a plain Identity Value. We first collect all of the information we need, then we destruct the
    // previous value in place, and then we construct the Identity Value in place.

    ASSERT(m_type == value->m_type);

    if (m_type == Void) {
        replaceWithNopIgnoringType();
        return;
    }

    unsigned index = m_index;
    Type type = m_type;
    Origin origin = m_origin;
    BasicBlock* owner = this->owner;

    RELEASE_ASSERT(type == value->type());

    this->~Value();

    new (this) Value(Identity, type, origin, value);

    this->owner = owner;
    this->m_index = index;
}

void Value::replaceWithBottom(InsertionSet& insertionSet, size_t index)
{
    replaceWithBottom(BottomProvider(insertionSet, index));
}

void Value::replaceWithNop()
{
    RELEASE_ASSERT(m_type == Void);
    replaceWithNopIgnoringType();
}

void Value::replaceWithNopIgnoringType()
{
    unsigned index = m_index;
    Origin origin = m_origin;
    BasicBlock* owner = this->owner;

    this->~Value();

    new (this) Value(Nop, Void, origin);

    this->owner = owner;
    this->m_index = index;
}

void Value::replaceWithPhi()
{
    if (m_type == Void) {
        replaceWithNop();
        return;
    }
    
    unsigned index = m_index;
    Origin origin = m_origin;
    BasicBlock* owner = this->owner;
    Type type = m_type;

    this->~Value();

    new (this) Value(Phi, type, origin);

    this->owner = owner;
    this->m_index = index;
}

void Value::replaceWithJump(BasicBlock* owner, FrequentedBlock target)
{
    RELEASE_ASSERT(owner->last() == this);
    
    unsigned index = m_index;
    Origin origin = m_origin;
    
    this->~Value();
    
    new (this) Value(Jump, Void, origin);
    
    this->owner = owner;
    this->m_index = index;
    
    owner->setSuccessors(target);
}

void Value::replaceWithOops(BasicBlock* owner)
{
    RELEASE_ASSERT(owner->last() == this);
    
    unsigned index = m_index;
    Origin origin = m_origin;
    
    this->~Value();
    
    new (this) Value(Oops, Void, origin);
    
    this->owner = owner;
    this->m_index = index;
    
    owner->clearSuccessors();
}

void Value::replaceWithJump(FrequentedBlock target)
{
    replaceWithJump(owner, target);
}

void Value::replaceWithOops()
{
    replaceWithOops(owner);
}

void Value::dump(PrintStream& out) const
{
    bool isConstant = false;

    switch (opcode()) {
    case Const32:
        out.print("$", asInt32(), "(");
        isConstant = true;
        break;
    case Const64:
        out.print("$", asInt64(), "(");
        isConstant = true;
        break;
    case ConstFloat:
        out.print("$", asFloat(), "(");
        isConstant = true;
        break;
    case ConstDouble:
        out.print("$", asDouble(), "(");
        isConstant = true;
        break;
    default:
        break;
    }
    
    out.print(dumpPrefix, m_index);

    if (isConstant)
        out.print(")");
}

Value* Value::cloneImpl() const
{
    return new Value(*this);
}

void Value::dumpChildren(CommaPrinter& comma, PrintStream& out) const
{
    for (Value* child : children())
        out.print(comma, pointerDump(child));
}

void Value::deepDump(const Procedure* proc, PrintStream& out) const
{
    out.print(m_type, " ", dumpPrefix, m_index, " = ", m_kind);

    out.print("(");
    CommaPrinter comma;
    dumpChildren(comma, out);

    if (m_origin)
        out.print(comma, OriginDump(proc, m_origin));

    dumpMeta(comma, out);

    {
        CString string = toCString(effects());
        if (string.length())
            out.print(comma, string);
    }

    out.print(")");
}

void Value::dumpSuccessors(const BasicBlock* block, PrintStream& out) const
{
    // Note that this must not crash if we have the wrong number of successors, since someone
    // debugging a number-of-successors bug will probably want to dump IR!
    
    if (opcode() == Branch && block->numSuccessors() == 2) {
        out.print("Then:", block->taken(), ", Else:", block->notTaken());
        return;
    }
    
    out.print(listDump(block->successors()));
}

Value* Value::negConstant(Procedure&) const
{
    return nullptr;
}

Value* Value::addConstant(Procedure&, int32_t) const
{
    return nullptr;
}

Value* Value::addConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::subConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::mulConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::checkAddConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::checkSubConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::checkMulConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::checkNegConstant(Procedure&) const
{
    return nullptr;
}

Value* Value::divConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::uDivConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::modConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::uModConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::bitAndConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::bitOrConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::bitXorConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::shlConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::sShrConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::zShrConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::rotRConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::rotLConstant(Procedure&, const Value*) const
{
    return nullptr;
}

Value* Value::bitwiseCastConstant(Procedure&) const
{
    return nullptr;
}

Value* Value::iToDConstant(Procedure&) const
{
    return nullptr;
}

Value* Value::iToFConstant(Procedure&) const
{
    return nullptr;
}

Value* Value::doubleToFloatConstant(Procedure&) const
{
    return nullptr;
}

Value* Value::floatToDoubleConstant(Procedure&) const
{
    return nullptr;
}

Value* Value::absConstant(Procedure&) const
{
    return nullptr;
}

Value* Value::ceilConstant(Procedure&) const
{
    return nullptr;
}

Value* Value::floorConstant(Procedure&) const
{
    return nullptr;
}

Value* Value::sqrtConstant(Procedure&) const
{
    return nullptr;
}

TriState Value::equalConstant(const Value*) const
{
    return MixedTriState;
}

TriState Value::notEqualConstant(const Value*) const
{
    return MixedTriState;
}

TriState Value::lessThanConstant(const Value*) const
{
    return MixedTriState;
}

TriState Value::greaterThanConstant(const Value*) const
{
    return MixedTriState;
}

TriState Value::lessEqualConstant(const Value*) const
{
    return MixedTriState;
}

TriState Value::greaterEqualConstant(const Value*) const
{
    return MixedTriState;
}

TriState Value::aboveConstant(const Value*) const
{
    return MixedTriState;
}

TriState Value::belowConstant(const Value*) const
{
    return MixedTriState;
}

TriState Value::aboveEqualConstant(const Value*) const
{
    return MixedTriState;
}

TriState Value::belowEqualConstant(const Value*) const
{
    return MixedTriState;
}

TriState Value::equalOrUnorderedConstant(const Value*) const
{
    return MixedTriState;
}

Value* Value::invertedCompare(Procedure& proc) const
{
    if (!numChildren())
        return nullptr;
    if (std::optional<Opcode> invertedOpcode = B3::invertedCompare(opcode(), child(0)->type())) {
        ASSERT(!kind().hasExtraBits());
        return proc.add<Value>(*invertedOpcode, type(), origin(), children());
    }
    return nullptr;
}

bool Value::isRounded() const
{
    ASSERT(isFloat(type()));
    switch (opcode()) {
    case Floor:
    case Ceil:
    case IToD:
    case IToF:
        return true;

    case ConstDouble: {
        double value = asDouble();
        return std::isfinite(value) && value == ceil(value);
    }

    case ConstFloat: {
        float value = asFloat();
        return std::isfinite(value) && value == ceilf(value);
    }

    default:
        return false;
    }
}

bool Value::returnsBool() const
{
    if (type() != Int32)
        return false;
    switch (opcode()) {
    case Const32:
        return asInt32() == 0 || asInt32() == 1;
    case BitAnd:
        return child(1)->isInt32(1)
            || (child(0)->returnsBool() && child(1)->hasInt() && child(1)->asInt() & 1);
    case Equal:
    case NotEqual:
    case LessThan:
    case GreaterThan:
    case LessEqual:
    case GreaterEqual:
    case Above:
    case Below:
    case AboveEqual:
    case BelowEqual:
    case EqualOrUnordered:
    case AtomicWeakCAS:
        return true;
    case Phi:
        // FIXME: We should have a story here.
        // https://bugs.webkit.org/show_bug.cgi?id=150725
        return false;
    default:
        return false;
    }
}

TriState Value::asTriState() const
{
    switch (opcode()) {
    case Const32:
        return triState(!!asInt32());
    case Const64:
        return triState(!!asInt64());
    case ConstDouble:
        // Use "!= 0" to really emphasize what this mean with respect to NaN and such.
        return triState(asDouble() != 0);
    case ConstFloat:
        return triState(asFloat() != 0.);
    default:
        return MixedTriState;
    }
}

Effects Value::effects() const
{
    Effects result;
    switch (opcode()) {
    case Nop:
    case Identity:
    case Const32:
    case Const64:
    case ConstDouble:
    case ConstFloat:
    case SlotBase:
    case ArgumentReg:
    case FramePointer:
    case Add:
    case Sub:
    case Mul:
    case Neg:
    case BitAnd:
    case BitOr:
    case BitXor:
    case Shl:
    case SShr:
    case ZShr:
    case RotR:
    case RotL:
    case Clz:
    case Abs:
    case Ceil:
    case Floor:
    case Sqrt:
    case BitwiseCast:
    case SExt8:
    case SExt16:
    case SExt32:
    case ZExt32:
    case Trunc:
    case IToD:
    case IToF:
    case FloatToDouble:
    case DoubleToFloat:
    case Equal:
    case NotEqual:
    case LessThan:
    case GreaterThan:
    case LessEqual:
    case GreaterEqual:
    case Above:
    case Below:
    case AboveEqual:
    case BelowEqual:
    case EqualOrUnordered:
    case Select:
    case Depend:
        break;
    case Div:
    case UDiv:
    case Mod:
    case UMod:
        result.controlDependent = true;
        break;
    case Load8Z:
    case Load8S:
    case Load16Z:
    case Load16S:
    case Load: {
        const MemoryValue* memory = as<MemoryValue>();
        result.reads = memory->range();
        if (memory->hasFence()) {
            result.writes = memory->fenceRange();
            result.fence = true;
        }
        result.controlDependent = true;
        break;
    }
    case Store8:
    case Store16:
    case Store: {
        const MemoryValue* memory = as<MemoryValue>();
        result.writes = memory->range();
        if (memory->hasFence()) {
            result.reads = memory->fenceRange();
            result.fence = true;
        }
        result.controlDependent = true;
        break;
    }
    case AtomicWeakCAS:
    case AtomicStrongCAS:
    case AtomicXchgAdd:
    case AtomicXchgAnd:
    case AtomicXchgOr:
    case AtomicXchgSub:
    case AtomicXchgXor:
    case AtomicXchg: {
        const AtomicValue* atomic = as<AtomicValue>();
        result.reads = atomic->range() | atomic->fenceRange();
        result.writes = atomic->range() | atomic->fenceRange();
        if (atomic->hasFence())
            result.fence = true;
        result.controlDependent = true;
        break;
    }
    case WasmAddress:
        result.readsPinned = true;
        break;
    case Fence: {
        const FenceValue* fence = as<FenceValue>();
        result.reads = fence->read;
        result.writes = fence->write;
        result.fence = true;
        break;
    }
    case CCall:
        result = as<CCallValue>()->effects;
        break;
    case Patchpoint:
        result = as<PatchpointValue>()->effects;
        break;
    case CheckAdd:
    case CheckSub:
    case CheckMul:
    case Check:
        result = Effects::forCheck();
        break;
    case WasmBoundsCheck:
        result.readsPinned = true;
        result.exitsSideways = true;
        break;
    case Upsilon:
    case Set:
        result.writesLocalState = true;
        break;
    case Phi:
    case Get:
        result.readsLocalState = true;
        break;
    case Jump:
    case Branch:
    case Switch:
    case Return:
    case Oops:
    case EntrySwitch:
        result.terminal = true;
        break;
    }
    if (traps()) {
        result.exitsSideways = true;
        result.reads = HeapRange::top();
    }
    return result;
}

ValueKey Value::key() const
{
    switch (opcode()) {
    case FramePointer:
        return ValueKey(kind(), type());
    case Identity:
    case Abs:
    case Ceil:
    case Floor:
    case Sqrt:
    case SExt8:
    case SExt16:
    case SExt32:
    case ZExt32:
    case Clz:
    case Trunc:
    case IToD:
    case IToF:
    case FloatToDouble:
    case DoubleToFloat:
    case Check:
    case BitwiseCast:
    case Neg:
    case Depend:
        return ValueKey(kind(), type(), child(0));
    case Add:
    case Sub:
    case Mul:
    case Div:
    case UDiv:
    case Mod:
    case UMod:
    case BitAnd:
    case BitOr:
    case BitXor:
    case Shl:
    case SShr:
    case ZShr:
    case RotR:
    case RotL:
    case Equal:
    case NotEqual:
    case LessThan:
    case GreaterThan:
    case Above:
    case Below:
    case AboveEqual:
    case BelowEqual:
    case EqualOrUnordered:
    case CheckAdd:
    case CheckSub:
    case CheckMul:
        return ValueKey(kind(), type(), child(0), child(1));
    case Select:
        return ValueKey(kind(), type(), child(0), child(1), child(2));
    case Const32:
        return ValueKey(Const32, type(), static_cast<int64_t>(asInt32()));
    case Const64:
        return ValueKey(Const64, type(), asInt64());
    case ConstDouble:
        return ValueKey(ConstDouble, type(), asDouble());
    case ConstFloat:
        return ValueKey(ConstFloat, type(), asFloat());
    case ArgumentReg:
        return ValueKey(
            ArgumentReg, type(),
            static_cast<int64_t>(as<ArgumentRegValue>()->argumentReg().index()));
    case SlotBase:
        return ValueKey(
            SlotBase, type(),
            static_cast<int64_t>(as<SlotBaseValue>()->slot()->index()));
    default:
        return ValueKey();
    }
}

bool Value::performSubstitution()
{
    bool result = false;
    for (Value*& child : children()) {
        while (child->opcode() == Identity) {
            result = true;
            child = child->child(0);
        }
    }
    return result;
}

bool Value::isFree() const
{
    switch (opcode()) {
    case Const32:
    case Const64:
    case ConstDouble:
    case ConstFloat:
    case Identity:
    case Nop:
        return true;
    default:
        return false;
    }
}

void Value::dumpMeta(CommaPrinter&, PrintStream&) const
{
}

Type Value::typeFor(Kind kind, Value* firstChild, Value* secondChild)
{
    switch (kind.opcode()) {
    case Identity:
    case Add:
    case Sub:
    case Mul:
    case Div:
    case UDiv:
    case Mod:
    case UMod:
    case Neg:
    case BitAnd:
    case BitOr:
    case BitXor:
    case Shl:
    case SShr:
    case ZShr:
    case RotR:
    case RotL:
    case Clz:
    case Abs:
    case Ceil:
    case Floor:
    case Sqrt:
    case CheckAdd:
    case CheckSub:
    case CheckMul:
    case Depend:
        return firstChild->type();
    case FramePointer:
        return pointerType();
    case SExt8:
    case SExt16:
    case Equal:
    case NotEqual:
    case LessThan:
    case GreaterThan:
    case LessEqual:
    case GreaterEqual:
    case Above:
    case Below:
    case AboveEqual:
    case BelowEqual:
    case EqualOrUnordered:
        return Int32;
    case Trunc:
        return firstChild->type() == Int64 ? Int32 : Float;
    case SExt32:
    case ZExt32:
        return Int64;
    case FloatToDouble:
    case IToD:
        return Double;
    case DoubleToFloat:
    case IToF:
        return Float;
    case BitwiseCast:
        switch (firstChild->type()) {
        case Int64:
            return Double;
        case Double:
            return Int64;
        case Int32:
            return Float;
        case Float:
            return Int32;
        case Void:
            ASSERT_NOT_REACHED();
        }
        return Void;
    case Nop:
    case Jump:
    case Branch:
    case Return:
    case Oops:
    case EntrySwitch:
    case WasmBoundsCheck:
        return Void;
    case Select:
        ASSERT(secondChild);
        return secondChild->type();
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }
}

void Value::badKind(Kind kind, unsigned numArgs)
{
    dataLog("Bad kind ", kind, " with ", numArgs, " args.\n");
    RELEASE_ASSERT_NOT_REACHED();
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
