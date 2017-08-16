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

#include "B3Origin.h"
#include "B3Type.h"
#include "B3TypeMap.h"
#include <wtf/Insertion.h>
#include <wtf/Vector.h>

namespace JSC { namespace B3 {

class BasicBlock;
class Procedure;
class Value;

typedef WTF::Insertion<Value*> Insertion;

class InsertionSet {
public:
    InsertionSet(Procedure& procedure)
        : m_procedure(procedure)
    {
    }

    bool isEmpty() const { return m_insertions.isEmpty(); }

    Procedure& code() { return m_procedure; }

    void appendInsertion(const Insertion& insertion)
    {
        m_insertions.append(insertion);
    }

    Value* insertValue(size_t index, Value* value)
    {
        appendInsertion(Insertion(index, value));
        return value;
    }

    template<typename ValueType, typename... Arguments>
    ValueType* insert(size_t index, Arguments... arguments);

    Value* insertIntConstant(size_t index, Origin, Type, int64_t value);
    Value* insertIntConstant(size_t index, Value* likeValue, int64_t value);

    Value* insertBottom(size_t index, Origin, Type);
    Value* insertBottom(size_t index, Value*);
    
    Value* insertClone(size_t index, Value*);

    void execute(BasicBlock*);

private:
    Procedure& m_procedure;
    Vector<Insertion, 8> m_insertions;

    TypeMap<Value*> m_bottomForType;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
