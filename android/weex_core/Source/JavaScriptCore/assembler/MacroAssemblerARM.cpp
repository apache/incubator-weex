/*
 * Copyright (C) 2013-2015 Apple Inc.
 * Copyright (C) 2009 University of Szeged
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(ASSEMBLER) && CPU(ARM_TRADITIONAL)

#include "MacroAssemblerARM.h"

#include <wtf/InlineASM.h>

#if OS(LINUX)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <asm/hwcap.h>
#endif

namespace JSC {

static bool isVFPPresent()
{
#if OS(LINUX)
    int fd = open("/proc/self/auxv", O_RDONLY);
    if (fd != -1) {
        Elf32_auxv_t aux;
        while (read(fd, &aux, sizeof(Elf32_auxv_t))) {
            if (aux.a_type == AT_HWCAP) {
                close(fd);
                return aux.a_un.a_val & HWCAP_VFP;
            }
        }
        close(fd);
    }
#endif // OS(LINUX)

#if (COMPILER(GCC_OR_CLANG) && defined(__VFP_FP__))
    return true;
#else
    return false;
#endif
}

const bool MacroAssemblerARM::s_isVFPPresent = isVFPPresent();

#if CPU(ARMV5_OR_LOWER)
/* On ARMv5 and below, natural alignment is required. */
void MacroAssemblerARM::load32WithUnalignedHalfWords(BaseIndex address, RegisterID dest)
{
    ARMWord op2;

    ASSERT(address.scale >= 0 && address.scale <= 3);
    op2 = m_assembler.lsl(address.index, static_cast<int>(address.scale));

    if (address.offset >= 0 && address.offset + 0x2 <= 0xff) {
        m_assembler.add(ARMRegisters::S0, address.base, op2);
        m_assembler.halfDtrUp(ARMAssembler::LoadUint16, dest, ARMRegisters::S0, ARMAssembler::getOp2Half(address.offset));
        m_assembler.halfDtrUp(ARMAssembler::LoadUint16, ARMRegisters::S0, ARMRegisters::S0, ARMAssembler::getOp2Half(address.offset + 0x2));
    } else if (address.offset < 0 && address.offset >= -0xff) {
        m_assembler.add(ARMRegisters::S0, address.base, op2);
        m_assembler.halfDtrDown(ARMAssembler::LoadUint16, dest, ARMRegisters::S0, ARMAssembler::getOp2Half(-address.offset));
        m_assembler.halfDtrDown(ARMAssembler::LoadUint16, ARMRegisters::S0, ARMRegisters::S0, ARMAssembler::getOp2Half(-address.offset - 0x2));
    } else {
        m_assembler.moveImm(address.offset, ARMRegisters::S0);
        m_assembler.add(ARMRegisters::S0, ARMRegisters::S0, op2);
        m_assembler.halfDtrUpRegister(ARMAssembler::LoadUint16, dest, address.base, ARMRegisters::S0);
        m_assembler.add(ARMRegisters::S0, ARMRegisters::S0, ARMAssembler::Op2Immediate | 0x2);
        m_assembler.halfDtrUpRegister(ARMAssembler::LoadUint16, ARMRegisters::S0, address.base, ARMRegisters::S0);
    }
    m_assembler.orr(dest, dest, m_assembler.lsl(ARMRegisters::S0, 16));
}
#endif // CPU(ARMV5_OR_LOWER)

#if ENABLE(MASM_PROBE)

extern "C" void ctiMasmProbeTrampoline();

#if COMPILER(GCC_OR_CLANG)
    
// The following are offsets for MacroAssemblerARM::ProbeContext fields accessed
// by the ctiMasmProbeTrampoline stub.

#define PTR_SIZE 4
#define PROBE_PROBE_FUNCTION_OFFSET (0 * PTR_SIZE)
#define PROBE_ARG1_OFFSET (1 * PTR_SIZE)
#define PROBE_ARG2_OFFSET (2 * PTR_SIZE)

#define PROBE_FIRST_GPREG_OFFSET (4 * PTR_SIZE)

