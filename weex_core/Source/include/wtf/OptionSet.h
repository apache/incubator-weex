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

#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <wtf/Assertions.h>
#include <wtf/MathExtras.h>

namespace WTF {

// OptionSet is a class that represents a set of enumerators in a space-efficient manner. The enumerators
// must be powers of two greater than 0. This class is useful as a replacement for passing a bitmask of
// enumerators around.
template<typename T> class OptionSet {
    static_assert(std::is_enum<T>::value, "T is not an enum type");
    typedef typename std::make_unsigned<typename std::underlying_type<T>::type>::type StorageType;

public:
    template<typename StorageType> class Iterator {
    public:
        // Isolate the rightmost set bit.
        T operator*() const { return static_cast<T>(m_value & -m_value); }

        // Iterates from smallest to largest enum value by turning off the rightmost set bit.
        Iterator& operator++()
        {
            m_value &= m_value - 1;
            return *this;
        }

        Iterator& operator++(int) = delete;

        bool operator==(const Iterator& other) const { return m_value == other.m_value; }
        bool operator!=(const Iterator& other) const { return m_value != other.m_value; }

    private:
        Iterator(StorageType value) : m_value(value) { }
        friend OptionSet;

        StorageType m_value;
    };
    using iterator = Iterator<StorageType>;

    static constexpr OptionSet fromRaw(StorageType storageType)
    {
        return OptionSet(static_cast<T>(storageType), FromRawValue);
    }

    constexpr OptionSet() = default;

#if ASSERT_DISABLED
    constexpr OptionSet(T t)
        : m_storage(static_cast<StorageType>(t))
    {
    }
#else
    OptionSet(T t)
        : m_storage(static_cast<StorageType>(t))
    {
        ASSERT_WITH_MESSAGE(hasOneBitSet(static_cast<StorageType>(t)), "Enumerator is not a positive power of two.");
    }
#endif

    // FIXME: Make this constexpr once we adopt C++14 as C++11 does not support for-loops
    // in a constexpr function.
    OptionSet(std::initializer_list<T> initializerList)
    {
        for (auto& option : initializerList) {
            ASSERT_WITH_MESSAGE(hasOneBitSet(static_cast<StorageType>(option)), "Enumerator is not a positive power of two.");
            m_storage |= static_cast<StorageType>(option);
        }
    }

    constexpr StorageType toRaw() const { return m_storage; }

    constexpr bool isEmpty() const { return !m_storage; }

    constexpr iterator begin() const { return m_storage; }
    constexpr iterator end() const { return 0; }

    constexpr bool contains(OptionSet optionSet) const
    {
        return m_storage & optionSet.m_storage;
    }

    constexpr friend bool operator==(OptionSet lhs, OptionSet rhs)
    {
        return lhs.m_storage == rhs.m_storage;
    }

    constexpr friend bool operator!=(OptionSet lhs, OptionSet rhs)
    {
        return lhs.m_storage != rhs.m_storage;
    }

    friend OptionSet& operator|=(OptionSet& lhs, OptionSet rhs)
    {
        lhs.m_storage |= rhs.m_storage;

        return lhs;
    }

    friend OptionSet& operator-=(OptionSet& lhs, OptionSet rhs)
    {
        lhs.m_storage &= ~rhs.m_storage;

        return lhs;
    }

    constexpr friend OptionSet operator-(OptionSet lhs, OptionSet rhs)
    {
        return OptionSet::fromRaw(lhs.m_storage & ~rhs.m_storage);
    }

private:
    enum InitializationTag { FromRawValue };
    constexpr OptionSet(T t, InitializationTag)
        : m_storage(static_cast<StorageType>(t))
    {
    }
    StorageType m_storage { 0 };
};

}

using WTF::OptionSet;
