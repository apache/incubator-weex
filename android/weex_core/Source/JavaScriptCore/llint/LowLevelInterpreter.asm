# Copyright (C) 2011-2017 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.

# Crash course on the language that this is written in (which I just call
# "assembly" even though it's more than that):
#
# - Mostly gas-style operand ordering. The last operand tends to be the
#   destination. So "a := b" is written as "mov b, a". But unlike gas,
#   comparisons are in-order, so "if (a < b)" is written as
#   "bilt a, b, ...".
#
# - "b" = byte, "h" = 16-bit word, "i" = 32-bit word, "p" = pointer.
#   For 32-bit, "i" and "p" are interchangeable except when an op supports one
#   but not the other.
#
# - In general, valid operands for macro invocations and instructions are
#   registers (eg "t0"), addresses (eg "4[t0]"), base-index addresses
#   (eg "7[t0, t1, 2]"), absolute addresses (eg "0xa0000000[]"), or labels
#   (eg "_foo" or ".foo"). Macro invocations can also take anonymous
#   macros as operands. Instructions cannot take anonymous macros.
#
# - Labels must have names that begin with either "_" or ".".  A "." label
#   is local and gets renamed before code gen to minimize namespace
#   pollution. A "_" label is an extern symbol (i.e. ".globl"). The "_"
#   may or may not be removed during code gen depending on whether the asm
#   conventions for C name mangling on the target platform mandate a "_"
#   prefix.
#
# - A "macro" is a lambda expression, which may be either anonymous or
#   named. But this has caveats. "macro" can take zero or more arguments,
#   which may be macros or any valid operands, but it can only return
#   code. But you can do Turing-complete things via continuation passing
#   style: "macro foo (a, b) b(a, a) end foo(foo, foo)". Actually, don't do
#   that, since you'll just crash the assembler.
#
# - An "if" is a conditional on settings. Any identifier supplied in the
#   predicate of an "if" is assumed to be a #define that is available
#   during code gen. So you can't use "if" for computation in a macro, but
#   you can use it to select different pieces of code for different
#   platforms.
#
# - Arguments to macros follow lexical scoping rather than dynamic scoping.
#   Const's also follow lexical scoping and may override (hide) arguments
#   or other consts. All variables (arguments and constants) can be bound
#   to operands. Additionally, arguments (but not constants) can be bound
#   to macros.

# The following general-purpose registers are available:
#
#  - cfr and sp hold the call frame and (native) stack pointer respectively.
#  They are callee-save registers, and guaranteed to be distinct from all other
#  registers on all architectures.
#
#  - lr is defined on non-X86 architectures (ARM64, ARMv7, ARM,
#  ARMv7_TRADITIONAL, MIPS and CLOOP) and holds the return PC
#
#  - pc holds the (native) program counter on 32-bits ARM architectures (ARM,
#  ARMv7, ARMv7_TRADITIONAL)
#
#  - t0, t1, t2, t3, t4 and optionally t5 are temporary registers that can get trashed on
#  calls, and are pairwise distinct registers. t4 holds the JS program counter, so use
#  with caution in opcodes (actually, don't use it in opcodes at all, except as PC).
#
#  - r0 and r1 are the platform's customary return registers, and thus are
#  two distinct registers
#
#  - a0, a1, a2 and a3 are the platform's customary argument registers, and
#  thus are pairwise distinct registers. Be mindful that:
#    + On X86, there are no argument registers. a0 and a1 are edx and
#    ecx following the fastcall convention, but you should still use the stack
#    to pass your arguments. The cCall2 and cCall4 macros do this for you.
#    + On X86_64_WIN, you should allocate space on the stack for the arguments,
#    and the return convention is weird for > 8 bytes types. The only place we
#    use > 8 bytes return values is on a cCall, and cCall2 and cCall4 handle
#    this for you.
#
#  - The only registers guaranteed to be caller-saved are r0, r1, a0, a1 and a2, and
#  you should be mindful of that in functions that are called directly from C.
#  If you need more registers, you should push and pop them like a good
#  assembly citizen, because any other register will be callee-saved on X86.
#
# You can additionally assume:
#
#  - a3, t2, t3, t4 and t5 are never return registers; t0, t1, a0, a1 and a2
#  can be return registers.
#
#  - t4 and t5 are never argument registers, t3 can only be a3, t1 can only be
#  a1; but t0 and t2 can be either a0 or a2.
#
#  - On 64 bits, there are callee-save registers named csr0, csr1, ... csrN.
#  The last three csr registers are used used to store the PC base and
#  two special tag values. Don't use them for anything else.
#
# Additional platform-specific details (you shouldn't rely on this remaining
# true):
#
#  - For consistency with the baseline JIT, t0 is always r0 (and t1 is always
#  r1 on 32 bits platforms). You should use the r version when you need return
#  registers, and the t version otherwise: code using t0 (or t1) should still
#  work if swapped with e.g. t3, while code using r0 (or r1) should not. There
#  *may* be legacy code relying on this.
#
#  - On all platforms other than X86, t0 can only be a0 and t2 can only be a2.
#
#  - On all platforms other than X86 and X86_64, a2 is not a return register.
#  a2 is r0 on X86 (because we have so few registers) and r1 on X86_64 (because
#  the ABI enforces it).
#
# The following floating-point registers are available:
#
#  - ft0-ft5 are temporary floating-point registers that get trashed on calls,
#  and are pairwise distinct.
#
#  - fa0 and fa1 are the platform's customary floating-point argument
#  registers, and are both distinct. On 64-bits platforms, fa2 and fa3 are
#  additional floating-point argument registers.
#
#  - fr is the platform's customary floating-point return register
#
# You can assume that ft1-ft5 or fa1-fa3 are never fr, and that ftX is never
# faY if X != Y.

# First come the common protocols that both interpreters use. Note that each
# of these must have an ASSERT() in LLIntData.cpp

# Work-around for the fact that the toolchain's awareness of armv7k / armv7s
# results in a separate slab in the fat binary, yet the offlineasm doesn't know
# to expect it.
if ARMv7k
end
if ARMv7s
end

# These declarations must match interpreter/JSStack.h.

if JSVALUE64
    const PtrSize = 8
    const CallFrameHeaderSlots = 5
else
    const PtrSize = 4
    const CallFrameHeaderSlots = 4
    const CallFrameAlignSlots = 1
end
const SlotSize = 8

const JSEnvironmentRecord_variables = (sizeof JSEnvironmentRecord + SlotSize - 1) & ~(SlotSize - 1)
const DirectArguments_storage = (sizeof DirectArguments + SlotSize - 1) & ~(SlotSize - 1)

const StackAlignment = 16
const StackAlignmentSlots = 2
const StackAlignmentMask = StackAlignment - 1

const CallerFrameAndPCSize = 2 * PtrSize

const CallerFrame = 0
const ReturnPC = CallerFrame + PtrSize
const CodeBlock = ReturnPC + PtrSize
const Callee = CodeBlock + SlotSize
const ArgumentCount = Callee + SlotSize
const ThisArgumentOffset = ArgumentCount + SlotSize
const FirstArgumentOffset = ThisArgumentOffset + SlotSize
const CallFrameHeaderSize = ThisArgumentOffset

