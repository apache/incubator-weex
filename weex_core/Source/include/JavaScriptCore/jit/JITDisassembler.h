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

#if ENABLE(JIT)

#include "MacroAssembler.h"
#include <wtf/Vector.h>
#include <wtf/text/CString.h>

namespace JSC {

class CodeBlock;
class LinkBuffer;

namespace Profiler {
class Compilation;
}

class JITDisassembler {
    WTF_MAKE_FAST_ALLOCATED;
public:
    JITDisassembler(CodeBlock*);
    ~JITDisassembler();
    
    void setStartOfCode(MacroAssembler::Label label) { m_startOfCode = label; }
    void setForBytecodeMainPath(unsigned bytecodeIndex, MacroAssembler::Label label)
    {
        m_labelForBytecodeIndexInMainPath[bytecodeIndex] = label;
    }
    void setForBytecodeSlowPath(unsigned bytecodeIndex, MacroAssembler::Label label)
    {
        m_labelForBytecodeIndexInSlowPath[bytecodeIndex] = label;
    }
    void setEndOfSlowPath(MacroAssembler::Label label) { m_endOfSlowPath = label; }
    void setEndOfCode(MacroAssembler::Label label) { m_endOfCode = label; }
    
    void dump(LinkBuffer&);
    void dump(PrintStream&, LinkBuffer&);
    void reportToProfiler(Profiler::Compilation*, LinkBuffer&);

private:
    void dumpHeader(PrintStream&, LinkBuffer&);
    MacroAssembler::Label firstSlowLabel();
    
    struct DumpedOp {
        unsigned index;
        CString disassembly;
    };
    Vector<DumpedOp> dumpVectorForInstructions(LinkBuffer&, const char* prefix, Vector<MacroAssembler::Label>& labels, MacroAssembler::Label endLabel);
        
    void dumpForInstructions(PrintStream&, LinkBuffer&, const char* prefix, Vector<MacroAssembler::Label>& labels, MacroAssembler::Label endLabel);
    void reportInstructions(Profiler::Compilation*, LinkBuffer&, const char* prefix, Vector<MacroAssembler::Label>& labels, MacroAssembler::Label endLabel);
    
    void dumpDisassembly(PrintStream&, LinkBuffer&, MacroAssembler::Label from, MacroAssembler::Label to);
    
    CodeBlock* m_codeBlock;
    MacroAssembler::Label m_startOfCode;
    Vector<MacroAssembler::Label> m_labelForBytecodeIndexInMainPath;
    Vector<MacroAssembler::Label> m_labelForBytecodeIndexInSlowPath;
    MacroAssembler::Label m_endOfSlowPath;
    MacroAssembler::Label m_endOfCode;
};

} // namespace JSC

#endif // ENABLE(JIT)
