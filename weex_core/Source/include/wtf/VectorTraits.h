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
