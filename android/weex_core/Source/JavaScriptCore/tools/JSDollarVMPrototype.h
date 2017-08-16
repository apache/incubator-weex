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

#pragma once

#include "JSObject.h"

namespace JSC {

class Heap;

class JSDollarVMPrototype : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;

    DECLARE_INFO;

    static JSDollarVMPrototype* create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
    {
        JSDollarVMPrototype* prototype = new (NotNull, allocateCell<JSDollarVMPrototype>(vm.heap)) JSDollarVMPrototype(vm, structure);
        prototype->finishCreation(vm, globalObject);
        return prototype;
    }
    
    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

    // The following are exported because they are designed to be callable from
    // lldb. The JS versions are implemented on top of these.
    
    JS_EXPORT_PRIVATE static bool currentThreadOwnsJSLock(ExecState*);
    JS_EXPORT_PRIVATE static void gc(ExecState*);
    JS_EXPORT_PRIVATE static void edenGC(ExecState*);
    JS_EXPORT_PRIVATE static bool isInHeap(Heap*, void*);
    JS_EXPORT_PRIVATE static bool isInObjectSpace(Heap*, void*);
    JS_EXPORT_PRIVATE static bool isInStorageSpace(Heap*, void*);
    JS_EXPORT_PRIVATE static bool isValidCell(Heap*, JSCell*);
    JS_EXPORT_PRIVATE static bool isValidCodeBlock(ExecState*, CodeBlock*);
    JS_EXPORT_PRIVATE static CodeBlock* codeBlockForFrame(CallFrame* topCallFrame, unsigned frameNumber);
    JS_EXPORT_PRIVATE static void printCallFrame(CallFrame*);
    JS_EXPORT_PRIVATE static void printStack(CallFrame* topCallFrame);
    JS_EXPORT_PRIVATE static void printValue(JSValue);

private:
    JSDollarVMPrototype(VM& vm, Structure* structure)
        : Base(vm, structure)
    {
    }

    void finishCreation(VM&, JSGlobalObject*);
    void addFunction(VM&, JSGlobalObject*, const char* name, NativeFunction, unsigned arguments);
};

} // namespace JSC
