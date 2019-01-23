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

#if ENABLE(DFG_JIT)

#include "DFGRegisteredStructure.h"
#include "StructureSet.h"
#include <wtf/TinyPtrSet.h>

namespace JSC {

class TrackedReferences;

namespace DFG {

struct AbstractValue;
class StructureAbstractValue;

class RegisteredStructureSet : public TinyPtrSet<RegisteredStructure> {
public:

    RegisteredStructureSet()
    { }
    
    RegisteredStructureSet(RegisteredStructure structure)
        : TinyPtrSet(structure)
    {
    }
    
    ALWAYS_INLINE RegisteredStructureSet(const RegisteredStructureSet& other)
        : TinyPtrSet(other)
    {
    }
    
    RegisteredStructure onlyStructure() const
    {
        return onlyEntry();
    }

    StructureSet toStructureSet() const
    {
        StructureSet result;
        forEach([&] (RegisteredStructure structure) { result.add(structure.get()); });
        return result;
    }

    void filter(const DFG::StructureAbstractValue&);
    void filter(SpeculatedType);
    void filterArrayModes(ArrayModes);
    void filter(const DFG::AbstractValue&);
    
    SpeculatedType speculationFromStructures() const;
    ArrayModes arrayModesFromStructures() const;

    void validateReferences(const TrackedReferences&) const;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