# Some value representation constants.
if JSVALUE64
    const TagBitTypeOther = 0x2
    const TagBitBool      = 0x4
    const TagBitUndefined = 0x8
    const ValueEmpty      = 0x0
    const ValueFalse      = TagBitTypeOther | TagBitBool
    const ValueTrue       = TagBitTypeOther | TagBitBool | 1
    const ValueUndefined  = TagBitTypeOther | TagBitUndefined
    const ValueNull       = TagBitTypeOther
    const TagTypeNumber   = 0xffff000000000000
    const TagMask         = TagTypeNumber | TagBitTypeOther
else
    const Int32Tag = -1
    const BooleanTag = -2
    const NullTag = -3
    const UndefinedTag = -4
    const CellTag = -5
    const EmptyValueTag = -6
    const DeletedValueTag = -7
    const LowestTag = DeletedValueTag
end

# NOTE: The values below must be in sync with what is in PutByIdFlags.h.
const PutByIdPrimaryTypeMask = 0x6
const PutByIdPrimaryTypeSecondary = 0x0
const PutByIdPrimaryTypeObjectWithStructure = 0x2
const PutByIdPrimaryTypeObjectWithStructureOrOther = 0x4
const PutByIdSecondaryTypeMask = -0x8
const PutByIdSecondaryTypeBottom = 0x0
const PutByIdSecondaryTypeBoolean = 0x8
const PutByIdSecondaryTypeOther = 0x10
const PutByIdSecondaryTypeInt32 = 0x18
const PutByIdSecondaryTypeNumber = 0x20
const PutByIdSecondaryTypeString = 0x28
const PutByIdSecondaryTypeSymbol = 0x30
const PutByIdSecondaryTypeObject = 0x38
const PutByIdSecondaryTypeObjectOrOther = 0x40
const PutByIdSecondaryTypeTop = 0x48

const CallOpCodeSize = 9

if X86_64 or ARM64 or C_LOOP
    const maxFrameExtentForSlowPathCall = 0
elsif ARM or ARMv7_TRADITIONAL or ARMv7
    const maxFrameExtentForSlowPathCall = 24
elsif X86 or X86_WIN
    const maxFrameExtentForSlowPathCall = 40
elsif MIPS
    const maxFrameExtentForSlowPathCall = 40
elsif X86_64_WIN
    const maxFrameExtentForSlowPathCall = 64
end

if X86_64 or X86_64_WIN or ARM64
    const CalleeSaveSpaceAsVirtualRegisters = 3
else
    const CalleeSaveSpaceAsVirtualRegisters = 0
end

const CalleeSaveSpaceStackAligned = (CalleeSaveSpaceAsVirtualRegisters * SlotSize + StackAlignment - 1) & ~StackAlignmentMask


# Watchpoint states
const ClearWatchpoint = 0
const IsWatched = 1
const IsInvalidated = 2

# ShadowChicken data
const ShadowChickenTailMarker = 0x7a11

# ArithProfile data
const ArithProfileInt = 0x100000
const ArithProfileIntInt = 0x120000
const ArithProfileNumber = 0x200000
const ArithProfileNumberInt = 0x220000
const ArithProfileNumberNumber = 0x240000
const ArithProfileIntNumber = 0x140000

# Some register conventions.
if JSVALUE64
    # - Use a pair of registers to represent the PC: one register for the
    #   base of the bytecodes, and one register for the index.
    # - The PC base (or PB for short) must be stored in a callee-save register.
    # - C calls are still given the Instruction* rather than the PC index.
    #   This requires an add before the call, and a sub after.
    const PC = t4 # When changing this, make sure LLIntPC is up to date in LLIntPCRanges.h
    if ARM64
        const PB = csr7
        const tagTypeNumber = csr8
        const tagMask = csr9
    elsif X86_64
        const PB = csr2
        const tagTypeNumber = csr3
        const tagMask = csr4
    elsif X86_64_WIN
        const PB = csr4
        const tagTypeNumber = csr5
        const tagMask = csr6
    elsif C_LOOP
        const PB = csr0
        const tagTypeNumber = csr1
        const tagMask = csr2
    end

    macro loadisFromInstruction(offset, dest)
        loadis offset * 8[PB, PC, 8], dest
    end
    
    macro loadpFromInstruction(offset, dest)
        loadp offset * 8[PB, PC, 8], dest
    end
    
    macro storeisToInstruction(value, offset)
        storei value, offset * 8[PB, PC, 8]
    end

    macro storepToInstruction(value, offset)
        storep value, offset * 8[PB, PC, 8]
    end

else
    const PC = t4 # When changing this, make sure LLIntPC is up to date in LLIntPCRanges.h
    macro loadisFromInstruction(offset, dest)
        loadis offset * 4[PC], dest
    end
    
    macro loadpFromInstruction(offset, dest)
        loadp offset * 4[PC], dest
    end

    macro storeisToInstruction(value, offset)
        storei value, offset * 4[PC]
    end
end

if X86_64_WIN
    const extraTempReg = t0
else
    const extraTempReg = t5
end

# Constants for reasoning about value representation.
if BIG_ENDIAN
    const TagOffset = 0
    const PayloadOffset = 4
else
    const TagOffset = 4
    const PayloadOffset = 0
end

# Constant for reasoning about butterflies.
const IsArray                  = 0x01
const IndexingShapeMask        = 0x0E
const NoIndexingShape          = 0x00
const Int32Shape               = 0x04
const DoubleShape              = 0x06
const ContiguousShape          = 0x08
const ArrayStorageShape        = 0x0A
const SlowPutArrayStorageShape = 0x0C

# Type constants.
const StringType = 6
const SymbolType = 7
const ObjectType = 23
const FinalObjectType = 24
const JSFunctionType = 26
const ArrayType = 34
const DerivedArrayType = 35
const ProxyObjectType = 53

# The typed array types need to be numbered in a particular order because of the manually written
# switch statement in get_by_val and put_by_val.
const Int8ArrayType = 36
const Int16ArrayType = 37
const Int32ArrayType = 38
const Uint8ArrayType = 39
const Uint8ClampedArrayType = 40
const Uint16ArrayType = 41
const Uint32ArrayType = 42
const Float32ArrayType = 43
const Float64ArrayType = 44

const FirstArrayType = Int8ArrayType
const LastArrayType = Float64ArrayType

# Type flags constants.
const MasqueradesAsUndefined = 1
const ImplementsDefaultHasInstance = 2

# Bytecode operand constants.
const FirstConstantRegisterIndex = 0x40000000

# Code type constants.
const GlobalCode = 0
const EvalCode = 1
const FunctionCode = 2
const ModuleCode = 3

# The interpreter steals the tag word of the argument count.
const LLIntReturnPC = ArgumentCount + TagOffset

# String flags.
const HashFlags8BitBuffer = 8

# Copied from PropertyOffset.h
const firstOutOfLineOffset = 100

