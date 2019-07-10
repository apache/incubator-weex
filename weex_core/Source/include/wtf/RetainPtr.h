/*
 *  Copyright (C) 2005, 2006, 2007, 2008, 2010, 2013, 2014 Apple Inc. All rights reserved.
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

#ifndef RetainPtr_h
#define RetainPtr_h

#include <wtf/Platform.h>

#if USE(CF) || defined(__OBJC__)

#include <wtf/HashTraits.h>
#include <algorithm>
#include <cstddef>

#if USE(CF)
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif

#ifndef CF_RELEASES_ARGUMENT
#define CF_RELEASES_ARGUMENT
#endif

#ifndef NS_RELEASES_ARGUMENT
#define NS_RELEASES_ARGUMENT
#endif

namespace WTF {

// Unlike most most of our smart pointers, RetainPtr can take either the pointer type or the pointed-to type,
// so both RetainPtr<NSDictionary> and RetainPtr<CFDictionaryRef> will work.

template<typename T> class RetainPtr;

template<typename T> RetainPtr<T> adoptCF(T CF_RELEASES_ARGUMENT) WARN_UNUSED_RETURN;
template<typename T> RetainPtr<T> adoptNS(T NS_RELEASES_ARGUMENT) WARN_UNUSED_RETURN;

template<typename T> class RetainPtr {
public:
    typedef typename std::remove_pointer<T>::type ValueType;
    typedef ValueType* PtrType;
    typedef CFTypeRef StorageType;

    RetainPtr() : m_ptr(nullptr) { }
    RetainPtr(PtrType ptr) : m_ptr(toStorageType(ptr)) { if (m_ptr) CFRetain(m_ptr); }

    RetainPtr(const RetainPtr& o) : m_ptr(o.m_ptr) { if (StorageType ptr = m_ptr) CFRetain(ptr); }

    RetainPtr(RetainPtr&& o) : m_ptr(toStorageType(o.leakRef())) { }
    template<typename U> RetainPtr(RetainPtr<U>&& o) : m_ptr(toStorageType(o.leakRef())) { }

    // Hash table deleted values, which are only constructed and never copied or destroyed.
    RetainPtr(HashTableDeletedValueType) : m_ptr(hashTableDeletedValue()) { }
    bool isHashTableDeletedValue() const { return m_ptr == hashTableDeletedValue(); }
    
    ~RetainPtr();
    
    template<typename U> RetainPtr(const RetainPtr<U>&);

    void clear();
    PtrType leakRef() WARN_UNUSED_RETURN;
    PtrType autorelease();

    PtrType get() const { return fromStorageType(m_ptr); }
    PtrType operator->() const { return fromStorageType(m_ptr); }
    explicit operator PtrType() const { return fromStorageType(m_ptr); }
    explicit operator bool() const { return m_ptr; }

    bool operator!() const { return !m_ptr; }

#if !(defined (__OBJC__) && __has_feature(objc_arc))
    // This function is useful for passing RetainPtrs to functions that return
    // CF types as out parameters.
    PtrType* operator&()
    {
        // Require that the pointer is null, to prevent leaks.
        ASSERT(!m_ptr);

        return (PtrType*)&m_ptr;
    }
#endif

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    typedef StorageType RetainPtr::*UnspecifiedBoolType;
    operator UnspecifiedBoolType() const { return m_ptr ? &RetainPtr::m_ptr : nullptr; }
    
    RetainPtr& operator=(const RetainPtr&);
    template<typename U> RetainPtr& operator=(const RetainPtr<U>&);
    RetainPtr& operator=(PtrType);
    template<typename U> RetainPtr& operator=(U*);

    RetainPtr& operator=(RetainPtr&&);
    template<typename U> RetainPtr& operator=(RetainPtr<U>&&);

    void swap(RetainPtr&);

    template<typename U> friend RetainPtr<U> adoptCF(U CF_RELEASES_ARGUMENT) WARN_UNUSED_RETURN;
    template<typename U> friend RetainPtr<U> adoptNS(U NS_RELEASES_ARGUMENT) WARN_UNUSED_RETURN;

private:
    enum AdoptTag { Adopt };
    RetainPtr(PtrType ptr, AdoptTag) : m_ptr(toStorageType(ptr)) { }

    static PtrType hashTableDeletedValue() { return reinterpret_cast<PtrType>(-1); }

#if defined (__OBJC__) && __has_feature(objc_arc)
    template<typename U>
    typename std::enable_if<std::is_convertible<U, id>::value, PtrType>::type
    fromStorageTypeHelper(StorageType ptr) const
    {
        return (__bridge PtrType)ptr;
    }

    template<typename U>
    typename std::enable_if<!std::is_convertible<U, id>::value, PtrType>::type
    fromStorageTypeHelper(StorageType ptr) const
    {
        return (PtrType)ptr;
    }

    PtrType fromStorageType(StorageType ptr) const { return fromStorageTypeHelper<PtrType>(ptr); }
    StorageType toStorageType(id ptr) const { return (__bridge StorageType)ptr; }
    StorageType toStorageType(CFTypeRef ptr) const { return (StorageType)ptr; }
#else
    PtrType fromStorageType(StorageType ptr) const { return (PtrType)ptr; }
    StorageType toStorageType(PtrType ptr) const { return (StorageType)ptr; }
#endif

    StorageType m_ptr;
};

template<typename T> inline RetainPtr<T>::~RetainPtr()
{
    if (StorageType ptr = std::exchange(m_ptr, nullptr))
        CFRelease(ptr);
}

// Helper function for creating a RetainPtr using template argument deduction.
template<typename T> inline RetainPtr<T> retainPtr(T) WARN_UNUSED_RETURN;

template<typename T> template<typename U> inline RetainPtr<T>::RetainPtr(const RetainPtr<U>& o)
    : m_ptr(toStorageType(o.get()))
{
    if (StorageType ptr = m_ptr)
        CFRetain(ptr);
}

template<typename T> inline void RetainPtr<T>::clear()
{
    if (StorageType ptr = std::exchange(m_ptr, nullptr))
        CFRelease(ptr);
}

template<typename T> inline typename RetainPtr<T>::PtrType RetainPtr<T>::leakRef()
{
    return fromStorageType(std::exchange(m_ptr, nullptr));
}

#ifdef __OBJC__
template<typename T> inline auto RetainPtr<T>::autorelease() -> PtrType
{
    return (__bridge PtrType)CFBridgingRelease(leakRef());
}
#endif

template<typename T> inline RetainPtr<T>& RetainPtr<T>::operator=(const RetainPtr& o)
{
    RetainPtr ptr = o;
    swap(ptr);
    return *this;
}

template<typename T> template<typename U> inline RetainPtr<T>& RetainPtr<T>::operator=(const RetainPtr<U>& o)
{
    RetainPtr ptr = o;
    swap(ptr);
    return *this;
}

template<typename T> inline RetainPtr<T>& RetainPtr<T>::operator=(PtrType optr)
{
    RetainPtr ptr = optr;
    swap(ptr);
    return *this;
}

template<typename T> template<typename U> inline RetainPtr<T>& RetainPtr<T>::operator=(U* optr)
{
    RetainPtr ptr = optr;
    swap(ptr);
    return *this;
}

template<typename T> inline RetainPtr<T>& RetainPtr<T>::operator=(RetainPtr&& o)
{
    RetainPtr ptr = WTFMove(o);
    swap(ptr);
    return *this;
}

template<typename T> template<typename U> inline RetainPtr<T>& RetainPtr<T>::operator=(RetainPtr<U>&& o)
{
    RetainPtr ptr = WTFMove(o);
    swap(ptr);
    return *this;
}

template<typename T> inline void RetainPtr<T>::swap(RetainPtr& o)
{
    std::swap(m_ptr, o.m_ptr);
}

template<typename T> inline void swap(RetainPtr<T>& a, RetainPtr<T>& b)
{
    a.swap(b);
}

template<typename T, typename U> inline bool operator==(const RetainPtr<T>& a, const RetainPtr<U>& b)
{ 
    return a.get() == b.get(); 
}

template<typename T, typename U> inline bool operator==(const RetainPtr<T>& a, U* b)
{
    return a.get() == b; 
}

template<typename T, typename U> inline bool operator==(T* a, const RetainPtr<U>& b) 
{
    return a == b.get(); 
}

template<typename T, typename U> inline bool operator!=(const RetainPtr<T>& a, const RetainPtr<U>& b)
{ 
    return a.get() != b.get(); 
}

template<typename T, typename U> inline bool operator!=(const RetainPtr<T>& a, U* b)
{
    return a.get() != b; 
}

template<typename T, typename U> inline bool operator!=(T* a, const RetainPtr<U>& b)
{ 
    return a != b.get(); 
}

template<typename T> inline RetainPtr<T> adoptCF(T CF_RELEASES_ARGUMENT ptr)
{
#ifdef __OBJC__
    static_assert((!std::is_convertible<T, id>::value), "Don't use adoptCF with Objective-C pointer types, use adoptNS.");
#endif
    return RetainPtr<T>(ptr, RetainPtr<T>::Adopt);
}

#ifdef __OBJC__
template<typename T> inline RetainPtr<T> adoptNS(T NS_RELEASES_ARGUMENT ptr)
{
#if __has_feature(objc_arc)
    return ptr;
#elif defined(OBJC_NO_GC)
    return RetainPtr<T>(ptr, RetainPtr<T>::Adopt);
#else
    RetainPtr<T> result = ptr;
    [ptr release];
    return result;
#endif
}
#endif

template<typename T> inline RetainPtr<T> retainPtr(T ptr)
{
    return ptr;
}

template <typename T> struct IsSmartPtr<RetainPtr<T>> {
    static const bool value = true;
};

template<typename P> struct HashTraits<RetainPtr<P>> : SimpleClassHashTraits<RetainPtr<P>> {
};

template<typename P> struct DefaultHash<RetainPtr<P>> {
    typedef PtrHash<RetainPtr<P>> Hash;
};

template <typename P>
struct RetainPtrObjectHashTraits : SimpleClassHashTraits<RetainPtr<P>> {
    static const RetainPtr<P>& emptyValue()
    {
        static RetainPtr<P>& null = *(new RetainPtr<P>);
        return null;
    }
};

template <typename P>
struct RetainPtrObjectHash {
    static unsigned hash(const RetainPtr<P>& o)
    {
        ASSERT_WITH_MESSAGE(o.get(), "attempt to use null RetainPtr in HashTable");
        return static_cast<unsigned>(CFHash(o.get()));
    }
    static bool equal(const RetainPtr<P>& a, const RetainPtr<P>& b)
    {
        return CFEqual(a.get(), b.get());
    }
    static const bool safeToCompareToEmptyOrDeleted = false;
};

#ifdef __OBJC__
template<typename T> T* dynamic_objc_cast(id object)
{
    if ([object isKindOfClass:[T class]])
        return (T *)object;

    return nil;
}
#endif

} // namespace WTF

using WTF::RetainPtr;
using WTF::adoptCF;
using WTF::adoptNS;
using WTF::retainPtr;

#ifdef __OBJC__
using WTF::dynamic_objc_cast;
#endif

#endif // USE(CF) || defined(__OBJC__)

#endif // WTF_RetainPtr_h
