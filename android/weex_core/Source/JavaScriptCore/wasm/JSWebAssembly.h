/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#if ENABLE(WEBASSEMBLY)

#include "JSObject.h"
#include "js/JSWebAssemblyCallee.h"
#include "js/JSWebAssemblyCompileError.h"
#include "js/JSWebAssemblyInstance.h"
#include "js/JSWebAssemblyLinkError.h"
#include "js/JSWebAssemblyMemory.h"
#include "js/JSWebAssemblyModule.h"
#include "js/JSWebAssemblyRuntimeError.h"
#include "js/JSWebAssemblyTable.h"
#include "js/WebAssemblyCompileErrorConstructor.h"
#include "js/WebAssemblyCompileErrorPrototype.h"
#include "js/WebAssemblyFunction.h"
#include "js/WebAssemblyInstanceConstructor.h"
#include "js/WebAssemblyInstancePrototype.h"
#include "js/WebAssemblyLinkErrorConstructor.h"
#include "js/WebAssemblyLinkErrorPrototype.h"
#include "js/WebAssemblyMemoryConstructor.h"
#include "js/WebAssemblyMemoryPrototype.h"
#include "js/WebAssemblyModuleConstructor.h"
#include "js/WebAssemblyModulePrototype.h"
#include "js/WebAssemblyModuleRecord.h"
#include "js/WebAssemblyPrototype.h"
#include "js/WebAssemblyRuntimeErrorConstructor.h"
#include "js/WebAssemblyRuntimeErrorPrototype.h"
#include "js/WebAssemblyTableConstructor.h"
#include "js/WebAssemblyTablePrototype.h"
#include "js/WebAssemblyToJSCallee.h"

namespace JSC {

class JSWebAssembly : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;

    static JSWebAssembly* create(VM&, JSGlobalObject*, Structure*);
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue);

    DECLARE_INFO;

protected:
    void finishCreation(VM&, JSGlobalObject*);

private:
    JSWebAssembly(VM&, Structure*);
};

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
