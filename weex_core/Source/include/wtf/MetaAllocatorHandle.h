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
#ifndef WTF_MetaAllocatorHandle_h
#define WTF_MetaAllocatorHandle_h

#include <wtf/Assertions.h>
#include <wtf/RedBlackTree.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/RefPtr.h>

namespace WTF {

class MetaAllocator;
class PrintStream;

class MetaAllocatorHandle : public ThreadSafeRefCounted<MetaAllocatorHandle>, public RedBlackTree<MetaAllocatorHandle, void*>::Node {
private:
    MetaAllocatorHandle(MetaAllocator*, void* start, size_t sizeInBytes, void* ownerUID);
    
public:
    WTF_EXPORT_PRIVATE ~MetaAllocatorHandle();
    
    void* start() const
    {
        return m_start;
    }
    
    void* end() const
    {
        return reinterpret_cast<void*>(endAsInteger());
    }
    
    uintptr_t startAsInteger() const
    {
        return reinterpret_cast<uintptr_t>(m_start);
    }
    
    uintptr_t endAsInteger() const
    {
        return startAsInteger() + m_sizeInBytes;
    }
        
    size_t sizeInBytes() const
    {
        return m_sizeInBytes;
    }
    
    bool containsIntegerAddress(uintptr_t address) const
    {
        return address - startAsInteger() < sizeInBytes();
    }
    
    bool contains(void* address) const
    {
        return containsIntegerAddress(reinterpret_cast<uintptr_t>(address));
    }
        
    WTF_EXPORT_PRIVATE void shrink(size_t newSizeInBytes);
    
    bool isManaged()
    {
        return !!m_allocator;
    }
        
    MetaAllocator* allocator()
    {
        ASSERT(m_allocator);
        return m_allocator;
    }

    void* ownerUID()
    {
        return m_ownerUID;
    }

    void* key()
    {
        return m_start;
    }

    WTF_EXPORT_PRIVATE void dump(PrintStream& out) const;
    
private:
    friend class MetaAllocator;
    
    MetaAllocator* m_allocator;
    void* m_start;
    size_t m_sizeInBytes;
    void* m_ownerUID;
};

}

#endif
