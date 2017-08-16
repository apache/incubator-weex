/*
 * Copyright (C) 2007, 2008, 2012, 2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "JSEnvironmentRecord.h"

#include "HeapSnapshotBuilder.h"
#include "JSCInlines.h"

namespace JSC {

const ClassInfo JSEnvironmentRecord::s_info = { "EnvironmentRecord", &Base::s_info, 0, CREATE_METHOD_TABLE(JSEnvironmentRecord) };

void JSEnvironmentRecord::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSEnvironmentRecord* thisObject = jsCast<JSEnvironmentRecord*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);
    visitor.appendValuesHidden(thisObject->variables(), thisObject->symbolTable()->scopeSize());
}

void JSEnvironmentRecord::heapSnapshot(JSCell* cell, HeapSnapshotBuilder& builder)
{
    JSEnvironmentRecord* thisObject = jsCast<JSEnvironmentRecord*>(cell);
    Base::heapSnapshot(cell, builder);

    ConcurrentJSLocker locker(thisObject->symbolTable()->m_lock);
    SymbolTable::Map::iterator end = thisObject->symbolTable()->end(locker);
    for (SymbolTable::Map::iterator it = thisObject->symbolTable()->begin(locker); it != end; ++it) {
        SymbolTableEntry::Fast entry = it->value;
        ASSERT(!entry.isNull());
        ScopeOffset offset = entry.scopeOffset();
        if (!thisObject->isValidScopeOffset(offset))
            continue;

        JSValue toValue = thisObject->variableAt(offset).get();
        if (toValue && toValue.isCell())
            builder.appendVariableNameEdge(thisObject, toValue.asCell(), it->key.get());
    }
}

} // namespace JSC
