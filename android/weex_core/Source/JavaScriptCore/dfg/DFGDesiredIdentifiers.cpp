/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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
#include "DFGDesiredIdentifiers.h"

#if ENABLE(DFG_JIT)

#include "CodeBlock.h"
#include "DFGCommonData.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

DesiredIdentifiers::DesiredIdentifiers()
    : m_codeBlock(nullptr)
    , m_didProcessIdentifiers(false)
{
}

DesiredIdentifiers::DesiredIdentifiers(CodeBlock* codeBlock)
    : m_codeBlock(codeBlock)
    , m_didProcessIdentifiers(false)
{
}

DesiredIdentifiers::~DesiredIdentifiers()
{
}

unsigned DesiredIdentifiers::numberOfIdentifiers()
{
    return m_codeBlock->numberOfIdentifiers() + m_addedIdentifiers.size();
}

unsigned DesiredIdentifiers::ensure(UniquedStringImpl* rep)
{
    if (!m_didProcessIdentifiers) {
        // Do this now instead of the constructor so that we don't pay the price on the main
        // thread. Also, not all compilations need to call ensure().
        for (unsigned index = m_codeBlock->numberOfIdentifiers(); index--;)
            m_identifierNumberForName.add(m_codeBlock->identifier(index).impl(), index);
        m_didProcessIdentifiers = true;
    }

    auto addResult = m_identifierNumberForName.add(rep, numberOfIdentifiers());
    unsigned result = addResult.iterator->value;
    if (addResult.isNewEntry) {
        m_addedIdentifiers.append(rep);
        ASSERT(at(result) == rep);
    }
    return result;
}

UniquedStringImpl* DesiredIdentifiers::at(unsigned index) const
{
    UniquedStringImpl* result;
    if (index < m_codeBlock->numberOfIdentifiers())
        result = m_codeBlock->identifier(index).impl();
    else
        result = m_addedIdentifiers[index - m_codeBlock->numberOfIdentifiers()];
    ASSERT(result->hasAtLeastOneRef());
    return result;
}

void DesiredIdentifiers::reallyAdd(VM& vm, CommonData* commonData)
{
    for (auto rep : m_addedIdentifiers) {
        ASSERT(rep->hasAtLeastOneRef());
        commonData->dfgIdentifiers.append(Identifier::fromUid(&vm, rep));
    }
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

