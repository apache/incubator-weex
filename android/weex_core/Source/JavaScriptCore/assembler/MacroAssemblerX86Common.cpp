/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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

#if ENABLE(ASSEMBLER) && (CPU(X86) || CPU(X86_64))
#include "MacroAssemblerX86Common.h"

#include <wtf/InlineASM.h>

namespace JSC {

#if ENABLE(MASM_PROBE)

extern "C" void ctiMasmProbeTrampoline();

#if COMPILER(GCC_OR_CLANG)

// The following are offsets for MacroAssemblerX86Common::ProbeContext fields accessed
// by the ctiMasmProbeTrampoline stub.

#if CPU(X86)
#define PTR_SIZE 4
#else // CPU(X86_64)
#define PTR_SIZE 8
#endif

#define PROBE_PROBE_FUNCTION_OFFSET (0 * PTR_SIZE)
#define PROBE_ARG1_OFFSET (1 * PTR_SIZE)
#define PROBE_ARG2_OFFSET (2 * PTR_SIZE)

#define PROBE_FIRST_GPR_OFFSET (3 * PTR_SIZE)
#define PROBE_CPU_EAX_OFFSET (PROBE_FIRST_GPR_OFFSET + (0 * PTR_SIZE))
#define PROBE_CPU_ECX_OFFSET (PROBE_FIRST_GPR_OFFSET + (1 * PTR_SIZE))
#define PROBE_CPU_EDX_OFFSET (PROBE_FIRST_GPR_OFFSET + (2 * PTR_SIZE))
#define PROBE_CPU_EBX_OFFSET (PROBE_FIRST_GPR_OFFSET + (3 * PTR_SIZE))
#define PROBE_CPU_ESP_OFFSET (PROBE_FIRST_GPR_OFFSET + (4 * PTR_SIZE))
#define PROBE_CPU_EBP_OFFSET (PROBE_FIRST_GPR_OFFSET + (5 * PTR_SIZE))
#define PROBE_CPU_ESI_OFFSET (PROBE_FIRST_GPR_OFFSET + (6 * PTR_SIZE))
#define PROBE_CPU_EDI_OFFSET (PROBE_FIRST_GPR_OFFSET + (7 * PTR_SIZE))

#if CPU(X86)
#define PROBE_FIRST_SPECIAL_OFFSET (PROBE_FIRST_GPR_OFFSET + (8 * PTR_SIZE))
#else // CPU(X86_64)
#define PROBE_CPU_R8_OFFSET (PROBE_FIRST_GPR_OFFSET + (8 * PTR_SIZE))
#define PROBE_CPU_R9_OFFSET (PROBE_FIRST_GPR_OFFSET + (9 * PTR_SIZE))
#define PROBE_CPU_R10_OFFSET (PROBE_FIRST_GPR_OFFSET + (10 * PTR_SIZE))
#define PROBE_CPU_R11_OFFSET (PROBE_FIRST_GPR_OFFSET + (11 * PTR_SIZE))
#define PROBE_CPU_R12_OFFSET (PROBE_FIRST_GPR_OFFSET + (12 * PTR_SIZE))
#define PROBE_CPU_R13_OFFSET (PROBE_FIRST_GPR_OFFSET + (13 * PTR_SIZE))
#define PROBE_CPU_R14_OFFSET (PROBE_FIRST_GPR_OFFSET + (14 * PTR_SIZE))
#define PROBE_CPU_R15_OFFSET (PROBE_FIRST_GPR_OFFSET + (15 * PTR_SIZE))
#define PROBE_FIRST_SPECIAL_OFFSET (PROBE_FIRST_GPR_OFFSET + (16 * PTR_SIZE))
#endif // CPU(X86_64)

#define PROBE_CPU_EIP_OFFSET (PROBE_FIRST_SPECIAL_OFFSET + (0 * PTR_SIZE))
#define PROBE_CPU_EFLAGS_OFFSET (PROBE_FIRST_SPECIAL_OFFSET + (1 * PTR_SIZE))
#define PROBE_FIRST_XMM_OFFSET (PROBE_FIRST_SPECIAL_OFFSET + (2 * PTR_SIZE))

#define XMM_SIZE 8
#define PROBE_CPU_XMM0_OFFSET (PROBE_FIRST_XMM_OFFSET + (0 * XMM_SIZE))
#define PROBE_CPU_XMM1_OFFSET (PROBE_FIRST_XMM_OFFSET + (1 * XMM_SIZE))
#define PROBE_CPU_XMM2_OFFSET (PROBE_FIRST_XMM_OFFSET + (2 * XMM_SIZE))
#define PROBE_CPU_XMM3_OFFSET (PROBE_FIRST_XMM_OFFSET + (3 * XMM_SIZE))
#define PROBE_CPU_XMM4_OFFSET (PROBE_FIRST_XMM_OFFSET + (4 * XMM_SIZE))
#define PROBE_CPU_XMM5_OFFSET (PROBE_FIRST_XMM_OFFSET + (5 * XMM_SIZE))
#define PROBE_CPU_XMM6_OFFSET (PROBE_FIRST_XMM_OFFSET + (6 * XMM_SIZE))
#define PROBE_CPU_XMM7_OFFSET (PROBE_FIRST_XMM_OFFSET + (7 * XMM_SIZE))

#if CPU(X86)
#define PROBE_SIZE (PROBE_CPU_XMM7_OFFSET + XMM_SIZE)
#else // CPU(X86_64)
#define PROBE_CPU_XMM8_OFFSET (PROBE_FIRST_XMM_OFFSET + (8 * XMM_SIZE))
#define PROBE_CPU_XMM9_OFFSET (PROBE_FIRST_XMM_OFFSET + (9 * XMM_SIZE))
#define PROBE_CPU_XMM10_OFFSET (PROBE_FIRST_XMM_OFFSET + (10 * XMM_SIZE))
#define PROBE_CPU_XMM11_OFFSET (PROBE_FIRST_XMM_OFFSET + (11 * XMM_SIZE))
#define PROBE_CPU_XMM12_OFFSET (PROBE_FIRST_XMM_OFFSET + (12 * XMM_SIZE))
#define PROBE_CPU_XMM13_OFFSET (PROBE_FIRST_XMM_OFFSET + (13 * XMM_SIZE))
#define PROBE_CPU_XMM14_OFFSET (PROBE_FIRST_XMM_OFFSET + (14 * XMM_SIZE))
#define PROBE_CPU_XMM15_OFFSET (PROBE_FIRST_XMM_OFFSET + (15 * XMM_SIZE))
#define PROBE_SIZE (PROBE_CPU_XMM15_OFFSET + XMM_SIZE)
#endif // CPU(X86_64)

// These ASSERTs remind you that if you change the layout of ProbeContext,
// you need to change ctiMasmProbeTrampoline offsets above to match.
#define PROBE_OFFSETOF(x) offsetof(struct MacroAssemblerX86Common::ProbeContext, x)
COMPILE_ASSERT(PROBE_OFFSETOF(probeFunction) == PROBE_PROBE_FUNCTION_OFFSET, ProbeContext_probeFunction_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(arg1) == PROBE_ARG1_OFFSET, ProbeContext_arg1_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(arg2) == PROBE_ARG2_OFFSET, ProbeContext_arg2_offset_matches_ctiMasmProbeTrampoline);

COMPILE_ASSERT(PROBE_OFFSETOF(cpu.eax) == PROBE_CPU_EAX_OFFSET, ProbeContext_cpu_eax_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.ecx) == PROBE_CPU_ECX_OFFSET, ProbeContext_cpu_ecx_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.edx) == PROBE_CPU_EDX_OFFSET, ProbeContext_cpu_edx_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.ebx) == PROBE_CPU_EBX_OFFSET, ProbeContext_cpu_ebx_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.esp) == PROBE_CPU_ESP_OFFSET, ProbeContext_cpu_esp_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.ebp) == PROBE_CPU_EBP_OFFSET, ProbeContext_cpu_ebp_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.esi) == PROBE_CPU_ESI_OFFSET, ProbeContext_cpu_esi_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.edi) == PROBE_CPU_EDI_OFFSET, ProbeContext_cpu_edi_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.eip) == PROBE_CPU_EIP_OFFSET, ProbeContext_cpu_eip_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.eflags) == PROBE_CPU_EFLAGS_OFFSET, ProbeContext_cpu_eflags_offset_matches_ctiMasmProbeTrampoline);

