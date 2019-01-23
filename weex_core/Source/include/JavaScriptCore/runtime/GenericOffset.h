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

#include <limits.h>
#include <wtf/Assertions.h>

namespace JSC {

// A mixin for creating the various kinds of variable offsets that our engine supports.
template<typename T>
class GenericOffset {
    WTF_MAKE_FAST_ALLOCATED;
public:
    static const unsigned invalidOffset = UINT_MAX;
    
    GenericOffset()
        : m_offset(invalidOffset)
    {
    }
    
    explicit GenericOffset(unsigned offset)
        : m_offset(offset)
    {
    }
    
    bool operator!() const { return m_offset == invalidOffset; }
    
    unsigned offsetUnchecked() const
    {
        return m_offset;
    }
    
    unsigned offset() const
    {
        ASSERT(m_offset != invalidOffset);
        return m_offset;
    }
    
    bool operator==(const T& other) const
    {
        return m_offset == other.offsetUnchecked();
    }
    bool operator!=(const T& other) const
    {
        return m_offset != other.offsetUnchecked();
    }
    bool operator<(const T& other) const
    {
        return m_offset < other.offsetUnchecked();
    }
    bool operator>(const T& other) const
    {
        return m_offset > other.offsetUnchecked();
    }
    bool operator<=(const T& other) const
    {
        return m_offset <= other.offsetUnchecked();
    }
    bool operator>=(const T& other) const
    {
        return m_offset >= other.offsetUnchecked();
    }
    
    T operator+(int value) const
    {
        return T(offset() + value);
    }
    T operator-(int value) const
    {
        return T(offset() - value);
    }
    T& operator+=(int value)
    {
        return *this = *this + value;
    }
    T& operator-=(int value)
    {
        return *this = *this - value;
    }
    
private:
    unsigned m_offset;
};

} // namespace JSC
