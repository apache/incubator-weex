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

namespace JSC {

ALWAYS_INLINE bool isIOS()
{
#if PLATFORM(IOS)
    return true;
#else
    return false;
#endif
}

ALWAYS_INLINE bool isInt9(int32_t value)
{
    return value == ((value << 23) >> 23);
}

template<typename Type>
ALWAYS_INLINE bool isUInt12(Type value)
{
    return !(value & ~static_cast<Type>(0xfff));
}

template<int datasize>
ALWAYS_INLINE bool isValidScaledUImm12(int32_t offset)
{
    int32_t maxPImm = 4095 * (datasize / 8);
    if (offset < 0)
        return false;
    if (offset > maxPImm)
        return false;
    if (offset & ((datasize / 8) - 1))
        return false;
    return true;
}

ALWAYS_INLINE bool isValidSignedImm9(int32_t value)
{
    return isInt9(value);
}

class ARM64LogicalImmediate {
public:
    static ARM64LogicalImmediate create32(uint32_t value)
    {
        // Check for 0, -1 - these cannot be encoded.
        if (!value || !~value)
            return InvalidLogicalImmediate;

        // First look for a 32-bit pattern, then for repeating 16-bit
        // patterns, 8-bit, 4-bit, and finally 2-bit.

        unsigned hsb, lsb;
        bool inverted;
        if (findBitRange<32>(value, hsb, lsb, inverted))
            return encodeLogicalImmediate<32>(hsb, lsb, inverted);

        if ((value & 0xffff) != (value >> 16))
            return InvalidLogicalImmediate;
        value &= 0xffff;

        if (findBitRange<16>(value, hsb, lsb, inverted))
            return encodeLogicalImmediate<16>(hsb, lsb, inverted);

        if ((value & 0xff) != (value >> 8))
            return InvalidLogicalImmediate;
        value &= 0xff;

        if (findBitRange<8>(value, hsb, lsb, inverted))
            return encodeLogicalImmediate<8>(hsb, lsb, inverted);

        if ((value & 0xf) != (value >> 4))
            return InvalidLogicalImmediate;
        value &= 0xf;

        if (findBitRange<4>(value, hsb, lsb, inverted))
            return encodeLogicalImmediate<4>(hsb, lsb, inverted);

        if ((value & 0x3) != (value >> 2))
            return InvalidLogicalImmediate;
        value &= 0x3;

        if (findBitRange<2>(value, hsb, lsb, inverted))
            return encodeLogicalImmediate<2>(hsb, lsb, inverted);

        return InvalidLogicalImmediate;
    }

    static ARM64LogicalImmediate create64(uint64_t value)
    {
        // Check for 0, -1 - these cannot be encoded.
        if (!value || !~value)
            return InvalidLogicalImmediate;

        // Look for a contiguous bit range.
        unsigned hsb, lsb;
        bool inverted;
        if (findBitRange<64>(value, hsb, lsb, inverted))
            return encodeLogicalImmediate<64>(hsb, lsb, inverted);

        // If the high & low 32 bits are equal, we can try for a 32-bit (or narrower) pattern.
        if (static_cast<uint32_t>(value) == static_cast<uint32_t>(value >> 32))
            return create32(static_cast<uint32_t>(value));
        return InvalidLogicalImmediate;
    }

    int value() const
    {
        ASSERT(isValid());
        return m_value;
    }

    bool isValid() const
    {
        return m_value != InvalidLogicalImmediate;
    }

    bool is64bit() const
    {
        return m_value & (1 << 12);
    }

private:
    ARM64LogicalImmediate(int value)
        : m_value(value)
    {
    }

    // Generate a mask with bits in the range hsb..0 set, for example:
    //   hsb:63 = 0xffffffffffffffff
    //   hsb:42 = 0x000007ffffffffff
    //   hsb: 0 = 0x0000000000000001
    static uint64_t mask(unsigned hsb)
    {
        ASSERT(hsb < 64);
        return 0xffffffffffffffffull >> (63 - hsb);
    }

    template<unsigned N>
    static void partialHSB(uint64_t& value, unsigned&result)
    {
        if (value & (0xffffffffffffffffull << N)) {
            result += N;
            value >>= N;
        }
    }

