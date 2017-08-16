/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "PCToCodeOriginMap.h"

#if ENABLE(JIT)

#include "B3PCToOriginMap.h"
#include "DFGNode.h"
#include "LinkBuffer.h"

#if COMPILER(MSVC)
// See https://msdn.microsoft.com/en-us/library/4wz07268.aspx
#pragma warning(disable: 4333)
#endif

namespace JSC {

namespace {

class DeltaCompressionBuilder {
public:
    DeltaCompressionBuilder(size_t maxSize)
        : m_offset(0)
        , m_maxSize(maxSize)
    {
        m_buffer = static_cast<uint8_t*>(fastMalloc(m_maxSize));
    }

    template <typename T>
    void write(T item)
    {
        RELEASE_ASSERT(m_offset + sizeof(T) <= m_maxSize);
        static const uint8_t mask = std::numeric_limits<uint8_t>::max();
        for (unsigned i = 0; i < sizeof(T); i++) {
            *(m_buffer + m_offset) = static_cast<uint8_t>(item & mask);
            item = item >> (sizeof(uint8_t) * 8);
            m_offset += 1;
        }
    }

    uint8_t* m_buffer; 
    size_t m_offset;
    size_t m_maxSize;
};

class DeltaCompresseionReader {
public:
    DeltaCompresseionReader(uint8_t* buffer, size_t size)
        : m_buffer(buffer)
        , m_size(size)
        , m_offset(0)
    { }

