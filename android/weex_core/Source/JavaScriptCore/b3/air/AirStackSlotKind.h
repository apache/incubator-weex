/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
 */

#pragma once

#if ENABLE(B3_JIT)

namespace JSC { namespace B3 { namespace Air {

enum class StackSlotKind : uint8_t {
    // A locked stack slot is an area of stack requested by the client. It cannot be killed. The
    // client can get its FP offset and write to it from stack walking code, so we must assume
    // that reads and writes to a locked stack slot can be clobbered the same way as reads and
    // writes to any memory location.
    Locked,

    // A spill slot. These have fundamentally different behavior than a typical memory location.
    // They are lowered to from temporaries. This means for example that a 32-bit ZDef store to a
    // 8 byte stack slot will zero the top 4 bytes, even though a 32-bit ZDef store to any other
    // kind of memory location would do no such thing. UseAddr on a spill slot is not allowed, so
    // they never escape.
    Spill

    // FIXME: We should add a third mode, which means that the stack slot will be read asynchronously
    // as with Locked, but never written to asynchronously. Then, Air could optimize spilling and
    // filling by tracking whether the value had been stored to a read-only locked slot. If it had,
    // then we can refill from that slot.
    // https://bugs.webkit.org/show_bug.cgi?id=150587
};

} } } // namespace JSC::B3::Air

namespace WTF {

class PrintStream;

void printInternal(PrintStream&, JSC::B3::Air::StackSlotKind);

} // namespace WTF

#endif // ENABLE(B3_JIT)
