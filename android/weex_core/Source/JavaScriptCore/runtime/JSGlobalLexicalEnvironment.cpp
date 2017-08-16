/*
 * Copyright (C) 2015 Apple Inc. All Rights Reserved.
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
#include "JSGlobalLexicalEnvironment.h"

#include "JSCInlines.h"

namespace JSC {

const ClassInfo JSGlobalLexicalEnvironment::s_info = { "JSGlobalLexicalEnvironment", &Base::s_info, 0, CREATE_METHOD_TABLE(JSGlobalLexicalEnvironment) };

void JSGlobalLexicalEnvironment::destroy(JSCell* cell)
{
    static_cast<JSGlobalLexicalEnvironment*>(cell)->JSGlobalLexicalEnvironment::~JSGlobalLexicalEnvironment();
}

bool JSGlobalLexicalEnvironment::getOwnPropertySlot(JSObject* object, ExecState*, PropertyName propertyName, PropertySlot& slot)
{
    JSGlobalLexicalEnvironment* thisObject = jsCast<JSGlobalLexicalEnvironment*>(object);
    return symbolTableGet(thisObject, propertyName, slot);
}

bool JSGlobalLexicalEnvironment::put(JSCell* cell, ExecState* exec, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    JSGlobalLexicalEnvironment* thisObject = jsCast<JSGlobalLexicalEnvironment*>(cell);
    ASSERT(!Heap::heap(value) || Heap::heap(value) == Heap::heap(thisObject));
    bool alwaysThrowWhenAssigningToConstProperty = true;
    bool ignoreConstAssignmentError = slot.isInitialization();
    bool putResult = false;
    symbolTablePutTouchWatchpointSet(thisObject, exec, propertyName, value, alwaysThrowWhenAssigningToConstProperty, ignoreConstAssignmentError, putResult);
    return putResult;
}

bool JSGlobalLexicalEnvironment::isConstVariable(UniquedStringImpl* impl)
{
    SymbolTableEntry entry = symbolTable()->get(impl);
    ASSERT(!entry.isNull());
    return entry.isReadOnly();
}

JSValue JSGlobalLexicalEnvironment::toThis(JSCell*, ExecState* exec, ECMAMode ecmaMode)
{
    if (ecmaMode == StrictMode)
        return jsUndefined();
    return exec->globalThisValue();
}

} // namespace JSC
