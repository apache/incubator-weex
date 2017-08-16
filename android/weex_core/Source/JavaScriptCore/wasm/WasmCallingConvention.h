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

#pragma once

#if ENABLE(WEBASSEMBLY)

#include "AllowMacroScratchRegisterUsage.h"
#include "B3ArgumentRegValue.h"
#include "B3BasicBlock.h"
#include "B3Const64Value.h"
#include "B3ConstrainedValue.h"
#include "B3MemoryValue.h"
#include "B3PatchpointValue.h"
#include "B3Procedure.h"
#include "B3StackmapGenerationParams.h"
#include "CallFrame.h"
#include "LinkBuffer.h"
#include "RegisterSet.h"
#include "WasmFormat.h"
#include "WasmSignature.h"

namespace JSC { namespace Wasm {

typedef unsigned (*NextOffset)(unsigned currentOffset, B3::Type type);

template<unsigned headerSize, NextOffset updateOffset>
class CallingConvention {
public:
    CallingConvention(Vector<Reg>&& gprArgs, Vector<Reg>&& fprArgs, RegisterSet&& calleeSaveRegisters)
        : m_gprArgs(gprArgs)
        , m_fprArgs(fprArgs)
        , m_calleeSaveRegisters(calleeSaveRegisters)
    {
    }

private:
    B3::ValueRep marshallArgumentImpl(Vector<Reg> regArgs, B3::Type type, size_t& count, size_t& stackOffset) const
    {
        if (count < regArgs.size())
            return B3::ValueRep::reg(regArgs[count++]);

        count++;
        B3::ValueRep result = B3::ValueRep::stackArgument(stackOffset);
        stackOffset = updateOffset(stackOffset, type);
        return result;
    }

    B3::ValueRep marshallArgument(B3::Type type, size_t& gpArgumentCount, size_t& fpArgumentCount, size_t& stackOffset) const
    {
        switch (type) {
        case B3::Int32:
        case B3::Int64:
            return marshallArgumentImpl(m_gprArgs, type, gpArgumentCount, stackOffset);
        case B3::Float:
        case B3::Double:
            return marshallArgumentImpl(m_fprArgs, type, fpArgumentCount, stackOffset);
        case B3::Void:
            break;
        }
        RELEASE_ASSERT_NOT_REACHED();
    }

public:
    static unsigned headerSizeInBytes() { return headerSize; }
    void setupFrameInPrologue(CodeLocationDataLabelPtr* calleeMoveLocation, B3::Procedure& proc, B3::Origin origin, B3::BasicBlock* block) const
    {
        static_assert(CallFrameSlot::callee * sizeof(Register) < headerSize, "We rely on this here for now.");
        static_assert(CallFrameSlot::codeBlock * sizeof(Register) < headerSize, "We rely on this here for now.");

        B3::PatchpointValue* getCalleePatchpoint = block->appendNew<B3::PatchpointValue>(proc, B3::Int64, origin);
        getCalleePatchpoint->resultConstraint = B3::ValueRep::SomeRegister;
        getCalleePatchpoint->effects = B3::Effects::none();
        getCalleePatchpoint->setGenerator(
            [=] (CCallHelpers& jit, const B3::StackmapGenerationParams& params) {
                GPRReg result = params[0].gpr();
                MacroAssembler::DataLabelPtr moveLocation = jit.moveWithPatch(MacroAssembler::TrustedImmPtr(nullptr), result);
                jit.addLinkTask([calleeMoveLocation, moveLocation] (LinkBuffer& linkBuffer) {
                    *calleeMoveLocation = linkBuffer.locationOf(moveLocation);
                });
            });

        B3::Value* framePointer = block->appendNew<B3::Value>(proc, B3::FramePointer, origin);
        B3::Value* offsetOfCallee = block->appendNew<B3::Const64Value>(proc, origin, CallFrameSlot::callee * sizeof(Register));
        block->appendNew<B3::MemoryValue>(proc, B3::Store, origin,
            getCalleePatchpoint,
            block->appendNew<B3::Value>(proc, B3::Add, origin, framePointer, offsetOfCallee));

        // FIXME: We shouldn't have to store zero into the CodeBlock* spot in the call frame,
        // but there are places that interpret non-null CodeBlock slot to mean a valid CodeBlock.
        // When doing unwinding, we'll need to verify that the entire runtime is OK with a non-null
        // CodeBlock not implying that the CodeBlock is valid.
        // https://bugs.webkit.org/show_bug.cgi?id=165321
        B3::Value* offsetOfCodeBlock = block->appendNew<B3::Const64Value>(proc, origin, CallFrameSlot::codeBlock * sizeof(Register));
        block->appendNew<B3::MemoryValue>(proc, B3::Store, origin,
            block->appendNew<B3::Const64Value>(proc, origin, 0),
            block->appendNew<B3::Value>(proc, B3::Add, origin, framePointer, offsetOfCodeBlock));
    }

