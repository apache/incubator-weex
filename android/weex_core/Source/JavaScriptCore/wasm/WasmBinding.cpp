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
#include "WasmBinding.h"

#if ENABLE(WEBASSEMBLY)

#include "CCallHelpers.h"
#include "FrameTracers.h"
#include "JITExceptions.h"
#include "JSCInlines.h"
#include "JSWebAssemblyInstance.h"
#include "LinkBuffer.h"
#include "NativeErrorConstructor.h"
#include "WasmCallingConvention.h"
#include "WasmExceptionType.h"

namespace JSC { namespace Wasm {

typedef CCallHelpers JIT;

static void materializeImportJSCell(VM* vm, JIT& jit, unsigned importIndex, GPRReg result)
{
    // We're calling out of the current WebAssembly.Instance, which is identified on VM. That Instance has a list of all its import functions.
    jit.loadPtr(&vm->topJSWebAssemblyInstance, result);
    jit.loadPtr(JIT::Address(result, JSWebAssemblyInstance::offsetOfImportFunction(importIndex)), result);
}

static MacroAssemblerCodeRef wasmToJs(VM* vm, Bag<CallLinkInfo>& callLinkInfos, SignatureIndex signatureIndex, unsigned importIndex)
{
    // FIXME: This function doesn't properly abstract away the calling convention.
    // It'd be super easy to do so: https://bugs.webkit.org/show_bug.cgi?id=169401
    const WasmCallingConvention& wasmCC = wasmCallingConvention();
    const JSCCallingConvention& jsCC = jscCallingConvention();
    const Signature* signature = SignatureInformation::get(vm, signatureIndex);
    unsigned argCount = signature->argumentCount();
    JIT jit(vm, nullptr);

    // Below, we assume that the JS calling convention is always on the stack.
    ASSERT(!jsCC.m_gprArgs.size());
    ASSERT(!jsCC.m_fprArgs.size());

    jit.emitFunctionPrologue();
    jit.store64(JIT::TrustedImm32(0), JIT::Address(GPRInfo::callFrameRegister, CallFrameSlot::codeBlock * static_cast<int>(sizeof(Register)))); // FIXME Stop using 0 as codeBlocks. https://bugs.webkit.org/show_bug.cgi?id=165321
    jit.storePtr(JIT::TrustedImmPtr(vm->webAssemblyToJSCallee.get()), JIT::Address(GPRInfo::callFrameRegister, CallFrameSlot::callee * static_cast<int>(sizeof(Register))));


    {
        bool hasBadI64Use = false;
        hasBadI64Use |= signature->returnType() == I64;
        for (unsigned argNum = 0; argNum < argCount && !hasBadI64Use; ++argNum) {
            Type argType = signature->argument(argNum);
            switch (argType) {
            case Void:
            case Func:
            case Anyfunc:
                RELEASE_ASSERT_NOT_REACHED();

            case I64: {
                hasBadI64Use = true;
                break;
            }

            default:
                break;
            }
        }

        if (hasBadI64Use) {
            jit.copyCalleeSavesToVMEntryFrameCalleeSavesBuffer();
            jit.move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
            auto call = jit.call();
            jit.jumpToExceptionHandler();

            void (*throwBadI64)(ExecState*) = [] (ExecState* exec) -> void {
                VM* vm = &exec->vm();
                NativeCallFrameTracer tracer(vm, exec);

                {
                    auto throwScope = DECLARE_THROW_SCOPE(*vm);
                    JSGlobalObject* globalObject = vm->topJSWebAssemblyInstance->globalObject();
                    auto* error = ErrorInstance::create(exec, *vm, globalObject->typeErrorConstructor()->errorStructure(), ASCIILiteral("i64 not allowed as return type or argument to an imported function"));
                    throwException(exec, throwScope, error);
                }

                genericUnwind(vm, exec);
                ASSERT(!!vm->callFrameForCatch);
            };

            LinkBuffer linkBuffer(*vm, jit, GLOBAL_THUNK_ID);
            linkBuffer.link(call, throwBadI64);
            return FINALIZE_CODE(linkBuffer, ("WebAssembly->JavaScript invalid i64 use in import[%i]", importIndex));
        }
    }

    // Here we assume that the JS calling convention saves at least all the wasm callee saved. We therefore don't need to save and restore more registers since the wasm callee already took care of this.
    RegisterSet missingCalleeSaves = wasmCC.m_calleeSaveRegisters;
    missingCalleeSaves.exclude(jsCC.m_calleeSaveRegisters);
    ASSERT(missingCalleeSaves.isEmpty());

    // FIXME perform a stack check before updating SP. https://bugs.webkit.org/show_bug.cgi?id=165546

    const unsigned numberOfParameters = argCount + 1; // There is a "this" argument.
    const unsigned numberOfRegsForCall = CallFrame::headerSizeInRegisters + numberOfParameters;
    const unsigned numberOfBytesForCall = numberOfRegsForCall * sizeof(Register) - sizeof(CallerFrameAndPC);
    const unsigned stackOffset = WTF::roundUpToMultipleOf(stackAlignmentBytes(), numberOfBytesForCall);
    jit.subPtr(MacroAssembler::TrustedImm32(stackOffset), MacroAssembler::stackPointerRegister);
    JIT::Address calleeFrame = CCallHelpers::Address(MacroAssembler::stackPointerRegister, -static_cast<ptrdiff_t>(sizeof(CallerFrameAndPC)));

    // FIXME make these loops which switch on Signature if there are many arguments on the stack. It'll otherwise be huge for huge signatures. https://bugs.webkit.org/show_bug.cgi?id=165547
    
    // First go through the integer parameters, freeing up their register for use afterwards.
    {
        unsigned marshalledGPRs = 0;
        unsigned marshalledFPRs = 0;
        unsigned calleeFrameOffset = CallFrameSlot::firstArgument * static_cast<int>(sizeof(Register));
        unsigned frOffset = CallFrame::headerSizeInRegisters * static_cast<int>(sizeof(Register));
        for (unsigned argNum = 0; argNum < argCount; ++argNum) {
            Type argType = signature->argument(argNum);
            switch (argType) {
            case Void:
            case Func:
            case Anyfunc:
            case I64:
                RELEASE_ASSERT_NOT_REACHED(); // Handled above.
            case I32: {
                GPRReg gprReg;
                if (marshalledGPRs < wasmCC.m_gprArgs.size())
                    gprReg = wasmCC.m_gprArgs[marshalledGPRs].gpr();
                else {
                    // We've already spilled all arguments, these registers are available as scratch.
                    gprReg = GPRInfo::argumentGPR0;
                    jit.load64(JIT::Address(GPRInfo::callFrameRegister, frOffset), gprReg);
                    frOffset += sizeof(Register);
                }
                ++marshalledGPRs;
                jit.zeroExtend32ToPtr(gprReg, gprReg); // Clear non-int32 and non-tag bits.
                jit.boxInt32(gprReg, JSValueRegs(gprReg), DoNotHaveTagRegisters);
                jit.store64(gprReg, calleeFrame.withOffset(calleeFrameOffset));
                calleeFrameOffset += sizeof(Register);
                break;
            }
            case F32:
            case F64:
                // Skipped: handled below.
                if (marshalledFPRs >= wasmCC.m_fprArgs.size())
                    frOffset += sizeof(Register);
                ++marshalledFPRs;
                calleeFrameOffset += sizeof(Register);
                break;
            }
        }
    }
    
    {
        // Integer registers have already been spilled, these are now available.
        GPRReg doubleEncodeOffsetGPRReg = GPRInfo::argumentGPR0;
        GPRReg scratch = GPRInfo::argumentGPR1;
        bool hasMaterializedDoubleEncodeOffset = false;
        auto materializeDoubleEncodeOffset = [&hasMaterializedDoubleEncodeOffset, &jit] (GPRReg dest) {
            if (!hasMaterializedDoubleEncodeOffset) {
                static_assert(DoubleEncodeOffset == 1ll << 48, "codegen assumes this below");
                jit.move(JIT::TrustedImm32(1), dest);
                jit.lshift64(JIT::TrustedImm32(48), dest);
                hasMaterializedDoubleEncodeOffset = true;
            }
        };

        unsigned marshalledGPRs = 0;
        unsigned marshalledFPRs = 0;
        unsigned calleeFrameOffset = CallFrameSlot::firstArgument * static_cast<int>(sizeof(Register));
        unsigned frOffset = CallFrame::headerSizeInRegisters * static_cast<int>(sizeof(Register));
        for (unsigned argNum = 0; argNum < argCount; ++argNum) {
            Type argType = signature->argument(argNum);
            switch (argType) {
            case Void:
            case Func:
            case Anyfunc:
            case I64:
                RELEASE_ASSERT_NOT_REACHED(); // Handled above.
            case I32:
                // Skipped: handled above.
                if (marshalledGPRs >= wasmCC.m_gprArgs.size())
                    frOffset += sizeof(Register);
                ++marshalledGPRs;
                calleeFrameOffset += sizeof(Register);
                break;
            case F32: {
                FPRReg fprReg;
                if (marshalledFPRs < wasmCC.m_fprArgs.size())
                    fprReg = wasmCC.m_fprArgs[marshalledFPRs].fpr();
                else {
                    // We've already spilled all arguments, these registers are available as scratch.
                    fprReg = FPRInfo::argumentFPR0;
                    jit.loadFloat(JIT::Address(GPRInfo::callFrameRegister, frOffset), fprReg);
                    frOffset += sizeof(Register);
                }
                jit.convertFloatToDouble(fprReg, fprReg);
                jit.purifyNaN(fprReg);
                jit.moveDoubleTo64(fprReg, scratch);
                materializeDoubleEncodeOffset(doubleEncodeOffsetGPRReg);
                jit.add64(doubleEncodeOffsetGPRReg, scratch);
                jit.store64(scratch, calleeFrame.withOffset(calleeFrameOffset));
                calleeFrameOffset += sizeof(Register);
                ++marshalledFPRs;
                break;
            }
            case F64: {
                FPRReg fprReg;
                if (marshalledFPRs < wasmCC.m_fprArgs.size())
                    fprReg = wasmCC.m_fprArgs[marshalledFPRs].fpr();
                else {
                    // We've already spilled all arguments, these registers are available as scratch.
                    fprReg = FPRInfo::argumentFPR0;
                    jit.loadDouble(JIT::Address(GPRInfo::callFrameRegister, frOffset), fprReg);
                    frOffset += sizeof(Register);
                }
                jit.purifyNaN(fprReg);
                jit.moveDoubleTo64(fprReg, scratch);
                materializeDoubleEncodeOffset(doubleEncodeOffsetGPRReg);
                jit.add64(doubleEncodeOffsetGPRReg, scratch);
                jit.store64(scratch, calleeFrame.withOffset(calleeFrameOffset));
                calleeFrameOffset += sizeof(Register);
                ++marshalledFPRs;
                break;
            }
            }
        }
    }

    GPRReg importJSCellGPRReg = GPRInfo::regT0; // Callee needs to be in regT0 for slow path below.
    ASSERT(!wasmCC.m_calleeSaveRegisters.get(importJSCellGPRReg));

    materializeImportJSCell(vm, jit, importIndex, importJSCellGPRReg);

    jit.store64(importJSCellGPRReg, calleeFrame.withOffset(CallFrameSlot::callee * static_cast<int>(sizeof(Register))));
    jit.store32(JIT::TrustedImm32(numberOfParameters), calleeFrame.withOffset(CallFrameSlot::argumentCount * static_cast<int>(sizeof(Register)) + PayloadOffset));
    jit.store64(JIT::TrustedImm64(ValueUndefined), calleeFrame.withOffset(CallFrameSlot::thisArgument * static_cast<int>(sizeof(Register))));

    // FIXME Tail call if the wasm return type is void and no registers were spilled. https://bugs.webkit.org/show_bug.cgi?id=165488

    CallLinkInfo* callLinkInfo = callLinkInfos.add();
    callLinkInfo->setUpCall(CallLinkInfo::Call, CodeOrigin(), importJSCellGPRReg);
    JIT::DataLabelPtr targetToCheck;
    JIT::TrustedImmPtr initialRightValue(0);
    JIT::Jump slowPath = jit.branchPtrWithPatch(MacroAssembler::NotEqual, importJSCellGPRReg, targetToCheck, initialRightValue);
    JIT::Call fastCall = jit.nearCall();
    JIT::Jump done = jit.jump();
    slowPath.link(&jit);
    // Callee needs to be in regT0 here.
    jit.move(MacroAssembler::TrustedImmPtr(callLinkInfo), GPRInfo::regT2); // Link info needs to be in regT2.
    JIT::Call slowCall = jit.nearCall();
    done.link(&jit);

    CCallHelpers::JumpList exceptionChecks;

    switch (signature->returnType()) {
    case Void:
        // Discard.
        break;
    case Func:
    case Anyfunc:
        // For the JavaScript embedding, imports with these types in their signature return are a WebAssembly.Module validation error.
        RELEASE_ASSERT_NOT_REACHED();
        break;
    case I64: {
        RELEASE_ASSERT_NOT_REACHED(); // Handled above.
    }
    case I32: {
        CCallHelpers::JumpList done;
        CCallHelpers::JumpList slowPath;

        slowPath.append(jit.branchIfNotNumber(GPRInfo::returnValueGPR, DoNotHaveTagRegisters));
        slowPath.append(jit.branchIfNotInt32(JSValueRegs(GPRInfo::returnValueGPR), DoNotHaveTagRegisters));
        jit.zeroExtend32ToPtr(GPRInfo::returnValueGPR, GPRInfo::returnValueGPR);
        done.append(jit.jump());

        slowPath.link(&jit);
        jit.setupArgumentsWithExecState(GPRInfo::returnValueGPR);
        auto call = jit.call();
        exceptionChecks.append(jit.emitJumpIfException());

        int32_t (*convertToI32)(ExecState*, JSValue) = [] (ExecState* exec, JSValue v) -> int32_t { 
            VM* vm = &exec->vm();
            NativeCallFrameTracer tracer(vm, exec);
            return v.toInt32(exec);
        };
        jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
            linkBuffer.link(call, convertToI32);
        });

        done.link(&jit);
        break;
    }
    case F32: {
        CCallHelpers::JumpList done;
        auto notANumber = jit.branchIfNotNumber(GPRInfo::returnValueGPR, DoNotHaveTagRegisters);
        auto isDouble = jit.branchIfNotInt32(JSValueRegs(GPRInfo::returnValueGPR), DoNotHaveTagRegisters);
        // We're an int32
        jit.signExtend32ToPtr(GPRInfo::returnValueGPR, GPRInfo::returnValueGPR);
        jit.convertInt64ToFloat(GPRInfo::returnValueGPR, FPRInfo::returnValueFPR);
        done.append(jit.jump());

        isDouble.link(&jit);
        jit.move(JIT::TrustedImm64(TagTypeNumber), GPRInfo::returnValueGPR2);
        jit.add64(GPRInfo::returnValueGPR2, GPRInfo::returnValueGPR);
        jit.move64ToDouble(GPRInfo::returnValueGPR, FPRInfo::returnValueFPR);
        jit.convertDoubleToFloat(FPRInfo::returnValueFPR, FPRInfo::returnValueFPR);
        done.append(jit.jump());

        notANumber.link(&jit);
        jit.setupArgumentsWithExecState(GPRInfo::returnValueGPR);
        auto call = jit.call();
        exceptionChecks.append(jit.emitJumpIfException());

        float (*convertToF32)(ExecState*, JSValue) = [] (ExecState* exec, JSValue v) -> float { 
            VM* vm = &exec->vm();
            NativeCallFrameTracer tracer(vm, exec);
            return static_cast<float>(v.toNumber(exec));
        };
        jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
            linkBuffer.link(call, convertToF32);
        });

