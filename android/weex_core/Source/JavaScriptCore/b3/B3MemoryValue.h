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

#pragma once

#if ENABLE(B3_JIT)

#include "AirArg.h"
#include "B3Bank.h"
#include "B3HeapRange.h"
#include "B3Value.h"

namespace JSC { namespace B3 {

class JS_EXPORT_PRIVATE MemoryValue : public Value {
public:
    static bool accepts(Kind kind)
    {
        return isMemoryAccess(kind.opcode());
    }

    ~MemoryValue();

    int32_t offset() const { return m_offset; }
    void setOffset(int32_t offset) { m_offset = offset; }
    
    // You don't have to worry about using legal offsets unless you've entered quirks mode.
    bool isLegalOffset(int32_t offset) const;
    bool isLegalOffset(int64_t offset) const;
    
    // A necessary consequence of MemoryValue having an offset is that it participates in instruction
    // selection. This tells you if this will get lowered to something that requires an offsetless
    // address.
    bool requiresSimpleAddr() const;

    const HeapRange& range() const { return m_range; }
    void setRange(const HeapRange& range) { m_range = range; }
    
    // This is an alias for range.
    const HeapRange& accessRange() const { return range(); }
    void setAccessRange(const HeapRange& range) { setRange(range); }
    
    const HeapRange& fenceRange() const { return m_fenceRange; }
    void setFenceRange(const HeapRange& range) { m_fenceRange = range; }

    bool isStore() const { return B3::isStore(opcode()); }
    bool isLoad() const { return B3::isLoad(opcode()); }

    bool hasFence() const { return !!fenceRange(); }
    bool isExotic() const { return hasFence() || isAtomic(opcode()); }

    Type accessType() const;
    Bank accessBank() const;
    size_t accessByteSize() const;
    
    Width accessWidth() const;

    bool isCanonicalWidth() const { return B3::isCanonicalWidth(accessWidth()); }

protected:
    void dumpMeta(CommaPrinter&, PrintStream&) const override;

    Value* cloneImpl() const override;

    template<typename... Arguments>
    MemoryValue(CheckedOpcodeTag, Kind kind, Type type, Origin origin, int32_t offset, HeapRange range, HeapRange fenceRange, Arguments... arguments)
        : Value(CheckedOpcode, kind, type, origin, arguments...)
        , m_offset(offset)
        , m_range(range)
        , m_fenceRange(fenceRange)
    {
    }
    
private:
    friend class Procedure;
    
    // Use this form for Load (but not Load8Z, Load8S, or any of the Loads that have a suffix that
    // describes the returned type).
    MemoryValue(Kind, Type, Origin, Value* pointer, int32_t offset = 0, HeapRange range = HeapRange::top(), HeapRange fenceRange = HeapRange());

    // Use this form for loads where the return type is implied.
    MemoryValue(Kind, Origin, Value* pointer, int32_t offset = 0, HeapRange range = HeapRange::top(), HeapRange fenceRange = HeapRange());

    // Use this form for stores.
    MemoryValue(Kind, Origin, Value* value, Value* pointer, int32_t offset = 0, HeapRange range = HeapRange::top(), HeapRange fenceRange = HeapRange());
    
    int32_t m_offset { 0 };
    HeapRange m_range { HeapRange::top() };
    HeapRange m_fenceRange { HeapRange() };
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
