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

#include "B3StackmapValue.h"

namespace JSC { namespace B3 {

class CheckValue : public StackmapValue {
public:
    static bool accepts(Kind kind)
    {
        switch (kind.opcode()) {
        case CheckAdd:
        case CheckSub:
        case CheckMul:
        case Check:
            return true;
        default:
            return false;
        }
    }

    ~CheckValue();

    void convertToAdd();

protected:
    Value* cloneImpl() const override;
    
private:
    friend class Procedure;

    // Use this form for CheckAdd, CheckSub, and CheckMul.
    JS_EXPORT_PRIVATE CheckValue(Kind, Origin, Value* left, Value* right);

    // Use this form for Check.
    JS_EXPORT_PRIVATE CheckValue(Kind, Origin, Value* predicate);
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
