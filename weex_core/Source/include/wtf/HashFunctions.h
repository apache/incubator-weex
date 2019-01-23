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
#ifndef WTF_HashFunctions_h
#define WTF_HashFunctions_h

#include <stdint.h>
#include <wtf/GetPtr.h>
#include <wtf/RefPtr.h>

namespace WTF {

    template<size_t size> struct IntTypes;
    template<> struct IntTypes<1> { typedef int8_t SignedType; typedef uint8_t UnsignedType; };
    template<> struct IntTypes<2> { typedef int16_t SignedType; typedef uint16_t UnsignedType; };
    template<> struct IntTypes<4> { typedef int32_t SignedType; typedef uint32_t UnsignedType; };
    template<> struct IntTypes<8> { typedef int64_t SignedType; typedef uint64_t UnsignedType; };

    // integer hash function

    // Thomas Wang's 32 Bit Mix Function: http://www.cris.com/~Ttwang/tech/inthash.htm
    inline unsigned intHash(uint8_t key8)
    {
        unsigned key = key8;
        key += ~(key << 15);
        key ^= (key >> 10);
        key += (key << 3);
        key ^= (key >> 6);
        key += ~(key << 11);
        key ^= (key >> 16);
        return key;
    }

    // Thomas Wang's 32 Bit Mix Function: http://www.cris.com/~Ttwang/tech/inthash.htm
    inline unsigned intHash(uint16_t key16)
    {
        unsigned key = key16;
        key += ~(key << 15);
        key ^= (key >> 10);
        key += (key << 3);
        key ^= (key >> 6);
        key += ~(key << 11);
        key ^= (key >> 16);
        return key;
    }

    // Thomas Wang's 32 Bit Mix Function: http://www.cris.com/~Ttwang/tech/inthash.htm
    inline unsigned intHash(uint32_t key) 
    {
        key += ~(key << 15);
        key ^= (key >> 10);
        key += (key << 3);
        key ^= (key >> 6);
        key += ~(key << 11);
        key ^= (key >> 16);
        return key;
    }
    
    // Thomas Wang's 64 bit Mix Function: http://www.cris.com/~Ttwang/tech/inthash.htm
    inline unsigned intHash(uint64_t key)
    {
        key += ~(key << 32);
        key ^= (key >> 22);
        key += ~(key << 13);
        key ^= (key >> 8);
        key += (key << 3);
        key ^= (key >> 15);
        key += ~(key << 27);
        key ^= (key >> 31);
        return static_cast<unsigned>(key);
    }

    // Compound integer hash method: http://opendatastructures.org/versions/edition-0.1d/ods-java/node33.html#SECTION00832000000000000000
    inline unsigned pairIntHash(unsigned key1, unsigned key2)
    {
        unsigned shortRandom1 = 277951225; // A random 32-bit value.
        unsigned shortRandom2 = 95187966; // A random 32-bit value.
        uint64_t longRandom = 19248658165952622LL; // A random 64-bit value.

        uint64_t product = longRandom * (shortRandom1 * key1 + shortRandom2 * key2);
        unsigned highBits = static_cast<unsigned>(product >> (sizeof(uint64_t) - sizeof(unsigned)));
        return highBits;
    }

    template<typename T> struct IntHash {
        static unsigned hash(T key) { return intHash(static_cast<typename IntTypes<sizeof(T)>::UnsignedType>(key)); }
        static bool equal(T a, T b) { return a == b; }
        static const bool safeToCompareToEmptyOrDeleted = true;
    };

    template<typename T> struct FloatHash {
        typedef typename IntTypes<sizeof(T)>::UnsignedType Bits;
        static unsigned hash(T key)
        {
            return intHash(bitwise_cast<Bits>(key));
        }
        static bool equal(T a, T b)
        {
            return bitwise_cast<Bits>(a) == bitwise_cast<Bits>(b);
        }
        static const bool safeToCompareToEmptyOrDeleted = true;
    };

    // pointer identity hash function

    template<typename T, bool isSmartPointer> 
    struct PtrHashBase;

    template <typename T>
    struct PtrHashBase<T, false /* isSmartPtr */> {
        typedef T PtrType; 

        static unsigned hash(PtrType key) { return IntHash<uintptr_t>::hash(reinterpret_cast<uintptr_t>(key)); }
        static bool equal(PtrType a, PtrType b) { return a == b; }
        static const bool safeToCompareToEmptyOrDeleted = true;
    };

