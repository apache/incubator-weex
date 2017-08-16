/*
 * Copyright (C) 2010, 2014-2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <utility>
#include <wtf/Forward.h>
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/SHA1.h>
#include <wtf/Seconds.h>
#include <wtf/Vector.h>
#include <wtf/persistence/Decoder.h>
#include <wtf/persistence/Encoder.h>

namespace WTF {
namespace Persistence {

template<typename T, typename U> struct Coder<std::pair<T, U>> {
    static void encode(Encoder& encoder, const std::pair<T, U>& pair)
    {
        encoder << pair.first << pair.second;
    }

    static bool decode(Decoder& decoder, std::pair<T, U>& pair)
    {
        T first;
        if (!decoder.decode(first))
            return false;

        U second;
        if (!decoder.decode(second))
            return false;

        pair.first = first;
        pair.second = second;
        return true;
    }
};

template<typename Rep, typename Period> struct Coder<std::chrono::duration<Rep, Period>> {
    static void encode(Encoder& encoder, const std::chrono::duration<Rep, Period>& duration)
    {
        static_assert(std::is_integral<Rep>::value && std::is_signed<Rep>::value && sizeof(Rep) <= sizeof(int64_t), "Serialization of this Rep type is not supported yet. Only signed integer type which can be fit in an int64_t is currently supported.");
        encoder << static_cast<int64_t>(duration.count());
    }

    static bool decode(Decoder& decoder, std::chrono::duration<Rep, Period>& result)
    {
        int64_t count;
        if (!decoder.decode(count))
            return false;
        result = std::chrono::duration<Rep, Period>(static_cast<Rep>(count));
        return true;
    }
};

template<typename T> struct Coder<std::optional<T>> {
    static void encode(Encoder& encoder, const std::optional<T>& optional)
    {
        if (!optional) {
            encoder << false;
            return;
        }
        
        encoder << true;
        encoder << optional.value();
    }
    
    static bool decode(Decoder& decoder, std::optional<T>& optional)
    {
        bool isEngaged;
        if (!decoder.decode(isEngaged))
            return false;
        
        if (!isEngaged) {
            optional = std::nullopt;
            return true;
        }
        
        T value;
        if (!decoder.decode(value))
            return false;
        
        optional = WTFMove(value);
        return true;
    }
};

template<typename KeyType, typename ValueType> struct Coder<WTF::KeyValuePair<KeyType, ValueType>> {
    static void encode(Encoder& encoder, const WTF::KeyValuePair<KeyType, ValueType>& pair)
    {
        encoder << pair.key << pair.value;
    }

    static bool decode(Decoder& decoder, WTF::KeyValuePair<KeyType, ValueType>& pair)
    {
        KeyType key;
        if (!decoder.decode(key))
            return false;

        ValueType value;
        if (!decoder.decode(value))
            return false;

        pair.key = key;
        pair.value = value;
        return true;
    }
};

template<bool fixedSizeElements, typename T, size_t inlineCapacity> struct VectorCoder;

template<typename T, size_t inlineCapacity> struct VectorCoder<false, T, inlineCapacity> {
    static void encode(Encoder& encoder, const Vector<T, inlineCapacity>& vector)
    {
        encoder << static_cast<uint64_t>(vector.size());
        for (size_t i = 0; i < vector.size(); ++i)
            encoder << vector[i];
    }

    static bool decode(Decoder& decoder, Vector<T, inlineCapacity>& vector)
    {
        uint64_t size;
        if (!decoder.decode(size))
            return false;

        Vector<T, inlineCapacity> tmp;
        for (size_t i = 0; i < size; ++i) {
            T element;
            if (!decoder.decode(element))
                return false;
            
            tmp.append(WTFMove(element));
        }

        tmp.shrinkToFit();
        vector.swap(tmp);
        return true;
    }
};

template<typename T, size_t inlineCapacity> struct VectorCoder<true, T, inlineCapacity> {
    static void encode(Encoder& encoder, const Vector<T, inlineCapacity>& vector)
    {
        encoder << static_cast<uint64_t>(vector.size());
        encoder.encodeFixedLengthData(reinterpret_cast<const uint8_t*>(vector.data()), vector.size() * sizeof(T), alignof(T));
    }
    
    static bool decode(Decoder& decoder, Vector<T, inlineCapacity>& vector)
    {
        uint64_t size;
        if (!decoder.decode(size))
            return false;

        // Since we know the total size of the elements, we can allocate the vector in
        // one fell swoop. Before allocating we must however make sure that the decoder buffer
        // is big enough.
        if (!decoder.bufferIsLargeEnoughToContain<T>(size))
            return false;

        Vector<T, inlineCapacity> temp;
        temp.resize(size);

        decoder.decodeFixedLengthData(reinterpret_cast<uint8_t*>(temp.data()), size * sizeof(T));

        vector.swap(temp);
        return true;
    }
};

template<typename T, size_t inlineCapacity> struct Coder<Vector<T, inlineCapacity>> : VectorCoder<std::is_arithmetic<T>::value, T, inlineCapacity> { };

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg> struct Coder<HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>> {
    typedef HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg> HashMapType;

    static void encode(Encoder& encoder, const HashMapType& hashMap)
    {
        encoder << static_cast<uint64_t>(hashMap.size());
        for (typename HashMapType::const_iterator it = hashMap.begin(), end = hashMap.end(); it != end; ++it)
            encoder << *it;
    }

    static bool decode(Decoder& decoder, HashMapType& hashMap)
    {
        uint64_t hashMapSize;
        if (!decoder.decode(hashMapSize))
            return false;

        HashMapType tempHashMap;
        for (uint64_t i = 0; i < hashMapSize; ++i) {
            KeyArg key;
            MappedArg value;
            if (!decoder.decode(key))
                return false;
            if (!decoder.decode(value))
                return false;

            if (!tempHashMap.add(key, value).isNewEntry) {
                // The hash map already has the specified key, bail.
                return false;
            }
        }

        hashMap.swap(tempHashMap);
        return true;
    }
};

template<typename KeyArg, typename HashArg, typename KeyTraitsArg> struct Coder<HashSet<KeyArg, HashArg, KeyTraitsArg>> {
    typedef HashSet<KeyArg, HashArg, KeyTraitsArg> HashSetType;

    static void encode(Encoder& encoder, const HashSetType& hashSet)
    {
        encoder << static_cast<uint64_t>(hashSet.size());
        for (typename HashSetType::const_iterator it = hashSet.begin(), end = hashSet.end(); it != end; ++it)
            encoder << *it;
    }

    static bool decode(Decoder& decoder, HashSetType& hashSet)
    {
        uint64_t hashSetSize;
        if (!decoder.decode(hashSetSize))
            return false;

        HashSetType tempHashSet;
        for (uint64_t i = 0; i < hashSetSize; ++i) {
            KeyArg key;
            if (!decoder.decode(key))
                return false;

            if (!tempHashSet.add(key).isNewEntry) {
                // The hash map already has the specified key, bail.
                return false;
            }
        }

        hashSet.swap(tempHashSet);
        return true;
    }
};

template<> struct Coder<std::chrono::system_clock::time_point> {
    static void encode(Encoder& encoder, const std::chrono::system_clock::time_point& timePoint)
    {
        encoder << static_cast<int64_t>(timePoint.time_since_epoch().count());
    }
    
    static bool decode(Decoder& decoder, std::chrono::system_clock::time_point& result)
    {
        int64_t time;
        if (!decoder.decode(time))
            return false;

        result = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(static_cast<std::chrono::system_clock::rep>(time)));
        return true;
    }
};

template<> struct Coder<Seconds> {
    static void encode(Encoder& encoder, const Seconds& seconds)
    {
        encoder << seconds.value();
    }

    static bool decode(Decoder& decoder, Seconds& result)
    {
        double value;
        if (!decoder.decode(value))
            return false;

        result = Seconds(value);
        return true;
    }
};

template<> struct Coder<AtomicString> {
    WTF_EXPORT_PRIVATE static void encode(Encoder&, const AtomicString&);
    WTF_EXPORT_PRIVATE static bool decode(Decoder&, AtomicString&);
};

template<> struct Coder<CString> {
    WTF_EXPORT_PRIVATE static void encode(Encoder&, const CString&);
    WTF_EXPORT_PRIVATE static bool decode(Decoder&, CString&);
};

template<> struct Coder<String> {
    WTF_EXPORT_PRIVATE static void encode(Encoder&, const String&);
    WTF_EXPORT_PRIVATE static bool decode(Decoder&, String&);
};

template<> struct Coder<SHA1::Digest> {
    WTF_EXPORT_PRIVATE static void encode(Encoder&, const SHA1::Digest&);
    WTF_EXPORT_PRIVATE static bool decode(Decoder&, SHA1::Digest&);
};

}
}