    template<typename Functor>
    void loadArguments(const Signature* signature, B3::Procedure& proc, B3::BasicBlock* block, B3::Origin origin, const Functor& functor) const
    {
        B3::Value* framePointer = block->appendNew<B3::Value>(proc, B3::FramePointer, origin);

        size_t gpArgumentCount = 0;
        size_t fpArgumentCount = 0;
        size_t stackOffset = headerSize;

        for (size_t i = 0; i < signature->argumentCount(); ++i) {
            B3::Type type = toB3Type(signature->argument(i));
            B3::Value* argument;
            B3::ValueRep rep = marshallArgument(type, gpArgumentCount, fpArgumentCount, stackOffset);
            if (rep.isReg()) {
                argument = block->appendNew<B3::ArgumentRegValue>(proc, origin, rep.reg());
                if (type == B3::Int32 || type == B3::Float)
                    argument = block->appendNew<B3::Value>(proc, B3::Trunc, origin, argument);
            } else {
                ASSERT(rep.isStackArgument());
                B3::Value* address = block->appendNew<B3::Value>(proc, B3::Add, origin, framePointer,
                    block->appendNew<B3::Const64Value>(proc, origin, rep.offsetFromSP()));
                argument = block->appendNew<B3::MemoryValue>(proc, B3::Load, type, origin, address);
            }
            functor(argument, i);
        }
    }

    // It's expected that the pachpointFunctor sets the generator for the call operation.
    template<typename Functor>
    B3::Value* setupCall(B3::Procedure& proc, B3::BasicBlock* block, B3::Origin origin, const Vector<B3::Value*>& arguments, B3::Type returnType, const Functor& patchpointFunctor) const
    {
        size_t gpArgumentCount = 0;
        size_t fpArgumentCount = 0;
        size_t stackOffset = headerSize - sizeof(CallerFrameAndPC);

        Vector<B3::ConstrainedValue> constrainedArguments;
        for (B3::Value* argument : arguments) {
            B3::ValueRep rep = marshallArgument(argument->type(), gpArgumentCount, fpArgumentCount, stackOffset);
            constrainedArguments.append(B3::ConstrainedValue(argument, rep));
        }

        proc.requestCallArgAreaSizeInBytes(WTF::roundUpToMultipleOf(stackAlignmentBytes(), stackOffset));

        B3::PatchpointValue* patchpoint = block->appendNew<B3::PatchpointValue>(proc, returnType, origin);
        patchpoint->clobberEarly(RegisterSet::macroScratchRegisters());
        patchpoint->clobberLate(RegisterSet::volatileRegistersForJSCall());
        patchpointFunctor(patchpoint);
        patchpoint->appendVector(constrainedArguments);

        switch (returnType) {
        case B3::Void:
            return nullptr;
        case B3::Float:
        case B3::Double:
            patchpoint->resultConstraint = B3::ValueRep::reg(FPRInfo::returnValueFPR);
            break;
        case B3::Int32:
        case B3::Int64:
            patchpoint->resultConstraint = B3::ValueRep::reg(GPRInfo::returnValueGPR);
            break;
        }
        return patchpoint;
    }

    const Vector<Reg> m_gprArgs;
    const Vector<Reg> m_fprArgs;
    const RegisterSet m_calleeSaveRegisters;
    const RegisterSet m_callerSaveRegisters;
};

inline unsigned nextJSCOffset(unsigned currentOffset, B3::Type)
{
    return currentOffset + sizeof(Register);
}

constexpr unsigned jscHeaderSize = ExecState::headerSizeInRegisters * sizeof(Register);
typedef CallingConvention<jscHeaderSize, nextJSCOffset> JSCCallingConvention;

typedef JSCCallingConvention WasmCallingConvention;

const JSCCallingConvention& jscCallingConvention();
const WasmCallingConvention& wasmCallingConvention();

} } // namespace JSC::Wasm

#endif // ENABLE(WEBASSEMBLY)
