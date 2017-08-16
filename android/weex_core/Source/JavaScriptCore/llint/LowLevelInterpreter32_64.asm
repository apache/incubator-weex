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


# Utilities
macro dispatch(advance)
    addp advance * 4, PC
    jmp [PC]
end

macro dispatchBranchWithOffset(pcOffset)
    lshifti 2, pcOffset
    addp pcOffset, PC
    jmp [PC]
end

macro dispatchBranch(pcOffset)
    loadi pcOffset, t0
    dispatchBranchWithOffset(t0)
end

macro dispatchAfterCall()
    loadi ArgumentCount + TagOffset[cfr], PC
    loadi 4[PC], t3
    storei r1, TagOffset[cfr, t3, 8]
    storei r0, PayloadOffset[cfr, t3, 8]
    valueProfile(r1, r0, 4 * (CallOpCodeSize - 1), t3)
    dispatch(CallOpCodeSize)
end

macro cCall2(function)
    if ARM or ARMv7 or ARMv7_TRADITIONAL or MIPS
        call function
    elsif X86 or X86_WIN
        subp 8, sp
        push a1
        push a0
        call function
        addp 16, sp
    elsif C_LOOP
        cloopCallSlowPath function, a0, a1
    else
        error
    end
end

macro cCall2Void(function)
    if C_LOOP
        cloopCallSlowPathVoid function, a0, a1
    else
        cCall2(function)
    end
end

macro cCall4(function)
    if ARM or ARMv7 or ARMv7_TRADITIONAL or MIPS
        call function
    elsif X86 or X86_WIN
        push a3
        push a2
        push a1
        push a0
        call function
        addp 16, sp
    elsif C_LOOP
        error
    else
        error
    end
end

macro callSlowPath(slowPath)
    move cfr, a0
    move PC, a1
    cCall2(slowPath)
    move r0, PC
end

macro doVMEntry(makeCall)
    functionPrologue()
    pushCalleeSaves()

    # x86 needs to load arguments from the stack
    if X86 or X86_WIN
        loadp 16[cfr], a2
        loadp 12[cfr], a1
        loadp 8[cfr], a0
    end

    const entry = a0
    const vm = a1
    const protoCallFrame = a2

    # We are using t3, t4 and t5 as temporaries through the function.
    # Since we have the guarantee that tX != aY when X != Y, we are safe from
    # aliasing problems with our arguments.

    if ARMv7
        vmEntryRecord(cfr, t3)
        move t3, sp
    else
        vmEntryRecord(cfr, sp)
    end

    storep vm, VMEntryRecord::m_vm[sp]
    loadp VM::topCallFrame[vm], t4
    storep t4, VMEntryRecord::m_prevTopCallFrame[sp]
    loadp VM::topVMEntryFrame[vm], t4
    storep t4, VMEntryRecord::m_prevTopVMEntryFrame[sp]

    # Align stack pointer
    if X86_WIN or MIPS
        addp CallFrameAlignSlots * SlotSize, sp, t3
        andp ~StackAlignmentMask, t3
        subp t3, CallFrameAlignSlots * SlotSize, sp
    elsif ARM or ARMv7 or ARMv7_TRADITIONAL
        addp CallFrameAlignSlots * SlotSize, sp, t3
        clrbp t3, StackAlignmentMask, t3
        if ARMv7
            subp t3, CallFrameAlignSlots * SlotSize, t3
            move t3, sp
        else
            subp t3, CallFrameAlignSlots * SlotSize, sp
        end
    end

    loadi ProtoCallFrame::paddedArgCount[protoCallFrame], t4
    addp CallFrameHeaderSlots, t4, t4
    lshiftp 3, t4
    subp sp, t4, t3

    # Ensure that we have enough additional stack capacity for the incoming args,
    # and the frame for the JS code we're executing. We need to do this check
    # before we start copying the args from the protoCallFrame below.
    if C_LOOP
        bpaeq t3, VM::m_cloopStackLimit[vm], .stackHeightOK
    else
        bpaeq t3, VM::m_softStackLimit[vm], .stackHeightOK
    end

    if C_LOOP
        move entry, t4
        move vm, t5
        cloopCallSlowPath _llint_stack_check_at_vm_entry, vm, t3
        bpeq t0, 0, .stackCheckFailed
        move t4, entry
        move t5, vm
        jmp .stackHeightOK

.stackCheckFailed:
        move t4, entry
        move t5, vm
    end

    subp 8, sp # Align stack for cCall2() to make a call.
    move vm, a0
    move protoCallFrame, a1
    cCall2(_llint_throw_stack_overflow_error)

    if ARMv7
        vmEntryRecord(cfr, t3)
        move t3, sp
    else
        vmEntryRecord(cfr, sp)
    end

    loadp VMEntryRecord::m_vm[sp], t5
    loadp VMEntryRecord::m_prevTopCallFrame[sp], t4
    storep t4, VM::topCallFrame[t5]
    loadp VMEntryRecord::m_prevTopVMEntryFrame[sp], t4
    storep t4, VM::topVMEntryFrame[t5]

    if ARMv7
        subp cfr, CalleeRegisterSaveSize, t5
        move t5, sp
    else
        subp cfr, CalleeRegisterSaveSize, sp
    end

    popCalleeSaves()
    functionEpilogue()
    ret

.stackHeightOK:
    move t3, sp
    move 4, t3

.copyHeaderLoop:
    subi 1, t3
    loadi TagOffset[protoCallFrame, t3, 8], t5
    storei t5, TagOffset + CodeBlock[sp, t3, 8]
    loadi PayloadOffset[protoCallFrame, t3, 8], t5
    storei t5, PayloadOffset + CodeBlock[sp, t3, 8]
    btinz t3, .copyHeaderLoop

    loadi PayloadOffset + ProtoCallFrame::argCountAndCodeOriginValue[protoCallFrame], t4
    subi 1, t4
    loadi ProtoCallFrame::paddedArgCount[protoCallFrame], t5
    subi 1, t5

    bieq t4, t5, .copyArgs
.fillExtraArgsLoop:
    subi 1, t5
    storei UndefinedTag, ThisArgumentOffset + 8 + TagOffset[sp, t5, 8]
    storei 0, ThisArgumentOffset + 8 + PayloadOffset[sp, t5, 8]
    bineq t4, t5, .fillExtraArgsLoop

.copyArgs:
    loadp ProtoCallFrame::args[protoCallFrame], t3

.copyArgsLoop:
    btiz t4, .copyArgsDone
    subi 1, t4
    loadi TagOffset[t3, t4, 8], t5
    storei t5, ThisArgumentOffset + 8 + TagOffset[sp, t4, 8]
    loadi PayloadOffset[t3, t4, 8], t5
    storei t5, ThisArgumentOffset + 8 + PayloadOffset[sp, t4, 8]
    jmp .copyArgsLoop

.copyArgsDone:
    storep sp, VM::topCallFrame[vm]
    storep cfr, VM::topVMEntryFrame[vm]

    makeCall(entry, t3, t4)

    if ARMv7
        vmEntryRecord(cfr, t3)
        move t3, sp
    else
        vmEntryRecord(cfr, sp)
    end

    loadp VMEntryRecord::m_vm[sp], t5
    loadp VMEntryRecord::m_prevTopCallFrame[sp], t4
    storep t4, VM::topCallFrame[t5]
    loadp VMEntryRecord::m_prevTopVMEntryFrame[sp], t4
    storep t4, VM::topVMEntryFrame[t5]

    if ARMv7
        subp cfr, CalleeRegisterSaveSize, t5
        move t5, sp
    else
        subp cfr, CalleeRegisterSaveSize, sp
    end

    popCalleeSaves()
    functionEpilogue()
    ret
end

macro makeJavaScriptCall(entry, temp, unused)
    addp CallerFrameAndPCSize, sp
    checkStackPointerAlignment(temp, 0xbad0dc02)
    if C_LOOP
        cloopCallJSFunction entry
    else
        call entry
    end
    checkStackPointerAlignment(temp, 0xbad0dc03)
    subp CallerFrameAndPCSize, sp
end

macro makeHostFunctionCall(entry, temp1, temp2)
    move entry, temp1
    storep cfr, [sp]
    if C_LOOP
        move sp, a0
        storep lr, PtrSize[sp]
        cloopCallNative temp1
    elsif X86 or X86_WIN
        # Put callee frame pointer on stack as arg0, also put it in ecx for "fastcall" targets
        move 0, temp2
        move temp2, 4[sp] # put 0 in ReturnPC
        move sp, a0 # a0 is ecx
        push temp2 # Push dummy arg1
        push a0
        call temp1
        addp 8, sp
    else
        move sp, a0
        call temp1
    end
end

_handleUncaughtException:
    loadp Callee + PayloadOffset[cfr], t3
    andp MarkedBlockMask, t3
    loadp MarkedBlock::m_vm[t3], t3
    restoreCalleeSavesFromVMEntryFrameCalleeSavesBuffer(t3, t0)
    loadp VM::callFrameForCatch[t3], cfr
    storep 0, VM::callFrameForCatch[t3]

    loadp CallerFrame[cfr], cfr

    if ARMv7
        vmEntryRecord(cfr, t3)
        move t3, sp
    else
        vmEntryRecord(cfr, sp)
    end

    loadp VMEntryRecord::m_vm[sp], t3
    loadp VMEntryRecord::m_prevTopCallFrame[sp], t5
    storep t5, VM::topCallFrame[t3]
    loadp VMEntryRecord::m_prevTopVMEntryFrame[sp], t5
    storep t5, VM::topVMEntryFrame[t3]

    if ARMv7
        subp cfr, CalleeRegisterSaveSize, t3
        move t3, sp
    else
        subp cfr, CalleeRegisterSaveSize, sp
    end

    popCalleeSaves()
    functionEpilogue()
    ret

macro doReturnFromHostFunction(extraStackSpace)
    functionEpilogue(extraStackSpace)
    ret
end

# Debugging operation if you'd like to print an operand in the instruction stream. fromWhere
# should be an immediate integer - any integer you like; use it to identify the place you're
# debugging from. operand should likewise be an immediate, and should identify the operand
# in the instruction stream you'd like to print out.
macro traceOperand(fromWhere, operand)
    move fromWhere, a2
    move operand, a3
    move cfr, a0
    move PC, a1
    cCall4(_llint_trace_operand)
    move r0, PC
    move r1, cfr
end

# Debugging operation if you'd like to print the value of an operand in the instruction
# stream. Same as traceOperand(), but assumes that the operand is a register, and prints its
# value.
macro traceValue(fromWhere, operand)
    move fromWhere, a2
    move operand, a3
    move cfr, a0
    move PC, a1
    cCall4(_llint_trace_value)
    move r0, PC
    move r1, cfr
end

# Call a slowPath for call opcodes.
macro callCallSlowPath(slowPath, action)
    storep PC, ArgumentCount + TagOffset[cfr]
    move cfr, a0
    move PC, a1
    cCall2(slowPath)
    action(r0, r1)
end

macro callTrapHandler(throwHandler)
    storei PC, ArgumentCount + TagOffset[cfr]
    move cfr, a0
    move PC, a1
    cCall2(_llint_slow_path_handle_traps)
    btpnz r0, throwHandler
    loadi ArgumentCount + TagOffset[cfr], PC
end

macro checkSwitchToJITForLoop()
    checkSwitchToJIT(
        1,
        macro ()
            storei PC, ArgumentCount + TagOffset[cfr]
            move cfr, a0
            move PC, a1
            cCall2(_llint_loop_osr)
            btpz r0, .recover
            move r1, sp
            jmp r0
        .recover:
            loadi ArgumentCount + TagOffset[cfr], PC
        end)
