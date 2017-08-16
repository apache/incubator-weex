/*
 * Copyright (C) 2012-2014, 2016 Apple Inc. All rights reserved.
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

#include "config.h"
#include "ProfilerBytecodeSequence.h"

#include "CodeBlock.h"
#include "Interpreter.h"
#include "JSCInlines.h"
#include "JSGlobalObject.h"
#include "Operands.h"
#include <wtf/StringPrintStream.h>

namespace JSC { namespace Profiler {

BytecodeSequence::BytecodeSequence(CodeBlock* codeBlock)
{
    StringPrintStream out;
    
    for (unsigned i = 0; i < codeBlock->numberOfArgumentValueProfiles(); ++i) {
        ConcurrentJSLocker locker(codeBlock->m_lock);
        CString description = codeBlock->valueProfileForArgument(i)->briefDescription(locker);
        if (!description.length())
            continue;
        out.reset();
        out.print("arg", i, ": ", description);
        m_header.append(out.toCString());
    }
    
    StubInfoMap stubInfos;
    codeBlock->getStubInfoMap(stubInfos);
    
    for (unsigned bytecodeIndex = 0; bytecodeIndex < codeBlock->instructions().size();) {
        out.reset();
        codeBlock->dumpBytecode(out, bytecodeIndex, stubInfos);
        m_sequence.append(Bytecode(bytecodeIndex, codeBlock->vm()->interpreter->getOpcodeID(codeBlock->instructions()[bytecodeIndex].u.opcode), out.toCString()));
        bytecodeIndex += opcodeLength(
            codeBlock->vm()->interpreter->getOpcodeID(
                codeBlock->instructions()[bytecodeIndex].u.opcode));
    }
}

BytecodeSequence::~BytecodeSequence()
{
}

unsigned BytecodeSequence::indexForBytecodeIndex(unsigned bytecodeIndex) const
{
    return binarySearch<Bytecode, unsigned>(m_sequence, m_sequence.size(), bytecodeIndex, getBytecodeIndexForBytecode) - m_sequence.begin();
}

const Bytecode& BytecodeSequence::forBytecodeIndex(unsigned bytecodeIndex) const
{
    return at(indexForBytecodeIndex(bytecodeIndex));
}

void BytecodeSequence::addSequenceProperties(ExecState* exec, JSObject* result) const
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSArray* header = constructEmptyArray(exec, 0);
    RETURN_IF_EXCEPTION(scope, void());
    for (unsigned i = 0; i < m_header.size(); ++i) {
        header->putDirectIndex(exec, i, jsString(exec, String::fromUTF8(m_header[i])));
        RETURN_IF_EXCEPTION(scope, void());
    }
    result->putDirect(vm, exec->propertyNames().header, header);
    
    JSArray* sequence = constructEmptyArray(exec, 0);
    RETURN_IF_EXCEPTION(scope, void());
    for (unsigned i = 0; i < m_sequence.size(); ++i) {
        sequence->putDirectIndex(exec, i, m_sequence[i].toJS(exec));
        RETURN_IF_EXCEPTION(scope, void());
    }
    result->putDirect(vm, exec->propertyNames().bytecode, sequence);
}

} } // namespace JSC::Profiler

