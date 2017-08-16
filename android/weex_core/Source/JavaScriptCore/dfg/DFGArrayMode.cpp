/*
 * Copyright (C) 2012-2016 Apple Inc. All rights reserved.
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
#include "DFGArrayMode.h"

#if ENABLE(DFG_JIT)

#include "ArrayPrototype.h"
#include "DFGAbstractValue.h"
#include "DFGGraph.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

ArrayMode ArrayMode::fromObserved(const ConcurrentJSLocker& locker, ArrayProfile* profile, Array::Action action, bool makeSafe)
{
    Array::Class nonArray;
    if (profile->usesOriginalArrayStructures(locker))
        nonArray = Array::OriginalNonArray;
    else
        nonArray = Array::NonArray;
    
    ArrayModes observed = profile->observedArrayModes(locker);
    switch (observed) {
    case 0:
        return ArrayMode(Array::Unprofiled);
    case asArrayModes(NonArray):
        if (action == Array::Write && !profile->mayInterceptIndexedAccesses(locker))
            return ArrayMode(Array::SelectUsingArguments, nonArray, Array::OutOfBounds, Array::Convert);
        return ArrayMode(Array::SelectUsingPredictions, nonArray).withSpeculationFromProfile(locker, profile, makeSafe);

    case asArrayModes(ArrayWithUndecided):
        if (action == Array::Write)
            return ArrayMode(Array::SelectUsingArguments, Array::Array, Array::OutOfBounds, Array::Convert);
        return ArrayMode(Array::Undecided, Array::Array, Array::OutOfBounds, Array::AsIs).withProfile(locker, profile, makeSafe);
        
    case asArrayModes(NonArray) | asArrayModes(ArrayWithUndecided):
        if (action == Array::Write && !profile->mayInterceptIndexedAccesses(locker))
            return ArrayMode(Array::SelectUsingArguments, Array::PossiblyArray, Array::OutOfBounds, Array::Convert);
        return ArrayMode(Array::SelectUsingPredictions).withSpeculationFromProfile(locker, profile, makeSafe);

    case asArrayModes(NonArrayWithInt32):
        return ArrayMode(Array::Int32, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case asArrayModes(ArrayWithInt32):
        return ArrayMode(Array::Int32, Array::Array, Array::AsIs).withProfile(locker, profile, makeSafe);
    case asArrayModes(NonArrayWithInt32) | asArrayModes(ArrayWithInt32):
        return ArrayMode(Array::Int32, Array::PossiblyArray, Array::AsIs).withProfile(locker, profile, makeSafe);

    case asArrayModes(NonArrayWithDouble):
        return ArrayMode(Array::Double, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case asArrayModes(ArrayWithDouble):
        return ArrayMode(Array::Double, Array::Array, Array::AsIs).withProfile(locker, profile, makeSafe);
    case asArrayModes(NonArrayWithDouble) | asArrayModes(ArrayWithDouble):
        return ArrayMode(Array::Double, Array::PossiblyArray, Array::AsIs).withProfile(locker, profile, makeSafe);

    case asArrayModes(NonArrayWithContiguous):
        return ArrayMode(Array::Contiguous, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case asArrayModes(ArrayWithContiguous):
        return ArrayMode(Array::Contiguous, Array::Array, Array::AsIs).withProfile(locker, profile, makeSafe);
    case asArrayModes(NonArrayWithContiguous) | asArrayModes(ArrayWithContiguous):
        return ArrayMode(Array::Contiguous, Array::PossiblyArray, Array::AsIs).withProfile(locker, profile, makeSafe);

    case asArrayModes(NonArrayWithArrayStorage):
        return ArrayMode(Array::ArrayStorage, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case asArrayModes(NonArrayWithSlowPutArrayStorage):
    case asArrayModes(NonArrayWithArrayStorage) | asArrayModes(NonArrayWithSlowPutArrayStorage):
        return ArrayMode(Array::SlowPutArrayStorage, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case asArrayModes(ArrayWithArrayStorage):
        return ArrayMode(Array::ArrayStorage, Array::Array, Array::AsIs).withProfile(locker, profile, makeSafe);
    case asArrayModes(ArrayWithSlowPutArrayStorage):
    case asArrayModes(ArrayWithArrayStorage) | asArrayModes(ArrayWithSlowPutArrayStorage):
        return ArrayMode(Array::SlowPutArrayStorage, Array::Array, Array::AsIs).withProfile(locker, profile, makeSafe);
    case asArrayModes(NonArrayWithArrayStorage) | asArrayModes(ArrayWithArrayStorage):
        return ArrayMode(Array::ArrayStorage, Array::PossiblyArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case asArrayModes(NonArrayWithSlowPutArrayStorage) | asArrayModes(ArrayWithSlowPutArrayStorage):
    case asArrayModes(NonArrayWithArrayStorage) | asArrayModes(ArrayWithArrayStorage) | asArrayModes(NonArrayWithSlowPutArrayStorage) | asArrayModes(ArrayWithSlowPutArrayStorage):
        return ArrayMode(Array::SlowPutArrayStorage, Array::PossiblyArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case Int8ArrayMode:
        return ArrayMode(Array::Int8Array, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case Int16ArrayMode:
        return ArrayMode(Array::Int16Array, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case Int32ArrayMode:
        return ArrayMode(Array::Int32Array, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case Uint8ArrayMode:
        return ArrayMode(Array::Uint8Array, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case Uint8ClampedArrayMode:
        return ArrayMode(Array::Uint8ClampedArray, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case Uint16ArrayMode:
        return ArrayMode(Array::Uint16Array, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case Uint32ArrayMode:
        return ArrayMode(Array::Uint32Array, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case Float32ArrayMode:
        return ArrayMode(Array::Float32Array, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);
    case Float64ArrayMode:
        return ArrayMode(Array::Float64Array, nonArray, Array::AsIs).withProfile(locker, profile, makeSafe);

    default:
        if ((observed & asArrayModes(NonArray)) && profile->mayInterceptIndexedAccesses(locker))
            return ArrayMode(Array::SelectUsingPredictions).withSpeculationFromProfile(locker, profile, makeSafe);
        
        Array::Type type;
        Array::Class arrayClass;
        
        if (shouldUseSlowPutArrayStorage(observed))
            type = Array::SlowPutArrayStorage;
        else if (shouldUseFastArrayStorage(observed))
            type = Array::ArrayStorage;
        else if (shouldUseContiguous(observed))
            type = Array::Contiguous;
        else if (shouldUseDouble(observed))
            type = Array::Double;
        else if (shouldUseInt32(observed))
            type = Array::Int32;
        else
            type = Array::SelectUsingArguments;
        
        if (hasSeenArray(observed) && hasSeenNonArray(observed))
            arrayClass = Array::PossiblyArray;
        else if (hasSeenArray(observed))
            arrayClass = Array::Array;
        else if (hasSeenNonArray(observed))
            arrayClass = nonArray;
        else
            arrayClass = Array::PossiblyArray;
        
        return ArrayMode(type, arrayClass, Array::Convert).withProfile(locker, profile, makeSafe);
    }
}

static bool canBecomeGetArrayLength(Graph& graph, Node* node)
{
    if (node->op() != GetById)
        return false;
    auto uid = graph.identifiers()[node->identifierNumber()];
    return uid == graph.m_vm.propertyNames->length.impl();
}

ArrayMode ArrayMode::refine(
    Graph& graph, Node* node,
    SpeculatedType base, SpeculatedType index, SpeculatedType value) const
{
    if (!base || !index) {
        // It can be that we had a legitimate arrayMode but no incoming predictions. That'll
        // happen if we inlined code based on, say, a global variable watchpoint, but later
        // realized that the callsite could not have possibly executed. It may be worthwhile
        // to fix that, but for now I'm leaving it as-is.
        return ArrayMode(Array::ForceExit);
    }
    
    if (!isInt32Speculation(index))
        return ArrayMode(Array::Generic);
    
    // If we had exited because of an exotic object behavior, then don't try to specialize.
    if (graph.hasExitSite(node->origin.semantic, ExoticObjectMode))
        return ArrayMode(Array::Generic);
    
    // Note: our profiling currently doesn't give us good information in case we have
    // an unlikely control flow path that sets the base to a non-cell value. Value
    // profiling and prediction propagation will probably tell us that the value is
    // either a cell or not, but that doesn't tell us which is more likely: that this
    // is an array access on a cell (what we want and can optimize) or that the user is
    // doing a crazy by-val access on a primitive (we can't easily optimize this and
    // don't want to). So, for now, we assume that if the base is not a cell according
    // to value profiling, but the array profile tells us something else, then we
    // should just trust the array profile.
    
    switch (type()) {
    case Array::SelectUsingArguments:
        if (!value)
            return withType(Array::ForceExit);
        if (isInt32Speculation(value))
            return withTypeAndConversion(Array::Int32, Array::Convert);
        if (isFullNumberSpeculation(value))
            return withTypeAndConversion(Array::Double, Array::Convert);
        return withTypeAndConversion(Array::Contiguous, Array::Convert);
    case Array::Undecided: {
        // If we have an OriginalArray and the JSArray prototype chain is sane,
        // any indexed access always return undefined. We have a fast path for that.
        JSGlobalObject* globalObject = graph.globalObjectFor(node->origin.semantic);
        if ((node->op() == GetByVal || canBecomeGetArrayLength(graph, node))
            && arrayClass() == Array::OriginalArray
            && globalObject->arrayPrototypeChainIsSane()
            && !graph.hasExitSite(node->origin.semantic, OutOfBounds)) {
            graph.watchpoints().addLazily(globalObject->arrayPrototype()->structure()->transitionWatchpointSet());
            graph.watchpoints().addLazily(globalObject->objectPrototype()->structure()->transitionWatchpointSet());
            if (globalObject->arrayPrototypeChainIsSane())
                return withSpeculation(Array::SaneChain);
        }
        return ArrayMode(Array::Generic);
    }
    case Array::Int32:
        if (!value || isInt32Speculation(value))
            return *this;
        if (isFullNumberSpeculation(value))
            return withTypeAndConversion(Array::Double, Array::Convert);
        return withTypeAndConversion(Array::Contiguous, Array::Convert);
        
    case Array::Double:
        if (!value || isFullNumberSpeculation(value))
            return *this;
        return withTypeAndConversion(Array::Contiguous, Array::Convert);
        
    case Array::Contiguous:
        return *this;

    case Array::Int8Array:
    case Array::Int16Array:
    case Array::Int32Array:
    case Array::Uint8Array:
    case Array::Uint8ClampedArray:
    case Array::Uint16Array:
    case Array::Uint32Array:
    case Array::Float32Array:
    case Array::Float64Array:
        switch (node->op()) {
        case PutByVal:
            if (graph.hasExitSite(node->origin.semantic, OutOfBounds) || !isInBounds())
                return withSpeculation(Array::OutOfBounds);
            return withSpeculation(Array::InBounds);
        default:
            return withSpeculation(Array::InBounds);
        }
        return *this;
    case Array::Unprofiled:
    case Array::SelectUsingPredictions: {
        base &= ~SpecOther;
        
        if (isStringSpeculation(base))
            return withType(Array::String);
        
        if (isDirectArgumentsSpeculation(base) || isScopedArgumentsSpeculation(base)) {
            // Handle out-of-bounds accesses as generic accesses.
            if (graph.hasExitSite(node->origin.semantic, OutOfBounds) || !isInBounds())
                return ArrayMode(Array::Generic);
            
            if (isDirectArgumentsSpeculation(base))
                return withType(Array::DirectArguments);
            return withType(Array::ScopedArguments);
        }
        
        ArrayMode result;
        switch (node->op()) {
        case PutByVal:
            if (graph.hasExitSite(node->origin.semantic, OutOfBounds) || !isInBounds())
                result = withSpeculation(Array::OutOfBounds);
            else
                result = withSpeculation(Array::InBounds);
            break;
            
        default:
            result = withSpeculation(Array::InBounds);
            break;
        }
        
        if (isInt8ArraySpeculation(base))
            return result.withType(Array::Int8Array);
        
        if (isInt16ArraySpeculation(base))
            return result.withType(Array::Int16Array);
        
        if (isInt32ArraySpeculation(base))
            return result.withType(Array::Int32Array);
        
        if (isUint8ArraySpeculation(base))
            return result.withType(Array::Uint8Array);
        
        if (isUint8ClampedArraySpeculation(base))
            return result.withType(Array::Uint8ClampedArray);
        
        if (isUint16ArraySpeculation(base))
            return result.withType(Array::Uint16Array);
        
        if (isUint32ArraySpeculation(base))
            return result.withType(Array::Uint32Array);
        
        if (isFloat32ArraySpeculation(base))
            return result.withType(Array::Float32Array);
        
        if (isFloat64ArraySpeculation(base))
            return result.withType(Array::Float64Array);

        if (type() == Array::Unprofiled)
            return ArrayMode(Array::ForceExit);
        return ArrayMode(Array::Generic);
    }

    default:
        return *this;
    }
}

Structure* ArrayMode::originalArrayStructure(Graph& graph, const CodeOrigin& codeOrigin) const
{
    JSGlobalObject* globalObject = graph.globalObjectFor(codeOrigin);
    
    switch (arrayClass()) {
    case Array::OriginalArray: {
        switch (type()) {
        case Array::Int32:
            return globalObject->originalArrayStructureForIndexingType(ArrayWithInt32);
        case Array::Double:
            return globalObject->originalArrayStructureForIndexingType(ArrayWithDouble);
        case Array::Contiguous:
            return globalObject->originalArrayStructureForIndexingType(ArrayWithContiguous);
        case Array::Undecided:
            return globalObject->originalArrayStructureForIndexingType(ArrayWithUndecided);
        case Array::ArrayStorage:
            return globalObject->originalArrayStructureForIndexingType(ArrayWithArrayStorage);
        default:
            CRASH();
            return nullptr;
        }
    }
        
    case Array::OriginalNonArray: {
        TypedArrayType type = typedArrayType();
        if (type == NotTypedArray)
            return nullptr;
        
        return globalObject->typedArrayStructureConcurrently(type);
    }
        
    default:
        return nullptr;
    }
}

Structure* ArrayMode::originalArrayStructure(Graph& graph, Node* node) const
{
    return originalArrayStructure(graph, node->origin.semantic);
}

bool ArrayMode::alreadyChecked(Graph& graph, Node* node, const AbstractValue& value, IndexingType shape) const
{
    switch (arrayClass()) {
    case Array::OriginalArray: {
        if (value.m_structure.isTop())
            return false;
        for (unsigned i = value.m_structure.size(); i--;) {
            RegisteredStructure structure = value.m_structure[i];
            if ((structure->indexingType() & IndexingShapeMask) != shape)
                return false;
            if (!(structure->indexingType() & IsArray))
                return false;
            if (!graph.globalObjectFor(node->origin.semantic)->isOriginalArrayStructure(structure.get()))
                return false;
        }
        return true;
    }
        
    case Array::Array: {
        if (arrayModesAlreadyChecked(value.m_arrayModes, asArrayModes(shape | IsArray)))
            return true;
        if (value.m_structure.isTop())
            return false;
        for (unsigned i = value.m_structure.size(); i--;) {
            RegisteredStructure structure = value.m_structure[i];
            if ((structure->indexingType() & IndexingShapeMask) != shape)
                return false;
            if (!(structure->indexingType() & IsArray))
                return false;
        }
        return true;
    }
        
    default: {
        if (arrayModesAlreadyChecked(value.m_arrayModes, asArrayModes(shape) | asArrayModes(shape | IsArray)))
            return true;
        if (value.m_structure.isTop())
            return false;
        for (unsigned i = value.m_structure.size(); i--;) {
            RegisteredStructure structure = value.m_structure[i];
            if ((structure->indexingType() & IndexingShapeMask) != shape)
                return false;
        }
        return true;
    } }
}

bool ArrayMode::alreadyChecked(Graph& graph, Node* node, const AbstractValue& value) const
{
    switch (type()) {
    case Array::Generic:
        return true;
        
    case Array::ForceExit:
        return false;
        
    case Array::String:
        return speculationChecked(value.m_type, SpecString);
        
    case Array::Int32:
        return alreadyChecked(graph, node, value, Int32Shape);
        
    case Array::Double:
        return alreadyChecked(graph, node, value, DoubleShape);
        
    case Array::Contiguous:
        return alreadyChecked(graph, node, value, ContiguousShape);
        
    case Array::ArrayStorage:
        return alreadyChecked(graph, node, value, ArrayStorageShape);

    case Array::Undecided:
        return alreadyChecked(graph, node, value, UndecidedShape);
        
    case Array::SlowPutArrayStorage:
        switch (arrayClass()) {
        case Array::OriginalArray: {
            CRASH();
            return false;
        }
        
        case Array::Array: {
            if (arrayModesAlreadyChecked(value.m_arrayModes, asArrayModes(ArrayWithArrayStorage) | asArrayModes(ArrayWithSlowPutArrayStorage)))
                return true;
            if (value.m_structure.isTop())
                return false;
            for (unsigned i = value.m_structure.size(); i--;) {
                RegisteredStructure structure = value.m_structure[i];
                if (!hasAnyArrayStorage(structure->indexingType()))
                    return false;
                if (!(structure->indexingType() & IsArray))
                    return false;
            }
            return true;
        }
        
        default: {
            if (arrayModesAlreadyChecked(value.m_arrayModes, asArrayModes(NonArrayWithArrayStorage) | asArrayModes(ArrayWithArrayStorage) | asArrayModes(NonArrayWithSlowPutArrayStorage) | asArrayModes(ArrayWithSlowPutArrayStorage)))
                return true;
            if (value.m_structure.isTop())
                return false;
            for (unsigned i = value.m_structure.size(); i--;) {
                RegisteredStructure structure = value.m_structure[i];
                if (!hasAnyArrayStorage(structure->indexingType()))
                    return false;
            }
            return true;
        } }
        
    case Array::DirectArguments:
        return speculationChecked(value.m_type, SpecDirectArguments);
        
    case Array::ScopedArguments:
        return speculationChecked(value.m_type, SpecScopedArguments);
        
    case Array::Int8Array:
        return speculationChecked(value.m_type, SpecInt8Array);
        
    case Array::Int16Array:
        return speculationChecked(value.m_type, SpecInt16Array);
        
    case Array::Int32Array:
        return speculationChecked(value.m_type, SpecInt32Array);
        
    case Array::Uint8Array:
        return speculationChecked(value.m_type, SpecUint8Array);
        
    case Array::Uint8ClampedArray:
        return speculationChecked(value.m_type, SpecUint8ClampedArray);
        
    case Array::Uint16Array:
        return speculationChecked(value.m_type, SpecUint16Array);
        
    case Array::Uint32Array:
        return speculationChecked(value.m_type, SpecUint32Array);

    case Array::Float32Array:
        return speculationChecked(value.m_type, SpecFloat32Array);

    case Array::Float64Array:
        return speculationChecked(value.m_type, SpecFloat64Array);

    case Array::AnyTypedArray:
        return speculationChecked(value.m_type, SpecTypedArrayView);

    case Array::SelectUsingPredictions:
    case Array::Unprofiled:
    case Array::SelectUsingArguments:
        break;
    }
    
    CRASH();
    return false;
}

const char* arrayTypeToString(Array::Type type)
{
    switch (type) {
    case Array::SelectUsingPredictions:
        return "SelectUsingPredictions";
    case Array::SelectUsingArguments:
        return "SelectUsingArguments";
    case Array::Unprofiled:
        return "Unprofiled";
    case Array::Generic:
        return "Generic";
    case Array::ForceExit:
        return "ForceExit";
    case Array::String:
        return "String";
    case Array::Undecided:
        return "Undecided";
    case Array::Int32:
        return "Int32";
    case Array::Double:
        return "Double";
    case Array::Contiguous:
        return "Contiguous";
    case Array::ArrayStorage:
        return "ArrayStorage";
    case Array::SlowPutArrayStorage:
        return "SlowPutArrayStorage";
    case Array::DirectArguments:
        return "DirectArguments";
    case Array::ScopedArguments:
        return "ScopedArguments";
    case Array::Int8Array:
        return "Int8Array";
    case Array::Int16Array:
        return "Int16Array";
    case Array::Int32Array:
        return "Int32Array";
    case Array::Uint8Array:
        return "Uint8Array";
    case Array::Uint8ClampedArray:
        return "Uint8ClampedArray";
    case Array::Uint16Array:
        return "Uint16Array";
    case Array::Uint32Array:
        return "Uint32Array";
    case Array::Float32Array:
        return "Float32Array";
    case Array::Float64Array:
        return "Float64Array";
    case Array::AnyTypedArray:
        return "AnyTypedArray";
    default:
        // Better to return something then it is to crash. Remember, this method
        // is being called from our main diagnostic tool, the IR dumper. It's like
        // a stack trace. So if we get here then probably something has already
        // gone wrong.
        return "Unknown!";
    }
}

const char* arrayClassToString(Array::Class arrayClass)
{
    switch (arrayClass) {
    case Array::Array:
        return "Array";
    case Array::OriginalArray:
        return "OriginalArray";
    case Array::NonArray:
        return "NonArray";
    case Array::OriginalNonArray:
        return "OriginalNonArray";
    case Array::PossiblyArray:
        return "PossiblyArray";
    default:
        return "Unknown!";
    }
}

const char* arraySpeculationToString(Array::Speculation speculation)
{
    switch (speculation) {
    case Array::SaneChain:
        return "SaneChain";
    case Array::InBounds:
        return "InBounds";
    case Array::ToHole:
        return "ToHole";
    case Array::OutOfBounds:
        return "OutOfBounds";
    default:
        return "Unknown!";
    }
}

const char* arrayConversionToString(Array::Conversion conversion)
{
    switch (conversion) {
    case Array::AsIs:
        return "AsIs";
    case Array::Convert:
        return "Convert";
    default:
        return "Unknown!";
    }
}

IndexingType toIndexingShape(Array::Type type)
{
    switch (type) {
    case Array::Int32:
        return Int32Shape;
    case Array::Double:
        return DoubleShape;
    case Array::Contiguous:
        return ContiguousShape;
    case Array::ArrayStorage:
        return ArrayStorageShape;
    case Array::SlowPutArrayStorage:
        return SlowPutArrayStorageShape;
    default:
        return NoIndexingShape;
    }
}

TypedArrayType toTypedArrayType(Array::Type type)
{
    switch (type) {
    case Array::Int8Array:
        return TypeInt8;
    case Array::Int16Array:
        return TypeInt16;
    case Array::Int32Array:
        return TypeInt32;
    case Array::Uint8Array:
        return TypeUint8;
    case Array::Uint8ClampedArray:
        return TypeUint8Clamped;
    case Array::Uint16Array:
        return TypeUint16;
    case Array::Uint32Array:
        return TypeUint32;
    case Array::Float32Array:
        return TypeFloat32;
    case Array::Float64Array:
        return TypeFloat64;
    case Array::AnyTypedArray:
        RELEASE_ASSERT_NOT_REACHED();
        return NotTypedArray;
    default:
        return NotTypedArray;
    }
}

Array::Type toArrayType(TypedArrayType type)
{
    switch (type) {
    case TypeInt8:
        return Array::Int8Array;
    case TypeInt16:
        return Array::Int16Array;
    case TypeInt32:
        return Array::Int32Array;
    case TypeUint8:
        return Array::Uint8Array;
    case TypeUint8Clamped:
        return Array::Uint8ClampedArray;
    case TypeUint16:
        return Array::Uint16Array;
    case TypeUint32:
        return Array::Uint32Array;
    case TypeFloat32:
        return Array::Float32Array;
    case TypeFloat64:
        return Array::Float64Array;
    default:
        return Array::Generic;
    }
}

Array::Type refineTypedArrayType(Array::Type oldType, TypedArrayType newType)
{
    if (oldType == Array::Generic)
        return oldType;
    Array::Type newArrayType = toArrayType(newType);
    if (newArrayType == Array::Generic)
        return newArrayType;

    if (oldType != newArrayType)
        return Array::AnyTypedArray;
    return oldType;
}

bool permitsBoundsCheckLowering(Array::Type type)
{
    switch (type) {
    case Array::Int32:
    case Array::Double:
    case Array::Contiguous:
    case Array::Int8Array:
    case Array::Int16Array:
    case Array::Int32Array:
    case Array::Uint8Array:
    case Array::Uint8ClampedArray:
    case Array::Uint16Array:
    case Array::Uint32Array:
    case Array::Float32Array:
    case Array::Float64Array:
    case Array::AnyTypedArray:
        return true;
    default:
        // These don't allow for bounds check lowering either because the bounds
        // check involves something other than GetArrayLength (like ArrayStorage),
        // or because the bounds check isn't a speculation (like String, sort of),
        // or because the type implies an impure access.
        return false;
    }
}

bool ArrayMode::permitsBoundsCheckLowering() const
{
    return DFG::permitsBoundsCheckLowering(type()) && isInBounds();
}

void ArrayMode::dump(PrintStream& out) const
{
    out.print(type(), "+", arrayClass(), "+", speculation(), "+", conversion());
}

} } // namespace JSC::DFG

namespace WTF {

void printInternal(PrintStream& out, JSC::DFG::Array::Type type)
{
    out.print(JSC::DFG::arrayTypeToString(type));
}

void printInternal(PrintStream& out, JSC::DFG::Array::Class arrayClass)
{
    out.print(JSC::DFG::arrayClassToString(arrayClass));
}

void printInternal(PrintStream& out, JSC::DFG::Array::Speculation speculation)
{
    out.print(JSC::DFG::arraySpeculationToString(speculation));
}

void printInternal(PrintStream& out, JSC::DFG::Array::Conversion conversion)
{
    out.print(JSC::DFG::arrayConversionToString(conversion));
}

} // namespace WTF

#endif // ENABLE(DFG_JIT)

