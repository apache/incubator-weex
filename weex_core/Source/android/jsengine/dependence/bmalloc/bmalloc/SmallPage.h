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
#ifndef SmallPage_h
#define SmallPage_h

#include "BAssert.h"
#include "List.h"
#include "Mutex.h"
#include "VMAllocate.h"
#include <mutex>

namespace bmalloc {

class SmallPage : public ListNode<SmallPage> {
public:
    SmallPage()
        : m_hasFreeLines(true)
    {
    }

    void ref(std::lock_guard<StaticMutex>&);
    bool deref(std::lock_guard<StaticMutex>&);
    unsigned refCount(std::lock_guard<StaticMutex>&) { return m_refCount; }
    
    size_t sizeClass() { return m_sizeClass; }
    void setSizeClass(size_t sizeClass) { m_sizeClass = sizeClass; }
    
    bool hasFreeLines(std::lock_guard<StaticMutex>&) const { return m_hasFreeLines; }
    void setHasFreeLines(std::lock_guard<StaticMutex>&, bool hasFreeLines) { m_hasFreeLines = hasFreeLines; }
    
    SmallLine* begin();

    unsigned char slide() const { return m_slide; }
    void setSlide(unsigned char slide) { m_slide = slide; }
    
private:
    unsigned char m_hasFreeLines: 1;
    unsigned char m_refCount: 7;
    unsigned char m_sizeClass;
    unsigned char m_slide;

static_assert(
    sizeClassCount <= std::numeric_limits<decltype(m_sizeClass)>::max(),
    "Largest size class must fit in SmallPage metadata");
};

inline void SmallPage::ref(std::lock_guard<StaticMutex>&)
{
    BASSERT(!m_slide);
    ++m_refCount;
    BASSERT(m_refCount);
}

inline bool SmallPage::deref(std::lock_guard<StaticMutex>&)
{
    BASSERT(!m_slide);
    BASSERT(m_refCount);
    --m_refCount;
    return !m_refCount;
}

} // namespace bmalloc

#endif // SmallPage_h