        done.link(&jit);
        break;
    }
    case F64: {
        CCallHelpers::JumpList done;
        auto notANumber = jit.branchIfNotNumber(GPRInfo::returnValueGPR, DoNotHaveTagRegisters);
        auto isDouble = jit.branchIfNotInt32(JSValueRegs(GPRInfo::returnValueGPR), DoNotHaveTagRegisters);
        // We're an int32
        jit.signExtend32ToPtr(GPRInfo::returnValueGPR, GPRInfo::returnValueGPR);
        jit.convertInt64ToDouble(GPRInfo::returnValueGPR, FPRInfo::returnValueFPR);
        done.append(jit.jump());

        isDouble.link(&jit);
        jit.move(JIT::TrustedImm64(TagTypeNumber), GPRInfo::returnValueGPR2);
        jit.add64(GPRInfo::returnValueGPR2, GPRInfo::returnValueGPR);
        jit.move64ToDouble(GPRInfo::returnValueGPR, FPRInfo::returnValueFPR);
        done.append(jit.jump());

        notANumber.link(&jit);
        jit.setupArgumentsWithExecState(GPRInfo::returnValueGPR);
        auto call = jit.call();
        exceptionChecks.append(jit.emitJumpIfException());

        double (*convertToF64)(ExecState*, JSValue) = [] (ExecState* exec, JSValue v) -> double { 
            VM* vm = &exec->vm();
            NativeCallFrameTracer tracer(vm, exec);
            return v.toNumber(exec);
        };
        jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
            linkBuffer.link(call, convertToF64);
        });

        done.link(&jit);
        break;
    }
    }

    jit.emitFunctionEpilogue();
    jit.ret();

    if (!exceptionChecks.empty()) {
        exceptionChecks.link(&jit);
        jit.copyCalleeSavesToVMEntryFrameCalleeSavesBuffer();
        jit.move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
        auto call = jit.call();
        jit.jumpToExceptionHandler();

        void (*doUnwinding)(ExecState*) = [] (ExecState* exec) -> void {
            VM* vm = &exec->vm();
            NativeCallFrameTracer tracer(vm, exec);
            genericUnwind(vm, exec);
            ASSERT(!!vm->callFrameForCatch);
        };

        jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
            linkBuffer.link(call, doUnwinding);
        });
    }

    LinkBuffer patchBuffer(*vm, jit, GLOBAL_THUNK_ID);
    patchBuffer.link(slowCall, FunctionPtr(vm->getCTIStub(linkCallThunkGenerator).code().executableAddress()));
    CodeLocationLabel callReturnLocation(patchBuffer.locationOfNearCall(slowCall));
    CodeLocationLabel hotPathBegin(patchBuffer.locationOf(targetToCheck));
    CodeLocationNearCall hotPathOther = patchBuffer.locationOfNearCall(fastCall);
    callLinkInfo->setCallLocations(callReturnLocation, hotPathBegin, hotPathOther);