end

macro loadVariable(operand, index, tag, payload)
    loadisFromInstruction(operand, index)
    loadi TagOffset[cfr, index, 8], tag
    loadi PayloadOffset[cfr, index, 8], payload
end

# Index, tag, and payload must be different registers. Index is not
# changed.
macro loadConstantOrVariable(index, tag, payload)
    bigteq index, FirstConstantRegisterIndex, .constant
    loadi TagOffset[cfr, index, 8], tag
    loadi PayloadOffset[cfr, index, 8], payload
    jmp .done
.constant:
    loadp CodeBlock[cfr], payload
    loadp CodeBlock::m_constantRegisters + VectorBufferOffset[payload], payload
    # There is a bit of evil here: if the index contains a value >= FirstConstantRegisterIndex,
    # then value << 3 will be equal to (value - FirstConstantRegisterIndex) << 3.
    loadp TagOffset[payload, index, 8], tag
    loadp PayloadOffset[payload, index, 8], payload
.done:
end

macro loadConstantOrVariableTag(index, tag)
    bigteq index, FirstConstantRegisterIndex, .constant
    loadi TagOffset[cfr, index, 8], tag
    jmp .done
.constant:
    loadp CodeBlock[cfr], tag
    loadp CodeBlock::m_constantRegisters + VectorBufferOffset[tag], tag
    # There is a bit of evil here: if the index contains a value >= FirstConstantRegisterIndex,
    # then value << 3 will be equal to (value - FirstConstantRegisterIndex) << 3.
    loadp TagOffset[tag, index, 8], tag
.done:
end

# Index and payload may be the same register. Index may be clobbered.
macro loadConstantOrVariable2Reg(index, tag, payload)
    bigteq index, FirstConstantRegisterIndex, .constant
    loadi TagOffset[cfr, index, 8], tag
    loadi PayloadOffset[cfr, index, 8], payload
    jmp .done
.constant:
    loadp CodeBlock[cfr], tag
    loadp CodeBlock::m_constantRegisters + VectorBufferOffset[tag], tag
    # There is a bit of evil here: if the index contains a value >= FirstConstantRegisterIndex,
    # then value << 3 will be equal to (value - FirstConstantRegisterIndex) << 3.
    lshifti 3, index
    addp index, tag
    loadp PayloadOffset[tag], payload
    loadp TagOffset[tag], tag
.done:
end

macro loadConstantOrVariablePayloadTagCustom(index, tagCheck, payload)
    bigteq index, FirstConstantRegisterIndex, .constant
    tagCheck(TagOffset[cfr, index, 8])
    loadi PayloadOffset[cfr, index, 8], payload
    jmp .done
.constant:
    loadp CodeBlock[cfr], payload
    loadp CodeBlock::m_constantRegisters + VectorBufferOffset[payload], payload
    # There is a bit of evil here: if the index contains a value >= FirstConstantRegisterIndex,
    # then value << 3 will be equal to (value - FirstConstantRegisterIndex) << 3.
    tagCheck(TagOffset[payload, index, 8])
    loadp PayloadOffset[payload, index, 8], payload
.done:
end

# Index and payload must be different registers. Index is not mutated. Use
# this if you know what the tag of the variable should be. Doing the tag
# test as part of loading the variable reduces register use, but may not
# be faster than doing loadConstantOrVariable followed by a branch on the
# tag.
macro loadConstantOrVariablePayload(index, expectedTag, payload, slow)
    loadConstantOrVariablePayloadTagCustom(
        index,
        macro (actualTag) bineq actualTag, expectedTag, slow end,
        payload)
end

macro loadConstantOrVariablePayloadUnchecked(index, payload)
    loadConstantOrVariablePayloadTagCustom(
        index,
        macro (actualTag) end,
        payload)
end

macro writeBarrierOnOperand(cellOperand)
    loadisFromInstruction(cellOperand, t1)
    loadConstantOrVariablePayload(t1, CellTag, t2, .writeBarrierDone)
    skipIfIsRememberedOrInEden(
        t2, 
        macro()
            push cfr, PC
            # We make two extra slots because cCall2 will poke.
            subp 8, sp
            move t2, a1 # t2 can be a0 on x86
            move cfr, a0
            cCall2Void(_llint_write_barrier_slow)
            addp 8, sp
            pop PC, cfr
        end)
.writeBarrierDone:
end

macro writeBarrierOnOperands(cellOperand, valueOperand)
    loadisFromInstruction(valueOperand, t1)
    loadConstantOrVariableTag(t1, t0)
    bineq t0, CellTag, .writeBarrierDone

    writeBarrierOnOperand(cellOperand)
.writeBarrierDone:
end

macro writeBarrierOnGlobal(valueOperand, loadHelper)
    loadisFromInstruction(valueOperand, t1)
    loadConstantOrVariableTag(t1, t0)
    bineq t0, CellTag, .writeBarrierDone

    loadHelper(t3)

    skipIfIsRememberedOrInEden(
        t3,
        macro()
            push cfr, PC
            # We make two extra slots because cCall2 will poke.
            subp 8, sp
            move cfr, a0
            move t3, a1
            cCall2Void(_llint_write_barrier_slow)
            addp 8, sp
            pop PC, cfr
        end)
.writeBarrierDone:
end

macro writeBarrierOnGlobalObject(valueOperand)
    writeBarrierOnGlobal(valueOperand,
        macro(registerToStoreGlobal)
            loadp CodeBlock[cfr], registerToStoreGlobal
            loadp CodeBlock::m_globalObject[registerToStoreGlobal], registerToStoreGlobal
        end)
end

macro writeBarrierOnGlobalLexicalEnvironment(valueOperand)
    writeBarrierOnGlobal(valueOperand,
        macro(registerToStoreGlobal)
            loadp CodeBlock[cfr], registerToStoreGlobal
            loadp CodeBlock::m_globalObject[registerToStoreGlobal], registerToStoreGlobal
            loadp JSGlobalObject::m_globalLexicalEnvironment[registerToStoreGlobal], registerToStoreGlobal
        end)
end

macro valueProfile(tag, payload, operand, scratch)
    loadp operand[PC], scratch
    storei tag, ValueProfile::m_buckets + TagOffset[scratch]
    storei payload, ValueProfile::m_buckets + PayloadOffset[scratch]
end


# Entrypoints into the interpreter

# Expects that CodeBlock is in t1, which is what prologue() leaves behind.
macro functionArityCheck(doneLabel, slowPath)
    loadi PayloadOffset + ArgumentCount[cfr], t0
    biaeq t0, CodeBlock::m_numParameters[t1], doneLabel
    move cfr, a0
    move PC, a1
    cCall2(slowPath)   # This slowPath has a simple protocol: t0 = 0 => no error, t0 != 0 => error
    btiz r0, .noError
    move r1, cfr   # r1 contains caller frame
    jmp _llint_throw_from_slow_path_trampoline

.noError:
    # r1 points to ArityCheckData.
    loadp CommonSlowPaths::ArityCheckData::thunkToCall[r1], t3
    btpz t3, .proceedInline
    
    loadp CommonSlowPaths::ArityCheckData::paddedStackSpace[r1], a0
    call t3
    if ASSERT_ENABLED
        loadp ReturnPC[cfr], t0
        loadp [t0], t0
    end
    jmp .continue

.proceedInline:
    loadi CommonSlowPaths::ArityCheckData::paddedStackSpace[r1], t1
    btiz t1, .continue
    loadi PayloadOffset + ArgumentCount[cfr], t2
    addi CallFrameHeaderSlots, t2

    // Check if there are some unaligned slots we can use
    move t1, t3
    andi StackAlignmentSlots - 1, t3
    btiz t3, .noExtraSlot
.fillExtraSlots:
    move 0, t0
    storei t0, PayloadOffset[cfr, t2, 8]
    move UndefinedTag, t0
    storei t0, TagOffset[cfr, t2, 8]
    addi 1, t2
    bsubinz 1, t3, .fillExtraSlots
    andi ~(StackAlignmentSlots - 1), t1
    btiz t1, .continue

.noExtraSlot:
    // Move frame up t1 slots
    negi t1
    move cfr, t3
    move t1, t0
    lshiftp 3, t0
    addp t0, cfr
    addp t0, sp
.copyLoop:
    loadi PayloadOffset[t3], t0
    storei t0, PayloadOffset[t3, t1, 8]
    loadi TagOffset[t3], t0
    storei t0, TagOffset[t3, t1, 8]
    addp 8, t3
    bsubinz 1, t2, .copyLoop

    // Fill new slots with JSUndefined
    move t1, t2
.fillLoop:
    move 0, t0
    storei t0, PayloadOffset[t3, t1, 8]
    move UndefinedTag, t0
    storei t0, TagOffset[t3, t1, 8]
    addp 8, t3
    baddinz 1, t2, .fillLoop

.continue:
    # Reload CodeBlock and PC, since the slow_path clobbered it.
    loadp CodeBlock[cfr], t1
    loadp CodeBlock::m_instructions[t1], PC
    jmp doneLabel
end

macro branchIfException(label)
    loadp Callee + PayloadOffset[cfr], t3
    andp MarkedBlockMask, t3
    loadp MarkedBlock::m_vm[t3], t3
    btiz VM::m_exception[t3], .noException
    jmp label
.noException:
end


# Instruction implementations

_llint_op_enter:
    traceExecution()
    checkStackPointerAlignment(t2, 0xdead00e1)
    loadp CodeBlock[cfr], t2                // t2<CodeBlock> = cfr.CodeBlock
    loadi CodeBlock::m_numVars[t2], t2      // t2<size_t> = t2<CodeBlock>.m_numVars
    btiz t2, .opEnterDone
    move UndefinedTag, t0
    move 0, t1
    negi t2
.opEnterLoop:
    storei t0, TagOffset[cfr, t2, 8]
    storei t1, PayloadOffset[cfr, t2, 8]
    addi 1, t2
    btinz t2, .opEnterLoop
.opEnterDone:
    callOpcodeSlowPath(_slow_path_enter)
    dispatch(1)


_llint_op_get_argument:
    traceExecution()
    loadisFromInstruction(1, t1)
    loadisFromInstruction(2, t2)
    loadi PayloadOffset + ArgumentCount[cfr], t0
    bilteq t0, t2, .opGetArgumentOutOfBounds
    loadi ThisArgumentOffset + TagOffset[cfr, t2, 8], t0
    loadi ThisArgumentOffset + PayloadOffset[cfr, t2, 8], t3
    storei t0, TagOffset[cfr, t1, 8]
    storei t3, PayloadOffset[cfr, t1, 8]
    valueProfile(t0, t3, 12, t1)
    dispatch(4)

.opGetArgumentOutOfBounds:
    storei UndefinedTag, TagOffset[cfr, t1, 8]
    storei 0, PayloadOffset[cfr, t1, 8]
    valueProfile(UndefinedTag, 0, 12, t1)
    dispatch(4)


_llint_op_argument_count:
    traceExecution()
    loadisFromInstruction(1, t2)
    loadi PayloadOffset + ArgumentCount[cfr], t0
    subi 1, t0
    move Int32Tag, t1
    storei t1, TagOffset[cfr, t2, 8]
    storei t0, PayloadOffset[cfr, t2, 8]
    dispatch(2)


_llint_op_get_scope:
    traceExecution()
    loadi Callee + PayloadOffset[cfr], t0
    loadi JSCallee::m_scope[t0], t0
    loadisFromInstruction(1, t1)
    storei CellTag, TagOffset[cfr, t1, 8]
    storei t0, PayloadOffset[cfr, t1, 8]
    dispatch(2)


