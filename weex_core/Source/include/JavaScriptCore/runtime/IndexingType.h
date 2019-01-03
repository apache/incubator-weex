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

#include "SpeculatedType.h"
#include <wtf/LockAlgorithm.h>
#include <wtf/StdLibExtras.h>

namespace JSC {

/*
    Structure of the IndexingType
    =============================
    Conceptually, the IndexingType looks like this:

    struct IndexingType {
        uint8_t isArray:1;                    // bit 0
        uint8_t shape:4;                      // bit 1 - 3
        uint8_t mayHaveIndexedAccessors:1;    // bit 4
    };

    The shape values (e.g. Int32Shape, ContiguousShape, etc) are an enumeration of
    various shapes (though not necessarily sequential in terms of their values).
    Hence, shape values are not bitwise exclusive with respect to each other.
*/

typedef uint8_t IndexingType;

// Flags for testing the presence of capabilities.
static const IndexingType IsArray                  = 0x01;

// The shape of the indexed property storage.
static const IndexingType IndexingShapeMask               = 0x0E;
static const IndexingType NoIndexingShape                 = 0x00;
static const IndexingType UndecidedShape                  = 0x02; // Only useful for arrays.
static const IndexingType Int32Shape                      = 0x04;
static const IndexingType DoubleShape                     = 0x06;
static const IndexingType ContiguousShape                 = 0x08;
static const IndexingType ArrayStorageShape               = 0x0A;
static const IndexingType SlowPutArrayStorageShape        = 0x0C;

static const IndexingType IndexingShapeShift              = 1;
static const IndexingType NumberOfIndexingShapes          = 7;

// Additional flags for tracking the history of the type. These are usually
// masked off unless you ask for them directly.
static const IndexingType MayHaveIndexedAccessors         = 0x10;

// The IndexingType field of JSCells is stolen for locks.
static const IndexingType IndexingTypeLockIsHeld          = 0x20;
static const IndexingType IndexingTypeLockHasParked       = 0x40;

// List of acceptable array types.
static const IndexingType NonArray                        = 0x0;
static const IndexingType NonArrayWithInt32               = Int32Shape;
static const IndexingType NonArrayWithDouble              = DoubleShape;
static const IndexingType NonArrayWithContiguous          = ContiguousShape;
static const IndexingType NonArrayWithArrayStorage        = ArrayStorageShape;
static const IndexingType NonArrayWithSlowPutArrayStorage = SlowPutArrayStorageShape;
static const IndexingType ArrayClass                      = IsArray; // I'd want to call this "Array" but this would lead to disastrous namespace pollution.
static const IndexingType ArrayWithUndecided              = IsArray | UndecidedShape;
static const IndexingType ArrayWithInt32                  = IsArray | Int32Shape;
static const IndexingType ArrayWithDouble                 = IsArray | DoubleShape;
static const IndexingType ArrayWithContiguous             = IsArray | ContiguousShape;
static const IndexingType ArrayWithArrayStorage           = IsArray | ArrayStorageShape;
static const IndexingType ArrayWithSlowPutArrayStorage    = IsArray | SlowPutArrayStorageShape;

#define ALL_BLANK_INDEXING_TYPES \
    NonArray:                    \
    case ArrayClass

#define ALL_UNDECIDED_INDEXING_TYPES \
    ArrayWithUndecided

#define ALL_INT32_INDEXING_TYPES      \
    NonArrayWithInt32:                \
    case ArrayWithInt32

#define ALL_DOUBLE_INDEXING_TYPES     \
    NonArrayWithDouble:               \
    case ArrayWithDouble

#define ALL_CONTIGUOUS_INDEXING_TYPES \
    NonArrayWithContiguous:           \
    case ArrayWithContiguous

#define ARRAY_WITH_ARRAY_STORAGE_INDEXING_TYPES \
    ArrayWithArrayStorage:                      \
    case ArrayWithSlowPutArrayStorage
    
#define ALL_ARRAY_STORAGE_INDEXING_TYPES                \
    NonArrayWithArrayStorage:                           \
    case NonArrayWithSlowPutArrayStorage:               \
    case ARRAY_WITH_ARRAY_STORAGE_INDEXING_TYPES

static inline bool hasIndexedProperties(IndexingType indexingType)
{
    return (indexingType & IndexingShapeMask) != NoIndexingShape;
}

static inline bool hasUndecided(IndexingType indexingType)
{
    return (indexingType & IndexingShapeMask) == UndecidedShape;
}

static inline bool hasInt32(IndexingType indexingType)
{
    return (indexingType & IndexingShapeMask) == Int32Shape;
}

static inline bool hasDouble(IndexingType indexingType)
{
    return (indexingType & IndexingShapeMask) == DoubleShape;
}

static inline bool hasContiguous(IndexingType indexingType)
{
    return (indexingType & IndexingShapeMask) == ContiguousShape;
}

static inline bool hasArrayStorage(IndexingType indexingType)
{
    return (indexingType & IndexingShapeMask) == ArrayStorageShape;
}

static inline bool hasAnyArrayStorage(IndexingType indexingType)
{
    return static_cast<uint8_t>(indexingType & IndexingShapeMask) >= ArrayStorageShape;
}

static inline bool shouldUseSlowPut(IndexingType indexingType)
{
    return (indexingType & IndexingShapeMask) == SlowPutArrayStorageShape;
}

inline IndexingType indexingTypeForValue(JSValue value)
{
    if (value.isInt32())
        return Int32Shape;

    if (value.isNumber() && value.asNumber() == value.asNumber())
        return DoubleShape;

    return ContiguousShape;
}

// Return an indexing type that can handle all of the elements of both indexing types.
IndexingType leastUpperBoundOfIndexingTypes(IndexingType, IndexingType);

IndexingType leastUpperBoundOfIndexingTypeAndType(IndexingType, SpeculatedType);
IndexingType leastUpperBoundOfIndexingTypeAndValue(IndexingType, JSValue);

void dumpIndexingType(PrintStream&, IndexingType);
MAKE_PRINT_ADAPTOR(IndexingTypeDump, IndexingType, dumpIndexingType);

// Mask of all possible types.
static const IndexingType AllArrayTypes            = IndexingShapeMask | IsArray;

// Mask of all possible types including the history.
static const IndexingType AllArrayTypesAndHistory  = AllArrayTypes | MayHaveIndexedAccessors;

typedef LockAlgorithm<IndexingType, IndexingTypeLockIsHeld, IndexingTypeLockHasParked> IndexingTypeLockAlgorithm;

} // namespace JSC
