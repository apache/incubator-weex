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

#include "JSFunction.h"

namespace JSC {

class JSGlobalObject;
class NativeStdFunctionCell;

typedef std::function<EncodedJSValue (ExecState*)> NativeStdFunction;

class JSNativeStdFunction : public JSFunction {
public:
    typedef JSFunction Base;

    const static unsigned StructureFlags = Base::StructureFlags;

    DECLARE_EXPORT_INFO;

    JS_EXPORT_PRIVATE static JSNativeStdFunction* create(VM&, JSGlobalObject*, int length, const String& name, NativeStdFunction&&, Intrinsic = NoIntrinsic, NativeFunction nativeConstructor = callHostFunctionAsConstructor);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        ASSERT(globalObject);
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSFunctionType, StructureFlags), info());
    }

    NativeStdFunctionCell* nativeStdFunctionCell() { return m_functionCell.get(); }

protected:
    static void visitChildren(JSCell*, SlotVisitor&);

    void finishCreation(VM&, NativeExecutable*, int length, const String& name, NativeStdFunctionCell*);

private:
    JSNativeStdFunction(VM&, JSGlobalObject*, Structure*);

    WriteBarrier<NativeStdFunctionCell> m_functionCell;
};

} // namespace JSC
