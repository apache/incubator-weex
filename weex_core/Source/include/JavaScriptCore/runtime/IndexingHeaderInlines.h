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
#include "IndexingHeader.h"
#include "Structure.h"

namespace JSC {

inline size_t IndexingHeader::preCapacity(Structure* structure)
{
    if (LIKELY(!hasAnyArrayStorage(structure->indexingType())))
        return 0;
    
    return arrayStorage()->m_indexBias;
}

inline size_t IndexingHeader::indexingPayloadSizeInBytes(Structure* structure)
{
    switch (structure->indexingType()) {
    case ALL_UNDECIDED_INDEXING_TYPES:
    case ALL_INT32_INDEXING_TYPES:
    case ALL_DOUBLE_INDEXING_TYPES:
    case ALL_CONTIGUOUS_INDEXING_TYPES:
        return vectorLength() * sizeof(EncodedJSValue);
        
    case ALL_ARRAY_STORAGE_INDEXING_TYPES:
        return ArrayStorage::sizeFor(arrayStorage()->vectorLength());
        
    default:
        ASSERT(!hasIndexedProperties(structure->indexingType()));
        return 0;
    }
}

} // namespace JSC
