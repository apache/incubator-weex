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

#if ENABLE(WEBASSEMBLY)

#include "B3Compilation.h"
#include "B3Type.h"
#include "CodeLocation.h"
#include "Identifier.h"
#include "MacroAssemblerCodeRef.h"
#include "RegisterAtOffsetList.h"
#include "WasmMemoryInformation.h"
#include "WasmOps.h"
#include "WasmPageCount.h"
#include "WasmSignature.h"
#include <limits>
#include <memory>
#include <wtf/Optional.h>
#include <wtf/Vector.h>

namespace JSC {

class JSFunction;

namespace Wasm {

inline bool isValueType(Type type)
{
    switch (type) {
    case I32:
    case I64:
    case F32:
    case F64:
        return true;
    default:
        break;
    }
    return false;
}
    
enum class ExternalKind : uint8_t {
    // FIXME auto-generate this. https://bugs.webkit.org/show_bug.cgi?id=165231
    Function = 0,
    Table = 1,
    Memory = 2,
    Global = 3,
};

template<typename Int>
static bool isValidExternalKind(Int val)
{
    switch (val) {
    case static_cast<Int>(ExternalKind::Function):
    case static_cast<Int>(ExternalKind::Table):
    case static_cast<Int>(ExternalKind::Memory):
    case static_cast<Int>(ExternalKind::Global):
        return true;
    default:
        return false;
    }
}

static_assert(static_cast<int>(ExternalKind::Function) == 0, "Wasm needs Function to have the value 0");
static_assert(static_cast<int>(ExternalKind::Table)    == 1, "Wasm needs Table to have the value 1");
static_assert(static_cast<int>(ExternalKind::Memory)   == 2, "Wasm needs Memory to have the value 2");
static_assert(static_cast<int>(ExternalKind::Global)   == 3, "Wasm needs Global to have the value 3");

static inline const char* makeString(ExternalKind kind)
{
    switch (kind) {
    case ExternalKind::Function: return "Function";
    case ExternalKind::Table: return "Table";
    case ExternalKind::Memory: return "Memory";
    case ExternalKind::Global: return "Global";
    }
    RELEASE_ASSERT_NOT_REACHED();
    return "?";
}

struct Import {
    Identifier module;
    Identifier field;
    ExternalKind kind;
    unsigned kindIndex; // Index in the vector of the corresponding kind.
};

struct Export {
    Identifier field;
    ExternalKind kind;
    unsigned kindIndex; // Index in the vector of the corresponding kind.
};

struct Global {
    enum Mutability : uint8_t {
        // FIXME auto-generate this. https://bugs.webkit.org/show_bug.cgi?id=165231
        Mutable = 1,
        Immutable = 0
    };

    enum InitializationType {
        IsImport,
        FromGlobalImport,
        FromExpression
    };

    Mutability mutability;
    Type type;
    InitializationType initializationType { IsImport };
    uint64_t initialBitsOrImportNumber { 0 };
};

struct FunctionLocationInBinary {
    size_t start;
    size_t end;
};

class I32InitExpr {
    enum Type : uint8_t {
        Global,
        Const
    };

    I32InitExpr(Type type, uint32_t bits)
        : m_bits(bits)
        , m_type(type)
    { }

public:
    I32InitExpr() = delete;

    static I32InitExpr globalImport(uint32_t globalImportNumber) { return I32InitExpr(Global, globalImportNumber); }
    static I32InitExpr constValue(uint32_t constValue) { return I32InitExpr(Const, constValue); }

    bool isConst() const { return m_type == Const; }
    bool isGlobalImport() const { return m_type == Global; }
    uint32_t constValue() const
    {
        RELEASE_ASSERT(isConst());
        return m_bits;
    }
    uint32_t globalImportIndex() const
    {
        RELEASE_ASSERT(isGlobalImport());
        return m_bits;
    }

private:
    uint32_t m_bits;
    Type m_type;
};

struct Segment {
    uint32_t sizeInBytes;
    I32InitExpr offset;
    // Bytes are allocated at the end.
    uint8_t& byte(uint32_t pos)
    {
        ASSERT(pos < sizeInBytes);
        return *reinterpret_cast<uint8_t*>(reinterpret_cast<char*>(this) + sizeof(Segment) + pos);
    }
    static Segment* create(I32InitExpr, uint32_t);
    static void destroy(Segment*);
    typedef std::unique_ptr<Segment, decltype(&Segment::destroy)> Ptr;
    static Ptr adoptPtr(Segment*);
};

struct Element {
    Element(I32InitExpr offset)
        : offset(offset)
    { }

