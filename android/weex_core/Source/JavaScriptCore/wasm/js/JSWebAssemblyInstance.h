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

#include "JSDestructibleObject.h"
#include "JSObject.h"
#include "JSWebAssemblyCodeBlock.h"
#include "JSWebAssemblyMemory.h"
#include "JSWebAssemblyTable.h"

namespace JSC {

class JSModuleNamespaceObject;
class JSWebAssemblyModule;

class JSWebAssemblyInstance : public JSDestructibleObject {
public:
    typedef JSDestructibleObject Base;


    static JSWebAssemblyInstance* create(VM&, Structure*, JSWebAssemblyModule*, JSModuleNamespaceObject*);
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    DECLARE_INFO;

    JSWebAssemblyModule* module() const
    {
        ASSERT(m_codeBlock);
        return m_codeBlock->module();
    }

    JSWebAssemblyCodeBlock* codeBlock() const
    {
        ASSERT(m_codeBlock);
        return m_codeBlock.get();
    }

    WriteBarrier<JSObject>* importFunction(unsigned idx)
    {
        RELEASE_ASSERT(idx < m_numImportFunctions);
        return &importFunctions()[idx];
    }

    WriteBarrier<JSObject>* importFunctions()
    {
        return bitwise_cast<WriteBarrier<JSObject>*>(bitwise_cast<char*>(this) + offsetOfImportFunctions());
    }

    void setImportFunction(VM& vm, JSObject* value, unsigned idx)
    {
        importFunction(idx)->set(vm, this, value);
    }

    JSWebAssemblyMemory* memory() { return m_memory.get(); }
    // Calling this might trigger a recompile.
    void setMemory(VM&, ExecState*, JSWebAssemblyMemory*);
    Wasm::Memory::Mode memoryMode() { return memory()->memory().mode(); }

    JSWebAssemblyTable* table() { return m_table.get(); }
    void setTable(VM& vm, JSWebAssemblyTable* table) { m_table.set(vm, this, table); }

    int32_t loadI32Global(unsigned i) const { return m_globals.get()[i]; }
    int64_t loadI64Global(unsigned i) const { return m_globals.get()[i]; }
    float loadF32Global(unsigned i) const { return bitwise_cast<float>(loadI32Global(i)); }
    double loadF64Global(unsigned i) const { return bitwise_cast<double>(loadI64Global(i)); }
    void setGlobal(unsigned i, int64_t bits) { m_globals.get()[i] = bits; }

    static size_t offsetOfImportFunction(unsigned idx)
    {
        return offsetOfImportFunctions() + sizeof(WriteBarrier<JSCell>) * idx;
    }

    static ptrdiff_t offsetOfMemory() { return OBJECT_OFFSETOF(JSWebAssemblyInstance, m_memory); }
    static ptrdiff_t offsetOfTable() { return OBJECT_OFFSETOF(JSWebAssemblyInstance, m_table); }
    static ptrdiff_t offsetOfGlobals() { return OBJECT_OFFSETOF(JSWebAssemblyInstance, m_globals); }
    static size_t offsetOfImportFunctions() { return WTF::roundUpToMultipleOf<sizeof(WriteBarrier<JSCell>)>(sizeof(JSWebAssemblyInstance)); }
    static size_t offsetOfImportFunction(size_t importFunctionNum) { return offsetOfImportFunctions() + importFunctionNum * sizeof(sizeof(WriteBarrier<JSCell>)); }

protected:
    JSWebAssemblyInstance(VM&, Structure*, unsigned numImportFunctions);
    void finishCreation(VM&, JSWebAssemblyModule*, JSModuleNamespaceObject*);
    static void destroy(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);

    static size_t allocationSize(unsigned numImportFunctions)
    {
        return offsetOfImportFunctions() + sizeof(WriteBarrier<JSCell>) * numImportFunctions;
    }

private:
    WriteBarrier<JSWebAssemblyCodeBlock> m_codeBlock;
    WriteBarrier<JSModuleNamespaceObject> m_moduleNamespaceObject;
    WriteBarrier<JSWebAssemblyMemory> m_memory;
    WriteBarrier<JSWebAssemblyTable> m_table;
    MallocPtr<uint64_t> m_globals;
    unsigned m_numImportFunctions;
};

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
