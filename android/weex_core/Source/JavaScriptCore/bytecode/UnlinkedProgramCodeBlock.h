/*
 * Copyright (C) 2012-2016 Apple Inc. All Rights Reserved.
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

#include "UnlinkedGlobalCodeBlock.h"

namespace JSC {

class UnlinkedProgramCodeBlock final : public UnlinkedGlobalCodeBlock {
public:
    typedef UnlinkedGlobalCodeBlock Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static UnlinkedProgramCodeBlock* create(VM* vm, const ExecutableInfo& info, DebuggerMode debuggerMode)
    {
        UnlinkedProgramCodeBlock* instance = new (NotNull, allocateCell<UnlinkedProgramCodeBlock>(vm->heap)) UnlinkedProgramCodeBlock(vm, vm->unlinkedProgramCodeBlockStructure.get(), info, debuggerMode);
        instance->finishCreation(*vm);
        return instance;
    }

    static void destroy(JSCell*);

    void setVariableDeclarations(const VariableEnvironment& environment) { m_varDeclarations = environment; }
    const VariableEnvironment& variableDeclarations() const { return m_varDeclarations; }

    void setLexicalDeclarations(const VariableEnvironment& environment) { m_lexicalDeclarations = environment; }
    const VariableEnvironment& lexicalDeclarations() const { return m_lexicalDeclarations; }

    static void visitChildren(JSCell*, SlotVisitor&);

private:
    UnlinkedProgramCodeBlock(VM* vm, Structure* structure, const ExecutableInfo& info, DebuggerMode debuggerMode)
        : Base(vm, structure, GlobalCode, info, debuggerMode)
    {
    }

    VariableEnvironment m_varDeclarations;
    VariableEnvironment m_lexicalDeclarations;

public:
    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
    {
        return Structure::create(vm, globalObject, proto, TypeInfo(UnlinkedProgramCodeBlockType, StructureFlags), info());
    }

    DECLARE_INFO;
};

}
