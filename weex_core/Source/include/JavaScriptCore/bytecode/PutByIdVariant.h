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

#include "ObjectPropertyConditionSet.h"
#include "PropertyOffset.h"
#include "StructureSet.h"

namespace JSC {

class CallLinkStatus;

class PutByIdVariant {
public:
    enum Kind {
        NotSet,
        Replace,
        Transition,
        Setter
    };
    
    PutByIdVariant()
        : m_kind(NotSet)
        , m_newStructure(nullptr)
        , m_offset(invalidOffset)
    {
    }
    
    PutByIdVariant(const PutByIdVariant&);
    PutByIdVariant& operator=(const PutByIdVariant&);

    static PutByIdVariant replace(const StructureSet&, PropertyOffset, const InferredType::Descriptor&);
    
    static PutByIdVariant transition(
        const StructureSet& oldStructure, Structure* newStructure,
        const ObjectPropertyConditionSet&, PropertyOffset, const InferredType::Descriptor&);
    
    static PutByIdVariant setter(
        const StructureSet&, PropertyOffset, const ObjectPropertyConditionSet&,
        std::unique_ptr<CallLinkStatus>);
    
    Kind kind() const { return m_kind; }
    
    bool isSet() const { return kind() != NotSet; }
    bool operator!() const { return !isSet(); }
    
    const StructureSet& structure() const
    {
        ASSERT(kind() == Replace || kind() == Setter);
        return m_oldStructure;
    }
    
    const StructureSet& structureSet() const
    {
        return structure();
    }
    
    const StructureSet& oldStructure() const
    {
        ASSERT(kind() == Transition || kind() == Replace || kind() == Setter);
        return m_oldStructure;
    }
    
    StructureSet& oldStructure()
    {
        ASSERT(kind() == Transition || kind() == Replace || kind() == Setter);
        return m_oldStructure;
    }
    
    Structure* oldStructureForTransition() const;
    
    Structure* newStructure() const
    {
        ASSERT(kind() == Transition);
        return m_newStructure;
    }

    InferredType::Descriptor requiredType() const
    {
        return m_requiredType;
    }

    bool writesStructures() const;
    bool reallocatesStorage() const;
    bool makesCalls() const;
    
    const ObjectPropertyConditionSet& conditionSet() const { return m_conditionSet; }
    
    // We don't support intrinsics for Setters (it would be sweet if we did) but we need this for templated helpers.
    Intrinsic intrinsic() const { return NoIntrinsic; }

    // This is needed for templated helpers.
    bool isPropertyUnset() const { return false; }

    PropertyOffset offset() const
    {
        ASSERT(isSet());
        return m_offset;
    }
    
    CallLinkStatus* callLinkStatus() const
    {
        ASSERT(kind() == Setter);
        return m_callLinkStatus.get();
    }

    bool attemptToMerge(const PutByIdVariant& other);
    
    void dump(PrintStream&) const;
    void dumpInContext(PrintStream&, DumpContext*) const;

private:
    bool attemptToMergeTransitionWithReplace(const PutByIdVariant& replace);
    
    Kind m_kind;
    StructureSet m_oldStructure;
    Structure* m_newStructure;
    ObjectPropertyConditionSet m_conditionSet;
    PropertyOffset m_offset;
    InferredType::Descriptor m_requiredType;
    std::unique_ptr<CallLinkStatus> m_callLinkStatus;
};

} // namespace JSC
