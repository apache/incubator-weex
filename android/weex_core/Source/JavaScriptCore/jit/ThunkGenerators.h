/*
 * Copyright (C) 2010, 2012, 2013 Apple Inc. All rights reserved.
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

#pragma once

#include "CodeSpecializationKind.h"
#include "ThunkGenerator.h"

#if ENABLE(JIT)
namespace JSC {

class CallLinkInfo;

MacroAssemblerCodeRef throwExceptionFromCallSlowPathGenerator(VM*);

MacroAssemblerCodeRef linkCallThunk(VM*, CallLinkInfo&, CodeSpecializationKind);
MacroAssemblerCodeRef linkCallThunkGenerator(VM*);
MacroAssemblerCodeRef linkPolymorphicCallThunkGenerator(VM*);

MacroAssemblerCodeRef virtualThunkFor(VM*, CallLinkInfo&);

MacroAssemblerCodeRef nativeCallGenerator(VM*);
MacroAssemblerCodeRef nativeConstructGenerator(VM*);
MacroAssemblerCodeRef nativeTailCallGenerator(VM*);
MacroAssemblerCodeRef nativeTailCallWithoutSavedTagsGenerator(VM*);
MacroAssemblerCodeRef arityFixupGenerator(VM*);
MacroAssemblerCodeRef unreachableGenerator(VM*);

MacroAssemblerCodeRef charCodeAtThunkGenerator(VM*);
MacroAssemblerCodeRef charAtThunkGenerator(VM*);
MacroAssemblerCodeRef clz32ThunkGenerator(VM*);
MacroAssemblerCodeRef fromCharCodeThunkGenerator(VM*);
MacroAssemblerCodeRef absThunkGenerator(VM*);
MacroAssemblerCodeRef ceilThunkGenerator(VM*);
MacroAssemblerCodeRef expThunkGenerator(VM*);
MacroAssemblerCodeRef floorThunkGenerator(VM*);
MacroAssemblerCodeRef logThunkGenerator(VM*);
MacroAssemblerCodeRef roundThunkGenerator(VM*);
MacroAssemblerCodeRef sqrtThunkGenerator(VM*);
MacroAssemblerCodeRef imulThunkGenerator(VM*);
MacroAssemblerCodeRef randomThunkGenerator(VM*);
MacroAssemblerCodeRef truncThunkGenerator(VM*);

MacroAssemblerCodeRef boundThisNoArgsFunctionCallGenerator(VM*);

#if ENABLE(WEBASSEMBLY)
MacroAssemblerCodeRef throwExceptionFromWasmThunkGenerator(VM*);
#endif

}
#endif // ENABLE(JIT)
