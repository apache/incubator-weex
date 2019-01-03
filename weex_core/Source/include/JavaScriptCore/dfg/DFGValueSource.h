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
