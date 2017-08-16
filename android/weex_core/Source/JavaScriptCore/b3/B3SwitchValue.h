/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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

#pragma once

#if ENABLE(B3_JIT)

#include "B3CaseCollection.h"
#include "B3SwitchCase.h"
#include "B3Value.h"

namespace JSC { namespace B3 {

class SwitchValue : public Value {
public:
    static bool accepts(Kind kind) { return kind == Switch; }

    ~SwitchValue();

    // numCaseValues() + 1 == numSuccessors().
    unsigned numCaseValues() const { return m_values.size(); }

    // The successor for this case value is at the same index.
    int64_t caseValue(unsigned index) const { return m_values[index]; }
    
    const Vector<int64_t>& caseValues() const { return m_values; }

    CaseCollection cases(const BasicBlock* owner) const { return CaseCollection(this, owner); }
    CaseCollection cases() const { return cases(owner); }

    // This removes the case and reorders things a bit. If you're iterating the cases from 0 to N,
    // then you can keep iterating after this so long as you revisit this same index (which will now
    // contain some other case value). This removes the case that was removed.
    SwitchCase removeCase(BasicBlock*, unsigned index);

    bool hasFallThrough(const BasicBlock*) const;
    bool hasFallThrough() const;

    // These two functions can be called in any order.
    void setFallThrough(BasicBlock*, const FrequentedBlock&);
    void appendCase(BasicBlock*, const SwitchCase&);
    
    JS_EXPORT_PRIVATE void setFallThrough(const FrequentedBlock&);
    JS_EXPORT_PRIVATE void appendCase(const SwitchCase&);

    void dumpSuccessors(const BasicBlock*, PrintStream&) const override;

protected:
    void dumpMeta(CommaPrinter&, PrintStream&) const override;

    Value* cloneImpl() const override;

private:
    friend class Procedure;

    JS_EXPORT_PRIVATE SwitchValue(Origin, Value* child);

    Vector<int64_t> m_values;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
