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

#include "AirInst.h"
#include "B3SparseCollection.h"
#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>
#include <wtf/ScopedLambda.h>
#include <wtf/text/CString.h>

namespace JSC { namespace B3 { namespace Air {

class Code;
struct GenerationContext;

class Special {
    WTF_MAKE_NONCOPYABLE(Special);
    WTF_MAKE_FAST_ALLOCATED;
public:
    static const char* const dumpPrefix;
    
    Special();
    virtual ~Special();

    Code& code() const { return *m_code; }

    CString name() const;

    virtual void forEachArg(Inst&, const ScopedLambda<Inst::EachArgCallback>&) = 0;
    virtual bool isValid(Inst&) = 0;
    virtual bool admitsStack(Inst&, unsigned argIndex) = 0;
    virtual std::optional<unsigned> shouldTryAliasingDef(Inst&);

    // This gets called on for each Inst that uses this Special. Note that there is no way to
    // guarantee that a Special gets used from just one Inst, because Air might taildup late. So,
    // if you want to pass this information down to generate(), then you have to either:
    //
    // 1) Generate Air that starts with a separate Special per Patch Inst, and then merge
    //    usedRegister sets. This is probably not great, but it optimizes for the common case that
    //    Air didn't duplicate code or that such duplication didn't cause any interesting changes to
    //    register assignment.
    //
    // 2) Have the Special maintain a HashMap<Inst*, RegisterSet>. This works because the analysis
    //    that feeds into this call is performed just before code generation and there is no way
    //    for the Vector<>'s that contain the Insts to be reallocated. This allows generate() to
    //    consult the HashMap.
    //
    // 3) Hybrid: you could use (1) and fire up a HashMap if you see multiple calls.
    //
    // Note that it's not possible to rely on reportUsedRegisters() being called in the same order
    // as generate(). If we could rely on that, then we could just have each Special instance
    // maintain a Vector of RegisterSet's and then process that vector in the right order in
    // generate(). But, the ordering difference is unlikely to change since it would harm the
    // performance of the liveness analysis.
    //
    // Currently, we do (1) for B3 stackmaps.
    virtual void reportUsedRegisters(Inst&, const RegisterSet&) = 0;
    
    virtual CCallHelpers::Jump generate(Inst&, CCallHelpers&, GenerationContext&) = 0;

    virtual RegisterSet extraEarlyClobberedRegs(Inst&) = 0;
    virtual RegisterSet extraClobberedRegs(Inst&) = 0;
    
    // By default, this returns false.
    virtual bool isTerminal(Inst&);

    // By default, this returns true.
    virtual bool hasNonArgEffects(Inst&);

    // By default, this returns true.
    virtual bool hasNonArgNonControlEffects(Inst&);

    void dump(PrintStream&) const;
    void deepDump(PrintStream&) const;

protected:
    virtual void dumpImpl(PrintStream&) const = 0;
    virtual void deepDumpImpl(PrintStream&) const = 0;

private:
    friend class Code;
    friend class SparseCollection<Special>;

    unsigned m_index { UINT_MAX };
    Code* m_code { nullptr };
};

class DeepSpecialDump {
public:
    DeepSpecialDump(const Special* special)
        : m_special(special)
    {
    }

    void dump(PrintStream& out) const
    {
        if (m_special)
            m_special->deepDump(out);
        else
            out.print("<null>");
    }

private:
    const Special* m_special;
};

inline DeepSpecialDump deepDump(const Special* special)
{
    return DeepSpecialDump(special);
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
