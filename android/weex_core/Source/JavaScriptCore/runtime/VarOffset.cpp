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

#include "config.h"
#include "VarOffset.h"

namespace JSC {

void VarOffset::dump(PrintStream& out) const
{
    switch (m_kind) {
    case VarKind::Invalid:
        out.print("invalid");
        return;
    case VarKind::Scope:
        out.print(scopeOffset());
        return;
    case VarKind::Stack:
        out.print(stackOffset());
        return;
    case VarKind::DirectArgument:
        out.print(capturedArgumentsOffset());
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace JSC

namespace WTF {

using namespace JSC;

void printInternal(PrintStream& out, VarKind varKind)
{
    switch (varKind) {
    case VarKind::Invalid:
        out.print("Invalid");
        return;
    case VarKind::Scope:
        out.print("Scope");
        return;
    case VarKind::Stack:
        out.print("Stack");
        return;
    case VarKind::DirectArgument:
        out.print("DirectArgument");
        return;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