_llint_op_to_this:
    traceExecution()
    loadi 4[PC], t0
    bineq TagOffset[cfr, t0, 8], CellTag, .opToThisSlow
    loadi PayloadOffset[cfr, t0, 8], t0
    bbneq JSCell::m_type[t0], FinalObjectType, .opToThisSlow
    loadpFromInstruction(2, t2)
    bpneq JSCell::m_structureID[t0], t2, .opToThisSlow
    dispatch(4)

.opToThisSlow:
    callOpcodeSlowPath(_slow_path_to_this)
    dispatch(4)


_llint_op_check_tdz:
    traceExecution()
    loadisFromInstruction(1, t0)
    loadConstantOrVariableTag(t0, t1)
    bineq t1, EmptyValueTag, .opNotTDZ
    callOpcodeSlowPath(_slow_path_throw_tdz_error)

.opNotTDZ:
    dispatch(2)


_llint_op_mov:
    traceExecution()
    loadi 8[PC], t1
    loadi 4[PC], t0
    loadConstantOrVariable(t1, t2, t3)
    storei t2, TagOffset[cfr, t0, 8]
    storei t3, PayloadOffset[cfr, t0, 8]
    dispatch(3)


_llint_op_not:
    traceExecution()
    loadi 8[PC], t0
    loadi 4[PC], t1
    loadConstantOrVariable(t0, t2, t3)
    bineq t2, BooleanTag, .opNotSlow
    xori 1, t3
    storei t2, TagOffset[cfr, t1, 8]
    storei t3, PayloadOffset[cfr, t1, 8]
    dispatch(3)

.opNotSlow:
    callOpcodeSlowPath(_slow_path_not)
    dispatch(3)


_llint_op_eq:
    traceExecution()
    loadi 12[PC], t2
    loadi 8[PC], t0
    loadConstantOrVariable(t2, t3, t1)
    loadConstantOrVariable2Reg(t0, t2, t0)
    bineq t2, t3, .opEqSlow
    bieq t2, CellTag, .opEqSlow
    bib t2, LowestTag, .opEqSlow
    loadi 4[PC], t2
    cieq t0, t1, t0
    storei BooleanTag, TagOffset[cfr, t2, 8]
    storei t0, PayloadOffset[cfr, t2, 8]
    dispatch(4)

.opEqSlow:
    callOpcodeSlowPath(_slow_path_eq)
    dispatch(4)


_llint_op_eq_null:
    traceExecution()
    loadi 8[PC], t0
    loadi 4[PC], t3
    assertNotConstant(t0)
    loadi TagOffset[cfr, t0, 8], t1
    loadi PayloadOffset[cfr, t0, 8], t0
    bineq t1, CellTag, .opEqNullImmediate
    btbnz JSCell::m_flags[t0], MasqueradesAsUndefined, .opEqNullMasqueradesAsUndefined
    move 0, t1
    jmp .opEqNullNotImmediate
.opEqNullMasqueradesAsUndefined:
    loadp JSCell::m_structureID[t0], t1
    loadp CodeBlock[cfr], t0
    loadp CodeBlock::m_globalObject[t0], t0
    cpeq Structure::m_globalObject[t1], t0, t1
    jmp .opEqNullNotImmediate
.opEqNullImmediate:
    cieq t1, NullTag, t2
    cieq t1, UndefinedTag, t1
    ori t2, t1
.opEqNullNotImmediate:
    storei BooleanTag, TagOffset[cfr, t3, 8]
    storei t1, PayloadOffset[cfr, t3, 8]
    dispatch(3)


_llint_op_neq:
    traceExecution()
    loadi 12[PC], t2
    loadi 8[PC], t0
    loadConstantOrVariable(t2, t3, t1)
    loadConstantOrVariable2Reg(t0, t2, t0)
    bineq t2, t3, .opNeqSlow
    bieq t2, CellTag, .opNeqSlow
    bib t2, LowestTag, .opNeqSlow
    loadi 4[PC], t2
    cineq t0, t1, t0
    storei BooleanTag, TagOffset[cfr, t2, 8]
    storei t0, PayloadOffset[cfr, t2, 8]
    dispatch(4)

.opNeqSlow:
    callOpcodeSlowPath(_slow_path_neq)
    dispatch(4)
    

_llint_op_neq_null:
    traceExecution()
    loadi 8[PC], t0
    loadi 4[PC], t3
    assertNotConstant(t0)
    loadi TagOffset[cfr, t0, 8], t1
    loadi PayloadOffset[cfr, t0, 8], t0
    bineq t1, CellTag, .opNeqNullImmediate
    btbnz JSCell::m_flags[t0], MasqueradesAsUndefined, .opNeqNullMasqueradesAsUndefined
    move 1, t1
    jmp .opNeqNullNotImmediate
.opNeqNullMasqueradesAsUndefined:
    loadp JSCell::m_structureID[t0], t1
    loadp CodeBlock[cfr], t0
    loadp CodeBlock::m_globalObject[t0], t0
    cpneq Structure::m_globalObject[t1], t0, t1
    jmp .opNeqNullNotImmediate
.opNeqNullImmediate:
    cineq t1, NullTag, t2
    cineq t1, UndefinedTag, t1
    andi t2, t1
.opNeqNullNotImmediate:
    storei BooleanTag, TagOffset[cfr, t3, 8]
    storei t1, PayloadOffset[cfr, t3, 8]
    dispatch(3)


macro strictEq(equalityOperation, slowPath)
    loadi 12[PC], t2
    loadi 8[PC], t0
    loadConstantOrVariable(t2, t3, t1)
    loadConstantOrVariable2Reg(t0, t2, t0)
    bineq t2, t3, .slow
    bib t2, LowestTag, .slow
    bineq t2, CellTag, .notStringOrSymbol
    bbaeq JSCell::m_type[t0], ObjectType, .notStringOrSymbol
    bbb JSCell::m_type[t1], ObjectType, .slow
.notStringOrSymbol:
    loadi 4[PC], t2
    equalityOperation(t0, t1, t0)
    storei BooleanTag, TagOffset[cfr, t2, 8]
    storei t0, PayloadOffset[cfr, t2, 8]
    dispatch(4)

.slow:
    callOpcodeSlowPath(slowPath)
    dispatch(4)
end

_llint_op_stricteq:
    traceExecution()
    strictEq(macro (left, right, result) cieq left, right, result end, _slow_path_stricteq)


_llint_op_nstricteq:
    traceExecution()
    strictEq(macro (left, right, result) cineq left, right, result end, _slow_path_nstricteq)


_llint_op_inc:
    traceExecution()
    loadi 4[PC], t0
    bineq TagOffset[cfr, t0, 8], Int32Tag, .opIncSlow
    loadi PayloadOffset[cfr, t0, 8], t1
    baddio 1, t1, .opIncSlow
    storei t1, PayloadOffset[cfr, t0, 8]
    dispatch(2)

.opIncSlow:
    callOpcodeSlowPath(_slow_path_inc)
    dispatch(2)


_llint_op_dec:
    traceExecution()
    loadi 4[PC], t0
    bineq TagOffset[cfr, t0, 8], Int32Tag, .opDecSlow
    loadi PayloadOffset[cfr, t0, 8], t1
    bsubio 1, t1, .opDecSlow
    storei t1, PayloadOffset[cfr, t0, 8]
    dispatch(2)

.opDecSlow:
    callOpcodeSlowPath(_slow_path_dec)
    dispatch(2)


_llint_op_to_number:
    traceExecution()
    loadi 8[PC], t0
    loadi 4[PC], t1
    loadConstantOrVariable(t0, t2, t3)
    bieq t2, Int32Tag, .opToNumberIsInt
    biaeq t2, LowestTag, .opToNumberSlow
.opToNumberIsInt:
    storei t2, TagOffset[cfr, t1, 8]
    storei t3, PayloadOffset[cfr, t1, 8]
    valueProfile(t2, t3, 12, t1)
    dispatch(4)

.opToNumberSlow:
    callOpcodeSlowPath(_slow_path_to_number)
    dispatch(4)


_llint_op_to_string:
    traceExecution()
    loadi 8[PC], t0
    loadi 4[PC], t1
    loadConstantOrVariable(t0, t2, t3)
    bineq t2, CellTag, .opToStringSlow
    bbneq JSCell::m_type[t3], StringType, .opToStringSlow
.opToStringIsString:
    storei t2, TagOffset[cfr, t1, 8]
    storei t3, PayloadOffset[cfr, t1, 8]
    dispatch(3)

.opToStringSlow:
    callOpcodeSlowPath(_slow_path_to_string)
    dispatch(3)


_llint_op_negate:
    traceExecution()
    loadi 8[PC], t0
    loadi 4[PC], t3
    loadConstantOrVariable(t0, t1, t2)
    loadisFromInstruction(3, t0)
    bineq t1, Int32Tag, .opNegateSrcNotInt
    btiz t2, 0x7fffffff, .opNegateSlow
    negi t2
    ori ArithProfileInt, t0
    storei Int32Tag, TagOffset[cfr, t3, 8]
    storeisToInstruction(t0, 3)
    storei t2, PayloadOffset[cfr, t3, 8]
    dispatch(4)
.opNegateSrcNotInt:
    bia t1, LowestTag, .opNegateSlow
    xori 0x80000000, t1
    ori ArithProfileNumber, t0
    storei t2, PayloadOffset[cfr, t3, 8]
    storeisToInstruction(t0, 3)
    storei t1, TagOffset[cfr, t3, 8]
    dispatch(4)

.opNegateSlow:
    callOpcodeSlowPath(_slow_path_negate)
    dispatch(4)


macro binaryOpCustomStore(integerOperationAndStore, doubleOperation, slowPath)
    loadi 12[PC], t2
    loadi 8[PC], t0
    loadConstantOrVariable(t2, t3, t1)
    loadConstantOrVariable2Reg(t0, t2, t0)
    bineq t2, Int32Tag, .op1NotInt
    bineq t3, Int32Tag, .op2NotInt
    loadisFromInstruction(4, t5)
    ori ArithProfileIntInt, t5
    storeisToInstruction(t5, 4)
    loadi 4[PC], t2
    integerOperationAndStore(t3, t1, t0, .slow, t2)
    dispatch(5)

.op1NotInt:
    # First operand is definitely not an int, the second operand could be anything.
    bia t2, LowestTag, .slow
    bib t3, LowestTag, .op1NotIntOp2Double
    bineq t3, Int32Tag, .slow
    loadisFromInstruction(4, t5)
    ori ArithProfileNumberInt, t5
    storeisToInstruction(t5, 4)
    ci2d t1, ft1
    jmp .op1NotIntReady
.op1NotIntOp2Double:
    fii2d t1, t3, ft1
    loadisFromInstruction(4, t5)
    ori ArithProfileNumberNumber, t5
    storeisToInstruction(t5, 4)
.op1NotIntReady:
    loadi 4[PC], t1
    fii2d t0, t2, ft0
    doubleOperation(ft1, ft0)
    stored ft0, [cfr, t1, 8]
    dispatch(5)

.op2NotInt:
    # First operand is definitely an int, the second operand is definitely not.
    loadi 4[PC], t2
    bia t3, LowestTag, .slow
    loadisFromInstruction(4, t5)
    ori ArithProfileIntNumber, t5
    storeisToInstruction(t5, 4)
    ci2d t0, ft0
    fii2d t1, t3, ft1
    doubleOperation(ft1, ft0)
    stored ft0, [cfr, t2, 8]
    dispatch(5)

.slow:
    callOpcodeSlowPath(slowPath)
    dispatch(5)
end

