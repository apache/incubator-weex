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

#include "B3Const32Value.h"
#include "B3Const64Value.h"

namespace JSC { namespace B3 {

// Usually you want to use Const32Value or Const64Value directly. But this is useful for writing
// platform-agnostic code. Note that a ConstPtrValue will behave like either a Const32Value or
// Const64Value depending on platform.

#if USE(JSVALUE64)
typedef Const64Value ConstPtrValueBase;
#else
typedef Const32Value ConstPtrValueBase;
#endif

class ConstPtrValue : public ConstPtrValueBase {
public:
    void* value() const
    {
        return bitwise_cast<void*>(ConstPtrValueBase::value());
    }

private:
    friend class Procedure;

    template<typename T>
    ConstPtrValue(Origin origin, T* pointer)
        : ConstPtrValueBase(origin, bitwise_cast<intptr_t>(pointer))
    {
    }
    template<typename T>
    ConstPtrValue(Origin origin, T pointer)
        : ConstPtrValueBase(origin, static_cast<intptr_t>(pointer))
    {
    }
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
