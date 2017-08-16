/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "ScopedArgumentsTable.h"

#include "JSCInlines.h"

namespace JSC {

const ClassInfo ScopedArgumentsTable::s_info = { "ScopedArgumentsTable", 0, 0, CREATE_METHOD_TABLE(ScopedArgumentsTable) };

ScopedArgumentsTable::ScopedArgumentsTable(VM& vm)
    : Base(vm, vm.scopedArgumentsTableStructure.get())
    , m_length(0)
    , m_locked(false)
{
}

ScopedArgumentsTable::~ScopedArgumentsTable()
{
}

void ScopedArgumentsTable::destroy(JSCell* cell)
{
    static_cast<ScopedArgumentsTable*>(cell)->ScopedArgumentsTable::~ScopedArgumentsTable();
}

ScopedArgumentsTable* ScopedArgumentsTable::create(VM& vm)
{
    ScopedArgumentsTable* result =
        new (NotNull, allocateCell<ScopedArgumentsTable>(vm.heap)) ScopedArgumentsTable(vm);
    result->finishCreation(vm);
    return result;
}

ScopedArgumentsTable* ScopedArgumentsTable::create(VM& vm, uint32_t length)
{
    ScopedArgumentsTable* result = create(vm);
    result->m_length = length;
    result->m_arguments = std::make_unique<ScopeOffset[]>(length);
    return result;
}

ScopedArgumentsTable* ScopedArgumentsTable::clone(VM& vm)
{
    ScopedArgumentsTable* result = create(vm, m_length);
    for (unsigned i = m_length; i--;)
        result->m_arguments[i] = m_arguments[i];
    return result;
}

ScopedArgumentsTable* ScopedArgumentsTable::setLength(VM& vm, uint32_t newLength)
{
    if (LIKELY(!m_locked)) {
        std::unique_ptr<ScopeOffset[]> newArguments = std::make_unique<ScopeOffset[]>(newLength);
        for (unsigned i = std::min(m_length, newLength); i--;)
            newArguments[i] = m_arguments[i];
        m_length = newLength;
        m_arguments = WTFMove(newArguments);
        return this;
    }
    
    ScopedArgumentsTable* result = create(vm, newLength);
    for (unsigned i = std::min(m_length, newLength); i--;)
        result->m_arguments[i] = m_arguments[i];
    return result;
}

ScopedArgumentsTable* ScopedArgumentsTable::set(VM& vm, uint32_t i, ScopeOffset value)
{
    ScopedArgumentsTable* result;
    if (UNLIKELY(m_locked))
        result = clone(vm);
    else
        result = this;
    result->at(i) = value;
    return result;
}

Structure* ScopedArgumentsTable::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
}

} // namespace JSC