# ResolveType
const GlobalProperty = 0
const GlobalVar = 1
const GlobalLexicalVar = 2
const ClosureVar = 3
const LocalClosureVar = 4
const ModuleVar = 5
const GlobalPropertyWithVarInjectionChecks = 6
const GlobalVarWithVarInjectionChecks = 7
const GlobalLexicalVarWithVarInjectionChecks = 8
const ClosureVarWithVarInjectionChecks = 9

const ResolveTypeMask = 0x3ff
const InitializationModeMask = 0xffc00
const InitializationModeShift = 10
const NotInitialization = 2

const MarkedBlockSize = 16 * 1024
const MarkedBlockMask = ~(MarkedBlockSize - 1)

const BlackThreshold = 0

# Allocation constants
if JSVALUE64
    const JSFinalObjectSizeClassIndex = 1
else
    const JSFinalObjectSizeClassIndex = 3
end

# This must match wtf/Vector.h
const VectorBufferOffset = 0
if JSVALUE64
    const VectorSizeOffset = 12
else
    const VectorSizeOffset = 8
end

# Some common utilities.
macro crash()
    if C_LOOP
        cloopCrash
    else
        call _llint_crash
    end
end

macro assert(assertion)
    if ASSERT_ENABLED
        assertion(.ok)
        crash()
    .ok:
    end
end

# The probe macro can be used to insert some debugging code without perturbing scalar
# registers. Presently, the probe macro only preserves scalar registers. Hence, the
# C probe callback function should not trash floating point registers.
#
# The macro you pass to probe() can pass whatever registers you like to your probe
# callback function. However, you need to be mindful of which of the registers are
# also used as argument registers, and ensure that you don't trash the register value
# before storing it in the probe callback argument register that you desire.
#
# Here's an example of how it's used:
#
#     probe(
#         macro()
#             move cfr, a0 # pass the ExecState* as arg0.
#             move t0, a1 # pass the value of register t0 as arg1.
#             call _cProbeCallbackFunction # to do whatever you want.
#         end
#     )
#
if X86_64
    macro probe(action)
        # save all the registers that the LLInt may use.
        push a0, a1
        push a2, a3
        push t0, t1
        push t2, t3
        push t4, t5

        action()

        # restore all the registers we saved previously.
        pop t5, t4
        pop t3, t2
        pop t1, t0
        pop a3, a2
        pop a1, a0
    end
end

macro checkStackPointerAlignment(tempReg, location)
    if ARM64 or C_LOOP
        # ARM64 will check for us!
        # C_LOOP does not need the alignment, and can use a little perf
        # improvement from avoiding useless work.
    else
        if ARM or ARMv7 or ARMv7_TRADITIONAL
            # ARM can't do logical ops with the sp as a source
            move sp, tempReg
            andp StackAlignmentMask, tempReg
        else
            andp sp, StackAlignmentMask, tempReg
        end
        btpz tempReg, .stackPointerOkay
        move location, tempReg
        break
    .stackPointerOkay:
    end
end

if C_LOOP or ARM64 or X86_64 or X86_64_WIN
    const CalleeSaveRegisterCount = 0
elsif ARM or ARMv7_TRADITIONAL or ARMv7
    const CalleeSaveRegisterCount = 7
elsif MIPS
    const CalleeSaveRegisterCount = 1
elsif X86 or X86_WIN
    const CalleeSaveRegisterCount = 3
end

const CalleeRegisterSaveSize = CalleeSaveRegisterCount * PtrSize

# VMEntryTotalFrameSize includes the space for struct VMEntryRecord and the
# callee save registers rounded up to keep the stack aligned
const VMEntryTotalFrameSize = (CalleeRegisterSaveSize + sizeof VMEntryRecord + StackAlignment - 1) & ~StackAlignmentMask

macro pushCalleeSaves()
    if C_LOOP or ARM64 or X86_64 or X86_64_WIN
    elsif ARM or ARMv7_TRADITIONAL
        emit "push {r4-r10}"
    elsif ARMv7
        emit "push {r4-r6, r8-r11}"
    elsif MIPS
        emit "addiu $sp, $sp, -4"
        emit "sw $s4, 0($sp)"
        # save $gp to $s4 so that we can restore it after a function call
        emit "move $s4, $gp"
    elsif X86
        emit "push %esi"
        emit "push %edi"
        emit "push %ebx"
    elsif X86_WIN
        emit "push esi"
        emit "push edi"
        emit "push ebx"
    end
end

macro popCalleeSaves()
    if C_LOOP or ARM64 or X86_64 or X86_64_WIN
    elsif ARM or ARMv7_TRADITIONAL
        emit "pop {r4-r10}"
    elsif ARMv7
        emit "pop {r4-r6, r8-r11}"
    elsif MIPS
        emit "lw $s4, 0($sp)"
        emit "addiu $sp, $sp, 4"
    elsif X86
        emit "pop %ebx"
        emit "pop %edi"
        emit "pop %esi"
    elsif X86_WIN
        emit "pop ebx"
        emit "pop edi"
        emit "pop esi"
    end
end

macro preserveCallerPCAndCFR()
    if C_LOOP or ARM or ARMv7 or ARMv7_TRADITIONAL or MIPS
        push lr
        push cfr
    elsif X86 or X86_WIN or X86_64 or X86_64_WIN
        push cfr
    elsif ARM64
        push cfr, lr
    else
        error
    end
    move sp, cfr
end

macro restoreCallerPCAndCFR()
    move cfr, sp
    if C_LOOP or ARM or ARMv7 or ARMv7_TRADITIONAL or MIPS
        pop cfr
        pop lr
    elsif X86 or X86_WIN or X86_64 or X86_64_WIN
        pop cfr
    elsif ARM64
        pop lr, cfr
    end
end

macro preserveCalleeSavesUsedByLLInt()
    subp CalleeSaveSpaceStackAligned, sp
    if C_LOOP
    elsif ARM or ARMv7_TRADITIONAL
    elsif ARMv7
    elsif ARM64
        emit "stp x27, x28, [x29, #-16]"
        emit "stp xzr, x26, [x29, #-32]"
    elsif MIPS
    elsif X86
    elsif X86_WIN
    elsif X86_64
        storep csr4, -8[cfr]
        storep csr3, -16[cfr]
        storep csr2, -24[cfr]
    elsif X86_64_WIN
        storep csr6, -8[cfr]
        storep csr5, -16[cfr]
        storep csr4, -24[cfr]
    end
end

macro restoreCalleeSavesUsedByLLInt()
    if C_LOOP
    elsif ARM or ARMv7_TRADITIONAL
    elsif ARMv7
    elsif ARM64
        emit "ldp xzr, x26, [x29, #-32]"
        emit "ldp x27, x28, [x29, #-16]"
    elsif MIPS
    elsif X86
    elsif X86_WIN
    elsif X86_64
        loadp -24[cfr], csr2
        loadp -16[cfr], csr3
        loadp -8[cfr], csr4
    elsif X86_64_WIN
        loadp -24[cfr], csr4
        loadp -16[cfr], csr5
        loadp -8[cfr], csr6
    end
end

