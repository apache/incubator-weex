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

#if COMPILER(GCC) && ASSERT_DISABLED
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif // COMPILER(GCC) && ASSERT_DISABLED

namespace JSC { namespace Wasm {

#define FOR_EACH_WASM_SECTION(macro) \
    macro(Type,     1, "Function signature declarations") \
    macro(Import,   2, "Import declarations") \
    macro(Function, 3, "Function declarations") \
    macro(Table,    4, "Indirect function table and other tables") \
    macro(Memory,   5, "Memory attributes") \
    macro(Global,   6, "Global declarations") \
    macro(Export,   7, "Exports") \
    macro(Start,    8, "Start function declaration") \
    macro(Element,  9, "Elements section") \
    macro(Code,    10, "Function bodies (code)") \
    macro(Data,    11, "Data segments")

enum class Section : uint8_t {
#define DEFINE_WASM_SECTION_ENUM(NAME, ID, DESCRIPTION) NAME = ID,
    FOR_EACH_WASM_SECTION(DEFINE_WASM_SECTION_ENUM)
#undef DEFINE_WASM_SECTION_ENUM
    Custom
};

template<typename Int>
static inline bool isValidSection(Int section)
{
    switch (section) {
#define VALIDATE_SECTION(NAME, ID, DESCRIPTION) case static_cast<Int>(Section::NAME): return true;
        FOR_EACH_WASM_SECTION(VALIDATE_SECTION)
#undef VALIDATE_SECTION
    default:
        return false;
    }
}

static inline bool validateOrder(Section previous, Section next)
{
    if (previous == Section::Custom)
        return true;
    return static_cast<uint8_t>(previous) < static_cast<uint8_t>(next);
}

static inline const char* makeString(Section section)
{
    switch (section) {
    case Section::Custom:
        return "Custom";
#define STRINGIFY_SECTION_NAME(NAME, ID, DESCRIPTION) case Section::NAME: return #NAME;
        FOR_EACH_WASM_SECTION(STRINGIFY_SECTION_NAME)
#undef STRINGIFY_SECTION_NAME
    }
}

} } // namespace JSC::Wasm

#if COMPILER(GCC) && ASSERT_DISABLED
#pragma GCC diagnostic pop
#endif // COMPILER(GCC) && ASSERT_DISABLED

#endif // ENABLE(WEBASSEMBLY)
