/*
 *  Copyright (C) 2006 Apple Inc.
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

#ifndef WTF_GetPtr_h
#define WTF_GetPtr_h

#include <memory>

namespace WTF {

enum HashTableDeletedValueType { HashTableDeletedValue };
enum HashTableEmptyValueType { HashTableEmptyValue };

template <typename T> inline T* getPtr(T* p) { return p; }

template <typename T> struct IsSmartPtr {
    static const bool value = false;
};

template <typename T, bool isSmartPtr>
struct GetPtrHelperBase;

template <typename T>
struct GetPtrHelperBase<T, false /* isSmartPtr */> {
    typedef T* PtrType;
    static T* getPtr(T& p) { return std::addressof(p); }
};

template <typename T>
struct GetPtrHelperBase<T, true /* isSmartPtr */> {
    typedef typename T::PtrType PtrType;
    static PtrType getPtr(const T& p) { return p.get(); }
};

template <typename T>
struct GetPtrHelper : GetPtrHelperBase<T, IsSmartPtr<T>::value> {
};

template <typename T>
inline typename GetPtrHelper<T>::PtrType getPtr(T& p)
{
    return GetPtrHelper<T>::getPtr(p);
}

template <typename T>
inline typename GetPtrHelper<T>::PtrType getPtr(const T& p)
{
    return GetPtrHelper<T>::getPtr(p);
}

// Explicit specialization for C++ standard library types.

template <typename T, typename Deleter> struct IsSmartPtr<std::unique_ptr<T, Deleter>> {
    static const bool value = true;
};

template <typename T, typename Deleter>
struct GetPtrHelper<std::unique_ptr<T, Deleter>> {
    typedef T* PtrType;
    static T* getPtr(const std::unique_ptr<T, Deleter>& p) { return p.get(); }
};

} // namespace WTF

#endif // WTF_GetPtr_h
