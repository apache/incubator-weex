/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
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
#include "CodeBlockSet.h"

#include "CodeBlock.h"
#include "JSCInlines.h"
#include <wtf/CommaPrinter.h>

namespace JSC {

CodeBlockSet::CodeBlockSet()
{
}

CodeBlockSet::~CodeBlockSet()
{
}

void CodeBlockSet::add(CodeBlock* codeBlock)
{
    LockHolder locker(&m_lock);
    bool isNewEntry = m_newCodeBlocks.add(codeBlock).isNewEntry;
    ASSERT_UNUSED(isNewEntry, isNewEntry);
}

void CodeBlockSet::promoteYoungCodeBlocks(const AbstractLocker&)
{
    ASSERT(m_lock.isLocked());
    m_oldCodeBlocks.add(m_newCodeBlocks.begin(), m_newCodeBlocks.end());
    m_newCodeBlocks.clear();
}

void CodeBlockSet::clearMarksForFullCollection()
{
    LockHolder locker(&m_lock);
    for (CodeBlock* codeBlock : m_oldCodeBlocks)
        codeBlock->clearVisitWeaklyHasBeenCalled();
}

void CodeBlockSet::lastChanceToFinalize(VM& vm)
{
    LockHolder locker(&m_lock);
    for (CodeBlock* codeBlock : m_newCodeBlocks)
        codeBlock->structure(vm)->classInfo()->methodTable.destroy(codeBlock);

    for (CodeBlock* codeBlock : m_oldCodeBlocks)
        codeBlock->structure(vm)->classInfo()->methodTable.destroy(codeBlock);
}

void CodeBlockSet::deleteUnmarkedAndUnreferenced(VM& vm, CollectionScope scope)
{
    LockHolder locker(&m_lock);
    Vector<CodeBlock*> unmarked;
    
    auto consider = [&] (HashSet<CodeBlock*>& set) {
        for (CodeBlock* codeBlock : set) {
            if (Heap::isMarked(codeBlock))
                continue;;
            unmarked.append(codeBlock);
        }
        for (CodeBlock* codeBlock : unmarked) {
            codeBlock->structure(vm)->classInfo()->methodTable.destroy(codeBlock);
            set.remove(codeBlock);
        }
        unmarked.resize(0);
    };

    switch (scope) {
    case CollectionScope::Eden:
        consider(m_newCodeBlocks);
        break;
    case CollectionScope::Full:
        consider(m_oldCodeBlocks);
        consider(m_newCodeBlocks);
        break;
    }

    // Any remaining young CodeBlocks are live and need to be promoted to the set of old CodeBlocks.
    promoteYoungCodeBlocks(locker);
}

bool CodeBlockSet::contains(const AbstractLocker&, void* candidateCodeBlock)
{
    RELEASE_ASSERT(m_lock.isLocked());
    CodeBlock* codeBlock = static_cast<CodeBlock*>(candidateCodeBlock);
    if (!HashSet<CodeBlock*>::isValidValue(codeBlock))
        return false;
    return m_oldCodeBlocks.contains(codeBlock) || m_newCodeBlocks.contains(codeBlock) || m_currentlyExecuting.contains(codeBlock);
}

void CodeBlockSet::clearCurrentlyExecuting()
{
    m_currentlyExecuting.clear();
}

void CodeBlockSet::dump(PrintStream& out) const
{
    CommaPrinter comma;
    out.print("{old = [");
    for (CodeBlock* codeBlock : m_oldCodeBlocks)
        out.print(comma, pointerDump(codeBlock));
    out.print("], new = [");
    comma = CommaPrinter();
    for (CodeBlock* codeBlock : m_newCodeBlocks)
        out.print(comma, pointerDump(codeBlock));
    out.print("], currentlyExecuting = [");
    comma = CommaPrinter();
    for (CodeBlock* codeBlock : m_currentlyExecuting)
        out.print(comma, pointerDump(codeBlock));
    out.print("]}");
}

} // namespace JSC

