/*
 * Copyright (C) 2016 Yusuke Suzuki <utatane.tea@gmail.com>
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

#include "JSGlobalObject.h"
#include "JSObject.h"
#include "SourceCode.h"

namespace JSC {

class JSSourceCode : public JSCell {
public:
    using Base = JSCell;

    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;
    static const bool needsDestruction = true;

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSSourceCodeType, StructureFlags), info());
    }

    static JSSourceCode* create(VM& vm, Structure* structure, SourceCode&& sourceCode)
    {
        auto* result = new (NotNull, allocateCell<JSSourceCode>(vm.heap)) JSSourceCode(vm, structure, WTFMove(sourceCode));
        result->finishCreation(vm);
        return result;
    }

    static JSSourceCode* create(VM& vm, SourceCode&& sourceCode)
    {
        return create(vm, vm.sourceCodeStructure.get(), WTFMove(sourceCode));
    }

    const SourceCode& sourceCode() const
    {
        return m_sourceCode;
    }

    static void destroy(JSCell*);

private:
    JSSourceCode(VM& vm, Structure* structure, SourceCode&& sourceCode)
        : Base(vm, structure)
        , m_sourceCode(WTFMove(sourceCode))
    {
    }

    SourceCode m_sourceCode;
};

} // namespace JSC
