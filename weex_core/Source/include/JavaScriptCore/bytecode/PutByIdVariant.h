/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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