    // Find the bit number of the highest bit set in a non-zero value, for example:
    //   0x8080808080808080 = hsb:63
    //   0x0000000000000001 = hsb: 0
    //   0x000007ffffe00000 = hsb:42
    static unsigned highestSetBit(uint64_t value)
    {
        ASSERT(value);
        unsigned hsb = 0;
        partialHSB<32>(value, hsb);
        partialHSB<16>(value, hsb);
        partialHSB<8>(value, hsb);
        partialHSB<4>(value, hsb);
        partialHSB<2>(value, hsb);
        partialHSB<1>(value, hsb);
        return hsb;
    }

    // This function takes a value and a bit width, where value obeys the following constraints:
    //   * bits outside of the width of the value must be zero.
    //   * bits within the width of value must neither be all clear or all set.
    // The input is inspected to detect values that consist of either two or three contiguous
    // ranges of bits. The output range hsb..lsb will describe the second range of the value.
    // if the range is set, inverted will be false, and if the range is clear, inverted will
    // be true. For example (with width 8):
    //   00001111 = hsb:3, lsb:0, inverted:false
    //   11110000 = hsb:3, lsb:0, inverted:true
    //   00111100 = hsb:5, lsb:2, inverted:false
    //   11000011 = hsb:5, lsb:2, inverted:true
    template<unsigned width>
    static bool findBitRange(uint64_t value, unsigned& hsb, unsigned& lsb, bool& inverted)
    {
        ASSERT(value & mask(width - 1));
        ASSERT(value != mask(width - 1));
        ASSERT(!(value & ~mask(width - 1)));

        // Detect cases where the top bit is set; if so, flip all the bits & set invert.
        // This halves the number of patterns we need to look for.
        const uint64_t msb = 1ull << (width - 1);
        if ((inverted = (value & msb)))
            value ^= mask(width - 1);

        // Find the highest set bit in value, generate a corresponding mask & flip all
        // bits under it.
        hsb = highestSetBit(value);
        value ^= mask(hsb);
        if (!value) {
            // If this cleared the value, then the range hsb..0 was all set.
            lsb = 0;
            return true;
        }

        // Try making one more mask, and flipping the bits!
        lsb = highestSetBit(value);
        value ^= mask(lsb);
        if (!value) {
            // Success - but lsb actually points to the hsb of a third range - add one
            // to get to the lsb of the mid range.
            ++lsb;
            return true;
        }

        return false;
    }

    // Encodes the set of immN:immr:imms fields found in a logical immediate.
    template<unsigned width>
    static int encodeLogicalImmediate(unsigned hsb, unsigned lsb, bool inverted)
    {
        // Check width is a power of 2!
        ASSERT(!(width & (width -1)));
        ASSERT(width <= 64 && width >= 2);
        ASSERT(hsb >= lsb);
        ASSERT(hsb < width);

        int immN = 0;
        int imms = 0;
        int immr = 0;

        // For 64-bit values this is easy - just set immN to true, and imms just
        // contains the bit number of the highest set bit of the set range. For
        // values with narrower widths, these are encoded by a leading set of
        // one bits, followed by a zero bit, followed by the remaining set of bits
        // being the high bit of the range. For a 32-bit immediate there are no
        // leading one bits, just a zero followed by a five bit number. For a
        // 16-bit immediate there is one one bit, a zero bit, and then a four bit
        // bit-position, etc.
        if (width == 64)
            immN = 1;
        else
            imms = 63 & ~(width + width - 1);

        if (inverted) {
            // if width is 64 & hsb is 62, then we have a value something like:
            //   0x80000000ffffffff (in this case with lsb 32).
            // The ror should be by 1, imms (effectively set width minus 1) is
            // 32. Set width is full width minus cleared width.
            immr = (width - 1) - hsb;
            imms |= (width - ((hsb - lsb) + 1)) - 1;
        } else {
            // if width is 64 & hsb is 62, then we have a value something like:
            //   0x7fffffff00000000 (in this case with lsb 32).
            // The value is effectively rol'ed by lsb, which is equivalent to
            // a ror by width - lsb (or 0, in the case where lsb is 0). imms
            // is hsb - lsb.
            immr = (width - lsb) & (width - 1);
            imms |= hsb - lsb;
        }

        return immN << 12 | immr << 6 | imms;
    }

    static const int InvalidLogicalImmediate = -1;

    int m_value;
};

} // namespace JSC.
