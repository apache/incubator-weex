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

#include "JSCell.h"
#include "RegisterAtOffsetList.h"
#include "Structure.h"
#include "WasmFormat.h"

namespace JSC {

class JSWebAssemblyCallee : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static JSWebAssemblyCallee* create(VM& vm, Wasm::Entrypoint&& entrypoint)
    {
        JSWebAssemblyCallee* callee = new (NotNull, allocateCell<JSWebAssemblyCallee>(vm.heap)) JSWebAssemblyCallee(vm);
        callee->finishCreation(vm, WTFMove(entrypoint));
        return callee;
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype) 
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
    }

    DECLARE_EXPORT_INFO;
    static const bool needsDestruction = true;
    static void destroy(JSCell*);

    void* entrypoint() { return m_entrypoint.compilation->code().executableAddress(); }

    RegisterAtOffsetList* calleeSaveRegisters() { return &m_entrypoint.calleeSaveRegisters; }

private:
    void finishCreation(VM&, Wasm::Entrypoint&&);
    JSWebAssemblyCallee(VM&);

    Wasm::Entrypoint m_entrypoint;
};

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
