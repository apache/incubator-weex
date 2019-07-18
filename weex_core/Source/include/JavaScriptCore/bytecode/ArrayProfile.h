/*
 * Copyright (C) 2012, 2013 Apple Inc. All rights reserved.
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

#include "ConcurrentJSLock.h"
#include "JSArray.h"
#include "Structure.h"
#include <wtf/SegmentedVector.h>

namespace JSC {

class CodeBlock;
class LLIntOffsetsExtractor;

// This is a bitfield where each bit represents an type of array access that we have seen.
// There are 16 indexing types that use the lower bits.
// There are 9 typed array types taking the bits 16 to 25.
typedef unsigned ArrayModes;

const ArrayModes Int8ArrayMode = 1 << 16;
const ArrayModes Int16ArrayMode = 1 << 17;
const ArrayModes Int32ArrayMode = 1 << 18;
const ArrayModes Uint8ArrayMode = 1 << 19;
const ArrayModes Uint8ClampedArrayMode = 1 << 20;
const ArrayModes Uint16ArrayMode = 1 << 21;
const ArrayModes Uint32ArrayMode = 1 << 22;
const ArrayModes Float32ArrayMode = 1 << 23;
const ArrayModes Float64ArrayMode = 1 << 24;

#define asArrayModes(type) \
    (static_cast<unsigned>(1) << static_cast<unsigned>(type))

#define ALL_TYPED_ARRAY_MODES \
    (Int8ArrayMode            \
    | Int16ArrayMode          \
    | Int32ArrayMode          \
    | Uint8ArrayMode          \
    | Uint8ClampedArrayMode   \
    | Uint16ArrayMode         \
    | Uint32ArrayMode         \
    | Float32ArrayMode        \
    | Float64ArrayMode        \
    )

#define ALL_NON_ARRAY_ARRAY_MODES                       \
    (asArrayModes(NonArray)                             \
    | asArrayModes(NonArrayWithInt32)                   \
    | asArrayModes(NonArrayWithDouble)                  \
    | asArrayModes(NonArrayWithContiguous)              \
    | asArrayModes(NonArrayWithArrayStorage)            \
    | asArrayModes(NonArrayWithSlowPutArrayStorage)     \
    | ALL_TYPED_ARRAY_MODES)

#define ALL_ARRAY_ARRAY_MODES                           \
    (asArrayModes(ArrayClass)                           \
    | asArrayModes(ArrayWithUndecided)                  \
    | asArrayModes(ArrayWithInt32)                      \
    | asArrayModes(ArrayWithDouble)                     \
    | asArrayModes(ArrayWithContiguous)                 \
    | asArrayModes(ArrayWithArrayStorage)               \
    | asArrayModes(ArrayWithSlowPutArrayStorage))

#define ALL_ARRAY_MODES (ALL_NON_ARRAY_ARRAY_MODES | ALL_ARRAY_ARRAY_MODES)

inline ArrayModes arrayModeFromStructure(Structure* structure)
{
    switch (structure->classInfo()->typedArrayStorageType) {
    case TypeInt8:
        return Int8ArrayMode;
    case TypeUint8:
        return Uint8ArrayMode;
    case TypeUint8Clamped:
        return Uint8ClampedArrayMode;
    case TypeInt16:
        return Int16ArrayMode;
    case TypeUint16:
        return Uint16ArrayMode;
    case TypeInt32:
        return Int32ArrayMode;
    case TypeUint32:
        return Uint32ArrayMode;
    case TypeFloat32:
        return Float32ArrayMode;
    case TypeFloat64:
        return Float64ArrayMode;
    case TypeDataView:
    case NotTypedArray:
        break;
    }
    return asArrayModes(structure->indexingType());
}

void dumpArrayModes(PrintStream&, ArrayModes);
MAKE_PRINT_ADAPTOR(ArrayModesDump, ArrayModes, dumpArrayModes);

inline bool mergeArrayModes(ArrayModes& left, ArrayModes right)
{
    ArrayModes newModes = left | right;
    if (newModes == left)
        return false;
    left = newModes;
    return true;
}

inline bool arrayModesAreClearOrTop(ArrayModes modes)
{
    return !modes || modes == ALL_ARRAY_MODES;
}

// Checks if proven is a subset of expected.
inline bool arrayModesAlreadyChecked(ArrayModes proven, ArrayModes expected)
{
    return (expected | proven) == expected;
}

inline bool arrayModesInclude(ArrayModes arrayModes, IndexingType shape)
{
    return !!(arrayModes & (asArrayModes(NonArray | shape) | asArrayModes(ArrayClass | shape)));
}

inline bool shouldUseSlowPutArrayStorage(ArrayModes arrayModes)
{
    return arrayModesInclude(arrayModes, SlowPutArrayStorageShape);
}

inline bool shouldUseFastArrayStorage(ArrayModes arrayModes)
{
    return arrayModesInclude(arrayModes, ArrayStorageShape);
}

inline bool shouldUseContiguous(ArrayModes arrayModes)
{
    return arrayModesInclude(arrayModes, ContiguousShape);
}

inline bool shouldUseDouble(ArrayModes arrayModes)
{
    return arrayModesInclude(arrayModes, DoubleShape);
}

inline bool shouldUseInt32(ArrayModes arrayModes)
{
    return arrayModesInclude(arrayModes, Int32Shape);
}

inline bool hasSeenArray(ArrayModes arrayModes)
{
    return arrayModes & ALL_ARRAY_ARRAY_MODES;
}

inline bool hasSeenNonArray(ArrayModes arrayModes)
{
    return arrayModes & ALL_NON_ARRAY_ARRAY_MODES;
}

class ArrayProfile {
public:
    ArrayProfile()
        : m_bytecodeOffset(std::numeric_limits<unsigned>::max())
        , m_lastSeenStructureID(0)
        , m_mayStoreToHole(false)
        , m_outOfBounds(false)
        , m_mayInterceptIndexedAccesses(false)
        , m_usesOriginalArrayStructures(true)
        , m_didPerformFirstRunPruning(false)
        , m_observedArrayModes(0)
    {
    }
    
    ArrayProfile(unsigned bytecodeOffset)
        : m_bytecodeOffset(bytecodeOffset)
        , m_lastSeenStructureID(0)
        , m_mayStoreToHole(false)
        , m_outOfBounds(false)
        , m_mayInterceptIndexedAccesses(false)
        , m_usesOriginalArrayStructures(true)
        , m_didPerformFirstRunPruning(false)
        , m_observedArrayModes(0)
    {
    }
    
    unsigned bytecodeOffset() const { return m_bytecodeOffset; }
    
    StructureID* addressOfLastSeenStructureID() { return &m_lastSeenStructureID; }
    ArrayModes* addressOfArrayModes() { return &m_observedArrayModes; }
    bool* addressOfMayStoreToHole() { return &m_mayStoreToHole; }

    void setOutOfBounds() { m_outOfBounds = true; }
    bool* addressOfOutOfBounds() { return &m_outOfBounds; }
    
    void observeStructure(Structure* structure)
    {
        m_lastSeenStructureID = structure->id();
    }
    
    void computeUpdatedPrediction(const ConcurrentJSLocker&, CodeBlock*);
    void computeUpdatedPrediction(const ConcurrentJSLocker&, CodeBlock*, Structure* lastSeenStructure);
    
    ArrayModes observedArrayModes(const ConcurrentJSLocker&) const { return m_observedArrayModes; }
    bool mayInterceptIndexedAccesses(const ConcurrentJSLocker&) const { return m_mayInterceptIndexedAccesses; }
    
    bool mayStoreToHole(const ConcurrentJSLocker&) const { return m_mayStoreToHole; }
    bool outOfBounds(const ConcurrentJSLocker&) const { return m_outOfBounds; }
    
    bool usesOriginalArrayStructures(const ConcurrentJSLocker&) const { return m_usesOriginalArrayStructures; }
    
    CString briefDescription(const ConcurrentJSLocker&, CodeBlock*);
    CString briefDescriptionWithoutUpdating(const ConcurrentJSLocker&);
    
private:
    friend class LLIntOffsetsExtractor;
    
    static Structure* polymorphicStructure() { return static_cast<Structure*>(reinterpret_cast<void*>(1)); }
    
    unsigned m_bytecodeOffset;
    StructureID m_lastSeenStructureID;
    bool m_mayStoreToHole; // This flag may become overloaded to indicate other special cases that were encountered during array access, as it depends on indexing type. Since we currently have basically just one indexing type (two variants of ArrayStorage), this flag for now just means exactly what its name implies.
    bool m_outOfBounds;
    bool m_mayInterceptIndexedAccesses : 1;
    bool m_usesOriginalArrayStructures : 1;
    bool m_didPerformFirstRunPruning : 1;
    ArrayModes m_observedArrayModes;
};

typedef SegmentedVector<ArrayProfile, 4> ArrayProfileVector;

} // namespace JSC
