/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
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
#include "LinkBuffer.h"

#if ENABLE(ASSEMBLER)

#include "CodeBlock.h"
#include "JITCode.h"
#include "JSCInlines.h"
#include "Options.h"
#include "VM.h"
#include <wtf/CompilationThread.h>

namespace JSC {

bool shouldDumpDisassemblyFor(CodeBlock* codeBlock)
{
    if (codeBlock && JITCode::isOptimizingJIT(codeBlock->jitType()) && Options::dumpDFGDisassembly())
        return true;
    return Options::dumpDisassembly();
}

LinkBuffer::CodeRef LinkBuffer::finalizeCodeWithoutDisassembly()
{
    performFinalization();
    
    ASSERT(m_didAllocate);
    if (m_executableMemory)
        return CodeRef(*m_executableMemory);
    
    return CodeRef::createSelfManagedCodeRef(MacroAssemblerCodePtr(m_code));
}

LinkBuffer::CodeRef LinkBuffer::finalizeCodeWithDisassembly(const char* format, ...)
{
    CodeRef result = finalizeCodeWithoutDisassembly();

    if (m_alreadyDisassembled)
        return result;
    
    StringPrintStream out;
    out.printf("Generated JIT code for ");
    va_list argList;
    va_start(argList, format);
    out.vprintf(format, argList);
    va_end(argList);
    out.printf(":\n");

    out.printf("    Code at [%p, %p):\n", result.code().executableAddress(), static_cast<char*>(result.code().executableAddress()) + result.size());
    
    CString header = out.toCString();
    
    if (Options::asyncDisassembly()) {
        disassembleAsynchronously(header, result, m_size, "    ");
        return result;
    }
    
    dataLog(header);
    disassemble(result.code(), m_size, "    ", WTF::dataFile());
    
    return result;
}

#if ENABLE(BRANCH_COMPACTION)
static ALWAYS_INLINE void recordLinkOffsets(AssemblerData& assemblerData, int32_t regionStart, int32_t regionEnd, int32_t offset)
{
    int32_t ptr = regionStart / sizeof(int32_t);
    const int32_t end = regionEnd / sizeof(int32_t);
    int32_t* offsets = reinterpret_cast_ptr<int32_t*>(assemblerData.buffer());
    while (ptr < end)
        offsets[ptr++] = offset;
}

template <typename InstructionType>
void LinkBuffer::copyCompactAndLinkCode(MacroAssembler& macroAssembler, void* ownerUID, JITCompilationEffort effort)
{
    allocate(macroAssembler, ownerUID, effort);
    const size_t initialSize = macroAssembler.m_assembler.codeSize();
    if (didFailToAllocate())
        return;

    Vector<LinkRecord, 0, UnsafeVectorOverflow>& jumpsToLink = macroAssembler.jumpsToLink();
    m_assemblerStorage = macroAssembler.m_assembler.buffer().releaseAssemblerData();
    uint8_t* inData = reinterpret_cast<uint8_t*>(m_assemblerStorage.buffer());

    AssemblerData outBuffer(m_size);

    uint8_t* outData = reinterpret_cast<uint8_t*>(outBuffer.buffer());
    uint8_t* codeOutData = reinterpret_cast<uint8_t*>(m_code);

    int readPtr = 0;
    int writePtr = 0;
    unsigned jumpCount = jumpsToLink.size();
    if (m_shouldPerformBranchCompaction) {
        for (unsigned i = 0; i < jumpCount; ++i) {
            int offset = readPtr - writePtr;
            ASSERT(!(offset & 1));
                
            // Copy the instructions from the last jump to the current one.
            size_t regionSize = jumpsToLink[i].from() - readPtr;
            InstructionType* copySource = reinterpret_cast_ptr<InstructionType*>(inData + readPtr);
            InstructionType* copyEnd = reinterpret_cast_ptr<InstructionType*>(inData + readPtr + regionSize);
            InstructionType* copyDst = reinterpret_cast_ptr<InstructionType*>(outData + writePtr);
            ASSERT(!(regionSize % 2));
            ASSERT(!(readPtr % 2));
            ASSERT(!(writePtr % 2));
            while (copySource != copyEnd)
                *copyDst++ = *copySource++;
            recordLinkOffsets(m_assemblerStorage, readPtr, jumpsToLink[i].from(), offset);
            readPtr += regionSize;
            writePtr += regionSize;
                
            // Calculate absolute address of the jump target, in the case of backwards
            // branches we need to be precise, forward branches we are pessimistic
            const uint8_t* target;
            if (jumpsToLink[i].to() >= jumpsToLink[i].from())
                target = codeOutData + jumpsToLink[i].to() - offset; // Compensate for what we have collapsed so far
            else
                target = codeOutData + jumpsToLink[i].to() - executableOffsetFor(jumpsToLink[i].to());
                
            JumpLinkType jumpLinkType = MacroAssembler::computeJumpType(jumpsToLink[i], codeOutData + writePtr, target);
            // Compact branch if we can...
            if (MacroAssembler::canCompact(jumpsToLink[i].type())) {
                // Step back in the write stream
                int32_t delta = MacroAssembler::jumpSizeDelta(jumpsToLink[i].type(), jumpLinkType);
                if (delta) {
                    writePtr -= delta;
                    recordLinkOffsets(m_assemblerStorage, jumpsToLink[i].from() - delta, readPtr, readPtr - writePtr);
                }
            }
            jumpsToLink[i].setFrom(writePtr);
        }
    } else {
        if (!ASSERT_DISABLED) {
            for (unsigned i = 0; i < jumpCount; ++i)
                ASSERT(!MacroAssembler::canCompact(jumpsToLink[i].type()));
        }
    }
    // Copy everything after the last jump
    memcpy(outData + writePtr, inData + readPtr, initialSize - readPtr);
    recordLinkOffsets(m_assemblerStorage, readPtr, initialSize, readPtr - writePtr);
        
    for (unsigned i = 0; i < jumpCount; ++i) {
        uint8_t* location = codeOutData + jumpsToLink[i].from();
        uint8_t* target = codeOutData + jumpsToLink[i].to() - executableOffsetFor(jumpsToLink[i].to());
        MacroAssembler::link(jumpsToLink[i], outData + jumpsToLink[i].from(), location, target);
    }

    jumpsToLink.clear();

    size_t compactSize = writePtr + initialSize - readPtr;
    if (m_executableMemory) {
        m_size = compactSize;
        m_executableMemory->shrink(m_size);
    } else {
        size_t nopSizeInBytes = initialSize - compactSize;
        bool isCopyingToExecutableMemory = false;
        MacroAssembler::AssemblerType_T::fillNops(outData + compactSize, nopSizeInBytes, isCopyingToExecutableMemory);
    }

    performJITMemcpy(m_code, outData, m_size);

#if DUMP_LINK_STATISTICS
    dumpLinkStatistics(m_code, initialSize, m_size);
#endif
#if DUMP_CODE
    dumpCode(m_code, m_size);
#endif
}
#endif


void LinkBuffer::linkCode(MacroAssembler& macroAssembler, void* ownerUID, JITCompilationEffort effort)
{
    // Ensure that the end of the last invalidation point does not extend beyond the end of the buffer.
    macroAssembler.label();

#if !ENABLE(BRANCH_COMPACTION)
#if defined(ASSEMBLER_HAS_CONSTANT_POOL) && ASSEMBLER_HAS_CONSTANT_POOL
    macroAssembler.m_assembler.buffer().flushConstantPool(false);
#endif
    allocate(macroAssembler, ownerUID, effort);
    if (!m_didAllocate)
        return;
    ASSERT(m_code);
    AssemblerBuffer& buffer = macroAssembler.m_assembler.buffer();
#if CPU(ARM_TRADITIONAL)
    macroAssembler.m_assembler.prepareExecutableCopy(m_code);
#endif
    performJITMemcpy(m_code, buffer.data(), buffer.codeSize());
#if CPU(MIPS)
    macroAssembler.m_assembler.relocateJumps(buffer.data(), m_code);
#endif
#elif CPU(ARM_THUMB2)
    copyCompactAndLinkCode<uint16_t>(macroAssembler, ownerUID, effort);
#elif CPU(ARM64)
    copyCompactAndLinkCode<uint32_t>(macroAssembler, ownerUID, effort);
#endif // !ENABLE(BRANCH_COMPACTION)

    m_linkTasks = WTFMove(macroAssembler.m_linkTasks);
}

void LinkBuffer::allocate(MacroAssembler& macroAssembler, void* ownerUID, JITCompilationEffort effort)
{
    size_t initialSize = macroAssembler.m_assembler.codeSize();
    if (m_code) {
        if (initialSize > m_size)
            return;
        
        size_t nopsToFillInBytes = m_size - initialSize;
        macroAssembler.emitNops(nopsToFillInBytes);
        m_didAllocate = true;
        return;
    }
    
    ASSERT(m_vm != nullptr);
    m_executableMemory = m_vm->executableAllocator.allocate(*m_vm, initialSize, ownerUID, effort);
    if (!m_executableMemory)
        return;
    m_code = m_executableMemory->start();
    m_size = initialSize;
    m_didAllocate = true;
}

void LinkBuffer::performFinalization()
{
    for (auto& task : m_linkTasks)
        task->run(*this);

#ifndef NDEBUG
    ASSERT(!isCompilationThread());
    ASSERT(!m_completed);
    ASSERT(isValid());
    m_completed = true;
#endif
    
    MacroAssembler::cacheFlush(code(), m_size);
}

#if DUMP_LINK_STATISTICS
void LinkBuffer::dumpLinkStatistics(void* code, size_t initializeSize, size_t finalSize)
{
    static unsigned linkCount = 0;
    static unsigned totalInitialSize = 0;
    static unsigned totalFinalSize = 0;
    linkCount++;
    totalInitialSize += initialSize;
    totalFinalSize += finalSize;
    dataLogF("link %p: orig %u, compact %u (delta %u, %.2f%%)\n", 
            code, static_cast<unsigned>(initialSize), static_cast<unsigned>(finalSize),
            static_cast<unsigned>(initialSize - finalSize),
            100.0 * (initialSize - finalSize) / initialSize);
    dataLogF("\ttotal %u: orig %u, compact %u (delta %u, %.2f%%)\n", 
            linkCount, totalInitialSize, totalFinalSize, totalInitialSize - totalFinalSize,
            100.0 * (totalInitialSize - totalFinalSize) / totalInitialSize);
}
#endif

#if DUMP_CODE
void LinkBuffer::dumpCode(void* code, size_t size)
{
#if CPU(ARM_THUMB2)
    // Dump the generated code in an asm file format that can be assembled and then disassembled
    // for debugging purposes. For example, save this output as jit.s:
    //   gcc -arch armv7 -c jit.s
    //   otool -tv jit.o
    static unsigned codeCount = 0;
    unsigned short* tcode = static_cast<unsigned short*>(code);
    size_t tsize = size / sizeof(short);
    char nameBuf[128];
    snprintf(nameBuf, sizeof(nameBuf), "_jsc_jit%u", codeCount++);
    dataLogF("\t.syntax unified\n"
            "\t.section\t__TEXT,__text,regular,pure_instructions\n"
            "\t.globl\t%s\n"
            "\t.align 2\n"
            "\t.code 16\n"
            "\t.thumb_func\t%s\n"
            "# %p\n"
            "%s:\n", nameBuf, nameBuf, code, nameBuf);
        
    for (unsigned i = 0; i < tsize; i++)
        dataLogF("\t.short\t0x%x\n", tcode[i]);
#elif CPU(ARM_TRADITIONAL)
    //   gcc -c jit.s
    //   objdump -D jit.o
    static unsigned codeCount = 0;
    unsigned int* tcode = static_cast<unsigned int*>(code);
    size_t tsize = size / sizeof(unsigned int);
    char nameBuf[128];
    snprintf(nameBuf, sizeof(nameBuf), "_jsc_jit%u", codeCount++);
    dataLogF("\t.globl\t%s\n"
            "\t.align 4\n"
            "\t.code 32\n"
            "\t.text\n"
            "# %p\n"
            "%s:\n", nameBuf, code, nameBuf);

    for (unsigned i = 0; i < tsize; i++)
        dataLogF("\t.long\t0x%x\n", tcode[i]);
#endif
}
#endif

} // namespace JSC

#endif // ENABLE(ASSEMBLER)


