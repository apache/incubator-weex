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
#ifndef WeakRandom_h
#define WeakRandom_h

#include <limits.h>
#include <wtf/CryptographicallyRandomNumber.h>
#include <wtf/StdLibExtras.h>

namespace WTF {

// The code used to generate random numbers are inlined manually in JIT code.
// So it needs to stay in sync with the JIT one.
class WeakRandom {
public:
    WeakRandom(unsigned seed = cryptographicallyRandomNumber())
    {
        setSeed(seed);
    }

    void setSeed(unsigned seed)
    {
        m_seed = seed;

        // A zero seed would cause an infinite series of zeroes.
        if (!seed)
            seed = 1;

        m_low = seed;
        m_high = seed;
        advance();
    }

    unsigned seed() const { return m_seed; }

    double get()
    {
        uint64_t value = advance() & ((1ULL << 53) - 1);
        return value * (1.0 / (1ULL << 53));
    }

    unsigned getUint32()
    {
        return static_cast<unsigned>(advance());
    }

    unsigned getUint32(unsigned limit)
    {
        if (limit <= 1)
            return 0;
        uint64_t cutoff = (static_cast<uint64_t>(std::numeric_limits<unsigned>::max()) + 1) / limit * limit;
        for (;;) {
            uint64_t value = getUint32();
            if (value >= cutoff)
                continue;
            return value % limit;
        }
    }

    static unsigned lowOffset() { return OBJECT_OFFSETOF(WeakRandom, m_low); }
    static unsigned highOffset() { return OBJECT_OFFSETOF(WeakRandom, m_high); }

private:
    uint64_t advance()
    {
        uint64_t x = m_low;
        uint64_t y = m_high;
        m_low = y;
        x ^= x << 23;
        x ^= x >> 17;
        x ^= y ^ (y >> 26);
        m_high = x;
        return x + y;
    }

    unsigned m_seed;
    uint64_t m_low;
    uint64_t m_high;
};

} // namespace WTF

using WTF::WeakRandom;

#endif // WeakRandom_h