macro binaryOp(integerOperation, doubleOperation, slowPath)
    binaryOpCustomStore(
        macro (int32Tag, left, right, slow, index)
            integerOperation(left, right, slow)
            storei int32Tag, TagOffset[cfr, index, 8]
            storei right, PayloadOffset[cfr, index, 8]
        end,
        doubleOperation, slowPath)
end

_llint_op_add:
    traceExecution()
    binaryOp(
        macro (left, right, slow) baddio left, right, slow end,
        macro (left, right) addd left, right end,
        _slow_path_add)


_llint_op_mul:
    traceExecution()
    binaryOpCustomStore(
        macro (int32Tag, left, right, slow, index)
            const scratch = int32Tag   # We know that we can reuse the int32Tag register since it has a constant.
            move right, scratch
            bmulio left, scratch, slow
            btinz scratch, .done
            bilt left, 0, slow
            bilt right, 0, slow
        .done:
            storei Int32Tag, TagOffset[cfr, index, 8]
            storei scratch, PayloadOffset[cfr, index, 8]
        end,
        macro (left, right) muld left, right end,
        _slow_path_mul)


_llint_op_sub:
    traceExecution()
    binaryOp(
        macro (left, right, slow) bsubio left, right, slow end,
        macro (left, right) subd left, right end,
        _slow_path_sub)


_llint_op_div:
    traceExecution()
    binaryOpCustomStore(
        macro (int32Tag, left, right, slow, index)
            ci2d left, ft0
            ci2d right, ft1
            divd ft0, ft1
            bcd2i ft1, right, .notInt
            storei int32Tag, TagOffset[cfr, index, 8]
            storei right, PayloadOffset[cfr, index, 8]
            jmp .done
        .notInt:
            stored ft1, [cfr, index, 8]
        .done:
        end,
        macro (left, right) divd left, right end,
        _slow_path_div)


macro bitOp(operation, slowPath, advance)
    loadi 12[PC], t2
    loadi 8[PC], t0
    loadConstantOrVariable(t2, t3, t1)
    loadConstantOrVariable2Reg(t0, t2, t0)
    bineq t3, Int32Tag, .slow
    bineq t2, Int32Tag, .slow
    loadi 4[PC], t2
    operation(t1, t0)
    storei t3, TagOffset[cfr, t2, 8]
    storei t0, PayloadOffset[cfr, t2, 8]
    dispatch(advance)

.slow:
    callOpcodeSlowPath(slowPath)
    dispatch(advance)
end

_llint_op_lshift:
    traceExecution()
    bitOp(
        macro (left, right) lshifti left, right end,
        _slow_path_lshift,
        4)


_llint_op_rshift:
    traceExecution()
    bitOp(
        macro (left, right) rshifti left, right end,
        _slow_path_rshift,
        4)


_llint_op_urshift:
    traceExecution()
    bitOp(
        macro (left, right) urshifti left, right end,
        _slow_path_urshift,
        4)


_llint_op_unsigned:
    traceExecution()
    loadi 4[PC], t0
    loadi 8[PC], t1
    loadConstantOrVariablePayload(t1, Int32Tag, t2, .opUnsignedSlow)
    bilt t2, 0, .opUnsignedSlow
    storei t2, PayloadOffset[cfr, t0, 8]
    storei Int32Tag, TagOffset[cfr, t0, 8]
    dispatch(3)
.opUnsignedSlow:
    callOpcodeSlowPath(_slow_path_unsigned)
    dispatch(3)


_llint_op_bitand:
    traceExecution()
    bitOp(
        macro (left, right) andi left, right end,
        _slow_path_bitand,
        5)


_llint_op_bitxor:
    traceExecution()
    bitOp(
        macro (left, right) xori left, right end,
        _slow_path_bitxor,
        5)


_llint_op_bitor:
    traceExecution()
    bitOp(
        macro (left, right) ori left, right end,
        _slow_path_bitor,
        5)


_llint_op_overrides_has_instance:
    traceExecution()

    loadisFromInstruction(1, t3)
    storei BooleanTag, TagOffset[cfr, t3, 8]

    # First check if hasInstanceValue is the one on Function.prototype[Symbol.hasInstance]
    loadisFromInstruction(3, t0)
    loadConstantOrVariablePayload(t0, CellTag, t2, .opOverrideshasInstanceValueNotCell)
    loadConstantOrVariable(t0, t1, t2)
    bineq t1, CellTag, .opOverrideshasInstanceValueNotCell

    # We don't need hasInstanceValue's tag register anymore.
    loadp CodeBlock[cfr], t1
    loadp CodeBlock::m_globalObject[t1], t1
    loadp JSGlobalObject::m_functionProtoHasInstanceSymbolFunction[t1], t1
    bineq t1, t2, .opOverrideshasInstanceValueNotDefault

    # We know the constructor is a cell.
    loadisFromInstruction(2, t0)
    loadConstantOrVariablePayloadUnchecked(t0, t1)
    tbz JSCell::m_flags[t1], ImplementsDefaultHasInstance, t0
    storei t0, PayloadOffset[cfr, t3, 8]
    dispatch(4)

.opOverrideshasInstanceValueNotCell:
.opOverrideshasInstanceValueNotDefault:
    storei 1, PayloadOffset[cfr, t3, 8]
    dispatch(4)

_llint_op_instanceof_custom:
    traceExecution()
    callOpcodeSlowPath(_llint_slow_path_instanceof_custom)
    dispatch(5)


_llint_op_is_empty:
    traceExecution()
    loadi 8[PC], t1
    loadi 4[PC], t0
    loadConstantOrVariable(t1, t2, t3)
    cieq t2, EmptyValueTag, t3
    storei BooleanTag, TagOffset[cfr, t0, 8]
    storei t3, PayloadOffset[cfr, t0, 8]
    dispatch(3)


_llint_op_is_undefined:
    traceExecution()
    loadi 8[PC], t1
    loadi 4[PC], t0
    loadConstantOrVariable(t1, t2, t3)
    storei BooleanTag, TagOffset[cfr, t0, 8]
    bieq t2, CellTag, .opIsUndefinedCell
    cieq t2, UndefinedTag, t3
    storei t3, PayloadOffset[cfr, t0, 8]
    dispatch(3)
.opIsUndefinedCell:
    btbnz JSCell::m_flags[t3], MasqueradesAsUndefined, .opIsUndefinedMasqueradesAsUndefined
    move 0, t1
    storei t1, PayloadOffset[cfr, t0, 8]
    dispatch(3)
.opIsUndefinedMasqueradesAsUndefined:
    loadp JSCell::m_structureID[t3], t1
    loadp CodeBlock[cfr], t3
    loadp CodeBlock::m_globalObject[t3], t3
    cpeq Structure::m_globalObject[t1], t3, t1
    storei t1, PayloadOffset[cfr, t0, 8]
    dispatch(3)


_llint_op_is_boolean:
    traceExecution()
    loadi 8[PC], t1
    loadi 4[PC], t2
    loadConstantOrVariableTag(t1, t0)
    cieq t0, BooleanTag, t0
    storei BooleanTag, TagOffset[cfr, t2, 8]
    storei t0, PayloadOffset[cfr, t2, 8]
    dispatch(3)


_llint_op_is_number:
    traceExecution()
    loadi 8[PC], t1
    loadi 4[PC], t2
    loadConstantOrVariableTag(t1, t0)
    storei BooleanTag, TagOffset[cfr, t2, 8]
    addi 1, t0
    cib t0, LowestTag + 1, t1
    storei t1, PayloadOffset[cfr, t2, 8]
    dispatch(3)


_llint_op_is_cell_with_type:
    traceExecution()
    loadi 8[PC], t1
    loadi 4[PC], t2
    loadConstantOrVariable(t1, t0, t3)
    storei BooleanTag, TagOffset[cfr, t2, 8]
    bineq t0, CellTag, .notCellCase
    loadi 12[PC], t0
    cbeq JSCell::m_type[t3], t0, t1
    storei t1, PayloadOffset[cfr, t2, 8]
    dispatch(4)
.notCellCase:
    storep 0, PayloadOffset[cfr, t2, 8]
    dispatch(4)


_llint_op_is_object:
    traceExecution()
    loadi 8[PC], t1
    loadi 4[PC], t2
    loadConstantOrVariable(t1, t0, t3)
    storei BooleanTag, TagOffset[cfr, t2, 8]
    bineq t0, CellTag, .opIsObjectNotCell
    cbaeq JSCell::m_type[t3], ObjectType, t1
    storei t1, PayloadOffset[cfr, t2, 8]
    dispatch(3)
.opIsObjectNotCell:
    storep 0, PayloadOffset[cfr, t2, 8]
    dispatch(3)


macro loadPropertyAtVariableOffsetKnownNotInline(propertyOffset, objectAndStorage, tag, payload)
    assert(macro (ok) bigteq propertyOffset, firstOutOfLineOffset, ok end)
    negi propertyOffset
    loadp JSObject::m_butterfly[objectAndStorage], objectAndStorage
    loadi TagOffset + (firstOutOfLineOffset - 2) * 8[objectAndStorage, propertyOffset, 8], tag
    loadi PayloadOffset + (firstOutOfLineOffset - 2) * 8[objectAndStorage, propertyOffset, 8], payload
end

macro loadPropertyAtVariableOffset(propertyOffset, objectAndStorage, tag, payload)
    bilt propertyOffset, firstOutOfLineOffset, .isInline
    loadp JSObject::m_butterfly[objectAndStorage], objectAndStorage
    negi propertyOffset
    jmp .ready
.isInline:
    addp sizeof JSObject - (firstOutOfLineOffset - 2) * 8, objectAndStorage
.ready:
    loadi TagOffset + (firstOutOfLineOffset - 2) * 8[objectAndStorage, propertyOffset, 8], tag
    loadi PayloadOffset + (firstOutOfLineOffset - 2) * 8[objectAndStorage, propertyOffset, 8], payload
end

macro storePropertyAtVariableOffset(propertyOffsetAsInt, objectAndStorage, tag, payload)
    bilt propertyOffsetAsInt, firstOutOfLineOffset, .isInline
    loadp JSObject::m_butterfly[objectAndStorage], objectAndStorage
    negi propertyOffsetAsInt
    jmp .ready
.isInline:
    addp sizeof JSObject - (firstOutOfLineOffset - 2) * 8, objectAndStorage
.ready:
    storei tag, TagOffset + (firstOutOfLineOffset - 2) * 8[objectAndStorage, propertyOffsetAsInt, 8]
    storei payload, PayloadOffset + (firstOutOfLineOffset - 2) * 8[objectAndStorage, propertyOffsetAsInt, 8]
end


# We only do monomorphic get_by_id caching for now, and we do not modify the
# opcode for own properties. We also allow for the cache to change anytime it fails,
# since ping-ponging is free. At best we get lucky and the get_by_id will continue
# to take fast path on the new cache. At worst we take slow path, which is what
# we would have been doing anyway. For prototype/unset properties, we will attempt to
# convert opcode into a get_by_id_proto_load/get_by_id_unset, respectively, after an
# execution counter hits zero.

_llint_op_get_by_id:
    traceExecution()
    loadi 8[PC], t0
    loadi 16[PC], t1
    loadConstantOrVariablePayload(t0, CellTag, t3, .opGetByIdSlow)
    loadi 20[PC], t2
    bineq JSCell::m_structureID[t3], t1, .opGetByIdSlow
    loadPropertyAtVariableOffset(t2, t3, t0, t1)
    loadi 4[PC], t2
    storei t0, TagOffset[cfr, t2, 8]
    storei t1, PayloadOffset[cfr, t2, 8]
    valueProfile(t0, t1, 32, t2)
    dispatch(9)

