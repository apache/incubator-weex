/*
 * Copyright (C) 2013, 2014 Apple Inc. All rights reserved.
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
#include "DFGOSRExitPreparation.h"

#if ENABLE(DFG_JIT)

#include "CodeBlock.h"
#include "JIT.h"
#include "JITCode.h"
#include "JITWorklist.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

void prepareCodeOriginForOSRExit(ExecState* exec, CodeOrigin codeOrigin)
{
    VM& vm = exec->vm();
    DeferGC deferGC(vm.heap);
    
    for (; codeOrigin.inlineCallFrame; codeOrigin = codeOrigin.inlineCallFrame->directCaller) {
        CodeBlock* codeBlock = codeOrigin.inlineCallFrame->baselineCodeBlock.get();
        JITWorklist::instance()->compileNow(codeBlock);
    }
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

