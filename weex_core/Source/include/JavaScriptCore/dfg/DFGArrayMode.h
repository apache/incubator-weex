/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
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

#if ENABLE(DFG_JIT)

#include "ArrayProfile.h"
#include "DFGNodeFlags.h"
#include "SpeculatedType.h"

namespace JSC {

struct CodeOrigin;

namespace DFG {

class Graph;
struct AbstractValue;
struct Node;

// Use a namespace + enum instead of enum alone to avoid the namespace collision
// that would otherwise occur, since we say things like "Int8Array" and "JSArray"
// in lots of other places, to mean subtly different things.
namespace Array {
enum Action {
    Read,
    Write
};

enum Type {
    SelectUsingPredictions, // Implies that we need predictions to decide. We will never get to the backend in this mode.
    SelectUsingArguments, // Implies that we use the Node's arguments to decide. We will never get to the backend in this mode.
    Unprofiled, // Implies that array profiling didn't see anything. But that could be because the operands didn't comply with basic type assumptions (base is cell, property is int). This either becomes Generic or ForceExit depending on value profiling.
    ForceExit, // Implies that we have no idea how to execute this operation, so we should just give up.
    Generic,
    String,

    Undecided,
    Int32,
    Double,
    Contiguous,
    ArrayStorage,
    SlowPutArrayStorage,
    
    DirectArguments,
    ScopedArguments,
    
    Int8Array,
    Int16Array,
    Int32Array,
    Uint8Array,
    Uint8ClampedArray,
    Uint16Array,
    Uint32Array,
    Float32Array,
    Float64Array,
    AnyTypedArray
};

enum Class {
    NonArray, // Definitely some object that is not a JSArray.
    OriginalNonArray, // Definitely some object that is not a JSArray, but that object has the original structure.
    Array, // Definitely a JSArray, and may or may not have custom properties or have undergone some other bizarre transitions.
    OriginalArray, // Definitely a JSArray, and still has one of the primordial JSArray structures for the global object that this code block (possibly inlined code block) belongs to.
    PossiblyArray // Some object that may or may not be a JSArray.
};

enum Speculation {
    SaneChain, // In bounds and the array prototype chain is still intact, i.e. loading a hole doesn't require special treatment.
    
    InBounds, // In bounds and not loading a hole.
    ToHole, // Potentially storing to a hole.
    OutOfBounds // Out-of-bounds access and anything can happen.
};
enum Conversion {
    AsIs,
    Convert
};
} // namespace Array

const char* arrayTypeToString(Array::Type);
const char* arrayClassToString(Array::Class);
const char* arraySpeculationToString(Array::Speculation);
const char* arrayConversionToString(Array::Conversion);

IndexingType toIndexingShape(Array::Type);

TypedArrayType toTypedArrayType(Array::Type);
Array::Type toArrayType(TypedArrayType);
Array::Type refineTypedArrayType(Array::Type, TypedArrayType);

bool permitsBoundsCheckLowering(Array::Type);

class ArrayMode {
public:
    ArrayMode()
    {
        u.asBytes.type = Array::SelectUsingPredictions;
        u.asBytes.arrayClass = Array::NonArray;
        u.asBytes.speculation = Array::InBounds;
        u.asBytes.conversion = Array::AsIs;
    }
    
    explicit ArrayMode(Array::Type type)
    {
        u.asBytes.type = type;
        u.asBytes.arrayClass = Array::NonArray;
        u.asBytes.speculation = Array::InBounds;
        u.asBytes.conversion = Array::AsIs;
    }
    
    ArrayMode(Array::Type type, Array::Class arrayClass)
    {
        u.asBytes.type = type;
        u.asBytes.arrayClass = arrayClass;
        u.asBytes.speculation = Array::InBounds;
        u.asBytes.conversion = Array::AsIs;
    }
    
    ArrayMode(Array::Type type, Array::Class arrayClass, Array::Speculation speculation, Array::Conversion conversion)
    {
        u.asBytes.type = type;
        u.asBytes.arrayClass = arrayClass;
        u.asBytes.speculation = speculation;
        u.asBytes.conversion = conversion;
    }
    
    ArrayMode(Array::Type type, Array::Class arrayClass, Array::Conversion conversion)
    {
        u.asBytes.type = type;
        u.asBytes.arrayClass = arrayClass;
        u.asBytes.speculation = Array::InBounds;
        u.asBytes.conversion = conversion;
    }
    
    Array::Type type() const { return static_cast<Array::Type>(u.asBytes.type); }
    Array::Class arrayClass() const { return static_cast<Array::Class>(u.asBytes.arrayClass); }
    Array::Speculation speculation() const { return static_cast<Array::Speculation>(u.asBytes.speculation); }
    Array::Conversion conversion() const { return static_cast<Array::Conversion>(u.asBytes.conversion); }
    
    unsigned asWord() const { return u.asWord; }
    