#if CPU(X86_64)
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r8) == PROBE_CPU_R8_OFFSET, ProbeContext_cpu_r8_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r9) == PROBE_CPU_R9_OFFSET, ProbeContext_cpu_r9_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r10) == PROBE_CPU_R10_OFFSET, ProbeContext_cpu_r10_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r11) == PROBE_CPU_R11_OFFSET, ProbeContext_cpu_r11_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r12) == PROBE_CPU_R12_OFFSET, ProbeContext_cpu_r12_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r13) == PROBE_CPU_R13_OFFSET, ProbeContext_cpu_r13_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r14) == PROBE_CPU_R14_OFFSET, ProbeContext_cpu_r14_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.r15) == PROBE_CPU_R15_OFFSET, ProbeContext_cpu_r15_offset_matches_ctiMasmProbeTrampoline);
#endif // CPU(X86_64)

COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm0) == PROBE_CPU_XMM0_OFFSET, ProbeContext_cpu_xmm0_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm1) == PROBE_CPU_XMM1_OFFSET, ProbeContext_cpu_xmm1_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm2) == PROBE_CPU_XMM2_OFFSET, ProbeContext_cpu_xmm2_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm3) == PROBE_CPU_XMM3_OFFSET, ProbeContext_cpu_xmm3_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm4) == PROBE_CPU_XMM4_OFFSET, ProbeContext_cpu_xmm4_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm5) == PROBE_CPU_XMM5_OFFSET, ProbeContext_cpu_xmm5_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm6) == PROBE_CPU_XMM6_OFFSET, ProbeContext_cpu_xmm6_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm7) == PROBE_CPU_XMM7_OFFSET, ProbeContext_cpu_xmm7_offset_matches_ctiMasmProbeTrampoline);

