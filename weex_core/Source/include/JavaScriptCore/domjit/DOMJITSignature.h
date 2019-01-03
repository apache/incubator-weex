/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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
