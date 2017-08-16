/*
 * Copyright (C) 2010, 2012-2014, 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ThunkGenerators.h"

#include "CodeBlock.h"
#include "DFGSpeculativeJIT.h"
#include "JITExceptions.h"
#include "JITOperations.h"
#include "JSArray.h"
#include "JSBoundFunction.h"
#include "MathCommon.h"
#include "MaxFrameExtentForSlowPathCall.h"
#include "JSCInlines.h"
#include "JSWebAssemblyInstance.h"
#include "JSWebAssemblyRuntimeError.h"
#include "SpecializedThunkJIT.h"
#include "WasmExceptionType.h"
#include <wtf/InlineASM.h>
#include <wtf/StringPrintStream.h>
#include <wtf/text/StringImpl.h>

#if ENABLE(JIT)

namespace JSC {

inline void emitPointerValidation(CCallHelpers& jit, GPRReg pointerGPR)
{
    if (ASSERT_DISABLED)
        return;
    CCallHelpers::Jump isNonZero = jit.branchTestPtr(CCallHelpers::NonZero, pointerGPR);
    jit.abortWithReason(TGInvalidPointer);
    isNonZero.link(&jit);
    jit.pushToSave(pointerGPR);
    jit.load8(pointerGPR, pointerGPR);
    jit.popToRestore(pointerGPR);
}

// We will jump here if the JIT code tries to make a call, but the
// linking helper (C++ code) decides to throw an exception instead.
MacroAssemblerCodeRef throwExceptionFromCallSlowPathGenerator(VM* vm)
{
    CCallHelpers jit(vm);
    
    // The call pushed a return address, so we need to pop it back off to re-align the stack,
    // even though we won't use it.
    jit.preserveReturnAddressAfterCall(GPRInfo::nonPreservedNonReturnGPR);

    jit.copyCalleeSavesToVMEntryFrameCalleeSavesBuffer();

    jit.setupArguments(CCallHelpers::TrustedImmPtr(vm), GPRInfo::callFrameRegister);
    jit.move(CCallHelpers::TrustedImmPtr(bitwise_cast<void*>(lookupExceptionHandler)), GPRInfo::nonArgGPR0);
    emitPointerValidation(jit, GPRInfo::nonArgGPR0);
    jit.call(GPRInfo::nonArgGPR0);
    jit.jumpToExceptionHandler();

    LinkBuffer patchBuffer(*vm, jit, GLOBAL_THUNK_ID);
    return FINALIZE_CODE(patchBuffer, ("Throw exception from call slow path thunk"));
}

static void slowPathFor(
    CCallHelpers& jit, VM* vm, Sprt_JITOperation_ECli slowPathFunction)
{
    jit.emitFunctionPrologue();
    jit.storePtr(GPRInfo::callFrameRegister, &vm->topCallFrame);
#if OS(WINDOWS) && CPU(X86_64)
    // Windows X86_64 needs some space pointed to by arg0 for return types larger than 64 bits.
    // Other argument values are shift by 1. Use space on the stack for our two return values.
    // Moving the stack down maxFrameExtentForSlowPathCall bytes gives us room for our 3 arguments
    // and space for the 16 byte return area.
    jit.addPtr(CCallHelpers::TrustedImm32(-maxFrameExtentForSlowPathCall), CCallHelpers::stackPointerRegister);
    jit.move(GPRInfo::regT2, GPRInfo::argumentGPR2);
    jit.addPtr(CCallHelpers::TrustedImm32(32), CCallHelpers::stackPointerRegister, GPRInfo::argumentGPR0);
    jit.move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR1);
    jit.move(CCallHelpers::TrustedImmPtr(bitwise_cast<void*>(slowPathFunction)), GPRInfo::nonArgGPR0);
    emitPointerValidation(jit, GPRInfo::nonArgGPR0);
    jit.call(GPRInfo::nonArgGPR0);
    jit.loadPtr(CCallHelpers::Address(GPRInfo::returnValueGPR, 8), GPRInfo::returnValueGPR2);
    jit.loadPtr(CCallHelpers::Address(GPRInfo::returnValueGPR), GPRInfo::returnValueGPR);
    jit.addPtr(CCallHelpers::TrustedImm32(maxFrameExtentForSlowPathCall), CCallHelpers::stackPointerRegister);
#else
    if (maxFrameExtentForSlowPathCall)
        jit.addPtr(CCallHelpers::TrustedImm32(-maxFrameExtentForSlowPathCall), CCallHelpers::stackPointerRegister);
    jit.setupArgumentsWithExecState(GPRInfo::regT2);
    jit.move(CCallHelpers::TrustedImmPtr(bitwise_cast<void*>(slowPathFunction)), GPRInfo::nonArgGPR0);
    emitPointerValidation(jit, GPRInfo::nonArgGPR0);
    jit.call(GPRInfo::nonArgGPR0);
    if (maxFrameExtentForSlowPathCall)
        jit.addPtr(CCallHelpers::TrustedImm32(maxFrameExtentForSlowPathCall), CCallHelpers::stackPointerRegister);
#endif

    // This slow call will return the address of one of the following:
    // 1) Exception throwing thunk.
    // 2) Host call return value returner thingy.
    // 3) The function to call.
    // The second return value GPR will hold a non-zero value for tail calls.

    emitPointerValidation(jit, GPRInfo::returnValueGPR);
    jit.emitFunctionEpilogue();

    RELEASE_ASSERT(reinterpret_cast<void*>(KeepTheFrame) == reinterpret_cast<void*>(0));
    CCallHelpers::Jump doNotTrash = jit.branchTestPtr(CCallHelpers::Zero, GPRInfo::returnValueGPR2);

    jit.preserveReturnAddressAfterCall(GPRInfo::nonPreservedNonReturnGPR);
    jit.prepareForTailCallSlow(GPRInfo::returnValueGPR);

    doNotTrash.link(&jit);
    jit.jump(GPRInfo::returnValueGPR);
}

MacroAssemblerCodeRef linkCallThunkGenerator(VM* vm)
{
    // The return address is on the stack or in the link register. We will hence
    // save the return address to the call frame while we make a C++ function call
    // to perform linking and lazy compilation if necessary. We expect the callee
    // to be in regT0/regT1 (payload/tag), the CallFrame to have already
    // been adjusted, and all other registers to be available for use.
    CCallHelpers jit(vm);
    
    slowPathFor(jit, vm, operationLinkCall);
    
    LinkBuffer patchBuffer(*vm, jit, GLOBAL_THUNK_ID);
    return FINALIZE_CODE(patchBuffer, ("Link call slow path thunk"));
}

// For closure optimizations, we only include calls, since if you're using closures for
// object construction then you're going to lose big time anyway.
MacroAssemblerCodeRef linkPolymorphicCallThunkGenerator(VM* vm)
{
    CCallHelpers jit(vm);
    
    slowPathFor(jit, vm, operationLinkPolymorphicCall);
    
    LinkBuffer patchBuffer(*vm, jit, GLOBAL_THUNK_ID);
    return FINALIZE_CODE(patchBuffer, ("Link polymorphic call slow path thunk"));
}

// FIXME: We should distinguish between a megamorphic virtual call vs. a slow
// path virtual call so that we can enable fast tail calls for megamorphic
// virtual calls by using the shuffler.
// https://bugs.webkit.org/show_bug.cgi?id=148831
MacroAssemblerCodeRef virtualThunkFor(VM* vm, CallLinkInfo& callLinkInfo)
{
    // The callee is in regT0 (for JSVALUE32_64, the tag is in regT1).
    // The return address is on the stack, or in the link register. We will hence
    // jump to the callee, or save the return address to the call frame while we
    // make a C++ function call to the appropriate JIT operation.

    CCallHelpers jit(vm);
    
    CCallHelpers::JumpList slowCase;
    
    // This is a slow path execution, and regT2 contains the CallLinkInfo. Count the
    // slow path execution for the profiler.
    jit.add32(
        CCallHelpers::TrustedImm32(1),
        CCallHelpers::Address(GPRInfo::regT2, CallLinkInfo::offsetOfSlowPathCount()));

    // FIXME: we should have a story for eliminating these checks. In many cases,
    // the DFG knows that the value is definitely a cell, or definitely a function.
    
#if USE(JSVALUE64)
    slowCase.append(
        jit.branchTest64(
            CCallHelpers::NonZero, GPRInfo::regT0, GPRInfo::tagMaskRegister));
#else
    slowCase.append(
        jit.branch32(
            CCallHelpers::NotEqual, GPRInfo::regT1,
            CCallHelpers::TrustedImm32(JSValue::CellTag)));
#endif
    slowCase.append(jit.branchIfNotType(GPRInfo::regT0, JSFunctionType));
    
    // Now we know we have a JSFunction.
    
    jit.loadPtr(
        CCallHelpers::Address(GPRInfo::regT0, JSFunction::offsetOfExecutable()),
        GPRInfo::regT4);
    jit.loadPtr(
        CCallHelpers::Address(
            GPRInfo::regT4, ExecutableBase::offsetOfJITCodeWithArityCheckFor(
                callLinkInfo.specializationKind())),
        GPRInfo::regT4);
    slowCase.append(jit.branchTestPtr(CCallHelpers::Zero, GPRInfo::regT4));
    
    // Now we know that we have a CodeBlock, and we're committed to making a fast
    // call.
    
    // Make a tail call. This will return back to JIT code.
    emitPointerValidation(jit, GPRInfo::regT4);
    if (callLinkInfo.isTailCall()) {
        jit.preserveReturnAddressAfterCall(GPRInfo::regT0);
        jit.prepareForTailCallSlow(GPRInfo::regT4);
    }
    jit.jump(GPRInfo::regT4);

    slowCase.link(&jit);
    
    // Here we don't know anything, so revert to the full slow path.
    
    slowPathFor(jit, vm, operationVirtualCall);
    
    LinkBuffer patchBuffer(*vm, jit, GLOBAL_THUNK_ID);
    return FINALIZE_CODE(
        patchBuffer,
        ("Virtual %s slow path thunk",
        callLinkInfo.callMode() == CallMode::Regular ? "call" : callLinkInfo.callMode() == CallMode::Tail ? "tail call" : "construct"));
}

enum ThunkEntryType { EnterViaCall, EnterViaJumpWithSavedTags, EnterViaJumpWithoutSavedTags };

static MacroAssemblerCodeRef nativeForGenerator(VM* vm, CodeSpecializationKind kind, ThunkEntryType entryType = EnterViaCall)
{
    // FIXME: This should be able to log ShadowChicken prologue packets.
    // https://bugs.webkit.org/show_bug.cgi?id=155689
    
    int executableOffsetToFunction = NativeExecutable::offsetOfNativeFunctionFor(kind);
    
    JSInterfaceJIT jit(vm);

    switch (entryType) {
    case EnterViaCall:
        jit.emitFunctionPrologue();
        break;
    case EnterViaJumpWithSavedTags:
#if USE(JSVALUE64)
        // We're coming from a specialized thunk that has saved the prior tag registers' contents.
        // Restore them now.
#if CPU(ARM64)
        jit.popPair(JSInterfaceJIT::tagTypeNumberRegister, JSInterfaceJIT::tagMaskRegister);
#else
        jit.pop(JSInterfaceJIT::tagMaskRegister);
        jit.pop(JSInterfaceJIT::tagTypeNumberRegister);
#endif
#endif
        break;
    case EnterViaJumpWithoutSavedTags:
        jit.move(JSInterfaceJIT::framePointerRegister, JSInterfaceJIT::stackPointerRegister);
        break;
    }

    jit.emitPutToCallFrameHeader(0, CallFrameSlot::codeBlock);
    jit.storePtr(JSInterfaceJIT::callFrameRegister, &vm->topCallFrame);

#if CPU(X86)
    // Calling convention:      f(ecx, edx, ...);
    // Host function signature: f(ExecState*);
    jit.move(JSInterfaceJIT::callFrameRegister, X86Registers::ecx);

    jit.subPtr(JSInterfaceJIT::TrustedImm32(8), JSInterfaceJIT::stackPointerRegister); // Align stack after prologue.

    // call the function
    jit.emitGetFromCallFrameHeaderPtr(CallFrameSlot::callee, JSInterfaceJIT::regT1);
    jit.loadPtr(JSInterfaceJIT::Address(JSInterfaceJIT::regT1, JSFunction::offsetOfExecutable()), JSInterfaceJIT::regT1);
    jit.call(JSInterfaceJIT::Address(JSInterfaceJIT::regT1, executableOffsetToFunction));

    jit.addPtr(JSInterfaceJIT::TrustedImm32(8), JSInterfaceJIT::stackPointerRegister);

#elif CPU(X86_64)
#if !OS(WINDOWS)
    // Calling convention:      f(edi, esi, edx, ecx, ...);
    // Host function signature: f(ExecState*);
    jit.move(JSInterfaceJIT::callFrameRegister, X86Registers::edi);

    jit.emitGetFromCallFrameHeaderPtr(CallFrameSlot::callee, X86Registers::esi);
    jit.loadPtr(JSInterfaceJIT::Address(X86Registers::esi, JSFunction::offsetOfExecutable()), X86Registers::r9);
    jit.call(JSInterfaceJIT::Address(X86Registers::r9, executableOffsetToFunction));

#else
    // Calling convention:      f(ecx, edx, r8, r9, ...);
    // Host function signature: f(ExecState*);
    jit.move(JSInterfaceJIT::callFrameRegister, X86Registers::ecx);

    // Leave space for the callee parameter home addresses.
    // At this point the stack is aligned to 16 bytes, but if this changes at some point, we need to emit code to align it.
    jit.subPtr(JSInterfaceJIT::TrustedImm32(4 * sizeof(int64_t)), JSInterfaceJIT::stackPointerRegister);

    jit.emitGetFromCallFrameHeaderPtr(CallFrameSlot::callee, X86Registers::edx);
    jit.loadPtr(JSInterfaceJIT::Address(X86Registers::edx, JSFunction::offsetOfExecutable()), X86Registers::r9);
    jit.call(JSInterfaceJIT::Address(X86Registers::r9, executableOffsetToFunction));

    jit.addPtr(JSInterfaceJIT::TrustedImm32(4 * sizeof(int64_t)), JSInterfaceJIT::stackPointerRegister);
#endif

#elif CPU(ARM64)
    COMPILE_ASSERT(ARM64Registers::x0 != JSInterfaceJIT::regT3, T3_not_trampled_by_arg_0);
    COMPILE_ASSERT(ARM64Registers::x1 != JSInterfaceJIT::regT3, T3_not_trampled_by_arg_1);
    COMPILE_ASSERT(ARM64Registers::x2 != JSInterfaceJIT::regT3, T3_not_trampled_by_arg_2);

    // Host function signature: f(ExecState*);
    jit.move(JSInterfaceJIT::callFrameRegister, ARM64Registers::x0);

    jit.emitGetFromCallFrameHeaderPtr(CallFrameSlot::callee, ARM64Registers::x1);
    jit.loadPtr(JSInterfaceJIT::Address(ARM64Registers::x1, JSFunction::offsetOfExecutable()), ARM64Registers::x2);
    jit.call(JSInterfaceJIT::Address(ARM64Registers::x2, executableOffsetToFunction));
#elif CPU(ARM) || CPU(MIPS)
#if CPU(MIPS)
    // Allocate stack space for (unused) 16 bytes (8-byte aligned) for 4 arguments.
    jit.subPtr(JSInterfaceJIT::TrustedImm32(16), JSInterfaceJIT::stackPointerRegister);
#endif

    // Calling convention is f(argumentGPR0, argumentGPR1, ...).
    // Host function signature is f(ExecState*).
    jit.move(JSInterfaceJIT::callFrameRegister, JSInterfaceJIT::argumentGPR0);

    jit.emitGetFromCallFrameHeaderPtr(CallFrameSlot::callee, JSInterfaceJIT::argumentGPR1);
    jit.loadPtr(JSInterfaceJIT::Address(JSInterfaceJIT::argumentGPR1, JSFunction::offsetOfExecutable()), JSInterfaceJIT::regT2);
    jit.call(JSInterfaceJIT::Address(JSInterfaceJIT::regT2, executableOffsetToFunction));

#if CPU(MIPS)
    // Restore stack space
    jit.addPtr(JSInterfaceJIT::TrustedImm32(16), JSInterfaceJIT::stackPointerRegister);
#endif
#else
#error "JIT not supported on this platform."
    UNUSED_PARAM(executableOffsetToFunction);
    abortWithReason(TGNotSupported);
#endif

    // Check for an exception
#if USE(JSVALUE64)
    jit.load64(vm->addressOfException(), JSInterfaceJIT::regT2);
    JSInterfaceJIT::Jump exceptionHandler = jit.branchTest64(JSInterfaceJIT::NonZero, JSInterfaceJIT::regT2);
#else
    JSInterfaceJIT::Jump exceptionHandler = jit.branch32(
        JSInterfaceJIT::NotEqual,
        JSInterfaceJIT::AbsoluteAddress(vm->addressOfException()),
        JSInterfaceJIT::TrustedImm32(0));
#endif

    jit.emitFunctionEpilogue();
    // Return.
    jit.ret();

    // Handle an exception
    exceptionHandler.link(&jit);

    jit.copyCalleeSavesToVMEntryFrameCalleeSavesBuffer();
    jit.storePtr(JSInterfaceJIT::callFrameRegister, &vm->topCallFrame);

#if CPU(X86) && USE(JSVALUE32_64)
    jit.addPtr(JSInterfaceJIT::TrustedImm32(-12), JSInterfaceJIT::stackPointerRegister);
    jit.move(JSInterfaceJIT::callFrameRegister, JSInterfaceJIT::regT0);
    jit.push(JSInterfaceJIT::regT0);
#else
#if OS(WINDOWS)
    // Allocate space on stack for the 4 parameter registers.
    jit.subPtr(JSInterfaceJIT::TrustedImm32(4 * sizeof(int64_t)), JSInterfaceJIT::stackPointerRegister);
#endif
    jit.move(JSInterfaceJIT::callFrameRegister, JSInterfaceJIT::argumentGPR0);
#endif
    jit.move(JSInterfaceJIT::TrustedImmPtr(FunctionPtr(operationVMHandleException).value()), JSInterfaceJIT::regT3);
    jit.call(JSInterfaceJIT::regT3);
#if CPU(X86) && USE(JSVALUE32_64)
    jit.addPtr(JSInterfaceJIT::TrustedImm32(16), JSInterfaceJIT::stackPointerRegister);
#elif OS(WINDOWS)
    jit.addPtr(JSInterfaceJIT::TrustedImm32(4 * sizeof(int64_t)), JSInterfaceJIT::stackPointerRegister);
#endif

    jit.jumpToExceptionHandler();

    LinkBuffer patchBuffer(*vm, jit, GLOBAL_THUNK_ID);
    return FINALIZE_CODE(patchBuffer, ("native %s%s trampoline", entryType == EnterViaJumpWithSavedTags ? "Tail With Saved Tags " : entryType == EnterViaJumpWithoutSavedTags ? "Tail Without Saved Tags " : "", toCString(kind).data()));
}

MacroAssemblerCodeRef nativeCallGenerator(VM* vm)
{
    return nativeForGenerator(vm, CodeForCall);
}

MacroAssemblerCodeRef nativeTailCallGenerator(VM* vm)
{
    return nativeForGenerator(vm, CodeForCall, EnterViaJumpWithSavedTags);
}

MacroAssemblerCodeRef nativeTailCallWithoutSavedTagsGenerator(VM* vm)
{
    return nativeForGenerator(vm, CodeForCall, EnterViaJumpWithoutSavedTags);
}

MacroAssemblerCodeRef nativeConstructGenerator(VM* vm)
{
    return nativeForGenerator(vm, CodeForConstruct);
}

MacroAssemblerCodeRef arityFixupGenerator(VM* vm)
{
    JSInterfaceJIT jit(vm);

    // We enter with fixup count in argumentGPR0
    // We have the guarantee that a0, a1, a2, t3, t4 and t5 (or t0 for Windows) are all distinct :-)
#if USE(JSVALUE64)
#if OS(WINDOWS)
    const GPRReg extraTemp = JSInterfaceJIT::regT0;
#else
    const GPRReg extraTemp = JSInterfaceJIT::regT5;
#endif
#  if CPU(X86_64)
    jit.pop(JSInterfaceJIT::regT4);
#  endif
    jit.move(JSInterfaceJIT::callFrameRegister, JSInterfaceJIT::regT3);
    jit.load32(JSInterfaceJIT::Address(JSInterfaceJIT::callFrameRegister, CallFrameSlot::argumentCount * sizeof(Register)), JSInterfaceJIT::argumentGPR2);
    jit.add32(JSInterfaceJIT::TrustedImm32(CallFrame::headerSizeInRegisters), JSInterfaceJIT::argumentGPR2);

    // Check to see if we have extra slots we can use
    jit.move(JSInterfaceJIT::argumentGPR0, JSInterfaceJIT::argumentGPR1);
    jit.and32(JSInterfaceJIT::TrustedImm32(stackAlignmentRegisters() - 1), JSInterfaceJIT::argumentGPR1);
    JSInterfaceJIT::Jump noExtraSlot = jit.branchTest32(MacroAssembler::Zero, JSInterfaceJIT::argumentGPR1);
    jit.move(JSInterfaceJIT::TrustedImm64(ValueUndefined), extraTemp);
    JSInterfaceJIT::Label fillExtraSlots(jit.label());
    jit.store64(extraTemp, MacroAssembler::BaseIndex(JSInterfaceJIT::callFrameRegister, JSInterfaceJIT::argumentGPR2, JSInterfaceJIT::TimesEight));
    jit.add32(JSInterfaceJIT::TrustedImm32(1), JSInterfaceJIT::argumentGPR2);
    jit.branchSub32(JSInterfaceJIT::NonZero, JSInterfaceJIT::TrustedImm32(1), JSInterfaceJIT::argumentGPR1).linkTo(fillExtraSlots, &jit);
    jit.and32(JSInterfaceJIT::TrustedImm32(-stackAlignmentRegisters()), JSInterfaceJIT::argumentGPR0);
    JSInterfaceJIT::Jump done = jit.branchTest32(MacroAssembler::Zero, JSInterfaceJIT::argumentGPR0);
    noExtraSlot.link(&jit);

    jit.neg64(JSInterfaceJIT::argumentGPR0);

    // Adjust call frame register and stack pointer to account for missing args.
    // We need to change the stack pointer first before performing copy/fill loops.
    // This stack space below the stack pointer is considered unsed by OS. Therefore,
    // OS may corrupt this space when constructing a signal stack.
    jit.move(JSInterfaceJIT::argumentGPR0, extraTemp);
    jit.lshift64(JSInterfaceJIT::TrustedImm32(3), extraTemp);
    jit.addPtr(extraTemp, JSInterfaceJIT::callFrameRegister);
    jit.addPtr(extraTemp, JSInterfaceJIT::stackPointerRegister);

    // Move current frame down argumentGPR0 number of slots
    JSInterfaceJIT::Label copyLoop(jit.label());
    jit.load64(JSInterfaceJIT::regT3, extraTemp);
    jit.store64(extraTemp, MacroAssembler::BaseIndex(JSInterfaceJIT::regT3, JSInterfaceJIT::argumentGPR0, JSInterfaceJIT::TimesEight));
    jit.addPtr(JSInterfaceJIT::TrustedImm32(8), JSInterfaceJIT::regT3);
    jit.branchSub32(MacroAssembler::NonZero, JSInterfaceJIT::TrustedImm32(1), JSInterfaceJIT::argumentGPR2).linkTo(copyLoop, &jit);

    // Fill in argumentGPR0 missing arg slots with undefined
    jit.move(JSInterfaceJIT::argumentGPR0, JSInterfaceJIT::argumentGPR2);
    jit.move(JSInterfaceJIT::TrustedImm64(ValueUndefined), extraTemp);
    JSInterfaceJIT::Label fillUndefinedLoop(jit.label());
    jit.store64(extraTemp, MacroAssembler::BaseIndex(JSInterfaceJIT::regT3, JSInterfaceJIT::argumentGPR0, JSInterfaceJIT::TimesEight));
    jit.addPtr(JSInterfaceJIT::TrustedImm32(8), JSInterfaceJIT::regT3);
    jit.branchAdd32(MacroAssembler::NonZero, JSInterfaceJIT::TrustedImm32(1), JSInterfaceJIT::argumentGPR2).linkTo(fillUndefinedLoop, &jit);
    
    done.link(&jit);

#  if CPU(X86_64)
    jit.push(JSInterfaceJIT::regT4);
#  endif
    jit.ret();
#else
#  if CPU(X86)
    jit.pop(JSInterfaceJIT::regT4);
#  endif
    jit.move(JSInterfaceJIT::callFrameRegister, JSInterfaceJIT::regT3);
    jit.load32(JSInterfaceJIT::Address(JSInterfaceJIT::callFrameRegister, CallFrameSlot::argumentCount * sizeof(Register)), JSInterfaceJIT::argumentGPR2);
    jit.add32(JSInterfaceJIT::TrustedImm32(CallFrame::headerSizeInRegisters), JSInterfaceJIT::argumentGPR2);

    // Check to see if we have extra slots we can use
    jit.move(JSInterfaceJIT::argumentGPR0, JSInterfaceJIT::argumentGPR1);
    jit.and32(JSInterfaceJIT::TrustedImm32(stackAlignmentRegisters() - 1), JSInterfaceJIT::argumentGPR1);
    JSInterfaceJIT::Jump noExtraSlot = jit.branchTest32(MacroAssembler::Zero, JSInterfaceJIT::argumentGPR1);
    JSInterfaceJIT::Label fillExtraSlots(jit.label());
    jit.move(JSInterfaceJIT::TrustedImm32(0), JSInterfaceJIT::regT5);
    jit.store32(JSInterfaceJIT::regT5, MacroAssembler::BaseIndex(JSInterfaceJIT::callFrameRegister, JSInterfaceJIT::argumentGPR2, JSInterfaceJIT::TimesEight, PayloadOffset));
    jit.move(JSInterfaceJIT::TrustedImm32(JSValue::UndefinedTag), JSInterfaceJIT::regT5);
    jit.store32(JSInterfaceJIT::regT5, MacroAssembler::BaseIndex(JSInterfaceJIT::callFrameRegister, JSInterfaceJIT::argumentGPR2, JSInterfaceJIT::TimesEight, TagOffset));
    jit.add32(JSInterfaceJIT::TrustedImm32(1), JSInterfaceJIT::argumentGPR2);
    jit.branchSub32(JSInterfaceJIT::NonZero, JSInterfaceJIT::TrustedImm32(1), JSInterfaceJIT::argumentGPR1).linkTo(fillExtraSlots, &jit);
    jit.and32(JSInterfaceJIT::TrustedImm32(-stackAlignmentRegisters()), JSInterfaceJIT::argumentGPR0);
    JSInterfaceJIT::Jump done = jit.branchTest32(MacroAssembler::Zero, JSInterfaceJIT::argumentGPR0);
    noExtraSlot.link(&jit);

    jit.neg32(JSInterfaceJIT::argumentGPR0);

    // Move current frame down argumentGPR0 number of slots
    JSInterfaceJIT::Label copyLoop(jit.label());
    jit.load32(MacroAssembler::Address(JSInterfaceJIT::regT3, PayloadOffset), JSInterfaceJIT::regT5);
    jit.store32(JSInterfaceJIT::regT5, MacroAssembler::BaseIndex(JSInterfaceJIT::regT3, JSInterfaceJIT::argumentGPR0, JSInterfaceJIT::TimesEight, PayloadOffset));
    jit.load32(MacroAssembler::Address(JSInterfaceJIT::regT3, TagOffset), JSInterfaceJIT::regT5);
    jit.store32(JSInterfaceJIT::regT5, MacroAssembler::BaseIndex(JSInterfaceJIT::regT3, JSInterfaceJIT::argumentGPR0, JSInterfaceJIT::TimesEight, TagOffset));
    jit.addPtr(JSInterfaceJIT::TrustedImm32(8), JSInterfaceJIT::regT3);
    jit.branchSub32(MacroAssembler::NonZero, JSInterfaceJIT::TrustedImm32(1), JSInterfaceJIT::argumentGPR2).linkTo(copyLoop, &jit);

    // Fill in argumentGPR0 missing arg slots with undefined
    jit.move(JSInterfaceJIT::argumentGPR0, JSInterfaceJIT::argumentGPR2);
    JSInterfaceJIT::Label fillUndefinedLoop(jit.label());
    jit.move(JSInterfaceJIT::TrustedImm32(0), JSInterfaceJIT::regT5);
    jit.store32(JSInterfaceJIT::regT5, MacroAssembler::BaseIndex(JSInterfaceJIT::regT3, JSInterfaceJIT::argumentGPR0, JSInterfaceJIT::TimesEight, PayloadOffset));
    jit.move(JSInterfaceJIT::TrustedImm32(JSValue::UndefinedTag), JSInterfaceJIT::regT5);
    jit.store32(JSInterfaceJIT::regT5, MacroAssembler::BaseIndex(JSInterfaceJIT::regT3, JSInterfaceJIT::argumentGPR0, JSInterfaceJIT::TimesEight, TagOffset));

    jit.addPtr(JSInterfaceJIT::TrustedImm32(8), JSInterfaceJIT::regT3);
    jit.branchAdd32(MacroAssembler::NonZero, JSInterfaceJIT::TrustedImm32(1), JSInterfaceJIT::argumentGPR2).linkTo(fillUndefinedLoop, &jit);

    // Adjust call frame register and stack pointer to account for missing args
    jit.move(JSInterfaceJIT::argumentGPR0, JSInterfaceJIT::regT5);
    jit.lshift32(JSInterfaceJIT::TrustedImm32(3), JSInterfaceJIT::regT5);
    jit.addPtr(JSInterfaceJIT::regT5, JSInterfaceJIT::callFrameRegister);
    jit.addPtr(JSInterfaceJIT::regT5, JSInterfaceJIT::stackPointerRegister);

    done.link(&jit);

#  if CPU(X86)
    jit.push(JSInterfaceJIT::regT4);
#  endif
    jit.ret();
#endif

    LinkBuffer patchBuffer(*vm, jit, GLOBAL_THUNK_ID);
    return FINALIZE_CODE(patchBuffer, ("fixup arity"));
}

MacroAssemblerCodeRef unreachableGenerator(VM* vm)
{
    JSInterfaceJIT jit(vm);

    jit.breakpoint();

    LinkBuffer patchBuffer(*vm, jit, GLOBAL_THUNK_ID);
    return FINALIZE_CODE(patchBuffer, ("unreachable thunk"));
}

static void stringCharLoad(SpecializedThunkJIT& jit, VM* vm)
{
    // load string
    jit.loadJSStringArgument(*vm, SpecializedThunkJIT::ThisArgument, SpecializedThunkJIT::regT0);

    // Load string length to regT2, and start the process of loading the data pointer into regT0
    jit.load32(MacroAssembler::Address(SpecializedThunkJIT::regT0, ThunkHelpers::jsStringLengthOffset()), SpecializedThunkJIT::regT2);
    jit.loadPtr(MacroAssembler::Address(SpecializedThunkJIT::regT0, ThunkHelpers::jsStringValueOffset()), SpecializedThunkJIT::regT0);
    jit.appendFailure(jit.branchTest32(MacroAssembler::Zero, SpecializedThunkJIT::regT0));

    // load index
    jit.loadInt32Argument(0, SpecializedThunkJIT::regT1); // regT1 contains the index

    // Do an unsigned compare to simultaneously filter negative indices as well as indices that are too large
    jit.appendFailure(jit.branch32(MacroAssembler::AboveOrEqual, SpecializedThunkJIT::regT1, SpecializedThunkJIT::regT2));

    // Load the character
    SpecializedThunkJIT::JumpList is16Bit;
    SpecializedThunkJIT::JumpList cont8Bit;
    // Load the string flags
    jit.loadPtr(MacroAssembler::Address(SpecializedThunkJIT::regT0, StringImpl::flagsOffset()), SpecializedThunkJIT::regT2);
    jit.loadPtr(MacroAssembler::Address(SpecializedThunkJIT::regT0, StringImpl::dataOffset()), SpecializedThunkJIT::regT0);
    is16Bit.append(jit.branchTest32(MacroAssembler::Zero, SpecializedThunkJIT::regT2, MacroAssembler::TrustedImm32(StringImpl::flagIs8Bit())));
    jit.load8(MacroAssembler::BaseIndex(SpecializedThunkJIT::regT0, SpecializedThunkJIT::regT1, MacroAssembler::TimesOne, 0), SpecializedThunkJIT::regT0);
    cont8Bit.append(jit.jump());
    is16Bit.link(&jit);
    jit.load16(MacroAssembler::BaseIndex(SpecializedThunkJIT::regT0, SpecializedThunkJIT::regT1, MacroAssembler::TimesTwo, 0), SpecializedThunkJIT::regT0);
    cont8Bit.link(&jit);
}

static void charToString(SpecializedThunkJIT& jit, VM* vm, MacroAssembler::RegisterID src, MacroAssembler::RegisterID dst, MacroAssembler::RegisterID scratch)
{
    jit.appendFailure(jit.branch32(MacroAssembler::AboveOrEqual, src, MacroAssembler::TrustedImm32(0x100)));
    jit.move(MacroAssembler::TrustedImmPtr(vm->smallStrings.singleCharacterStrings()), scratch);
    jit.loadPtr(MacroAssembler::BaseIndex(scratch, src, MacroAssembler::ScalePtr, 0), dst);
    jit.appendFailure(jit.branchTestPtr(MacroAssembler::Zero, dst));
}

MacroAssemblerCodeRef charCodeAtThunkGenerator(VM* vm)
{
    SpecializedThunkJIT jit(vm, 1);
    stringCharLoad(jit, vm);
    jit.returnInt32(SpecializedThunkJIT::regT0);
    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "charCodeAt");
}

MacroAssemblerCodeRef charAtThunkGenerator(VM* vm)
{
    SpecializedThunkJIT jit(vm, 1);
    stringCharLoad(jit, vm);
    charToString(jit, vm, SpecializedThunkJIT::regT0, SpecializedThunkJIT::regT0, SpecializedThunkJIT::regT1);
    jit.returnJSCell(SpecializedThunkJIT::regT0);
    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "charAt");
}

MacroAssemblerCodeRef fromCharCodeThunkGenerator(VM* vm)
{
    SpecializedThunkJIT jit(vm, 1);
    // load char code
    jit.loadInt32Argument(0, SpecializedThunkJIT::regT0);
    charToString(jit, vm, SpecializedThunkJIT::regT0, SpecializedThunkJIT::regT0, SpecializedThunkJIT::regT1);
    jit.returnJSCell(SpecializedThunkJIT::regT0);
    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "fromCharCode");
}

MacroAssemblerCodeRef clz32ThunkGenerator(VM* vm)
{
    SpecializedThunkJIT jit(vm, 1);
    MacroAssembler::Jump nonIntArgJump;
    jit.loadInt32Argument(0, SpecializedThunkJIT::regT0, nonIntArgJump);

    SpecializedThunkJIT::Label convertedArgumentReentry(&jit);
    jit.countLeadingZeros32(SpecializedThunkJIT::regT0, SpecializedThunkJIT::regT1);
    jit.returnInt32(SpecializedThunkJIT::regT1);

    if (jit.supportsFloatingPointTruncate()) {
        nonIntArgJump.link(&jit);
        jit.loadDoubleArgument(0, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0);
        jit.branchTruncateDoubleToInt32(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0, SpecializedThunkJIT::BranchIfTruncateSuccessful).linkTo(convertedArgumentReentry, &jit);
        jit.appendFailure(jit.jump());
    } else
        jit.appendFailure(nonIntArgJump);

    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "clz32");
}

MacroAssemblerCodeRef sqrtThunkGenerator(VM* vm)
{
    SpecializedThunkJIT jit(vm, 1);
    if (!jit.supportsFloatingPointSqrt())
        return MacroAssemblerCodeRef::createSelfManagedCodeRef(vm->jitStubs->ctiNativeCall(vm));

    jit.loadDoubleArgument(0, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0);
    jit.sqrtDouble(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::fpRegT0);
    jit.returnDouble(SpecializedThunkJIT::fpRegT0);
    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "sqrt");
}


#define UnaryDoubleOpWrapper(function) function##Wrapper
enum MathThunkCallingConvention { };
typedef MathThunkCallingConvention(*MathThunk)(MathThunkCallingConvention);

#if CPU(X86_64) && COMPILER(GCC_OR_CLANG) && (OS(DARWIN) || OS(LINUX))

#define defineUnaryDoubleOpWrapper(function) \
    asm( \
        ".text\n" \
        ".globl " SYMBOL_STRING(function##Thunk) "\n" \
        HIDE_SYMBOL(function##Thunk) "\n" \
        SYMBOL_STRING(function##Thunk) ":" "\n" \
        "pushq %rax\n" \
        "call " GLOBAL_REFERENCE(function) "\n" \
        "popq %rcx\n" \
        "ret\n" \
    );\
    extern "C" { \
        MathThunkCallingConvention function##Thunk(MathThunkCallingConvention); \
    } \
    static MathThunk UnaryDoubleOpWrapper(function) = &function##Thunk;

#elif CPU(X86) && COMPILER(GCC_OR_CLANG) && OS(LINUX) && defined(__PIC__)
#define defineUnaryDoubleOpWrapper(function) \
    asm( \
        ".text\n" \
        ".globl " SYMBOL_STRING(function##Thunk) "\n" \
        HIDE_SYMBOL(function##Thunk) "\n" \
        SYMBOL_STRING(function##Thunk) ":" "\n" \
        "pushl %ebx\n" \
        "subl $20, %esp\n" \
        "movsd %xmm0, (%esp) \n" \
        "call __x86.get_pc_thunk.bx\n" \
        "addl $_GLOBAL_OFFSET_TABLE_, %ebx\n" \
        "call " GLOBAL_REFERENCE(function) "\n" \
        "fstpl (%esp) \n" \
        "movsd (%esp), %xmm0 \n" \
        "addl $20, %esp\n" \
        "popl %ebx\n" \
        "ret\n" \
    );\
    extern "C" { \
        MathThunkCallingConvention function##Thunk(MathThunkCallingConvention); \
    } \
    static MathThunk UnaryDoubleOpWrapper(function) = &function##Thunk;

#elif CPU(X86) && COMPILER(GCC_OR_CLANG) && (OS(DARWIN) || OS(LINUX))
#define defineUnaryDoubleOpWrapper(function) \
    asm( \
        ".text\n" \
        ".globl " SYMBOL_STRING(function##Thunk) "\n" \
        HIDE_SYMBOL(function##Thunk) "\n" \
        SYMBOL_STRING(function##Thunk) ":" "\n" \
        "subl $20, %esp\n" \
        "movsd %xmm0, (%esp) \n" \
        "call " GLOBAL_REFERENCE(function) "\n" \
        "fstpl (%esp) \n" \
        "movsd (%esp), %xmm0 \n" \
        "addl $20, %esp\n" \
        "ret\n" \
    );\
    extern "C" { \
        MathThunkCallingConvention function##Thunk(MathThunkCallingConvention); \
    } \
    static MathThunk UnaryDoubleOpWrapper(function) = &function##Thunk;

#elif CPU(ARM_THUMB2) && COMPILER(GCC_OR_CLANG) && PLATFORM(IOS)

#define defineUnaryDoubleOpWrapper(function) \
    asm( \
        ".text\n" \
        ".align 2\n" \
        ".globl " SYMBOL_STRING(function##Thunk) "\n" \
        HIDE_SYMBOL(function##Thunk) "\n" \
        ".thumb\n" \
        ".thumb_func " THUMB_FUNC_PARAM(function##Thunk) "\n" \
        SYMBOL_STRING(function##Thunk) ":" "\n" \
        "push {lr}\n" \
        "vmov r0, r1, d0\n" \
        "blx " GLOBAL_REFERENCE(function) "\n" \
        "vmov d0, r0, r1\n" \
        "pop {lr}\n" \
        "bx lr\n" \
    ); \
    extern "C" { \
        MathThunkCallingConvention function##Thunk(MathThunkCallingConvention); \
    } \
    static MathThunk UnaryDoubleOpWrapper(function) = &function##Thunk;

#elif CPU(ARM64)

#define defineUnaryDoubleOpWrapper(function) \
    asm( \
        ".text\n" \
        ".align 2\n" \
        ".globl " SYMBOL_STRING(function##Thunk) "\n" \
        HIDE_SYMBOL(function##Thunk) "\n" \
        SYMBOL_STRING(function##Thunk) ":" "\n" \
        "b " GLOBAL_REFERENCE(function) "\n" \
        ".previous" \
    ); \
    extern "C" { \
        MathThunkCallingConvention function##Thunk(MathThunkCallingConvention); \
    } \
    static MathThunk UnaryDoubleOpWrapper(function) = &function##Thunk;

#elif CPU(X86) && COMPILER(MSVC) && OS(WINDOWS)

// MSVC does not accept floor, etc, to be called directly from inline assembly, so we need to wrap these functions.
static double (_cdecl *floorFunction)(double) = floor;
static double (_cdecl *ceilFunction)(double) = ceil;
static double (_cdecl *truncFunction)(double) = trunc;
static double (_cdecl *expFunction)(double) = exp;
static double (_cdecl *logFunction)(double) = log;
static double (_cdecl *jsRoundFunction)(double) = jsRound;

#define defineUnaryDoubleOpWrapper(function) \
    extern "C" __declspec(naked) MathThunkCallingConvention function##Thunk(MathThunkCallingConvention) \
    { \
        __asm \
        { \
        __asm sub esp, 20 \
        __asm movsd mmword ptr [esp], xmm0  \
        __asm call function##Function \
        __asm fstp qword ptr [esp] \
        __asm movsd xmm0, mmword ptr [esp] \
        __asm add esp, 20 \
        __asm ret \
        } \
    } \
    static MathThunk UnaryDoubleOpWrapper(function) = &function##Thunk;

#else

#define defineUnaryDoubleOpWrapper(function) \
    static MathThunk UnaryDoubleOpWrapper(function) = 0
#endif

defineUnaryDoubleOpWrapper(jsRound);
defineUnaryDoubleOpWrapper(exp);
defineUnaryDoubleOpWrapper(log);
defineUnaryDoubleOpWrapper(floor);
defineUnaryDoubleOpWrapper(ceil);
defineUnaryDoubleOpWrapper(trunc);

static const double halfConstant = 0.5;
    
MacroAssemblerCodeRef floorThunkGenerator(VM* vm)
{
    SpecializedThunkJIT jit(vm, 1);
    MacroAssembler::Jump nonIntJump;
    if (!UnaryDoubleOpWrapper(floor) || !jit.supportsFloatingPoint())
        return MacroAssemblerCodeRef::createSelfManagedCodeRef(vm->jitStubs->ctiNativeCall(vm));
    jit.loadInt32Argument(0, SpecializedThunkJIT::regT0, nonIntJump);
    jit.returnInt32(SpecializedThunkJIT::regT0);
    nonIntJump.link(&jit);
    jit.loadDoubleArgument(0, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0);

    if (jit.supportsFloatingPointRounding()) {
        SpecializedThunkJIT::JumpList doubleResult;
        jit.floorDouble(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::fpRegT0);
        jit.branchConvertDoubleToInt32(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0, doubleResult, SpecializedThunkJIT::fpRegT1);
        jit.returnInt32(SpecializedThunkJIT::regT0);
        doubleResult.link(&jit);
        jit.returnDouble(SpecializedThunkJIT::fpRegT0);
        return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "floor");
    }

    SpecializedThunkJIT::Jump intResult;
    SpecializedThunkJIT::JumpList doubleResult;
    if (jit.supportsFloatingPointTruncate()) {
        jit.moveZeroToDouble(SpecializedThunkJIT::fpRegT1);
        doubleResult.append(jit.branchDouble(MacroAssembler::DoubleEqual, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::fpRegT1));
        SpecializedThunkJIT::JumpList slowPath;
        // Handle the negative doubles in the slow path for now.
        slowPath.append(jit.branchDouble(MacroAssembler::DoubleLessThanOrUnordered, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::fpRegT1));
        slowPath.append(jit.branchTruncateDoubleToInt32(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0));
        intResult = jit.jump();
        slowPath.link(&jit);
    }
    jit.callDoubleToDoublePreservingReturn(UnaryDoubleOpWrapper(floor));
    jit.branchConvertDoubleToInt32(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0, doubleResult, SpecializedThunkJIT::fpRegT1);
    if (jit.supportsFloatingPointTruncate())
        intResult.link(&jit);
    jit.returnInt32(SpecializedThunkJIT::regT0);
    doubleResult.link(&jit);
    jit.returnDouble(SpecializedThunkJIT::fpRegT0);
    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "floor");
}

MacroAssemblerCodeRef ceilThunkGenerator(VM* vm)
{
    SpecializedThunkJIT jit(vm, 1);
    if (!UnaryDoubleOpWrapper(ceil) || !jit.supportsFloatingPoint())
        return MacroAssemblerCodeRef::createSelfManagedCodeRef(vm->jitStubs->ctiNativeCall(vm));
    MacroAssembler::Jump nonIntJump;
    jit.loadInt32Argument(0, SpecializedThunkJIT::regT0, nonIntJump);
    jit.returnInt32(SpecializedThunkJIT::regT0);
    nonIntJump.link(&jit);
    jit.loadDoubleArgument(0, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0);
    if (jit.supportsFloatingPointRounding())
        jit.ceilDouble(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::fpRegT0);
    else
        jit.callDoubleToDoublePreservingReturn(UnaryDoubleOpWrapper(ceil));

    SpecializedThunkJIT::JumpList doubleResult;
    jit.branchConvertDoubleToInt32(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0, doubleResult, SpecializedThunkJIT::fpRegT1);
    jit.returnInt32(SpecializedThunkJIT::regT0);
    doubleResult.link(&jit);
    jit.returnDouble(SpecializedThunkJIT::fpRegT0);
    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "ceil");
}

MacroAssemblerCodeRef truncThunkGenerator(VM* vm)
{
    SpecializedThunkJIT jit(vm, 1);
    if (!UnaryDoubleOpWrapper(trunc) || !jit.supportsFloatingPoint())
        return MacroAssemblerCodeRef::createSelfManagedCodeRef(vm->jitStubs->ctiNativeCall(vm));
    MacroAssembler::Jump nonIntJump;
    jit.loadInt32Argument(0, SpecializedThunkJIT::regT0, nonIntJump);
    jit.returnInt32(SpecializedThunkJIT::regT0);
    nonIntJump.link(&jit);
    jit.loadDoubleArgument(0, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0);
    if (jit.supportsFloatingPointRounding())
        jit.roundTowardZeroDouble(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::fpRegT0);
    else
        jit.callDoubleToDoublePreservingReturn(UnaryDoubleOpWrapper(trunc));

    SpecializedThunkJIT::JumpList doubleResult;
    jit.branchConvertDoubleToInt32(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0, doubleResult, SpecializedThunkJIT::fpRegT1);
    jit.returnInt32(SpecializedThunkJIT::regT0);
    doubleResult.link(&jit);
    jit.returnDouble(SpecializedThunkJIT::fpRegT0);
    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "trunc");
}

MacroAssemblerCodeRef roundThunkGenerator(VM* vm)
{
    SpecializedThunkJIT jit(vm, 1);
    if (!UnaryDoubleOpWrapper(jsRound) || !jit.supportsFloatingPoint())
        return MacroAssemblerCodeRef::createSelfManagedCodeRef(vm->jitStubs->ctiNativeCall(vm));
    MacroAssembler::Jump nonIntJump;
    jit.loadInt32Argument(0, SpecializedThunkJIT::regT0, nonIntJump);
    jit.returnInt32(SpecializedThunkJIT::regT0);
    nonIntJump.link(&jit);
    jit.loadDoubleArgument(0, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0);
    SpecializedThunkJIT::Jump intResult;
    SpecializedThunkJIT::JumpList doubleResult;
    if (jit.supportsFloatingPointTruncate()) {
        jit.moveZeroToDouble(SpecializedThunkJIT::fpRegT1);
        doubleResult.append(jit.branchDouble(MacroAssembler::DoubleEqual, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::fpRegT1));
        SpecializedThunkJIT::JumpList slowPath;
        // Handle the negative doubles in the slow path for now.
        slowPath.append(jit.branchDouble(MacroAssembler::DoubleLessThanOrUnordered, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::fpRegT1));
        jit.loadDouble(MacroAssembler::TrustedImmPtr(&halfConstant), SpecializedThunkJIT::fpRegT1);
        jit.addDouble(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::fpRegT1);
        slowPath.append(jit.branchTruncateDoubleToInt32(SpecializedThunkJIT::fpRegT1, SpecializedThunkJIT::regT0));
        intResult = jit.jump();
        slowPath.link(&jit);
    }
    jit.callDoubleToDoublePreservingReturn(UnaryDoubleOpWrapper(jsRound));
    jit.branchConvertDoubleToInt32(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0, doubleResult, SpecializedThunkJIT::fpRegT1);
    if (jit.supportsFloatingPointTruncate())
        intResult.link(&jit);
    jit.returnInt32(SpecializedThunkJIT::regT0);
    doubleResult.link(&jit);
    jit.returnDouble(SpecializedThunkJIT::fpRegT0);
    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "round");
}

MacroAssemblerCodeRef expThunkGenerator(VM* vm)
{
    if (!UnaryDoubleOpWrapper(exp))
        return MacroAssemblerCodeRef::createSelfManagedCodeRef(vm->jitStubs->ctiNativeCall(vm));
    SpecializedThunkJIT jit(vm, 1);
    if (!jit.supportsFloatingPoint())
        return MacroAssemblerCodeRef::createSelfManagedCodeRef(vm->jitStubs->ctiNativeCall(vm));
    jit.loadDoubleArgument(0, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0);
    jit.callDoubleToDoublePreservingReturn(UnaryDoubleOpWrapper(exp));
    jit.returnDouble(SpecializedThunkJIT::fpRegT0);
    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "exp");
}

MacroAssemblerCodeRef logThunkGenerator(VM* vm)
{
    if (!UnaryDoubleOpWrapper(log))
        return MacroAssemblerCodeRef::createSelfManagedCodeRef(vm->jitStubs->ctiNativeCall(vm));
    SpecializedThunkJIT jit(vm, 1);
    if (!jit.supportsFloatingPoint())
        return MacroAssemblerCodeRef::createSelfManagedCodeRef(vm->jitStubs->ctiNativeCall(vm));
    jit.loadDoubleArgument(0, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0);
    jit.callDoubleToDoublePreservingReturn(UnaryDoubleOpWrapper(log));
    jit.returnDouble(SpecializedThunkJIT::fpRegT0);
    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "log");
}

MacroAssemblerCodeRef absThunkGenerator(VM* vm)
{
    SpecializedThunkJIT jit(vm, 1);
    if (!jit.supportsFloatingPointAbs())
        return MacroAssemblerCodeRef::createSelfManagedCodeRef(vm->jitStubs->ctiNativeCall(vm));

#if USE(JSVALUE64)
    unsigned virtualRegisterIndex = CallFrame::argumentOffset(0);
    jit.load64(AssemblyHelpers::addressFor(virtualRegisterIndex), GPRInfo::regT0);
    MacroAssembler::Jump notInteger = jit.branch64(MacroAssembler::Below, GPRInfo::regT0, GPRInfo::tagTypeNumberRegister);

    // Abs Int32.
    jit.rshift32(GPRInfo::regT0, MacroAssembler::TrustedImm32(31), GPRInfo::regT1);
    jit.add32(GPRInfo::regT1, GPRInfo::regT0);
    jit.xor32(GPRInfo::regT1, GPRInfo::regT0);

    // IntMin cannot be inverted.
    MacroAssembler::Jump integerIsIntMin = jit.branchTest32(MacroAssembler::Signed, GPRInfo::regT0);

    // Box and finish.
    jit.or64(GPRInfo::tagTypeNumberRegister, GPRInfo::regT0);
    MacroAssembler::Jump doneWithIntegers = jit.jump();

    // Handle Doubles.
    notInteger.link(&jit);
    jit.appendFailure(jit.branchTest64(MacroAssembler::Zero, GPRInfo::regT0, GPRInfo::tagTypeNumberRegister));
    jit.unboxDoubleWithoutAssertions(GPRInfo::regT0, GPRInfo::regT0, FPRInfo::fpRegT0);
    MacroAssembler::Label absFPR0Label = jit.label();
    jit.absDouble(FPRInfo::fpRegT0, FPRInfo::fpRegT1);
    jit.boxDouble(FPRInfo::fpRegT1, GPRInfo::regT0);

    // Tail.
    doneWithIntegers.link(&jit);
    jit.returnJSValue(GPRInfo::regT0);

    // We know the value of regT0 is IntMin. We could load that value from memory but
    // it is simpler to just convert it.
    integerIsIntMin.link(&jit);
    jit.convertInt32ToDouble(GPRInfo::regT0, FPRInfo::fpRegT0);
    jit.jump().linkTo(absFPR0Label, &jit);
#else
    MacroAssembler::Jump nonIntJump;
    jit.loadInt32Argument(0, SpecializedThunkJIT::regT0, nonIntJump);
    jit.rshift32(SpecializedThunkJIT::regT0, MacroAssembler::TrustedImm32(31), SpecializedThunkJIT::regT1);
    jit.add32(SpecializedThunkJIT::regT1, SpecializedThunkJIT::regT0);
    jit.xor32(SpecializedThunkJIT::regT1, SpecializedThunkJIT::regT0);
    jit.appendFailure(jit.branchTest32(MacroAssembler::Signed, SpecializedThunkJIT::regT0));
    jit.returnInt32(SpecializedThunkJIT::regT0);
    nonIntJump.link(&jit);
    // Shame about the double int conversion here.
    jit.loadDoubleArgument(0, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0);
    jit.absDouble(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::fpRegT1);
    jit.returnDouble(SpecializedThunkJIT::fpRegT1);
#endif
    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "abs");
}

MacroAssemblerCodeRef imulThunkGenerator(VM* vm)
{
    SpecializedThunkJIT jit(vm, 2);
    MacroAssembler::Jump nonIntArg0Jump;
    jit.loadInt32Argument(0, SpecializedThunkJIT::regT0, nonIntArg0Jump);
    SpecializedThunkJIT::Label doneLoadingArg0(&jit);
    MacroAssembler::Jump nonIntArg1Jump;
    jit.loadInt32Argument(1, SpecializedThunkJIT::regT1, nonIntArg1Jump);
    SpecializedThunkJIT::Label doneLoadingArg1(&jit);
    jit.mul32(SpecializedThunkJIT::regT1, SpecializedThunkJIT::regT0);
    jit.returnInt32(SpecializedThunkJIT::regT0);

    if (jit.supportsFloatingPointTruncate()) {
        nonIntArg0Jump.link(&jit);
        jit.loadDoubleArgument(0, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0);
        jit.branchTruncateDoubleToInt32(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT0, SpecializedThunkJIT::BranchIfTruncateSuccessful).linkTo(doneLoadingArg0, &jit);
        jit.appendFailure(jit.jump());
    } else
        jit.appendFailure(nonIntArg0Jump);

    if (jit.supportsFloatingPointTruncate()) {
        nonIntArg1Jump.link(&jit);
        jit.loadDoubleArgument(1, SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT1);
        jit.branchTruncateDoubleToInt32(SpecializedThunkJIT::fpRegT0, SpecializedThunkJIT::regT1, SpecializedThunkJIT::BranchIfTruncateSuccessful).linkTo(doneLoadingArg1, &jit);
        jit.appendFailure(jit.jump());
    } else
        jit.appendFailure(nonIntArg1Jump);

    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "imul");
}

MacroAssemblerCodeRef randomThunkGenerator(VM* vm)
{
    SpecializedThunkJIT jit(vm, 0);
    if (!jit.supportsFloatingPoint())
        return MacroAssemblerCodeRef::createSelfManagedCodeRef(vm->jitStubs->ctiNativeCall(vm));

#if USE(JSVALUE64)
    jit.emitRandomThunk(SpecializedThunkJIT::regT0, SpecializedThunkJIT::regT1, SpecializedThunkJIT::regT2, SpecializedThunkJIT::regT3, SpecializedThunkJIT::fpRegT0);
    jit.returnDouble(SpecializedThunkJIT::fpRegT0);

    return jit.finalize(vm->jitStubs->ctiNativeTailCall(vm), "random");
#else
    return MacroAssemblerCodeRef::createSelfManagedCodeRef(vm->jitStubs->ctiNativeCall(vm));
#endif
}

MacroAssemblerCodeRef boundThisNoArgsFunctionCallGenerator(VM* vm)
{
    CCallHelpers jit(vm);
    
    jit.emitFunctionPrologue();
    
    // Set up our call frame.
    jit.storePtr(CCallHelpers::TrustedImmPtr(nullptr), CCallHelpers::addressFor(CallFrameSlot::codeBlock));
    jit.store32(CCallHelpers::TrustedImm32(0), CCallHelpers::tagFor(CallFrameSlot::argumentCount));

    unsigned extraStackNeeded = 0;
    if (unsigned stackMisalignment = sizeof(CallerFrameAndPC) % stackAlignmentBytes())
        extraStackNeeded = stackAlignmentBytes() - stackMisalignment;
    
    // We need to forward all of the arguments that we were passed. We aren't allowed to do a tail
    // call here as far as I can tell. At least not so long as the generic path doesn't do a tail
    // call, since that would be way too weird.
    
    // The formula for the number of stack bytes needed given some number of parameters (including
    // this) is:
    //
    //     stackAlign((numParams + CallFrameHeaderSize) * sizeof(Register) - sizeof(CallerFrameAndPC))
    //
    // Probably we want to write this as:
    //
    //     stackAlign((numParams + (CallFrameHeaderSize - CallerFrameAndPCSize)) * sizeof(Register))
    //
    // That's really all there is to this. We have all the registers we need to do it.
    
    jit.load32(CCallHelpers::payloadFor(CallFrameSlot::argumentCount), GPRInfo::regT1);
    jit.add32(CCallHelpers::TrustedImm32(CallFrame::headerSizeInRegisters - CallerFrameAndPC::sizeInRegisters), GPRInfo::regT1, GPRInfo::regT2);
    jit.lshift32(CCallHelpers::TrustedImm32(3), GPRInfo::regT2);
    jit.add32(CCallHelpers::TrustedImm32(stackAlignmentBytes() - 1), GPRInfo::regT2);
    jit.and32(CCallHelpers::TrustedImm32(-stackAlignmentBytes()), GPRInfo::regT2);
    
    if (extraStackNeeded)
        jit.add32(CCallHelpers::TrustedImm32(extraStackNeeded), GPRInfo::regT2);
    
    // At this point regT1 has the actual argument count and regT2 has the amount of stack we will
    // need.
    
    jit.subPtr(GPRInfo::regT2, CCallHelpers::stackPointerRegister);

    // Do basic callee frame setup, including 'this'.
    
    jit.loadCell(CCallHelpers::addressFor(CallFrameSlot::callee), GPRInfo::regT3);

    jit.store32(GPRInfo::regT1, CCallHelpers::calleeFramePayloadSlot(CallFrameSlot::argumentCount));
    
    JSValueRegs valueRegs = JSValueRegs::withTwoAvailableRegs(GPRInfo::regT0, GPRInfo::regT2);
    jit.loadValue(CCallHelpers::Address(GPRInfo::regT3, JSBoundFunction::offsetOfBoundThis()), valueRegs);
    jit.storeValue(valueRegs, CCallHelpers::calleeArgumentSlot(0));

    jit.loadPtr(CCallHelpers::Address(GPRInfo::regT3, JSBoundFunction::offsetOfTargetFunction()), GPRInfo::regT3);
    jit.storeCell(GPRInfo::regT3, CCallHelpers::calleeFrameSlot(CallFrameSlot::callee));
    
    // OK, now we can start copying. This is a simple matter of copying parameters from the caller's
    // frame to the callee's frame. Note that we know that regT1 (the argument count) must be at
    // least 1.
    jit.sub32(CCallHelpers::TrustedImm32(1), GPRInfo::regT1);
    CCallHelpers::Jump done = jit.branchTest32(CCallHelpers::Zero, GPRInfo::regT1);
    
    CCallHelpers::Label loop = jit.label();
    jit.sub32(CCallHelpers::TrustedImm32(1), GPRInfo::regT1);
    jit.loadValue(CCallHelpers::addressFor(virtualRegisterForArgument(1)).indexedBy(GPRInfo::regT1, CCallHelpers::TimesEight), valueRegs);
    jit.storeValue(valueRegs, CCallHelpers::calleeArgumentSlot(1).indexedBy(GPRInfo::regT1, CCallHelpers::TimesEight));
    jit.branchTest32(CCallHelpers::NonZero, GPRInfo::regT1).linkTo(loop, &jit);
    
    done.link(&jit);
    
    jit.loadPtr(
        CCallHelpers::Address(GPRInfo::regT3, JSFunction::offsetOfExecutable()),
        GPRInfo::regT0);
    jit.loadPtr(
        CCallHelpers::Address(
            GPRInfo::regT0, ExecutableBase::offsetOfJITCodeWithArityCheckFor(CodeForCall)),
        GPRInfo::regT0);
    CCallHelpers::Jump noCode = jit.branchTestPtr(CCallHelpers::Zero, GPRInfo::regT0);
    
    emitPointerValidation(jit, GPRInfo::regT0);
    jit.call(GPRInfo::regT0);
    
    jit.emitFunctionEpilogue();
    jit.ret();
    
    LinkBuffer linkBuffer(*vm, jit, GLOBAL_THUNK_ID);
    linkBuffer.link(noCode, CodeLocationLabel(vm->jitStubs->ctiNativeTailCallWithoutSavedTags(vm)));
    return FINALIZE_CODE(
        linkBuffer, ("Specialized thunk for bound function calls with no arguments"));
}

#if ENABLE(WEBASSEMBLY)
MacroAssemblerCodeRef throwExceptionFromWasmThunkGenerator(VM* vm)
{
    CCallHelpers jit(vm);

    // The thing that jumps here must move ExceptionType into the argumentGPR1 and jump here.
    // We're allowed to use temp registers here, but not callee saves.
    {
        RegisterSet usedRegisters = RegisterSet::stubUnavailableRegisters();
        usedRegisters.set(GPRInfo::argumentGPR1);
        jit.copyCalleeSavesToVMEntryFrameCalleeSavesBuffer(usedRegisters);
    }

    jit.move(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    CCallHelpers::Call call = jit.call();
    jit.jumpToExceptionHandler();

    void (*throwWasmException)(ExecState*, Wasm::ExceptionType) = [] (ExecState* exec, Wasm::ExceptionType type) {
        VM* vm = &exec->vm();
        NativeCallFrameTracer tracer(vm, exec);

        {
            auto throwScope = DECLARE_THROW_SCOPE(*vm);
            JSGlobalObject* globalObject = vm->topJSWebAssemblyInstance->globalObject();

            JSWebAssemblyRuntimeError* error = JSWebAssemblyRuntimeError::create(exec, *vm, globalObject->WebAssemblyRuntimeErrorStructure(), Wasm::errorMessageForExceptionType(type));
            throwException(exec, throwScope, error);
        }

        genericUnwind(vm, exec);
        ASSERT(!!vm->callFrameForCatch);
    };

    LinkBuffer linkBuffer(*vm, jit, GLOBAL_THUNK_ID);
    linkBuffer.link(call, throwWasmException);
    return FINALIZE_CODE(
        linkBuffer, ("Throw exception from Wasm"));
}
#endif // ENABLE(WEBASSEMBLY)

} // namespace JSC

#endif // ENABLE(JIT)