#define GPREG_SIZE 4
#define PROBE_CPU_R0_OFFSET (PROBE_FIRST_GPREG_OFFSET + (0 * GPREG_SIZE))
#define PROBE_CPU_R1_OFFSET (PROBE_FIRST_GPREG_OFFSET + (1 * GPREG_SIZE))
#define PROBE_CPU_R2_OFFSET (PROBE_FIRST_GPREG_OFFSET + (2 * GPREG_SIZE))
#define PROBE_CPU_R3_OFFSET (PROBE_FIRST_GPREG_OFFSET + (3 * GPREG_SIZE))
#define PROBE_CPU_R4_OFFSET (PROBE_FIRST_GPREG_OFFSET + (4 * GPREG_SIZE))
#define PROBE_CPU_R5_OFFSET (PROBE_FIRST_GPREG_OFFSET + (5 * GPREG_SIZE))
#define PROBE_CPU_R6_OFFSET (PROBE_FIRST_GPREG_OFFSET + (6 * GPREG_SIZE))
#define PROBE_CPU_R7_OFFSET (PROBE_FIRST_GPREG_OFFSET + (7 * GPREG_SIZE))
#define PROBE_CPU_R8_OFFSET (PROBE_FIRST_GPREG_OFFSET + (8 * GPREG_SIZE))
#define PROBE_CPU_R9_OFFSET (PROBE_FIRST_GPREG_OFFSET + (9 * GPREG_SIZE))
#define PROBE_CPU_R10_OFFSET (PROBE_FIRST_GPREG_OFFSET + (10 * GPREG_SIZE))
#define PROBE_CPU_R11_OFFSET (PROBE_FIRST_GPREG_OFFSET + (11 * GPREG_SIZE))
#define PROBE_CPU_IP_OFFSET (PROBE_FIRST_GPREG_OFFSET + (12 * GPREG_SIZE))
#define PROBE_CPU_SP_OFFSET (PROBE_FIRST_GPREG_OFFSET + (13 * GPREG_SIZE))
#define PROBE_CPU_LR_OFFSET (PROBE_FIRST_GPREG_OFFSET + (14 * GPREG_SIZE))
#define PROBE_CPU_PC_OFFSET (PROBE_FIRST_GPREG_OFFSET + (15 * GPREG_SIZE))

#define PROBE_CPU_APSR_OFFSET (PROBE_FIRST_GPREG_OFFSET + (16 * GPREG_SIZE))
#define PROBE_CPU_FPSCR_OFFSET (PROBE_FIRST_GPREG_OFFSET + (17 * GPREG_SIZE))

#define PROBE_FIRST_FPREG_OFFSET (PROBE_FIRST_GPREG_OFFSET + (18 * GPREG_SIZE))

#define FPREG_SIZE 8
#define PROBE_CPU_D0_OFFSET (PROBE_FIRST_FPREG_OFFSET + (0 * FPREG_SIZE))
#define PROBE_CPU_D1_OFFSET (PROBE_FIRST_FPREG_OFFSET + (1 * FPREG_SIZE))
#define PROBE_CPU_D2_OFFSET (PROBE_FIRST_FPREG_OFFSET + (2 * FPREG_SIZE))
#define PROBE_CPU_D3_OFFSET (PROBE_FIRST_FPREG_OFFSET + (3 * FPREG_SIZE))
#define PROBE_CPU_D4_OFFSET (PROBE_FIRST_FPREG_OFFSET + (4 * FPREG_SIZE))
#define PROBE_CPU_D5_OFFSET (PROBE_FIRST_FPREG_OFFSET + (5 * FPREG_SIZE))
#define PROBE_CPU_D6_OFFSET (PROBE_FIRST_FPREG_OFFSET + (6 * FPREG_SIZE))
#define PROBE_CPU_D7_OFFSET (PROBE_FIRST_FPREG_OFFSET + (7 * FPREG_SIZE))
#define PROBE_CPU_D8_OFFSET (PROBE_FIRST_FPREG_OFFSET + (8 * FPREG_SIZE))
#define PROBE_CPU_D9_OFFSET (PROBE_FIRST_FPREG_OFFSET + (9 * FPREG_SIZE))
#define PROBE_CPU_D10_OFFSET (PROBE_FIRST_FPREG_OFFSET + (10 * FPREG_SIZE))
#define PROBE_CPU_D11_OFFSET (PROBE_FIRST_FPREG_OFFSET + (11 * FPREG_SIZE))
#define PROBE_CPU_D12_OFFSET (PROBE_FIRST_FPREG_OFFSET + (12 * FPREG_SIZE))
#define PROBE_CPU_D13_OFFSET (PROBE_FIRST_FPREG_OFFSET + (13 * FPREG_SIZE))
#define PROBE_CPU_D14_OFFSET (PROBE_FIRST_FPREG_OFFSET + (14 * FPREG_SIZE))
#define PROBE_CPU_D15_OFFSET (PROBE_FIRST_FPREG_OFFSET + (15 * FPREG_SIZE))

