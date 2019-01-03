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

#include "CodeBlock.h"

namespace JSC {

// We sometimes what to print the CodeBlock's ID before setting its JITCode. At that
// point the CodeBlock will claim a bogus JITType. This helper class lets us do that.

class CodeBlockWithJITType {
public:
    CodeBlockWithJITType(CodeBlock* codeBlock, JITCode::JITType jitType)
        : m_codeBlock(codeBlock)
        , m_jitType(jitType)
    {
    }
    
    void dump(PrintStream& out) const
    {
        m_codeBlock->dumpAssumingJITType(out, m_jitType);
    }
private:
    CodeBlock* m_codeBlock;
    JITCode::JITType m_jitType;
};

} // namespace JSC