macro copyCalleeSavesToVMEntryFrameCalleeSavesBuffer(vm, temp)
    if ARM64 or X86_64 or X86_64_WIN
        loadp VM::topVMEntryFrame[vm], temp
        vmEntryRecord(temp, temp)
        leap VMEntryRecord::calleeSaveRegistersBuffer[temp], temp
        if ARM64
            storep csr0, [temp]
            storep csr1, 8[temp]
            storep csr2, 16[temp]
            storep csr3, 24[temp]
            storep csr4, 32[temp]
            storep csr5, 40[temp]
            storep csr6, 48[temp]
            storep csr7, 56[temp]
            storep csr8, 64[temp]
            storep csr9, 72[temp]
            stored csfr0, 80[temp]
            stored csfr1, 88[temp]
            stored csfr2, 96[temp]
            stored csfr3, 104[temp]
            stored csfr4, 112[temp]
            stored csfr5, 120[temp]
            stored csfr6, 128[temp]
            stored csfr7, 136[temp]
        elsif X86_64
            storep csr0, [temp]
            storep csr1, 8[temp]
            storep csr2, 16[temp]
            storep csr3, 24[temp]
            storep csr4, 32[temp]
        elsif X86_64_WIN
            storep csr0, [temp]
            storep csr1, 8[temp]
            storep csr2, 16[temp]
            storep csr3, 24[temp]
            storep csr4, 32[temp]
            storep csr5, 40[temp]
            storep csr6, 48[temp]
        end
    end
end

macro restoreCalleeSavesFromVMEntryFrameCalleeSavesBuffer(vm, temp)
    if ARM64 or X86_64 or X86_64_WIN
        loadp VM::topVMEntryFrame[vm], temp
        vmEntryRecord(temp, temp)
        leap VMEntryRecord::calleeSaveRegistersBuffer[temp], temp
        if ARM64
            loadp [temp], csr0
            loadp 8[temp], csr1
            loadp 16[temp], csr2
            loadp 24[temp], csr3
            loadp 32[temp], csr4
            loadp 40[temp], csr5
            loadp 48[temp], csr6
            loadp 56[temp], csr7
            loadp 64[temp], csr8
            loadp 72[temp], csr9
            loadd 80[temp], csfr0
            loadd 88[temp], csfr1
            loadd 96[temp], csfr2
            loadd 104[temp], csfr3
            loadd 112[temp], csfr4
            loadd 120[temp], csfr5
            loadd 128[temp], csfr6
            loadd 136[temp], csfr7
        elsif X86_64
            loadp [temp], csr0
            loadp 8[temp], csr1
            loadp 16[temp], csr2
            loadp 24[temp], csr3
            loadp 32[temp], csr4
        elsif X86_64_WIN
            loadp [temp], csr0
            loadp 8[temp], csr1
            loadp 16[temp], csr2
            loadp 24[temp], csr3
            loadp 32[temp], csr4
            loadp 40[temp], csr5
            loadp 48[temp], csr6
        end
    end
end

macro preserveReturnAddressAfterCall(destinationRegister)
    if C_LOOP or ARM or ARMv7 or ARMv7_TRADITIONAL or ARM64 or MIPS
        # In C_LOOP case, we're only preserving the bytecode vPC.
        move lr, destinationRegister
    elsif X86 or X86_WIN or X86_64 or X86_64_WIN
        pop destinationRegister
    else
        error
    end
end

macro functionPrologue()
    if X86 or X86_WIN or X86_64 or X86_64_WIN
        push cfr
    elsif ARM64
        push cfr, lr
    elsif C_LOOP or ARM or ARMv7 or ARMv7_TRADITIONAL or MIPS
        push lr
        push cfr
    end
    move sp, cfr
end

macro functionEpilogue()
    if X86 or X86_WIN or X86_64 or X86_64_WIN
        pop cfr
    elsif ARM64
        pop lr, cfr
    elsif C_LOOP or ARM or ARMv7 or ARMv7_TRADITIONAL or MIPS
        pop cfr
        pop lr
    end
end

macro vmEntryRecord(entryFramePointer, resultReg)
    subp entryFramePointer, VMEntryTotalFrameSize, resultReg
end

macro getFrameRegisterSizeForCodeBlock(codeBlock, size)
    loadi CodeBlock::m_numCalleeLocals[codeBlock], size
    lshiftp 3, size
    addp maxFrameExtentForSlowPathCall, size
end

macro restoreStackPointerAfterCall()
    loadp CodeBlock[cfr], t2
    getFrameRegisterSizeForCodeBlock(t2, t2)
    if ARMv7
        subp cfr, t2, t2
        move t2, sp
    else
        subp cfr, t2, sp
    end
end

macro traceExecution()
    if COLLECT_STATS
        callSlowPath(_llint_count_opcode)
    end
    if EXECUTION_TRACING
        callSlowPath(_llint_trace)
    end
end

macro traceSlowPathExecution()
    if COLLECT_STATS
        callSlowPath(_llint_count_opcode_slow_path)
    end
end

macro callOpcodeSlowPath(slowPath)
    traceSlowPathExecution()
    callSlowPath(slowPath)
end

macro callTargetFunction(callee)
    if C_LOOP
        cloopCallJSFunction callee
    else
        call callee
    end
    restoreStackPointerAfterCall()
    dispatchAfterCall()
end

macro prepareForRegularCall(callee, temp1, temp2, temp3)
    addp CallerFrameAndPCSize, sp
end

# sp points to the new frame
macro prepareForTailCall(callee, temp1, temp2, temp3)
    restoreCalleeSavesUsedByLLInt()

    loadi PayloadOffset + ArgumentCount[cfr], temp2
    loadp CodeBlock[cfr], temp1
    loadp CodeBlock::m_numParameters[temp1], temp1
    bilteq temp1, temp2, .noArityFixup
    move temp1, temp2

.noArityFixup:
    # We assume < 2^28 arguments
    muli SlotSize, temp2
    addi StackAlignment - 1 + CallFrameHeaderSize, temp2
    andi ~StackAlignmentMask, temp2

    move cfr, temp1
    addp temp2, temp1

    loadi PayloadOffset + ArgumentCount[sp], temp2
    # We assume < 2^28 arguments
    muli SlotSize, temp2
    addi StackAlignment - 1 + CallFrameHeaderSize, temp2
    andi ~StackAlignmentMask, temp2

    if ARM or ARMv7_TRADITIONAL or ARMv7 or ARM64 or C_LOOP or MIPS
        addp 2 * PtrSize, sp
        subi 2 * PtrSize, temp2
        loadp PtrSize[cfr], lr
    else
        addp PtrSize, sp
        subi PtrSize, temp2
        loadp PtrSize[cfr], temp3
        storep temp3, [sp]
    end

    subp temp2, temp1
    loadp [cfr], cfr

.copyLoop:
    subi PtrSize, temp2
    loadp [sp, temp2, 1], temp3
    storep temp3, [temp1, temp2, 1]
    btinz temp2, .copyLoop

    move temp1, sp
    jmp callee
end

macro slowPathForCall(slowPath, prepareCall)
    traceSlowPathExecution()
    callCallSlowPath(
        slowPath,
        # Those are r0 and r1
        macro (callee, calleeFramePtr)
            btpz calleeFramePtr, .dontUpdateSP
            move calleeFramePtr, sp
            prepareCall(callee, t2, t3, t4)
        .dontUpdateSP:
            callTargetFunction(callee)
        end)
