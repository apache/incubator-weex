/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "JSFunction.h"
#include "JSWebAssemblyCodeBlock.h"

namespace JSC {

class WebAssemblyWrapperFunction : public JSFunction {
public:
    typedef JSFunction Base;

    const static unsigned StructureFlags = Base::StructureFlags;

    DECLARE_INFO;

    static WebAssemblyWrapperFunction* create(VM&, JSGlobalObject*, JSObject*, unsigned importIndex, JSWebAssemblyCodeBlock*, Wasm::SignatureIndex);
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    Wasm::SignatureIndex signatureIndex() const { return m_signatureIndex; }
    void* wasmEntrypoint() { return m_wasmEntrypointCode; }
    JSObject* function() { return m_function.get(); }

protected:
    static void visitChildren(JSCell*, SlotVisitor&);

    void finishCreation(VM&, NativeExecutable*, unsigned length, const String& name, JSObject*, JSWebAssemblyCodeBlock*);

private:
    WebAssemblyWrapperFunction(VM&, JSGlobalObject*, Structure*, Wasm::SignatureIndex, void* wasmEntrypointCode);

    // We keep a reference to our CodeBlock because we have a raw
    // pointer to asm code that it owns.
    WriteBarrier<JSWebAssemblyCodeBlock> m_codeBlock;
    WriteBarrier<JSObject> m_function;
    void* m_wasmEntrypointCode;
    Wasm::SignatureIndex m_signatureIndex;
};

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
