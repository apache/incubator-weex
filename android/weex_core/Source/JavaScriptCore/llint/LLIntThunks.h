/*
 * Copyright (C) 2012, 2013 Apple Inc. All rights reserved.
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

#include "MacroAssemblerCodeRef.h"

namespace JSC {

class VM;
struct ProtoCallFrame;
typedef int64_t EncodedJSValue;

extern "C" {
    EncodedJSValue vmEntryToJavaScript(void*, VM*, ProtoCallFrame*);
    EncodedJSValue vmEntryToNative(void*, VM*, ProtoCallFrame*);
}

EncodedJSValue JS_EXPORT_PRIVATE vmEntryToWasm(void*, VM*, ProtoCallFrame*);

namespace LLInt {

MacroAssemblerCodeRef functionForCallEntryThunkGenerator(VM*);
MacroAssemblerCodeRef functionForConstructEntryThunkGenerator(VM*);
MacroAssemblerCodeRef functionForCallArityCheckThunkGenerator(VM*);
MacroAssemblerCodeRef functionForConstructArityCheckThunkGenerator(VM*);
MacroAssemblerCodeRef evalEntryThunkGenerator(VM*);
MacroAssemblerCodeRef programEntryThunkGenerator(VM*);
MacroAssemblerCodeRef moduleProgramEntryThunkGenerator(VM*);

} } // namespace JSC::LLInt
