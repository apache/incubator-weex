/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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
#include <wtf/Insertion.h>
#include <wtf/Vector.h>

namespace JSC { namespace B3 { namespace Air {

class BasicBlock;
class Code;

typedef WTF::Insertion<Inst> Insertion;

class InsertionSet {
public:
    InsertionSet(Code& code)
        : m_code(code)
    {
    }

    Code& code() { return m_code; }

    template<typename T>
    void appendInsertion(T&& insertion)
    {
        m_insertions.append(std::forward<T>(insertion));
    }

    template<typename Inst>
    void insertInst(size_t index, Inst&& inst)
    {
        appendInsertion(Insertion(index, std::forward<Inst>(inst)));
    }

    template <typename InstVector>
    void insertInsts(size_t index, const InstVector& insts)
    {
        for (const Inst& inst : insts)
            insertInst(index, inst);
    }
    void insertInsts(size_t index, Vector<Inst>&&);
    
    template<typename... Arguments>
    void insert(size_t index, Arguments&&... arguments)
    {
        insertInst(index, Inst(std::forward<Arguments>(arguments)...));
    }

    void execute(BasicBlock*);

private:
    Code& m_code;
    Vector<Insertion, 8> m_insertions;
};

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
