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

#include "ArrayConventions.h"
#include "Butterfly.h"
#include "IndexingHeader.h"
#include "MarkedSpace.h"
#include "SparseArrayValueMap.h"
#include "Structure.h"
#include "WriteBarrier.h"
#include <wtf/Noncopyable.h>

namespace JSC {

// This struct holds the actual data values of an array. A JSArray object points to its contained ArrayStorage
// struct by pointing to m_vector. To access the contained ArrayStorage struct, use the getStorage() and 
// setStorage() methods. It is important to note that there may be space before the ArrayStorage that 
// is used to quick unshift / shift operation. The actual allocated pointer is available by using:
//     getStorage() - m_indexBias * sizeof(JSValue)
// All slots in ArrayStorage (slots from 0 to vectorLength) are expected to be initialized to a JSValue or,
// for hole slots, JSValue().
struct ArrayStorage {
    WTF_MAKE_NONCOPYABLE(ArrayStorage);
private:
    ArrayStorage() { } // Not directly instantiable. Can only be created as part of a Butterfly.
public:
    
    static ArrayStorage* from(Butterfly* butterfly) { return reinterpret_cast_ptr<ArrayStorage*>(butterfly); }
    static ArrayStorage* from(IndexingHeader* indexingHeader) { return indexingHeader->arrayStorage(); }
    
    Butterfly* butterfly() { return reinterpret_cast<Butterfly*>(this); }
    IndexingHeader* indexingHeader() { return IndexingHeader::from(this); }
    const IndexingHeader* indexingHeader() const { return IndexingHeader::from(this); }
    
    // We steal two fields from the indexing header: vectorLength and length.
    unsigned length() const { return indexingHeader()->publicLength(); }
    void setLength(unsigned length) { indexingHeader()->setPublicLength(length); }
    unsigned vectorLength() const { return indexingHeader()->vectorLength(); }
    void setVectorLength(unsigned length) { indexingHeader()->setVectorLength(length); }
    
    ALWAYS_INLINE void copyHeaderFromDuringGC(const ArrayStorage& other)
    {
        m_sparseMap.copyFrom(other.m_sparseMap);
        m_indexBias = other.m_indexBias;
        m_numValuesInVector = other.m_numValuesInVector;
    }
    
    bool hasHoles() const
    {
        return m_numValuesInVector != length();
    }   

    bool inSparseMode()
    {
        return m_sparseMap && m_sparseMap->sparseMode();
    }

    ContiguousJSValues vector() { return ContiguousJSValues(m_vector, vectorLength()); }

    WriteBarrier<SparseArrayValueMap> m_sparseMap;
    unsigned m_indexBias;
    unsigned m_numValuesInVector;
#if USE(JSVALUE32_64)
    uintptr_t m_padding;
#endif
    WriteBarrier<Unknown> m_vector[1];
    
    static ptrdiff_t lengthOffset() { return Butterfly::offsetOfPublicLength(); }
    static ptrdiff_t vectorLengthOffset() { return Butterfly::offsetOfVectorLength(); }
    static ptrdiff_t numValuesInVectorOffset() { return OBJECT_OFFSETOF(ArrayStorage, m_numValuesInVector); }
    static ptrdiff_t vectorOffset() { return OBJECT_OFFSETOF(ArrayStorage, m_vector); }
    static ptrdiff_t indexBiasOffset() { return OBJECT_OFFSETOF(ArrayStorage, m_indexBias); }
    static ptrdiff_t sparseMapOffset() { return OBJECT_OFFSETOF(ArrayStorage, m_sparseMap); }
    
    static size_t sizeFor(unsigned vectorLength)
    {
        return ArrayStorage::vectorOffset() + vectorLength * sizeof(WriteBarrier<Unknown>);
    }
    
    static size_t totalSizeFor(unsigned indexBias, size_t propertyCapacity, unsigned vectorLength)
    {
        return Butterfly::totalSize(indexBias, propertyCapacity, true, sizeFor(vectorLength));
    }
    
    size_t totalSize(size_t propertyCapacity) const
    {
        return totalSizeFor(m_indexBias, propertyCapacity, vectorLength());
    }
    
    size_t totalSize(Structure* structure) const
    {
        return totalSize(structure->outOfLineCapacity());
    }
    
    static unsigned availableVectorLength(unsigned indexBias, size_t propertyCapacity, unsigned vectorLength)
    {
        size_t cellSize = MarkedSpace::optimalSizeFor(totalSizeFor(indexBias, propertyCapacity, vectorLength));
        
        vectorLength = (cellSize - totalSizeFor(indexBias, propertyCapacity, 0)) / sizeof(WriteBarrier<Unknown>);

        return vectorLength;
    }
    
    static unsigned availableVectorLength(unsigned indexBias, Structure* structure, unsigned vectorLength)
    {
        return availableVectorLength(indexBias, structure->outOfLineCapacity(), vectorLength);
    }
    
    unsigned availableVectorLength(size_t propertyCapacity, unsigned vectorLength)
    {
        return availableVectorLength(m_indexBias, propertyCapacity, vectorLength);
    }
    
    unsigned availableVectorLength(Structure* structure, unsigned vectorLength)
    {
        return availableVectorLength(structure->outOfLineCapacity(), vectorLength);
    }

    static unsigned optimalVectorLength(unsigned indexBias, size_t propertyCapacity, unsigned vectorLength)
    {
        vectorLength = std::max(BASE_ARRAY_STORAGE_VECTOR_LEN, vectorLength);
        return availableVectorLength(indexBias, propertyCapacity, vectorLength);
    }
    
    static unsigned optimalVectorLength(unsigned indexBias, Structure* structure, unsigned vectorLength)
    {
        return optimalVectorLength(indexBias, structure->outOfLineCapacity(), vectorLength);
    }
    
    unsigned optimalVectorLength(size_t propertyCapacity, unsigned vectorLength)
    {
        return optimalVectorLength(m_indexBias, propertyCapacity, vectorLength);
    }
    
    unsigned optimalVectorLength(Structure* structure, unsigned vectorLength)
    {
        return optimalVectorLength(structure->outOfLineCapacity(), vectorLength);
    }
};

} // namespace JSC
