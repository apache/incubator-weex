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
#ifndef Atomics_h
#define Atomics_h

#include <atomic>
#include <wtf/StdLibExtras.h>

#if OS(WINDOWS)
#if !COMPILER(GCC_OR_CLANG)
extern "C" void _ReadWriteBarrier(void);
#pragma intrinsic(_ReadWriteBarrier)
#endif
#include <windows.h>
#include <intrin.h>
#endif

namespace WTF {

ALWAYS_INLINE bool hasFence(std::memory_order order)
{
    return order != std::memory_order_relaxed;
}

enum class TransactionAbortLikelihood {
    Unlikely,
    Likely
};
    
// Atomic wraps around std::atomic with the sole purpose of making the compare_exchange
// operations not alter the expected value. This is more in line with how we typically
// use CAS in our code.
//
// Atomic is a struct without explicitly defined constructors so that it can be
// initialized at compile time.

template<typename T>
struct Atomic {
    // Don't pass a non-default value for the order parameter unless you really know
    // what you are doing and have thought about it very hard. The cost of seq_cst
    // is usually not high enough to justify the risk.

    ALWAYS_INLINE T load(std::memory_order order = std::memory_order_seq_cst) const { return value.load(order); }
    
    ALWAYS_INLINE T loadRelaxed() const { return load(std::memory_order_relaxed); }

    ALWAYS_INLINE void store(T desired, std::memory_order order = std::memory_order_seq_cst) { value.store(desired, order); }

    ALWAYS_INLINE bool compareExchangeWeak(T expected, T desired, std::memory_order order = std::memory_order_seq_cst)
    {
        T expectedOrActual = expected;
        return value.compare_exchange_weak(expectedOrActual, desired, order);
    }

    ALWAYS_INLINE bool compareExchangeWeakRelaxed(T expected, T desired)
    {
        return compareExchangeWeak(expected, desired, std::memory_order_relaxed);
    }

    ALWAYS_INLINE bool compareExchangeWeak(T expected, T desired, std::memory_order order_success, std::memory_order order_failure)
    {
        T expectedOrActual = expected;
        return value.compare_exchange_weak(expectedOrActual, desired, order_success, order_failure);
    }

    ALWAYS_INLINE T compareExchangeStrong(T expected, T desired, std::memory_order order = std::memory_order_seq_cst)
    {
        T expectedOrActual = expected;
        value.compare_exchange_strong(expectedOrActual, desired, order);
        return expectedOrActual;
    }

    ALWAYS_INLINE T compareExchangeStrong(T expected, T desired, std::memory_order order_success, std::memory_order order_failure)
    {
        T expectedOrActual = expected;
        value.compare_exchange_strong(expectedOrActual, desired, order_success, order_failure);
        return expectedOrActual;
    }

    template<typename U>
    ALWAYS_INLINE T exchangeAdd(U operand, std::memory_order order = std::memory_order_seq_cst) { return value.fetch_add(operand, order); }
    
    template<typename U>
    ALWAYS_INLINE T exchangeAnd(U operand, std::memory_order order = std::memory_order_seq_cst) { return value.fetch_and(operand, order); }
    
    template<typename U>
    ALWAYS_INLINE T exchangeOr(U operand, std::memory_order order = std::memory_order_seq_cst) { return value.fetch_or(operand, order); }
    
    template<typename U>
    ALWAYS_INLINE T exchangeSub(U operand, std::memory_order order = std::memory_order_seq_cst) { return value.fetch_sub(operand, order); }
    
    template<typename U>
    ALWAYS_INLINE T exchangeXor(U operand, std::memory_order order = std::memory_order_seq_cst) { return value.fetch_xor(operand, order); }
    
    ALWAYS_INLINE T exchange(T newValue, std::memory_order order = std::memory_order_seq_cst) { return value.exchange(newValue, order); }
    
#if HAVE(LL_SC)
    ALWAYS_INLINE T loadLink(std::memory_order order = std::memory_order_seq_cst);
    ALWAYS_INLINE bool storeCond(T value,  std::memory_order order = std::memory_order_seq_cst);
#endif // HAVE(LL_SC)

    ALWAYS_INLINE T prepare(std::memory_order order = std::memory_order_seq_cst)
    {
#if HAVE(LL_SC)
        return loadLink(order);
#else
        UNUSED_PARAM(order);
        return load(std::memory_order_relaxed);
#endif
    }
    
#if HAVE(LL_SC)
    static const bool prepareIsFast = false;
#else
    static const bool prepareIsFast = true;
#endif