#if CPU(X86_64)
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm8) == PROBE_CPU_XMM8_OFFSET, ProbeContext_cpu_xmm8_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm9) == PROBE_CPU_XMM9_OFFSET, ProbeContext_cpu_xmm9_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm10) == PROBE_CPU_XMM10_OFFSET, ProbeContext_cpu_xmm10_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm11) == PROBE_CPU_XMM11_OFFSET, ProbeContext_cpu_xmm11_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm12) == PROBE_CPU_XMM12_OFFSET, ProbeContext_cpu_xmm12_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm13) == PROBE_CPU_XMM13_OFFSET, ProbeContext_cpu_xmm13_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm14) == PROBE_CPU_XMM14_OFFSET, ProbeContext_cpu_xmm14_offset_matches_ctiMasmProbeTrampoline);
COMPILE_ASSERT(PROBE_OFFSETOF(cpu.xmm15) == PROBE_CPU_XMM15_OFFSET, ProbeContext_cpu_xmm15_offset_matches_ctiMasmProbeTrampoline);
#endif // CPU(X86_64)

COMPILE_ASSERT(sizeof(MacroAssemblerX86Common::ProbeContext) == PROBE_SIZE, ProbeContext_size_matches_ctiMasmProbeTrampoline);

#undef PROBE_OFFSETOF