#define PROBE_SIZE (PROBE_FIRST_FPREG_OFFSET + (16 * FPREG_SIZE))

// These ASSERTs remind you that if you change the layout of ProbeContext,
// you need to change ctiMasmProbeTrampoline offsets above to match.
#define PROBE_OFFSETOF(x) offsetof(struct MacroAssemblerARM::ProbeContext, x)
COMPILE_ASSERT(PROBE_OFFSETOF(probeFunction) == PROBE_PROBE_FUNCTION_OFFSET, ProbeContext_probeFunction_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(arg1) == PROBE_ARG1_OFFSET, ProbeContext_arg1_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(arg2) == PROBE_ARG2_OFFSET, ProbeContext_arg2_offset_matches_ctiMasmProbeTrampoline);

COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r0) == PROBE_CPU_R0_OFFSET, ProbeContext_cpu_r0_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r1) == PROBE_CPU_R1_OFFSET, ProbeContext_cpu_r1_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r2) == PROBE_CPU_R2_OFFSET, ProbeContext_cpu_r2_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r3) == PROBE_CPU_R3_OFFSET, ProbeContext_cpu_r3_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r4) == PROBE_CPU_R4_OFFSET, ProbeContext_cpu_r4_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r5) == PROBE_CPU_R5_OFFSET, ProbeContext_cpu_r5_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r6) == PROBE_CPU_R6_OFFSET, ProbeContext_cpu_r6_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r7) == PROBE_CPU_R7_OFFSET, ProbeContext_cpu_r7_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r8) == PROBE_CPU_R8_OFFSET, ProbeContext_cpu_r8_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r9) == PROBE_CPU_R9_OFFSET, ProbeContext_cpu_r9_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r10) == PROBE_CPU_R10_OFFSET, ProbeContext_cpu_r10_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r11) == PROBE_CPU_R11_OFFSET, ProbeContext_cpu_r11_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.ip) == PROBE_CPU_IP_OFFSET, ProbeContext_cpu_ip_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.sp) == PROBE_CPU_SP_OFFSET, ProbeContext_cpu_sp_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.lr) == PROBE_CPU_LR_OFFSET, ProbeContext_cpu_lr_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.pc) == PROBE_CPU_PC_OFFSET, ProbeContext_cpu_pc_offset_matches_ctiMasmProbeTrampoline);

COMPILE_ASSERT(PROBE_OFFSETOF(cpu.apsr) == PROBE_CPU_APSR_OFFSET, ProbeContext_cpu_apsr_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.fpscr) == PROBE_CPU_FPSCR_OFFSET, ProbeContext_cpu_fpscr_offset_matches_ctiMasmProbeTrampoline);

COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d0) == PROBE_CPU_D0_OFFSET, ProbeContext_cpu_d0_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d1) == PROBE_CPU_D1_OFFSET, ProbeContext_cpu_d1_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d2) == PROBE_CPU_D2_OFFSET, ProbeContext_cpu_d2_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d3) == PROBE_CPU_D3_OFFSET, ProbeContext_cpu_d3_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d4) == PROBE_CPU_D4_OFFSET, ProbeContext_cpu_d4_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d5) == PROBE_CPU_D5_OFFSET, ProbeContext_cpu_d5_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d6) == PROBE_CPU_D6_OFFSET, ProbeContext_cpu_d6_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d7) == PROBE_CPU_D7_OFFSET, ProbeContext_cpu_d7_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d8) == PROBE_CPU_D8_OFFSET, ProbeContext_cpu_d8_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d9) == PROBE_CPU_D9_OFFSET, ProbeContext_cpu_d9_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d10) == PROBE_CPU_D10_OFFSET, ProbeContext_cpu_d10_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d11) == PROBE_CPU_D11_OFFSET, ProbeContext_cpu_d11_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d12) == PROBE_CPU_D12_OFFSET, ProbeContext_cpu_d12_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d13) == PROBE_CPU_D13_OFFSET, ProbeContext_cpu_d13_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d14) == PROBE_CPU_D14_OFFSET, ProbeContext_cpu_d14_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.d15) == PROBE_CPU_D15_OFFSET, ProbeContext_cpu_d15_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(sizeof(MacroAssemblerARM::ProbeContext) == PROBE_SIZE, ProbeContext_size_matches_ctiMasmProbeTrampoline);
#undef PROBE_OFFSETOF

