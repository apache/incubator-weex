/*
 * Copyright (C) 2011, 2013-2015 Apple Inc. All rights reserved.
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

#if ENABLE(DFG_JIT)

#include "DFGCommon.h"
#include "DFGFlushFormat.h"
#include "DFGMinifiedID.h"
#include "DataFormat.h"
#include "SpeculatedType.h"
#include "ValueRecovery.h"

namespace JSC { namespace DFG {

enum ValueSourceKind {
    SourceNotSet,
    ValueInJSStack,
    Int32InJSStack,
    Int52InJSStack,
    CellInJSStack,
    BooleanInJSStack,
    DoubleInJSStack,
    SourceIsDead,
    HaveNode
};

static inline ValueSourceKind dataFormatToValueSourceKind(DataFormat dataFormat)
{
    switch (dataFormat) {
    case DataFormatInt32:
        return Int32InJSStack;
    case DataFormatInt52:
        return Int52InJSStack;
    case DataFormatDouble:
        return DoubleInJSStack;
    case DataFormatBoolean:
        return BooleanInJSStack;
    case DataFormatCell:
        return CellInJSStack;
    case DataFormatDead:
        return SourceIsDead;
    default:
        RELEASE_ASSERT(dataFormat & DataFormatJS);
        return ValueInJSStack;
    }
}

static inline DataFormat valueSourceKindToDataFormat(ValueSourceKind kind)
{
    switch (kind) {
    case ValueInJSStack:
        return DataFormatJS;
    case Int32InJSStack:
        return DataFormatInt32;
    case Int52InJSStack:
        return DataFormatInt52;
    case CellInJSStack:
        return DataFormatCell;
    case BooleanInJSStack:
        return DataFormatBoolean;
    case DoubleInJSStack:
        return DataFormatDouble;
    case SourceIsDead:
        return DataFormatDead;
    default:
        return DataFormatNone;
    }
}

static inline bool isInJSStack(ValueSourceKind kind)
{
    DataFormat format = valueSourceKindToDataFormat(kind);
    return format != DataFormatNone && format < DataFormatOSRMarker;
}

// Can this value be recovered without having to look at register allocation state or
// DFG node liveness?
static inline bool isTriviallyRecoverable(ValueSourceKind kind)
{
    return valueSourceKindToDataFormat(kind) != DataFormatNone;
}

class ValueSource {
public:
    ValueSource()
        : m_kind(SourceNotSet)
    {
    }
    
    explicit ValueSource(ValueSourceKind valueSourceKind)
        : m_kind(valueSourceKind)
    {
        ASSERT(kind() == SourceIsDead);
    }
    
    explicit ValueSource(MinifiedID id)
        : m_kind(HaveNode)
        , m_value(id.bits())
    {
        ASSERT(!!id);
        ASSERT(kind() == HaveNode);
    }
    
    ValueSource(ValueSourceKind valueSourceKind, VirtualRegister where)
        : m_kind(valueSourceKind)
        , m_value(static_cast<intptr_t>(where.offset()))
    {
        ASSERT(kind() != SourceNotSet);
        ASSERT(kind() != HaveNode);
    }
    
    static ValueSource forFlushFormat(VirtualRegister where, FlushFormat format)
    {
        switch (format) {
        case DeadFlush:
        case ConflictingFlush:
            return ValueSource(SourceIsDead);
        case FlushedJSValue:
            return ValueSource(ValueInJSStack, where);
        case FlushedDouble:
            return ValueSource(DoubleInJSStack, where);
        case FlushedInt32:
            return ValueSource(Int32InJSStack, where);
        case FlushedInt52:
            return ValueSource(Int52InJSStack, where);
        case FlushedCell:
            return ValueSource(CellInJSStack, where);
        case FlushedBoolean:
            return ValueSource(BooleanInJSStack, where);
        }
        RELEASE_ASSERT_NOT_REACHED();
        return ValueSource();
    }
    
    static ValueSource forDataFormat(VirtualRegister where, DataFormat dataFormat)
    {
        return ValueSource(dataFormatToValueSourceKind(dataFormat), where);
    }
    
    bool isSet() const
    {
        return kind() != SourceNotSet;
    }
    
    bool operator!() const { return !isSet(); }
    
    ValueSourceKind kind() const
    {
        return m_kind;
    }
    
    bool isInJSStack() const { return JSC::DFG::isInJSStack(kind()); }
    bool isTriviallyRecoverable() const { return JSC::DFG::isTriviallyRecoverable(kind()); }
    
    DataFormat dataFormat() const
    {
        return valueSourceKindToDataFormat(kind());
    }
    
    ValueRecovery valueRecovery() const
    {
        ASSERT(isTriviallyRecoverable());
        switch (kind()) {
        case SourceIsDead:
            return ValueRecovery::constant(jsUndefined());
            
        default:
            return ValueRecovery::displacedInJSStack(virtualRegister(), dataFormat());
        }
    }
    
    MinifiedID id() const
    {
        ASSERT(kind() == HaveNode);
        return MinifiedID::fromBits(m_value);
    }
    
    VirtualRegister virtualRegister() const
    {
        ASSERT(isInJSStack());
        return VirtualRegister(m_value);
    }
    
    void dump(PrintStream&) const;
    void dumpInContext(PrintStream&, DumpContext*) const;
    
private:
    ValueSourceKind m_kind;
    uintptr_t m_value;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