    static ArrayMode fromWord(unsigned word)
    {
        return ArrayMode(word);
    }
    
    static ArrayMode fromObserved(const ConcurrentJSLocker&, ArrayProfile*, Array::Action, bool makeSafe);
    
    ArrayMode withSpeculation(Array::Speculation speculation) const
    {
        return ArrayMode(type(), arrayClass(), speculation, conversion());
    }
    
    ArrayMode withArrayClass(Array::Class arrayClass) const
    {
        return ArrayMode(type(), arrayClass, speculation(), conversion());
    }
    
    ArrayMode withSpeculationFromProfile(const ConcurrentJSLocker& locker, ArrayProfile* profile, bool makeSafe) const
    {
        Array::Speculation mySpeculation;

        if (makeSafe)
            mySpeculation = Array::OutOfBounds;
        else if (profile->mayStoreToHole(locker))
            mySpeculation = Array::ToHole;
        else
            mySpeculation = Array::InBounds;
        
        return withSpeculation(mySpeculation);
    }
    
    ArrayMode withProfile(const ConcurrentJSLocker& locker, ArrayProfile* profile, bool makeSafe) const
    {
        Array::Class myArrayClass;

        if (isJSArray()) {
            if (profile->usesOriginalArrayStructures(locker) && benefitsFromOriginalArray())
                myArrayClass = Array::OriginalArray;
            else
                myArrayClass = Array::Array;
        } else
            myArrayClass = arrayClass();
        
        return withArrayClass(myArrayClass).withSpeculationFromProfile(locker, profile, makeSafe);
    }
    
    ArrayMode withType(Array::Type type) const
    {
        return ArrayMode(type, arrayClass(), speculation(), conversion());
    }
    
    ArrayMode withConversion(Array::Conversion conversion) const
    {
        return ArrayMode(type(), arrayClass(), speculation(), conversion);
    }
    
    ArrayMode withTypeAndConversion(Array::Type type, Array::Conversion conversion) const
    {
        return ArrayMode(type, arrayClass(), speculation(), conversion);
    }
    
    ArrayMode refine(Graph&, Node*, SpeculatedType base, SpeculatedType index, SpeculatedType value = SpecNone) const;
    
    bool alreadyChecked(Graph&, Node*, const AbstractValue&) const;
    
    void dump(PrintStream&) const;
    
    bool usesButterfly() const
    {
        switch (type()) {
        case Array::Undecided:
        case Array::Int32:
        case Array::Double:
        case Array::Contiguous:
        case Array::ArrayStorage:
        case Array::SlowPutArrayStorage:
            return true;
        default:
            return false;
        }
    }
    
    bool isJSArray() const
    {
        switch (arrayClass()) {
        case Array::Array:
        case Array::OriginalArray:
            return true;
        default:
            return false;
        }
    }
    
    bool isJSArrayWithOriginalStructure() const
    {
        return arrayClass() == Array::OriginalArray;
    }
    
    bool isSaneChain() const
    {
        return speculation() == Array::SaneChain;
    }
    
    bool isInBounds() const
    {
        switch (speculation()) {
        case Array::SaneChain:
        case Array::InBounds:
            return true;
        default:
            return false;
        }
    }
    
    bool mayStoreToHole() const
    {
        return !isInBounds();
    }
    
    bool isOutOfBounds() const
    {
        return speculation() == Array::OutOfBounds;
    }
    
    bool isSlowPut() const
    {
        return type() == Array::SlowPutArrayStorage;
    }
    
    bool canCSEStorage() const
    {
        switch (type()) {
        case Array::SelectUsingPredictions:
        case Array::SelectUsingArguments:
        case Array::Unprofiled:
        case Array::Undecided:
        case Array::ForceExit:
        case Array::Generic:
        case Array::DirectArguments:
        case Array::ScopedArguments:
            return false;
        default:
            return true;
        }
    }
    
    bool lengthNeedsStorage() const
    {
        switch (type()) {
        case Array::Undecided:
        case Array::Int32:
        case Array::Double:
        case Array::Contiguous:
        case Array::ArrayStorage:
        case Array::SlowPutArrayStorage:
            return true;
        default:
            return false;
        }
    }
    
    ArrayMode modeForPut() const
    {
        switch (type()) {
        case Array::String:
        case Array::DirectArguments:
        case Array::ScopedArguments:
            return ArrayMode(Array::Generic);
        default:
            return *this;
        }
    }
    
    bool isSpecific() const
    {
        switch (type()) {
        case Array::SelectUsingPredictions:
        case Array::SelectUsingArguments:
        case Array::Unprofiled:
        case Array::ForceExit:
        case Array::Generic:
            return false;
        default:
            return true;
        }
    }
    