.opGetByIdSlow:
    callOpcodeSlowPath(_llint_slow_path_get_by_id)
    dispatch(9)


_llint_op_get_by_id_proto_load:
    traceExecution()
    loadi 8[PC], t0
    loadi 16[PC], t1
    loadConstantOrVariablePayload(t0, CellTag, t3, .opGetByIdProtoSlow)
    loadi 20[PC], t2
    bineq JSCell::m_structureID[t3], t1, .opGetByIdProtoSlow
    loadpFromInstruction(6, t3)
    loadPropertyAtVariableOffset(t2, t3, t0, t1)
    loadi 4[PC], t2
    storei t0, TagOffset[cfr, t2, 8]
    storei t1, PayloadOffset[cfr, t2, 8]
    valueProfile(t0, t1, 32, t2)
    dispatch(9)

.opGetByIdProtoSlow:
    callOpcodeSlowPath(_llint_slow_path_get_by_id)
    dispatch(9)


_llint_op_get_by_id_unset:
    traceExecution()
    loadi 8[PC], t0
    loadi 16[PC], t1
    loadConstantOrVariablePayload(t0, CellTag, t3, .opGetByIdUnsetSlow)
    bineq JSCell::m_structureID[t3], t1, .opGetByIdUnsetSlow
    loadi 4[PC], t2
    storei UndefinedTag, TagOffset[cfr, t2, 8]
    storei 0, PayloadOffset[cfr, t2, 8]
    valueProfile(UndefinedTag, 0, 32, t2)
    dispatch(9)

.opGetByIdUnsetSlow:
    callOpcodeSlowPath(_llint_slow_path_get_by_id)
    dispatch(9)


_llint_op_get_array_length:
    traceExecution()
    loadi 8[PC], t0
    loadp 16[PC], t1
    loadConstantOrVariablePayload(t0, CellTag, t3, .opGetArrayLengthSlow)
    move t3, t2
    arrayProfile(t2, t1, t0)
    btiz t2, IsArray, .opGetArrayLengthSlow
    btiz t2, IndexingShapeMask, .opGetArrayLengthSlow
    loadi 4[PC], t1
    loadp JSObject::m_butterfly[t3], t0
    loadi -sizeof IndexingHeader + IndexingHeader::u.lengths.publicLength[t0], t0
    bilt t0, 0, .opGetArrayLengthSlow
    valueProfile(Int32Tag, t0, 32, t2)
    storep t0, PayloadOffset[cfr, t1, 8]
    storep Int32Tag, TagOffset[cfr, t1, 8]
    dispatch(9)

.opGetArrayLengthSlow:
    callOpcodeSlowPath(_llint_slow_path_get_by_id)
    dispatch(9)


_llint_op_put_by_id:
    traceExecution()
    writeBarrierOnOperands(1, 3)
    loadi 4[PC], t3
    loadConstantOrVariablePayload(t3, CellTag, t0, .opPutByIdSlow)
    loadi JSCell::m_structureID[t0], t2
    bineq t2, 16[PC], .opPutByIdSlow

    # At this point, we have:
    # t2 -> currentStructureID
    # t0 -> object base
    # We will lose currentStructureID in the shenanigans below.

    loadi 12[PC], t1
    loadConstantOrVariable(t1, t2, t3)
    loadi 32[PC], t1

    # At this point, we have:
    # t0 -> object base
    # t1 -> put by id flags
    # t2 -> value tag
    # t3 -> value payload

    btinz t1, PutByIdPrimaryTypeMask, .opPutByIdTypeCheckObjectWithStructureOrOther

    # We have one of the non-structure type checks. Find out which one.
    andi PutByIdSecondaryTypeMask, t1
    bilt t1, PutByIdSecondaryTypeString, .opPutByIdTypeCheckLessThanString

    # We are one of the following: String, Symbol, Object, ObjectOrOther, Top
    bilt t1, PutByIdSecondaryTypeObjectOrOther, .opPutByIdTypeCheckLessThanObjectOrOther

    # We are either ObjectOrOther or Top.
    bieq t1, PutByIdSecondaryTypeTop, .opPutByIdDoneCheckingTypes

    # Check if we are ObjectOrOther.
    bieq t2, CellTag, .opPutByIdTypeCheckObject
.opPutByIdTypeCheckOther:
    bieq t2, NullTag, .opPutByIdDoneCheckingTypes
    bieq t2, UndefinedTag, .opPutByIdDoneCheckingTypes
    jmp .opPutByIdSlow

.opPutByIdTypeCheckLessThanObjectOrOther:
    # We are either String, Symbol or Object.
    bineq t2, CellTag, .opPutByIdSlow
    bieq t1, PutByIdSecondaryTypeObject, .opPutByIdTypeCheckObject
    bieq t1, PutByIdSecondaryTypeSymbol, .opPutByIdTypeCheckSymbol
    bbeq JSCell::m_type[t3], StringType, .opPutByIdDoneCheckingTypes
    jmp .opPutByIdSlow
.opPutByIdTypeCheckObject:
    bbaeq JSCell::m_type[t3], ObjectType, .opPutByIdDoneCheckingTypes
    jmp .opPutByIdSlow
.opPutByIdTypeCheckSymbol:
    bbeq JSCell::m_type[t3], SymbolType, .opPutByIdDoneCheckingTypes
    jmp .opPutByIdSlow

.opPutByIdTypeCheckLessThanString:
    # We are one of the following: Bottom, Boolean, Other, Int32, Number.
    bilt t1, PutByIdSecondaryTypeInt32, .opPutByIdTypeCheckLessThanInt32

    # We are either Int32 or Number.
    bieq t1, PutByIdSecondaryTypeNumber, .opPutByIdTypeCheckNumber

    bieq t2, Int32Tag, .opPutByIdDoneCheckingTypes
    jmp .opPutByIdSlow

.opPutByIdTypeCheckNumber:
    bib t2, LowestTag + 1, .opPutByIdDoneCheckingTypes
    jmp .opPutByIdSlow

.opPutByIdTypeCheckLessThanInt32:
    # We are one of the following: Bottom, Boolean, Other
    bineq t1, PutByIdSecondaryTypeBoolean, .opPutByIdTypeCheckBottomOrOther
    bieq t2, BooleanTag, .opPutByIdDoneCheckingTypes
    jmp .opPutByIdSlow

.opPutByIdTypeCheckBottomOrOther:
    bieq t1, PutByIdSecondaryTypeOther, .opPutByIdTypeCheckOther
    jmp .opPutByIdSlow

.opPutByIdTypeCheckObjectWithStructureOrOther:
    bieq t2, CellTag, .opPutByIdTypeCheckObjectWithStructure
    btinz t1, PutByIdPrimaryTypeObjectWithStructureOrOther, .opPutByIdTypeCheckOther
    jmp .opPutByIdSlow

.opPutByIdTypeCheckObjectWithStructure:
    andi PutByIdSecondaryTypeMask, t1
    bineq t1, JSCell::m_structureID[t3], .opPutByIdSlow

.opPutByIdDoneCheckingTypes:
    loadi 24[PC], t1

    btiz t1, .opPutByIdNotTransition

    # This is the transition case. t1 holds the new Structure*. If we have a chain, we need to
    # check it. t0 is the base. We may clobber t1 to use it as scratch.
    loadp 28[PC], t3
    btpz t3, .opPutByIdTransitionDirect

    loadi 16[PC], t2 # Need old structure again.
    loadp StructureChain::m_vector[t3], t3
    assert(macro (ok) btpnz t3, ok end)

    loadp Structure::m_prototype[t2], t2
    btpz t2, .opPutByIdTransitionChainDone
.opPutByIdTransitionChainLoop:
    loadp [t3], t1
    bpneq t1, JSCell::m_structureID[t2], .opPutByIdSlow
    addp 4, t3
    loadp Structure::m_prototype[t1], t2
    btpnz t2, .opPutByIdTransitionChainLoop

.opPutByIdTransitionChainDone:
    loadi 24[PC], t1

.opPutByIdTransitionDirect:
    storei t1, JSCell::m_structureID[t0]
    loadi 12[PC], t1
    loadConstantOrVariable(t1, t2, t3)
    loadi 20[PC], t1
    storePropertyAtVariableOffset(t1, t0, t2, t3)
    writeBarrierOnOperand(1)
    dispatch(9)

.opPutByIdNotTransition:
    # The only thing live right now is t0, which holds the base.
    loadi 12[PC], t1
    loadConstantOrVariable(t1, t2, t3)
    loadi 20[PC], t1
    storePropertyAtVariableOffset(t1, t0, t2, t3)
    dispatch(9)

.opPutByIdSlow:
    callOpcodeSlowPath(_llint_slow_path_put_by_id)
    dispatch(9)


_llint_op_get_by_val:
    traceExecution()
    loadi 8[PC], t2
    loadConstantOrVariablePayload(t2, CellTag, t0, .opGetByValSlow)
    move t0, t2
    loadp 16[PC], t3
    arrayProfile(t2, t3, t1)
    loadi 12[PC], t3
    loadConstantOrVariablePayload(t3, Int32Tag, t1, .opGetByValSlow)
    loadp JSObject::m_butterfly[t0], t3
    andi IndexingShapeMask, t2
    bieq t2, Int32Shape, .opGetByValIsContiguous
    bineq t2, ContiguousShape, .opGetByValNotContiguous
.opGetByValIsContiguous:
    
    biaeq t1, -sizeof IndexingHeader + IndexingHeader::u.lengths.publicLength[t3], .opGetByValOutOfBounds
    loadi TagOffset[t3, t1, 8], t2
    loadi PayloadOffset[t3, t1, 8], t1
    jmp .opGetByValDone

.opGetByValNotContiguous:
    bineq t2, DoubleShape, .opGetByValNotDouble
    biaeq t1, -sizeof IndexingHeader + IndexingHeader::u.lengths.publicLength[t3], .opGetByValOutOfBounds
    loadd [t3, t1, 8], ft0
    bdnequn ft0, ft0, .opGetByValSlow
    # FIXME: This could be massively optimized.
    fd2ii ft0, t1, t2
    loadi 4[PC], t0
    jmp .opGetByValNotEmpty

.opGetByValNotDouble:
    subi ArrayStorageShape, t2
    bia t2, SlowPutArrayStorageShape - ArrayStorageShape, .opGetByValSlow
    biaeq t1, -sizeof IndexingHeader + IndexingHeader::u.lengths.vectorLength[t3], .opGetByValOutOfBounds
    loadi ArrayStorage::m_vector + TagOffset[t3, t1, 8], t2
    loadi ArrayStorage::m_vector + PayloadOffset[t3, t1, 8], t1

.opGetByValDone:
    loadi 4[PC], t0
    bieq t2, EmptyValueTag, .opGetByValOutOfBounds
.opGetByValNotEmpty:
    storei t2, TagOffset[cfr, t0, 8]
    storei t1, PayloadOffset[cfr, t0, 8]
    valueProfile(t2, t1, 20, t0)
    dispatch(6)

.opGetByValOutOfBounds:
    loadpFromInstruction(4, t0)
    storeb 1, ArrayProfile::m_outOfBounds[t0]
.opGetByValSlow:
    callOpcodeSlowPath(_llint_slow_path_get_by_val)
    dispatch(6)


macro contiguousPutByVal(storeCallback)
    biaeq t3, -sizeof IndexingHeader + IndexingHeader::u.lengths.publicLength[t0], .outOfBounds
.storeResult:
    loadi 12[PC], t2
    storeCallback(t2, t1, t0, t3)
    dispatch(5)

