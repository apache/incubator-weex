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

#pragma once

#include "JSSegmentedVariableObject.h"

namespace JSC {

class JSGlobalLexicalEnvironment : public JSSegmentedVariableObject {

public:
    typedef JSSegmentedVariableObject Base;

    static const unsigned StructureFlags = Base::StructureFlags | OverridesGetOwnPropertySlot | OverridesToThis;

    static JSGlobalLexicalEnvironment* create(VM& vm, Structure* structure, JSScope* parentScope)
    {
        JSGlobalLexicalEnvironment* result =
            new (NotNull, allocateCell<JSGlobalLexicalEnvironment>(vm.heap)) JSGlobalLexicalEnvironment(vm, structure, parentScope);
        result->finishCreation(vm);
        result->symbolTable()->setScopeType(SymbolTable::ScopeType::GlobalLexicalScope);
        return result;
    }

    static bool getOwnPropertySlot(JSObject*, ExecState*, PropertyName, PropertySlot&);
    static bool put(JSCell*, ExecState*, PropertyName, JSValue, PutPropertySlot&);

    static void destroy(JSCell*);
    // We don't need a destructor because we use a finalizer instead.
    static const bool needsDestruction = false;

    bool isEmpty() const { return !symbolTable()->size(); }
    bool isConstVariable(UniquedStringImpl*);

    static JSValue toThis(JSCell*, ExecState*, ECMAMode);
    
    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject)
    {
        return Structure::create(vm, globalObject, jsNull(), TypeInfo(GlobalLexicalEnvironmentType, StructureFlags), info());
    }

protected:
    JSGlobalLexicalEnvironment(VM& vm, Structure* structure, JSScope* scope)
        : Base(vm, structure, scope)
    {
    }
};

} // namespace JSC