    template <typename T>
    struct PtrHashBase<T, true /* isSmartPtr */> {
        typedef typename GetPtrHelper<T>::PtrType PtrType; 

        static unsigned hash(PtrType key) { return IntHash<uintptr_t>::hash(reinterpret_cast<uintptr_t>(key)); }
        static bool equal(PtrType a, PtrType b) { return a == b; }
        static const bool safeToCompareToEmptyOrDeleted = true;

        static unsigned hash(const T& key) { return hash(getPtr(key)); }
        static bool equal(const T& a, const T& b) { return getPtr(a) == getPtr(b); }
        static bool equal(PtrType a, const T& b) { return a == getPtr(b); }
        static bool equal(const T& a, PtrType b) { return getPtr(a) == b; }
    };

    template<typename T> struct PtrHash : PtrHashBase<T, IsSmartPtr<T>::value> {
    };

    template<typename P> struct PtrHash<Ref<P>> : PtrHashBase<Ref<P>, IsSmartPtr<Ref<P>>::value> {
        static const bool safeToCompareToEmptyOrDeleted = false;
    };

    // default hash function for each type

    template<typename T> struct DefaultHash;

    template<typename T, typename U> struct PairHash {
        static unsigned hash(const std::pair<T, U>& p)
        {
            return pairIntHash(DefaultHash<T>::Hash::hash(p.first), DefaultHash<U>::Hash::hash(p.second));
        }
        static bool equal(const std::pair<T, U>& a, const std::pair<T, U>& b)
        {
            return DefaultHash<T>::Hash::equal(a.first, b.first) && DefaultHash<U>::Hash::equal(a.second, b.second);
        }
        static const bool safeToCompareToEmptyOrDeleted = DefaultHash<T>::Hash::safeToCompareToEmptyOrDeleted && DefaultHash<U>::Hash::safeToCompareToEmptyOrDeleted;
    };

    template<typename T, typename U> struct IntPairHash {
        static unsigned hash(const std::pair<T, U>& p) { return pairIntHash(p.first, p.second); }
        static bool equal(const std::pair<T, U>& a, const std::pair<T, U>& b) { return PairHash<T, T>::equal(a, b); }
        static const bool safeToCompareToEmptyOrDeleted = PairHash<T, U>::safeToCompareToEmptyOrDeleted;
    };

    template<typename... Types>
    struct TupleHash {
        template<size_t I = 0>
        static typename std::enable_if<I < sizeof...(Types) - 1, unsigned>::type hash(const std::tuple<Types...>& t)
        {
            using IthTupleElementType = typename std::tuple_element<I, typename std::tuple<Types...>>::type;
            return pairIntHash(DefaultHash<IthTupleElementType>::Hash::hash(std::get<I>(t)), hash<I + 1>(t));
        }

        template<size_t I = 0>
        static typename std::enable_if<I == sizeof...(Types) - 1, unsigned>::type hash(const std::tuple<Types...>& t)
        {
            using IthTupleElementType = typename std::tuple_element<I, typename std::tuple<Types...>>::type;
            return DefaultHash<IthTupleElementType>::Hash::hash(std::get<I>(t));
        }

        template<size_t I = 0>
        static typename std::enable_if<I < sizeof...(Types) - 1, bool>::type equal(const std::tuple<Types...>& a, const std::tuple<Types...>& b)
        {
            using IthTupleElementType = typename std::tuple_element<I, typename std::tuple<Types...>>::type;
            return DefaultHash<IthTupleElementType>::Hash::equal(std::get<I>(a), std::get<I>(b)) && equal<I + 1>(a, b);
        }

        template<size_t I = 0>
        static typename std::enable_if<I == sizeof...(Types) - 1, bool>::type equal(const std::tuple<Types...>& a, const std::tuple<Types...>& b)
        {
            using IthTupleElementType = typename std::tuple_element<I, typename std::tuple<Types...>>::type;
            return DefaultHash<IthTupleElementType>::Hash::equal(std::get<I>(a), std::get<I>(b));
        }

        // We should use safeToCompareToEmptyOrDeleted = DefaultHash<Types>::Hash::safeToCompareToEmptyOrDeleted &&... whenever
        // we switch to C++17. We can't do anything better here right now because GCC can't do C++.
        template<typename BoolType>
        static constexpr bool allTrue(BoolType value) { return value; }
        template<typename BoolType, typename... BoolTypes>
        static constexpr bool allTrue(BoolType value, BoolTypes... values) { return value && allTrue(values...); }
        static const bool safeToCompareToEmptyOrDeleted = allTrue(DefaultHash<Types>::Hash::safeToCompareToEmptyOrDeleted...);
    };

