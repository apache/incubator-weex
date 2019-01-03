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

#include <stdint.h>
#include <wtf/Lock.h>
#include <wtf/Locker.h>
#include <wtf/Noncopyable.h>
#include <wtf/ThreadingPrimitives.h>

namespace JSC {

class Heap;
class SlotVisitor;

template<typename T>
class ListableHandler {
    WTF_MAKE_NONCOPYABLE(ListableHandler);

public:    
    bool isOnList() const
    {
        return m_nextAndFlag & 1;
    }
    
protected:
    ListableHandler()
        : m_nextAndFlag(0)
    {
    }
    
    virtual ~ListableHandler() { }
    
    T* next() const
    {
        return reinterpret_cast<T*>(m_nextAndFlag & ~1);
    }

private:
    // Allow these classes to use ListableHandler::List.
    friend class Heap;
    friend class SlotVisitor;
    
    class List {
        WTF_MAKE_NONCOPYABLE(List);
    public:
        List()
            : m_first(0)
        {
        }
        
        void addThreadSafe(T* handler)
        {
            LockHolder locker(&m_lock);
            addNotThreadSafe(handler);
        }
        
        bool hasNext()
        {
            return !!m_first;
        }
        
        T* head()
        {
            return m_first;
        }
        
        T* removeNext()
        {
            T* current = m_first;
            T* next = current->next();
            current->m_nextAndFlag = 0;
            m_first = next;
            return current;
        }
        
        void removeAll()
        {
            while (hasNext())
                removeNext();
        }
        
    private:
        void addNotThreadSafe(T* handler)
        {
            if (handler->m_nextAndFlag & 1)
                return;
            handler->m_nextAndFlag = reinterpret_cast<uintptr_t>(m_first) | 1;
            m_first = handler;
        }
        
        Lock m_lock;
        T* m_first;
    };
    
    uintptr_t m_nextAndFlag;
};

} // namespace JSC
