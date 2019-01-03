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

#include "Opcode.h"
#include "UnlinkedCodeBlock.h"
#include <wtf/RefCountedArray.h>

namespace JSC {

class UnlinkedInstructionStream {
    WTF_MAKE_FAST_ALLOCATED;
public:
    explicit UnlinkedInstructionStream(const Vector<UnlinkedInstruction, 0, UnsafeVectorOverflow>&);

    unsigned count() const { return m_instructionCount; }
    size_t sizeInBytes() const;

    class Reader {
    public:
        explicit Reader(const UnlinkedInstructionStream&);

        const UnlinkedInstruction* next();
        bool atEnd() const { return m_index == m_stream.m_data.size(); }

    private:
        unsigned char read8();
        unsigned read32();

        const UnlinkedInstructionStream& m_stream;
        UnlinkedInstruction m_unpackedBuffer[16];
        unsigned m_index;
    };

#ifndef NDEBUG
    const RefCountedArray<UnlinkedInstruction>& unpackForDebugging() const;
#endif

private:
    friend class Reader;

#ifndef NDEBUG
    mutable RefCountedArray<UnlinkedInstruction> m_unpackedInstructionsForDebugging;
#endif

    RefCountedArray<unsigned char> m_data;
    unsigned m_instructionCount;
};

// Unlinked instructions are packed in a simple stream format.
//
// The first byte is always the opcode.
// It's followed by an opcode-dependent number of argument values.
// The first 3 bits of each value determines the format:
//
//     5-bit positive integer (1 byte total)
//     5-bit negative integer (1 byte total)
//     13-bit positive integer (2 bytes total)
//     13-bit negative integer (2 bytes total)
//     5-bit constant register index, based at 0x40000000 (1 byte total)
//     13-bit constant register index, based at 0x40000000 (2 bytes total)
//     32-bit raw value (5 bytes total)

enum PackedValueType {
    Positive5Bit = 0,
    Negative5Bit,
    Positive13Bit,
    Negative13Bit,
    ConstantRegister5Bit,
    ConstantRegister13Bit,
    Full32Bit
};

ALWAYS_INLINE UnlinkedInstructionStream::Reader::Reader(const UnlinkedInstructionStream& stream)
    : m_stream(stream)
    , m_index(0)
{
}

ALWAYS_INLINE unsigned char UnlinkedInstructionStream::Reader::read8()
{
    return m_stream.m_data.data()[m_index++];
}

ALWAYS_INLINE unsigned UnlinkedInstructionStream::Reader::read32()
{
    const unsigned char* data = &m_stream.m_data.data()[m_index];
    unsigned char type = data[0] >> 5;

    switch (type) {
    case Positive5Bit:
        m_index++;
        return data[0];
    case Negative5Bit:
        m_index++;
        return 0xffffffe0 | data[0];
    case Positive13Bit:
        m_index += 2;
        return ((data[0] & 0x1F) << 8) | data[1];
    case Negative13Bit:
        m_index += 2;
        return 0xffffe000 | ((data[0] & 0x1F) << 8) | data[1];
    case ConstantRegister5Bit:
        m_index++;
        return 0x40000000 | (data[0] & 0x1F);
    case ConstantRegister13Bit:
        m_index += 2;
        return 0x40000000 | ((data[0] & 0x1F) << 8) | data[1];
    default:
        ASSERT(type == Full32Bit);
        m_index += 5;
        return data[1] | data[2] << 8 | data[3] << 16 | data[4] << 24;
    }
}

ALWAYS_INLINE const UnlinkedInstruction* UnlinkedInstructionStream::Reader::next()
{
    m_unpackedBuffer[0].u.opcode = static_cast<OpcodeID>(read8());
    unsigned opLength = opcodeLength(m_unpackedBuffer[0].u.opcode);
    for (unsigned i = 1; i < opLength; ++i)
        m_unpackedBuffer[i].u.unsignedValue = read32();
    return m_unpackedBuffer;
}

} // namespace JSC
