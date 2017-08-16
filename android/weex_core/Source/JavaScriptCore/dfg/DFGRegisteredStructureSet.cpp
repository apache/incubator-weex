/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "config.h"
#include "DFGRegisteredStructureSet.h"

#if ENABLE(DFG_JIT)

#include "DFGAbstractValue.h"
#include "TrackedReferences.h"

namespace JSC { namespace DFG {

void RegisteredStructureSet::filter(const DFG::StructureAbstractValue& other)
{
    genericFilter(
        [&] (RegisteredStructure structure) -> bool {
            return other.contains(structure); 
        });
}

void RegisteredStructureSet::filter(SpeculatedType type)
{
    genericFilter(
        [&] (RegisteredStructure structure) -> bool {
            return type & speculationFromStructure(structure.get());
        });
}

void RegisteredStructureSet::filterArrayModes(ArrayModes arrayModes)
{
    genericFilter(
        [&] (RegisteredStructure structure) -> bool {
            return arrayModes & arrayModeFromStructure(structure.get());
        });
}

void RegisteredStructureSet::filter(const DFG::AbstractValue& other)
{
    filter(other.m_structure);
    filter(other.m_type);
    filterArrayModes(other.m_arrayModes);
}

SpeculatedType RegisteredStructureSet::speculationFromStructures() const
{
    SpeculatedType result = SpecNone;
    forEach(
        [&] (RegisteredStructure structure) {
            mergeSpeculation(result, speculationFromStructure(structure.get()));
        });
    return result;
}

ArrayModes RegisteredStructureSet::arrayModesFromStructures() const
{
    ArrayModes result = 0;
    forEach(
        [&] (RegisteredStructure structure) {
            mergeArrayModes(result, asArrayModes(structure->indexingType()));
        });
    return result;
}

void RegisteredStructureSet::validateReferences(const TrackedReferences& trackedReferences) const
{
    // The type system should help us here, but protect people from getting that wrong using bitwise_cast or something crazy.
    forEach(
        [&] (RegisteredStructure structure) {
            trackedReferences.check(structure.get());
        });
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
