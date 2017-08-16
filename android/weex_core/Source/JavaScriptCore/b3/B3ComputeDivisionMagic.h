/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 * This contains code taken from LLVM's APInt class. That code implements finding the magic
 * numbers for strength-reducing division. The LLVM code on which this code is based was
 * implemented using "Hacker's Delight", Henry S. Warren, Jr., chapter 10.
 *
 * ==============================================================================
 * LLVM Release License
 * ==============================================================================
 * University of Illinois/NCSA
 * Open Source License
 * 
 * Copyright (c) 2003-2014 University of Illinois at Urbana-Champaign.
 * All rights reserved.
 * 
 * Developed by:
 * 
 *     LLVM Team
 * 
 *     University of Illinois at Urbana-Champaign
 * 
 *     http://llvm.org
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal with
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimers.
 * 
 *     * Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimers in the
 *       documentation and/or other materials provided with the distribution.
 * 
 *     * Neither the names of the LLVM Team, University of Illinois at
 *       Urbana-Champaign, nor the names of its contributors may be used to
 *       endorse or promote products derived from this Software without specific
 *       prior written permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
 * SOFTWARE.
 */

#pragma once

#if ENABLE(B3_JIT)

namespace JSC { namespace B3 {

template<typename T>
struct DivisionMagic {
    T magicMultiplier;
    unsigned shift;
};

// This contains code taken from LLVM's APInt::magic(). It's modestly adapted to our style, but
// not completely, to make it easier to apply their changes in the future.
template<typename T>
DivisionMagic<T> computeDivisionMagic(T divisor)
{
    typedef typename std::make_unsigned<T>::type UnsignedT;
    UnsignedT d = divisor;
    unsigned p;
    UnsignedT ad, anc, delta, q1, r1, q2, r2, t;
    UnsignedT signedMin = static_cast<UnsignedT>(std::numeric_limits<T>::min());
    DivisionMagic<T> mag;
    unsigned bitWidth = sizeof(divisor) * 8;

    // This code doesn't like to think of signedness as a type. Instead it likes to think that
    // operations have signedness. This is how we generally do it in B3 as well. For this reason,
    // we cast all the operated values once to unsigned. And later, we convert it to signed.
    // Only `divisor` have signedness here.

    ad = divisor < 0 ? -divisor : divisor; // -(signed min value) < signed max value. So there is no loss.
    t = signedMin + (d >> (bitWidth - 1));
    anc = t - 1 - (t % ad);   // absolute value of nc
    p = bitWidth - 1;    // initialize p
    q1 = signedMin / anc;   // initialize q1 = 2p/abs(nc)
    r1 = signedMin - q1*anc;    // initialize r1 = rem(2p,abs(nc))
    q2 = signedMin / ad;    // initialize q2 = 2p/abs(d)
    r2 = signedMin - q2*ad;     // initialize r2 = rem(2p,abs(d))
    do {
        p = p + 1;
        q1 = q1 << 1;          // update q1 = 2p/abs(nc)
        r1 = r1 << 1;          // update r1 = rem(2p/abs(nc))
        if (r1 >= anc) {  // must be unsigned comparison
            q1 = q1 + 1;
            r1 = r1 - anc;
        }
        q2 = q2 << 1;          // update q2 = 2p/abs(d)
        r2 = r2 << 1;          // update r2 = rem(2p/abs(d))
        if (r2 >= ad) {   // must be unsigned comparison
            q2 = q2 + 1;
            r2 = r2 - ad;
        }
        delta = ad - r2;
    } while (q1 < delta || (q1 == delta && r1 == 0));

    mag.magicMultiplier = q2 + 1;
    if (divisor < 0)
        mag.magicMultiplier = -mag.magicMultiplier;   // resulting magic number
    mag.shift = p - bitWidth;          // resulting shift

    return mag;
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