    bool supportsSelfLength() const
    {
        switch (type()) {
        case Array::SelectUsingPredictions:
        case Array::Unprofiled:
        case Array::ForceExit:
        case Array::Generic:
        // TypedArrays do not have a self length property as of ES6.
        case Array::Int8Array:
        case Array::Int16Array:
        case Array::Int32Array:
        case Array::Uint8Array:
        case Array::Uint8ClampedArray:
        case Array::Uint16Array:
        case Array::Uint32Array:
        case Array::Float32Array:
        case Array::Float64Array:
            return false;
        case Array::Int32:
        case Array::Double:
        case Array::Contiguous:
        case Array::ArrayStorage:
        case Array::SlowPutArrayStorage:
            return isJSArray();
        default:
            return true;
        }
    }
    
    bool permitsBoundsCheckLowering() const;
    
    bool benefitsFromOriginalArray() const
    {
        switch (type()) {
        case Array::Int32:
        case Array::Double:
        case Array::Contiguous:
        case Array::Undecided:
        case Array::ArrayStorage:
            return true;
        default:
            return false;
        }
    }
    
    // Returns 0 if this is not OriginalArray.
    Structure* originalArrayStructure(Graph&, const CodeOrigin&) const;
    Structure* originalArrayStructure(Graph&, Node*) const;
    
    bool doesConversion() const
    {
        return conversion() != Array::AsIs;
    }

    bool structureWouldPassArrayModeFiltering(Structure* structure)
    {
        return arrayModesAlreadyChecked(arrayModeFromStructure(structure), arrayModesThatPassFiltering());
    }
    
    ArrayModes arrayModesThatPassFiltering() const
    {
        switch (type()) {
        case Array::Generic:
            return ALL_ARRAY_MODES;
        case Array::Int32:
            return arrayModesWithIndexingShape(Int32Shape);
        case Array::Double:
            return arrayModesWithIndexingShape(DoubleShape);
        case Array::Contiguous:
            return arrayModesWithIndexingShape(ContiguousShape);
        case Array::ArrayStorage:
            return arrayModesWithIndexingShape(ArrayStorageShape);
        case Array::SlowPutArrayStorage:
            return arrayModesWithIndexingShapes(SlowPutArrayStorageShape, ArrayStorageShape);
        default:
            return asArrayModes(NonArray);
        }
    }
    
    bool getIndexedPropertyStorageMayTriggerGC() const
    {
        return type() == Array::String;
    }
    
    IndexingType shapeMask() const
    {
        return toIndexingShape(type());
    }
    
    TypedArrayType typedArrayType() const
    {
        return toTypedArrayType(type());
    }

    bool isSomeTypedArrayView() const
    {
        return type() == Array::AnyTypedArray || isTypedView(typedArrayType());
    }
    
    bool operator==(const ArrayMode& other) const
    {
        return type() == other.type()
            && arrayClass() == other.arrayClass()
            && speculation() == other.speculation()
            && conversion() == other.conversion();
    }
    
    bool operator!=(const ArrayMode& other) const
    {
        return !(*this == other);
    }
private:
    explicit ArrayMode(unsigned word)
    {
        u.asWord = word;
    }
    
    ArrayModes arrayModesWithIndexingShape(IndexingType shape) const
    {
        switch (arrayClass()) {
        case Array::NonArray:
        case Array::OriginalNonArray:
            return asArrayModes(shape);
        case Array::Array:
        case Array::OriginalArray:
            return asArrayModes(shape | IsArray);
        case Array::PossiblyArray:
            return asArrayModes(shape) | asArrayModes(shape | IsArray);
        default:
            // This is only necessary for C++ compilers that don't understand enums.
            return 0;
        }
    }
    
    ArrayModes arrayModesWithIndexingShapes(IndexingType shape1, IndexingType shape2) const
    {
        ArrayModes arrayMode1 = arrayModesWithIndexingShape(shape1);
        ArrayModes arrayMode2 = arrayModesWithIndexingShape(shape2);
        return arrayMode1 | arrayMode2;
    }

    bool alreadyChecked(Graph&, Node*, const AbstractValue&, IndexingType shape) const;
    
    union {
        struct {
            uint8_t type;
            uint8_t arrayClass;
            uint8_t speculation;
            uint8_t conversion;
        } asBytes;
        unsigned asWord;
    } u;
};

static inline bool canCSEStorage(const ArrayMode& arrayMode)
{
    return arrayMode.canCSEStorage();
}

static inline bool lengthNeedsStorage(const ArrayMode& arrayMode)
{
    return arrayMode.lengthNeedsStorage();
}

static inline bool neverNeedsStorage(const ArrayMode&)
{
    return false;
}

} } // namespace JSC::DFG

namespace WTF {

class PrintStream;
void printInternal(PrintStream&, JSC::DFG::Array::Type);
void printInternal(PrintStream&, JSC::DFG::Array::Class);
void printInternal(PrintStream&, JSC::DFG::Array::Speculation);
void printInternal(PrintStream&, JSC::DFG::Array::Conversion);

} // namespace WTF

#endif // ENABLE(DFG_JIT)