end

macro arrayProfile(cellAndIndexingType, profile, scratch)
    const cell = cellAndIndexingType
    const indexingType = cellAndIndexingType 
    loadi JSCell::m_structureID[cell], scratch
    storei scratch, ArrayProfile::m_lastSeenStructureID[profile]
    loadb JSCell::m_indexingTypeAndMisc[cell], indexingType
end

macro skipIfIsRememberedOrInEden(cell, slowPath)
    memfence
    bba JSCell::m_cellState[cell], BlackThreshold, .done
    slowPath()
.done:
end

macro notifyWrite(set, slow)
    bbneq WatchpointSet::m_state[set], IsInvalidated, slow
end

macro checkSwitchToJIT(increment, action)
    loadp CodeBlock[cfr], t0
    baddis increment, CodeBlock::m_llintExecuteCounter + BaselineExecutionCounter::m_counter[t0], .continue
    action()
    .continue:
end

macro checkSwitchToJITForEpilogue()
    checkSwitchToJIT(
        10,
        macro ()
            callOpcodeSlowPath(_llint_replace)
        end)
end

macro assertNotConstant(index)
    assert(macro (ok) bilt index, FirstConstantRegisterIndex, ok end)
end

macro functionForCallCodeBlockGetter(targetRegister)
    if JSVALUE64
        loadp Callee[cfr], targetRegister
    else
        loadp Callee + PayloadOffset[cfr], targetRegister
    end
    loadp JSFunction::m_executable[targetRegister], targetRegister
    loadp FunctionExecutable::m_codeBlockForCall[targetRegister], targetRegister
end

macro functionForConstructCodeBlockGetter(targetRegister)
    if JSVALUE64
        loadp Callee[cfr], targetRegister
    else
        loadp Callee + PayloadOffset[cfr], targetRegister
    end
    loadp JSFunction::m_executable[targetRegister], targetRegister
    loadp FunctionExecutable::m_codeBlockForConstruct[targetRegister], targetRegister
end

macro notFunctionCodeBlockGetter(targetRegister)
    loadp CodeBlock[cfr], targetRegister
end

macro functionCodeBlockSetter(sourceRegister)
    storep sourceRegister, CodeBlock[cfr]
end

macro notFunctionCodeBlockSetter(sourceRegister)
    # Nothing to do!
end

# Do the bare minimum required to execute code. Sets up the PC, leave the CodeBlock*
# in t1. May also trigger prologue entry OSR.
macro prologue(codeBlockGetter, codeBlockSetter, osrSlowPath, traceSlowPath)
    # Set up the call frame and check if we should OSR.
    preserveCallerPCAndCFR()

    if EXECUTION_TRACING
        subp maxFrameExtentForSlowPathCall, sp
        callSlowPath(traceSlowPath)
        addp maxFrameExtentForSlowPathCall, sp
    end
    codeBlockGetter(t1)
    if not C_LOOP
        baddis 5, CodeBlock::m_llintExecuteCounter + BaselineExecutionCounter::m_counter[t1], .continue
        if JSVALUE64
            move cfr, a0
            move PC, a1
            cCall2(osrSlowPath)
        else
            # We are after the function prologue, but before we have set up sp from the CodeBlock.
            # Temporarily align stack pointer for this call.
            subp 8, sp
            move cfr, a0
            move PC, a1
            cCall2(osrSlowPath)
            addp 8, sp
        end
        btpz r0, .recover
        move cfr, sp # restore the previous sp
        # pop the callerFrame since we will jump to a function that wants to save it
        if ARM64
            pop lr, cfr
        elsif ARM or ARMv7 or ARMv7_TRADITIONAL or MIPS
            pop cfr
            pop lr
        else
            pop cfr
        end
        jmp r0
    .recover:
        codeBlockGetter(t1)
    .continue:
    end

    codeBlockSetter(t1)

    preserveCalleeSavesUsedByLLInt()

    # Set up the PC.
    if JSVALUE64
        loadp CodeBlock::m_instructions[t1], PB
        move 0, PC
    else
        loadp CodeBlock::m_instructions[t1], PC
    end

    # Get new sp in t0 and check stack height.
    getFrameRegisterSizeForCodeBlock(t1, t0)
    subp cfr, t0, t0
    loadp CodeBlock::m_vm[t1], t2
    if C_LOOP
        bpbeq VM::m_cloopStackLimit[t2], t0, .stackHeightOK
    else
        bpbeq VM::m_softStackLimit[t2], t0, .stackHeightOK
    end

    # Stack height check failed - need to call a slow_path.
    # Set up temporary stack pointer for call including callee saves
    subp maxFrameExtentForSlowPathCall, sp
    callSlowPath(_llint_stack_check)
    bpeq r1, 0, .stackHeightOKGetCodeBlock
    move r1, cfr
    dispatch(0) # Go to exception handler in PC

.stackHeightOKGetCodeBlock:
    # Stack check slow path returned that the stack was ok.
    # Since they were clobbered, need to get CodeBlock and new sp
    codeBlockGetter(t1)
    getFrameRegisterSizeForCodeBlock(t1, t0)
    subp cfr, t0, t0

.stackHeightOK:
    move t0, sp

    if JSVALUE64
        move TagTypeNumber, tagTypeNumber
        addp TagBitTypeOther, tagTypeNumber, tagMask
    end
end

# Expects that CodeBlock is in t1, which is what prologue() leaves behind.
# Must call dispatch(0) after calling this.
macro functionInitialization(profileArgSkip)
    # Profile the arguments. Unfortunately, we have no choice but to do this. This
    # code is pretty horrendous because of the difference in ordering between
    # arguments and value profiles, the desire to have a simple loop-down-to-zero
    # loop, and the desire to use only three registers so as to preserve the PC and
    # the code block. It is likely that this code should be rewritten in a more
    # optimal way for architectures that have more than five registers available
    # for arbitrary use in the interpreter.
    loadi CodeBlock::m_numParameters[t1], t0
    addp -profileArgSkip, t0 # Use addi because that's what has the peephole
    assert(macro (ok) bpgteq t0, 0, ok end)
    btpz t0, .argumentProfileDone
    loadp CodeBlock::m_argumentValueProfiles + VectorBufferOffset[t1], t3
    mulp sizeof ValueProfile, t0, t2 # Aaaaahhhh! Need strength reduction!
    lshiftp 3, t0
    addp t2, t3
.argumentProfileLoop:
    if JSVALUE64
        loadq ThisArgumentOffset - 8 + profileArgSkip * 8[cfr, t0], t2
        subp sizeof ValueProfile, t3
        storeq t2, profileArgSkip * sizeof ValueProfile + ValueProfile::m_buckets[t3]
    else
        loadi ThisArgumentOffset + TagOffset - 8 + profileArgSkip * 8[cfr, t0], t2
        subp sizeof ValueProfile, t3
        storei t2, profileArgSkip * sizeof ValueProfile + ValueProfile::m_buckets + TagOffset[t3]
        loadi ThisArgumentOffset + PayloadOffset - 8 + profileArgSkip * 8[cfr, t0], t2
        storei t2, profileArgSkip * sizeof ValueProfile + ValueProfile::m_buckets + PayloadOffset[t3]
    end
    baddpnz -8, t0, .argumentProfileLoop
