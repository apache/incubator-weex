/*
 *  Copyright (C) 2005, 2006, 2007, 2008, 2014 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef WTF_Vector_h
#define WTF_Vector_h

#include <initializer_list>
#include <limits>
#include <string.h>
#include <type_traits>
#include <utility>
#include <wtf/CheckedArithmetic.h>
#include <wtf/FastMalloc.h>
#include <wtf/MallocPtr.h>
#include <wtf/Noncopyable.h>
#include <wtf/StdLibExtras.h>
#include <wtf/ValueCheck.h>
#include <wtf/VectorTraits.h>

#if ASAN_ENABLED
extern "C" void __sanitizer_annotate_contiguous_container(const void* begin, const void* end, const void* old_mid, const void* new_mid);
#endif

namespace WTF {

const size_t notFound = static_cast<size_t>(-1);

template <bool needsDestruction, typename T>
struct VectorDestructor;

template<typename T>
struct VectorDestructor<false, T>
{
    static void destruct(T*, T*) {}
};

template<typename T>
struct VectorDestructor<true, T>
{
    static void destruct(T* begin, T* end) 
    {
        for (T* cur = begin; cur != end; ++cur)
            cur->~T();
    }
};

template <bool needsInitialization, bool canInitializeWithMemset, typename T>
struct VectorInitializer;

template<bool ignore, typename T>
struct VectorInitializer<false, ignore, T>
{
    static void initialize(T*, T*) {}
};

template<typename T>
struct VectorInitializer<true, false, T>
{
    static void initialize(T* begin, T* end) 
    {
        for (T* cur = begin; cur != end; ++cur)
            new (NotNull, cur) T;
    }
};

template<typename T>
struct VectorInitializer<true, true, T>
{
    static void initialize(T* begin, T* end) 
    {
        memset(begin, 0, reinterpret_cast<char*>(end) - reinterpret_cast<char*>(begin));
    }
};

template <bool canMoveWithMemcpy, typename T>
struct VectorMover;

template<typename T>
struct VectorMover<false, T>
{
    static void move(T* src, T* srcEnd, T* dst)
    {
        while (src != srcEnd) {
            new (NotNull, dst) T(WTFMove(*src));
            src->~T();
            ++dst;
            ++src;
        }
    }
    static void moveOverlapping(T* src, T* srcEnd, T* dst)
    {
        if (src > dst)
            move(src, srcEnd, dst);
        else {
            T* dstEnd = dst + (srcEnd - src);
            while (src != srcEnd) {
                --srcEnd;
                --dstEnd;
                new (NotNull, dstEnd) T(WTFMove(*srcEnd));
                srcEnd->~T();
            }
        }
    }
};

template<typename T>
struct VectorMover<true, T>
{
    static void move(const T* src, const T* srcEnd, T* dst) 
    {
        memcpy(dst, src, reinterpret_cast<const char*>(srcEnd) - reinterpret_cast<const char*>(src));
    }
    static void moveOverlapping(const T* src, const T* srcEnd, T* dst) 
    {
        memmove(dst, src, reinterpret_cast<const char*>(srcEnd) - reinterpret_cast<const char*>(src));
    }
};

template <bool canCopyWithMemcpy, typename T>
struct VectorCopier;

template<typename T>
struct VectorCopier<false, T>
{
    template<typename U>
    static void uninitializedCopy(const T* src, const T* srcEnd, U* dst)
    {
        while (src != srcEnd) {
            new (NotNull, dst) U(*src);
            ++dst;
            ++src;
        }
    }
};

template<typename T>
struct VectorCopier<true, T>
{
    static void uninitializedCopy(const T* src, const T* srcEnd, T* dst)
    {
        memcpy(dst, src, reinterpret_cast<const char*>(srcEnd) - reinterpret_cast<const char*>(src));
    }
    template<typename U>
    static void uninitializedCopy(const T* src, const T* srcEnd, U* dst)
    {
        VectorCopier<false, T>::uninitializedCopy(src, srcEnd, dst);
    }
};

template <bool canFillWithMemset, typename T>
struct VectorFiller;

template<typename T>
struct VectorFiller<false, T>
{
    static void uninitializedFill(T* dst, T* dstEnd, const T& val) 
    {
        while (dst != dstEnd) {
            new (NotNull, dst) T(val);
            ++dst;
        }
    }
};

template<typename T>
struct VectorFiller<true, T>
{
    static void uninitializedFill(T* dst, T* dstEnd, const T& val) 
    {
        static_assert(sizeof(T) == 1, "Size of type T should be equal to one!");
#if COMPILER(GCC_OR_CLANG) && defined(_FORTIFY_SOURCE)
        if (!__builtin_constant_p(dstEnd - dst) || (!(dstEnd - dst)))
#endif
            memset(dst, val, dstEnd - dst);
    }
};

template<bool canCompareWithMemcmp, typename T>
struct VectorComparer;

template<typename T>
struct VectorComparer<false, T>
{
    static bool compare(const T* a, const T* b, size_t size)
    {
        for (size_t i = 0; i < size; ++i)
            if (!(a[i] == b[i]))
                return false;
        return true;
    }
};

template<typename T>
struct VectorComparer<true, T>
{
    static bool compare(const T* a, const T* b, size_t size)
    {
        return memcmp(a, b, sizeof(T) * size) == 0;
    }
};

template<typename T>
struct VectorTypeOperations
{
    static void destruct(T* begin, T* end)
    {
        VectorDestructor<!std::is_trivially_destructible<T>::value, T>::destruct(begin, end);
    }

    static void initialize(T* begin, T* end)
    {
        VectorInitializer<VectorTraits<T>::needsInitialization, VectorTraits<T>::canInitializeWithMemset, T>::initialize(begin, end);
    }

    static void move(T* src, T* srcEnd, T* dst)
    {
        VectorMover<VectorTraits<T>::canMoveWithMemcpy, T>::move(src, srcEnd, dst);
    }

    static void moveOverlapping(T* src, T* srcEnd, T* dst)
    {
        VectorMover<VectorTraits<T>::canMoveWithMemcpy, T>::moveOverlapping(src, srcEnd, dst);
    }

    static void uninitializedCopy(const T* src, const T* srcEnd, T* dst)
    {
        VectorCopier<VectorTraits<T>::canCopyWithMemcpy, T>::uninitializedCopy(src, srcEnd, dst);
    }

    static void uninitializedFill(T* dst, T* dstEnd, const T& val)
    {
        VectorFiller<VectorTraits<T>::canFillWithMemset, T>::uninitializedFill(dst, dstEnd, val);
    }
    
    static bool compare(const T* a, const T* b, size_t size)
    {
        return VectorComparer<VectorTraits<T>::canCompareWithMemcmp, T>::compare(a, b, size);
    }
};

template<typename T>
class VectorBufferBase {
    WTF_MAKE_NONCOPYABLE(VectorBufferBase);
public:
    void allocateBuffer(size_t newCapacity)
    {
        ASSERT(newCapacity);
        if (newCapacity > std::numeric_limits<unsigned>::max() / sizeof(T))
            CRASH();
        size_t sizeToAllocate = newCapacity * sizeof(T);
        m_capacity = sizeToAllocate / sizeof(T);
        m_buffer = static_cast<T*>(fastMalloc(sizeToAllocate));
    }

    bool tryAllocateBuffer(size_t newCapacity)
    {
        ASSERT(newCapacity);
        if (newCapacity > std::numeric_limits<unsigned>::max() / sizeof(T))
            return false;

        size_t sizeToAllocate = newCapacity * sizeof(T);
        T* newBuffer;
        if (tryFastMalloc(sizeToAllocate).getValue(newBuffer)) {
            m_capacity = sizeToAllocate / sizeof(T);
            m_buffer = newBuffer;
            return true;
        }
        return false;
    }

    bool shouldReallocateBuffer(size_t newCapacity) const
    {
        return VectorTraits<T>::canMoveWithMemcpy && m_capacity && newCapacity;
    }

    void reallocateBuffer(size_t newCapacity)
    {
        ASSERT(shouldReallocateBuffer(newCapacity));
        if (newCapacity > std::numeric_limits<size_t>::max() / sizeof(T))
            CRASH();
        size_t sizeToAllocate = newCapacity * sizeof(T);
        m_capacity = sizeToAllocate / sizeof(T);
        m_buffer = static_cast<T*>(fastRealloc(m_buffer, sizeToAllocate));
    }

    void deallocateBuffer(T* bufferToDeallocate)
    {
        if (!bufferToDeallocate)
            return;
        
        if (m_buffer == bufferToDeallocate) {
            m_buffer = 0;
            m_capacity = 0;
        }

        fastFree(bufferToDeallocate);
    }

    T* buffer() { return m_buffer; }
    const T* buffer() const { return m_buffer; }
    static ptrdiff_t bufferMemoryOffset() { return OBJECT_OFFSETOF(VectorBufferBase, m_buffer); }
    size_t capacity() const { return m_capacity; }

    MallocPtr<T> releaseBuffer()
    {
        T* buffer = m_buffer;
        m_buffer = 0;
        m_capacity = 0;
        return adoptMallocPtr(buffer);
    }

protected:
    VectorBufferBase()
        : m_buffer(0)
        , m_capacity(0)
        , m_size(0)
    {
    }

    VectorBufferBase(T* buffer, size_t capacity, size_t size)
        : m_buffer(buffer)
        , m_capacity(capacity)
        , m_size(size)
    {
    }

    ~VectorBufferBase()
    {
        // FIXME: It would be nice to find a way to ASSERT that m_buffer hasn't leaked here.
    }

    T* m_buffer;
    unsigned m_capacity;
    unsigned m_size; // Only used by the Vector subclass, but placed here to avoid padding the struct.
};

template<typename T, size_t inlineCapacity>
class VectorBuffer;

template<typename T>
class VectorBuffer<T, 0> : private VectorBufferBase<T> {
private:
    typedef VectorBufferBase<T> Base;
public:
    VectorBuffer()
    {
    }

    VectorBuffer(size_t capacity, size_t size = 0)
    {
        m_size = size;
        // Calling malloc(0) might take a lock and may actually do an
        // allocation on some systems.
        if (capacity)
            allocateBuffer(capacity);
    }

    ~VectorBuffer()
    {
        deallocateBuffer(buffer());
    }
    
    void swap(VectorBuffer<T, 0>& other, size_t, size_t)
    {
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_capacity, other.m_capacity);
    }
    
    void restoreInlineBufferIfNeeded() { }

#if ASAN_ENABLED
    void* endOfBuffer()
    {
        return buffer() + capacity();
    }
#endif

    using Base::allocateBuffer;
    using Base::tryAllocateBuffer;
    using Base::shouldReallocateBuffer;
    using Base::reallocateBuffer;
    using Base::deallocateBuffer;

    using Base::buffer;
    using Base::capacity;
    using Base::bufferMemoryOffset;

    using Base::releaseBuffer;

protected:
    using Base::m_size;

private:
    using Base::m_buffer;
    using Base::m_capacity;
};

template<typename T, size_t inlineCapacity>
class VectorBuffer : private VectorBufferBase<T> {
    WTF_MAKE_NONCOPYABLE(VectorBuffer);
private:
    typedef VectorBufferBase<T> Base;
public:
    VectorBuffer()
        : Base(inlineBuffer(), inlineCapacity, 0)
    {
    }

    VectorBuffer(size_t capacity, size_t size = 0)
        : Base(inlineBuffer(), inlineCapacity, size)
    {
        if (capacity > inlineCapacity)
            Base::allocateBuffer(capacity);
    }

    ~VectorBuffer()
    {
        deallocateBuffer(buffer());
    }

    void allocateBuffer(size_t newCapacity)
    {
        // FIXME: This should ASSERT(!m_buffer) to catch misuse/leaks.
        if (newCapacity > inlineCapacity)
            Base::allocateBuffer(newCapacity);
        else {
            m_buffer = inlineBuffer();
            m_capacity = inlineCapacity;
        }
    }

    bool tryAllocateBuffer(size_t newCapacity)
    {
        if (newCapacity > inlineCapacity)
            return Base::tryAllocateBuffer(newCapacity);
        m_buffer = inlineBuffer();
        m_capacity = inlineCapacity;
        return true;
    }

    void deallocateBuffer(T* bufferToDeallocate)
    {
        if (bufferToDeallocate == inlineBuffer())
            return;
        Base::deallocateBuffer(bufferToDeallocate);
    }

    bool shouldReallocateBuffer(size_t newCapacity) const
    {
        // We cannot reallocate the inline buffer.
        return Base::shouldReallocateBuffer(newCapacity) && std::min(static_cast<size_t>(m_capacity), newCapacity) > inlineCapacity;
    }

    void reallocateBuffer(size_t newCapacity)
    {
        ASSERT(shouldReallocateBuffer(newCapacity));
        Base::reallocateBuffer(newCapacity);
    }

    void swap(VectorBuffer& other, size_t mySize, size_t otherSize)
    {
        if (buffer() == inlineBuffer() && other.buffer() == other.inlineBuffer()) {
            swapInlineBuffer(other, mySize, otherSize);
            std::swap(m_capacity, other.m_capacity);
        } else if (buffer() == inlineBuffer()) {
            m_buffer = other.m_buffer;
            other.m_buffer = other.inlineBuffer();
            swapInlineBuffer(other, mySize, 0);
            std::swap(m_capacity, other.m_capacity);
        } else if (other.buffer() == other.inlineBuffer()) {
            other.m_buffer = m_buffer;
            m_buffer = inlineBuffer();
            swapInlineBuffer(other, 0, otherSize);
            std::swap(m_capacity, other.m_capacity);
        } else {
            std::swap(m_buffer, other.m_buffer);
            std::swap(m_capacity, other.m_capacity);
        }
    }

    void restoreInlineBufferIfNeeded()
    {
        if (m_buffer)
            return;
        m_buffer = inlineBuffer();
        m_capacity = inlineCapacity;
    }

#if ASAN_ENABLED
    void* endOfBuffer()
    {
        ASSERT(buffer());
        static_assert((offsetof(VectorBuffer, m_inlineBuffer) + sizeof(m_inlineBuffer)) % 8 == 0, "Inline buffer end needs to be on 8 byte boundary for ASan annotations to work.");

        if (buffer() == inlineBuffer())
            return reinterpret_cast<char*>(m_inlineBuffer) + sizeof(m_inlineBuffer);

        return buffer() + capacity();
    }
#endif

    using Base::buffer;
    using Base::capacity;
    using Base::bufferMemoryOffset;

    MallocPtr<T> releaseBuffer()
    {
        if (buffer() == inlineBuffer())
            return nullptr;
        return Base::releaseBuffer();
    }

protected:
    using Base::m_size;

private:
    using Base::m_buffer;
    using Base::m_capacity;
    
    void swapInlineBuffer(VectorBuffer& other, size_t mySize, size_t otherSize)
    {
        // FIXME: We could make swap part of VectorTypeOperations
        // https://bugs.webkit.org/show_bug.cgi?id=128863
        swapInlineBuffers(inlineBuffer(), other.inlineBuffer(), mySize, otherSize);
    }
    
    static void swapInlineBuffers(T* left, T* right, size_t leftSize, size_t rightSize)
    {
        if (left == right)
            return;
        
        ASSERT(leftSize <= inlineCapacity);
        ASSERT(rightSize <= inlineCapacity);
        
        size_t swapBound = std::min(leftSize, rightSize);
        for (unsigned i = 0; i < swapBound; ++i)
            std::swap(left[i], right[i]);
        VectorTypeOperations<T>::move(left + swapBound, left + leftSize, right + swapBound);
        VectorTypeOperations<T>::move(right + swapBound, right + rightSize, left + swapBound);
    }

    T* inlineBuffer() { return reinterpret_cast_ptr<T*>(m_inlineBuffer); }
    const T* inlineBuffer() const { return reinterpret_cast_ptr<const T*>(m_inlineBuffer); }

#if ASAN_ENABLED
    // ASan needs the buffer to begin and end on 8-byte boundaries for annotations to work.
    // FIXME: Add a redzone before the buffer to catch off by one accesses. We don't need a guard after, because the buffer is the last member variable.
    static const size_t asanInlineBufferAlignment = std::alignment_of<T>::value >= 8 ? std::alignment_of<T>::value : 8;
    static const size_t asanAdjustedInlineCapacity = ((sizeof(T) * inlineCapacity + 7) & ~7) / sizeof(T);
    typename std::aligned_storage<sizeof(T), asanInlineBufferAlignment>::type m_inlineBuffer[asanAdjustedInlineCapacity];
#else
    typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type m_inlineBuffer[inlineCapacity];
#endif
};

struct UnsafeVectorOverflow {
    static NO_RETURN_DUE_TO_ASSERT void overflowed()
    {
        ASSERT_NOT_REACHED();
    }
};

template<typename T, size_t inlineCapacity = 0, typename OverflowHandler = CrashOnOverflow, size_t minCapacity = 16>
class Vector : private VectorBuffer<T, inlineCapacity> {
    WTF_MAKE_FAST_ALLOCATED;
private:
    typedef VectorBuffer<T, inlineCapacity> Base;
    typedef VectorTypeOperations<T> TypeOperations;

public:
    typedef T ValueType;

    typedef T* iterator;
    typedef const T* const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    Vector()
    {
    }

    // Unlike in std::vector, this constructor does not initialize POD types.
    explicit Vector(size_t size)
        : Base(size, size)
    {
        asanSetInitialBufferSizeTo(size);

        if (begin())
            TypeOperations::initialize(begin(), end());
    }

    Vector(size_t size, const T& val)
        : Base(size, size)
    {
        asanSetInitialBufferSizeTo(size);

        if (begin())
            TypeOperations::uninitializedFill(begin(), end(), val);
    }

    Vector(std::initializer_list<T> initializerList)
    {
        reserveInitialCapacity(initializerList.size());

        asanSetInitialBufferSizeTo(initializerList.size());

        for (const auto& element : initializerList)
            uncheckedAppend(element);
    }

    ~Vector()
    {
        if (m_size)
            TypeOperations::destruct(begin(), end());

        asanSetBufferSizeToFullCapacity(0);
    }

    Vector(const Vector&);
    template<size_t otherCapacity, typename otherOverflowBehaviour, size_t otherMinimumCapacity>
    explicit Vector(const Vector<T, otherCapacity, otherOverflowBehaviour, otherMinimumCapacity>&);

    Vector& operator=(const Vector&);
    template<size_t otherCapacity, typename otherOverflowBehaviour, size_t otherMinimumCapacity>
    Vector& operator=(const Vector<T, otherCapacity, otherOverflowBehaviour, otherMinimumCapacity>&);

    Vector(Vector&&);
    Vector& operator=(Vector&&);

    size_t size() const { return m_size; }
    static ptrdiff_t sizeMemoryOffset() { return OBJECT_OFFSETOF(Vector, m_size); }
    size_t capacity() const { return Base::capacity(); }
    bool isEmpty() const { return !size(); }

    T& at(size_t i)
    {
        if (UNLIKELY(i >= size()))
            OverflowHandler::overflowed();
        return Base::buffer()[i];
    }
    const T& at(size_t i) const 
    {
        if (UNLIKELY(i >= size()))
            OverflowHandler::overflowed();
        return Base::buffer()[i];
    }
    T& at(Checked<size_t> i)
    {
        RELEASE_ASSERT(i < size());
        return Base::buffer()[i];
    }
    const T& at(Checked<size_t> i) const
    {
        RELEASE_ASSERT(i < size());
        return Base::buffer()[i];
    }

    T& operator[](size_t i) { return at(i); }
    const T& operator[](size_t i) const { return at(i); }
    T& operator[](Checked<size_t> i) { return at(i); }
    const T& operator[](Checked<size_t> i) const { return at(i); }

    T* data() { return Base::buffer(); }
    const T* data() const { return Base::buffer(); }
    static ptrdiff_t dataMemoryOffset() { return Base::bufferMemoryOffset(); }

    iterator begin() { return data(); }
    iterator end() { return begin() + m_size; }
    const_iterator begin() const { return data(); }
    const_iterator end() const { return begin() + m_size; }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    T& first() { return at(0); }
    const T& first() const { return at(0); }
    T& last() { return at(size() - 1); }
    const T& last() const { return at(size() - 1); }
    
    T takeLast()
    {
        T result = WTFMove(last());
        removeLast();
        return result;
    }
    
    template<typename U> bool contains(const U&) const;
    template<typename U> size_t find(const U&) const;
    template<typename U> size_t reverseFind(const U&) const;

    void shrink(size_t size);
    void grow(size_t size);
    void resize(size_t size);
    void resizeToFit(size_t size);
    void reserveCapacity(size_t newCapacity);
    bool tryReserveCapacity(size_t newCapacity);
    void reserveInitialCapacity(size_t initialCapacity);
    void shrinkCapacity(size_t newCapacity);
    void shrinkToFit() { shrinkCapacity(size()); }

    void clear() { shrinkCapacity(0); }

    void append(ValueType&& value) { append<ValueType>(std::forward<ValueType>(value)); }
    template<typename U> void append(U&&);
    template<typename... Args> void constructAndAppend(Args&&...);
    template<typename... Args> bool tryConstructAndAppend(Args&&...);

    void uncheckedAppend(ValueType&& value) { uncheckedAppend<ValueType>(std::forward<ValueType>(value)); }
    template<typename U> void uncheckedAppend(U&&);

    template<typename U> void append(const U*, size_t);
    template<typename U, size_t otherCapacity> void appendVector(const Vector<U, otherCapacity>&);
    template<typename U> bool tryAppend(const U*, size_t);

    template<typename U> void insert(size_t position, const U*, size_t);
    template<typename U> void insert(size_t position, U&&);
    template<typename U, size_t c> void insertVector(size_t position, const Vector<U, c>&);

    void remove(size_t position);
    void remove(size_t position, size_t length);
    template<typename U> bool removeFirst(const U&);
    template<typename MatchFunction> bool removeFirstMatching(const MatchFunction&);
    template<typename U> unsigned removeAll(const U&);
    template<typename MatchFunction> unsigned removeAllMatching(const MatchFunction&);

    void removeLast() 
    {
        if (UNLIKELY(isEmpty()))
            OverflowHandler::overflowed();
        shrink(size() - 1); 
    }

    void fill(const T&, size_t);
    void fill(const T& val) { fill(val, size()); }

    template<typename Iterator> void appendRange(Iterator start, Iterator end);

    MallocPtr<T> releaseBuffer();

    void swap(Vector<T, inlineCapacity, OverflowHandler, minCapacity>& other)
    {
#if ASAN_ENABLED
        if (this == std::addressof(other)) // ASan will crash if we try to restrict access to the same buffer twice.
            return;
#endif

        // Make it possible to copy inline buffers.
        asanSetBufferSizeToFullCapacity();
        other.asanSetBufferSizeToFullCapacity();

        Base::swap(other, m_size, other.m_size);
        std::swap(m_size, other.m_size);

        asanSetInitialBufferSizeTo(m_size);
        other.asanSetInitialBufferSizeTo(other.m_size);
    }

    void reverse();

    void checkConsistency();

private:
    void expandCapacity(size_t newMinCapacity);
    T* expandCapacity(size_t newMinCapacity, T*);
    bool tryExpandCapacity(size_t newMinCapacity);
    const T* tryExpandCapacity(size_t newMinCapacity, const T*);
    template<typename U> U* expandCapacity(size_t newMinCapacity, U*); 
    template<typename U> void appendSlowCase(U&&);
    template<typename... Args> void constructAndAppendSlowCase(Args&&...);
    template<typename... Args> bool tryConstructAndAppendSlowCase(Args&&...);

    void asanSetInitialBufferSizeTo(size_t);
    void asanSetBufferSizeToFullCapacity(size_t);
    void asanSetBufferSizeToFullCapacity() { asanSetBufferSizeToFullCapacity(size()); }

    void asanBufferSizeWillChangeTo(size_t);

    using Base::m_size;
    using Base::buffer;
    using Base::capacity;
    using Base::swap;
    using Base::allocateBuffer;
    using Base::deallocateBuffer;
    using Base::tryAllocateBuffer;
    using Base::shouldReallocateBuffer;
    using Base::reallocateBuffer;
    using Base::restoreInlineBufferIfNeeded;
    using Base::releaseBuffer;
#if ASAN_ENABLED
    using Base::endOfBuffer;
#endif
};

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
Vector<T, inlineCapacity, OverflowHandler, minCapacity>::Vector(const Vector& other)
    : Base(other.capacity(), other.size())
{
    asanSetInitialBufferSizeTo(other.size());

    if (begin())
        TypeOperations::uninitializedCopy(other.begin(), other.end(), begin());
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
template<size_t otherCapacity, typename otherOverflowBehaviour, size_t otherMinimumCapacity>
Vector<T, inlineCapacity, OverflowHandler, minCapacity>::Vector(const Vector<T, otherCapacity, otherOverflowBehaviour, otherMinimumCapacity>& other)
    : Base(other.capacity(), other.size())
{
    asanSetInitialBufferSizeTo(other.size());

    if (begin())
        TypeOperations::uninitializedCopy(other.begin(), other.end(), begin());
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
Vector<T, inlineCapacity, OverflowHandler, minCapacity>& Vector<T, inlineCapacity, OverflowHandler, minCapacity>::operator=(const Vector<T, inlineCapacity, OverflowHandler, minCapacity>& other)
{
    if (&other == this)
        return *this;
    
    if (size() > other.size())
        shrink(other.size());
    else if (other.size() > capacity()) {
        clear();
        reserveCapacity(other.size());
        ASSERT(begin());
    }

    asanBufferSizeWillChangeTo(other.size());

    std::copy(other.begin(), other.begin() + size(), begin());
    TypeOperations::uninitializedCopy(other.begin() + size(), other.end(), end());
    m_size = other.size();

    return *this;
}

inline bool typelessPointersAreEqual(const void* a, const void* b) { return a == b; }

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
template<size_t otherCapacity, typename otherOverflowBehaviour, size_t otherMinimumCapacity>
Vector<T, inlineCapacity, OverflowHandler, minCapacity>& Vector<T, inlineCapacity, OverflowHandler, minCapacity>::operator=(const Vector<T, otherCapacity, otherOverflowBehaviour, otherMinimumCapacity>& other)
{
    // If the inline capacities match, we should call the more specific
    // template.  If the inline capacities don't match, the two objects
    // shouldn't be allocated the same address.
    ASSERT(!typelessPointersAreEqual(&other, this));

    if (size() > other.size())
        shrink(other.size());
    else if (other.size() > capacity()) {
        clear();
        reserveCapacity(other.size());
        ASSERT(begin());
    }
    
    asanBufferSizeWillChangeTo(other.size());

    std::copy(other.begin(), other.begin() + size(), begin());
    TypeOperations::uninitializedCopy(other.begin() + size(), other.end(), end());
    m_size = other.size();

    return *this;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline Vector<T, inlineCapacity, OverflowHandler, minCapacity>::Vector(Vector<T, inlineCapacity, OverflowHandler, minCapacity>&& other)
{
    swap(other);
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline Vector<T, inlineCapacity, OverflowHandler, minCapacity>& Vector<T, inlineCapacity, OverflowHandler, minCapacity>::operator=(Vector<T, inlineCapacity, OverflowHandler, minCapacity>&& other)
{
    swap(other);
    return *this;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
template<typename U>
bool Vector<T, inlineCapacity, OverflowHandler, minCapacity>::contains(const U& value) const
{
    return find(value) != notFound;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
template<typename U>
size_t Vector<T, inlineCapacity, OverflowHandler, minCapacity>::find(const U& value) const
{
    for (size_t i = 0; i < size(); ++i) {
        if (at(i) == value)
            return i;
    }
    return notFound;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
template<typename U>
size_t Vector<T, inlineCapacity, OverflowHandler, minCapacity>::reverseFind(const U& value) const
{
    for (size_t i = 1; i <= size(); ++i) {
        const size_t index = size() - i;
        if (at(index) == value)
            return index;
    }
    return notFound;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::fill(const T& val, size_t newSize)
{
    if (size() > newSize)
        shrink(newSize);
    else if (newSize > capacity()) {
        clear();
        reserveCapacity(newSize);
        ASSERT(begin());
    }

    asanBufferSizeWillChangeTo(newSize);

    std::fill(begin(), end(), val);
    TypeOperations::uninitializedFill(end(), begin() + newSize, val);
    m_size = newSize;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
template<typename Iterator>
void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::appendRange(Iterator start, Iterator end)
{
    for (Iterator it = start; it != end; ++it)
        append(*it);
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::expandCapacity(size_t newMinCapacity)
{
    reserveCapacity(std::max(newMinCapacity, std::max(static_cast<size_t>(minCapacity), capacity() + capacity() / 4 + 1)));
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
T* Vector<T, inlineCapacity, OverflowHandler, minCapacity>::expandCapacity(size_t newMinCapacity, T* ptr)
{
    if (ptr < begin() || ptr >= end()) {
        expandCapacity(newMinCapacity);
        return ptr;
    }
    size_t index = ptr - begin();
    expandCapacity(newMinCapacity);
    return begin() + index;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
bool Vector<T, inlineCapacity, OverflowHandler, minCapacity>::tryExpandCapacity(size_t newMinCapacity)
{
    return tryReserveCapacity(std::max(newMinCapacity, std::max(static_cast<size_t>(minCapacity), capacity() + capacity() / 4 + 1)));
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
const T* Vector<T, inlineCapacity, OverflowHandler, minCapacity>::tryExpandCapacity(size_t newMinCapacity, const T* ptr)
{
    if (ptr < begin() || ptr >= end()) {
        if (!tryExpandCapacity(newMinCapacity))
            return 0;
        return ptr;
    }
    size_t index = ptr - begin();
    if (!tryExpandCapacity(newMinCapacity))
        return 0;
    return begin() + index;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename U>
inline U* Vector<T, inlineCapacity, OverflowHandler, minCapacity>::expandCapacity(size_t newMinCapacity, U* ptr)
{
    expandCapacity(newMinCapacity);
    return ptr;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::resize(size_t size)
{
    if (size <= m_size) {
        TypeOperations::destruct(begin() + size, end());
        asanBufferSizeWillChangeTo(size);
    } else {
        if (size > capacity())
            expandCapacity(size);
        asanBufferSizeWillChangeTo(size);
        if (begin())
            TypeOperations::initialize(end(), begin() + size);
    }
    
    m_size = size;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::resizeToFit(size_t size)
{
    reserveCapacity(size);
    resize(size);
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::shrink(size_t size)
{
    ASSERT(size <= m_size);
    TypeOperations::destruct(begin() + size, end());
    asanBufferSizeWillChangeTo(size);
    m_size = size;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::grow(size_t size)
{
    ASSERT(size >= m_size);
    if (size > capacity())
        expandCapacity(size);
    asanBufferSizeWillChangeTo(size);
    if (begin())
        TypeOperations::initialize(end(), begin() + size);
    m_size = size;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::asanSetInitialBufferSizeTo(size_t size)
{
#if ASAN_ENABLED
    if (!buffer())
        return;

    // This function resticts buffer access to only elements in [begin(), end()) range, making ASan detect an error
    // when accessing elements in [end(), endOfBuffer()) range.
    // A newly allocated buffer can be accessed without restrictions, so "old_mid" argument equals "end" argument.
    __sanitizer_annotate_contiguous_container(buffer(), endOfBuffer(), endOfBuffer(), buffer() + size);
#else
    UNUSED_PARAM(size);
#endif
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::asanSetBufferSizeToFullCapacity(size_t size)
{
#if ASAN_ENABLED
    if (!buffer())
        return;

    // ASan requires that the annotation is returned to its initial state before deallocation.
    __sanitizer_annotate_contiguous_container(buffer(), endOfBuffer(), buffer() + size, endOfBuffer());
#else
    UNUSED_PARAM(size);
#endif
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::asanBufferSizeWillChangeTo(size_t newSize)
{
#if ASAN_ENABLED
    if (!buffer())
        return;

    // Change allowed range.
    __sanitizer_annotate_contiguous_container(buffer(), endOfBuffer(), buffer() + size(), buffer() + newSize);
#else
    UNUSED_PARAM(newSize);
#endif
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::reserveCapacity(size_t newCapacity)
{
    if (newCapacity <= capacity())
        return;
    T* oldBuffer = begin();
    T* oldEnd = end();

    asanSetBufferSizeToFullCapacity();

    Base::allocateBuffer(newCapacity);
    ASSERT(begin());

    asanSetInitialBufferSizeTo(size());

    TypeOperations::move(oldBuffer, oldEnd, begin());
    Base::deallocateBuffer(oldBuffer);
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
bool Vector<T, inlineCapacity, OverflowHandler, minCapacity>::tryReserveCapacity(size_t newCapacity)
{
    if (newCapacity <= capacity())
        return true;
    T* oldBuffer = begin();
    T* oldEnd = end();

    asanSetBufferSizeToFullCapacity();

    if (!Base::tryAllocateBuffer(newCapacity)) {
        asanSetInitialBufferSizeTo(size());
        return false;
    }
    ASSERT(begin());

    asanSetInitialBufferSizeTo(size());

    TypeOperations::move(oldBuffer, oldEnd, begin());
    Base::deallocateBuffer(oldBuffer);
    return true;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::reserveInitialCapacity(size_t initialCapacity)
{
    ASSERT(!m_size);
    ASSERT(capacity() == inlineCapacity);
    if (initialCapacity > inlineCapacity)
        Base::allocateBuffer(initialCapacity);
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::shrinkCapacity(size_t newCapacity)
{
    if (newCapacity >= capacity())
        return;

    if (newCapacity < size()) 
        shrink(newCapacity);

    asanSetBufferSizeToFullCapacity();

    T* oldBuffer = begin();
    if (newCapacity > 0) {
        if (Base::shouldReallocateBuffer(newCapacity)) {
            Base::reallocateBuffer(newCapacity);
            asanSetInitialBufferSizeTo(size());
            return;
        }

        T* oldEnd = end();
        Base::allocateBuffer(newCapacity);
        if (begin() != oldBuffer)
            TypeOperations::move(oldBuffer, oldEnd, begin());
    }

    Base::deallocateBuffer(oldBuffer);
    Base::restoreInlineBufferIfNeeded();

    asanSetInitialBufferSizeTo(size());
}

// Templatizing these is better than just letting the conversion happen implicitly,
// because for instance it allows a PassRefPtr to be appended to a RefPtr vector
// without refcount thrash.
template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename U>
void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::append(const U* data, size_t dataSize)
{
    size_t newSize = m_size + dataSize;
    if (newSize > capacity()) {
        data = expandCapacity(newSize, data);
        ASSERT(begin());
    }
    if (newSize < m_size)
        CRASH();
    asanBufferSizeWillChangeTo(newSize);
    T* dest = end();
    VectorCopier<std::is_trivial<T>::value, U>::uninitializedCopy(data, std::addressof(data[dataSize]), dest);
    m_size = newSize;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename U>
bool Vector<T, inlineCapacity, OverflowHandler, minCapacity>::tryAppend(const U* data, size_t dataSize)
{
    size_t newSize = m_size + dataSize;
    if (newSize > capacity()) {
        data = tryExpandCapacity(newSize, data);
        if (!data)
            return false;
        ASSERT(begin());
    }
    if (newSize < m_size)
        return false;
    asanBufferSizeWillChangeTo(newSize);
    T* dest = end();
    VectorCopier<std::is_trivial<T>::value, U>::uninitializedCopy(data, std::addressof(data[dataSize]), dest);
    m_size = newSize;
    return true;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename U>
ALWAYS_INLINE void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::append(U&& value)
{
    if (size() != capacity()) {
        asanBufferSizeWillChangeTo(m_size + 1);
        new (NotNull, end()) T(std::forward<U>(value));
        ++m_size;
        return;
    }

    appendSlowCase(std::forward<U>(value));
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename... Args>
ALWAYS_INLINE void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::constructAndAppend(Args&&... args)
{
    if (size() != capacity()) {
        asanBufferSizeWillChangeTo(m_size + 1);
        new (NotNull, end()) T(std::forward<Args>(args)...);
        ++m_size;
        return;
    }

    constructAndAppendSlowCase(std::forward<Args>(args)...);
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename... Args>
ALWAYS_INLINE bool Vector<T, inlineCapacity, OverflowHandler, minCapacity>::tryConstructAndAppend(Args&&... args)
{
    if (size() != capacity()) {
        asanBufferSizeWillChangeTo(m_size + 1);
        new (NotNull, end()) T(std::forward<Args>(args)...);
        ++m_size;
        return true;
    }
    
    return tryConstructAndAppendSlowCase(std::forward<Args>(args)...);
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename U>
void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::appendSlowCase(U&& value)
{
    ASSERT(size() == capacity());

    auto ptr = const_cast<typename std::remove_const<typename std::remove_reference<U>::type>::type*>(std::addressof(value));
    ptr = expandCapacity(size() + 1, ptr);
    ASSERT(begin());

    asanBufferSizeWillChangeTo(m_size + 1);
    new (NotNull, end()) T(std::forward<U>(*ptr));
    ++m_size;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename... Args>
void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::constructAndAppendSlowCase(Args&&... args)
{
    ASSERT(size() == capacity());

    expandCapacity(size() + 1);
    ASSERT(begin());

    asanBufferSizeWillChangeTo(m_size + 1);
    new (NotNull, end()) T(std::forward<Args>(args)...);
    ++m_size;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename... Args>
bool Vector<T, inlineCapacity, OverflowHandler, minCapacity>::tryConstructAndAppendSlowCase(Args&&... args)
{
    ASSERT(size() == capacity());
    
    if (UNLIKELY(!tryExpandCapacity(size() + 1)))
        return false;
    ASSERT(begin());
    
    asanBufferSizeWillChangeTo(m_size + 1);
    new (NotNull, end()) T(std::forward<Args>(args)...);
    ++m_size;
    return true;
}

// This version of append saves a branch in the case where you know that the
// vector's capacity is large enough for the append to succeed.

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename U>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::uncheckedAppend(U&& value)
{
    ASSERT(size() < capacity());

    asanBufferSizeWillChangeTo(m_size + 1);

    auto ptr = std::addressof(value);
    new (NotNull, end()) T(std::forward<U>(*ptr));
    ++m_size;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename U, size_t otherCapacity>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::appendVector(const Vector<U, otherCapacity>& val)
{
    append(val.begin(), val.size());
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename U>
void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::insert(size_t position, const U* data, size_t dataSize)
{
    ASSERT_WITH_SECURITY_IMPLICATION(position <= size());
    size_t newSize = m_size + dataSize;
    if (newSize > capacity()) {
        data = expandCapacity(newSize, data);
        ASSERT(begin());
    }
    if (newSize < m_size)
        CRASH();
    asanBufferSizeWillChangeTo(newSize);
    T* spot = begin() + position;
    TypeOperations::moveOverlapping(spot, end(), spot + dataSize);
    VectorCopier<std::is_trivial<T>::value, U>::uninitializedCopy(data, std::addressof(data[dataSize]), spot);
    m_size = newSize;
}
 
template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename U>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::insert(size_t position, U&& value)
{
    ASSERT_WITH_SECURITY_IMPLICATION(position <= size());

    auto ptr = const_cast<typename std::remove_const<typename std::remove_reference<U>::type>::type*>(std::addressof(value));
    if (size() == capacity()) {
        ptr = expandCapacity(size() + 1, ptr);
        ASSERT(begin());
    }

    asanBufferSizeWillChangeTo(m_size + 1);

    T* spot = begin() + position;
    TypeOperations::moveOverlapping(spot, end(), spot + 1);
    new (NotNull, spot) T(std::forward<U>(*ptr));
    ++m_size;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> template<typename U, size_t c>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::insertVector(size_t position, const Vector<U, c>& val)
{
    insert(position, val.begin(), val.size());
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::remove(size_t position)
{
    ASSERT_WITH_SECURITY_IMPLICATION(position < size());
    T* spot = begin() + position;
    spot->~T();
    TypeOperations::moveOverlapping(spot + 1, end(), spot);
    asanBufferSizeWillChangeTo(m_size - 1);
    --m_size;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::remove(size_t position, size_t length)
{
    ASSERT_WITH_SECURITY_IMPLICATION(position <= size());
    ASSERT_WITH_SECURITY_IMPLICATION(position + length <= size());
    T* beginSpot = begin() + position;
    T* endSpot = beginSpot + length;
    TypeOperations::destruct(beginSpot, endSpot); 
    TypeOperations::moveOverlapping(endSpot, end(), beginSpot);
    asanBufferSizeWillChangeTo(m_size - length);
    m_size -= length;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
template<typename U>
inline bool Vector<T, inlineCapacity, OverflowHandler, minCapacity>::removeFirst(const U& value)
{
    return removeFirstMatching([&value] (const T& current) {
        return current == value;
    });
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
template<typename MatchFunction>
inline bool Vector<T, inlineCapacity, OverflowHandler, minCapacity>::removeFirstMatching(const MatchFunction& matches)
{
    for (size_t i = 0; i < size(); ++i) {
        if (matches(at(i))) {
            remove(i);
            return true;
        }
    }
    return false;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
template<typename U>
inline unsigned Vector<T, inlineCapacity, OverflowHandler, minCapacity>::removeAll(const U& value)
{
    return removeAllMatching([&value] (const T& current) {
        return current == value;
    });
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
template<typename MatchFunction>
inline unsigned Vector<T, inlineCapacity, OverflowHandler, minCapacity>::removeAllMatching(const MatchFunction& matches)
{
    iterator holeBegin = end();
    iterator holeEnd = end();
    unsigned matchCount = 0;
    for (auto it = begin(), itEnd = end(); it != itEnd; ++it) {
        if (matches(*it)) {
            if (holeBegin == end())
                holeBegin = it;
            else if (holeEnd != it) {
                TypeOperations::moveOverlapping(holeEnd, it, holeBegin);
                holeBegin += it - holeEnd;
            }
            holeEnd = it + 1;
            it->~T();
            ++matchCount;
        }
    }
    if (holeEnd != end())
        TypeOperations::moveOverlapping(holeEnd, end(), holeBegin);
    asanBufferSizeWillChangeTo(m_size - matchCount);
    m_size -= matchCount;
    return matchCount;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::reverse()
{
    for (size_t i = 0; i < m_size / 2; ++i)
        std::swap(at(i), at(m_size - 1 - i));
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline MallocPtr<T> Vector<T, inlineCapacity, OverflowHandler, minCapacity>::releaseBuffer()
{
    // FIXME: Find a way to preserve annotations on the returned buffer.
    // ASan requires that all annotations are removed before deallocation,
    // and MallocPtr doesn't implement that.
    asanSetBufferSizeToFullCapacity();

    auto buffer = Base::releaseBuffer();
    if (inlineCapacity && !buffer && m_size) {
        // If the vector had some data, but no buffer to release,
        // that means it was using the inline buffer. In that case,
        // we create a brand new buffer so the caller always gets one.
        size_t bytes = m_size * sizeof(T);
        buffer = adoptMallocPtr(static_cast<T*>(fastMalloc(bytes)));
        memcpy(buffer.get(), data(), bytes);
    }
    m_size = 0;
    // FIXME: Should we call Base::restoreInlineBufferIfNeeded() here?
    return buffer;
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline void Vector<T, inlineCapacity, OverflowHandler, minCapacity>::checkConsistency()
{
#if !ASSERT_DISABLED
    for (size_t i = 0; i < size(); ++i)
        ValueCheck<T>::checkConsistency(at(i));
#endif
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline void swap(Vector<T, inlineCapacity, OverflowHandler, minCapacity>& a, Vector<T, inlineCapacity, OverflowHandler, minCapacity>& b)
{
    a.swap(b);
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
bool operator==(const Vector<T, inlineCapacity, OverflowHandler, minCapacity>& a, const Vector<T, inlineCapacity, OverflowHandler, minCapacity>& b)
{
    if (a.size() != b.size())
        return false;

    return VectorTypeOperations<T>::compare(a.data(), b.data(), a.size());
}

template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity>
inline bool operator!=(const Vector<T, inlineCapacity, OverflowHandler, minCapacity>& a, const Vector<T, inlineCapacity, OverflowHandler, minCapacity>& b)
{
    return !(a == b);
}

#if !ASSERT_DISABLED
template<typename T> struct ValueCheck<Vector<T>> {
    typedef Vector<T> TraitType;
    static void checkConsistency(const Vector<T>& v)
    {
        v.checkConsistency();
    }
};
#endif

} // namespace WTF

using WTF::Vector;
using WTF::UnsafeVectorOverflow;
using WTF::notFound;

#endif // WTF_Vector_h
