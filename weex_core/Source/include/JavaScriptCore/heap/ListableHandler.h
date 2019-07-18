/*
 *  Copyright (C) 2011-2016 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
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
