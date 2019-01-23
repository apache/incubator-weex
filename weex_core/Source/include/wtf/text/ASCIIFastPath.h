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
#ifndef ASCIIFastPath_h
#define ASCIIFastPath_h

#include <stdint.h>
#include <unicode/utypes.h>
#include <wtf/StdLibExtras.h>
#include <wtf/text/LChar.h>

#if CPU(X86_SSE2)
#include <emmintrin.h>
#endif

namespace WTF {

template <uintptr_t mask>
inline bool isAlignedTo(const void* pointer)
{
    return !(reinterpret_cast<uintptr_t>(pointer) & mask);
}

// Assuming that a pointer is the size of a "machine word", then
// uintptr_t is an integer type that is also a machine word.
typedef uintptr_t MachineWord;
const uintptr_t machineWordAlignmentMask = sizeof(MachineWord) - 1;

inline bool isAlignedToMachineWord(const void* pointer)
{
    return isAlignedTo<machineWordAlignmentMask>(pointer);
}

template<typename T> inline T* alignToMachineWord(T* pointer)
{
    return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(pointer) & ~machineWordAlignmentMask);
}

template<size_t size, typename CharacterType> struct NonASCIIMask;
template<> struct NonASCIIMask<4, UChar> {
    static inline uint32_t value() { return 0xFF80FF80U; }
};
template<> struct NonASCIIMask<4, LChar> {
    static inline uint32_t value() { return 0x80808080U; }
};
template<> struct NonASCIIMask<8, UChar> {
    static inline uint64_t value() { return 0xFF80FF80FF80FF80ULL; }
};
template<> struct NonASCIIMask<8, LChar> {
    static inline uint64_t value() { return 0x8080808080808080ULL; }
};


template<typename CharacterType>
inline bool isAllASCII(MachineWord word)
{
    return !(word & NonASCIIMask<sizeof(MachineWord), CharacterType>::value());
}

// Note: This function assume the input is likely all ASCII, and
// does not leave early if it is not the case.
template<typename CharacterType>
inline bool charactersAreAllASCII(const CharacterType* characters, size_t length)
{
    MachineWord allCharBits = 0;
    const CharacterType* end = characters + length;

    // Prologue: align the input.
    while (!isAlignedToMachineWord(characters) && characters != end) {
        allCharBits |= *characters;
        ++characters;
    }

    // Compare the values of CPU word size.
    const CharacterType* wordEnd = alignToMachineWord(end);
    const size_t loopIncrement = sizeof(MachineWord) / sizeof(CharacterType);
    while (characters < wordEnd) {
        allCharBits |= *(reinterpret_cast_ptr<const MachineWord*>(characters));
        characters += loopIncrement;
    }

    // Process the remaining bytes.
    while (characters != end) {
        allCharBits |= *characters;
        ++characters;
    }

    MachineWord nonASCIIBitMask = NonASCIIMask<sizeof(MachineWord), CharacterType>::value();
    return !(allCharBits & nonASCIIBitMask);
}

inline void copyLCharsFromUCharSource(LChar* destination, const UChar* source, size_t length)
{
#if CPU(X86_SSE2)
    const uintptr_t memoryAccessSize = 16; // Memory accesses on 16 byte (128 bit) alignment
    const uintptr_t memoryAccessMask = memoryAccessSize - 1;

    size_t i = 0;
    for (;i < length && !isAlignedTo<memoryAccessMask>(&source[i]); ++i) {
        ASSERT(!(source[i] & 0xff00));
        destination[i] = static_cast<LChar>(source[i]);
    }

    const uintptr_t sourceLoadSize = 32; // Process 32 bytes (16 UChars) each iteration
    const size_t ucharsPerLoop = sourceLoadSize / sizeof(UChar);
    if (length > ucharsPerLoop) {
        const size_t endLength = length - ucharsPerLoop + 1;
        for (; i < endLength; i += ucharsPerLoop) {
#ifndef NDEBUG
            for (unsigned checkIndex = 0; checkIndex < ucharsPerLoop; ++checkIndex)
                ASSERT(!(source[i+checkIndex] & 0xff00));
#endif
            __m128i first8UChars = _mm_load_si128(reinterpret_cast<const __m128i*>(&source[i]));
            __m128i second8UChars = _mm_load_si128(reinterpret_cast<const __m128i*>(&source[i+8]));
            __m128i packedChars = _mm_packus_epi16(first8UChars, second8UChars);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(&destination[i]), packedChars);
        }
    }

    for (; i < length; ++i) {
        ASSERT(!(source[i] & 0xff00));
        destination[i] = static_cast<LChar>(source[i]);
    }
#elif COMPILER(GCC_OR_CLANG) && CPU(ARM64) && defined(NDEBUG)
    const LChar* const end = destination + length;
    const uintptr_t memoryAccessSize = 16;

    if (length >= memoryAccessSize) {
        const uintptr_t memoryAccessMask = memoryAccessSize - 1;

        // Vector interleaved unpack, we only store the lower 8 bits.
        const uintptr_t lengthLeft = end - destination;
        const LChar* const simdEnd = destination + (lengthLeft & ~memoryAccessMask);
        do {
            asm("ld2   { v0.16B, v1.16B }, [%[SOURCE]], #32\n\t"
                "st1   { v0.16B }, [%[DESTINATION]], #16\n\t"
                : [SOURCE]"+r" (source), [DESTINATION]"+r" (destination)
                :
                : "memory", "v0", "v1");
        } while (destination != simdEnd);
    }

    while (destination != end)
        *destination++ = static_cast<LChar>(*source++);
#elif COMPILER(GCC_OR_CLANG) && CPU(ARM_NEON) && !(CPU(BIG_ENDIAN) || CPU(MIDDLE_ENDIAN)) && defined(NDEBUG)
    const LChar* const end = destination + length;
    const uintptr_t memoryAccessSize = 8;

    if (length >= (2 * memoryAccessSize) - 1) {
        // Prefix: align dst on 64 bits.
        const uintptr_t memoryAccessMask = memoryAccessSize - 1;
        while (!isAlignedTo<memoryAccessMask>(destination))
            *destination++ = static_cast<LChar>(*source++);

        // Vector interleaved unpack, we only store the lower 8 bits.
        const uintptr_t lengthLeft = end - destination;
        const LChar* const simdEnd = end - (lengthLeft % memoryAccessSize);
        do {
            asm("vld2.8   { d0-d1 }, [%[SOURCE]] !\n\t"
                "vst1.8   { d0 }, [%[DESTINATION],:64] !\n\t"
                : [SOURCE]"+r" (source), [DESTINATION]"+r" (destination)
                :
                : "memory", "d0", "d1");
        } while (destination != simdEnd);
    }

    while (destination != end)
        *destination++ = static_cast<LChar>(*source++);
#else
    for (size_t i = 0; i < length; ++i) {
        ASSERT(!(source[i] & 0xff00));
        destination[i] = static_cast<LChar>(source[i]);
    }
#endif
}

} // namespace WTF

#endif // ASCIIFastPath_h
