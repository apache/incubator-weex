/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "Nodes.h"

namespace JSC {

class JSModuleRecord;
class SourceCode;

class ModuleAnalyzer {
    WTF_MAKE_NONCOPYABLE(ModuleAnalyzer);
public:
    ModuleAnalyzer(ExecState*, const Identifier& moduleKey, const SourceCode&, const VariableEnvironment& declaredVariables, const VariableEnvironment& lexicalVariables);

    JSModuleRecord* analyze(ModuleProgramNode&);

    VM& vm() { return *m_vm; }

    JSModuleRecord* moduleRecord() { return m_moduleRecord.get(); }

private:
    void exportVariable(ModuleProgramNode&, const RefPtr<UniquedStringImpl>&, const VariableEnvironmentEntry&);

    VM* m_vm;
    Strong<JSModuleRecord> m_moduleRecord;
};

} // namespace JSC
