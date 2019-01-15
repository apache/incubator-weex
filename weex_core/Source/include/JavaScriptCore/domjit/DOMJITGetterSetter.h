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
