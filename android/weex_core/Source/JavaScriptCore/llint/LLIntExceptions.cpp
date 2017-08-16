/*
 * Copyright (C) 2011, 2016 Apple Inc. All rights reserved.
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
#include "LLIntExceptions.h"
#include "CallFrame.h"
#include "CodeBlock.h"
#include "Instruction.h"
#include "LLIntCommon.h"
#include "LLIntData.h"
#include "LowLevelInterpreter.h"
#include "JSCInlines.h"

#if LLINT_SLOW_PATH_TRACING
#include "Exception.h"
#endif

namespace JSC { namespace LLInt {

Instruction* returnToThrowForThrownException(ExecState* exec)
{
    UNUSED_PARAM(exec);
    return LLInt::exceptionInstructions();
}

Instruction* returnToThrow(ExecState* exec)
{
    UNUSED_PARAM(exec);
#if LLINT_SLOW_PATH_TRACING
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);
    dataLog("Throwing exception ", JSValue(scope.exception()), " (returnToThrow).\n");
#endif
    return LLInt::exceptionInstructions();
}

void* callToThrow(ExecState* exec)
{
    UNUSED_PARAM(exec);
#if LLINT_SLOW_PATH_TRACING
    VM* vm = &exec->vm();
    auto scope = DECLARE_THROW_SCOPE(*vm);
    dataLog("Throwing exception ", JSValue(scope.exception()), " (callToThrow).\n");
#endif
    return LLInt::getCodePtr(llint_throw_during_call_trampoline);
}

} } // namespace JSC::LLInt