#if CPU(X86)
asm (
    ".globl " SYMBOL_STRING(ctiMasmProbeTrampoline) "\n"
    HIDE_SYMBOL(ctiMasmProbeTrampoline) "\n"
    SYMBOL_STRING(ctiMasmProbeTrampoline) ":" "\n"

    "pushfd" "\n"

    // MacroAssemblerX86Common::probe() has already generated code to store some values.
    // Together with the eflags pushed above, the top of stack now looks like
    // this:
    //     esp[0 * ptrSize]: eflags
    //     esp[1 * ptrSize]: return address / saved eip
    //     esp[2 * ptrSize]: probeFunction
    //     esp[3 * ptrSize]: arg1
    //     esp[4 * ptrSize]: arg2
    //     esp[5 * ptrSize]: saved eax
    //     esp[6 * ptrSize]: saved esp

    "movl %esp, %eax" "\n"
    "subl $" STRINGIZE_VALUE_OF(PROBE_SIZE) ", %esp" "\n"

    // The X86_64 ABI specifies that the worse case stack alignment requirement
    // is 32 bytes.
    "andl $~0x1f, %esp" "\n"

    "movl %ebp, " STRINGIZE_VALUE_OF(PROBE_CPU_EBP_OFFSET) "(%esp)" "\n"
    "movl %esp, %ebp" "\n" // Save the ProbeContext*.

    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_CPU_ECX_OFFSET) "(%ebp)" "\n"
    "movl %edx, " STRINGIZE_VALUE_OF(PROBE_CPU_EDX_OFFSET) "(%ebp)" "\n"
    "movl %ebx, " STRINGIZE_VALUE_OF(PROBE_CPU_EBX_OFFSET) "(%ebp)" "\n"
    "movl %esi, " STRINGIZE_VALUE_OF(PROBE_CPU_ESI_OFFSET) "(%ebp)" "\n"
    "movl %edi, " STRINGIZE_VALUE_OF(PROBE_CPU_EDI_OFFSET) "(%ebp)" "\n"

    "movl 0 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%eax), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_CPU_EFLAGS_OFFSET) "(%ebp)" "\n"
    "movl 1 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%eax), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_CPU_EIP_OFFSET) "(%ebp)" "\n"
    "movl 2 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%eax), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_PROBE_FUNCTION_OFFSET) "(%ebp)" "\n"
    "movl 3 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%eax), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_ARG1_OFFSET) "(%ebp)" "\n"
    "movl 4 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%eax), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_ARG2_OFFSET) "(%ebp)" "\n"
    "movl 5 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%eax), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_CPU_EAX_OFFSET) "(%ebp)" "\n"
    "movl 6 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%eax), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_CPU_ESP_OFFSET) "(%ebp)" "\n"

    "movq %xmm0, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM0_OFFSET) "(%ebp)" "\n"
    "movq %xmm1, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM1_OFFSET) "(%ebp)" "\n"
    "movq %xmm2, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM2_OFFSET) "(%ebp)" "\n"
    "movq %xmm3, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM3_OFFSET) "(%ebp)" "\n"
    "movq %xmm4, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM4_OFFSET) "(%ebp)" "\n"
    "movq %xmm5, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM5_OFFSET) "(%ebp)" "\n"
    "movq %xmm6, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM6_OFFSET) "(%ebp)" "\n"
    "movq %xmm7, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM7_OFFSET) "(%ebp)" "\n"

    // Reserve stack space for the arg while maintaining the required stack
    // pointer 32 byte alignment:
    "subl $0x20, %esp" "\n"
    "movl %ebp, 0(%esp)" "\n" // the ProbeContext* arg.

    "call *" STRINGIZE_VALUE_OF(PROBE_PROBE_FUNCTION_OFFSET) "(%ebp)" "\n"

    // To enable probes to modify register state, we copy all registers
    // out of the ProbeContext before returning.

    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_EDX_OFFSET) "(%ebp), %edx" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_EBX_OFFSET) "(%ebp), %ebx" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_ESI_OFFSET) "(%ebp), %esi" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_EDI_OFFSET) "(%ebp), %edi" "\n"

    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM0_OFFSET) "(%ebp), %xmm0" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM1_OFFSET) "(%ebp), %xmm1" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM2_OFFSET) "(%ebp), %xmm2" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM3_OFFSET) "(%ebp), %xmm3" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM4_OFFSET) "(%ebp), %xmm4" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM5_OFFSET) "(%ebp), %xmm5" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM6_OFFSET) "(%ebp), %xmm6" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM7_OFFSET) "(%ebp), %xmm7" "\n"

    // There are 6 more registers left to restore:
    //     eax, ecx, ebp, esp, eip, and eflags.
    // We need to handle these last few restores carefully because:
    //
    // 1. We need to push the return address on the stack for ret to use.
    //    That means we need to write to the stack.
    // 2. The user probe function may have altered the restore value of esp to
    //    point to the vicinity of one of the restore values for the remaining
    //    registers left to be restored.
    //    That means, for requirement 1, we may end up writing over some of the
    //    restore values. We can check for this, and first copy the restore
    //    values to a "safe area" on the stack before commencing with the action
    //    for requirement 1.
    // 3. For requirement 2, we need to ensure that the "safe area" is
    //    protected from interrupt handlers overwriting it. Hence, the esp needs
    //    to be adjusted to include the "safe area" before we start copying the
    //    the restore values.

    "movl %ebp, %eax" "\n"
    "addl $" STRINGIZE_VALUE_OF(PROBE_CPU_EFLAGS_OFFSET) ", %eax" "\n"
    "cmpl %eax, " STRINGIZE_VALUE_OF(PROBE_CPU_ESP_OFFSET) "(%ebp)" "\n"
    "jg " SYMBOL_STRING(ctiMasmProbeTrampolineEnd) "\n"

    // Locate the "safe area" at 2x sizeof(ProbeContext) below where the new
    // rsp will be. This time we don't have to 32-byte align it because we're
    // not using to store any xmm regs.
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_ESP_OFFSET) "(%ebp), %eax" "\n"
    "subl $2 * " STRINGIZE_VALUE_OF(PROBE_SIZE) ", %eax" "\n"
    "movl %eax, %esp" "\n"

    "subl $" STRINGIZE_VALUE_OF(PROBE_CPU_EAX_OFFSET) ", %eax" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_EAX_OFFSET) "(%ebp), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_CPU_EAX_OFFSET) "(%eax)" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_ECX_OFFSET) "(%ebp), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_CPU_ECX_OFFSET) "(%eax)" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_EBP_OFFSET) "(%ebp), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_CPU_EBP_OFFSET) "(%eax)" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_ESP_OFFSET) "(%ebp), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_CPU_ESP_OFFSET) "(%eax)" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_EIP_OFFSET) "(%ebp), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_CPU_EIP_OFFSET) "(%eax)" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_EFLAGS_OFFSET) "(%ebp), %ecx" "\n"
    "movl %ecx, " STRINGIZE_VALUE_OF(PROBE_CPU_EFLAGS_OFFSET) "(%eax)" "\n"
    "movl %eax, %ebp" "\n"

    SYMBOL_STRING(ctiMasmProbeTrampolineEnd) ":" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_ESP_OFFSET) "(%ebp), %eax" "\n"
    "subl $5 * " STRINGIZE_VALUE_OF(PTR_SIZE) ", %eax" "\n"
    // At this point, %esp should be < %eax.

    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_EFLAGS_OFFSET) "(%ebp), %ecx" "\n"
    "movl %ecx, 0 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%eax)" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_EAX_OFFSET) "(%ebp), %ecx" "\n"
    "movl %ecx, 1 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%eax)" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_ECX_OFFSET) "(%ebp), %ecx" "\n"
    "movl %ecx, 2 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%eax)" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_EBP_OFFSET) "(%ebp), %ecx" "\n"
    "movl %ecx, 3 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%eax)" "\n"
    "movl " STRINGIZE_VALUE_OF(PROBE_CPU_EIP_OFFSET) "(%ebp), %ecx" "\n"
    "movl %ecx, 4 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%eax)" "\n"
    "movl %eax, %esp" "\n"

    "popfd" "\n"
    "popl %eax" "\n"
    "popl %ecx" "\n"
    "popl %ebp" "\n"
    "ret" "\n"
);
#endif // CPU(X86)

