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
#ifndef PageBlock_h
#define PageBlock_h

namespace WTF {

WTF_EXPORT_PRIVATE size_t pageSize();
WTF_EXPORT_PRIVATE size_t pageMask();
inline bool isPageAligned(void* address) { return !(reinterpret_cast<intptr_t>(address) & (pageSize() - 1)); }
inline bool isPageAligned(size_t size) { return !(size & (pageSize() - 1)); }
inline bool isPowerOfTwo(size_t size) { return !(size & (size - 1)); }

class PageBlock {
public:
    PageBlock();
    PageBlock(const PageBlock&);
    PageBlock(void*, size_t, bool hasGuardPages);
    
    void* base() const { return m_base; }
    size_t size() const { return m_size; }

    operator bool() const { return !!m_realBase; }

    bool contains(void* containedBase, size_t containedSize)
    {
        return containedBase >= m_base
            && (static_cast<char*>(containedBase) + containedSize) <= (static_cast<char*>(m_base) + m_size);
    }

private:
    void* m_realBase;
    void* m_base;
    size_t m_size;
};

inline PageBlock::PageBlock()
    : m_realBase(0)
    , m_base(0)
    , m_size(0)
{
}

inline PageBlock::PageBlock(const PageBlock& other)
    : m_realBase(other.m_realBase)
    , m_base(other.m_base)
    , m_size(other.m_size)
{
}

inline PageBlock::PageBlock(void* base, size_t size, bool hasGuardPages)
    : m_realBase(base)
    , m_base(static_cast<char*>(base) + ((base && hasGuardPages) ? pageSize() : 0))
    , m_size(size)
{
}

} // namespace WTF

using WTF::pageSize;
using WTF::isPageAligned;
using WTF::isPowerOfTwo;

#endif // PageBlock_h
