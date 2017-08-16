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

#include "DOMJITCallDOMGetterPatchpoint.h"
#include "PropertySlot.h"
#include "PutPropertySlot.h"
#include "SpeculatedType.h"

namespace JSC { namespace DOMJIT {

class GetterSetter {
public:
    typedef PropertySlot::GetValueFunc CustomGetter;
    typedef PutPropertySlot::PutValueFunc CustomSetter;

    GetterSetter(CustomGetter getter, CustomSetter setter, const ClassInfo* classInfo, SpeculatedType resultType)
        : m_getter(getter)
        , m_setter(setter)
        , m_thisClassInfo(classInfo)
        , m_resultType(resultType)
    {
    }

    virtual ~GetterSetter() { }

    CustomGetter getter() const { return m_getter; }
    CustomSetter setter() const { return m_setter; }
    const ClassInfo* thisClassInfo() const { return m_thisClassInfo; }
    SpeculatedType resultType() const { return m_resultType; }

#if ENABLE(JIT)
    virtual Ref<DOMJIT::CallDOMGetterPatchpoint> callDOMGetter() = 0;
    virtual Ref<DOMJIT::Patchpoint> checkDOM() = 0;
#endif

private:
    CustomGetter m_getter;
    CustomSetter m_setter;
    const ClassInfo* m_thisClassInfo;
    SpeculatedType m_resultType;
};

} }