    // make IntHash the default hash function for many integer types

    template<> struct DefaultHash<bool> { typedef IntHash<uint8_t> Hash; };
    template<> struct DefaultHash<short> { typedef IntHash<unsigned> Hash; };
    template<> struct DefaultHash<unsigned short> { typedef IntHash<unsigned> Hash; };
    template<> struct DefaultHash<int> { typedef IntHash<unsigned> Hash; };
    template<> struct DefaultHash<unsigned> { typedef IntHash<unsigned> Hash; };
    template<> struct DefaultHash<long> { typedef IntHash<unsigned long> Hash; };
    template<> struct DefaultHash<unsigned long> { typedef IntHash<unsigned long> Hash; };
    template<> struct DefaultHash<long long> { typedef IntHash<unsigned long long> Hash; };
    template<> struct DefaultHash<unsigned long long> { typedef IntHash<unsigned long long> Hash; };

#if defined(_NATIVE_WCHAR_T_DEFINED)
    template<> struct DefaultHash<wchar_t> { typedef IntHash<wchar_t> Hash; };
#endif

    template<> struct DefaultHash<float> { typedef FloatHash<float> Hash; };
    template<> struct DefaultHash<double> { typedef FloatHash<double> Hash; };

    // make PtrHash the default hash function for pointer types that don't specialize

    template<typename P> struct DefaultHash<P*> { typedef PtrHash<P*> Hash; };
    template<typename P> struct DefaultHash<RefPtr<P>> { typedef PtrHash<RefPtr<P>> Hash; };
    template<typename P> struct DefaultHash<Ref<P>> { typedef PtrHash<Ref<P>> Hash; };

    template<typename P, typename Deleter> struct DefaultHash<std::unique_ptr<P, Deleter>> { typedef PtrHash<std::unique_ptr<P, Deleter>> Hash; };

    // make IntPairHash the default hash function for pairs of (at most) 32-bit integers.

    template<> struct DefaultHash<std::pair<short, short>> { typedef IntPairHash<short, short> Hash; };
    template<> struct DefaultHash<std::pair<short, unsigned short>> { typedef IntPairHash<short, unsigned short> Hash; };
    template<> struct DefaultHash<std::pair<short, int>> { typedef IntPairHash<short, int> Hash; };
    template<> struct DefaultHash<std::pair<short, unsigned>> { typedef IntPairHash<short, unsigned> Hash; };
    template<> struct DefaultHash<std::pair<unsigned short, short>> { typedef IntPairHash<unsigned short, short> Hash; };
    template<> struct DefaultHash<std::pair<unsigned short, unsigned short>> { typedef IntPairHash<unsigned short, unsigned short> Hash; };
    template<> struct DefaultHash<std::pair<unsigned short, int>> { typedef IntPairHash<unsigned short, int> Hash; };
    template<> struct DefaultHash<std::pair<unsigned short, unsigned>> { typedef IntPairHash<unsigned short, unsigned> Hash; };
    template<> struct DefaultHash<std::pair<int, short>> { typedef IntPairHash<int, short> Hash; };
    template<> struct DefaultHash<std::pair<int, unsigned short>> { typedef IntPairHash<int, unsigned short> Hash; };
    template<> struct DefaultHash<std::pair<int, int>> { typedef IntPairHash<int, int> Hash; };
    template<> struct DefaultHash<std::pair<int, unsigned>> { typedef IntPairHash<unsigned, unsigned> Hash; };
    template<> struct DefaultHash<std::pair<unsigned, short>> { typedef IntPairHash<unsigned, short> Hash; };
    template<> struct DefaultHash<std::pair<unsigned, unsigned short>> { typedef IntPairHash<unsigned, unsigned short> Hash; };
    template<> struct DefaultHash<std::pair<unsigned, int>> { typedef IntPairHash<unsigned, int> Hash; };
    template<> struct DefaultHash<std::pair<unsigned, unsigned>> { typedef IntPairHash<unsigned, unsigned> Hash; };

    // make PairHash the default hash function for pairs of arbitrary values.

    template<typename T, typename U> struct DefaultHash<std::pair<T, U>> { typedef PairHash<T, U> Hash; };
    template<typename... Types> struct DefaultHash<std::tuple<Types...>> { typedef TupleHash<Types...> Hash; };

} // namespace WTF

using WTF::DefaultHash;
using WTF::IntHash;
using WTF::PtrHash;

#endif // WTF_HashFunctions_h
