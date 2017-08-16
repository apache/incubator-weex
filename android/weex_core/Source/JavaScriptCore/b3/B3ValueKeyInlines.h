/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "B3Procedure.h"
#include "B3Value.h"
#include "B3ValueKey.h"

namespace JSC { namespace B3 {

inline ValueKey::ValueKey(Kind kind, Type type, Value* child)
    : m_kind(kind)
    , m_type(type)
{
    u.indices[0] = child->index();
}

inline ValueKey::ValueKey(Kind kind, Type type, Value* left, Value* right)
    : m_kind(kind)
    , m_type(type)
{
    u.indices[0] = left->index();
    u.indices[1] = right->index();
}

inline ValueKey::ValueKey(Kind kind, Type type, Value* a, Value* b, Value* c)
    : m_kind(kind)
    , m_type(type)
{
    u.indices[0] = a->index();
    u.indices[1] = b->index();
    u.indices[2] = c->index();
}

inline Value* ValueKey::child(Procedure& proc, unsigned index) const
{
    return proc.values()[index];
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
