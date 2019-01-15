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

#include <wtf/Threading.h>

namespace WTF {

template<typename LockType>
class RecursiveLockAdapter {
public:
    RecursiveLockAdapter()
    {
    }
    
    void lock()
    {
        ThreadIdentifier me = currentThread();
        if (me == m_owner) {
            m_recursionCount++;
            return;
        }
        
        m_lock.lock();
        ASSERT(!m_owner);
        ASSERT(!m_recursionCount);
        m_owner = me;
        m_recursionCount = 1;
    }
    
    void unlock()
    {
        if (--m_recursionCount)
            return;
        m_owner = 0;
        m_lock.unlock();
    }
    
    bool tryLock()
    {
        ThreadIdentifier me = currentThread();
        if (me == m_owner) {
            m_recursionCount++;
            return true;
        }
        
        if (!m_lock.tryLock())
            return false;
        
        ASSERT(!m_owner);
        ASSERT(!m_recursionCount);
        m_owner = me;
        m_recursionCount = 1;
        return true;
    }
    
    bool isLocked() const
    {
        return m_lock.isLocked();
    }
    
private:
    ThreadIdentifier m_owner { 0 };
    unsigned m_recursionCount { 0 };
    LockType m_lock;
};

} // namespace WTF


