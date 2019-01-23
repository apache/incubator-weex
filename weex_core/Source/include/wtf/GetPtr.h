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
