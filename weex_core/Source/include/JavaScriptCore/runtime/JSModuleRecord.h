/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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

#include "AbstractModuleRecord.h"

namespace JSC {

class ModuleProgramExecutable;

// Based on the Source Text Module Record
// http://www.ecma-international.org/ecma-262/6.0/#sec-source-text-module-records
class JSModuleRecord : public AbstractModuleRecord {
    friend class LLIntOffsetsExtractor;
public:
    typedef AbstractModuleRecord Base;

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);
    static JSModuleRecord* create(ExecState*, VM&, Structure*, const Identifier&, const SourceCode&, const VariableEnvironment&, const VariableEnvironment&);

    void link(ExecState*);
    JS_EXPORT_PRIVATE JSValue evaluate(ExecState*);

    const SourceCode& sourceCode() const { return m_sourceCode; }
    const VariableEnvironment& declaredVariables() const { return m_declaredVariables; }
    const VariableEnvironment& lexicalVariables() const { return m_lexicalVariables; }

    ModuleProgramExecutable* moduleProgramExecutable() const { return m_moduleProgramExecutable.get(); }

private:
    JSModuleRecord(VM&, Structure*, const Identifier&, const SourceCode&, const VariableEnvironment&, const VariableEnvironment&);

    void finishCreation(ExecState*, VM&);

    static void visitChildren(JSCell*, SlotVisitor&);
    static void destroy(JSCell*);

    void instantiateDeclarations(ExecState*, ModuleProgramExecutable*);

    SourceCode m_sourceCode;
    VariableEnvironment m_declaredVariables;
    VariableEnvironment m_lexicalVariables;
    WriteBarrier<ModuleProgramExecutable> m_moduleProgramExecutable;
};

} // namespace JSC