#if CPU(X86_64)
asm (
    ".globl " SYMBOL_STRING(ctiMasmProbeTrampoline) "\n"
    HIDE_SYMBOL(ctiMasmProbeTrampoline) "\n"
    SYMBOL_STRING(ctiMasmProbeTrampoline) ":" "\n"

    "pushfq" "\n"

    // MacroAssemblerX86Common::probe() has already generated code to store some values.
    // Together with the rflags pushed above, the top of stack now looks like
    // this:
    //     esp[0 * ptrSize]: rflags
    //     esp[1 * ptrSize]: return address / saved rip
    //     esp[2 * ptrSize]: probeFunction
    //     esp[3 * ptrSize]: arg1
    //     esp[4 * ptrSize]: arg2
    //     esp[5 * ptrSize]: saved rax
    //     esp[6 * ptrSize]: saved rsp

    "movq %rsp, %rax" "\n"
    "subq $" STRINGIZE_VALUE_OF(PROBE_SIZE) ", %rsp" "\n"

    // The X86_64 ABI specifies that the worse case stack alignment requirement
    // is 32 bytes.
    "andq $~0x1f, %rsp" "\n"

    "movq %rbp, " STRINGIZE_VALUE_OF(PROBE_CPU_EBP_OFFSET) "(%rsp)" "\n"
    "movq %rsp, %rbp" "\n" // Save the ProbeContext*.

    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_CPU_ECX_OFFSET) "(%rbp)" "\n"
    "movq %rdx, " STRINGIZE_VALUE_OF(PROBE_CPU_EDX_OFFSET) "(%rbp)" "\n"
    "movq %rbx, " STRINGIZE_VALUE_OF(PROBE_CPU_EBX_OFFSET) "(%rbp)" "\n"
    "movq %rsi, " STRINGIZE_VALUE_OF(PROBE_CPU_ESI_OFFSET) "(%rbp)" "\n"
    "movq %rdi, " STRINGIZE_VALUE_OF(PROBE_CPU_EDI_OFFSET) "(%rbp)" "\n"

    "movq 0 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%rax), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_CPU_EFLAGS_OFFSET) "(%rbp)" "\n"
    "movq 1 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%rax), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_CPU_EIP_OFFSET) "(%rbp)" "\n"
    "movq 2 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%rax), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_PROBE_FUNCTION_OFFSET) "(%rbp)" "\n"
    "movq 3 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%rax), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_ARG1_OFFSET) "(%rbp)" "\n"
    "movq 4 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%rax), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_ARG2_OFFSET) "(%rbp)" "\n"
    "movq 5 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%rax), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_CPU_EAX_OFFSET) "(%rbp)" "\n"
    "movq 6 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%rax), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_CPU_ESP_OFFSET) "(%rbp)" "\n"

    "movq %r8, " STRINGIZE_VALUE_OF(PROBE_CPU_R8_OFFSET) "(%rbp)" "\n"
    "movq %r9, " STRINGIZE_VALUE_OF(PROBE_CPU_R9_OFFSET) "(%rbp)" "\n"
    "movq %r10, " STRINGIZE_VALUE_OF(PROBE_CPU_R10_OFFSET) "(%rbp)" "\n"
    "movq %r11, " STRINGIZE_VALUE_OF(PROBE_CPU_R11_OFFSET) "(%rbp)" "\n"
    "movq %r12, " STRINGIZE_VALUE_OF(PROBE_CPU_R12_OFFSET) "(%rbp)" "\n"
    "movq %r13, " STRINGIZE_VALUE_OF(PROBE_CPU_R13_OFFSET) "(%rbp)" "\n"
    "movq %r14, " STRINGIZE_VALUE_OF(PROBE_CPU_R14_OFFSET) "(%rbp)" "\n"
    "movq %r15, " STRINGIZE_VALUE_OF(PROBE_CPU_R15_OFFSET) "(%rbp)" "\n"

    "movq %xmm0, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM0_OFFSET) "(%rbp)" "\n"
    "movq %xmm1, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM1_OFFSET) "(%rbp)" "\n"
    "movq %xmm2, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM2_OFFSET) "(%rbp)" "\n"
    "movq %xmm3, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM3_OFFSET) "(%rbp)" "\n"
    "movq %xmm4, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM4_OFFSET) "(%rbp)" "\n"
    "movq %xmm5, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM5_OFFSET) "(%rbp)" "\n"
    "movq %xmm6, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM6_OFFSET) "(%rbp)" "\n"
    "movq %xmm7, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM7_OFFSET) "(%rbp)" "\n"
    "movq %xmm8, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM8_OFFSET) "(%rbp)" "\n"
    "movq %xmm9, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM9_OFFSET) "(%rbp)" "\n"
    "movq %xmm10, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM10_OFFSET) "(%rbp)" "\n"
    "movq %xmm11, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM11_OFFSET) "(%rbp)" "\n"
    "movq %xmm12, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM12_OFFSET) "(%rbp)" "\n"
    "movq %xmm13, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM13_OFFSET) "(%rbp)" "\n"
    "movq %xmm14, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM14_OFFSET) "(%rbp)" "\n"
    "movq %xmm15, " STRINGIZE_VALUE_OF(PROBE_CPU_XMM15_OFFSET) "(%rbp)" "\n"

    "movq %rbp, %rdi" "\n" // the ProbeContext* arg.
    "call *" STRINGIZE_VALUE_OF(PROBE_PROBE_FUNCTION_OFFSET) "(%rbp)" "\n"

    // To enable probes to modify register state, we copy all registers
    // out of the ProbeContext before returning.

    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_EDX_OFFSET) "(%rbp), %rdx" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_EBX_OFFSET) "(%rbp), %rbx" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_ESI_OFFSET) "(%rbp), %rsi" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_EDI_OFFSET) "(%rbp), %rdi" "\n"

    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_R8_OFFSET) "(%rbp), %r8" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_R9_OFFSET) "(%rbp), %r9" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_R10_OFFSET) "(%rbp), %r10" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_R11_OFFSET) "(%rbp), %r11" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_R12_OFFSET) "(%rbp), %r12" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_R13_OFFSET) "(%rbp), %r13" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_R14_OFFSET) "(%rbp), %r14" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_R15_OFFSET) "(%rbp), %r15" "\n"

    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM0_OFFSET) "(%rbp), %xmm0" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM1_OFFSET) "(%rbp), %xmm1" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM2_OFFSET) "(%rbp), %xmm2" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM3_OFFSET) "(%rbp), %xmm3" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM4_OFFSET) "(%rbp), %xmm4" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM5_OFFSET) "(%rbp), %xmm5" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM6_OFFSET) "(%rbp), %xmm6" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM7_OFFSET) "(%rbp), %xmm7" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM8_OFFSET) "(%rbp), %xmm8" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM9_OFFSET) "(%rbp), %xmm9" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM10_OFFSET) "(%rbp), %xmm10" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM11_OFFSET) "(%rbp), %xmm11" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM12_OFFSET) "(%rbp), %xmm12" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM13_OFFSET) "(%rbp), %xmm13" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM14_OFFSET) "(%rbp), %xmm14" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_XMM15_OFFSET) "(%rbp), %xmm15" "\n"

    // There are 6 more registers left to restore:
    //     rax, rcx, rbp, rsp, rip, and rflags.
    // We need to handle these last few restores carefully because:
    //
    // 1. We need to push the return address on the stack for ret to use
    //    That means we need to write to the stack.
    // 2. The user probe function may have altered the restore value of esp to
    //    point to the vicinity of one of the restore values for the remaining
    //    registers left to be restored.
    //    That means, for requirement 1, we may end up writing over some of the
    //    restore values. We can check for this, and first copy the restore
    //    values to a "safe area" on the stack before commencing with the action
    //    for requirement 1.
    // 3. For both requirement 2, we need to ensure that the "safe area" is
    //    protected from interrupt handlers overwriting it. Hence, the esp needs
    //    to be adjusted to include the "safe area" before we start copying the
    //    the restore values.

    "movq %rbp, %rax" "\n"
    "addq $" STRINGIZE_VALUE_OF(PROBE_CPU_EFLAGS_OFFSET) ", %rax" "\n"
    "cmpq %rax, " STRINGIZE_VALUE_OF(PROBE_CPU_ESP_OFFSET) "(%rbp)" "\n"
    "jg " SYMBOL_STRING(ctiMasmProbeTrampolineEnd) "\n"

    // Locate the "safe area" at 2x sizeof(ProbeContext) below where the new
    // rsp will be. This time we don't have to 32-byte align it because we're
    // not using to store any xmm regs.
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_ESP_OFFSET) "(%rbp), %rax" "\n"
    "subq $2 * " STRINGIZE_VALUE_OF(PROBE_SIZE) ", %rax" "\n"
    "movq %rax, %rsp" "\n"

    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_EAX_OFFSET) "(%rbp), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_CPU_EAX_OFFSET) "(%rax)" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_ECX_OFFSET) "(%rbp), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_CPU_ECX_OFFSET) "(%rax)" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_EBP_OFFSET) "(%rbp), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_CPU_EBP_OFFSET) "(%rax)" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_ESP_OFFSET) "(%rbp), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_CPU_ESP_OFFSET) "(%rax)" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_EIP_OFFSET) "(%rbp), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_CPU_EIP_OFFSET) "(%rax)" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_EFLAGS_OFFSET) "(%rbp), %rcx" "\n"
    "movq %rcx, " STRINGIZE_VALUE_OF(PROBE_CPU_EFLAGS_OFFSET) "(%rax)" "\n"
    "movq %rax, %rbp" "\n"

    SYMBOL_STRING(ctiMasmProbeTrampolineEnd) ":" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_ESP_OFFSET) "(%rbp), %rax" "\n"
    "subq $5 * " STRINGIZE_VALUE_OF(PTR_SIZE) ", %rax" "\n"
    // At this point, %rsp should be < %rax.

    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_EFLAGS_OFFSET) "(%rbp), %rcx" "\n"
    "movq %rcx, 0 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%rax)" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_EAX_OFFSET) "(%rbp), %rcx" "\n"
    "movq %rcx, 1 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%rax)" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_ECX_OFFSET) "(%rbp), %rcx" "\n"
    "movq %rcx, 2 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%rax)" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_EBP_OFFSET) "(%rbp), %rcx" "\n"
    "movq %rcx, 3 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%rax)" "\n"
    "movq " STRINGIZE_VALUE_OF(PROBE_CPU_EIP_OFFSET) "(%rbp), %rcx" "\n"
    "movq %rcx, 4 * " STRINGIZE_VALUE_OF(PTR_SIZE) "(%rax)" "\n"
    "movq %rax, %rsp" "\n"

    "popfq" "\n"
    "popq %rax" "\n"
    "popq %rcx" "\n"
    "popq %rbp" "\n"
    "ret" "\n"
);
#endif // CPU(X86_64)

