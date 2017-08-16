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

#include "config.h"
#include "MacroAssemblerPrinter.h"

#if ENABLE(MASM_PROBE)

#include "MacroAssembler.h"

namespace JSC {

using CPUState = MacroAssembler::CPUState;
using ProbeContext = MacroAssembler::ProbeContext;
using RegisterID = MacroAssembler::RegisterID;
using FPRegisterID = MacroAssembler::FPRegisterID;

static void printIndent(int indentation)
{
    for (; indentation > 0; indentation--)
        dataLog("    ");
}

#define INDENT printIndent(indentation)
    
void printCPU(CPUState& cpu, int indentation)
{
    INDENT, dataLog("cpu: {\n");
    printCPURegisters(cpu, indentation + 1);
    INDENT, dataLog("}\n");
}

void printCPURegisters(CPUState& cpu, int indentation)
{
#if USE(JSVALUE32_64)
    #define INTPTR_HEX_VALUE_FORMAT "0x%08lx"
#else
    #define INTPTR_HEX_VALUE_FORMAT "0x%016lx"
#endif

    #define PRINT_GPREGISTER(_type, _regName) { \
        intptr_t value = reinterpret_cast<intptr_t>(cpu._regName); \
        INDENT, dataLogF("%6s: " INTPTR_HEX_VALUE_FORMAT "  %ld\n", #_regName, value, value) ; \
    }
    FOR_EACH_CPU_GPREGISTER(PRINT_GPREGISTER)
    FOR_EACH_CPU_SPECIAL_REGISTER(PRINT_GPREGISTER)
    #undef PRINT_GPREGISTER
    #undef INTPTR_HEX_VALUE_FORMAT
    
    #define PRINT_FPREGISTER(_type, _regName) { \
        uint64_t* u = reinterpret_cast<uint64_t*>(&cpu._regName); \
        double* d = reinterpret_cast<double*>(&cpu._regName); \
        INDENT, dataLogF("%6s: 0x%016llx  %.13g\n", #_regName, *u, *d); \
    }
    FOR_EACH_CPU_FPREGISTER(PRINT_FPREGISTER)
    #undef PRINT_FPREGISTER
}

static void printPC(CPUState& cpu)
{
    union {
        void* voidPtr;
        intptr_t intptrValue;
    } u;
#if CPU(X86) || CPU(X86_64)
    u.voidPtr = cpu.eip;
#elif CPU(ARM_TRADITIONAL) || CPU(ARM_THUMB2) || CPU(ARM64)
    u.voidPtr = cpu.pc;
#else
#error "Unsupported CPU"
#endif
    dataLogF("pc:<%p %ld>", u.voidPtr, u.intptrValue);
}

void printRegister(CPUState& cpu, RegisterID regID)
{
    const char* name = CPUState::gprName(regID);
    union {
        void* voidPtr;
        intptr_t intptrValue;
    } u;
    u.voidPtr = cpu.gpr(regID);
    dataLogF("%s:<%p %ld>", name, u.voidPtr, u.intptrValue);
}

void printRegister(CPUState& cpu, FPRegisterID regID)
{
    const char* name = CPUState::fprName(regID);
    union {
        double doubleValue;
        uint64_t uint64Value;
    } u;
    u.doubleValue = cpu.fpr(regID);
    dataLogF("%s:<0x%016llx %.13g>", name, u.uint64Value, u.doubleValue);
}

void printMemory(CPUState& cpu, const Memory& memory)
{
    uint8_t* ptr = nullptr;
    switch (memory.addressType) {
    case Memory::AddressType::Address: {
        ptr = reinterpret_cast<uint8_t*>(cpu.gpr(memory.u.address.base));
        ptr += memory.u.address.offset;
        break;
    }
    case Memory::AddressType::AbsoluteAddress: {
        ptr = reinterpret_cast<uint8_t*>(const_cast<void*>(memory.u.absoluteAddress.m_ptr));
        break;
    }
    }

    if (memory.dumpStyle == Memory::SingleWordDump) {
        if (memory.numBytes == sizeof(int8_t)) {
            auto p = reinterpret_cast<int8_t*>(ptr);
            dataLogF("%p:<0x%02x %d>", p, *p, *p);
            return;
        }
        if (memory.numBytes == sizeof(int16_t)) {
            auto p = reinterpret_cast<int16_t*>(ptr);
            dataLogF("%p:<0x%04x %d>", p, *p, *p);
            return;
        }
        if (memory.numBytes == sizeof(int32_t)) {
            auto p = reinterpret_cast<int32_t*>(ptr);
            dataLogF("%p:<0x%08x %d>", p, *p, *p);
            return;
        }
        if (memory.numBytes == sizeof(int64_t)) {
            auto p = reinterpret_cast<int64_t*>(ptr);
            dataLogF("%p:<0x%016llx %lld>", p, *p, *p);
            return;
        }
        // Else, unknown word size. Fall thru and dump in the generic way.
    }

    // Generic dump: dump rows of 16 bytes in 4 byte groupings.
    size_t numBytes = memory.numBytes;
    for (size_t i = 0; i < numBytes; i++) {
        if (!(i % 16))
            dataLogF("%p: ", &ptr[i]);
        else if (!(i % 4))
            dataLog(" ");

        dataLogF("%02x", ptr[i]);

        if (i % 16 == 15)
            dataLog("\n");
    }
    if (numBytes % 16 < 15)
        dataLog("\n");
}

void MacroAssemblerPrinter::printCallback(ProbeContext* context)
{
    typedef PrintArg Arg;
    PrintArgsList& argsList =
    *reinterpret_cast<PrintArgsList*>(context->arg1);
    for (size_t i = 0; i < argsList.size(); i++) {
        auto& arg = argsList[i];
        switch (arg.type) {
        case Arg::Type::AllRegisters:
            printCPU(context->cpu, 1);
            break;
        case Arg::Type::PCRegister:
            printPC(context->cpu);
            break;
        case Arg::Type::RegisterID:
            printRegister(context->cpu, arg.u.gpRegisterID);
            break;
        case Arg::Type::FPRegisterID:
            printRegister(context->cpu, arg.u.fpRegisterID);
            break;
        case Arg::Type::Memory:
            printMemory(context->cpu, arg.u.memory);
            break;
        case Arg::Type::ConstCharPtr:
            dataLog(arg.u.constCharPtr);
            break;
        case Arg::Type::ConstVoidPtr:
            dataLogF("%p", arg.u.constVoidPtr);
            break;
        case Arg::Type::IntptrValue:
            dataLog(arg.u.intptrValue);
            break;
        case Arg::Type::UintptrValue:
            dataLog(arg.u.uintptrValue);
            break;
        }
    }
}

} // namespace JSC

#endif // ENABLE(MASM_PROBE)