    I32InitExpr offset;
    Vector<uint32_t> functionIndices;
};

class TableInformation {
public:
    TableInformation()
    {
        ASSERT(!*this);
    }

    TableInformation(uint32_t initial, std::optional<uint32_t> maximum, bool isImport)
        : m_initial(initial)
        , m_maximum(maximum)
        , m_isImport(isImport)
        , m_isValid(true)
    {
        ASSERT(*this);
    }

    explicit operator bool() const { return m_isValid; }
    bool isImport() const { return m_isImport; }
    uint32_t initial() const { return m_initial; }
    std::optional<uint32_t> maximum() const { return m_maximum; }

private:
    uint32_t m_initial;
    std::optional<uint32_t> m_maximum;
    bool m_isImport { false };
    bool m_isValid { false };
};
    
struct CustomSection {
    String name;
    Vector<uint8_t> payload;
};

struct ModuleInformation {
    Vector<Import> imports;
    Vector<SignatureIndex> importFunctionSignatureIndices;
    Vector<SignatureIndex> internalFunctionSignatureIndices;

    MemoryInformation memory;

    Vector<Export> exports;
    std::optional<uint32_t> startFunctionIndexSpace;
    Vector<Segment::Ptr> data;
    Vector<Element> elements;
    TableInformation tableInformation;
    Vector<Global> globals;
    unsigned firstInternalGlobal { 0 };
    Vector<CustomSection> customSections;

    size_t functionIndexSpaceSize() const { return importFunctionSignatureIndices.size() + internalFunctionSignatureIndices.size(); }
    bool isImportedFunctionFromFunctionIndexSpace(size_t functionIndex) const
    {
        ASSERT(functionIndex < functionIndexSpaceSize());
        return functionIndex < importFunctionSignatureIndices.size();
    }
    SignatureIndex signatureIndexFromFunctionIndexSpace(size_t functionIndex) const
    {
        return isImportedFunctionFromFunctionIndexSpace(functionIndex)
            ? importFunctionSignatureIndices[functionIndex]
            : internalFunctionSignatureIndices[functionIndex - importFunctionSignatureIndices.size()];
    }

    uint32_t importFunctionCount() const { return importFunctionSignatureIndices.size(); }

    ~ModuleInformation();
};

struct UnlinkedWasmToWasmCall {
    CodeLocationCall callLocation;
    size_t functionIndex;
    enum class Target : uint8_t {
        ToJs,
        ToWasm,
    } target;
};

struct Entrypoint {
    std::unique_ptr<B3::Compilation> compilation;
    RegisterAtOffsetList calleeSaveRegisters;
};

struct WasmInternalFunction {
    CodeLocationDataLabelPtr wasmCalleeMoveLocation;
    CodeLocationDataLabelPtr jsToWasmCalleeMoveLocation;

    Entrypoint wasmEntrypoint;
    Entrypoint jsToWasmEntrypoint;
};

struct WasmExitStubs {
    MacroAssemblerCodeRef wasmToJs;
    MacroAssemblerCodeRef wasmToWasm;
};

// WebAssembly direct calls and call_indirect use indices into "function index space". This space starts with all imports, and then all internal functions.
// CallableFunction and FunctionIndexSpace are only meant as fast lookup tables for these opcodes, and do not own code.
struct CallableFunction {
    CallableFunction() = default;

    CallableFunction(SignatureIndex signatureIndex, void* code = nullptr)
        : signatureIndex(signatureIndex)
        , code(code)
    {
    }

    // FIXME pack the SignatureIndex and the code pointer into one 64-bit value. https://bugs.webkit.org/show_bug.cgi?id=165511
    SignatureIndex signatureIndex { Signature::invalidIndex };
    void* code { nullptr };
};
typedef Vector<CallableFunction> FunctionIndexSpace;

} } // namespace JSC::Wasm

#endif // ENABLE(WEBASSEMBLY)