#if !defined(NDEBUG)
    String signatureDescription = SignatureInformation::get(vm, signatureIndex)->toString();
#else
    String signatureDescription;
#endif
    return FINALIZE_CODE(patchBuffer, ("WebAssembly->JavaScript import[%i] %s", importIndex, signatureDescription.ascii().data()));
}

static MacroAssemblerCodeRef wasmToWasm(VM* vm, unsigned importIndex)
{
    const PinnedRegisterInfo& pinnedRegs = PinnedRegisterInfo::get();
    JIT jit(vm, nullptr);

    GPRReg scratch = GPRInfo::nonPreservedNonArgumentGPR;

    // B3's call codegen ensures that the JSCell is a WebAssemblyFunction.
    materializeImportJSCell(vm, jit, importIndex, scratch);

    // Get the callee's WebAssembly.Instance and set it as vm.topJSWebAssemblyInstance. The caller will take care of restoring its own Instance.
    GPRReg baseMemory = pinnedRegs.baseMemoryPointer;
    ASSERT(baseMemory != scratch);
    jit.loadPtr(JIT::Address(scratch, WebAssemblyFunction::offsetOfInstance()), baseMemory); // Instance*.
    jit.storePtr(baseMemory, &vm->topJSWebAssemblyInstance);

    // FIXME the following code assumes that all WebAssembly.Instance have the same pinned registers. https://bugs.webkit.org/show_bug.cgi?id=162952
    // Set up the callee's baseMemory register as well as the memory size registers.
    jit.loadPtr(JIT::Address(baseMemory, JSWebAssemblyInstance::offsetOfMemory()), baseMemory); // JSWebAssemblyMemory*.
    const auto& sizeRegs = pinnedRegs.sizeRegisters;
    ASSERT(sizeRegs.size() >= 1);
    ASSERT(sizeRegs[0].sizeRegister != baseMemory);
    ASSERT(sizeRegs[0].sizeRegister != scratch);
    ASSERT(!sizeRegs[0].sizeOffset); // The following code assumes we start at 0, and calculates subsequent size registers relative to 0.
    jit.loadPtr(JIT::Address(baseMemory, JSWebAssemblyMemory::offsetOfSize()), sizeRegs[0].sizeRegister); // Memory size.
    jit.loadPtr(JIT::Address(baseMemory, JSWebAssemblyMemory::offsetOfMemory()), baseMemory); // WasmMemory::void*.
    for (unsigned i = 1; i < sizeRegs.size(); ++i) {
        ASSERT(sizeRegs[i].sizeRegister != baseMemory);
        ASSERT(sizeRegs[i].sizeRegister != scratch);
        jit.add64(JIT::TrustedImm32(-sizeRegs[i].sizeOffset), sizeRegs[0].sizeRegister, sizeRegs[i].sizeRegister);
    }

    // Tail call into the callee WebAssembly function.
    jit.loadPtr(JIT::Address(scratch, WebAssemblyFunction::offsetOfWasmEntryPointCode()), scratch);
    jit.jump(scratch);

    LinkBuffer patchBuffer(*vm, jit, GLOBAL_THUNK_ID);
    return FINALIZE_CODE(patchBuffer, ("WebAssembly->WebAssembly import[%i]", importIndex));
}

WasmExitStubs exitStubGenerator(VM* vm, Bag<CallLinkInfo>& callLinkInfos, SignatureIndex signatureIndex, unsigned importIndex)
{
    WasmExitStubs stubs;
    stubs.wasmToJs = wasmToJs(vm, callLinkInfos, signatureIndex, importIndex);
    stubs.wasmToWasm = wasmToWasm(vm, importIndex);
    return stubs;
}

} } // namespace JSC::Wasm

#endif // ENABLE(WEBASSEMBLY)