    ALWAYS_INLINE bool attempt(T oldValue, T newValue, std::memory_order order = std::memory_order_seq_cst)
    {
#if HAVE(LL_SC)
        UNUSED_PARAM(oldValue);
        return storeCond(newValue, order);
#else
        return compareExchangeWeak(oldValue, newValue, order);
#endif
    }

    template<typename Func>
    ALWAYS_INLINE bool transaction(const Func& func, std::memory_order order = std::memory_order_seq_cst, TransactionAbortLikelihood abortLikelihood = TransactionAbortLikelihood::Likely)
    {
        // If preparing is not fast then we want to skip the loop when func would fail.
        if (!prepareIsFast && abortLikelihood == TransactionAbortLikelihood::Likely) {
            T oldValue = load(std::memory_order_relaxed);
            // Note: many funcs will constant-fold to true, which will kill all of this code.
            if (!func(oldValue))
                return false;
        }
        for (;;) {
            T oldValue = prepare(order);
            T newValue = oldValue;
            if (!func(newValue))
                return false;
            if (attempt(oldValue, newValue, order))
                return true;
        }
    }

    template<typename Func>
    ALWAYS_INLINE bool transactionRelaxed(const Func& func, TransactionAbortLikelihood abortLikelihood = TransactionAbortLikelihood::Likely)
    {
        return transaction(func, std::memory_order_relaxed, abortLikelihood);
    }

