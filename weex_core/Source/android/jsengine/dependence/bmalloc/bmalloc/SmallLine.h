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
#ifndef SmallLine_h
#define SmallLine_h

#include "BAssert.h"
#include "Mutex.h"
#include "ObjectType.h"
#include <mutex>

namespace bmalloc {

class SmallLine {
public:
    void ref(std::lock_guard<StaticMutex>&, unsigned char = 1);
    bool deref(std::lock_guard<StaticMutex>&);
    unsigned refCount(std::lock_guard<StaticMutex>&) { return m_refCount; }
    
    char* begin();
    char* end();

private:
    unsigned char m_refCount;

static_assert(
    smallLineSize / alignment <= std::numeric_limits<decltype(m_refCount)>::max(),
    "maximum object count must fit in SmallLine::m_refCount");

};

inline void SmallLine::ref(std::lock_guard<StaticMutex>&, unsigned char refCount)
{
    BASSERT(!m_refCount);
    m_refCount = refCount;
}

inline bool SmallLine::deref(std::lock_guard<StaticMutex>&)
{
    BASSERT(m_refCount);
    --m_refCount;
    return !m_refCount;
}

} // namespace bmalloc

#endif // SmallLine_h
