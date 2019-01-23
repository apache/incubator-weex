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

#include <type_traits>

namespace WTF {

template<typename> struct EnumTraits;

template<typename E, E...> struct EnumValues;

template<typename T, typename E> struct EnumValueChecker;

template<typename T, typename E, E e, E... es>
struct EnumValueChecker<T, EnumValues<E, e, es...>> {
    static constexpr bool isValidEnum(T t)
    {
        return (static_cast<T>(e) == t) ? true : EnumValueChecker<T, EnumValues<E, es...>>::isValidEnum(t);
    }
};

template<typename T, typename E>
struct EnumValueChecker<T, EnumValues<E>> {
    static constexpr bool isValidEnum(T)
    {
        return false;
    }
};

template<typename E, typename T>
constexpr auto isValidEnum(T t) -> std::enable_if_t<std::is_enum<E>::value, bool>
{
    static_assert(sizeof(T) >= std::underlying_type_t<E>(), "Integral type must be at least the size of the underlying enum type");

    return EnumValueChecker<T, typename EnumTraits<E>::values>::isValidEnum(t);
}

}

using WTF::isValidEnum;
