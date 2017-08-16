/*
 * Copyright (C) 2013-2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "DFGDesiredTransitions.h"

#if ENABLE(DFG_JIT)

#include "CodeBlock.h"
#include "DFGCommonData.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

DesiredTransition::DesiredTransition(CodeBlock* codeBlock, CodeBlock* codeOriginOwner, Structure* oldStructure, Structure* newStructure)
    : m_codeBlock(codeBlock)
    , m_codeOriginOwner(codeOriginOwner)
    , m_oldStructure(oldStructure)
    , m_newStructure(newStructure)
{
}

void DesiredTransition::reallyAdd(VM& vm, CommonData* common)
{
    common->transitions.append(
        WeakReferenceTransition(
            vm, m_codeBlock,
            m_codeOriginOwner,
            m_oldStructure, m_newStructure));
}

void DesiredTransition::visitChildren(SlotVisitor& visitor)
{
    visitor.appendUnbarriered(m_codeOriginOwner);
    visitor.appendUnbarriered(m_oldStructure);
    visitor.appendUnbarriered(m_newStructure);
}

DesiredTransitions::DesiredTransitions()
{
}

DesiredTransitions::~DesiredTransitions()
{
}

void DesiredTransitions::addLazily(CodeBlock* codeBlock, CodeBlock* codeOriginOwner, Structure* oldStructure, Structure* newStructure)
{
    m_transitions.append(DesiredTransition(codeBlock, codeOriginOwner, oldStructure, newStructure));
}

void DesiredTransitions::reallyAdd(VM& vm, CommonData* common)
{
    for (unsigned i = 0; i < m_transitions.size(); i++)
        m_transitions[i].reallyAdd(vm, common);
}

void DesiredTransitions::visitChildren(SlotVisitor& visitor)
{
    for (unsigned i = 0; i < m_transitions.size(); i++)
        m_transitions[i].visitChildren(visitor);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
