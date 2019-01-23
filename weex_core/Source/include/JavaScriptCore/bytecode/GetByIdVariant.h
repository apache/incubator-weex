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

#include "CallLinkStatus.h"
#include "JSCJSValue.h"
#include "ObjectPropertyConditionSet.h"
#include "PropertyOffset.h"
#include "StructureSet.h"

namespace JSC {
namespace DOMJIT {
class GetterSetter;
}

class CallLinkStatus;
class GetByIdStatus;
struct DumpContext;

class GetByIdVariant {
public:
    GetByIdVariant(
        const StructureSet& structureSet = StructureSet(), PropertyOffset offset = invalidOffset,
        const ObjectPropertyConditionSet& = ObjectPropertyConditionSet(),
        std::unique_ptr<CallLinkStatus> = nullptr,
        JSFunction* = nullptr,
        DOMJIT::GetterSetter* = nullptr);

    ~GetByIdVariant();
    
    GetByIdVariant(const GetByIdVariant&);
    GetByIdVariant& operator=(const GetByIdVariant&);
    
    bool isSet() const { return !!m_structureSet.size(); }
    bool operator!() const { return !isSet(); }
    const StructureSet& structureSet() const { return m_structureSet; }
    StructureSet& structureSet() { return m_structureSet; }

    // A non-empty condition set means that this is a prototype load.
    const ObjectPropertyConditionSet& conditionSet() const { return m_conditionSet; }
    
    PropertyOffset offset() const { return m_offset; }
    CallLinkStatus* callLinkStatus() const { return m_callLinkStatus.get(); }
    JSFunction* intrinsicFunction() const { return m_intrinsicFunction; }
    Intrinsic intrinsic() const { return m_intrinsicFunction ? m_intrinsicFunction->intrinsic() : NoIntrinsic; }
    DOMJIT::GetterSetter* domJIT() const { return m_domJIT; }

    bool isPropertyUnset() const { return offset() == invalidOffset; }

    bool attemptToMerge(const GetByIdVariant& other);
    
    void dump(PrintStream&) const;
    void dumpInContext(PrintStream&, DumpContext*) const;
    
private:
    friend class GetByIdStatus;

    bool canMergeIntrinsicStructures(const GetByIdVariant&) const;
    
    StructureSet m_structureSet;
    ObjectPropertyConditionSet m_conditionSet;
    PropertyOffset m_offset;
    std::unique_ptr<CallLinkStatus> m_callLinkStatus;
    JSFunction* m_intrinsicFunction;
    DOMJIT::GetterSetter* m_domJIT;
};

} // namespace JSC
