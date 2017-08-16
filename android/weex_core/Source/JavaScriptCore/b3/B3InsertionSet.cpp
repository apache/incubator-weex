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

#include "config.h"
#include "B3InsertionSet.h"

#if ENABLE(B3_JIT)

#include "B3BasicBlock.h"
#include "B3ProcedureInlines.h"
#include "B3ValueInlines.h"
#include <wtf/BubbleSort.h>

namespace JSC { namespace B3 {

Value* InsertionSet::insertIntConstant(size_t index, Origin origin, Type type, int64_t value)
{
    return insertValue(index, m_procedure.addIntConstant(origin, type, value));
}

Value* InsertionSet::insertIntConstant(size_t index, Value* likeValue, int64_t value)
{
    return insertIntConstant(index, likeValue->origin(), likeValue->type(), value);
}

Value* InsertionSet::insertBottom(size_t index, Origin origin, Type type)
{
    Value*& bottom = m_bottomForType[type];
    if (!bottom)
        bottom = insertValue(index, m_procedure.addBottom(origin, type));
    return bottom;
}

Value* InsertionSet::insertBottom(size_t index, Value* likeValue)
{
    return insertBottom(index, likeValue->origin(), likeValue->type());
}

Value* InsertionSet::insertClone(size_t index, Value* value)
{
    return insertValue(index, m_procedure.clone(value));
}

void InsertionSet::execute(BasicBlock* block)
{
    bubbleSort(m_insertions.begin(), m_insertions.end());
    executeInsertions(block->m_values, m_insertions);
    m_bottomForType = TypeMap<Value*>();
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

