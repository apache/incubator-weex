/*
 * Copyright (C) 2011, 2016 Apple Inc. All rights reserved.
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
#include "DFGExitProfile.h"

#if ENABLE(DFG_JIT)

#include "CodeBlock.h"
#include "VMInlines.h"

namespace JSC { namespace DFG {

void FrequentExitSite::dump(PrintStream& out) const
{
    out.print("bc#", m_bytecodeOffset, ": ", m_kind, "/", m_jitType);
}

ExitProfile::ExitProfile() { }
ExitProfile::~ExitProfile() { }

bool ExitProfile::add(const ConcurrentJSLocker&, CodeBlock* owner, const FrequentExitSite& site)
{
    ASSERT(site.jitType() != ExitFromAnything);

    CODEBLOCK_LOG_EVENT(owner, "frequentExit", (site));
    
    if (Options::verboseExitProfile())
        dataLog(pointerDump(owner), ": Adding exit site: ", site, "\n");
    
    // If we've never seen any frequent exits then create the list and put this site
    // into it.
    if (!m_frequentExitSites) {
        m_frequentExitSites = std::make_unique<Vector<FrequentExitSite>>();
        m_frequentExitSites->append(site);
        return true;
    }
    
    // Don't add it if it's already there. This is O(n), but that's OK, because we
    // know that the total number of places where code exits tends to not be large,
    // and this code is only used when recompilation is triggered.
    for (unsigned i = 0; i < m_frequentExitSites->size(); ++i) {
        if (m_frequentExitSites->at(i) == site)
            return false;
    }
    
    m_frequentExitSites->append(site);
    return true;
}

Vector<FrequentExitSite> ExitProfile::exitSitesFor(unsigned bytecodeIndex)
{
    Vector<FrequentExitSite> result;
    
    if (!m_frequentExitSites)
        return result;
    
    for (unsigned i = 0; i < m_frequentExitSites->size(); ++i) {
        if (m_frequentExitSites->at(i).bytecodeOffset() == bytecodeIndex)
            result.append(m_frequentExitSites->at(i));
    }
    
    return result;
}

bool ExitProfile::hasExitSite(const ConcurrentJSLocker&, const FrequentExitSite& site) const
{
    if (!m_frequentExitSites)
        return false;
    
    for (unsigned i = m_frequentExitSites->size(); i--;) {
        if (site.subsumes(m_frequentExitSites->at(i)))
            return true;
    }
    return false;
}

QueryableExitProfile::QueryableExitProfile() { }
QueryableExitProfile::~QueryableExitProfile() { }

void QueryableExitProfile::initialize(const ConcurrentJSLocker&, const ExitProfile& profile)
{
    if (!profile.m_frequentExitSites)
        return;
    
    for (unsigned i = 0; i < profile.m_frequentExitSites->size(); ++i)
        m_frequentExitSites.add(profile.m_frequentExitSites->at(i));
}

} } // namespace JSC::DFG

#endif
