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

#if ENABLE(DFG_JIT)

#include "CodeOrigin.h"
#include "DFGCommon.h"
#include "DumpContext.h"
#include "MacroAssembler.h"
#include "ProfilerCompilation.h"
#include <wtf/HashMap.h>
#include <wtf/StringPrintStream.h>
#include <wtf/Vector.h>

namespace JSC {

class LinkBuffer;

namespace DFG {

class Graph;

class Disassembler {
    WTF_MAKE_FAST_ALLOCATED;
public:
    Disassembler(Graph&);
    
    void setStartOfCode(MacroAssembler::Label label) { m_startOfCode = label; }
    void setForBlockIndex(BlockIndex blockIndex, MacroAssembler::Label label)
    {
        m_labelForBlockIndex[blockIndex] = label;
    }
    void setForNode(Node* node, MacroAssembler::Label label)
    {
        ASSERT(label.isSet());
        m_labelForNode.add(node, label);
    }
    void setEndOfMainPath(MacroAssembler::Label label)
    {
        m_endOfMainPath = label;
    }
    void setEndOfCode(MacroAssembler::Label label)
    {
        m_endOfCode = label;
    }
    
    void dump(PrintStream&, LinkBuffer&);
    void dump(LinkBuffer&);
    void reportToProfiler(Profiler::Compilation*, LinkBuffer&);
    
private:
    void dumpHeader(PrintStream&, LinkBuffer&);
    
    struct DumpedOp {
        DumpedOp(CodeOrigin codeOrigin, CString text)
            : codeOrigin(codeOrigin)
            , text(text)
        {
        }
        
        CodeOrigin codeOrigin;
        CString text;
    };
    void append(Vector<DumpedOp>&, StringPrintStream&, CodeOrigin&);
    Vector<DumpedOp> createDumpList(LinkBuffer&);
    
    void dumpDisassembly(PrintStream&, const char* prefix, LinkBuffer&, MacroAssembler::Label& previousLabel, MacroAssembler::Label currentLabel, Node* context);
    
    Graph& m_graph;
    DumpContext m_dumpContext;
    MacroAssembler::Label m_startOfCode;
    Vector<MacroAssembler::Label> m_labelForBlockIndex;
    HashMap<Node*, MacroAssembler::Label> m_labelForNode;
    MacroAssembler::Label m_endOfMainPath;
    MacroAssembler::Label m_endOfCode;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
