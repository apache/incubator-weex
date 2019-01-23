/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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