asm (
    ".text" "\n"
    ".globl " SYMBOL_STRING(ctiMasmProbeTrampoline) "\n"
    HIDE_SYMBOL(ctiMasmProbeTrampoline) "\n"
    INLINE_ARM_FUNCTION(ctiMasmProbeTrampoline) "\n"
    SYMBOL_STRING(ctiMasmProbeTrampoline) ":" "\n"

    // MacroAssemblerARM::probe() has already generated code to store some values.
    // The top of stack now looks like this:
    //     esp[0 * ptrSize]: probeFunction
    //     esp[1 * ptrSize]: arg1
    //     esp[2 * ptrSize]: arg2
    //     esp[3 * ptrSize]: saved r3 / S0
    //     esp[4 * ptrSize]: saved ip
    //     esp[5 * ptrSize]: saved lr
    //     esp[6 * ptrSize]: saved sp

    "mov       ip, sp" "\n"
    "mov       r3, sp" "\n"
    "sub       r3, r3, #" STRINGIZE_VALUE_OF(PROBE_SIZE) "\n"

    // The ARM EABI specifies that the stack needs to be 16 byte aligned.
    "bic       r3, r3, #0xf" "\n"
    "mov       sp, r3" "\n"

    "str       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_PC_OFFSET) "]" "\n"
    "add       lr, sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_R0_OFFSET) "\n"
    "stmia     lr, { r0-r11 }" "\n"
    "mrs       lr, APSR" "\n"
    "str       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_APSR_OFFSET) "]" "\n"
    "vmrs      lr, FPSCR" "\n"
    "str       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_FPSCR_OFFSET) "]" "\n"

    "ldr       lr, [ip, #0 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_PROBE_FUNCTION_OFFSET) "]" "\n"
    "ldr       lr, [ip, #1 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_ARG1_OFFSET) "]" "\n"
    "ldr       lr, [ip, #2 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_ARG2_OFFSET) "]" "\n"
    "ldr       lr, [ip, #3 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_R3_OFFSET) "]" "\n"
    "ldr       lr, [ip, #4 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_IP_OFFSET) "]" "\n"
    "ldr       lr, [ip, #5 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_LR_OFFSET) "]" "\n"
    "ldr       lr, [ip, #6 * " STRINGIZE_VALUE_OF(PTR_SIZE) "]" "\n"
    "str       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_SP_OFFSET) "]" "\n"

    "ldr       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_PC_OFFSET) "]" "\n"

    "add       ip, sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_D0_OFFSET) "\n"
    "vstmia.64 ip, { d0-d15 }" "\n"

    "mov       fp, sp" "\n" // Save the ProbeContext*.

    "ldr       ip, [sp, #" STRINGIZE_VALUE_OF(PROBE_PROBE_FUNCTION_OFFSET) "]" "\n"
    "mov       r0, sp" "\n" // the ProbeContext* arg.
    "blx       ip" "\n"

    "mov       sp, fp" "\n"

    // To enable probes to modify register state, we copy all registers
    // out of the ProbeContext before returning.

    "add       ip, sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_D15_OFFSET + FPREG_SIZE) "\n"
    "vldmdb.64 ip!, { d0-d15 }" "\n"
    "add       ip, sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_R11_OFFSET + GPREG_SIZE) "\n"
    "ldmdb     ip, { r0-r11 }" "\n"
    "ldr       ip, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_FPSCR_OFFSET) "]" "\n"
    "vmsr      FPSCR, ip" "\n"

    // There are 5 more registers left to restore: ip, sp, lr, pc, and apsr.
    // There are 2 issues that complicate the restoration of these last few
    // registers:
    //
    // 1. Normal ARM calling convention relies on moving lr to pc to return to
    //    the caller. In our case, the address to return to is specified by
    //    ProbeContext.cpu.pc. And at that moment, we won't have any available
    //    scratch registers to hold the return address (lr needs to hold
    //    ProbeContext.cpu.lr, not the return address).
    //
    //    The solution is to store the return address on the stack and load the
    //     pc from there.
    //
    // 2. Issue 1 means we will need to write to the stack location at
    //    ProbeContext.cpu.sp - 4. But if the user probe function had  modified
    //    the value of ProbeContext.cpu.sp to point in the range between
    //    &ProbeContext.cpu.ip thru &ProbeContext.cpu.aspr, then the action for
    //    Issue 1 may trash the values to be restored before we can restore
    //    them.
    //
    //    The solution is to check if ProbeContext.cpu.sp contains a value in
    //    the undesirable range. If so, we copy the remaining ProbeContext
    //    register data to a safe range (at memory lower than where
    //    ProbeContext.cpu.sp points) first, and restore the remaining register
    //    from this new range.

    "add       ip, sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_APSR_OFFSET) "\n"
    "ldr       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_SP_OFFSET) "]" "\n"
    "cmp       lr, ip" "\n"
    "bgt     " SYMBOL_STRING(ctiMasmProbeTrampolineEnd) "\n"

    // We get here because the new expected stack pointer location is lower
    // than where it's supposed to be. This means the safe range of stack
    // memory where we'll be copying the remaining register restore values to
    // might be in a region of memory below the sp i.e. unallocated stack
    // memory. This in turn makes it vulnerable to interrupts potentially
    // trashing the copied values. To prevent that, we must first allocate the
    // needed stack memory by adjusting the sp before the copying.

    "sub       lr, lr, #(6 * " STRINGIZE_VALUE_OF(PTR_SIZE)
    " + " STRINGIZE_VALUE_OF(PROBE_CPU_IP_OFFSET) ")" "\n"

    "mov       ip, sp" "\n"
    "mov       sp, lr" "\n"
    "mov       lr, ip" "\n"

    "ldr       ip, [lr, #" STRINGIZE_VALUE_OF(PROBE_CPU_IP_OFFSET) "]" "\n"
    "str       ip, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_IP_OFFSET) "]" "\n"
    "ldr       ip, [lr, #" STRINGIZE_VALUE_OF(PROBE_CPU_SP_OFFSET) "]" "\n"
    "str       ip, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_SP_OFFSET) "]" "\n"
    "ldr       ip, [lr, #" STRINGIZE_VALUE_OF(PROBE_CPU_LR_OFFSET) "]" "\n"
    "str       ip, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_LR_OFFSET) "]" "\n"
    "ldr       ip, [lr, #" STRINGIZE_VALUE_OF(PROBE_CPU_PC_OFFSET) "]" "\n"
    "str       ip, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_PC_OFFSET) "]" "\n"
    "ldr       ip, [lr, #" STRINGIZE_VALUE_OF(PROBE_CPU_APSR_OFFSET) "]" "\n"
    "str       ip, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_APSR_OFFSET) "]" "\n"

    SYMBOL_STRING(ctiMasmProbeTrampolineEnd) ":" "\n"
    "ldr       ip, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_PC_OFFSET) "]" "\n"
    "ldr       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_SP_OFFSET) "]" "\n"
    "sub       lr, lr, #" STRINGIZE_VALUE_OF(PTR_SIZE) "\n"
    "str       ip, [lr]" "\n"
    "str       lr, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_SP_OFFSET) "]" "\n"

    "ldr       ip, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_APSR_OFFSET) "]" "\n"
    "msr       APSR, ip" "\n"
    "ldr       ip, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_LR_OFFSET) "]" "\n"
    "mov       lr, ip" "\n"
    "ldr       ip, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_IP_OFFSET) "]" "\n"
    "ldr       sp, [sp, #" STRINGIZE_VALUE_OF(PROBE_CPU_SP_OFFSET) "]" "\n"

    "pop       { pc }" "\n"
);
#endif // COMPILER(GCC_OR_CLANG)

void MacroAssemblerARM::probe(MacroAssemblerARM::ProbeFunction function, void* arg1, void* arg2)
{
    push(RegisterID::sp);
    push(RegisterID::lr);
    push(RegisterID::ip);
    push(RegisterID::S0);
    // The following uses RegisterID::S0. So, they must come after we push S0 above.
    push(trustedImm32FromPtr(arg2));
    push(trustedImm32FromPtr(arg1));
    push(trustedImm32FromPtr(function));

    move(trustedImm32FromPtr(ctiMasmProbeTrampoline), RegisterID::S0);
    m_assembler.blx(RegisterID::S0);

}
#endif // ENABLE(MASM_PROBE)

} // namespace JSC

#endif // ENABLE(ASSEMBLER) && CPU(ARM_TRADITIONAL)
