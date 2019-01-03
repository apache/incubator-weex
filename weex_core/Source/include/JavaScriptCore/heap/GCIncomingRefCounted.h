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

#include <wtf/DeferrableRefCounted.h>
#include <wtf/Vector.h>

namespace JSC {

class JSCell;

// A C-heap-allocated object that may have additional reference counts
// due to incoming references from the heap, which are tracked in
// reverse: the object knows its incoming references. Such objects also
// have the invariant that they don't have references back into the GC
// heap.

template<typename T>
class GCIncomingRefCounted : public DeferrableRefCounted<T> {
public:
    GCIncomingRefCounted()
        : m_encodedPointer(0)
    {
    }
    
    ~GCIncomingRefCounted()
    {
        if (hasVectorOfCells())
            delete vectorOfCells();
    }
    
    size_t numberOfIncomingReferences() const
    {
        if (!hasAnyIncoming())
            return 0;
        if (hasSingleton())
            return 1;
        return vectorOfCells()->size();
    }
    
    JSCell* incomingReferenceAt(size_t index) const
    {
        ASSERT(hasAnyIncoming());
        if (hasSingleton()) {
            ASSERT(!index);
            return singleton();
        }
        return vectorOfCells()->at(index);
    }
    
    // It's generally not a good idea to call this directly, since if this
    // returns true, you're supposed to add this object to the GC's list.
    // Call GCIncomingRefCountedSet::addReference() instead.
    bool addIncomingReference(JSCell*);
    
    // A filter function returns true if we wish to keep the incoming
    // reference, and false if we don't. This may delete the object,
    // and if it does so, this returns true. In general, you don't want
    // to use this with a filter function that can return false unless
    // you're also walking the GC's list.
    template<typename FilterFunctionType>
    bool filterIncomingReferences(FilterFunctionType&);
    
private:
    static uintptr_t singletonFlag() { return 1; }
    
    bool hasVectorOfCells() const { return !(m_encodedPointer & singletonFlag()); }
    bool hasAnyIncoming() const { return !!m_encodedPointer; }
    bool hasSingleton() const { return hasAnyIncoming() && !hasVectorOfCells(); }
    
    JSCell* singleton() const
    {
        ASSERT(hasSingleton());
        return bitwise_cast<JSCell*>(m_encodedPointer & ~singletonFlag());
    }
    
    Vector<JSCell*>* vectorOfCells() const
    {
        ASSERT(hasVectorOfCells());
        return bitwise_cast<Vector<JSCell*>*>(m_encodedPointer);
    }
    
    // Singleton flag is set: this is a JSCell*.
    // Singleton flag not set: this is a pointer to a vector of cells.
    uintptr_t m_encodedPointer;
};

} // namespace JSC
