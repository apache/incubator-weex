/*
 * Copyright (C) 2011, 2015 Apple Inc. All rights reserved.
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

#if ENABLE(JIT)

#include "CCallHelpers.h"
#include "CallVariant.h"
#include "JITOperations.h"
#include "PutKind.h"

namespace JSC {

enum class GetByIDKind {
    Normal,
    Try,
    WithThis
};

void repatchGetByID(ExecState*, JSValue, const Identifier&, const PropertySlot&, StructureStubInfo&, GetByIDKind);
void buildGetByIDList(ExecState*, JSValue, const Identifier&, const PropertySlot&, StructureStubInfo&);
void buildGetByIDProtoList(ExecState*, JSValue, const Identifier&, const PropertySlot&, StructureStubInfo&);
void repatchPutByID(ExecState*, JSValue, Structure*, const Identifier&, const PutPropertySlot&, StructureStubInfo&, PutKind);
void buildPutByIdList(ExecState*, JSValue, Structure*, const Identifier&, const PutPropertySlot&, StructureStubInfo&, PutKind);
void repatchIn(ExecState*, JSCell*, const Identifier&, bool wasFound, const PropertySlot&, StructureStubInfo&);
void linkFor(ExecState*, CallLinkInfo&, CodeBlock*, JSFunction* callee, MacroAssemblerCodePtr);
void linkDirectFor(ExecState*, CallLinkInfo&, CodeBlock*, MacroAssemblerCodePtr);
void linkSlowFor(ExecState*, CallLinkInfo&);
void unlinkFor(VM&, CallLinkInfo&);
void linkVirtualFor(ExecState*, CallLinkInfo&);
void linkPolymorphicCall(ExecState*, CallLinkInfo&, CallVariant);
void resetGetByID(CodeBlock*, StructureStubInfo&, GetByIDKind);
void resetPutByID(CodeBlock*, StructureStubInfo&);
void resetIn(CodeBlock*, StructureStubInfo&);
void ftlThunkAwareRepatchCall(CodeBlock*, CodeLocationCall, FunctionPtr newCalleeFunction);

} // namespace JSC

#endif // ENABLE(JIT)
