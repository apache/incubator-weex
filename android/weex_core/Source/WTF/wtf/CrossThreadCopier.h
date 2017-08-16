/*
 * Copyright (C) 2009, 2010 Google Inc. All rights reserved.
 * Copyright (C) 2014, 2015, 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
