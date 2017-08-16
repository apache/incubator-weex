/*
 * Copyright (C) 2006, 2007, 2008, 2016 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef WTF_VectorTraits_h
#define WTF_VectorTraits_h

#include <wtf/Ref.h>
#include <wtf/RefPtr.h>
#include <utility>
#include <memory>
#include <type_traits>

namespace WTF {

    class AtomicString;

    template<bool isPod, typename T>
    struct VectorTraitsBase;

    template<typename T>
    struct VectorTraitsBase<false, T>
    {
        static const bool needsInitialization = true;
        static const bool canInitializeWithMemset = false;
        static const bool canMoveWithMemcpy = false;
        static const bool canCopyWithMemcpy = false;
        static const bool canFillWithMemset = false;
        static const bool canCompareWithMemcmp = false;
    };

    template<typename T>
    struct VectorTraitsBase<true, T>
    {
        static const bool needsInitialization = false;
        static const bool canInitializeWithMemset = false;
        static const bool canMoveWithMemcpy = true;
        static const bool canCopyWithMemcpy = true;
        static const bool canFillWithMemset = sizeof(T) == sizeof(char) && std::is_integral<T>::value;
        static const bool canCompareWithMemcmp = true;
    };

    template<typename T>
    struct VectorTraits : VectorTraitsBase<std::is_pod<T>::value, T> { };

    struct SimpleClassVectorTraits : VectorTraitsBase<false, void>
    {
        static const bool canInitializeWithMemset = true;
        static const bool canMoveWithMemcpy = true;
        static const bool canCompareWithMemcmp = true;
    };

    // We know smart pointers are simple enough that initializing to 0 and moving with memcpy
    // (and then not destructing the original) will work.

    template<typename P> struct VectorTraits<RefPtr<P>> : SimpleClassVectorTraits { };
    template<typename P> struct VectorTraits<std::unique_ptr<P>> : SimpleClassVectorTraits { };
    template<typename P> struct VectorTraits<Ref<P>> : SimpleClassVectorTraits { };
    template<> struct VectorTraits<AtomicString> : SimpleClassVectorTraits { };

    template<typename First, typename Second>
    struct VectorTraits<std::pair<First, Second>>
    {
        typedef VectorTraits<First> FirstTraits;
        typedef VectorTraits<Second> SecondTraits;

        static const bool needsInitialization = FirstTraits::needsInitialization || SecondTraits::needsInitialization;
        static const bool canInitializeWithMemset = FirstTraits::canInitializeWithMemset && SecondTraits::canInitializeWithMemset;
        static const bool canMoveWithMemcpy = FirstTraits::canMoveWithMemcpy && SecondTraits::canMoveWithMemcpy;
        static const bool canCopyWithMemcpy = FirstTraits::canCopyWithMemcpy && SecondTraits::canCopyWithMemcpy;
        static const bool canFillWithMemset = false;
        static const bool canCompareWithMemcmp = FirstTraits::canCompareWithMemcmp && SecondTraits::canCompareWithMemcmp;
    };

} // namespace WTF

using WTF::VectorTraits;
using WTF::SimpleClassVectorTraits;

#endif // WTF_VectorTraits_h
