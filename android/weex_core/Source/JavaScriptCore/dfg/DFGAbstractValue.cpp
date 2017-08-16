/*
 * Copyright (C) 2013-2016 Apple Inc. All rights reserved.
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
#include "DFGAbstractValue.h"

#if ENABLE(DFG_JIT)

#include "DFGGraph.h"
#include "JSCInlines.h"
#include "TrackedReferences.h"

namespace JSC { namespace DFG {

void AbstractValue::observeTransitions(const TransitionVector& vector)
{
    if (m_type & SpecCell) {
        m_structure.observeTransitions(vector);
        ArrayModes newModes = 0;
        for (unsigned i = vector.size(); i--;) {
            if (m_arrayModes & asArrayModes(vector[i].previous->indexingType()))
                newModes |= asArrayModes(vector[i].next->indexingType());
        }
        m_arrayModes |= newModes;
    }
    checkConsistency();
}

void AbstractValue::set(Graph& graph, const FrozenValue& value, StructureClobberState clobberState)
{
    if (!!value && value.value().isCell()) {
        Structure* structure = value.structure();
        StructureRegistrationResult result;
        RegisteredStructure RegisteredStructure = graph.registerStructure(structure, result);
        if (result == StructureRegisteredAndWatched) {
            m_structure = RegisteredStructure;
            if (clobberState == StructuresAreClobbered) {
                m_arrayModes = ALL_ARRAY_MODES;
                m_structure.clobber();
            } else
                m_arrayModes = asArrayModes(structure->indexingType());
        } else {
            m_structure.makeTop();
            m_arrayModes = ALL_ARRAY_MODES;
        }
    } else {
        m_structure.clear();
        m_arrayModes = 0;
    }
    
    m_type = speculationFromValue(value.value());
    m_value = value.value();
    
    checkConsistency();
    assertIsRegistered(graph);
}

void AbstractValue::set(Graph& graph, Structure* structure)
{
    set(graph, graph.registerStructure(structure));
}

void AbstractValue::set(Graph& graph, RegisteredStructure structure)
{
    RELEASE_ASSERT(structure);
    
    m_structure = structure;
    m_arrayModes = asArrayModes(structure->indexingType());
    m_type = speculationFromStructure(structure.get());
    m_value = JSValue();
    
    checkConsistency();
    assertIsRegistered(graph);
}

void AbstractValue::set(Graph& graph, const RegisteredStructureSet& set)
{
    m_structure = set;
    m_arrayModes = set.arrayModesFromStructures();
    m_type = set.speculationFromStructures();
    m_value = JSValue();
    
    checkConsistency();
    assertIsRegistered(graph);
}

void AbstractValue::setType(Graph& graph, SpeculatedType type)
{
    SpeculatedType cellType = type & SpecCell;
    if (cellType) {
        if (!(cellType & ~SpecString))
            m_structure = graph.stringStructure;
        else if (isSymbolSpeculation(cellType))
            m_structure = graph.symbolStructure;
        else
            m_structure.makeTop();
        m_arrayModes = ALL_ARRAY_MODES;
    } else {
        m_structure.clear();
        m_arrayModes = 0;
    }
    m_type = type;
    m_value = JSValue();
    checkConsistency();
}

void AbstractValue::set(Graph& graph, const InferredType::Descriptor& descriptor)
{
    switch (descriptor.kind()) {
    case InferredType::Bottom:
        clear();
        return;
    case InferredType::Boolean:
        setType(SpecBoolean);
        return;
    case InferredType::Other:
        setType(SpecOther);
        return;
    case InferredType::Int32:
        setType(SpecInt32Only);
        return;
    case InferredType::Number:
        setType(SpecBytecodeNumber);
        return;
    case InferredType::String:
        set(graph, graph.m_vm.stringStructure.get());
        return;
    case InferredType::Symbol:
        set(graph, graph.m_vm.symbolStructure.get());
        return;
    case InferredType::ObjectWithStructure:
        set(graph, descriptor.structure());
        return;
    case InferredType::ObjectWithStructureOrOther:
        set(graph, descriptor.structure());
        merge(SpecOther);
        return;
    case InferredType::Object:
        setType(graph, SpecObject);
        return;
    case InferredType::ObjectOrOther:
        setType(graph, SpecObject | SpecOther);
        return;
    case InferredType::Top:
        makeHeapTop();
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

void AbstractValue::set(
    Graph& graph, const InferredType::Descriptor& descriptor, StructureClobberState clobberState)
{
    set(graph, descriptor);
    if (clobberState == StructuresAreClobbered)
        clobberStructures();
}

void AbstractValue::fixTypeForRepresentation(Graph& graph, NodeFlags representation, Node* node)
{
    if (representation == NodeResultDouble) {
        if (m_value) {
            ASSERT(m_value.isNumber());
            if (m_value.isInt32())
                m_value = jsDoubleNumber(m_value.asNumber());
        }
        if (m_type & SpecAnyInt) {
            m_type &= ~SpecAnyInt;
            m_type |= SpecAnyIntAsDouble;
        }
        if (m_type & ~SpecFullDouble)
            DFG_CRASH(graph, node, toCString("Abstract value ", *this, " for double node has type outside SpecFullDouble.\n").data());
    } else if (representation == NodeResultInt52) {
        if (m_type & SpecAnyIntAsDouble) {
            m_type &= ~SpecAnyIntAsDouble;
            m_type |= SpecInt52Only;
        }
        if (m_type & ~SpecAnyInt)
            DFG_CRASH(graph, node, toCString("Abstract value ", *this, " for int52 node has type outside SpecAnyInt.\n").data());
    } else {
        if (m_type & SpecInt52Only) {
            m_type &= ~SpecInt52Only;
            m_type |= SpecAnyIntAsDouble;
        }
        if (m_type & ~SpecBytecodeTop)
            DFG_CRASH(graph, node, toCString("Abstract value ", *this, " for value node has type outside SpecBytecodeTop.\n").data());
    }
    
    checkConsistency();
}

void AbstractValue::fixTypeForRepresentation(Graph& graph, Node* node)
{
    fixTypeForRepresentation(graph, node->result(), node);
}

bool AbstractValue::mergeOSREntryValue(Graph& graph, JSValue value)
{
    AbstractValue oldMe = *this;
    
    if (isClear()) {
        FrozenValue* frozenValue = graph.freeze(value);
        if (frozenValue->pointsToHeap()) {
            m_structure = graph.registerStructure(frozenValue->structure());
            m_arrayModes = asArrayModes(frozenValue->structure()->indexingType());
        } else {
            m_structure.clear();
            m_arrayModes = 0;
        }
        
        m_type = speculationFromValue(value);
        m_value = value;
    } else {
        mergeSpeculation(m_type, speculationFromValue(value));
        if (!!value && value.isCell()) {
            RegisteredStructure structure = graph.registerStructure(value.asCell()->structure());
            mergeArrayModes(m_arrayModes, asArrayModes(structure->indexingType()));
            m_structure.merge(RegisteredStructureSet(structure));
        }
        if (m_value != value)
            m_value = JSValue();
    }
    
    checkConsistency();
    assertIsRegistered(graph);
    
    return oldMe != *this;
}

bool AbstractValue::isType(Graph& graph, const InferredType::Descriptor& inferredType) const
{
    AbstractValue typeValue;
    typeValue.set(graph, inferredType);

    AbstractValue mergedValue = *this;
    mergedValue.merge(typeValue);

    return mergedValue == typeValue;
}

FiltrationResult AbstractValue::filter(
    Graph& graph, const RegisteredStructureSet& other, SpeculatedType admittedTypes)
{
    ASSERT(!(admittedTypes & SpecCell));
    
    if (isClear())
        return FiltrationOK;
    
    // FIXME: This could be optimized for the common case of m_type not
    // having structures, array modes, or a specific value.
    // https://bugs.webkit.org/show_bug.cgi?id=109663
    
    m_type &= other.speculationFromStructures() | admittedTypes;
    m_arrayModes &= other.arrayModesFromStructures();
    m_structure.filter(other);
    
    // It's possible that prior to the above two statements we had (Foo, TOP), where
    // Foo is a SpeculatedType that is disjoint with the passed RegisteredStructureSet. In that
    // case, we will now have (None, [someStructure]). In general, we need to make
    // sure that new information gleaned from the SpeculatedType needs to be fed back
    // into the information gleaned from the RegisteredStructureSet.
    m_structure.filter(m_type);
    
    filterArrayModesByType();
    filterValueByType();
    return normalizeClarity(graph);
}

FiltrationResult AbstractValue::changeStructure(Graph& graph, const RegisteredStructureSet& other)
{
    m_type &= other.speculationFromStructures();
    m_arrayModes = other.arrayModesFromStructures();
    m_structure = other;
    
    filterValueByType();
    
    return normalizeClarity(graph);
}

FiltrationResult AbstractValue::filterArrayModes(ArrayModes arrayModes)
{
    ASSERT(arrayModes);
    
    if (isClear())
        return FiltrationOK;
    
    m_type &= SpecCell;
    m_arrayModes &= arrayModes;
    return normalizeClarity();
}

FiltrationResult AbstractValue::filterClassInfo(Graph& graph, const ClassInfo* classInfo)
{
    // FIXME: AI should track ClassInfo to leverage hierarchical class information.
    // https://bugs.webkit.org/show_bug.cgi?id=162989
    if (isClear())
        return FiltrationOK;

    m_type &= speculationFromClassInfo(classInfo);
    m_structure.filterClassInfo(classInfo);

    m_structure.filter(m_type);

    filterArrayModesByType();
    filterValueByType();
    return normalizeClarity(graph);
}

FiltrationResult AbstractValue::filter(SpeculatedType type)
{
    if ((m_type & type) == m_type)
        return FiltrationOK;
    
    // Fast path for the case that we don't even have a cell.
    if (!(m_type & SpecCell)) {
        m_type &= type;
        FiltrationResult result;
        if (m_type == SpecNone) {
            clear();
            result = Contradiction;
        } else
            result = FiltrationOK;
        checkConsistency();
        return result;
    }
    
    m_type &= type;
    
    // It's possible that prior to this filter() call we had, say, (Final, TOP), and
    // the passed type is Array. At this point we'll have (None, TOP). The best way
    // to ensure that the structure filtering does the right thing is to filter on
    // the new type (None) rather than the one passed (Array).
    m_structure.filter(m_type);
    filterArrayModesByType();
    filterValueByType();
    return normalizeClarity();
}

FiltrationResult AbstractValue::filterByValue(const FrozenValue& value)
{
    FiltrationResult result = filter(speculationFromValue(value.value()));
    if (m_type)
        m_value = value.value();
    return result;
}

bool AbstractValue::contains(RegisteredStructure structure) const
{
    return couldBeType(speculationFromStructure(structure.get()))
        && (m_arrayModes & arrayModeFromStructure(structure.get()))
        && m_structure.contains(structure);
}

FiltrationResult AbstractValue::filter(const AbstractValue& other)
{
    m_type &= other.m_type;
    m_structure.filter(other.m_structure);
    m_arrayModes &= other.m_arrayModes;

    m_structure.filter(m_type);
    filterArrayModesByType();
    filterValueByType();
    
    if (normalizeClarity() == Contradiction)
        return Contradiction;
    
    if (m_value == other.m_value)
        return FiltrationOK;
    
    // Neither of us are BOTTOM, so an empty value means TOP.
    if (!m_value) {
        // We previously didn't prove a value but now we have done so.
        m_value = other.m_value; 
        return FiltrationOK;
    }
    
    if (!other.m_value) {
        // We had proved a value but the other guy hadn't, so keep our proof.
        return FiltrationOK;
    }
    
    // We both proved there to be a specific value but they are different.
    clear();
    return Contradiction;
}

FiltrationResult AbstractValue::filter(Graph& graph, const InferredType::Descriptor& descriptor)
{
    AbstractValue filterValue;
    filterValue.set(graph, descriptor);
    return filter(filterValue);
}

void AbstractValue::filterValueByType()
{
    // We could go further, and ensure that if the futurePossibleStructure contravenes
    // the value, then we could clear both of those things. But that's unlikely to help
    // in any realistic scenario, so we don't do it. Simpler is better.

    if (!!m_type) {
        // The type is still non-empty. It may be that the new type renders
        // the value empty because it contravenes the constant value we had.
        if (m_value && !validateType(m_value))
            clear();
        return;
    }
    
    // The type has been rendered empty. That means that the value must now be invalid,
    // as well.
    ASSERT(!m_value || !validateType(m_value));
    m_value = JSValue();
}

void AbstractValue::filterArrayModesByType()
{
    if (!(m_type & SpecCell))
        m_arrayModes = 0;
    else if (!(m_type & ~SpecArray))
        m_arrayModes &= ALL_ARRAY_ARRAY_MODES;
    
    // NOTE: If m_type doesn't have SpecArray set, that doesn't mean that the
    // array modes have to be a subset of ALL_NON_ARRAY_ARRAY_MODES, since
    // in the speculated type type-system, RegExpMatchesArry and ArrayPrototype
    // are Otherobj (since they are not *exactly* JSArray) but in the ArrayModes
    // type system they are arrays (since they expose the magical length
    // property and are otherwise allocated using array allocation). Hence the
    // following would be wrong:
    //
    // if (!(m_type & SpecArray))
    //    m_arrayModes &= ALL_NON_ARRAY_ARRAY_MODES;
}

bool AbstractValue::shouldBeClear() const
{
    if (m_type == SpecNone)
        return true;
    
    if (!(m_type & ~SpecCell)
        && (!m_arrayModes || m_structure.isClear()))
        return true;
    
    return false;
}

FiltrationResult AbstractValue::normalizeClarity()
{
    // It's useful to be able to quickly check if an abstract value is clear.
    // This normalizes everything to make that easy.
    
    FiltrationResult result;
    
    if (shouldBeClear()) {
        clear();
        result = Contradiction;
    } else
        result = FiltrationOK;

    checkConsistency();
    
    return result;
}

FiltrationResult AbstractValue::normalizeClarity(Graph& graph)
{
    FiltrationResult result = normalizeClarity();
    assertIsRegistered(graph);
    return result;
}

#if !ASSERT_DISABLED
void AbstractValue::checkConsistency() const
{
    if (!(m_type & SpecCell)) {
        ASSERT(m_structure.isClear());
        ASSERT(!m_arrayModes);
    }
    
    if (isClear())
        ASSERT(!m_value);
    
    if (!!m_value) {
        SpeculatedType type = m_type;
        // This relaxes the assertion below a bit, since we don't know the representation of the
        // node.
        if (type & SpecInt52Only)
            type |= SpecAnyIntAsDouble;
        ASSERT(mergeSpeculations(type, speculationFromValue(m_value)) == type);
    }
    
    // Note that it's possible for a prediction like (Final, []). This really means that
    // the value is bottom and that any code that uses the value is unreachable. But
    // we don't want to get pedantic about this as it would only increase the computational
    // complexity of the code.
}

void AbstractValue::assertIsRegistered(Graph& graph) const
{
    m_structure.assertIsRegistered(graph);
}
#endif

ResultType AbstractValue::resultType() const
{
    ASSERT(isType(SpecBytecodeTop));
    if (isType(SpecBoolean))
        return ResultType::booleanType();
    if (isType(SpecInt32Only))
        return ResultType::numberTypeIsInt32();
    if (isType(SpecBytecodeNumber))
        return ResultType::numberType();
    if (isType(SpecString))
        return ResultType::stringType();
    if (isType(SpecString | SpecBytecodeNumber))
        return ResultType::stringOrNumberType();
    return ResultType::unknownType();
}

void AbstractValue::dump(PrintStream& out) const
{
    dumpInContext(out, 0);
}

void AbstractValue::dumpInContext(PrintStream& out, DumpContext* context) const
{
    out.print("(", SpeculationDump(m_type));
    if (m_type & SpecCell) {
        out.print(
            ", ", ArrayModesDump(m_arrayModes), ", ",
            inContext(m_structure, context));
    }
    if (!!m_value)
        out.print(", ", inContext(m_value, context));
    out.print(")");
}

void AbstractValue::validateReferences(const TrackedReferences& trackedReferences)
{
    trackedReferences.check(m_value);
    m_structure.validateReferences(trackedReferences);
}

#if USE(JSVALUE64) && !defined(NDEBUG)
void AbstractValue::ensureCanInitializeWithZeros()
{
    std::aligned_storage<sizeof(AbstractValue), alignof(AbstractValue)>::type zeroFilledStorage;
    memset(static_cast<void*>(&zeroFilledStorage), 0, sizeof(AbstractValue));
    ASSERT(*this == *static_cast<AbstractValue*>(static_cast<void*>(&zeroFilledStorage)));
}
#endif

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