    template <typename T>
    T read()
    {
        RELEASE_ASSERT(m_offset + sizeof(T) <= m_size);
        T result = 0;
        for (unsigned i = 0; i < sizeof(T); i++) {
            uint8_t bitsAsInt8 = *(m_buffer + m_offset);
            T bits = static_cast<T>(bitsAsInt8);
            bits = bits << (sizeof(uint8_t) * 8 * i);
            result |= bits;
            m_offset += 1;
        }
        return result;
    }

private:
    uint8_t* m_buffer;
    size_t m_size;
    size_t m_offset;
};

} // anonymous namespace

PCToCodeOriginMapBuilder::PCToCodeOriginMapBuilder(VM& vm)
    : m_vm(vm)
    , m_shouldBuildMapping(vm.shouldBuilderPCToCodeOriginMapping())
{ }

PCToCodeOriginMapBuilder::PCToCodeOriginMapBuilder(PCToCodeOriginMapBuilder&& other)
    : m_vm(other.m_vm)
    , m_codeRanges(WTFMove(other.m_codeRanges))
    , m_shouldBuildMapping(other.m_shouldBuildMapping)
{ }

#if ENABLE(FTL_JIT)
PCToCodeOriginMapBuilder::PCToCodeOriginMapBuilder(VM& vm, B3::PCToOriginMap&& b3PCToOriginMap)
    : m_vm(vm)
    , m_shouldBuildMapping(vm.shouldBuilderPCToCodeOriginMapping())
{
    if (!m_shouldBuildMapping)
        return;

    for (const B3::PCToOriginMap::OriginRange& originRange : b3PCToOriginMap.ranges()) {
        DFG::Node* node = bitwise_cast<DFG::Node*>(originRange.origin.data());
        if (node)
            appendItem(originRange.label, node->origin.semantic);
        else
            appendItem(originRange.label, PCToCodeOriginMapBuilder::defaultCodeOrigin());
    }
}
#endif

void PCToCodeOriginMapBuilder::appendItem(MacroAssembler::Label label, const CodeOrigin& codeOrigin)
{
    if (!m_shouldBuildMapping)
        return;

    if (m_codeRanges.size()) {
        if (m_codeRanges.last().end == label)
            return;
        m_codeRanges.last().end = label;
        if (m_codeRanges.last().codeOrigin == codeOrigin || !codeOrigin)
            return;
    }

    m_codeRanges.append(CodeRange{label, label, codeOrigin});
}


static const uint8_t sentinelPCDelta = 0;
static const int8_t sentinelBytecodeDelta = 0;

PCToCodeOriginMap::PCToCodeOriginMap(PCToCodeOriginMapBuilder&& builder, LinkBuffer& linkBuffer)
{
    RELEASE_ASSERT(builder.didBuildMapping());

    if (!builder.m_codeRanges.size()) {
        m_pcRangeStart = std::numeric_limits<uintptr_t>::max();
        m_pcRangeEnd = std::numeric_limits<uintptr_t>::max();

        m_compressedPCBufferSize = 0;
        m_compressedPCs = nullptr;

        m_compressedCodeOriginsSize = 0;
        m_compressedCodeOrigins = nullptr;

        return;
    }

    // We do a final touch-up on the last range here because of how we generate the table.
    // The final range (if non empty) would be ignored if we didn't append any (arbitrary)
    // range as the last item of the vector.
    PCToCodeOriginMapBuilder::CodeRange& last = builder.m_codeRanges.last();
    if (!(last.start == last.end))
        builder.m_codeRanges.append(PCToCodeOriginMapBuilder::CodeRange{ last.end, last.end, last.codeOrigin }); // This range will never actually be found, but it ensures the real last range is found.

    DeltaCompressionBuilder pcCompressor((sizeof(uintptr_t) + sizeof(uint8_t)) * builder.m_codeRanges.size());
    void* lastPCValue = nullptr;
    auto buildPCTable = [&] (void* pcValue) {
        RELEASE_ASSERT(pcValue > lastPCValue);
        uintptr_t delta = bitwise_cast<uintptr_t>(pcValue) - bitwise_cast<uintptr_t>(lastPCValue);
        RELEASE_ASSERT(delta != sentinelPCDelta);
        lastPCValue = pcValue;
        if (delta > std::numeric_limits<uint8_t>::max()) {
            pcCompressor.write<uint8_t>(sentinelPCDelta);
            pcCompressor.write<uintptr_t>(delta);
            return;
        }

        pcCompressor.write<uint8_t>(static_cast<uint8_t>(delta));
    };

    DeltaCompressionBuilder codeOriginCompressor((sizeof(intptr_t) + sizeof(int8_t) + sizeof(int8_t) + sizeof(InlineCallFrame*)) * builder.m_codeRanges.size());
    CodeOrigin lastCodeOrigin(0, nullptr);
    auto buildCodeOriginTable = [&] (const CodeOrigin& codeOrigin) {
        intptr_t delta = static_cast<intptr_t>(codeOrigin.bytecodeIndex) - static_cast<intptr_t>(lastCodeOrigin.bytecodeIndex);
        lastCodeOrigin = codeOrigin;
        if (delta > std::numeric_limits<int8_t>::max() || delta < std::numeric_limits<int8_t>::min() || delta == sentinelBytecodeDelta) {
            codeOriginCompressor.write<int8_t>(sentinelBytecodeDelta);
            codeOriginCompressor.write<intptr_t>(delta);
        } else
            codeOriginCompressor.write<int8_t>(static_cast<int8_t>(delta));

        int8_t hasInlineCallFrameByte = codeOrigin.inlineCallFrame ? 1 : 0;
        codeOriginCompressor.write<int8_t>(hasInlineCallFrameByte);
        if (hasInlineCallFrameByte)
            codeOriginCompressor.write<uintptr_t>(bitwise_cast<uintptr_t>(codeOrigin.inlineCallFrame));
    };

    m_pcRangeStart = bitwise_cast<uintptr_t>(linkBuffer.locationOf(builder.m_codeRanges.first().start).dataLocation());
    m_pcRangeEnd = bitwise_cast<uintptr_t>(linkBuffer.locationOf(builder.m_codeRanges.last().end).dataLocation());
    m_pcRangeEnd -= 1;

    for (unsigned i = 0; i < builder.m_codeRanges.size(); i++) {
        PCToCodeOriginMapBuilder::CodeRange& codeRange = builder.m_codeRanges[i];
        void* start = linkBuffer.locationOf(codeRange.start).dataLocation();
        void* end = linkBuffer.locationOf(codeRange.end).dataLocation();
        ASSERT(m_pcRangeStart <= bitwise_cast<uintptr_t>(start));
        ASSERT(m_pcRangeEnd >= bitwise_cast<uintptr_t>(end) - 1);
        if (start == end)
            ASSERT(i == builder.m_codeRanges.size() - 1);
        if (i > 0)
            ASSERT(linkBuffer.locationOf(builder.m_codeRanges[i - 1].end).dataLocation() == start);

        buildPCTable(start);
        buildCodeOriginTable(codeRange.codeOrigin);
    }

    m_compressedPCBufferSize = pcCompressor.m_offset;
    m_compressedPCs = static_cast<uint8_t*>(fastRealloc(pcCompressor.m_buffer, m_compressedPCBufferSize));

    m_compressedCodeOriginsSize = codeOriginCompressor.m_offset;
    m_compressedCodeOrigins = static_cast<uint8_t*>(fastRealloc(codeOriginCompressor.m_buffer, m_compressedCodeOriginsSize));
}

PCToCodeOriginMap::~PCToCodeOriginMap()
{
    if (m_compressedPCs)
        fastFree(m_compressedPCs);
    if (m_compressedCodeOrigins)
        fastFree(m_compressedCodeOrigins);
}

double PCToCodeOriginMap::memorySize()
{
    double size = 0;
    size += m_compressedPCBufferSize;
    size += m_compressedCodeOriginsSize;
    return size;
}

std::optional<CodeOrigin> PCToCodeOriginMap::findPC(void* pc) const
{
    uintptr_t pcAsInt = bitwise_cast<uintptr_t>(pc);
    if (!(m_pcRangeStart <= pcAsInt && pcAsInt <= m_pcRangeEnd))
        return std::nullopt;

    uintptr_t currentPC = 0;
    CodeOrigin currentCodeOrigin(0, nullptr);

    DeltaCompresseionReader pcReader(m_compressedPCs, m_compressedPCBufferSize);
    DeltaCompresseionReader codeOriginReader(m_compressedCodeOrigins, m_compressedCodeOriginsSize);
    while (true) {
        uintptr_t previousPC = currentPC;
        {
            uint8_t value = pcReader.read<uint8_t>();
            uintptr_t delta;
            if (value == sentinelPCDelta)
                delta = pcReader.read<uintptr_t>();
            else
                delta = value;
            currentPC += delta;
        }

        CodeOrigin previousOrigin = currentCodeOrigin;
        {
            int8_t value = codeOriginReader.read<int8_t>();
            intptr_t delta;
            if (value == sentinelBytecodeDelta)
                delta = codeOriginReader.read<intptr_t>();
            else
                delta = static_cast<intptr_t>(value);

            currentCodeOrigin.bytecodeIndex = static_cast<unsigned>(static_cast<intptr_t>(currentCodeOrigin.bytecodeIndex) + delta);

            int8_t hasInlineFrame = codeOriginReader.read<int8_t>();
            ASSERT(hasInlineFrame == 0 || hasInlineFrame == 1);
            if (hasInlineFrame)
                currentCodeOrigin.inlineCallFrame = bitwise_cast<InlineCallFrame*>(codeOriginReader.read<uintptr_t>());
            else
                currentCodeOrigin.inlineCallFrame = nullptr;
        }

        if (previousPC) {
            uintptr_t startOfRange = previousPC;
            // We subtract 1 because we generate end points inclusively in this table, even though we are interested in ranges of the form: [previousPC, currentPC)
            uintptr_t endOfRange = currentPC - 1;
            if (startOfRange <= pcAsInt && pcAsInt <= endOfRange)
                return std::optional<CodeOrigin>(previousOrigin); // We return previousOrigin here because CodeOrigin's are mapped to the startValue of the range.
        }
    }

    RELEASE_ASSERT_NOT_REACHED();
    return std::nullopt;
}

} // namespace JSC

#endif // ENABLE(JIT)
