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

#include "Strong.h"
#include <limits.h>

namespace JSC {

class CodeBlock;
class JSObject;

class StackFrame {
public:
    StackFrame(VM& vm, JSCell* callee)
        : m_callee(vm, callee)
    { }

    StackFrame(VM& vm, JSCell* callee, CodeBlock* codeBlock, unsigned bytecodeOffset)
        : m_callee(vm, callee)
        , m_codeBlock(vm, codeBlock)
        , m_bytecodeOffset(bytecodeOffset)
    { }

    bool hasLineAndColumnInfo() const { return !!m_codeBlock; }
    
    void computeLineAndColumn(unsigned& line, unsigned& column) const;
    String functionName(VM&) const;
    intptr_t sourceID() const;
    String sourceURL() const;
    String toString(VM&) const;

    bool hasBytecodeOffset() const { return m_bytecodeOffset != UINT_MAX; }
    unsigned bytecodeOffset()
    {
        ASSERT(m_bytecodeOffset != UINT_MAX);
        return m_bytecodeOffset;
    }


private:
    Strong<JSCell> m_callee { };
    Strong<CodeBlock> m_codeBlock { };
    unsigned m_bytecodeOffset { UINT_MAX };
    
};

} // namespace JSC

