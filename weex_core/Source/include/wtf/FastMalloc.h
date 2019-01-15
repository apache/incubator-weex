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
#ifndef WTF_FastMalloc_h
#define WTF_FastMalloc_h

#include <new>
#include <stdlib.h>
#include <wtf/StdLibExtras.h>

namespace WTF {

#if !defined(NDEBUG)
WTF_EXPORT_PRIVATE void fastSetMaxSingleAllocationSize(size_t);
#endif

class TryMallocReturnValue {
public:
    TryMallocReturnValue(void*);
    TryMallocReturnValue(TryMallocReturnValue&&);
    ~TryMallocReturnValue();
    template<typename T> bool getValue(T*&) WARN_UNUSED_RETURN;
private:
    void operator=(TryMallocReturnValue&&) = delete;
    mutable void* m_data;
};

WTF_EXPORT_PRIVATE bool isFastMallocEnabled();

// These functions call CRASH() if an allocation fails.
WTF_EXPORT_PRIVATE void* fastMalloc(size_t) RETURNS_NONNULL;
WTF_EXPORT_PRIVATE void* fastZeroedMalloc(size_t) RETURNS_NONNULL;
WTF_EXPORT_PRIVATE void* fastCalloc(size_t numElements, size_t elementSize) RETURNS_NONNULL;
WTF_EXPORT_PRIVATE void* fastRealloc(void*, size_t) RETURNS_NONNULL;
WTF_EXPORT_PRIVATE char* fastStrDup(const char*) RETURNS_NONNULL;

WTF_EXPORT_PRIVATE TryMallocReturnValue tryFastMalloc(size_t);
TryMallocReturnValue tryFastZeroedMalloc(size_t);
WTF_EXPORT_PRIVATE TryMallocReturnValue tryFastCalloc(size_t numElements, size_t elementSize);

WTF_EXPORT_PRIVATE void fastFree(void*);

// Allocations from fastAlignedMalloc() must be freed using fastAlignedFree().
WTF_EXPORT_PRIVATE void* fastAlignedMalloc(size_t alignment, size_t) RETURNS_NONNULL;
WTF_EXPORT_PRIVATE void* tryFastAlignedMalloc(size_t alignment, size_t);
WTF_EXPORT_PRIVATE void fastAlignedFree(void*);

WTF_EXPORT_PRIVATE size_t fastMallocSize(const void*);

// FIXME: This is non-helpful; fastMallocGoodSize will be removed soon.
WTF_EXPORT_PRIVATE size_t fastMallocGoodSize(size_t);

WTF_EXPORT_PRIVATE void releaseFastMallocFreeMemory();
WTF_EXPORT_PRIVATE void releaseFastMallocFreeMemoryForThisThread();

struct FastMallocStatistics {
    size_t reservedVMBytes;
    size_t committedVMBytes;
    size_t freeListBytes;
};
WTF_EXPORT_PRIVATE FastMallocStatistics fastMallocStatistics();

// This defines a type which holds an unsigned integer and is the same
// size as the minimally aligned memory allocation.
typedef unsigned long long AllocAlignmentInteger;

inline TryMallocReturnValue::TryMallocReturnValue(void* data)
    : m_data(data)
{
}

inline TryMallocReturnValue::TryMallocReturnValue(TryMallocReturnValue&& source)
    : m_data(source.m_data)
{
    source.m_data = nullptr;
}

inline TryMallocReturnValue::~TryMallocReturnValue()
{
    ASSERT(!m_data);
}

template<typename T> inline bool TryMallocReturnValue::getValue(T*& data)
{
    data = static_cast<T*>(m_data);
    m_data = nullptr;
    return data;
}

} // namespace WTF

#if !defined(NDEBUG)
using WTF::fastSetMaxSingleAllocationSize;
#endif

using WTF::isFastMallocEnabled;
using WTF::fastCalloc;
using WTF::fastFree;
using WTF::fastMalloc;
using WTF::fastMallocGoodSize;
using WTF::fastMallocSize;
using WTF::fastRealloc;
using WTF::fastStrDup;
using WTF::fastZeroedMalloc;
using WTF::tryFastAlignedMalloc;
using WTF::tryFastCalloc;
using WTF::tryFastMalloc;
using WTF::tryFastZeroedMalloc;
using WTF::fastAlignedMalloc;
using WTF::fastAlignedFree;

#if COMPILER(GCC_OR_CLANG) && OS(DARWIN)
#define WTF_PRIVATE_INLINE __private_extern__ inline __attribute__((always_inline))
#elif COMPILER(GCC_OR_CLANG)
#define WTF_PRIVATE_INLINE inline __attribute__((always_inline))
#elif COMPILER(MSVC)
#define WTF_PRIVATE_INLINE __forceinline
#else
#define WTF_PRIVATE_INLINE inline
#endif

#define WTF_MAKE_FAST_ALLOCATED \
public: \
    void* operator new(size_t, void* p) { return p; } \
    void* operator new[](size_t, void* p) { return p; } \
    \
    void* operator new(size_t size) \
    { \
        return ::WTF::fastMalloc(size); \
    } \
    \
    void operator delete(void* p) \
    { \
        ::WTF::fastFree(p); \
    } \
    \
    void* operator new[](size_t size) \
    { \
        return ::WTF::fastMalloc(size); \
    } \
    \
    void operator delete[](void* p) \
    { \
        ::WTF::fastFree(p); \
    } \
    void* operator new(size_t, NotNullTag, void* location) \
    { \
        ASSERT(location); \
        return location; \
    } \
private: \
typedef int __thisIsHereToForceASemicolonAfterThisMacro

#endif /* WTF_FastMalloc_h */
