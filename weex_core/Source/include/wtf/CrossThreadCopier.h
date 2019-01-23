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

#include <wtf/Assertions.h>
#include <wtf/Forward.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/Threading.h>
#include <wtf/text/WTFString.h>

namespace WTF {

struct CrossThreadCopierBaseHelper {
    template<typename T> struct RemovePointer {
        typedef T Type;
    };
    template<typename T> struct RemovePointer<T*> {
        typedef T Type;
    };

    template<typename T> struct RemovePointer<RefPtr<T>> {
        typedef T Type;
    };

    template<typename T> struct RemovePointer<PassRefPtr<T>> {
        typedef T Type;
    };

    template<typename T> struct IsEnumOrConvertibleToInteger {
        static const bool value = std::is_integral<T>::value || std::is_enum<T>::value || std::is_convertible<T, long double>::value;
    };

    template<typename T> struct IsThreadSafeRefCountedPointer {
        static const bool value = std::is_convertible<typename RemovePointer<T>::Type*, ThreadSafeRefCounted<typename RemovePointer<T>::Type>*>::value;
    };
};

template<typename T> struct CrossThreadCopierPassThrough {
    typedef T Type;
    static Type copy(const T& parameter)
    {
        return parameter;
    }
};

template<bool isEnumOrConvertibleToInteger, bool isThreadSafeRefCounted, typename T> struct CrossThreadCopierBase;

// Integers get passed through without any changes.
template<typename T> struct CrossThreadCopierBase<true, false, T> : public CrossThreadCopierPassThrough<T> {
};

// Classes that have an isolatedCopy() method get a default specialization.
template<class T> struct CrossThreadCopierBase<false, false, T> {
    static T copy(const T& value)
    {
        return value.isolatedCopy();
    }
};

// Custom copy methods.
template<typename T> struct CrossThreadCopierBase<false, true, T> {
    typedef typename CrossThreadCopierBaseHelper::RemovePointer<T>::Type RefCountedType;
    static_assert(std::is_convertible<RefCountedType*, ThreadSafeRefCounted<RefCountedType>*>::value, "T is not convertible to ThreadSafeRefCounted!");

    typedef PassRefPtr<RefCountedType> Type;
    static Type copy(const T& refPtr)
    {
        return refPtr;
    }
};

template<> struct CrossThreadCopierBase<false, false, std::chrono::system_clock::time_point> {
    typedef std::chrono::system_clock::time_point Type;
    static Type copy(const Type& source)
    {
        return source;
    }
};

template<> struct CrossThreadCopierBase<false, false, WTF::ASCIILiteral> {
    typedef WTF::ASCIILiteral Type;
    static Type copy(const Type& source)
    {
        return source;
    }
};

template<typename T>
struct CrossThreadCopier : public CrossThreadCopierBase<CrossThreadCopierBaseHelper::IsEnumOrConvertibleToInteger<T>::value, CrossThreadCopierBaseHelper::IsThreadSafeRefCountedPointer<T>::value, T> {
};

// Default specialization for Vectors of CrossThreadCopyable classes.
template<typename T> struct CrossThreadCopierBase<false, false, Vector<T>> {
    typedef Vector<T> Type;
    static Type copy(const Type& source)
    {
        Type destination;
        destination.reserveInitialCapacity(source.size());
        for (auto& object : source)
            destination.uncheckedAppend(CrossThreadCopier<T>::copy(object));
        return destination;
    }
};

} // namespace WTF

using WTF::CrossThreadCopierBaseHelper;
using WTF::CrossThreadCopierBase;
using WTF::CrossThreadCopier;