#endif // COMPILER(GCC_OR_CLANG)

// What code is emitted for the probe?
// ==================================
// We want to keep the size of the emitted probe invocation code as compact as
// possible to minimize the perturbation to the JIT generated code. However,
// we also need to preserve the CPU registers and set up the ProbeContext to be
// passed to the user probe function.
//
// Hence, we do only the minimum here to preserve a scratch register (i.e. rax
// in this case) and the stack pointer (i.e. rsp), and pass the probe arguments.
// We'll let the ctiMasmProbeTrampoline handle the rest of the probe invocation
// work i.e. saving the CPUState (and setting up the ProbeContext), calling the
// user probe function, and restoring the CPUState before returning to JIT
// generated code.
//
// What registers need to be saved?
// ===============================
// The registers are saved for 2 reasons:
// 1. To preserve their state in the JITted code. This means that all registers
//    that are not callee saved needs to be saved. We also need to save the
//    condition code registers because the probe can be inserted between a test
//    and a branch.
// 2. To allow the probe to inspect the values of the registers for debugging
//    purposes. This means all registers need to be saved.
//
// In summary, save everything. But for reasons stated above, we should do the
// minimum here and let ctiMasmProbeTrampoline do the heavy lifting to save the
// full set.
//
// What values are in the saved registers?
// ======================================
// Conceptually, the saved registers should contain values as if the probe
// is not present in the JIT generated code. Hence, they should contain values
// that are expected at the start of the instruction immediately following the
// probe.
//
// Specifically, the saved stack pointer register will point to the stack
// position before we push the ProbeContext frame. The saved rip will point to
// the address of the instruction immediately following the probe. 