.argumentProfileDone:
end

macro doReturn()
    restoreCalleeSavesUsedByLLInt()
    restoreCallerPCAndCFR()
    ret
end

# stub to call into JavaScript or Native functions
# EncodedJSValue vmEntryToJavaScript(void* code, VM* vm, ProtoCallFrame* protoFrame)
# EncodedJSValue vmEntryToNativeFunction(void* code, VM* vm, ProtoCallFrame* protoFrame)

if C_LOOP
    _llint_vm_entry_to_javascript:
else
    global _vmEntryToJavaScript
    _vmEntryToJavaScript:
end
    doVMEntry(makeJavaScriptCall)


if C_LOOP
    _llint_vm_entry_to_native:
else
    global _vmEntryToNative
    _vmEntryToNative:
end
    doVMEntry(makeHostFunctionCall)


if not C_LOOP
    # void sanitizeStackForVMImpl(VM* vm)
    global _sanitizeStackForVMImpl
    _sanitizeStackForVMImpl:
        # We need three non-aliased caller-save registers. We are guaranteed
        # this for a0, a1 and a2 on all architectures.
        if X86 or X86_WIN
            loadp 4[sp], a0
        end
        const vm = a0
        const address = a1
        const zeroValue = a2
    
        loadp VM::m_lastStackTop[vm], address
        bpbeq sp, address, .zeroFillDone
    
        move 0, zeroValue
    .zeroFillLoop:
        storep zeroValue, [address]
        addp PtrSize, address
        bpa sp, address, .zeroFillLoop
    
    .zeroFillDone:
        move sp, address
        storep address, VM::m_lastStackTop[vm]
        ret
    
    # VMEntryRecord* vmEntryRecord(const VMEntryFrame* entryFrame)
    global _vmEntryRecord
    _vmEntryRecord:
        if X86 or X86_WIN
            loadp 4[sp], a0
        end

        vmEntryRecord(a0, r0)
        ret
end

if C_LOOP
    # Dummy entry point the C Loop uses to initialize.
    _llint_entry:
        crash()
else
    macro initPCRelative(pcBase)
        if X86_64 or X86_64_WIN or X86 or X86_WIN
            call _relativePCBase
        _relativePCBase:
            pop pcBase
        elsif ARM64
        elsif ARMv7
        _relativePCBase:
            move pc, pcBase
            subp 3, pcBase   # Need to back up the PC and set the Thumb2 bit
        elsif ARM or ARMv7_TRADITIONAL
        _relativePCBase:
            move pc, pcBase
            subp 8, pcBase
        elsif MIPS
            la _relativePCBase, pcBase
            setcallreg pcBase # needed to set $t9 to the right value for the .cpload created by the label.
        _relativePCBase:
        end
end

# The PC base is in t1, as this is what _llint_entry leaves behind through
# initPCRelative(t1)
macro setEntryAddress(index, label)
    if X86_64 or X86_64_WIN
        leap (label - _relativePCBase)[t1], t3
        move index, t4
        storep t3, [a0, t4, 8]
    elsif X86 or X86_WIN
        leap (label - _relativePCBase)[t1], t3
        move index, t4
        storep t3, [a0, t4, 4]
    elsif ARM64
        pcrtoaddr label, t1
        move index, t4
        storep t1, [a0, t4, 8]
    elsif ARM or ARMv7 or ARMv7_TRADITIONAL
        mvlbl (label - _relativePCBase), t4
        addp t4, t1, t4
        move index, t3
        storep t4, [a0, t3, 4]
    elsif MIPS
        la label, t4
        la _relativePCBase, t3
        subp t3, t4
        addp t4, t1, t4
        move index, t3
        storep t4, [a0, t3, 4]
    end
end

global _llint_entry
# Entry point for the llint to initialize.
_llint_entry:
    functionPrologue()
    pushCalleeSaves()
    if X86 or X86_WIN
        loadp 20[sp], a0
    end
    initPCRelative(t1)

    # Include generated bytecode initialization file.
    include InitBytecodes

    popCalleeSaves()
    functionEpilogue()
    ret
end

_llint_program_prologue:
    prologue(notFunctionCodeBlockGetter, notFunctionCodeBlockSetter, _llint_entry_osr, _llint_trace_prologue)
    dispatch(0)


_llint_module_program_prologue:
    prologue(notFunctionCodeBlockGetter, notFunctionCodeBlockSetter, _llint_entry_osr, _llint_trace_prologue)
    dispatch(0)


_llint_eval_prologue:
    prologue(notFunctionCodeBlockGetter, notFunctionCodeBlockSetter, _llint_entry_osr, _llint_trace_prologue)
    dispatch(0)


_llint_function_for_call_prologue:
    prologue(functionForCallCodeBlockGetter, functionCodeBlockSetter, _llint_entry_osr_function_for_call, _llint_trace_prologue_function_for_call)
    functionInitialization(0)
    dispatch(0)
    

_llint_function_for_construct_prologue:
    prologue(functionForConstructCodeBlockGetter, functionCodeBlockSetter, _llint_entry_osr_function_for_construct, _llint_trace_prologue_function_for_construct)
    functionInitialization(1)
    dispatch(0)
    

_llint_function_for_call_arity_check:
    prologue(functionForCallCodeBlockGetter, functionCodeBlockSetter, _llint_entry_osr_function_for_call_arityCheck, _llint_trace_arityCheck_for_call)
    functionArityCheck(.functionForCallBegin, _slow_path_call_arityCheck)
.functionForCallBegin:
    functionInitialization(0)
    dispatch(0)


_llint_function_for_construct_arity_check:
    prologue(functionForConstructCodeBlockGetter, functionCodeBlockSetter, _llint_entry_osr_function_for_construct_arityCheck, _llint_trace_arityCheck_for_construct)
    functionArityCheck(.functionForConstructBegin, _slow_path_construct_arityCheck)
.functionForConstructBegin:
    functionInitialization(1)
    dispatch(0)


# Value-representation-specific code.
if JSVALUE64
    include LowLevelInterpreter64
else
    include LowLevelInterpreter32_64
end


# Value-representation-agnostic code.
_llint_op_create_direct_arguments:
    traceExecution()
    callOpcodeSlowPath(_slow_path_create_direct_arguments)
    dispatch(2)


_llint_op_create_scoped_arguments:
    traceExecution()
    callOpcodeSlowPath(_slow_path_create_scoped_arguments)
    dispatch(3)


_llint_op_create_cloned_arguments:
    traceExecution()
    callOpcodeSlowPath(_slow_path_create_cloned_arguments)
    dispatch(2)


_llint_op_create_this:
    traceExecution()
    callOpcodeSlowPath(_slow_path_create_this)
    dispatch(5)


_llint_op_new_object:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_new_object)
    dispatch(4)


_llint_op_new_func:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_new_func)
    dispatch(4)


_llint_op_new_generator_func:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_new_generator_func)
    dispatch(4)


_llint_op_new_async_func:
    traceExecution()
    callSlowPath(_llint_slow_path_new_async_func)
    dispatch(4)