    std::atomic<T> value;
};

#if CPU(ARM64) && HAVE(LL_SC)
#define DEFINE_LL_SC(width, modifier, suffix)   \
    template<> \
    ALWAYS_INLINE uint ## width ## _t Atomic<uint ## width ##_t>::loadLink(std::memory_order order) \
    { \
        int ## width ## _t result; \
        if (hasFence(order)) { \
            asm volatile ( \
                "ldaxr" suffix " %" modifier "0, [%1]" \
                : "=r"(result) \
                : "r"(this) \
                : "memory"); \
        } else { \
            asm ( \
                "ldxr" suffix " %" modifier "0, [%1]" \
                : "=r"(result) \
                : "r"(this) \
                : "memory"); \
        } \
        return result; \
    } \
    \
    template<> \
    ALWAYS_INLINE bool Atomic<uint ## width ## _t>::storeCond(uint ## width ## _t value, std::memory_order order) \
    { \
        bool result; \
        if (hasFence(order)) { \
            asm volatile ( \
                "stlxr" suffix " %w0, %" modifier "1, [%2]" \
                : "=&r"(result) \
                : "r"(value), "r"(this) \
                : "memory"); \
        } else { \
            asm ( \
                "stxr" suffix " %w0, %" modifier "1, [%2]" \
                : "=&r"(result) \
                : "r"(value), "r"(this) \
                : "memory"); \
        } \
        return !result; \
    } \
    \
    template<> \
    ALWAYS_INLINE int ## width ## _t Atomic<int ## width ## _t>::loadLink(std::memory_order order) \
    { \
        return bitwise_cast<Atomic<uint ## width ## _t>*>(this)->loadLink(order); \
    } \
    \
    template<> \
    ALWAYS_INLINE bool Atomic<int ## width ## _t>::storeCond(int ## width ## _t value, std::memory_order order) \
    { \
        return bitwise_cast<Atomic<uint ## width ## _t>*>(this)->storeCond(value, order); \
    }

DEFINE_LL_SC(8, "w", "b")
DEFINE_LL_SC(16, "w", "h")
DEFINE_LL_SC(32, "w", "")
DEFINE_LL_SC(64, "", "")
#if OS(DARWIN)
DEFINE_LL_SC(ptr, "", "")
#endif

#undef DEFINE_LL_SC
#endif // CPU(ARM64) && HAVE(LL_SC)

template<typename T>
inline T atomicLoad(T* location, std::memory_order order = std::memory_order_seq_cst)
{
    return bitwise_cast<Atomic<T>*>(location)->load(order);
}

template<typename T>
inline void atomicStore(T* location, T newValue, std::memory_order order = std::memory_order_seq_cst)
{
    bitwise_cast<Atomic<T>*>(location)->store(newValue, order);
}

template<typename T>
inline bool atomicCompareExchangeWeak(T* location, T expected, T newValue, std::memory_order order = std::memory_order_seq_cst)
{
    return bitwise_cast<Atomic<T>*>(location)->compareExchangeWeak(expected, newValue, order);
}

template<typename T>
inline bool atomicCompareExchangeWeakRelaxed(T* location, T expected, T newValue)
{
    return bitwise_cast<Atomic<T>*>(location)->compareExchangeWeakRelaxed(expected, newValue);
}

template<typename T>
inline T atomicCompareExchangeStrong(T* location, T expected, T newValue, std::memory_order order = std::memory_order_seq_cst)
{
    return bitwise_cast<Atomic<T>*>(location)->compareExchangeStrong(expected, newValue, order);
}

template<typename T, typename U>
inline T atomicExchangeAdd(T* location, U operand, std::memory_order order = std::memory_order_seq_cst)
{
    return bitwise_cast<Atomic<T>*>(location)->exchangeAdd(operand, order);
}

template<typename T, typename U>
inline T atomicExchangeAnd(T* location, U operand, std::memory_order order = std::memory_order_seq_cst)
{
    return bitwise_cast<Atomic<T>*>(location)->exchangeAnd(operand, order);
}

template<typename T, typename U>
inline T atomicExchangeOr(T* location, U operand, std::memory_order order = std::memory_order_seq_cst)
{
    return bitwise_cast<Atomic<T>*>(location)->exchangeOr(operand, order);
}

template<typename T, typename U>
inline T atomicExchangeSub(T* location, U operand, std::memory_order order = std::memory_order_seq_cst)
{
    return bitwise_cast<Atomic<T>*>(location)->exchangeSub(operand, order);
}

template<typename T, typename U>
inline T atomicExchangeXor(T* location, U operand, std::memory_order order = std::memory_order_seq_cst)
{
    return bitwise_cast<Atomic<T>*>(location)->exchangeXor(operand, order);
}

template<typename T>
inline T atomicExchange(T* location, T newValue, std::memory_order order = std::memory_order_seq_cst)
{
    return bitwise_cast<Atomic<T>*>(location)->exchange(newValue, order);
}

// Just a compiler fence. Has no effect on the hardware, but tells the compiler
// not to move things around this call. Should not affect the compiler's ability
// to do things like register allocation and code motion over pure operations.
inline void compilerFence()
{
#if OS(WINDOWS) && !COMPILER(GCC_OR_CLANG)
    _ReadWriteBarrier();
#else
    asm volatile("" ::: "memory");
#endif
}

#if CPU(ARM_THUMB2) || CPU(ARM64)

// Full memory fence. No accesses will float above this, and no accesses will sink
// below it.
inline void arm_dmb()
{
    asm volatile("dmb ish" ::: "memory");
}

// Like the above, but only affects stores.
inline void arm_dmb_st()
{
    asm volatile("dmb ishst" ::: "memory");
}

inline void arm_isb()
{
    asm volatile("isb" ::: "memory");
}

inline void loadLoadFence() { arm_dmb(); }
inline void loadStoreFence() { arm_dmb(); }
inline void storeLoadFence() { arm_dmb(); }
inline void storeStoreFence() { arm_dmb_st(); }
inline void memoryBarrierAfterLock() { arm_dmb(); }
inline void memoryBarrierBeforeUnlock() { arm_dmb(); }
inline void crossModifyingCodeFence() { arm_isb(); }

#elif CPU(X86) || CPU(X86_64)

inline void x86_ortop()
{
#if OS(WINDOWS)
    MemoryBarrier();
#elif CPU(X86_64)
    // This has acqrel semantics and is much cheaper than mfence. For exampe, in the JSC GC, using
    // mfence as a store-load fence was a 9% slow-down on Octane/splay while using this was neutral.
    asm volatile("lock; orl $0, (%%rsp)" ::: "memory");
#else
    asm volatile("lock; orl $0, (%%esp)" ::: "memory");
#endif
}

inline void x86_cpuid()
{
#if OS(WINDOWS)
    int info[4];
    __cpuid(info, 0);
#elif CPU(X86)
    // GCC 4.9 on x86 in PIC mode can't use %ebx, so we have to save and restore it manually.
    // But since we don't care about what cpuid returns (we use it as a serializing instruction),
    // we can simply throw away what cpuid put in %ebx.
    intptr_t a = 0, c, d;
    asm volatile(
        "pushl %%ebx\n\t"
        "cpuid\n\t"
        "popl %%ebx\n\t"
        : "+a"(a), "=c"(c), "=d"(d)
        :
        : "memory");
#else
    intptr_t a = 0, b, c, d;
    asm volatile(
        "cpuid"
        : "+a"(a), "=b"(b), "=c"(c), "=d"(d)
        :
        : "memory");
#endif
}

inline void loadLoadFence() { compilerFence(); }
inline void loadStoreFence() { compilerFence(); }
inline void storeLoadFence() { x86_ortop(); }
inline void storeStoreFence() { compilerFence(); }
inline void memoryBarrierAfterLock() { compilerFence(); }
inline void memoryBarrierBeforeUnlock() { compilerFence(); }
inline void crossModifyingCodeFence() { x86_cpuid(); }

#else

inline void loadLoadFence() { std::atomic_thread_fence(std::memory_order_seq_cst); }
inline void loadStoreFence() { std::atomic_thread_fence(std::memory_order_seq_cst); }
inline void storeLoadFence() { std::atomic_thread_fence(std::memory_order_seq_cst); }
inline void storeStoreFence() { std::atomic_thread_fence(std::memory_order_seq_cst); }
inline void memoryBarrierAfterLock() { std::atomic_thread_fence(std::memory_order_seq_cst); }
inline void memoryBarrierBeforeUnlock() { std::atomic_thread_fence(std::memory_order_seq_cst); }
inline void crossModifyingCodeFence() { std::atomic_thread_fence(std::memory_order_seq_cst); } // Probably not strong enough.

#endif

typedef unsigned Dependency;

ALWAYS_INLINE Dependency nullDependency()
{
    return 0;
}

template <typename T, typename std::enable_if<sizeof(T) == 8>::type* = nullptr>
ALWAYS_INLINE Dependency dependency(T value)
{
    unsigned dependency;
    uint64_t copy = bitwise_cast<uint64_t>(value);
#if CPU(ARM64)
    // Create a magical zero value through inline assembly, whose computation
    // isn't visible to the optimizer. This zero is then usable as an offset in
    // further address computations: adding zero does nothing, but the compiler
    // doesn't know it. It's magical because it creates an address dependency
    // from the load of `location` to the uses of the dependency, which triggers
    // the ARM ISA's address dependency rule, a.k.a. the mythical C++ consume
    // ordering. This forces weak memory order CPUs to observe `location` and
    // dependent loads in their store order without the reader using a barrier
    // or an acquire load.
    asm("eor %w[dependency], %w[in], %w[in]"
        : [dependency] "=r"(dependency)
        : [in] "r"(copy));
#elif CPU(ARM)
    asm("eor %[dependency], %[in], %[in]"
        : [dependency] "=r"(dependency)
        : [in] "r"(copy));
#else
    // No dependency is needed for this architecture.
    loadLoadFence();
    dependency = 0;
    UNUSED_PARAM(copy);
#endif
    return dependency;
}

// FIXME: This code is almost identical to the other dependency() overload.
// https://bugs.webkit.org/show_bug.cgi?id=169405
template <typename T, typename std::enable_if<sizeof(T) == 4>::type* = nullptr>
ALWAYS_INLINE Dependency dependency(T value)
{
    unsigned dependency;
    uint32_t copy = bitwise_cast<uint32_t>(value);
#if CPU(ARM64)
    asm("eor %w[dependency], %w[in], %w[in]"
        : [dependency] "=r"(dependency)
        : [in] "r"(copy));
#elif CPU(ARM)
    asm("eor %[dependency], %[in], %[in]"
        : [dependency] "=r"(dependency)
        : [in] "r"(copy));
#else
    loadLoadFence();
    dependency = 0;
    UNUSED_PARAM(copy);
#endif
    return dependency;
}

template <typename T, typename std::enable_if<sizeof(T) == 2>::type* = nullptr>
ALWAYS_INLINE Dependency dependency(T value)
{
    return dependency(static_cast<uint32_t>(value));
}

template <typename T, typename std::enable_if<sizeof(T) == 1>::type* = nullptr>
ALWAYS_INLINE Dependency dependency(T value)
{
    return dependency(static_cast<uint32_t>(value));
}

template<typename T>
struct DependencyWith {
public:
    DependencyWith()
        : dependency(nullDependency())
        , value()
    {
    }
    
    DependencyWith(Dependency dependency, const T& value)
        : dependency(dependency)
        , value(value)
    {
    }
    
    Dependency dependency;
    T value;
};
    
template<typename T>
inline DependencyWith<T> dependencyWith(Dependency dependency, const T& value)
{
    return DependencyWith<T>(dependency, value);
}

template<typename T>
inline T* consume(T* pointer, Dependency dependency)
{
#if CPU(ARM64) || CPU(ARM)
    return bitwise_cast<T*>(bitwise_cast<char*>(pointer) + dependency);
#else
    UNUSED_PARAM(dependency);
    return pointer;
#endif
}

} // namespace WTF

using WTF::Atomic;
using WTF::Dependency;
using WTF::DependencyWith;
using WTF::TransactionAbortLikelihood;
using WTF::consume;
using WTF::dependency;
using WTF::dependencyWith;
using WTF::nullDependency;

#endif // Atomics_h
