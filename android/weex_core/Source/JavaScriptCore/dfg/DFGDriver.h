/*
 * Copyright (C) 2011, 2013, 2014 Apple Inc. All rights reserved.
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

#include "CallFrame.h"
#include "DFGCompilationMode.h"
#include "DFGPlan.h"

namespace JSC {

class CodeBlock;
class JITCode;
class MacroAssemblerCodePtr;
class VM;

namespace DFG {

JS_EXPORT_PRIVATE unsigned getNumCompilations();

// If the worklist is non-null, we do a concurrent compile. Otherwise we do a synchronous
// compile. Even if we do a synchronous compile, we call the callback with the result.
CompilationResult compile(
    VM&, CodeBlock*, CodeBlock* profiledDFGCodeBlock, CompilationMode,
    unsigned osrEntryBytecodeIndex, const Operands<JSValue>& mustHandleValues,
    Ref<DeferredCompilationCallback>&&);

} } // namespace JSC::DFG