.outOfBounds:
    biaeq t3, -sizeof IndexingHeader + IndexingHeader::u.lengths.vectorLength[t0], .opPutByValOutOfBounds
    loadp 16[PC], t2
    storeb 1, ArrayProfile::m_mayStoreToHole[t2]
    addi 1, t3, t2
    storei t2, -sizeof IndexingHeader + IndexingHeader::u.lengths.publicLength[t0]
    jmp .storeResult
end

macro putByVal(slowPath)
    traceExecution()
    writeBarrierOnOperands(1, 3)
    loadi 4[PC], t0
    loadConstantOrVariablePayload(t0, CellTag, t1, .opPutByValSlow)
    move t1, t2
    loadp 16[PC], t3
    arrayProfile(t2, t3, t0)
    loadi 8[PC], t0
    loadConstantOrVariablePayload(t0, Int32Tag, t3, .opPutByValSlow)
    loadp JSObject::m_butterfly[t1], t0
    andi IndexingShapeMask, t2
    bineq t2, Int32Shape, .opPutByValNotInt32
    contiguousPutByVal(
        macro (operand, scratch, base, index)
            loadConstantOrVariablePayload(operand, Int32Tag, scratch, .opPutByValSlow)
            storei Int32Tag, TagOffset[base, index, 8]
            storei scratch, PayloadOffset[base, index, 8]
        end)

.opPutByValNotInt32:
    bineq t2, DoubleShape, .opPutByValNotDouble
    contiguousPutByVal(
        macro (operand, scratch, base, index)
            const tag = scratch
            const payload = operand
            loadConstantOrVariable2Reg(operand, tag, payload)
            bineq tag, Int32Tag, .notInt
            ci2d payload, ft0
            jmp .ready
        .notInt:
            fii2d payload, tag, ft0
            bdnequn ft0, ft0, .opPutByValSlow
        .ready:
            stored ft0, [base, index, 8]
        end)

.opPutByValNotDouble:
    bineq t2, ContiguousShape, .opPutByValNotContiguous
    contiguousPutByVal(
        macro (operand, scratch, base, index)
            const tag = scratch
            const payload = operand
            loadConstantOrVariable2Reg(operand, tag, payload)
            storei tag, TagOffset[base, index, 8]
            storei payload, PayloadOffset[base, index, 8]
        end)

.opPutByValNotContiguous:
    bineq t2, ArrayStorageShape, .opPutByValSlow
    biaeq t3, -sizeof IndexingHeader + IndexingHeader::u.lengths.vectorLength[t0], .opPutByValOutOfBounds
    bieq ArrayStorage::m_vector + TagOffset[t0, t3, 8], EmptyValueTag, .opPutByValArrayStorageEmpty
.opPutByValArrayStorageStoreResult:
    loadi 12[PC], t2
    loadConstantOrVariable2Reg(t2, t1, t2)
    storei t1, ArrayStorage::m_vector + TagOffset[t0, t3, 8]
    storei t2, ArrayStorage::m_vector + PayloadOffset[t0, t3, 8]
    dispatch(5)

.opPutByValArrayStorageEmpty:
    loadp 16[PC], t1
    storeb 1, ArrayProfile::m_mayStoreToHole[t1]
    addi 1, ArrayStorage::m_numValuesInVector[t0]
    bib t3, -sizeof IndexingHeader + IndexingHeader::u.lengths.publicLength[t0], .opPutByValArrayStorageStoreResult
    addi 1, t3, t1
    storei t1, -sizeof IndexingHeader + IndexingHeader::u.lengths.publicLength[t0]
    jmp .opPutByValArrayStorageStoreResult

.opPutByValOutOfBounds:
    loadpFromInstruction(4, t0)
    storeb 1, ArrayProfile::m_outOfBounds[t0]
.opPutByValSlow:
    callOpcodeSlowPath(slowPath)
    dispatch(5)
end

_llint_op_put_by_val:
    putByVal(_llint_slow_path_put_by_val)

_llint_op_put_by_val_direct:
    putByVal(_llint_slow_path_put_by_val_direct)

_llint_op_jmp:
    traceExecution()
    dispatchBranch(4[PC])


macro jumpTrueOrFalse(conditionOp, slow)
    loadi 4[PC], t1
    loadConstantOrVariablePayload(t1, BooleanTag, t0, .slow)
    conditionOp(t0, .target)
    dispatch(3)

.target:
    dispatchBranch(8[PC])

.slow:
    callOpcodeSlowPath(slow)
    dispatch(0)
end


macro equalNull(cellHandler, immediateHandler)
    loadi 4[PC], t0
    assertNotConstant(t0)
    loadi TagOffset[cfr, t0, 8], t1
    loadi PayloadOffset[cfr, t0, 8], t0
    bineq t1, CellTag, .immediate
    loadp JSCell::m_structureID[t0], t2
    cellHandler(t2, JSCell::m_flags[t0], .target)
    dispatch(3)

.target:
    dispatchBranch(8[PC])

.immediate:
    ori 1, t1
    immediateHandler(t1, .target)
    dispatch(3)
end

_llint_op_jeq_null:
    traceExecution()
    equalNull(
        macro (structure, value, target) 
            btbz value, MasqueradesAsUndefined, .opJeqNullNotMasqueradesAsUndefined
            loadp CodeBlock[cfr], t0
            loadp CodeBlock::m_globalObject[t0], t0
            bpeq Structure::m_globalObject[structure], t0, target
.opJeqNullNotMasqueradesAsUndefined:
        end,
        macro (value, target) bieq value, NullTag, target end)
    

_llint_op_jneq_null:
    traceExecution()
    equalNull(
        macro (structure, value, target) 
            btbz value, MasqueradesAsUndefined, target 
            loadp CodeBlock[cfr], t0
            loadp CodeBlock::m_globalObject[t0], t0
            bpneq Structure::m_globalObject[structure], t0, target
        end,
        macro (value, target) bineq value, NullTag, target end)


_llint_op_jneq_ptr:
    traceExecution()
    loadi 4[PC], t0
    loadi 8[PC], t1
    loadp CodeBlock[cfr], t2
    loadp CodeBlock::m_globalObject[t2], t2
    bineq TagOffset[cfr, t0, 8], CellTag, .opJneqPtrBranch
    loadp JSGlobalObject::m_specialPointers[t2, t1, 4], t1
    bpeq PayloadOffset[cfr, t0, 8], t1, .opJneqPtrFallThrough
.opJneqPtrBranch:
    storei 1, 16[PC]
    dispatchBranch(12[PC])
.opJneqPtrFallThrough:
    dispatch(5)


macro compare(integerCompare, doubleCompare, slowPath)
    loadi 4[PC], t2
    loadi 8[PC], t3
    loadConstantOrVariable(t2, t0, t1)
    loadConstantOrVariable2Reg(t3, t2, t3)
    bineq t0, Int32Tag, .op1NotInt
    bineq t2, Int32Tag, .op2NotInt
    integerCompare(t1, t3, .jumpTarget)
    dispatch(4)

.op1NotInt:
    bia t0, LowestTag, .slow
    bib t2, LowestTag, .op1NotIntOp2Double
    bineq t2, Int32Tag, .slow
    ci2d t3, ft1
    jmp .op1NotIntReady
.op1NotIntOp2Double:
    fii2d t3, t2, ft1
.op1NotIntReady:
    fii2d t1, t0, ft0
    doubleCompare(ft0, ft1, .jumpTarget)
    dispatch(4)

.op2NotInt:
    ci2d t1, ft0
    bia t2, LowestTag, .slow
    fii2d t3, t2, ft1
    doubleCompare(ft0, ft1, .jumpTarget)
    dispatch(4)

.jumpTarget:
    dispatchBranch(12[PC])

.slow:
    callOpcodeSlowPath(slowPath)
    dispatch(0)
end


_llint_op_switch_imm:
    traceExecution()
    loadi 12[PC], t2
    loadi 4[PC], t3
    loadConstantOrVariable(t2, t1, t0)
    loadp CodeBlock[cfr], t2
    loadp CodeBlock::m_rareData[t2], t2
    muli sizeof SimpleJumpTable, t3   # FIXME: would be nice to peephole this!
    loadp CodeBlock::RareData::m_switchJumpTables + VectorBufferOffset[t2], t2
    addp t3, t2
    bineq t1, Int32Tag, .opSwitchImmNotInt
    subi SimpleJumpTable::min[t2], t0
    biaeq t0, SimpleJumpTable::branchOffsets + VectorSizeOffset[t2], .opSwitchImmFallThrough
    loadp SimpleJumpTable::branchOffsets + VectorBufferOffset[t2], t3
    loadi [t3, t0, 4], t1
    btiz t1, .opSwitchImmFallThrough
    dispatchBranchWithOffset(t1)

.opSwitchImmNotInt:
    bib t1, LowestTag, .opSwitchImmSlow  # Go to slow path if it's a double.
.opSwitchImmFallThrough:
    dispatchBranch(8[PC])

.opSwitchImmSlow:
    callOpcodeSlowPath(_llint_slow_path_switch_imm)
    dispatch(0)


_llint_op_switch_char:
    traceExecution()
    loadi 12[PC], t2
    loadi 4[PC], t3
    loadConstantOrVariable(t2, t1, t0)
    loadp CodeBlock[cfr], t2
    loadp CodeBlock::m_rareData[t2], t2
    muli sizeof SimpleJumpTable, t3
    loadp CodeBlock::RareData::m_switchJumpTables + VectorBufferOffset[t2], t2
    addp t3, t2
    bineq t1, CellTag, .opSwitchCharFallThrough
    bbneq JSCell::m_type[t0], StringType, .opSwitchCharFallThrough
    bineq JSString::m_length[t0], 1, .opSwitchCharFallThrough
    loadp JSString::m_value[t0], t0
    btpz  t0, .opSwitchOnRope
    loadp StringImpl::m_data8[t0], t1
    btinz StringImpl::m_hashAndFlags[t0], HashFlags8BitBuffer, .opSwitchChar8Bit
    loadh [t1], t0
    jmp .opSwitchCharReady
.opSwitchChar8Bit:
    loadb [t1], t0
.opSwitchCharReady:
    subi SimpleJumpTable::min[t2], t0
    biaeq t0, SimpleJumpTable::branchOffsets + VectorSizeOffset[t2], .opSwitchCharFallThrough
    loadp SimpleJumpTable::branchOffsets + VectorBufferOffset[t2], t2
    loadi [t2, t0, 4], t1
    btiz t1, .opSwitchCharFallThrough
    dispatchBranchWithOffset(t1)

.opSwitchCharFallThrough:
    dispatchBranch(8[PC])

.opSwitchOnRope:
    callOpcodeSlowPath(_llint_slow_path_switch_char)
    dispatch(0)


macro arrayProfileForCall()
    loadi 16[PC], t3
    negi t3
    bineq ThisArgumentOffset + TagOffset[cfr, t3, 8], CellTag, .done
    loadi ThisArgumentOffset + PayloadOffset[cfr, t3, 8], t0
    loadp JSCell::m_structureID[t0], t0
    loadpFromInstruction(CallOpCodeSize - 2, t1)
    storep t0, ArrayProfile::m_lastSeenStructureID[t1]
.done:
end

macro doCall(slowPath, prepareCall)
    loadi 8[PC], t0
    loadi 20[PC], t1
    loadp LLIntCallLinkInfo::callee[t1], t2
    loadConstantOrVariablePayload(t0, CellTag, t3, .opCallSlow)
    bineq t3, t2, .opCallSlow
    loadi 16[PC], t3
    lshifti 3, t3
    negi t3
    addp cfr, t3  # t3 contains the new value of cfr
    storei t2, Callee + PayloadOffset[t3]
    loadi 12[PC], t2
    storei PC, ArgumentCount + TagOffset[cfr]
    storei t2, ArgumentCount + PayloadOffset[t3]
    storei CellTag, Callee + TagOffset[t3]
    move t3, sp
    prepareCall(LLIntCallLinkInfo::machineCodeTarget[t1], t2, t3, t4)
    callTargetFunction(LLIntCallLinkInfo::machineCodeTarget[t1])

