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
 */

#pragma once

#if ENABLE(WEBASSEMBLY)

namespace JSC {

namespace Wasm {

#define FOR_EACH_EXCEPTION(macro) \
    macro(OutOfBoundsMemoryAccess,  "Out of bounds memory access") \
    macro(OutOfBoundsCallIndirect, "Out of bounds call_indirect") \
    macro(NullTableEntry,  "call_indirect to a null table entry") \
    macro(BadSignature, "call_indirect to a signature that does not match") \
    macro(OutOfBoundsTrunc, "Out of bounds Trunc operation") \
    macro(Unreachable, "Unreachable code should not be executed") \
    macro(DivisionByZero, "Division by zero") \
    macro(IntegerOverflow, "Integer overflow")

enum class ExceptionType : uint32_t {
#define MAKE_ENUM(enumName, error) enumName,
    FOR_EACH_EXCEPTION(MAKE_ENUM)
#undef MAKE_ENUM
};

ALWAYS_INLINE const char* errorMessageForExceptionType(ExceptionType type)
{
    switch (type) {
#define SWITCH_CASE(enumName, error) \
    case ExceptionType::enumName: return error;

    FOR_EACH_EXCEPTION(SWITCH_CASE)
#undef SWITCH_CASE
    }
    ASSERT_NOT_REACHED();
    return "";
}

} } // namespace JSC::Wasm

#endif // ENABLE(WEBASSEMBLY)
