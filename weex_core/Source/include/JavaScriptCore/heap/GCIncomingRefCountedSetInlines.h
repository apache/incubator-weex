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

#include "GCIncomingRefCountedSet.h"
#include "VM.h"

namespace JSC {

template<typename T>
GCIncomingRefCountedSet<T>::GCIncomingRefCountedSet()
    : m_bytes(0)
{
}

template<typename T>
void GCIncomingRefCountedSet<T>::lastChanceToFinalize()
{
    for (size_t i = m_vector.size(); i--;)
        m_vector[i]->filterIncomingReferences(removeAll);
}

template<typename T>
bool GCIncomingRefCountedSet<T>::addReference(JSCell* cell, T* object)
{
    if (!object->addIncomingReference(cell)) {
        ASSERT(object->isDeferred());
        ASSERT(object->numberOfIncomingReferences());
        return false;
    }
    m_vector.append(object);
    m_bytes += object->gcSizeEstimateInBytes();
    ASSERT(object->isDeferred());
    ASSERT(object->numberOfIncomingReferences());
    return true;
}

template<typename T>
void GCIncomingRefCountedSet<T>::sweep()
{
    for (size_t i = 0; i < m_vector.size(); ++i) {
        T* object = m_vector[i];
        size_t size = object->gcSizeEstimateInBytes();
        ASSERT(object->isDeferred());
        ASSERT(object->numberOfIncomingReferences());
        if (!object->filterIncomingReferences(removeDead))
            continue;
        m_bytes -= size;
        m_vector[i--] = m_vector.last();
        m_vector.removeLast();
    }
}

template<typename T>
bool GCIncomingRefCountedSet<T>::removeAll(JSCell*)
{
    return false;
}

template<typename T>
bool GCIncomingRefCountedSet<T>::removeDead(JSCell* cell)
{
    return Heap::isMarked(cell);
}

} // namespace JSC
