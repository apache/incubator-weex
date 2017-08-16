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

#include "AirArg.h"
#include "AirKind.h"
#include "B3StackmapSpecial.h"
#include <wtf/HashMap.h>

namespace JSC { namespace B3 {

namespace Air {
struct Inst;
}

// We want to lower Check instructions to a branch, but then we want to route that branch to our
// out-of-line code instead of doing anything else. For this reason, a CheckSpecial will remember
// which branch opcode we have selected along with the number of args in the overload we want. It
// will create an Inst with that opcode plus the appropriate args from the owning Inst whenever you
// call any of the callbacks.
//
// Note that for CheckAdd, CheckSub, and CheckMul we expect that the B3 arguments are the reverse
// of the Air arguments (Add(a, b) => Add32 b, a). Except:
// - CheckSub(0, x), which turns into BranchNeg32 x.
// - CheckMul(a, b), which turns into Mul32 b, a but we pass Any for a's ValueRep.

class CheckSpecial : public StackmapSpecial {
public:
    // Support for hash consing these things.
    class Key {
    public:
        Key()
            : m_stackmapRole(SameAsRep)
            , m_numArgs(0)
        {
        }
        
        Key(Air::Kind kind, unsigned numArgs, RoleMode stackmapRole = SameAsRep)
            : m_kind(kind)
            , m_stackmapRole(stackmapRole)
            , m_numArgs(numArgs)
        {
        }

        explicit Key(const Air::Inst&);

        bool operator==(const Key& other) const
        {
            return m_kind == other.m_kind
                && m_numArgs == other.m_numArgs
                && m_stackmapRole == other.m_stackmapRole;
        }

        bool operator!=(const Key& other) const
        {
            return !(*this == other);
        }

        explicit operator bool() const { return *this != Key(); }

        Air::Kind kind() const { return m_kind; }
        unsigned numArgs() const { return m_numArgs; }
        RoleMode stackmapRole() const { return m_stackmapRole; }

        void dump(PrintStream& out) const;

        Key(WTF::HashTableDeletedValueType)
            : m_stackmapRole(SameAsRep)
            , m_numArgs(1)
        {
        }

        bool isHashTableDeletedValue() const
        {
            return *this == Key(WTF::HashTableDeletedValue);
        }

        unsigned hash() const
        {
            // Seriously, we don't need to be smart here. It just doesn't matter.
            return m_kind.hash() + m_numArgs + m_stackmapRole;
        }
        
    private:
        Air::Kind m_kind;
        RoleMode m_stackmapRole;
        unsigned m_numArgs;
    };
    
    CheckSpecial(Air::Kind, unsigned numArgs, RoleMode stackmapRole = SameAsRep);
    CheckSpecial(const Key&);
    ~CheckSpecial();

protected:
    // Constructs and returns the Inst representing the branch that this will use.
    Air::Inst hiddenBranch(const Air::Inst&) const;

    void forEachArg(Air::Inst&, const ScopedLambda<Air::Inst::EachArgCallback>&) override;
    bool isValid(Air::Inst&) override;
    bool admitsStack(Air::Inst&, unsigned argIndex) override;
    std::optional<unsigned> shouldTryAliasingDef(Air::Inst&) override;

    // NOTE: the generate method will generate the hidden branch and then register a LatePath that
    // generates the stackmap. Super crazy dude!

    CCallHelpers::Jump generate(Air::Inst&, CCallHelpers&, Air::GenerationContext&) override;

    void dumpImpl(PrintStream&) const override;
    void deepDumpImpl(PrintStream&) const override;

private:
    Air::Kind m_checkKind;
    RoleMode m_stackmapRole;
    unsigned m_numCheckArgs;
};

struct CheckSpecialKeyHash {
    static unsigned hash(const CheckSpecial::Key& key) { return key.hash(); }
    static bool equal(const CheckSpecial::Key& a, const CheckSpecial::Key& b) { return a == b; }
    static const bool safeToCompareToEmptyOrDeleted = true;
};

} } // namespace JSC::B3

namespace WTF {

template<typename T> struct DefaultHash;
template<> struct DefaultHash<JSC::B3::CheckSpecial::Key> {
    typedef JSC::B3::CheckSpecialKeyHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<JSC::B3::CheckSpecial::Key> : SimpleClassHashTraits<JSC::B3::CheckSpecial::Key> {
    // I don't want to think about this very hard, it's not worth it. I'm a be conservative.
    static const bool emptyValueIsZero = false;
};

} // namespace WTF

#endif // ENABLE(B3_JIT)
