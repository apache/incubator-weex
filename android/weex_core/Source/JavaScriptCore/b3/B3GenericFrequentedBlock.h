/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "B3FrequencyClass.h"
#include <wtf/PrintStream.h>

namespace JSC { namespace B3 {

// A frequented block is a tuple of BasicBlock* and FrequencyClass. It's usually used as a
// successor edge.

template<typename BasicBlock>
class GenericFrequentedBlock {
public:
    GenericFrequentedBlock(
        BasicBlock* block = nullptr, FrequencyClass frequency = FrequencyClass::Normal)
        : m_block(block)
        , m_frequency(frequency)
    {
    }

    bool operator==(const GenericFrequentedBlock& other) const
    {
        return m_block == other.m_block
            && m_frequency == other.m_frequency;
    }

    bool operator!=(const GenericFrequentedBlock& other) const
    {
        return !(*this == other);
    }

    explicit operator bool() const
    {
        return *this != GenericFrequentedBlock();
    }

    BasicBlock* block() const { return m_block; }
    BasicBlock*& block() { return m_block; }
    FrequencyClass frequency() const { return m_frequency; }
    FrequencyClass& frequency() { return m_frequency; }

    bool isRare() const { return frequency() == FrequencyClass::Rare; }

    void dump(PrintStream& out) const
    {
        if (frequency() != FrequencyClass::Normal)
            out.print(frequency(), ":");
        out.print(pointerDump(m_block));
    }

private:
    BasicBlock* m_block;
    FrequencyClass m_frequency;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