.opCallSlow:
    slowPathForCall(slowPath, prepareCall)
end

_llint_op_ret:
    traceExecution()
    checkSwitchToJITForEpilogue()
    loadi 4[PC], t2
    loadConstantOrVariable(t2, t1, t0)
    doReturn()


_llint_op_to_primitive:
    traceExecution()
    loadi 8[PC], t2
    loadi 4[PC], t3
    loadConstantOrVariable(t2, t1, t0)
    bineq t1, CellTag, .opToPrimitiveIsImm
    bbaeq JSCell::m_type[t0], ObjectType, .opToPrimitiveSlowCase
.opToPrimitiveIsImm:
    storei t1, TagOffset[cfr, t3, 8]
    storei t0, PayloadOffset[cfr, t3, 8]
    dispatch(3)

.opToPrimitiveSlowCase:
    callOpcodeSlowPath(_slow_path_to_primitive)
    dispatch(3)


_llint_op_catch:
    # This is where we end up from the JIT's throw trampoline (because the
    # machine code return address will be set to _llint_op_catch), and from
    # the interpreter's throw trampoline (see _llint_throw_trampoline).
    # The throwing code must have known that we were throwing to the interpreter,
    # and have set VM::targetInterpreterPCForThrow.
    loadp Callee + PayloadOffset[cfr], t3
    andp MarkedBlockMask, t3
    loadp MarkedBlock::m_vm[t3], t3
    restoreCalleeSavesFromVMEntryFrameCalleeSavesBuffer(t3, t0)
    loadp VM::callFrameForCatch[t3], cfr
    storep 0, VM::callFrameForCatch[t3]
    restoreStackPointerAfterCall()

    loadi VM::targetInterpreterPCForThrow[t3], PC

    callOpcodeSlowPath(_llint_slow_path_check_if_exception_is_uncatchable_and_notify_profiler)
    bpeq r1, 0, .isCatchableException
    jmp _llint_throw_from_slow_path_trampoline

.isCatchableException:
    loadp Callee + PayloadOffset[cfr], t3
    andp MarkedBlockMask, t3
    loadp MarkedBlock::m_vm[t3], t3

    loadi VM::m_exception[t3], t0
    storei 0, VM::m_exception[t3]
    loadi 4[PC], t2
    storei t0, PayloadOffset[cfr, t2, 8]
    storei CellTag, TagOffset[cfr, t2, 8]

    loadi Exception::m_value + TagOffset[t0], t1
    loadi Exception::m_value + PayloadOffset[t0], t0
    loadi 8[PC], t2
    storei t0, PayloadOffset[cfr, t2, 8]
    storei t1, TagOffset[cfr, t2, 8]

    traceExecution()  # This needs to be here because we don't want to clobber t0, t1, t2, t3 above.
    dispatch(3)

_llint_op_end:
    traceExecution()
    checkSwitchToJITForEpilogue()
    loadi 4[PC], t0
    assertNotConstant(t0)
    loadi TagOffset[cfr, t0, 8], t1
    loadi PayloadOffset[cfr, t0, 8], t0
    doReturn()


_llint_throw_from_slow_path_trampoline:
    callSlowPath(_llint_slow_path_handle_exception)

    # When throwing from the interpreter (i.e. throwing from LLIntSlowPaths), so
    # the throw target is not necessarily interpreted code, we come to here.
    # This essentially emulates the JIT's throwing protocol.
    loadp Callee[cfr], t1
    andp MarkedBlockMask, t1
    loadp MarkedBlock::m_vm[t1], t1
    copyCalleeSavesToVMEntryFrameCalleeSavesBuffer(t1, t2)
    jmp VM::targetMachinePCForThrow[t1]


_llint_throw_during_call_trampoline:
    preserveReturnAddressAfterCall(t2)
    jmp _llint_throw_from_slow_path_trampoline


macro nativeCallTrampoline(executableOffsetToFunction)

    functionPrologue()
    storep 0, CodeBlock[cfr]
    loadi Callee + PayloadOffset[cfr], t1
    // Callee is still in t1 for code below
    if X86 or X86_WIN
        subp 8, sp # align stack pointer
        andp MarkedBlockMask, t1
        loadp MarkedBlock::m_vm[t1], t3
        storep cfr, VM::topCallFrame[t3]
        move cfr, a0  # a0 = ecx
        storep a0, [sp]
        loadi Callee + PayloadOffset[cfr], t1
        loadp JSFunction::m_executable[t1], t1
        checkStackPointerAlignment(t3, 0xdead0001)
        call executableOffsetToFunction[t1]
        loadp Callee + PayloadOffset[cfr], t3
        andp MarkedBlockMask, t3
        loadp MarkedBlock::m_vm[t3], t3
        addp 8, sp
    elsif ARM or ARMv7 or ARMv7_TRADITIONAL or C_LOOP or MIPS
        subp 8, sp # align stack pointer
        # t1 already contains the Callee.
        andp MarkedBlockMask, t1
        loadp MarkedBlock::m_vm[t1], t1
        storep cfr, VM::topCallFrame[t1]
        move cfr, a0
        loadi Callee + PayloadOffset[cfr], t1
        loadp JSFunction::m_executable[t1], t1
        checkStackPointerAlignment(t3, 0xdead0001)
        if C_LOOP
            cloopCallNative executableOffsetToFunction[t1]
        else
            call executableOffsetToFunction[t1]
        end
        loadp Callee + PayloadOffset[cfr], t3
        andp MarkedBlockMask, t3
        loadp MarkedBlock::m_vm[t3], t3
        addp 8, sp
    else
        error
    end
    
    btinz VM::m_exception[t3], .handleException

    functionEpilogue()
    ret

.handleException:
    storep cfr, VM::topCallFrame[t3]
    jmp _llint_throw_from_slow_path_trampoline
end


macro getConstantScope(dst)
    loadpFromInstruction(6, t0)
    loadisFromInstruction(dst, t1)
    storei CellTag, TagOffset[cfr, t1, 8]
    storei t0, PayloadOffset[cfr, t1, 8]
end

macro varInjectionCheck(slowPath)
    loadp CodeBlock[cfr], t0
    loadp CodeBlock::m_globalObject[t0], t0
    loadp JSGlobalObject::m_varInjectionWatchpoint[t0], t0
    bbeq WatchpointSet::m_state[t0], IsInvalidated, slowPath
end

macro resolveScope()
    loadp CodeBlock[cfr], t0
    loadisFromInstruction(5, t2)

    loadisFromInstruction(2, t0)
    loadp PayloadOffset[cfr, t0, 8], t0
    btiz t2, .resolveScopeLoopEnd

.resolveScopeLoop:
    loadp JSScope::m_next[t0], t0
    subi 1, t2
    btinz t2, .resolveScopeLoop

.resolveScopeLoopEnd:
    loadisFromInstruction(1, t1)
    storei CellTag, TagOffset[cfr, t1, 8]
    storei t0, PayloadOffset[cfr, t1, 8]
end


_llint_op_resolve_scope:
    traceExecution()
    loadisFromInstruction(4, t0)

#rGlobalProperty:
    bineq t0, GlobalProperty, .rGlobalVar
    getConstantScope(1)
    dispatch(7)

.rGlobalVar:
    bineq t0, GlobalVar, .rGlobalLexicalVar
    getConstantScope(1)
    dispatch(7)

.rGlobalLexicalVar:
    bineq t0, GlobalLexicalVar, .rClosureVar
    getConstantScope(1)
    dispatch(7)

.rClosureVar:
    bineq t0, ClosureVar, .rModuleVar
    resolveScope()
    dispatch(7)

.rModuleVar:
    bineq t0, ModuleVar, .rGlobalPropertyWithVarInjectionChecks
    getConstantScope(1)
    dispatch(7)

.rGlobalPropertyWithVarInjectionChecks:
    bineq t0, GlobalPropertyWithVarInjectionChecks, .rGlobalVarWithVarInjectionChecks
    varInjectionCheck(.rDynamic)
    getConstantScope(1)
    dispatch(7)

.rGlobalVarWithVarInjectionChecks:
    bineq t0, GlobalVarWithVarInjectionChecks, .rGlobalLexicalVarWithVarInjectionChecks
    varInjectionCheck(.rDynamic)
    getConstantScope(1)
    dispatch(7)

.rGlobalLexicalVarWithVarInjectionChecks:
    bineq t0, GlobalLexicalVarWithVarInjectionChecks, .rClosureVarWithVarInjectionChecks
    varInjectionCheck(.rDynamic)
    getConstantScope(1)
    dispatch(7)

.rClosureVarWithVarInjectionChecks:
    bineq t0, ClosureVarWithVarInjectionChecks, .rDynamic
    varInjectionCheck(.rDynamic)
    resolveScope()
    dispatch(7)

.rDynamic:
    callOpcodeSlowPath(_slow_path_resolve_scope)
    dispatch(7)


macro loadWithStructureCheck(operand, slowPath)
    loadisFromInstruction(operand, t0)
    loadp PayloadOffset[cfr, t0, 8], t0
    loadpFromInstruction(5, t1)
    bpneq JSCell::m_structureID[t0], t1, slowPath
end

macro getProperty()
    loadisFromInstruction(6, t3)
    loadPropertyAtVariableOffset(t3, t0, t1, t2)
    valueProfile(t1, t2, 28, t0)
    loadisFromInstruction(1, t0)
    storei t1, TagOffset[cfr, t0, 8]
    storei t2, PayloadOffset[cfr, t0, 8]
end

macro getGlobalVar(tdzCheckIfNecessary)
    loadpFromInstruction(6, t0)
    loadp TagOffset[t0], t1
    loadp PayloadOffset[t0], t2
    tdzCheckIfNecessary(t1)
    valueProfile(t1, t2, 28, t0)
    loadisFromInstruction(1, t0)
    storei t1, TagOffset[cfr, t0, 8]
    storei t2, PayloadOffset[cfr, t0, 8]
end

macro getClosureVar()
    loadisFromInstruction(6, t3)
    loadp JSEnvironmentRecord_variables + TagOffset[t0, t3, 8], t1
    loadp JSEnvironmentRecord_variables + PayloadOffset[t0, t3, 8], t2
    valueProfile(t1, t2, 28, t0)
    loadisFromInstruction(1, t0)
    storei t1, TagOffset[cfr, t0, 8]
    storei t2, PayloadOffset[cfr, t0, 8]
end

_llint_op_get_from_scope:
    traceExecution()
    loadisFromInstruction(4, t0)
    andi ResolveTypeMask, t0

#gGlobalProperty:
    bineq t0, GlobalProperty, .gGlobalVar
    loadWithStructureCheck(2, .gDynamic)
    getProperty()
    dispatch(8)

.gGlobalVar:
    bineq t0, GlobalVar, .gGlobalLexicalVar
    getGlobalVar(macro(t) end)
    dispatch(8)

.gGlobalLexicalVar:
    bineq t0, GlobalLexicalVar, .gClosureVar
    getGlobalVar(
        macro(tag)
            bieq tag, EmptyValueTag, .gDynamic
        end)
    dispatch(8)

.gClosureVar:
    bineq t0, ClosureVar, .gGlobalPropertyWithVarInjectionChecks
    loadVariable(2, t2, t1, t0)
    getClosureVar()
    dispatch(8)

