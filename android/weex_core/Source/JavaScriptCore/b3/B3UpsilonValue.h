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

#include "B3Value.h"

namespace JSC { namespace B3 {

class JS_EXPORT_PRIVATE UpsilonValue : public Value {
public:
    static bool accepts(Kind kind) { return kind == Upsilon; }

    ~UpsilonValue();

    Value* phi() const { return m_phi; }
    void setPhi(Value* phi)
    {
        ASSERT(child(0)->type() == phi->type());
        ASSERT(phi->opcode() == Phi);
        m_phi = phi;
    }

protected:
    void dumpMeta(CommaPrinter&, PrintStream&) const override;

    Value* cloneImpl() const override;

private:
    friend class Procedure;

    // Note that passing the Phi during construction is optional. A valid pattern is to first create
    // the Upsilons without the Phi, then create the Phi, then go back and tell the Upsilons about
    // the Phi. This allows you to emit code in its natural order.
    UpsilonValue(Origin origin, Value* value, Value* phi = nullptr)
        : Value(CheckedOpcode, Upsilon, Void, origin, value)
        , m_phi(phi)
    {
        if (phi)
            ASSERT(value->type() == phi->type());
    }

    Value* m_phi;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
