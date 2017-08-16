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

#include "config.h"
#include "WasmModuleParser.h"

#if ENABLE(WEBASSEMBLY)

#include "IdentifierInlines.h"
#include "JSWebAssemblyTable.h"
#include "WasmFormat.h"
#include "WasmMemoryInformation.h"
#include "WasmOps.h"
#include "WasmSections.h"

#include <sys/mman.h>

namespace JSC { namespace Wasm {

ALWAYS_INLINE I32InitExpr makeI32InitExpr(uint8_t opcode, uint32_t bits)
{
    RELEASE_ASSERT(opcode == I32Const || opcode == GetGlobal);
    if (opcode == I32Const)
        return I32InitExpr::constValue(bits);
    return I32InitExpr::globalImport(bits);
}

auto ModuleParser::parse() -> Result
{
    m_result.module = std::make_unique<ModuleInformation>();
    const size_t minSize = 8;
    uint32_t versionNumber;

    WASM_PARSER_FAIL_IF(length() < minSize, "expected a module of at least ", minSize, " bytes");
    WASM_PARSER_FAIL_IF(!consumeCharacter(0) || !consumeString("asm"), "modules doesn't start with '\\0asm'");
    WASM_PARSER_FAIL_IF(!parseUInt32(versionNumber), "can't parse version number");
    if (versionNumber != 0xD) // FIXME Stop supporting version 0xD temporarily. https://bugs.webkit.org/show_bug.cgi?id=168788
        WASM_PARSER_FAIL_IF(versionNumber != expectedVersionNumber, "unexpected version number ", versionNumber, " expected ", expectedVersionNumber);

    Section previousSection = Section::Custom;
    while (m_offset < length()) {
        uint8_t sectionByte;

        WASM_PARSER_FAIL_IF(!parseUInt7(sectionByte), "can't get section byte");

        Section section = Section::Custom;
        if (sectionByte) {
            if (isValidSection(sectionByte))
                section = static_cast<Section>(sectionByte);
        }

        uint32_t sectionLength;
        WASM_PARSER_FAIL_IF(!validateOrder(previousSection, section), "invalid section order, ", previousSection, " followed by ", section);
        WASM_PARSER_FAIL_IF(!parseVarUInt32(sectionLength), "can't get ", section, " section's length");
        WASM_PARSER_FAIL_IF(sectionLength > length() - m_offset, section, "section of size ", sectionLength, " would overflow Module's size");

        auto end = m_offset + sectionLength;

        switch (section) {
#define WASM_SECTION_PARSE(NAME, ID, DESCRIPTION)                   \
        case Section::NAME: {                                       \
            WASM_FAIL_IF_HELPER_FAILS(parse ## NAME());             \
            break;                                                  \
        }
        FOR_EACH_WASM_SECTION(WASM_SECTION_PARSE)
#undef WASM_SECTION_PARSE

        case Section::Custom: {
            WASM_FAIL_IF_HELPER_FAILS(parseCustom(sectionLength));
            break;
        }
        }

        WASM_PARSER_FAIL_IF(end != m_offset, "parsing ended before the end of ", section, " section");

        previousSection = section;
    }

    return WTFMove(m_result);
}

auto ModuleParser::parseType() -> PartialResult
{
    uint32_t count;

    WASM_PARSER_FAIL_IF(!parseVarUInt32(count), "can't get Type section's count");
    WASM_PARSER_FAIL_IF(count == std::numeric_limits<uint32_t>::max(), "Type section's count is too big ", count);
    WASM_PARSER_FAIL_IF(!m_result.moduleSignatureIndicesToUniquedSignatureIndices.tryReserveCapacity(count), "can't allocate enough memory for Type section's ", count, " entries");

    for (uint32_t i = 0; i < count; ++i) {
        int8_t type;
        uint32_t argumentCount;
        Vector<Type> argumentTypes;

        WASM_PARSER_FAIL_IF(!parseInt7(type), "can't get ", i, "th Type's type");
        WASM_PARSER_FAIL_IF(type != Func, i, "th Type is non-Func ", type);
        WASM_PARSER_FAIL_IF(!parseVarUInt32(argumentCount), "can't get ", i, "th Type's argument count");
        WASM_PARSER_FAIL_IF(argumentCount == std::numeric_limits<uint32_t>::max(), i, "th argument count is too big ", argumentCount);
        std::unique_ptr<Signature, void (*)(Signature*)> signature(Signature::create(argumentCount), &Signature::destroy);
        WASM_PARSER_FAIL_IF(!signature, "can't allocate enough memory for Type section's ", i, "th signature");

        for (unsigned i = 0; i < argumentCount; ++i) {
            Type argumentType;
            WASM_PARSER_FAIL_IF(!parseResultType(argumentType), "can't get ", i, "th argument Type");
            signature->argument(i) = argumentType;
        }

        uint8_t returnCount;
        WASM_PARSER_FAIL_IF(!parseVarUInt1(returnCount), "can't get ", i, "th Type's return count");
        Type returnType;
        if (returnCount) {
            Type value;
            WASM_PARSER_FAIL_IF(!parseValueType(value), "can't get ", i, "th Type's return value");
            returnType = static_cast<Type>(value);
        } else
            returnType = Type::Void;
        signature->returnType() = returnType;

        SignatureIndex signatureIndex = SignatureInformation::adopt(m_vm, signature.release());
        m_result.moduleSignatureIndicesToUniquedSignatureIndices.uncheckedAppend(signatureIndex);
    }
    return { };
}

auto ModuleParser::parseImport() -> PartialResult
{
    uint32_t importCount;
    WASM_PARSER_FAIL_IF(!parseVarUInt32(importCount), "can't get Import section's count");
    WASM_PARSER_FAIL_IF(importCount == std::numeric_limits<uint32_t>::max(), "Import section's count is too big ", importCount);
    WASM_PARSER_FAIL_IF(!m_result.module->globals.tryReserveCapacity(importCount), "can't allocate enough memory for ", importCount, " globals"); // FIXME this over-allocates when we fix the FIXMEs below.
    WASM_PARSER_FAIL_IF(!m_result.module->imports.tryReserveCapacity(importCount), "can't allocate enough memory for ", importCount, " imports"); // FIXME this over-allocates when we fix the FIXMEs below.
    WASM_PARSER_FAIL_IF(!m_result.module->importFunctionSignatureIndices.tryReserveCapacity(importCount), "can't allocate enough memory for ", importCount, " import function signatures"); // FIXME this over-allocates when we fix the FIXMEs below.

    for (uint32_t importNumber = 0; importNumber < importCount; ++importNumber) {
        Import imp;
        uint32_t moduleLen;
        uint32_t fieldLen;
        String moduleString;
        String fieldString;

        WASM_PARSER_FAIL_IF(!parseVarUInt32(moduleLen), "can't get ", importNumber, "th Import's module name length");
        WASM_PARSER_FAIL_IF(!consumeUTF8String(moduleString, moduleLen), "can't get ", importNumber, "th Import's module name of length ", moduleLen);
        imp.module = Identifier::fromString(m_vm, moduleString);

        WASM_PARSER_FAIL_IF(!parseVarUInt32(fieldLen), "can't get ", importNumber, "th Import's field name length in module '", moduleString, "'");
        WASM_PARSER_FAIL_IF(!consumeUTF8String(fieldString, fieldLen), "can't get ", importNumber, "th Import's field name of length ", moduleLen, " in module '", moduleString, "'");
        imp.field = Identifier::fromString(m_vm, fieldString);

        WASM_PARSER_FAIL_IF(!parseExternalKind(imp.kind), "can't get ", importNumber, "th Import's kind in module '", moduleString, "' field '", fieldString, "'");
        switch (imp.kind) {
        case ExternalKind::Function: {
            uint32_t functionSignatureIndex;
            WASM_PARSER_FAIL_IF(!parseVarUInt32(functionSignatureIndex), "can't get ", importNumber, "th Import's function signature in module '", moduleString, "' field '", fieldString, "'");
            WASM_PARSER_FAIL_IF(functionSignatureIndex >= m_result.moduleSignatureIndicesToUniquedSignatureIndices.size(), "invalid function signature for ", importNumber, "th Import, ", functionSignatureIndex, " is out of range of ", m_result.moduleSignatureIndicesToUniquedSignatureIndices.size(), " in module '", moduleString, "' field '", fieldString, "'");
            imp.kindIndex = m_result.module->importFunctionSignatureIndices.size();
            SignatureIndex signatureIndex = m_result.moduleSignatureIndicesToUniquedSignatureIndices[functionSignatureIndex];
            m_result.module->importFunctionSignatureIndices.uncheckedAppend(signatureIndex);
            break;
        }
        case ExternalKind::Table: {
            bool isImport = true;
            PartialResult result = parseTableHelper(isImport);
            if (UNLIKELY(!result))
                return result.getUnexpected();
            break;
        }
        case ExternalKind::Memory: {
            bool isImport = true;
            PartialResult result = parseMemoryHelper(isImport);
            if (UNLIKELY(!result))
                return result.getUnexpected();
            break;
        }
        case ExternalKind::Global: {
            Global global;
            WASM_FAIL_IF_HELPER_FAILS(parseGlobalType(global));
            WASM_PARSER_FAIL_IF(global.mutability == Global::Mutable, "Mutable Globals aren't supported");

            imp.kindIndex = m_result.module->globals.size();
            m_result.module->globals.uncheckedAppend(WTFMove(global));
            break;
        }
        }

        m_result.module->imports.uncheckedAppend(imp);
    }

    m_result.module->firstInternalGlobal = m_result.module->globals.size();
    return { };
}

auto ModuleParser::parseFunction() -> PartialResult
{
    uint32_t count;
    WASM_PARSER_FAIL_IF(!parseVarUInt32(count), "can't get Function section's count");
    WASM_PARSER_FAIL_IF(count == std::numeric_limits<uint32_t>::max(), "Function section's count is too big ", count);
    WASM_PARSER_FAIL_IF(!m_result.module->internalFunctionSignatureIndices.tryReserveCapacity(count), "can't allocate enough memory for ", count, " Function signatures");
    WASM_PARSER_FAIL_IF(!m_result.functionLocationInBinary.tryReserveCapacity(count), "can't allocate enough memory for ", count, "Function locations");

    for (uint32_t i = 0; i < count; ++i) {
        uint32_t typeNumber;
        WASM_PARSER_FAIL_IF(!parseVarUInt32(typeNumber), "can't get ", i, "th Function's type number");
        WASM_PARSER_FAIL_IF(typeNumber >= m_result.moduleSignatureIndicesToUniquedSignatureIndices.size(), i, "th Function type number is invalid ", typeNumber);

        SignatureIndex signatureIndex = m_result.moduleSignatureIndicesToUniquedSignatureIndices[typeNumber];
        // The Code section fixes up start and end.
        size_t start = 0;
        size_t end = 0;
        m_result.module->internalFunctionSignatureIndices.uncheckedAppend(signatureIndex);
        m_result.functionLocationInBinary.uncheckedAppend({ start, end });
    }

    return { };
}

auto ModuleParser::parseResizableLimits(uint32_t& initial, std::optional<uint32_t>& maximum) -> PartialResult
{
    ASSERT(!maximum);

    uint8_t flags;
    WASM_PARSER_FAIL_IF(!parseVarUInt1(flags), "can't parse resizable limits flags");
    WASM_PARSER_FAIL_IF(!parseVarUInt32(initial), "can't parse resizable limits initial page count");

    if (flags) {
        uint32_t maximumInt;
        WASM_PARSER_FAIL_IF(!parseVarUInt32(maximumInt), "can't parse resizable limits maximum page count");
        WASM_PARSER_FAIL_IF(initial > maximumInt, "resizable limits has a initial page count of ", initial, " which is greater than its maximum ", maximumInt);
        maximum = maximumInt;
    }

    return { };
}

auto ModuleParser::parseTableHelper(bool isImport) -> PartialResult
{
    WASM_PARSER_FAIL_IF(m_hasTable, "Table section cannot exist if an Import has a table");

    m_hasTable = true;

    int8_t type;
    WASM_PARSER_FAIL_IF(!parseInt7(type), "can't parse Table type");
    WASM_PARSER_FAIL_IF(type != Wasm::Anyfunc, "Table type should be anyfunc, got ", type);

    uint32_t initial;
    std::optional<uint32_t> maximum;
    PartialResult limits = parseResizableLimits(initial, maximum);
    if (UNLIKELY(!limits))
        return limits.getUnexpected();
    WASM_PARSER_FAIL_IF(!JSWebAssemblyTable::isValidSize(initial), "Table's initial page count of ", initial, " is invalid");

    ASSERT(!maximum || *maximum >= initial);

    m_result.module->tableInformation = TableInformation(initial, maximum, isImport);

    return { };
}

auto ModuleParser::parseTable() -> PartialResult
{
    uint32_t count;
    WASM_PARSER_FAIL_IF(!parseVarUInt32(count), "can't get Table's count");
    WASM_PARSER_FAIL_IF(count != 1, "Table count of ", count, " is invalid, only 1 is allowed for now");

    bool isImport = false;
    PartialResult result = parseTableHelper(isImport);
    if (UNLIKELY(!result))
        return result.getUnexpected();

    return { };
}

auto ModuleParser::parseMemoryHelper(bool isImport) -> PartialResult
{
    WASM_PARSER_FAIL_IF(!!m_result.module->memory, "Memory section cannot exist if an Import has a memory");

    PageCount initialPageCount;
    PageCount maximumPageCount;
    {
        uint32_t initial;
        std::optional<uint32_t> maximum;
        PartialResult limits = parseResizableLimits(initial, maximum);
        if (UNLIKELY(!limits))
            return limits.getUnexpected();
        ASSERT(!maximum || *maximum >= initial);
        WASM_PARSER_FAIL_IF(!PageCount::isValid(initial), "Memory's initial page count of ", initial, " is invalid");

        initialPageCount = PageCount(initial);

        if (maximum) {
            WASM_PARSER_FAIL_IF(!PageCount::isValid(*maximum), "Memory's maximum page count of ", *maximum, " is invalid");
            maximumPageCount = PageCount(*maximum);
        }
    }
    ASSERT(initialPageCount);
    ASSERT(!maximumPageCount || maximumPageCount >= initialPageCount);

    m_result.module->memory = MemoryInformation(*m_vm, initialPageCount, maximumPageCount, m_mode, isImport);
    return { };
}

auto ModuleParser::parseMemory() -> PartialResult
{
    uint8_t count;
    WASM_PARSER_FAIL_IF(!parseVarUInt1(count), "can't parse Memory section's count");

    if (!count)
        return { };

    WASM_PARSER_FAIL_IF(count != 1, "Memory section has more than one memory, WebAssembly currently only allows zero or one");

    bool isImport = false;
    return parseMemoryHelper(isImport);
}

auto ModuleParser::parseGlobal() -> PartialResult
{
    uint32_t globalCount;
    WASM_PARSER_FAIL_IF(!parseVarUInt32(globalCount), "can't get Global section's count");
    WASM_PARSER_FAIL_IF(!m_result.module->globals.tryReserveCapacity(globalCount + m_result.module->firstInternalGlobal), "can't allocate memory for ", globalCount + m_result.module->firstInternalGlobal, " globals");

    for (uint32_t globalIndex = 0; globalIndex < globalCount; ++globalIndex) {
        Global global;
        uint8_t initOpcode;

        WASM_FAIL_IF_HELPER_FAILS(parseGlobalType(global));
        Type typeForInitOpcode;
        WASM_FAIL_IF_HELPER_FAILS(parseInitExpr(initOpcode, global.initialBitsOrImportNumber, typeForInitOpcode));
        if (initOpcode == GetGlobal)
            global.initializationType = Global::FromGlobalImport;
        else
            global.initializationType = Global::FromExpression;
        WASM_PARSER_FAIL_IF(typeForInitOpcode != global.type, "Global init_expr opcode of type ", typeForInitOpcode, " doesn't match global's type ", global.type);

        m_result.module->globals.uncheckedAppend(WTFMove(global));
    }

    return { };
}

auto ModuleParser::parseExport() -> PartialResult
{
    uint32_t exportCount;
    WASM_PARSER_FAIL_IF(!parseVarUInt32(exportCount), "can't get Export section's count");
    WASM_PARSER_FAIL_IF(exportCount == std::numeric_limits<uint32_t>::max(), "Export section's count is too big ", exportCount);
    WASM_PARSER_FAIL_IF(!m_result.module->exports.tryReserveCapacity(exportCount), "can't allocate enough memory for ", exportCount, " exports");

    HashSet<String> exportNames;
    for (uint32_t exportNumber = 0; exportNumber < exportCount; ++exportNumber) {
        Export exp;
        uint32_t fieldLen;
        String fieldString;

        WASM_PARSER_FAIL_IF(!parseVarUInt32(fieldLen), "can't get ", exportNumber, "th Export's field name length");
        WASM_PARSER_FAIL_IF(!consumeUTF8String(fieldString, fieldLen), "can't get ", exportNumber, "th Export's field name of length ", fieldLen);
        WASM_PARSER_FAIL_IF(exportNames.contains(fieldString), "duplicate export: '", fieldString, "'");
        exportNames.add(fieldString);
        exp.field = Identifier::fromString(m_vm, fieldString);

        WASM_PARSER_FAIL_IF(!parseExternalKind(exp.kind), "can't get ", exportNumber, "th Export's kind, named '", fieldString, "'");
        WASM_PARSER_FAIL_IF(!parseVarUInt32(exp.kindIndex), "can't get ", exportNumber, "th Export's kind index, named '", fieldString, "'");
        switch (exp.kind) {
        case ExternalKind::Function: {
            WASM_PARSER_FAIL_IF(exp.kindIndex >= m_result.module->functionIndexSpaceSize(), exportNumber, "th Export has invalid function number ", exp.kindIndex, " it exceeds the function index space ", m_result.module->functionIndexSpaceSize(), ", named '", fieldString, "'");
            break;
        }
        case ExternalKind::Table: {
            WASM_PARSER_FAIL_IF(!m_hasTable, "can't export a non-existent Table");
            WASM_PARSER_FAIL_IF(exp.kindIndex, "can't export Table ", exp.kindIndex, " only zero-index Table is currently supported");
            break;
        }
        case ExternalKind::Memory: {
            WASM_PARSER_FAIL_IF(!m_result.module->memory, "can't export a non-existent Memory");
            WASM_PARSER_FAIL_IF(exp.kindIndex, "can't export Memory ", exp.kindIndex, " only one Table is currently supported");
            break;
        }
        case ExternalKind::Global: {
            WASM_PARSER_FAIL_IF(exp.kindIndex >= m_result.module->globals.size(), exportNumber, "th Export has invalid global number ", exp.kindIndex, " it exceeds the globals count ", m_result.module->globals.size(), ", named '", fieldString, "'");
            WASM_PARSER_FAIL_IF(m_result.module->globals[exp.kindIndex].mutability != Global::Immutable, exportNumber, "th Export isn't immutable, named '", fieldString, "'");
            break;
        }
        }

        m_result.module->exports.uncheckedAppend(exp);
    }

    return { };
}

auto ModuleParser::parseStart() -> PartialResult
{
    uint32_t startFunctionIndex;
    WASM_PARSER_FAIL_IF(!parseVarUInt32(startFunctionIndex), "can't get Start index");
    WASM_PARSER_FAIL_IF(startFunctionIndex >= m_result.module->functionIndexSpaceSize(), "Start index ", startFunctionIndex, " exceeds function index space ", m_result.module->functionIndexSpaceSize());
    SignatureIndex signatureIndex = m_result.module->signatureIndexFromFunctionIndexSpace(startFunctionIndex);
    const Signature* signature = SignatureInformation::get(m_vm, signatureIndex);
    WASM_PARSER_FAIL_IF(signature->argumentCount(), "Start function can't have arguments");
    WASM_PARSER_FAIL_IF(signature->returnType() != Void, "Start function can't return a value");
    m_result.module->startFunctionIndexSpace = startFunctionIndex;
    return { };
}

auto ModuleParser::parseElement() -> PartialResult
{
    WASM_PARSER_FAIL_IF(!m_hasTable, "Element section expects a Table to be present");

    uint32_t elementCount;
    WASM_PARSER_FAIL_IF(!parseVarUInt32(elementCount), "can't get Element section's count");
    WASM_PARSER_FAIL_IF(elementCount == std::numeric_limits<uint32_t>::max(), "Element section's count is too big ", elementCount);
    WASM_PARSER_FAIL_IF(!m_result.module->elements.tryReserveCapacity(elementCount), "can't allocate memory for ", elementCount, " Elements");
    for (unsigned elementNum = 0; elementNum < elementCount; ++elementNum) {
        uint32_t tableIndex;
        uint64_t initExprBits;
        uint8_t initOpcode;
        uint32_t indexCount;

        WASM_PARSER_FAIL_IF(!parseVarUInt32(tableIndex), "can't get ", elementNum, "th Element table index");
        WASM_PARSER_FAIL_IF(tableIndex, "Element section can only have one Table for now");
        Type initExprType;
        WASM_FAIL_IF_HELPER_FAILS(parseInitExpr(initOpcode, initExprBits, initExprType));
        WASM_PARSER_FAIL_IF(initExprType != I32, "Element init_expr must produce an i32");
        WASM_PARSER_FAIL_IF(!parseVarUInt32(indexCount), "can't get ", elementNum, "th index count for Element section");
        WASM_PARSER_FAIL_IF(indexCount == std::numeric_limits<uint32_t>::max(), "Element section's ", elementNum, "th index count is too big ", indexCount);

        ASSERT(!!m_result.module->tableInformation);

        Element element(makeI32InitExpr(initOpcode, initExprBits));
        WASM_PARSER_FAIL_IF(!element.functionIndices.tryReserveCapacity(indexCount), "can't allocate memory for ", indexCount, " Element indices");

        for (unsigned index = 0; index < indexCount; ++index) {
            uint32_t functionIndex;
            WASM_PARSER_FAIL_IF(!parseVarUInt32(functionIndex), "can't get Element section's ", elementNum, "th element's ", index, "th index");
            WASM_PARSER_FAIL_IF(functionIndex >= m_result.module->functionIndexSpaceSize(), "Element section's ", elementNum, "th element's ", index, "th index is ", functionIndex, " which exceeds the function index space size of ", m_result.module->functionIndexSpaceSize());

            element.functionIndices.uncheckedAppend(functionIndex);
        }

        m_result.module->elements.uncheckedAppend(WTFMove(element));
    }

    return { };
}

auto ModuleParser::parseCode() -> PartialResult
{
    uint32_t count;
    WASM_PARSER_FAIL_IF(!parseVarUInt32(count), "can't get Code section's count");
    WASM_PARSER_FAIL_IF(count == std::numeric_limits<uint32_t>::max(), "Code section's count is too big ", count);
    WASM_PARSER_FAIL_IF(count != m_result.functionLocationInBinary.size(), "Code section count ", count, " exceeds the declared number of functions ", m_result.functionLocationInBinary.size());

    for (uint32_t i = 0; i < count; ++i) {
        uint32_t functionSize;
        WASM_PARSER_FAIL_IF(!parseVarUInt32(functionSize), "can't get ", i, "th Code function's size");
        WASM_PARSER_FAIL_IF(functionSize > length(), "Code function's size ", functionSize, " exceeds the module's size ", length());
        WASM_PARSER_FAIL_IF(functionSize > length() - m_offset, "Code function's size ", functionSize, " exceeds the module's remaining size", length() - m_offset);

        m_result.functionLocationInBinary[i].start = m_offset;
        m_result.functionLocationInBinary[i].end = m_offset + functionSize;
        m_offset = m_result.functionLocationInBinary[i].end;
    }

    return { };
}

auto ModuleParser::parseInitExpr(uint8_t& opcode, uint64_t& bitsOrImportNumber, Type& resultType) -> PartialResult
{
    WASM_PARSER_FAIL_IF(!parseUInt8(opcode), "can't get init_expr's opcode");

    switch (opcode) {
    case I32Const: {
        int32_t constant;
        WASM_PARSER_FAIL_IF(!parseVarInt32(constant), "can't get constant value for init_expr's i32.const");
        bitsOrImportNumber = static_cast<uint64_t>(constant);
        resultType = I32;
        break;
    }

    case I64Const: {
        int64_t constant;
        WASM_PARSER_FAIL_IF(!parseVarInt64(constant), "can't get constant value for init_expr's i64.const");
        bitsOrImportNumber = constant;
        resultType = I64;
        break;
    }

    case F32Const: {
        uint32_t constant;
        WASM_PARSER_FAIL_IF(!parseUInt32(constant), "can't get constant value for init_expr's f32.const");
        bitsOrImportNumber = constant;
        resultType = F32;
        break;
    }

    case F64Const: {
        uint64_t constant;
        WASM_PARSER_FAIL_IF(!parseUInt64(constant), "can't get constant value for init_expr's f64.const");
        bitsOrImportNumber = constant;
        resultType = F64;
        break;
    }

    case GetGlobal: {
        uint32_t index;
        WASM_PARSER_FAIL_IF(!parseVarUInt32(index), "can't get get_global's index");

        WASM_PARSER_FAIL_IF(index >= m_result.module->globals.size(), "get_global's index ", index, " exceeds the number of globals ", m_result.module->globals.size());
        WASM_PARSER_FAIL_IF(index >= m_result.module->firstInternalGlobal, "get_global import kind index ", index, " exceeds the first internal global ", m_result.module->firstInternalGlobal);

        ASSERT(m_result.module->globals[index].mutability == Global::Immutable);
        resultType = m_result.module->globals[index].type;
        bitsOrImportNumber = index;
        break;
    }

    default:
        WASM_PARSER_FAIL_IF(false, "unknown init_expr opcode ", opcode);
    }

    uint8_t endOpcode;
    WASM_PARSER_FAIL_IF(!parseUInt8(endOpcode), "can't get init_expr's end opcode");
    WASM_PARSER_FAIL_IF(endOpcode != OpType::End, "init_expr should end with end, ended with ", endOpcode);

    return { };
}

auto ModuleParser::parseGlobalType(Global& global) -> PartialResult
{
    uint8_t mutability;
    WASM_PARSER_FAIL_IF(!parseValueType(global.type), "can't get Global's value type");
    WASM_PARSER_FAIL_IF(!parseVarUInt1(mutability), "can't get Global type's mutability");
    global.mutability = static_cast<Global::Mutability>(mutability);
    return { };
}

auto ModuleParser::parseData() -> PartialResult
{
    uint32_t segmentCount;
    WASM_PARSER_FAIL_IF(!m_result.module->memory, "Data section cannot exist without a Memory section or Import");
    WASM_PARSER_FAIL_IF(!parseVarUInt32(segmentCount), "can't get Data section's count");
    WASM_PARSER_FAIL_IF(segmentCount == std::numeric_limits<uint32_t>::max(), "Data section's count is too big ", segmentCount);
    WASM_PARSER_FAIL_IF(!m_result.module->data.tryReserveCapacity(segmentCount), "can't allocate enough memory for Data section's ", segmentCount, " segments");

    for (uint32_t segmentNumber = 0; segmentNumber < segmentCount; ++segmentNumber) {
        uint32_t index;
        uint64_t initExprBits;
        uint8_t initOpcode;
        uint32_t dataByteLength;

        WASM_PARSER_FAIL_IF(!parseVarUInt32(index), "can't get ", segmentNumber, "th Data segment's index");
        WASM_PARSER_FAIL_IF(index, segmentNumber, "th Data segment has non-zero index ", index);
        Type initExprType;
        WASM_FAIL_IF_HELPER_FAILS(parseInitExpr(initOpcode, initExprBits, initExprType));
        WASM_PARSER_FAIL_IF(initExprType != I32, segmentNumber, "th Data segment's init_expr must produce an i32");
        WASM_PARSER_FAIL_IF(!parseVarUInt32(dataByteLength), "can't get ", segmentNumber, "th Data segment's data byte length");
        WASM_PARSER_FAIL_IF(dataByteLength == std::numeric_limits<uint32_t>::max(), segmentNumber, "th Data segment's data byte length is too big ", dataByteLength);

        Segment* segment = Segment::create(makeI32InitExpr(initOpcode, initExprBits), dataByteLength);
        WASM_PARSER_FAIL_IF(!segment, "can't allocate enough memory for ", segmentNumber, "th Data segment of size ", dataByteLength);
        m_result.module->data.uncheckedAppend(Segment::adoptPtr(segment));
        for (uint32_t dataByte = 0; dataByte < dataByteLength; ++dataByte) {
            uint8_t byte;
            WASM_PARSER_FAIL_IF(!parseUInt8(byte), "can't get ", dataByte, "th data byte from ", segmentNumber, "th Data segment");
            segment->byte(dataByte) = byte;
        }
    }
    return { };
}
    
auto ModuleParser::parseCustom(uint32_t sectionLength) -> PartialResult
{
    const uint32_t customSectionStartOffset = m_offset;

    CustomSection section;
    uint32_t customSectionNumber = m_result.module->customSections.size() + 1;
    uint32_t nameLen;
    WASM_PARSER_FAIL_IF(!m_result.module->customSections.tryReserveCapacity(customSectionNumber), "can't allocate enough memory for ", customSectionNumber, "th custom section");
    WASM_PARSER_FAIL_IF(!parseVarUInt32(nameLen), "can't get ", customSectionNumber, "th custom section's name length");
    WASM_PARSER_FAIL_IF(!consumeUTF8String(section.name, nameLen), "nameLen get ", customSectionNumber, "th custom section's name of length ", nameLen);

    uint32_t payloadBytes = sectionLength - (m_offset - customSectionStartOffset);
    WASM_PARSER_FAIL_IF(!section.payload.tryReserveCapacity(payloadBytes), "can't allocate enough memory for ", customSectionNumber, "th custom section's ", payloadBytes, " bytes");
    for (uint32_t byteNumber = 0; byteNumber < payloadBytes; ++byteNumber) {
        uint8_t byte;
        WASM_PARSER_FAIL_IF(!parseUInt8(byte), "can't get ", byteNumber, "th data byte from ", customSectionNumber, "th custom section");
        section.payload.uncheckedAppend(byte);
    }
    
    m_result.module->customSections.uncheckedAppend(WTFMove(section));

    return { };
}

} } // namespace JSC::Wasm

#endif // ENABLE(WEBASSEMBLY)