_llint_op_new_array:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_new_array)
    dispatch(5)


_llint_op_new_array_with_spread:
    traceExecution()
    callOpcodeSlowPath(_slow_path_new_array_with_spread)
    dispatch(5)


_llint_op_spread:
    traceExecution()
    callOpcodeSlowPath(_slow_path_spread)
    dispatch(3)


_llint_op_new_array_with_size:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_new_array_with_size)
    dispatch(4)


_llint_op_new_array_buffer:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_new_array_buffer)
    dispatch(5)


_llint_op_new_regexp:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_new_regexp)
    dispatch(3)


_llint_op_less:
    traceExecution()
    callOpcodeSlowPath(_slow_path_less)
    dispatch(4)


_llint_op_lesseq:
    traceExecution()
    callOpcodeSlowPath(_slow_path_lesseq)
    dispatch(4)


_llint_op_greater:
    traceExecution()
    callOpcodeSlowPath(_slow_path_greater)
    dispatch(4)


_llint_op_greatereq:
    traceExecution()
    callOpcodeSlowPath(_slow_path_greatereq)
    dispatch(4)


_llint_op_mod:
    traceExecution()
    callOpcodeSlowPath(_slow_path_mod)
    dispatch(4)


_llint_op_pow:
    traceExecution()
    callOpcodeSlowPath(_slow_path_pow)
    dispatch(4)


_llint_op_typeof:
    traceExecution()
    callOpcodeSlowPath(_slow_path_typeof)
    dispatch(3)


_llint_op_is_object_or_null:
    traceExecution()
    callOpcodeSlowPath(_slow_path_is_object_or_null)
    dispatch(3)

_llint_op_is_function:
    traceExecution()
    callOpcodeSlowPath(_slow_path_is_function)
    dispatch(3)


_llint_op_in:
    traceExecution()
    callOpcodeSlowPath(_slow_path_in)
    dispatch(5)


_llint_op_try_get_by_id:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_try_get_by_id)
    dispatch(5)


_llint_op_del_by_id:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_del_by_id)
    dispatch(4)


_llint_op_del_by_val:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_del_by_val)
    dispatch(4)


_llint_op_put_by_index:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_put_by_index)
    dispatch(4)


_llint_op_put_getter_by_id:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_put_getter_by_id)
    dispatch(5)


_llint_op_put_setter_by_id:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_put_setter_by_id)
    dispatch(5)


_llint_op_put_getter_setter_by_id:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_put_getter_setter_by_id)
    dispatch(6)


_llint_op_put_getter_by_val:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_put_getter_by_val)
    dispatch(5)


_llint_op_put_setter_by_val:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_put_setter_by_val)
    dispatch(5)


_llint_op_define_data_property:
    traceExecution()
    callOpcodeSlowPath(_slow_path_define_data_property)
    dispatch(5)


_llint_op_define_accessor_property:
    traceExecution()
    callOpcodeSlowPath(_slow_path_define_accessor_property)
    dispatch(6)


_llint_op_jtrue:
    traceExecution()
    jumpTrueOrFalse(
        macro (value, target) btinz value, target end,
        _llint_slow_path_jtrue)


_llint_op_jfalse:
    traceExecution()
    jumpTrueOrFalse(
        macro (value, target) btiz value, target end,
        _llint_slow_path_jfalse)


_llint_op_jless:
    traceExecution()
    compare(
        macro (left, right, target) bilt left, right, target end,
        macro (left, right, target) bdlt left, right, target end,
        _llint_slow_path_jless)


_llint_op_jnless:
    traceExecution()
    compare(
        macro (left, right, target) bigteq left, right, target end,
        macro (left, right, target) bdgtequn left, right, target end,
        _llint_slow_path_jnless)


_llint_op_jgreater:
    traceExecution()
    compare(
        macro (left, right, target) bigt left, right, target end,
        macro (left, right, target) bdgt left, right, target end,
        _llint_slow_path_jgreater)


_llint_op_jngreater:
    traceExecution()
    compare(
        macro (left, right, target) bilteq left, right, target end,
        macro (left, right, target) bdltequn left, right, target end,
        _llint_slow_path_jngreater)


_llint_op_jlesseq:
    traceExecution()
    compare(
        macro (left, right, target) bilteq left, right, target end,
        macro (left, right, target) bdlteq left, right, target end,
        _llint_slow_path_jlesseq)


_llint_op_jnlesseq:
    traceExecution()
    compare(
        macro (left, right, target) bigt left, right, target end,
        macro (left, right, target) bdgtun left, right, target end,
        _llint_slow_path_jnlesseq)


_llint_op_jgreatereq:
    traceExecution()
    compare(
        macro (left, right, target) bigteq left, right, target end,
        macro (left, right, target) bdgteq left, right, target end,
        _llint_slow_path_jgreatereq)


_llint_op_jngreatereq:
    traceExecution()
    compare(
        macro (left, right, target) bilt left, right, target end,
        macro (left, right, target) bdltun left, right, target end,
        _llint_slow_path_jngreatereq)


_llint_op_loop_hint:
    traceExecution()
    checkSwitchToJITForLoop()
    dispatch(1)


_llint_op_check_traps:
    traceExecution()
    loadp CodeBlock[cfr], t1
    loadp CodeBlock::m_vm[t1], t1
    loadb VM::m_traps+VMTraps::m_needTrapHandling[t1], t0
    btpnz t0, .handleTraps
.afterHandlingTraps:
    dispatch(1)
.handleTraps:
    callTrapHandler(.throwHandler)
    jmp .afterHandlingTraps
.throwHandler:
    jmp _llint_throw_from_slow_path_trampoline


# Returns the packet pointer in t0.
macro acquireShadowChickenPacket(slow)
    loadp CodeBlock[cfr], t1
    loadp CodeBlock::m_vm[t1], t1
    loadp VM::m_shadowChicken[t1], t2
    loadp ShadowChicken::m_logCursor[t2], t0
    bpaeq t0, ShadowChicken::m_logEnd[t2], slow
    addp sizeof ShadowChicken::Packet, t0, t1
    storep t1, ShadowChicken::m_logCursor[t2]
end


_llint_op_switch_string:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_switch_string)
    dispatch(0)


_llint_op_new_func_exp:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_new_func_exp)
    dispatch(4)

_llint_op_new_generator_func_exp:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_new_generator_func_exp)
    dispatch(4)

_llint_op_new_async_func_exp:
    traceExecution()
    callSlowPath(_llint_slow_path_new_async_func_exp)
    dispatch(4)


_llint_op_set_function_name:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_set_function_name)
    dispatch(3)

_llint_op_call:
    traceExecution()
    arrayProfileForCall()
    doCall(_llint_slow_path_call, prepareForRegularCall)

_llint_op_tail_call:
    traceExecution()
    arrayProfileForCall()
    checkSwitchToJITForEpilogue()
    doCall(_llint_slow_path_call, prepareForTailCall)

_llint_op_construct:
    traceExecution()
    doCall(_llint_slow_path_construct, prepareForRegularCall)

