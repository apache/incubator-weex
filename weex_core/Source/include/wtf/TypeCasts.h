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
#ifndef TypeCasts_h
#define TypeCasts_h

#include <type_traits>

namespace WTF {

template <typename ExpectedType, typename ArgType, bool isBaseType = std::is_base_of<ExpectedType, ArgType>::value>
struct TypeCastTraits {
    static bool isOfType(ArgType&)
    {
        // If you're hitting this assertion, it is likely because you used
        // is<>() or downcast<>() with a type that doesn't have the needed
        // TypeCastTraits specialization. Please use the following macro
        // to add that specialization:
        // SPECIALIZE_TYPE_TRAITS_BEGIN() / SPECIALIZE_TYPE_TRAITS_END()
        static_assert(std::is_void<ExpectedType>::value, "Missing TypeCastTraits specialization");
        return false;
    }
};

// Template specialization for the case where ExpectedType is a base of ArgType,
// so we can return return true unconditionally.
template <typename ExpectedType, typename ArgType>
struct TypeCastTraits<ExpectedType, ArgType, true /* isBaseType */> {
    static bool isOfType(ArgType&) { return true; }
};

// Type checking function, to use before casting with downcast<>().
template <typename ExpectedType, typename ArgType>
inline bool is(ArgType& source)
{
    static_assert(std::is_base_of<ArgType, ExpectedType>::value, "Unnecessary type check");
    return TypeCastTraits<const ExpectedType, const ArgType>::isOfType(source);
}

template <typename ExpectedType, typename ArgType>
inline bool is(ArgType* source)
{
    static_assert(std::is_base_of<ArgType, ExpectedType>::value, "Unnecessary type check");
    return source && TypeCastTraits<const ExpectedType, const ArgType>::isOfType(*source);
}

// Update T's constness to match Reference's.
template <typename Reference, typename T>
struct match_constness {
    typedef typename std::conditional<std::is_const<Reference>::value, typename std::add_const<T>::type, typename std::remove_const<T>::type>::type type;
};

// Safe downcasting functions.
template<typename Target, typename Source>
inline typename match_constness<Source, Target>::type& downcast(Source& source)
{
    static_assert(!std::is_same<Source, Target>::value, "Unnecessary cast to same type");
    static_assert(std::is_base_of<Source, Target>::value, "Should be a downcast");
    ASSERT_WITH_SECURITY_IMPLICATION(is<Target>(source));
    return static_cast<typename match_constness<Source, Target>::type&>(source);
}
template<typename Target, typename Source>
inline typename match_constness<Source, Target>::type* downcast(Source* source)
{
    static_assert(!std::is_same<Source, Target>::value, "Unnecessary cast to same type");
    static_assert(std::is_base_of<Source, Target>::value, "Should be a downcast");
    ASSERT_WITH_SECURITY_IMPLICATION(!source || is<Target>(*source));
    return static_cast<typename match_constness<Source, Target>::type*>(source);
}

// Add support for type checking / casting using is<>() / downcast<>() helpers for a specific class.
#define SPECIALIZE_TYPE_TRAITS_BEGIN(ClassName) \
namespace WTF { \
template <typename ArgType> \
class TypeCastTraits<const ClassName, ArgType, false /* isBaseType */> { \
public: \
    static bool isOfType(ArgType& source) { return isType(source); } \
private:

#define SPECIALIZE_TYPE_TRAITS_END() \
}; \
}

} // namespace WTF

using WTF::TypeCastTraits;
using WTF::is;
using WTF::downcast;

#endif // TypeCasts_h
