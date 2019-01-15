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

#include "ArrayStorage.h"
#include "Butterfly.h"
#include "VM.h"
#include "Structure.h"

namespace JSC {

ALWAYS_INLINE unsigned Butterfly::availableContiguousVectorLength(size_t propertyCapacity, unsigned vectorLength)
{
    size_t cellSize = totalSize(0, propertyCapacity, true, sizeof(EncodedJSValue) * vectorLength);
    cellSize = MarkedSpace::optimalSizeFor(cellSize);
    vectorLength = (cellSize - totalSize(0, propertyCapacity, true, 0)) / sizeof(EncodedJSValue);
    return vectorLength;
}

ALWAYS_INLINE unsigned Butterfly::availableContiguousVectorLength(Structure* structure, unsigned vectorLength)
{
    return availableContiguousVectorLength(structure ? structure->outOfLineCapacity() : 0, vectorLength);
}

ALWAYS_INLINE unsigned Butterfly::optimalContiguousVectorLength(size_t propertyCapacity, unsigned vectorLength)
{
    if (!vectorLength)
        vectorLength = BASE_CONTIGUOUS_VECTOR_LEN_EMPTY;
    else
        vectorLength = std::max(BASE_CONTIGUOUS_VECTOR_LEN, vectorLength);
    return availableContiguousVectorLength(propertyCapacity, vectorLength);
}

ALWAYS_INLINE unsigned Butterfly::optimalContiguousVectorLength(Structure* structure, unsigned vectorLength)
{
    return optimalContiguousVectorLength(structure ? structure->outOfLineCapacity() : 0, vectorLength);
}

inline Butterfly* Butterfly::createUninitialized(VM& vm, JSCell*, size_t preCapacity, size_t propertyCapacity, bool hasIndexingHeader, size_t indexingPayloadSizeInBytes)
{
    size_t size = totalSize(preCapacity, propertyCapacity, hasIndexingHeader, indexingPayloadSizeInBytes);
    void* base = vm.auxiliarySpace.allocate(size);
    Butterfly* result = fromBase(base, preCapacity, propertyCapacity);
    return result;
}

inline Butterfly* Butterfly::tryCreate(VM& vm, JSCell*, size_t preCapacity, size_t propertyCapacity, bool hasIndexingHeader, const IndexingHeader& indexingHeader, size_t indexingPayloadSizeInBytes)
{
    size_t size = totalSize(preCapacity, propertyCapacity, hasIndexingHeader, indexingPayloadSizeInBytes);
    void* base = vm.auxiliarySpace.tryAllocate(size);
    if (!base)
        return nullptr;
    Butterfly* result = fromBase(base, preCapacity, propertyCapacity);
    if (hasIndexingHeader)
        *result->indexingHeader() = indexingHeader;
    memset(result->propertyStorage() - propertyCapacity, 0, propertyCapacity * sizeof(EncodedJSValue));
    return result;
}

inline Butterfly* Butterfly::create(VM& vm, JSCell* intendedOwner, size_t preCapacity, size_t propertyCapacity, bool hasIndexingHeader, const IndexingHeader& indexingHeader, size_t indexingPayloadSizeInBytes)
{
    Butterfly* result = tryCreate(vm, intendedOwner, preCapacity, propertyCapacity, hasIndexingHeader, indexingHeader, indexingPayloadSizeInBytes);

    RELEASE_ASSERT(result);
    return result;
}

inline Butterfly* Butterfly::create(VM& vm, JSCell* intendedOwner, Structure* structure)
{
    return create(
        vm, intendedOwner, 0, structure->outOfLineCapacity(),
        structure->hasIndexingHeader(intendedOwner), IndexingHeader(), 0);
}

inline void* Butterfly::base(Structure* structure)
{
    return base(indexingHeader()->preCapacity(structure), structure->outOfLineCapacity());
}

inline Butterfly* Butterfly::createOrGrowPropertyStorage(
    Butterfly* oldButterfly, VM& vm, JSCell* intendedOwner, Structure* structure, size_t oldPropertyCapacity, size_t newPropertyCapacity)
{
    RELEASE_ASSERT(newPropertyCapacity > oldPropertyCapacity);
    if (!oldButterfly)
        return create(vm, intendedOwner, 0, newPropertyCapacity, false, IndexingHeader(), 0);

    size_t preCapacity = oldButterfly->indexingHeader()->preCapacity(structure);
    size_t indexingPayloadSizeInBytes = oldButterfly->indexingHeader()->indexingPayloadSizeInBytes(structure);
    bool hasIndexingHeader = structure->hasIndexingHeader(intendedOwner);
    Butterfly* result = createUninitialized(
        vm, intendedOwner, preCapacity, newPropertyCapacity, hasIndexingHeader, indexingPayloadSizeInBytes);
    memcpy(
        result->propertyStorage() - oldPropertyCapacity,
        oldButterfly->propertyStorage() - oldPropertyCapacity,
        totalSize(0, oldPropertyCapacity, hasIndexingHeader, indexingPayloadSizeInBytes));
    memset(
        result->propertyStorage() - newPropertyCapacity,
        0,
        (newPropertyCapacity - oldPropertyCapacity) * sizeof(EncodedJSValue));
    return result;
}

inline Butterfly* Butterfly::createOrGrowArrayRight(
    Butterfly* oldButterfly, VM& vm, JSCell* intendedOwner, Structure* oldStructure,
    size_t propertyCapacity, bool hadIndexingHeader, size_t oldIndexingPayloadSizeInBytes,
    size_t newIndexingPayloadSizeInBytes)
{
    if (!oldButterfly) {
        return create(
            vm, intendedOwner, 0, propertyCapacity, true, IndexingHeader(),
            newIndexingPayloadSizeInBytes);
    }
    return oldButterfly->growArrayRight(
        vm, intendedOwner, oldStructure, propertyCapacity, hadIndexingHeader,
        oldIndexingPayloadSizeInBytes, newIndexingPayloadSizeInBytes);
}

inline Butterfly* Butterfly::growArrayRight(
    VM& vm, JSCell* intendedOwner, Structure* oldStructure, size_t propertyCapacity,
    bool hadIndexingHeader, size_t oldIndexingPayloadSizeInBytes,
    size_t newIndexingPayloadSizeInBytes)
{
    ASSERT_UNUSED(oldStructure, !indexingHeader()->preCapacity(oldStructure));
    ASSERT_UNUSED(intendedOwner, hadIndexingHeader == oldStructure->hasIndexingHeader(intendedOwner));
    void* theBase = base(0, propertyCapacity);
    size_t oldSize = totalSize(0, propertyCapacity, hadIndexingHeader, oldIndexingPayloadSizeInBytes);
    size_t newSize = totalSize(0, propertyCapacity, true, newIndexingPayloadSizeInBytes);
    void* newBase = vm.auxiliarySpace.tryAllocate(newSize);
    if (!newBase)
        return nullptr;
    // FIXME: This probably shouldn't be a memcpy.
    memcpy(newBase, theBase, oldSize);
    return fromBase(newBase, 0, propertyCapacity);
}

inline Butterfly* Butterfly::growArrayRight(
    VM& vm, JSCell* intendedOwner, Structure* oldStructure,
    size_t newIndexingPayloadSizeInBytes)
{
    return growArrayRight(
        vm, intendedOwner, oldStructure, oldStructure->outOfLineCapacity(),
        oldStructure->hasIndexingHeader(intendedOwner), 
        indexingHeader()->indexingPayloadSizeInBytes(oldStructure),
        newIndexingPayloadSizeInBytes);
}

inline Butterfly* Butterfly::resizeArray(
    VM& vm, JSCell* intendedOwner, size_t propertyCapacity, bool oldHasIndexingHeader,
    size_t oldIndexingPayloadSizeInBytes, size_t newPreCapacity, bool newHasIndexingHeader,
    size_t newIndexingPayloadSizeInBytes)
{
    Butterfly* result = createUninitialized(
        vm, intendedOwner, newPreCapacity, propertyCapacity, newHasIndexingHeader,
        newIndexingPayloadSizeInBytes);
    // FIXME: This could be made much more efficient if we used the property size,
    // not the capacity.
    void* to = result->propertyStorage() - propertyCapacity;
    void* from = propertyStorage() - propertyCapacity;
    size_t size = std::min(
        totalSize(0, propertyCapacity, oldHasIndexingHeader, oldIndexingPayloadSizeInBytes),
        totalSize(0, propertyCapacity, newHasIndexingHeader, newIndexingPayloadSizeInBytes));
    memcpy(to, from, size);
    return result;
}

inline Butterfly* Butterfly::resizeArray(
    VM& vm, JSCell* intendedOwner, Structure* structure, size_t newPreCapacity,
    size_t newIndexingPayloadSizeInBytes)
{
    bool hasIndexingHeader = structure->hasIndexingHeader(intendedOwner);
    return resizeArray(
        vm, intendedOwner, structure->outOfLineCapacity(), hasIndexingHeader,
        indexingHeader()->indexingPayloadSizeInBytes(structure), newPreCapacity,
        hasIndexingHeader, newIndexingPayloadSizeInBytes);
}

inline Butterfly* Butterfly::unshift(Structure* structure, size_t numberOfSlots)
{
    ASSERT(hasAnyArrayStorage(structure->indexingType()));
    ASSERT(numberOfSlots <= indexingHeader()->preCapacity(structure));
    unsigned propertyCapacity = structure->outOfLineCapacity();
    // FIXME: It would probably be wise to rewrite this as a loop since (1) we know in which
    // direction we're moving memory so we don't need the extra check of memmove and (2) we're
    // moving a small amount of memory in the common case so the throughput of memmove won't
    // amortize the overhead of calling it. And no, we cannot rely on the C++ compiler to
    // inline memmove (particularly since the size argument is likely to be variable), nor can
    // we rely on the compiler to recognize the ordering of the pointer arguments (since
    // propertyCapacity is variable and could cause wrap-around as far as the compiler knows).
    memmove(
        propertyStorage() - numberOfSlots - propertyCapacity,
        propertyStorage() - propertyCapacity,
        sizeof(EncodedJSValue) * propertyCapacity + sizeof(IndexingHeader) + ArrayStorage::sizeFor(0));
    return IndexingHeader::fromEndOf(propertyStorage() - numberOfSlots)->butterfly();
}

inline Butterfly* Butterfly::shift(Structure* structure, size_t numberOfSlots)
{
    ASSERT(hasAnyArrayStorage(structure->indexingType()));
    unsigned propertyCapacity = structure->outOfLineCapacity();
    // FIXME: See comment in unshift(), above.
    memmove(
        propertyStorage() - propertyCapacity + numberOfSlots,
        propertyStorage() - propertyCapacity,
        sizeof(EncodedJSValue) * propertyCapacity + sizeof(IndexingHeader) + ArrayStorage::sizeFor(0));
    return IndexingHeader::fromEndOf(propertyStorage() + numberOfSlots)->butterfly();
}

} // namespace JSC
