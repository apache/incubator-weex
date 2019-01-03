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

#include "GCIncomingRefCounted.h"
#include "Heap.h"

namespace JSC {

template<typename T>
bool GCIncomingRefCounted<T>::addIncomingReference(JSCell* cell)
{
    if (!hasAnyIncoming()) {
        m_encodedPointer = bitwise_cast<uintptr_t>(cell) | singletonFlag();
        this->setIsDeferred(true);
        ASSERT(hasSingleton());
        return true;
    }
    
    ASSERT(Heap::heap(incomingReferenceAt(0)) == Heap::heap(cell));
    
    if (hasSingleton()) {
        Vector<JSCell*>* vector = new Vector<JSCell*>();
        vector->append(singleton());
        vector->append(cell);
        m_encodedPointer = bitwise_cast<uintptr_t>(vector);
        ASSERT(hasVectorOfCells());
        return false;
    }
    
    vectorOfCells()->append(cell);
    return false;
}

template<typename T>
template<typename FilterFunctionType>
bool GCIncomingRefCounted<T>::filterIncomingReferences(FilterFunctionType& filterFunction)
{
    const bool verbose = false;
    
    if (verbose)
        dataLog("Filtering incoming references.\n");
    
    if (!hasAnyIncoming()) {
        ASSERT(!this->isDeferred());
        ASSERT(this->refCount());
        if (verbose)
            dataLog("    Has no incoming.\n");
        return false;
    }
    
    ASSERT(this->isDeferred());
    
    if (hasSingleton()) {
        if (filterFunction(singleton())) {
            if (verbose)
                dataLog("   Singleton passed.\n");
            return false;
        }
        
        if (verbose)
            dataLog("   Removing singleton.\n");
        m_encodedPointer = 0;
        ASSERT(!hasAnyIncoming());
        this->setIsDeferred(false);
        return true;
    }
    
    if (verbose)
        dataLog("   Has ", vectorOfCells()->size(), " entries.\n");
    for (size_t i = 0; i < vectorOfCells()->size(); ++i) {
        if (filterFunction(vectorOfCells()->at(i)))
            continue;
        vectorOfCells()->at(i--) = vectorOfCells()->last();
        vectorOfCells()->removeLast();
    }
    
    if (vectorOfCells()->size() >= 2) {
        if (verbose)
            dataLog("   Still has ", vectorOfCells()->size(), " entries.\n");
        return false;
    }
    
    if (vectorOfCells()->isEmpty()) {
        if (verbose)
            dataLog("   Removing.\n");
        delete vectorOfCells();
        m_encodedPointer = 0;
        ASSERT(!hasAnyIncoming());
        this->setIsDeferred(false);
        return true;
    }
    
    if (verbose)
        dataLog("   Shrinking to singleton.\n");
    JSCell* singleton = vectorOfCells()->at(0);
    delete vectorOfCells();
    m_encodedPointer = bitwise_cast<uintptr_t>(singleton) | singletonFlag();
    ASSERT(hasSingleton());
    return false;
}

} // namespace JSC
