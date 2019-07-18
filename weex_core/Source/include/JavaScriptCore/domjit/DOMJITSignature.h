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

#include "ClassInfo.h"
#include "DOMJITEffect.h"
#include "SpeculatedType.h"

namespace JSC { namespace DOMJIT {

// FIXME: Currently, we only support functions which arguments are up to 2.
// Eventually, we should extend this. But possibly, 2 or 3 can cover typical use cases.
// https://bugs.webkit.org/show_bug.cgi?id=164346
#define JSC_DOMJIT_SIGNATURE_MAX_ARGUMENTS 2
#define JSC_DOMJIT_SIGNATURE_MAX_ARGUMENTS_INCLUDING_THIS (1 + JSC_DOMJIT_SIGNATURE_MAX_ARGUMENTS)

class Patchpoint;

typedef Ref<Patchpoint> CheckDOMGeneratorFunction(void);

class Signature {
public:
    template<typename... Arguments>
    constexpr Signature(uintptr_t unsafeFunction, CheckDOMGeneratorFunction* checkDOMGeneratorFunction, const ClassInfo* classInfo, Effect effect, SpeculatedType result, Arguments... arguments)
        : unsafeFunction(unsafeFunction)
        , checkDOMGeneratorFunction(checkDOMGeneratorFunction)
        , classInfo(classInfo)
        , effect(effect)
        , result(result)
        , arguments {static_cast<SpeculatedType>(arguments)...}
        , argumentCount(sizeof...(Arguments))
    {
    }

#if ENABLE(JIT)
    Ref<Patchpoint> checkDOM() const
    {
        return checkDOMGeneratorFunction();
    }
#endif

    uintptr_t unsafeFunction;
    CheckDOMGeneratorFunction* checkDOMGeneratorFunction;
    const ClassInfo* const classInfo;
    const Effect effect;
    const SpeculatedType result;
    const SpeculatedType arguments[JSC_DOMJIT_SIGNATURE_MAX_ARGUMENTS];
    const unsigned argumentCount;
};

} }