macro doCallVarargs(frameSlowPath, slowPath, prepareCall)
    callOpcodeSlowPath(frameSlowPath)
    branchIfException(_llint_throw_from_slow_path_trampoline)
    # calleeFrame in r1
    if JSVALUE64
        move r1, sp
    else
        # The calleeFrame is not stack aligned, move down by CallerFrameAndPCSize to align
        if ARMv7
            subp r1, CallerFrameAndPCSize, t2
            move t2, sp
        else
            subp r1, CallerFrameAndPCSize, sp
        end
    end
    slowPathForCall(slowPath, prepareCall)
end

_llint_op_call_varargs:
    traceExecution()
    doCallVarargs(_llint_slow_path_size_frame_for_varargs, _llint_slow_path_call_varargs, prepareForRegularCall)

_llint_op_tail_call_varargs:
    traceExecution()
    checkSwitchToJITForEpilogue()
    # We lie and perform the tail call instead of preparing it since we can't
    # prepare the frame for a call opcode
    doCallVarargs(_llint_slow_path_size_frame_for_varargs, _llint_slow_path_call_varargs, prepareForTailCall)


_llint_op_tail_call_forward_arguments:
    traceExecution()
    checkSwitchToJITForEpilogue()
    # We lie and perform the tail call instead of preparing it since we can't
    # prepare the frame for a call opcode
    doCallVarargs(_llint_slow_path_size_frame_for_forward_arguments, _llint_slow_path_tail_call_forward_arguments, prepareForTailCall)


_llint_op_construct_varargs:
    traceExecution()
    doCallVarargs(_llint_slow_path_size_frame_for_varargs, _llint_slow_path_construct_varargs, prepareForRegularCall)


_llint_op_call_eval:
    traceExecution()
    
    # Eval is executed in one of two modes:
    #
    # 1) We find that we're really invoking eval() in which case the
    #    execution is perfomed entirely inside the slow_path, and it
    #    returns the PC of a function that just returns the return value
    #    that the eval returned.
    #
    # 2) We find that we're invoking something called eval() that is not
    #    the real eval. Then the slow_path returns the PC of the thing to
    #    call, and we call it.
    #
    # This allows us to handle two cases, which would require a total of
    # up to four pieces of state that cannot be easily packed into two
    # registers (C functions can return up to two registers, easily):
    #
    # - The call frame register. This may or may not have been modified
    #   by the slow_path, but the convention is that it returns it. It's not
    #   totally clear if that's necessary, since the cfr is callee save.
    #   But that's our style in this here interpreter so we stick with it.
    #
    # - A bit to say if the slow_path successfully executed the eval and has
    #   the return value, or did not execute the eval but has a PC for us
    #   to call.
    #
    # - Either:
    #   - The JS return value (two registers), or
    #
    #   - The PC to call.
    #
    # It turns out to be easier to just always have this return the cfr
    # and a PC to call, and that PC may be a dummy thunk that just
    # returns the JS value that the eval returned.
    
    slowPathForCall(_llint_slow_path_call_eval, prepareForRegularCall)


_llint_generic_return_point:
    dispatchAfterCall()


_llint_op_strcat:
    traceExecution()
    callOpcodeSlowPath(_slow_path_strcat)
    dispatch(4)


_llint_op_push_with_scope:
    traceExecution()
    callOpcodeSlowPath(_slow_path_push_with_scope)
    dispatch(4)


_llint_op_assert:
    traceExecution()
    callOpcodeSlowPath(_slow_path_assert)
    dispatch(3)


_llint_op_yield:
    notSupported()


_llint_op_create_lexical_environment:
    traceExecution()
    callOpcodeSlowPath(_slow_path_create_lexical_environment)
    dispatch(5)


_llint_op_throw:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_throw)
    dispatch(2)


_llint_op_throw_static_error:
    traceExecution()
    callOpcodeSlowPath(_slow_path_throw_static_error)
    dispatch(3)


_llint_op_debug:
    traceExecution()
    loadp CodeBlock[cfr], t0
    loadi CodeBlock::m_debuggerRequests[t0], t0
    btiz t0, .opDebugDone
    callOpcodeSlowPath(_llint_slow_path_debug)
.opDebugDone:                    
    dispatch(3)


_llint_native_call_trampoline:
    nativeCallTrampoline(NativeExecutable::m_function)


_llint_native_construct_trampoline:
    nativeCallTrampoline(NativeExecutable::m_constructor)

_llint_op_get_enumerable_length:
    traceExecution()
    callOpcodeSlowPath(_slow_path_get_enumerable_length)
    dispatch(3)

_llint_op_has_indexed_property:
    traceExecution()
    callOpcodeSlowPath(_slow_path_has_indexed_property)
    dispatch(5)

_llint_op_has_structure_property:
    traceExecution()
    callOpcodeSlowPath(_slow_path_has_structure_property)
    dispatch(5)

_llint_op_has_generic_property:
    traceExecution()
    callOpcodeSlowPath(_slow_path_has_generic_property)
    dispatch(4)

_llint_op_get_direct_pname:
    traceExecution()
    callOpcodeSlowPath(_slow_path_get_direct_pname)
    dispatch(7)

_llint_op_get_property_enumerator:
    traceExecution()
    callOpcodeSlowPath(_slow_path_get_property_enumerator)
    dispatch(3)

_llint_op_enumerator_structure_pname:
    traceExecution()
    callOpcodeSlowPath(_slow_path_next_structure_enumerator_pname)
    dispatch(4)

_llint_op_enumerator_generic_pname:
    traceExecution()
    callOpcodeSlowPath(_slow_path_next_generic_enumerator_pname)
    dispatch(4)

_llint_op_to_index_string:
    traceExecution()
    callOpcodeSlowPath(_slow_path_to_index_string)
    dispatch(3)

_llint_op_create_rest:
    traceExecution()
    callOpcodeSlowPath(_slow_path_create_rest)
    dispatch(4)

_llint_op_instanceof:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_instanceof)
    dispatch(4)

_llint_op_get_by_id_with_this:
    traceExecution()
    callOpcodeSlowPath(_slow_path_get_by_id_with_this)
    dispatch(6)

_llint_op_get_by_val_with_this:
    traceExecution()
    callOpcodeSlowPath(_slow_path_get_by_val_with_this)
    dispatch(6)

_llint_op_put_by_id_with_this:
    traceExecution()
    callOpcodeSlowPath(_slow_path_put_by_id_with_this)
    dispatch(5)

_llint_op_put_by_val_with_this:
    traceExecution()
    callOpcodeSlowPath(_slow_path_put_by_val_with_this)
    dispatch(5)

# Lastly, make sure that we can link even though we don't support all opcodes.
# These opcodes should never arise when using LLInt or either JIT. We assert
# as much.

macro notSupported()
    if ASSERT_ENABLED
        crash()
    else
        # We should use whatever the smallest possible instruction is, just to
        # ensure that there is a gap between instruction labels. If multiple
        # smallest instructions exist, we should pick the one that is most
        # likely result in execution being halted. Currently that is the break
        # instruction on all architectures we're interested in. (Break is int3
        # on Intel, which is 1 byte, and bkpt on ARMv7, which is 2 bytes.)
        break
    end
end
