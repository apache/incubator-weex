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
