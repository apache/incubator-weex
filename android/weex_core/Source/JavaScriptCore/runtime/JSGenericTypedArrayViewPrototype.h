/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

template<typename ViewClass>
class JSGenericTypedArrayViewPrototype : public JSNonFinalObject {
public:
    typedef JSNonFinalObject Base;

protected:
    JSGenericTypedArrayViewPrototype(VM&, Structure*);
    void finishCreation(VM&, JSGlobalObject*);

public:
    static JSGenericTypedArrayViewPrototype* create(
        VM&, JSGlobalObject*, Structure*);

    // FIXME: We should fix the warnings for extern-template in JSObject template classes: https://bugs.webkit.org/show_bug.cgi?id=161979
#if COMPILER(CLANG)
#if __has_warning("-Wundefined-var-template")
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif
#endif
    DECLARE_INFO;
#if COMPILER(CLANG)
#if __has_warning("-Wundefined-var-template")
#pragma clang diagnostic pop
#endif
#endif
    
    static Structure* createStructure(VM&, JSGlobalObject*, JSValue prototype);
};

} // namespace JSC
