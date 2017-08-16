/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
#include "B3Width.h"

#if ENABLE(B3_JIT)

#include <wtf/PrintStream.h>

namespace JSC { namespace B3 {

Type bestType(Bank bank, Width width)
{
    switch (width) {
    case Width8:
    case Width16:
    case Width32:
        switch (bank) {
        case GP:
            return Int32;
        case FP:
            return Float;
        }
        RELEASE_ASSERT_NOT_REACHED();
        return Void;
    case Width64:
        switch (bank) {
        case GP:
            return Int64;
        case FP:
            return Double;
        }
        RELEASE_ASSERT_NOT_REACHED();
        return Void;
    }
    RELEASE_ASSERT_NOT_REACHED();
    return Void;
}

} } // namespace JSC::B3

namespace WTF {

void printInternal(PrintStream& out, JSC::B3::Width width)
{
    switch (width) {
    case JSC::B3::Width8:
        out.print("8");
        return;
    case JSC::B3::Width16:
        out.print("16");
        return;
    case JSC::B3::Width32:
        out.print("32");
        return;
    case JSC::B3::Width64:
        out.print("64");
        return;
    }

    RELEASE_ASSERT_NOT_REACHED();
}

} // namespace WTF

#endif // ENABLE(B3_JIT)

