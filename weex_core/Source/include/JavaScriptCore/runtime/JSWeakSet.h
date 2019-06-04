/*
 * Copyright (C) 2015 Apple, Inc. All rights reserved.
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

#include "JSObject.h"

namespace JSC {

class WeakMapData;

class JSWeakSet : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;

    DECLARE_EXPORT_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    static JSWeakSet* create(VM& vm, Structure* structure)
    {
        JSWeakSet* instance = new (NotNull, allocateCell<JSWeakSet>(vm.heap)) JSWeakSet(vm, structure);
        instance->finishCreation(vm);
        return instance;
    }

    static JSWeakSet* create(ExecState* exec, Structure* structure)
    {
        return create(exec->vm(), structure);
    }

    WeakMapData* weakMapData() { return m_weakMapData.get(); }

    JSValue get(CallFrame*, JSObject*);
    bool has(CallFrame*, JSObject*);
    bool remove(CallFrame*, JSObject*);

    void set(CallFrame*, JSObject*, JSValue);
    void clear(CallFrame*);

private:
    JSWeakSet(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    void finishCreation(VM&);
    static void visitChildren(JSCell*, SlotVisitor&);
    static String toStringName(const JSObject*, ExecState*);

    WriteBarrier<WeakMapData> m_weakMapData;
};

} // namespace JSC