void MacroAssemblerX86Common::probe(MacroAssemblerX86Common::ProbeFunction function, void* arg1, void* arg2)
{
    push(RegisterID::esp);
    push(RegisterID::eax);
    move(TrustedImmPtr(arg2), RegisterID::eax);
    push(RegisterID::eax);
    move(TrustedImmPtr(arg1), RegisterID::eax);
    push(RegisterID::eax);
    move(TrustedImmPtr(reinterpret_cast<void*>(function)), RegisterID::eax);
    push(RegisterID::eax);
    move(TrustedImmPtr(reinterpret_cast<void*>(ctiMasmProbeTrampoline)), RegisterID::eax);
    call(RegisterID::eax);
}

#endif // ENABLE(MASM_PROBE)

#if CPU(X86) && !OS(MAC_OS_X)
MacroAssemblerX86Common::SSE2CheckState MacroAssemblerX86Common::s_sse2CheckState = NotCheckedSSE2;
#endif

MacroAssemblerX86Common::CPUIDCheckState MacroAssemblerX86Common::s_sse4_1CheckState = CPUIDCheckState::NotChecked;
MacroAssemblerX86Common::CPUIDCheckState MacroAssemblerX86Common::s_avxCheckState = CPUIDCheckState::NotChecked;
MacroAssemblerX86Common::CPUIDCheckState MacroAssemblerX86Common::s_lzcntCheckState = CPUIDCheckState::NotChecked;
MacroAssemblerX86Common::CPUIDCheckState MacroAssemblerX86Common::s_bmi1CheckState = CPUIDCheckState::NotChecked;

} // namespace JSC

#endif // ENABLE(ASSEMBLER) && (CPU(X86) || CPU(X86_64))
