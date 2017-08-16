/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "B3ValueKey.h"

#if ENABLE(B3_JIT)

#include "B3ArgumentRegValue.h"
#include "B3ProcedureInlines.h"
#include "B3SlotBaseValue.h"
#include "B3ValueInlines.h"
#include "B3ValueKeyInlines.h"

namespace JSC { namespace B3 {

ValueKey ValueKey::intConstant(Type type, int64_t value)
{
    switch (type) {
    case Int32:
        return ValueKey(Const32, Int32, value);
    case Int64:
        return ValueKey(Const64, Int64, value);
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return ValueKey();
    }
}

void ValueKey::dump(PrintStream& out) const
{
    out.print(m_type, " ", m_kind, "(", u.indices[0], ", ", u.indices[1], ", ", u.indices[2], ")");
}

Value* ValueKey::materialize(Procedure& proc, Origin origin) const
{
    switch (opcode()) {
    case FramePointer:
        return proc.add<Value>(kind(), type(), origin);
    case Identity:
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
        return proc.add<Value>(kind(), type(), origin, child(proc, 0));
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
        return proc.add<Value>(kind(), type(), origin, child(proc, 0), child(proc, 1));
    case Select:
        return proc.add<Value>(kind(), type(), origin, child(proc, 0), child(proc, 1), child(proc, 2));
    case Const32:
        return proc.add<Const32Value>(origin, static_cast<int32_t>(value()));
    case Const64:
        return proc.add<Const64Value>(origin, value());
    case ConstDouble:
        return proc.add<ConstDoubleValue>(origin, doubleValue());
    case ConstFloat:
        return proc.add<ConstFloatValue>(origin, floatValue());
    case ArgumentReg:
        return proc.add<ArgumentRegValue>(origin, Reg::fromIndex(static_cast<unsigned>(value())));
    case SlotBase:
        return proc.add<SlotBaseValue>(origin, proc.stackSlots()[value()]);
    default:
        return nullptr;
    }
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

