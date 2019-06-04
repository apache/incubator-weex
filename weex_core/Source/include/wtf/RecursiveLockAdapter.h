/*
 * Copyright (C) 2016 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
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


