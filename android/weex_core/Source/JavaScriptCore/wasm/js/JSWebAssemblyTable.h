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
#include "WebAssemblyWrapperFunction.h"
#include "WebAssemblyFunction.h"
#include <wtf/MallocPtr.h>

namespace JSC {

namespace Wasm {
struct CallableFunction;
}

class JSWebAssemblyTable : public JSDestructibleObject {
public:
    typedef JSDestructibleObject Base;

    static JSWebAssemblyTable* create(ExecState*, VM&, Structure*, uint32_t initial, std::optional<uint32_t> maximum);
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    DECLARE_INFO;

    std::optional<uint32_t> maximum() const { return m_maximum; }
    uint32_t size() const { return m_size; }
    bool grow(uint32_t newSize) WARN_UNUSED_RETURN;
    JSObject* getFunction(uint32_t index)
    {
        RELEASE_ASSERT(index < m_size);
        return m_jsFunctions.get()[index].get();
    }
    void clearFunction(uint32_t index);
    void setFunction(VM&, uint32_t index, WebAssemblyFunction*);
    void setFunction(VM&, uint32_t index, WebAssemblyWrapperFunction*);

    static ptrdiff_t offsetOfSize() { return OBJECT_OFFSETOF(JSWebAssemblyTable, m_size); }
    static ptrdiff_t offsetOfFunctions() { return OBJECT_OFFSETOF(JSWebAssemblyTable, m_functions); }

    static bool isValidSize(uint32_t size)
    {
        // This tops out at ~384 MB worth of data in this class.
        return size < (1 << 24);
    }

private:
    JSWebAssemblyTable(VM&, Structure*, uint32_t initial, std::optional<uint32_t> maximum);
    void finishCreation(VM&);
    static void destroy(JSCell*);
    static void visitChildren(JSCell*, SlotVisitor&);

    MallocPtr<Wasm::CallableFunction> m_functions;
    MallocPtr<WriteBarrier<JSObject>> m_jsFunctions;
    std::optional<uint32_t> m_maximum;
    uint32_t m_size;
};

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
