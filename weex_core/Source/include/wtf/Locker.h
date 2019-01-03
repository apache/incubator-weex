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
#ifndef Locker_h
#define Locker_h

#include <wtf/Assertions.h>
#include <wtf/Noncopyable.h>

namespace WTF {

enum NoLockingNecessaryTag { NoLockingNecessary };

class AbstractLocker {
    WTF_MAKE_NONCOPYABLE(AbstractLocker);
public:
    AbstractLocker(NoLockingNecessaryTag)
    {
    }
    
protected:
    AbstractLocker()
    {
    }
};

template <typename T> class Locker : public AbstractLocker {
public:
    explicit Locker(T& lockable) : m_lockable(&lockable) { lock(); }
    explicit Locker(T* lockable) : m_lockable(lockable) { lock(); }

    // You should be wary of using this constructor. It's only applicable
    // in places where there is a locking protocol for a particular object
    // but it's not necessary to engage in that protocol yet. For example,
    // this often happens when an object is newly allocated and it can not
    // be accessed concurrently.
    Locker(NoLockingNecessaryTag) : m_lockable(nullptr) { }
    
    Locker(int) = delete;

    ~Locker()
    {
        if (m_lockable)
            m_lockable->unlock();
    }
    
    static Locker tryLock(T& lockable)
    {
        Locker result(NoLockingNecessary);
        if (lockable.tryLock()) {
            result.m_lockable = &lockable;
            return result;
        }
        return result;
    }
    
    explicit operator bool() const { return !!m_lockable; }
    
    void unlockEarly()
    {
        m_lockable->unlock();
        m_lockable = 0;
    }
    
    // It's great to be able to pass lockers around. It enables custom locking adaptors like
    // JSC::LockDuringMarking.
    Locker(Locker&& other)
        : m_lockable(other.m_lockable)
    {
        other.m_lockable = nullptr;
    }
    
    Locker& operator=(Locker&& other)
    {
        if (m_lockable)
            m_lockable->unlock();
        m_lockable = other.m_lockable;
        other.m_lockable = nullptr;
        return *this;
    }
    
private:
    void lock()
    {
        if (m_lockable)
            m_lockable->lock();
    }
    
    T* m_lockable;
};

// Use this lock scope like so:
// auto locker = holdLock(lock);
template<typename LockType>
Locker<LockType> holdLock(LockType& lock)
{
    return Locker<LockType>(lock);
}

template<typename LockType>
Locker<LockType> tryHoldLock(LockType& lock)
{
    return Locker<LockType>::tryLock(lock);
}

}

using WTF::AbstractLocker;
using WTF::Locker;
using WTF::NoLockingNecessaryTag;
using WTF::NoLockingNecessary;
using WTF::holdLock;

#endif
