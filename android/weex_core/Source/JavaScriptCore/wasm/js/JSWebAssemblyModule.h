/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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

#include "JSDestructibleObject.h"
#include "JSObject.h"
#include "JSWebAssemblyCodeBlock.h"
#include "UnconditionalFinalizer.h"
#include "WasmFormat.h"
#include <wtf/Bag.h>
#include <wtf/Vector.h>

namespace JSC {

namespace Wasm {
class Plan;
}

class SymbolTable;
class JSWebAssemblyMemory;

class JSWebAssemblyModule : public JSDestructibleObject {
public:
    typedef JSDestructibleObject Base;

    static JSWebAssemblyModule* create(VM&, ExecState*, Structure*, uint8_t* source, size_t byteSize);
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    DECLARE_INFO;

    const Wasm::ModuleInformation& moduleInformation() const { return *m_moduleInformation.get(); }
    RefPtr<Wasm::Memory> takeReservedMemory() { return m_moduleInformation->memory.takeReservedMemory(); }
    SymbolTable* exportSymbolTable() const { return m_exportSymbolTable.get(); }
    Wasm::SignatureIndex signatureIndexFromFunctionIndexSpace(unsigned functionIndexSpace) const
    {
        return m_moduleInformation->signatureIndexFromFunctionIndexSpace(functionIndexSpace);
    }

    // Returns the code block that this module was originally compiled expecting to use. This won't need to recompile.
    JSWebAssemblyCodeBlock* codeBlock() { return m_codeBlocks[m_moduleInformation->memory.mode()].get(); }
    // Returns the appropriate code block for the given memory, possibly triggering a recompile.
    JSWebAssemblyCodeBlock* codeBlock(VM&, ExecState*, JSWebAssemblyMemory*);

private:
    JSWebAssemblyCodeBlock* buildCodeBlock(VM&, ExecState*, Wasm::Plan&, std::optional<Wasm::Memory::Mode> mode = std::nullopt);

    JSWebAssemblyModule(VM&, Structure*);
    void finishCreation(VM&, ExecState*, uint8_t* source, size_t byteSize);
    static void destroy(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);

    RefPtr<ArrayBuffer> m_sourceBuffer;
    std::unique_ptr<Wasm::ModuleInformation> m_moduleInformation;
    WriteBarrier<SymbolTable> m_exportSymbolTable;
    WriteBarrier<JSWebAssemblyCodeBlock> m_codeBlocks[Wasm::Memory::NumberOfModes];
};

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