.gGlobalPropertyWithVarInjectionChecks:
    bineq t0, GlobalPropertyWithVarInjectionChecks, .gGlobalVarWithVarInjectionChecks
    loadWithStructureCheck(2, .gDynamic)
    getProperty()
    dispatch(8)

.gGlobalVarWithVarInjectionChecks:
    bineq t0, GlobalVarWithVarInjectionChecks, .gGlobalLexicalVarWithVarInjectionChecks
    varInjectionCheck(.gDynamic)
    getGlobalVar(macro(t) end)
    dispatch(8)

.gGlobalLexicalVarWithVarInjectionChecks:
    bineq t0, GlobalLexicalVarWithVarInjectionChecks, .gClosureVarWithVarInjectionChecks
    varInjectionCheck(.gDynamic)
    getGlobalVar(
        macro(tag)
            bieq tag, EmptyValueTag, .gDynamic
        end)
    dispatch(8)

.gClosureVarWithVarInjectionChecks:
    bineq t0, ClosureVarWithVarInjectionChecks, .gDynamic
    varInjectionCheck(.gDynamic)
    loadVariable(2, t2, t1, t0)
    getClosureVar()
    dispatch(8)

.gDynamic:
    callOpcodeSlowPath(_llint_slow_path_get_from_scope)
    dispatch(8)


macro putProperty()
    loadisFromInstruction(3, t1)
    loadConstantOrVariable(t1, t2, t3)
    loadisFromInstruction(6, t1)
    storePropertyAtVariableOffset(t1, t0, t2, t3)
end

macro putGlobalVariable()
    loadisFromInstruction(3, t0)
    loadConstantOrVariable(t0, t1, t2)
    loadpFromInstruction(5, t3)
    notifyWrite(t3, .pDynamic)
    loadpFromInstruction(6, t0)
    storei t1, TagOffset[t0]
    storei t2, PayloadOffset[t0]
end

macro putClosureVar()
    loadisFromInstruction(3, t1)
    loadConstantOrVariable(t1, t2, t3)
    loadisFromInstruction(6, t1)
    storei t2, JSEnvironmentRecord_variables + TagOffset[t0, t1, 8]
    storei t3, JSEnvironmentRecord_variables + PayloadOffset[t0, t1, 8]
end

macro putLocalClosureVar()
    loadisFromInstruction(3, t1)
    loadConstantOrVariable(t1, t2, t3)
    loadpFromInstruction(5, t5)
    btpz t5, .noVariableWatchpointSet
    notifyWrite(t5, .pDynamic)
.noVariableWatchpointSet:
    loadisFromInstruction(6, t1)
    storei t2, JSEnvironmentRecord_variables + TagOffset[t0, t1, 8]
    storei t3, JSEnvironmentRecord_variables + PayloadOffset[t0, t1, 8]
end


_llint_op_put_to_scope:
    traceExecution()
    loadisFromInstruction(4, t0)
    andi ResolveTypeMask, t0

#pLocalClosureVar:
    bineq t0, LocalClosureVar, .pGlobalProperty
    writeBarrierOnOperands(1, 3)
    loadVariable(1, t2, t1, t0)
    putLocalClosureVar()
    dispatch(7)

.pGlobalProperty:
    bineq t0, GlobalProperty, .pGlobalVar
    writeBarrierOnOperands(1, 3)
    loadWithStructureCheck(1, .pDynamic)
    putProperty()
    dispatch(7)

.pGlobalVar:
    bineq t0, GlobalVar, .pGlobalLexicalVar
    writeBarrierOnGlobalObject(3)
    putGlobalVariable()
    dispatch(7)

.pGlobalLexicalVar:
    bineq t0, GlobalLexicalVar, .pClosureVar
    writeBarrierOnGlobalLexicalEnvironment(3)
    putGlobalVariable()
    dispatch(7)

.pClosureVar:
    bineq t0, ClosureVar, .pGlobalPropertyWithVarInjectionChecks
    writeBarrierOnOperands(1, 3)
    loadVariable(1, t2, t1, t0)
    putClosureVar()
    dispatch(7)

.pGlobalPropertyWithVarInjectionChecks:
    bineq t0, GlobalPropertyWithVarInjectionChecks, .pGlobalVarWithVarInjectionChecks
    writeBarrierOnOperands(1, 3)
    loadWithStructureCheck(1, .pDynamic)
    putProperty()
    dispatch(7)

.pGlobalVarWithVarInjectionChecks:
    bineq t0, GlobalVarWithVarInjectionChecks, .pGlobalLexicalVarWithVarInjectionChecks
    writeBarrierOnGlobalObject(3)
    varInjectionCheck(.pDynamic)
    putGlobalVariable()
    dispatch(7)

.pGlobalLexicalVarWithVarInjectionChecks:
    bineq t0, GlobalLexicalVarWithVarInjectionChecks, .pClosureVarWithVarInjectionChecks
    writeBarrierOnGlobalLexicalEnvironment(3)
    varInjectionCheck(.pDynamic)
    putGlobalVariable()
    dispatch(7)

.pClosureVarWithVarInjectionChecks:
    bineq t0, ClosureVarWithVarInjectionChecks, .pModuleVar
    writeBarrierOnOperands(1, 3)
    varInjectionCheck(.pDynamic)
    loadVariable(1, t2, t1, t0)
    putClosureVar()
    dispatch(7)

.pModuleVar:
    bineq t0, ModuleVar, .pDynamic
    callOpcodeSlowPath(_slow_path_throw_strict_mode_readonly_property_write_error)
    dispatch(7)

.pDynamic:
    callOpcodeSlowPath(_llint_slow_path_put_to_scope)
    dispatch(7)


_llint_op_get_from_arguments:
    traceExecution()
    loadisFromInstruction(2, t0)
    loadi PayloadOffset[cfr, t0, 8], t0
    loadi 12[PC], t1
    loadi DirectArguments_storage + TagOffset[t0, t1, 8], t2
    loadi DirectArguments_storage + PayloadOffset[t0, t1, 8], t3
    loadisFromInstruction(1, t1)
    valueProfile(t2, t3, 16, t0)
    storei t2, TagOffset[cfr, t1, 8]
    storei t3, PayloadOffset[cfr, t1, 8]
    dispatch(5)


_llint_op_put_to_arguments:
    traceExecution()
    writeBarrierOnOperands(1, 3)
    loadisFromInstruction(1, t0)
    loadi PayloadOffset[cfr, t0, 8], t0
    loadisFromInstruction(3, t1)
    loadConstantOrVariable(t1, t2, t3)
    loadi 8[PC], t1
    storei t2, DirectArguments_storage + TagOffset[t0, t1, 8]
    storei t3, DirectArguments_storage + PayloadOffset[t0, t1, 8]
    dispatch(4)


_llint_op_get_parent_scope:
    traceExecution()
    loadisFromInstruction(2, t0)
    loadp PayloadOffset[cfr, t0, 8], t0
    loadp JSScope::m_next[t0], t0
    loadisFromInstruction(1, t1)
    storei CellTag, TagOffset[cfr, t1, 8]
    storei t0, PayloadOffset[cfr, t1, 8]
    dispatch(3)


_llint_op_profile_type:
    traceExecution()
    loadp CodeBlock[cfr], t1
    loadp CodeBlock::m_vm[t1], t1
    # t1 is holding the pointer to the typeProfilerLog.
    loadp VM::m_typeProfilerLog[t1], t1

    # t0 is holding the payload, t5 is holding the tag.
    loadisFromInstruction(1, t2)
    loadConstantOrVariable(t2, t5, t0)

    bieq t5, EmptyValueTag, .opProfileTypeDone

    # t2 is holding the pointer to the current log entry.
    loadp TypeProfilerLog::m_currentLogEntryPtr[t1], t2

    # Store the JSValue onto the log entry.
    storei t5, TypeProfilerLog::LogEntry::value + TagOffset[t2]
    storei t0, TypeProfilerLog::LogEntry::value + PayloadOffset[t2]

    # Store the TypeLocation onto the log entry.
    loadpFromInstruction(2, t3)
    storep t3, TypeProfilerLog::LogEntry::location[t2]

    bieq t5, CellTag, .opProfileTypeIsCell
    storei 0, TypeProfilerLog::LogEntry::structureID[t2]
    jmp .opProfileTypeSkipIsCell
.opProfileTypeIsCell:
    loadi JSCell::m_structureID[t0], t3
    storei t3, TypeProfilerLog::LogEntry::structureID[t2]
.opProfileTypeSkipIsCell:
    
    # Increment the current log entry.
    addp sizeof TypeProfilerLog::LogEntry, t2
    storep t2, TypeProfilerLog::m_currentLogEntryPtr[t1]

    loadp TypeProfilerLog::m_logEndPtr[t1], t1
    bpneq t2, t1, .opProfileTypeDone
    callOpcodeSlowPath(_slow_path_profile_type_clear_log)

.opProfileTypeDone:
    dispatch(6)


_llint_op_profile_control_flow:
    traceExecution()
    loadpFromInstruction(1, t0)
    loadi BasicBlockLocation::m_executionCount[t0], t1
    addi 1, t1
    bieq t1, 0, .done # We overflowed.
    storei t1, BasicBlockLocation::m_executionCount[t0]
.done:
    dispatch(2)


_llint_op_get_rest_length:
    traceExecution()
    loadi PayloadOffset + ArgumentCount[cfr], t0
    subi 1, t0
    loadisFromInstruction(2, t1)
    bilteq t0, t1, .storeZero
    subi t1, t0
    jmp .finish
.storeZero:
    move 0, t0
.finish:
    loadisFromInstruction(1, t1)
    storei t0, PayloadOffset[cfr, t1, 8]
    storei Int32Tag, TagOffset[cfr, t1, 8]
    dispatch(3)


_llint_op_log_shadow_chicken_prologue:
    traceExecution()
    acquireShadowChickenPacket(.opLogShadowChickenPrologueSlow)
    storep cfr, ShadowChicken::Packet::frame[t0]
    loadp CallerFrame[cfr], t1
    storep t1, ShadowChicken::Packet::callerFrame[t0]
    loadp Callee + PayloadOffset[cfr], t1
    storep t1, ShadowChicken::Packet::callee[t0]
    loadisFromInstruction(1, t1)
    loadi PayloadOffset[cfr, t1, 8], t1
    storep t1, ShadowChicken::Packet::scope[t0]
    dispatch(2)
.opLogShadowChickenPrologueSlow:
    callOpcodeSlowPath(_llint_slow_path_log_shadow_chicken_prologue)
    dispatch(2)


_llint_op_log_shadow_chicken_tail:
    traceExecution()
    acquireShadowChickenPacket(.opLogShadowChickenTailSlow)
    storep cfr, ShadowChicken::Packet::frame[t0]
    storep ShadowChickenTailMarker, ShadowChicken::Packet::callee[t0]
    loadVariable(1, t3, t2, t1)
    storei t2, TagOffset + ShadowChicken::Packet::thisValue[t0]
    storei t1, PayloadOffset + ShadowChicken::Packet::thisValue[t0]
    loadisFromInstruction(2, t1)
    loadi PayloadOffset[cfr, t1, 8], t1
    storep t1, ShadowChicken::Packet::scope[t0]
    loadp CodeBlock[cfr], t1
    storep t1, ShadowChicken::Packet::codeBlock[t0]
    storei PC, ShadowChicken::Packet::callSiteIndex[t0]
    dispatch(3)
.opLogShadowChickenTailSlow:
    callOpcodeSlowPath(_llint_slow_path_log_shadow_chicken_tail)
    dispatch(3)
