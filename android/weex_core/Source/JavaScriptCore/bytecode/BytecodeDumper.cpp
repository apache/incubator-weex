/*
 * Copyright (C) 2017 Yusuke Suzuki <utatane.tea@gmail.com>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "BytecodeDumper.h"

#include "ArithProfile.h"
#include "CallLinkStatus.h"
#include "CodeBlock.h"
#include "Error.h"
#include "HeapInlines.h"
#include "InterpreterInlines.h"
#include "PolymorphicAccess.h"
#include "PutByIdFlags.h"
#include "StructureInlines.h"
#include "ToThisStatus.h"
#include "UnlinkedCodeBlock.h"

namespace JSC {

static StructureID getStructureID(const Instruction& instruction)
{
    return instruction.u.structureID;
}

static StructureID getStructureID(const UnlinkedInstruction&)
{
    return 0;
}

static Special::Pointer getSpecialPointer(const Instruction& instruction)
{
    return instruction.u.specialPointer;
}

static Special::Pointer getSpecialPointer(const UnlinkedInstruction& instruction)
{
    return static_cast<Special::Pointer>(instruction.u.operand);
}

static PutByIdFlags getPutByIdFlags(const Instruction& instruction)
{
    return instruction.u.putByIdFlags;
}

static PutByIdFlags getPutByIdFlags(const UnlinkedInstruction& instruction)
{
    return static_cast<PutByIdFlags>(instruction.u.operand);
}

static ToThisStatus getToThisStatus(const Instruction& instruction)
{
    return instruction.u.toThisStatus;
}

static ToThisStatus getToThisStatus(const UnlinkedInstruction& instruction)
{
    return static_cast<ToThisStatus>(instruction.u.operand);
}

static void* getPointer(const Instruction& instruction)
{
    return instruction.u.pointer;
}

static void* getPointer(const UnlinkedInstruction&)
{
    return nullptr;
}

static StructureChain* getStructureChain(const Instruction& instruction)
{
    return instruction.u.structureChain.get();
}

static StructureChain* getStructureChain(const UnlinkedInstruction&)
{
    return nullptr;
}

static Structure* getStructure(const Instruction& instruction)
{
    return instruction.u.structure.get();
}

static Structure* getStructure(const UnlinkedInstruction&)
{
    return nullptr;
}

static LLIntCallLinkInfo* getCallLinkInfo(const Instruction& instruction)
{
    return instruction.u.callLinkInfo;
}

static LLIntCallLinkInfo* getCallLinkInfo(const UnlinkedInstruction&)
{
    return nullptr;
}

static BasicBlockLocation* getBasicBlockLocation(const Instruction& instruction)
{
    return instruction.u.basicBlockLocation;
}

static BasicBlockLocation* getBasicBlockLocation(const UnlinkedInstruction&)
{
    return nullptr;
}

template<class Block>
void* BytecodeDumper<Block>::actualPointerFor(Special::Pointer) const
{
    return nullptr;
}

template<>
void* BytecodeDumper<CodeBlock>::actualPointerFor(Special::Pointer pointer) const
{
    return block()->globalObject()->actualPointerFor(pointer);
}

static void beginDumpProfiling(PrintStream& out, bool& hasPrintedProfiling)
{
    if (hasPrintedProfiling) {
        out.print("; ");
        return;
    }

    out.print("    ");
    hasPrintedProfiling = true;
}

template<class Block>
void BytecodeDumper<Block>::dumpValueProfiling(PrintStream&, const typename Block::Instruction*& it, bool&)
{
    ++it;
}

template<>
void BytecodeDumper<CodeBlock>::dumpValueProfiling(PrintStream& out, const typename CodeBlock::Instruction*& it, bool& hasPrintedProfiling)
{
    ConcurrentJSLocker locker(block()->m_lock);

    ++it;
    CString description = it->u.profile->briefDescription(locker);
    if (!description.length())
        return;
    beginDumpProfiling(out, hasPrintedProfiling);
    out.print(description);
}

template<class Block>
void BytecodeDumper<Block>::dumpArrayProfiling(PrintStream&, const typename Block::Instruction*& it, bool&)
{
    ++it;
}

template<>
void BytecodeDumper<CodeBlock>::dumpArrayProfiling(PrintStream& out, const typename CodeBlock::Instruction*& it, bool& hasPrintedProfiling)
{
    ConcurrentJSLocker locker(block()->m_lock);

    ++it;
    if (!it->u.arrayProfile)
        return;
    CString description = it->u.arrayProfile->briefDescription(locker, block());
    if (!description.length())
        return;
    beginDumpProfiling(out, hasPrintedProfiling);
    out.print(description);
}

template<class Block>
void BytecodeDumper<Block>::dumpProfilesForBytecodeOffset(PrintStream&, unsigned, bool&)
{
}

static void dumpRareCaseProfile(PrintStream& out, const char* name, RareCaseProfile* profile, bool& hasPrintedProfiling)
{
    if (!profile || !profile->m_counter)
        return;

    beginDumpProfiling(out, hasPrintedProfiling);
    out.print(name, profile->m_counter);
}

static void dumpArithProfile(PrintStream& out, ArithProfile* profile, bool& hasPrintedProfiling)
{
    if (!profile)
        return;

    beginDumpProfiling(out, hasPrintedProfiling);
    out.print("results: ", *profile);
}

template<>
void BytecodeDumper<CodeBlock>::dumpProfilesForBytecodeOffset(PrintStream& out, unsigned location, bool& hasPrintedProfiling)
{
    dumpRareCaseProfile(out, "rare case: ", block()->rareCaseProfileForBytecodeOffset(location), hasPrintedProfiling);
    {
        dumpArithProfile(out, block()->arithProfileForBytecodeOffset(location), hasPrintedProfiling);
    }

#if ENABLE(DFG_JIT)
    Vector<DFG::FrequentExitSite> exitSites = block()->exitProfile().exitSitesFor(location);
    if (!exitSites.isEmpty()) {
        out.print(" !! frequent exits: ");
        CommaPrinter comma;
        for (auto& exitSite : exitSites)
            out.print(comma, exitSite.kind(), " ", exitSite.jitType());
    }
#else // ENABLE(DFG_JIT)
    UNUSED_PARAM(location);
#endif // ENABLE(DFG_JIT)
}

template<class Block>
VM* BytecodeDumper<Block>::vm() const
{
    return block()->vm();
}

template<class Block>
const Identifier& BytecodeDumper<Block>::identifier(int index) const
{
    return block()->identifier(index);
}

static CString regexpToSourceString(RegExp* regExp)
{
    char postfix[5] = { '/', 0, 0, 0, 0 };
    int index = 1;
    if (regExp->global())
        postfix[index++] = 'g';
    if (regExp->ignoreCase())
        postfix[index++] = 'i';
    if (regExp->multiline())
        postfix[index] = 'm';
    if (regExp->sticky())
        postfix[index++] = 'y';
    if (regExp->unicode())
        postfix[index++] = 'u';

    return toCString("/", regExp->pattern().impl(), postfix);
}

static CString regexpName(int re, RegExp* regexp)
{
    return toCString(regexpToSourceString(regexp), "(@re", re, ")");
}

template<class Instruction>
static void printLocationAndOp(PrintStream& out, int location, const Instruction*&, const char* op)
{
    out.printf("[%4d] %-17s ", location, op);
}

static ALWAYS_INLINE bool isConstantRegisterIndex(int index)
{
    return index >= FirstConstantRegisterIndex;
}

NEVER_INLINE static const char* debugHookName(int debugHookType)
{
    switch (static_cast<DebugHookType>(debugHookType)) {
    case DidEnterCallFrame:
        return "didEnterCallFrame";
    case WillLeaveCallFrame:
        return "willLeaveCallFrame";
    case WillExecuteStatement:
        return "willExecuteStatement";
    case WillExecuteExpression:
        return "willExecuteExpression";
    case WillExecuteProgram:
        return "willExecuteProgram";
    case DidExecuteProgram:
        return "didExecuteProgram";
    case DidReachBreakpoint:
        return "didReachBreakpoint";
    }

    RELEASE_ASSERT_NOT_REACHED();
    return "";
}

template<class Block>
CString BytecodeDumper<Block>::registerName(int r) const
{
    if (isConstantRegisterIndex(r))
        return constantName(r);

    return toCString(VirtualRegister(r));
}

static CString idName(int id0, const Identifier& ident)
{
    return toCString(ident.impl(), "(@id", id0, ")");
}

template<class Block>
CString BytecodeDumper<Block>::constantName(int index) const
{
    JSValue value = block()->getConstant(index);
    return toCString(value, "(", VirtualRegister(index), ")");
}

template<class Block>
void BytecodeDumper<Block>::printUnaryOp(PrintStream& out, int location, const typename Block::Instruction*& it, const char* op)
{
    int r0 = (++it)->u.operand;
    int r1 = (++it)->u.operand;

    printLocationAndOp(out, location, it, op);
    out.printf("%s, %s", registerName(r0).data(), registerName(r1).data());
}

template<class Block>
void BytecodeDumper<Block>::printBinaryOp(PrintStream& out, int location, const typename Block::Instruction*& it, const char* op)
{
    int r0 = (++it)->u.operand;
    int r1 = (++it)->u.operand;
    int r2 = (++it)->u.operand;
    printLocationAndOp(out, location, it, op);
    out.printf("%s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data());
}

template<class Block>
void BytecodeDumper<Block>::printConditionalJump(PrintStream& out, const typename Block::Instruction*, const typename Block::Instruction*& it, int location, const char* op)
{
    int r0 = (++it)->u.operand;
    int offset = (++it)->u.operand;
    printLocationAndOp(out, location, it, op);
    out.printf("%s, %d(->%d)", registerName(r0).data(), offset, location + offset);
}

template<class Block>
void BytecodeDumper<Block>::printGetByIdOp(PrintStream& out, int location, const typename Block::Instruction*& it)
{
    const char* op;
    switch (vm()->interpreter->getOpcodeID(*it)) {
    case op_get_by_id:
        op = "get_by_id";
        break;
    case op_get_by_id_proto_load:
        op = "get_by_id_proto_load";
        break;
    case op_get_by_id_unset:
        op = "get_by_id_unset";
        break;
    case op_get_array_length:
        op = "array_length";
        break;
    default:
        RELEASE_ASSERT_NOT_REACHED();
#if COMPILER_QUIRK(CONSIDERS_UNREACHABLE_CODE)
        op = 0;
#endif
    }
    int r0 = (++it)->u.operand;
    int r1 = (++it)->u.operand;
    int id0 = (++it)->u.operand;
    printLocationAndOp(out, location, it, op);
    out.printf("%s, %s, %s", registerName(r0).data(), registerName(r1).data(), idName(id0, identifier(id0)).data());
    it += 4; // Increment up to the value profiler.
}

static void dumpStructure(PrintStream& out, const char* name, Structure* structure, const Identifier& ident)
{
    if (!structure)
        return;

    out.printf("%s = %p", name, structure);

    PropertyOffset offset = structure->getConcurrently(ident.impl());
    if (offset != invalidOffset)
        out.printf(" (offset = %d)", offset);
}

static void dumpChain(PrintStream& out, StructureChain* chain, const Identifier& ident)
{
    out.printf("chain = %p: [", chain);
    bool first = true;
    for (WriteBarrier<Structure>* currentStructure = chain->head(); *currentStructure; ++currentStructure) {
        if (first)
            first = false;
        else
            out.printf(", ");
        dumpStructure(out, "struct", currentStructure->get(), ident);
    }
    out.printf("]");
}

template<class Block>
void BytecodeDumper<Block>::printGetByIdCacheStatus(PrintStream& out, int location, const StubInfoMap& map)
{
    const auto* instruction = instructionsBegin() + location;

    const Identifier& ident = identifier(instruction[3].u.operand);

    UNUSED_PARAM(ident); // tell the compiler to shut up in certain platform configurations.

    if (vm()->interpreter->getOpcodeID(instruction[0]) == op_get_array_length)
        out.printf(" llint(array_length)");
    else if (StructureID structureID = getStructureID(instruction[4])) {
        Structure* structure = vm()->heap.structureIDTable().get(structureID);
        out.printf(" llint(");
        dumpStructure(out, "struct", structure, ident);
        out.printf(")");
        if (vm()->interpreter->getOpcodeID(instruction[0]) == op_get_by_id_proto_load)
            out.printf(" proto(%p)", getPointer(instruction[6]));
    }

#if ENABLE(JIT)
    if (StructureStubInfo* stubPtr = map.get(CodeOrigin(location))) {
        StructureStubInfo& stubInfo = *stubPtr;
        if (stubInfo.resetByGC)
            out.print(" (Reset By GC)");

        out.printf(" jit(");

        Structure* baseStructure = nullptr;
        PolymorphicAccess* stub = nullptr;

        switch (stubInfo.cacheType) {
        case CacheType::GetByIdSelf:
            out.printf("self");
            baseStructure = stubInfo.u.byIdSelf.baseObjectStructure.get();
            break;
        case CacheType::Stub:
            out.printf("stub");
            stub = stubInfo.u.stub;
            break;
        case CacheType::Unset:
            out.printf("unset");
            break;
        case CacheType::ArrayLength:
            out.printf("ArrayLength");
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }

        if (baseStructure) {
            out.printf(", ");
            dumpStructure(out, "struct", baseStructure, ident);
        }

        if (stub)
            out.print(", ", *stub);

        out.printf(")");
    }
#else
    UNUSED_PARAM(map);
#endif
}

template<class Block>
void BytecodeDumper<Block>::printPutByIdCacheStatus(PrintStream& out, int location, const StubInfoMap& map)
{
    const auto* instruction = instructionsBegin() + location;

    const Identifier& ident = identifier(instruction[2].u.operand);

    UNUSED_PARAM(ident); // tell the compiler to shut up in certain platform configurations.

    out.print(", ", getPutByIdFlags(instruction[8]));

    if (StructureID structureID = getStructureID(instruction[4])) {
        Structure* structure = vm()->heap.structureIDTable().get(structureID);
        out.print(" llint(");
        if (StructureID newStructureID = getStructureID(instruction[6])) {
            Structure* newStructure = vm()->heap.structureIDTable().get(newStructureID);
            dumpStructure(out, "prev", structure, ident);
            out.print(", ");
            dumpStructure(out, "next", newStructure, ident);
            if (StructureChain* chain = getStructureChain(instruction[7])) {
                out.print(", ");
                dumpChain(out, chain, ident);
            }
        } else
            dumpStructure(out, "struct", structure, ident);
        out.print(")");
    }

#if ENABLE(JIT)
    if (StructureStubInfo* stubPtr = map.get(CodeOrigin(location))) {
        StructureStubInfo& stubInfo = *stubPtr;
        if (stubInfo.resetByGC)
            out.print(" (Reset By GC)");

        out.printf(" jit(");

        switch (stubInfo.cacheType) {
        case CacheType::PutByIdReplace:
            out.print("replace, ");
            dumpStructure(out, "struct", stubInfo.u.byIdSelf.baseObjectStructure.get(), ident);
            break;
        case CacheType::Stub: {
            out.print("stub, ", *stubInfo.u.stub);
            break;
        }
        case CacheType::Unset:
            out.printf("unset");
            break;
        default:
            RELEASE_ASSERT_NOT_REACHED();
            break;
        }
        out.printf(")");
    }
#else
    UNUSED_PARAM(map);
#endif
}

#if ENABLE(JIT)
template<typename Block>
void BytecodeDumper<Block>::dumpCallLinkStatus(PrintStream&, unsigned, const CallLinkInfoMap&)
{
}

template<>
void BytecodeDumper<CodeBlock>::dumpCallLinkStatus(PrintStream& out, unsigned location, const CallLinkInfoMap& map)
{
    if (block()->jitType() != JITCode::FTLJIT)
        out.print(" status(", CallLinkStatus::computeFor(block(), location, map), ")");
}
#endif

template<class Block>
void BytecodeDumper<Block>::printCallOp(PrintStream& out, int location, const typename Block::Instruction*& it, const char* op, CacheDumpMode cacheDumpMode, bool& hasPrintedProfiling, const CallLinkInfoMap& map)
{
    int dst = (++it)->u.operand;
    int func = (++it)->u.operand;
    int argCount = (++it)->u.operand;
    int registerOffset = (++it)->u.operand;
    printLocationAndOp(out, location, it, op);
    out.print(registerName(dst), ", ", registerName(func), ", ", argCount, ", ", registerOffset);
    out.print(" (this at ", virtualRegisterForArgument(0, -registerOffset), ")");
    if (cacheDumpMode == DumpCaches) {
        LLIntCallLinkInfo* callLinkInfo = getCallLinkInfo(it[1]);
        if (callLinkInfo->lastSeenCallee) {
            out.printf(
                " llint(%p, exec %p)",
                callLinkInfo->lastSeenCallee.get(),
                callLinkInfo->lastSeenCallee->executable());
        }
#if ENABLE(JIT)
        if (CallLinkInfo* info = map.get(CodeOrigin(location))) {
            JSFunction* target = info->lastSeenCallee();
            if (target)
                out.printf(" jit(%p, exec %p)", target, target->executable());
        }

        dumpCallLinkStatus(out, location, map);
#else
        UNUSED_PARAM(map);
#endif
    }
    ++it;
    ++it;
    dumpArrayProfiling(out, it, hasPrintedProfiling);
    dumpValueProfiling(out, it, hasPrintedProfiling);
}

template<class Block>
void BytecodeDumper<Block>::printPutByIdOp(PrintStream& out, int location, const typename Block::Instruction*& it, const char* op)
{
    int r0 = (++it)->u.operand;
    int id0 = (++it)->u.operand;
    int r1 = (++it)->u.operand;
    printLocationAndOp(out, location, it, op);
    out.printf("%s, %s, %s", registerName(r0).data(), idName(id0, identifier(id0)).data(), registerName(r1).data());
    it += 5;
}

template<class Block>
void BytecodeDumper<Block>::printLocationOpAndRegisterOperand(PrintStream& out, int location, const typename Block::Instruction*& it, const char* op, int operand)
{
    printLocationAndOp(out, location, it, op);
    out.printf("%s", registerName(operand).data());
}

template<class Block>
void BytecodeDumper<Block>::dumpBytecode(PrintStream& out, const typename Block::Instruction* begin, const typename Block::Instruction*& it, const StubInfoMap& stubInfos, const CallLinkInfoMap& callLinkInfos)
{
    int location = it - begin;
    bool hasPrintedProfiling = false;
    OpcodeID opcode = vm()->interpreter->getOpcodeID(*it);
    switch (opcode) {
    case op_enter: {
        printLocationAndOp(out, location, it, "enter");
        break;
    }
    case op_get_scope: {
        int r0 = (++it)->u.operand;
        printLocationOpAndRegisterOperand(out, location, it, "get_scope", r0);
        break;
    }
    case op_create_direct_arguments: {
        int r0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "create_direct_arguments");
        out.printf("%s", registerName(r0).data());
        break;
    }
    case op_create_scoped_arguments: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "create_scoped_arguments");
        out.printf("%s, %s", registerName(r0).data(), registerName(r1).data());
        break;
    }
    case op_create_cloned_arguments: {
        int r0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "create_cloned_arguments");
        out.printf("%s", registerName(r0).data());
        break;
    }
    case op_argument_count: {
        int r0 = (++it)->u.operand;
        printLocationOpAndRegisterOperand(out, location, it, "argument_count", r0);
        break;
    }
    case op_get_argument: {
        int r0 = (++it)->u.operand;
        int index = (++it)->u.operand;
        printLocationOpAndRegisterOperand(out, location, it, "argument", r0);
        out.printf(", %d", index);
        dumpValueProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_create_rest: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        unsigned argumentOffset = (++it)->u.unsignedValue;
        printLocationAndOp(out, location, it, "create_rest");
        out.printf("%s, %s, ", registerName(r0).data(), registerName(r1).data());
        out.printf("ArgumentsOffset: %u", argumentOffset);
        break;
    }
    case op_get_rest_length: {
        int r0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "get_rest_length");
        out.printf("%s, ", registerName(r0).data());
        unsigned argumentOffset = (++it)->u.unsignedValue;
        out.printf("ArgumentsOffset: %u", argumentOffset);
        break;
    }
    case op_create_this: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        unsigned inferredInlineCapacity = (++it)->u.operand;
        unsigned cachedFunction = (++it)->u.operand;
        printLocationAndOp(out, location, it, "create_this");
        out.printf("%s, %s, %u, %u", registerName(r0).data(), registerName(r1).data(), inferredInlineCapacity, cachedFunction);
        break;
    }
    case op_to_this: {
        int r0 = (++it)->u.operand;
        printLocationOpAndRegisterOperand(out, location, it, "to_this", r0);
        Structure* structure = getStructure(*(++it));
        if (structure)
            out.print(", cache(struct = ", RawPointer(structure), ")");
        out.print(", ", getToThisStatus(*(++it)));
        break;
    }
    case op_check_tdz: {
        int r0 = (++it)->u.operand;
        printLocationOpAndRegisterOperand(out, location, it, "op_check_tdz", r0);
        break;
    }
    case op_new_object: {
        int r0 = (++it)->u.operand;
        unsigned inferredInlineCapacity = (++it)->u.operand;
        printLocationAndOp(out, location, it, "new_object");
        out.printf("%s, %u", registerName(r0).data(), inferredInlineCapacity);
        ++it; // Skip object allocation profile.
        break;
    }
    case op_new_array: {
        int dst = (++it)->u.operand;
        int argv = (++it)->u.operand;
        int argc = (++it)->u.operand;
        printLocationAndOp(out, location, it, "new_array");
        out.printf("%s, %s, %d", registerName(dst).data(), registerName(argv).data(), argc);
        ++it; // Skip array allocation profile.
        break;
    }
    case op_new_array_with_spread: {
        int dst = (++it)->u.operand;
        int argv = (++it)->u.operand;
        int argc = (++it)->u.operand;
        printLocationAndOp(out, location, it, "new_array_with_spread");
        out.printf("%s, %s, %d, ", registerName(dst).data(), registerName(argv).data(), argc);
        unsigned bitVectorIndex = (++it)->u.unsignedValue;
        const BitVector& bitVector = block()->bitVector(bitVectorIndex);
        out.print("BitVector:", bitVectorIndex, ":");
        for (unsigned i = 0; i < static_cast<unsigned>(argc); i++) {
            if (bitVector.get(i))
                out.print("1");
            else
                out.print("0");
        }
        break;
    }
    case op_spread: {
        int dst = (++it)->u.operand;
        int arg = (++it)->u.operand;
        printLocationAndOp(out, location, it, "spread");
        out.printf("%s, %s", registerName(dst).data(), registerName(arg).data());
        break;
    }
    case op_new_array_with_size: {
        int dst = (++it)->u.operand;
        int length = (++it)->u.operand;
        printLocationAndOp(out, location, it, "new_array_with_size");
        out.printf("%s, %s", registerName(dst).data(), registerName(length).data());
        ++it; // Skip array allocation profile.
        break;
    }
    case op_new_array_buffer: {
        int dst = (++it)->u.operand;
        int argv = (++it)->u.operand;
        int argc = (++it)->u.operand;
        printLocationAndOp(out, location, it, "new_array_buffer");
        out.printf("%s, %d, %d", registerName(dst).data(), argv, argc);
        ++it; // Skip array allocation profile.
        break;
    }
    case op_new_regexp: {
        int r0 = (++it)->u.operand;
        int re0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "new_regexp");
        out.printf("%s, ", registerName(r0).data());
        if (r0 >=0 && r0 < (int)block()->numberOfRegExps())
            out.printf("%s", regexpName(re0, block()->regexp(re0)).data());
        else
            out.printf("bad_regexp(%d)", re0);
        break;
    }
    case op_mov: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "mov");
        out.printf("%s, %s", registerName(r0).data(), registerName(r1).data());
        break;
    }
    case op_profile_type: {
        int r0 = (++it)->u.operand;
        ++it;
        ++it;
        ++it;
        ++it;
        printLocationAndOp(out, location, it, "op_profile_type");
        out.printf("%s", registerName(r0).data());
        break;
    }
    case op_profile_control_flow: {
        BasicBlockLocation* basicBlockLocation = getBasicBlockLocation(*(++it));
        printLocationAndOp(out, location, it, "profile_control_flow");
        if (basicBlockLocation)
            out.printf("[%d, %d]", basicBlockLocation->startOffset(), basicBlockLocation->endOffset());
        break;
    }
    case op_not: {
        printUnaryOp(out, location, it, "not");
        break;
    }
    case op_eq: {
        printBinaryOp(out, location, it, "eq");
        break;
    }
    case op_eq_null: {
        printUnaryOp(out, location, it, "eq_null");
        break;
    }
    case op_neq: {
        printBinaryOp(out, location, it, "neq");
        break;
    }
    case op_neq_null: {
        printUnaryOp(out, location, it, "neq_null");
        break;
    }
    case op_stricteq: {
        printBinaryOp(out, location, it, "stricteq");
        break;
    }
    case op_nstricteq: {
        printBinaryOp(out, location, it, "nstricteq");
        break;
    }
    case op_less: {
        printBinaryOp(out, location, it, "less");
        break;
    }
    case op_lesseq: {
        printBinaryOp(out, location, it, "lesseq");
        break;
    }
    case op_greater: {
        printBinaryOp(out, location, it, "greater");
        break;
    }
    case op_greatereq: {
        printBinaryOp(out, location, it, "greatereq");
        break;
    }
    case op_inc: {
        int r0 = (++it)->u.operand;
        printLocationOpAndRegisterOperand(out, location, it, "inc", r0);
        break;
    }
    case op_dec: {
        int r0 = (++it)->u.operand;
        printLocationOpAndRegisterOperand(out, location, it, "dec", r0);
        break;
    }
    case op_to_number: {
        printUnaryOp(out, location, it, "to_number");
        dumpValueProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_to_string: {
        printUnaryOp(out, location, it, "to_string");
        break;
    }
    case op_negate: {
        printUnaryOp(out, location, it, "negate");
        ++it; // op_negate has an extra operand for the ArithProfile.
        break;
    }
    case op_add: {
        printBinaryOp(out, location, it, "add");
        ++it;
        break;
    }
    case op_mul: {
        printBinaryOp(out, location, it, "mul");
        ++it;
        break;
    }
    case op_div: {
        printBinaryOp(out, location, it, "div");
        ++it;
        break;
    }
    case op_mod: {
        printBinaryOp(out, location, it, "mod");
        break;
    }
    case op_pow: {
        printBinaryOp(out, location, it, "pow");
        break;
    }
    case op_sub: {
        printBinaryOp(out, location, it, "sub");
        ++it;
        break;
    }
    case op_lshift: {
        printBinaryOp(out, location, it, "lshift");
        break;
    }
    case op_rshift: {
        printBinaryOp(out, location, it, "rshift");
        break;
    }
    case op_urshift: {
        printBinaryOp(out, location, it, "urshift");
        break;
    }
    case op_bitand: {
        printBinaryOp(out, location, it, "bitand");
        ++it;
        break;
    }
    case op_bitxor: {
        printBinaryOp(out, location, it, "bitxor");
        ++it;
        break;
    }
    case op_bitor: {
        printBinaryOp(out, location, it, "bitor");
        ++it;
        break;
    }
    case op_overrides_has_instance: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "overrides_has_instance");
        out.printf("%s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data());
        break;
    }
    case op_instanceof: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "instanceof");
        out.printf("%s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data());
        break;
    }
    case op_instanceof_custom: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        int r3 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "instanceof_custom");
        out.printf("%s, %s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data(), registerName(r3).data());
        break;
    }
    case op_unsigned: {
        printUnaryOp(out, location, it, "unsigned");
        break;
    }
    case op_typeof: {
        printUnaryOp(out, location, it, "typeof");
        break;
    }
    case op_is_empty: {
        printUnaryOp(out, location, it, "is_empty");
        break;
    }
    case op_is_undefined: {
        printUnaryOp(out, location, it, "is_undefined");
        break;
    }
    case op_is_boolean: {
        printUnaryOp(out, location, it, "is_boolean");
        break;
    }
    case op_is_number: {
        printUnaryOp(out, location, it, "is_number");
        break;
    }
    case op_is_cell_with_type: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int type = (++it)->u.operand;
        printLocationAndOp(out, location, it, "is_cell_with_type");
        out.printf("%s, %s, %d", registerName(r0).data(), registerName(r1).data(), type);
        break;
    }
    case op_is_object: {
        printUnaryOp(out, location, it, "is_object");
        break;
    }
    case op_is_object_or_null: {
        printUnaryOp(out, location, it, "is_object_or_null");
        break;
    }
    case op_is_function: {
        printUnaryOp(out, location, it, "is_function");
        break;
    }
    case op_in: {
        printBinaryOp(out, location, it, "in");
        dumpArrayProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_try_get_by_id: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int id0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "try_get_by_id");
        out.printf("%s, %s, %s", registerName(r0).data(), registerName(r1).data(), idName(id0, identifier(id0)).data());
        dumpValueProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_get_by_id:
    case op_get_by_id_proto_load:
    case op_get_by_id_unset:
    case op_get_array_length: {
        printGetByIdOp(out, location, it);
        printGetByIdCacheStatus(out, location, stubInfos);
        dumpValueProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_get_by_id_with_this: {
        printLocationAndOp(out, location, it, "get_by_id_with_this");
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        int id0 = (++it)->u.operand;
        out.printf("%s, %s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data(), idName(id0, identifier(id0)).data());
        dumpValueProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_get_by_val_with_this: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        int r3 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "get_by_val_with_this");
        out.printf("%s, %s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data(), registerName(r3).data());
        dumpValueProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_put_by_id: {
        printPutByIdOp(out, location, it, "put_by_id");
        printPutByIdCacheStatus(out, location, stubInfos);
        break;
    }
    case op_put_by_id_with_this: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int id0 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "put_by_id_with_this");
        out.printf("%s, %s, %s, %s", registerName(r0).data(), registerName(r1).data(), idName(id0, identifier(id0)).data(), registerName(r2).data());
        break;
    }
    case op_put_by_val_with_this: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        int r3 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "put_by_val_with_this");
        out.printf("%s, %s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data(), registerName(r3).data());
        break;
    }
    case op_put_getter_by_id: {
        int r0 = (++it)->u.operand;
        int id0 = (++it)->u.operand;
        int n0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "put_getter_by_id");
        out.printf("%s, %s, %d, %s", registerName(r0).data(), idName(id0, identifier(id0)).data(), n0, registerName(r1).data());
        break;
    }
    case op_put_setter_by_id: {
        int r0 = (++it)->u.operand;
        int id0 = (++it)->u.operand;
        int n0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "put_setter_by_id");
        out.printf("%s, %s, %d, %s", registerName(r0).data(), idName(id0, identifier(id0)).data(), n0, registerName(r1).data());
        break;
    }
    case op_put_getter_setter_by_id: {
        int r0 = (++it)->u.operand;
        int id0 = (++it)->u.operand;
        int n0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "put_getter_setter_by_id");
        out.printf("%s, %s, %d, %s, %s", registerName(r0).data(), idName(id0, identifier(id0)).data(), n0, registerName(r1).data(), registerName(r2).data());
        break;
    }
    case op_put_getter_by_val: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int n0 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "put_getter_by_val");
        out.printf("%s, %s, %d, %s", registerName(r0).data(), registerName(r1).data(), n0, registerName(r2).data());
        break;
    }
    case op_put_setter_by_val: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int n0 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "put_setter_by_val");
        out.printf("%s, %s, %d, %s", registerName(r0).data(), registerName(r1).data(), n0, registerName(r2).data());
        break;
    }
    case op_define_data_property: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        int r3 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "define_data_property");
        out.printf("%s, %s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data(), registerName(r3).data());
        break;
    }
    case op_define_accessor_property: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        int r3 = (++it)->u.operand;
        int r4 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "define_accessor_property");
        out.printf("%s, %s, %s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data(), registerName(r3).data(), registerName(r4).data());
        break;
    }
    case op_del_by_id: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int id0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "del_by_id");
        out.printf("%s, %s, %s", registerName(r0).data(), registerName(r1).data(), idName(id0, identifier(id0)).data());
        break;
    }
    case op_get_by_val: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "get_by_val");
        out.printf("%s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data());
        dumpArrayProfiling(out, it, hasPrintedProfiling);
        dumpValueProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_put_by_val: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "put_by_val");
        out.printf("%s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data());
        dumpArrayProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_put_by_val_direct: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "put_by_val_direct");
        out.printf("%s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data());
        dumpArrayProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_del_by_val: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int r2 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "del_by_val");
        out.printf("%s, %s, %s", registerName(r0).data(), registerName(r1).data(), registerName(r2).data());
        break;
    }
    case op_put_by_index: {
        int r0 = (++it)->u.operand;
        unsigned n0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "put_by_index");
        out.printf("%s, %u, %s", registerName(r0).data(), n0, registerName(r1).data());
        break;
    }
    case op_jmp: {
        int offset = (++it)->u.operand;
        printLocationAndOp(out, location, it, "jmp");
        out.printf("%d(->%d)", offset, location + offset);
        break;
    }
    case op_jtrue: {
        printConditionalJump(out, begin, it, location, "jtrue");
        break;
    }
    case op_jfalse: {
        printConditionalJump(out, begin, it, location, "jfalse");
        break;
    }
    case op_jeq_null: {
        printConditionalJump(out, begin, it, location, "jeq_null");
        break;
    }
    case op_jneq_null: {
        printConditionalJump(out, begin, it, location, "jneq_null");
        break;
    }
    case op_jneq_ptr: {
        int r0 = (++it)->u.operand;
        Special::Pointer pointer = getSpecialPointer(*(++it));
        int offset = (++it)->u.operand;
        printLocationAndOp(out, location, it, "jneq_ptr");
        out.printf("%s, %d (%p), %d(->%d)", registerName(r0).data(), pointer, actualPointerFor(pointer), offset, location + offset);
        ++it;
        break;
    }
    case op_jless: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int offset = (++it)->u.operand;
        printLocationAndOp(out, location, it, "jless");
        out.printf("%s, %s, %d(->%d)", registerName(r0).data(), registerName(r1).data(), offset, location + offset);
        break;
    }
    case op_jlesseq: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int offset = (++it)->u.operand;
        printLocationAndOp(out, location, it, "jlesseq");
        out.printf("%s, %s, %d(->%d)", registerName(r0).data(), registerName(r1).data(), offset, location + offset);
        break;
    }
    case op_jgreater: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int offset = (++it)->u.operand;
        printLocationAndOp(out, location, it, "jgreater");
        out.printf("%s, %s, %d(->%d)", registerName(r0).data(), registerName(r1).data(), offset, location + offset);
        break;
    }
    case op_jgreatereq: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int offset = (++it)->u.operand;
        printLocationAndOp(out, location, it, "jgreatereq");
        out.printf("%s, %s, %d(->%d)", registerName(r0).data(), registerName(r1).data(), offset, location + offset);
        break;
    }
    case op_jnless: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int offset = (++it)->u.operand;
        printLocationAndOp(out, location, it, "jnless");
        out.printf("%s, %s, %d(->%d)", registerName(r0).data(), registerName(r1).data(), offset, location + offset);
        break;
    }
    case op_jnlesseq: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int offset = (++it)->u.operand;
        printLocationAndOp(out, location, it, "jnlesseq");
        out.printf("%s, %s, %d(->%d)", registerName(r0).data(), registerName(r1).data(), offset, location + offset);
        break;
    }
    case op_jngreater: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int offset = (++it)->u.operand;
        printLocationAndOp(out, location, it, "jngreater");
        out.printf("%s, %s, %d(->%d)", registerName(r0).data(), registerName(r1).data(), offset, location + offset);
        break;
    }
    case op_jngreatereq: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int offset = (++it)->u.operand;
        printLocationAndOp(out, location, it, "jngreatereq");
        out.printf("%s, %s, %d(->%d)", registerName(r0).data(), registerName(r1).data(), offset, location + offset);
        break;
    }
    case op_loop_hint: {
        printLocationAndOp(out, location, it, "loop_hint");
        break;
    }
    case op_check_traps: {
        printLocationAndOp(out, location, it, "check_traps");
        break;
    }
    case op_log_shadow_chicken_prologue: {
        int r0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "log_shadow_chicken_prologue");
        out.printf("%s", registerName(r0).data());
        break;
    }
    case op_log_shadow_chicken_tail: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "log_shadow_chicken_tail");
        out.printf("%s, %s", registerName(r0).data(), registerName(r1).data());
        break;
    }
    case op_switch_imm: {
        int tableIndex = (++it)->u.operand;
        int defaultTarget = (++it)->u.operand;
        int scrutineeRegister = (++it)->u.operand;
        printLocationAndOp(out, location, it, "switch_imm");
        out.printf("%d, %d(->%d), %s", tableIndex, defaultTarget, location + defaultTarget, registerName(scrutineeRegister).data());
        break;
    }
    case op_switch_char: {
        int tableIndex = (++it)->u.operand;
        int defaultTarget = (++it)->u.operand;
        int scrutineeRegister = (++it)->u.operand;
        printLocationAndOp(out, location, it, "switch_char");
        out.printf("%d, %d(->%d), %s", tableIndex, defaultTarget, location + defaultTarget, registerName(scrutineeRegister).data());
        break;
    }
    case op_switch_string: {
        int tableIndex = (++it)->u.operand;
        int defaultTarget = (++it)->u.operand;
        int scrutineeRegister = (++it)->u.operand;
        printLocationAndOp(out, location, it, "switch_string");
        out.printf("%d, %d(->%d), %s", tableIndex, defaultTarget, location + defaultTarget, registerName(scrutineeRegister).data());
        break;
    }
    case op_new_func: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int f0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "new_func");
        out.printf("%s, %s, f%d", registerName(r0).data(), registerName(r1).data(), f0);
        break;
    }
    case op_new_generator_func: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int f0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "new_generator_func");
        out.printf("%s, %s, f%d", registerName(r0).data(), registerName(r1).data(), f0);
        break;
    }
    case op_new_async_func: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int f0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "new_async_func");
        out.printf("%s, %s, f%d", registerName(r0).data(), registerName(r1).data(), f0);
        break;
    }
    case op_new_func_exp: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int f0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "new_func_exp");
        out.printf("%s, %s, f%d", registerName(r0).data(), registerName(r1).data(), f0);
        break;
    }
    case op_new_generator_func_exp: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int f0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "new_generator_func_exp");
        out.printf("%s, %s, f%d", registerName(r0).data(), registerName(r1).data(), f0);
        break;
    }
    case op_new_async_func_exp: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int f0 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "new_async_func_exp");
        out.printf("%s, %s, f%d", registerName(r0).data(), registerName(r1).data(), f0);
        break;
    }
    case op_set_function_name: {
        int funcReg = (++it)->u.operand;
        int nameReg = (++it)->u.operand;
        printLocationAndOp(out, location, it, "set_function_name");
        out.printf("%s, %s", registerName(funcReg).data(), registerName(nameReg).data());
        break;
    }
    case op_call: {
        printCallOp(out, location, it, "call", DumpCaches, hasPrintedProfiling, callLinkInfos);
        break;
    }
    case op_tail_call: {
        printCallOp(out, location, it, "tail_call", DumpCaches, hasPrintedProfiling, callLinkInfos);
        break;
    }
    case op_call_eval: {
        printCallOp(out, location, it, "call_eval", DontDumpCaches, hasPrintedProfiling, callLinkInfos);
        break;
    }

    case op_construct_varargs:
    case op_call_varargs:
    case op_tail_call_varargs:
    case op_tail_call_forward_arguments: {
        int result = (++it)->u.operand;
        int callee = (++it)->u.operand;
        int thisValue = (++it)->u.operand;
        int arguments = (++it)->u.operand;
        int firstFreeRegister = (++it)->u.operand;
        int varArgOffset = (++it)->u.operand;
        ++it;
        const char* opName;
        if (opcode == op_call_varargs)
            opName = "call_varargs";
        else if (opcode == op_construct_varargs)
            opName = "construct_varargs";
        else if (opcode == op_tail_call_varargs)
            opName = "tail_call_varargs";
        else if (opcode == op_tail_call_forward_arguments)
            opName = "tail_call_forward_arguments";
        else
            RELEASE_ASSERT_NOT_REACHED();

        printLocationAndOp(out, location, it, opName);
        out.printf("%s, %s, %s, %s, %d, %d", registerName(result).data(), registerName(callee).data(), registerName(thisValue).data(), registerName(arguments).data(), firstFreeRegister, varArgOffset);
        dumpValueProfiling(out, it, hasPrintedProfiling);
        break;
    }

    case op_ret: {
        int r0 = (++it)->u.operand;
        printLocationOpAndRegisterOperand(out, location, it, "ret", r0);
        break;
    }
    case op_construct: {
        printCallOp(out, location, it, "construct", DumpCaches, hasPrintedProfiling, callLinkInfos);
        break;
    }
    case op_strcat: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int count = (++it)->u.operand;
        printLocationAndOp(out, location, it, "strcat");
        out.printf("%s, %s, %d", registerName(r0).data(), registerName(r1).data(), count);
        break;
    }
    case op_to_primitive: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "to_primitive");
        out.printf("%s, %s", registerName(r0).data(), registerName(r1).data());
        break;
    }
    case op_get_enumerable_length: {
        int dst = it[1].u.operand;
        int base = it[2].u.operand;
        printLocationAndOp(out, location, it, "op_get_enumerable_length");
        out.printf("%s, %s", registerName(dst).data(), registerName(base).data());
        it += OPCODE_LENGTH(op_get_enumerable_length) - 1;
        break;
    }
    case op_has_indexed_property: {
        int dst = (++it)->u.operand;
        int base = (++it)->u.operand;
        int propertyName = (++it)->u.operand;
        printLocationAndOp(out, location, it, "op_has_indexed_property");
        out.printf("%s, %s, %s", registerName(dst).data(), registerName(base).data(), registerName(propertyName).data());
        dumpArrayProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_has_structure_property: {
        int dst = it[1].u.operand;
        int base = it[2].u.operand;
        int propertyName = it[3].u.operand;
        int enumerator = it[4].u.operand;
        printLocationAndOp(out, location, it, "op_has_structure_property");
        out.printf("%s, %s, %s, %s", registerName(dst).data(), registerName(base).data(), registerName(propertyName).data(), registerName(enumerator).data());
        it += OPCODE_LENGTH(op_has_structure_property) - 1;
        break;
    }
    case op_has_generic_property: {
        int dst = it[1].u.operand;
        int base = it[2].u.operand;
        int propertyName = it[3].u.operand;
        printLocationAndOp(out, location, it, "op_has_generic_property");
        out.printf("%s, %s, %s", registerName(dst).data(), registerName(base).data(), registerName(propertyName).data());
        it += OPCODE_LENGTH(op_has_generic_property) - 1;
        break;
    }
    case op_get_direct_pname: {
        int dst = (++it)->u.operand;
        int base = (++it)->u.operand;
        int propertyName = (++it)->u.operand;
        int index = (++it)->u.operand;
        int enumerator = (++it)->u.operand;
        printLocationAndOp(out, location, it, "op_get_direct_pname");
        out.printf("%s, %s, %s, %s, %s", registerName(dst).data(), registerName(base).data(), registerName(propertyName).data(), registerName(index).data(), registerName(enumerator).data());
        dumpValueProfiling(out, it, hasPrintedProfiling);
        break;

    }
    case op_get_property_enumerator: {
        int dst = it[1].u.operand;
        int base = it[2].u.operand;
        printLocationAndOp(out, location, it, "op_get_property_enumerator");
        out.printf("%s, %s", registerName(dst).data(), registerName(base).data());
        it += OPCODE_LENGTH(op_get_property_enumerator) - 1;
        break;
    }
    case op_enumerator_structure_pname: {
        int dst = it[1].u.operand;
        int enumerator = it[2].u.operand;
        int index = it[3].u.operand;
        printLocationAndOp(out, location, it, "op_enumerator_structure_pname");
        out.printf("%s, %s, %s", registerName(dst).data(), registerName(enumerator).data(), registerName(index).data());
        it += OPCODE_LENGTH(op_enumerator_structure_pname) - 1;
        break;
    }
    case op_enumerator_generic_pname: {
        int dst = it[1].u.operand;
        int enumerator = it[2].u.operand;
        int index = it[3].u.operand;
        printLocationAndOp(out, location, it, "op_enumerator_generic_pname");
        out.printf("%s, %s, %s", registerName(dst).data(), registerName(enumerator).data(), registerName(index).data());
        it += OPCODE_LENGTH(op_enumerator_generic_pname) - 1;
        break;
    }
    case op_to_index_string: {
        int dst = it[1].u.operand;
        int index = it[2].u.operand;
        printLocationAndOp(out, location, it, "op_to_index_string");
        out.printf("%s, %s", registerName(dst).data(), registerName(index).data());
        it += OPCODE_LENGTH(op_to_index_string) - 1;
        break;
    }
    case op_push_with_scope: {
        int dst = (++it)->u.operand;
        int newScope = (++it)->u.operand;
        int currentScope = (++it)->u.operand;
        printLocationAndOp(out, location, it, "push_with_scope");
        out.printf("%s, %s, %s", registerName(dst).data(), registerName(newScope).data(), registerName(currentScope).data());
        break;
    }
    case op_get_parent_scope: {
        int dst = (++it)->u.operand;
        int parentScope = (++it)->u.operand;
        printLocationAndOp(out, location, it, "get_parent_scope");
        out.printf("%s, %s", registerName(dst).data(), registerName(parentScope).data());
        break;
    }
    case op_create_lexical_environment: {
        int dst = (++it)->u.operand;
        int scope = (++it)->u.operand;
        int symbolTable = (++it)->u.operand;
        int initialValue = (++it)->u.operand;
        printLocationAndOp(out, location, it, "create_lexical_environment");
        out.printf("%s, %s, %s, %s",
            registerName(dst).data(), registerName(scope).data(), registerName(symbolTable).data(), registerName(initialValue).data());
        break;
    }
    case op_catch: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "catch");
        out.printf("%s, %s", registerName(r0).data(), registerName(r1).data());
        break;
    }
    case op_throw: {
        int r0 = (++it)->u.operand;
        printLocationOpAndRegisterOperand(out, location, it, "throw", r0);
        break;
    }
    case op_throw_static_error: {
        int k0 = (++it)->u.operand;
        ErrorType k1 = static_cast<ErrorType>((++it)->u.unsignedValue);
        printLocationAndOp(out, location, it, "throw_static_error");
        out.printf("%s, ", constantName(k0).data());
        out.print(k1);
        break;
    }
    case op_debug: {
        int debugHookType = (++it)->u.operand;
        int hasBreakpointFlag = (++it)->u.operand;
        printLocationAndOp(out, location, it, "debug");
        out.printf("%s, %d", debugHookName(debugHookType), hasBreakpointFlag);
        break;
    }
    case op_assert: {
        int condition = (++it)->u.operand;
        int line = (++it)->u.operand;
        printLocationAndOp(out, location, it, "assert");
        out.printf("%s, %d", registerName(condition).data(), line);
        break;
    }
    case op_end: {
        int r0 = (++it)->u.operand;
        printLocationOpAndRegisterOperand(out, location, it, "end", r0);
        break;
    }
    case op_resolve_scope: {
        int r0 = (++it)->u.operand;
        int scope = (++it)->u.operand;
        int id0 = (++it)->u.operand;
        ResolveType resolveType = static_cast<ResolveType>((++it)->u.operand);
        int depth = (++it)->u.operand;
        void* pointer = getPointer(*(++it));
        printLocationAndOp(out, location, it, "resolve_scope");
        out.printf("%s, %s, %s, <%s>, %d, %p", registerName(r0).data(), registerName(scope).data(), idName(id0, identifier(id0)).data(), resolveTypeName(resolveType), depth, pointer);
        break;
    }
    case op_get_from_scope: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int id0 = (++it)->u.operand;
        GetPutInfo getPutInfo = GetPutInfo((++it)->u.operand);
        ++it; // Structure
        int operand = (++it)->u.operand; // Operand
        printLocationAndOp(out, location, it, "get_from_scope");
        out.print(registerName(r0), ", ", registerName(r1));
        if (static_cast<unsigned>(id0) == UINT_MAX)
            out.print(", anonymous");
        else
            out.print(", ", idName(id0, identifier(id0)));
        out.print(", ", getPutInfo.operand(), "<", resolveModeName(getPutInfo.resolveMode()), "|", resolveTypeName(getPutInfo.resolveType()), "|", initializationModeName(getPutInfo.initializationMode()), ">, ", operand);
        dumpValueProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_put_to_scope: {
        int r0 = (++it)->u.operand;
        int id0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        GetPutInfo getPutInfo = GetPutInfo((++it)->u.operand);
        ++it; // Structure
        int operand = (++it)->u.operand; // Operand
        printLocationAndOp(out, location, it, "put_to_scope");
        out.print(registerName(r0));
        if (static_cast<unsigned>(id0) == UINT_MAX)
            out.print(", anonymous");
        else
            out.print(", ", idName(id0, identifier(id0)));
        out.print(", ", registerName(r1), ", ", getPutInfo.operand(), "<", resolveModeName(getPutInfo.resolveMode()), "|", resolveTypeName(getPutInfo.resolveType()), "|", initializationModeName(getPutInfo.initializationMode()), ">, <structure>, ", operand);
        break;
    }
    case op_get_from_arguments: {
        int r0 = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        int offset = (++it)->u.operand;
        printLocationAndOp(out, location, it, "get_from_arguments");
        out.printf("%s, %s, %d", registerName(r0).data(), registerName(r1).data(), offset);
        dumpValueProfiling(out, it, hasPrintedProfiling);
        break;
    }
    case op_put_to_arguments: {
        int r0 = (++it)->u.operand;
        int offset = (++it)->u.operand;
        int r1 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "put_to_arguments");
        out.printf("%s, %d, %s", registerName(r0).data(), offset, registerName(r1).data());
        break;
    }
    case op_yield: {
        int r0 = (++it)->u.operand;
        unsigned yieldPoint = (++it)->u.unsignedValue;
        int r1 = (++it)->u.operand;
        printLocationAndOp(out, location, it, "yield");
        out.printf("%s, %u, %s", registerName(r0).data(), yieldPoint, registerName(r1).data());
        break;
    }
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }
    dumpProfilesForBytecodeOffset(out, location, hasPrintedProfiling);
    out.print("\n");
}

template<class Block>
void BytecodeDumper<Block>::dumpBytecode(Block* block, PrintStream& out, const typename Block::Instruction* begin, const typename Block::Instruction*& it, const StubInfoMap& stubInfos, const CallLinkInfoMap& callLinkInfos)
{
    BytecodeDumper dumper(block, begin);
    dumper.dumpBytecode(out, begin, it, stubInfos, callLinkInfos);
}

template<class Block>
void BytecodeDumper<Block>::dumpIdentifiers(PrintStream& out)
{
    if (size_t count = block()->numberOfIdentifiers()) {
        out.printf("\nIdentifiers:\n");
        size_t i = 0;
        do {
            out.printf("  id%u = %s\n", static_cast<unsigned>(i), identifier(i).string().utf8().data());
            ++i;
        } while (i != count);
    }
}

template<class Block>
void BytecodeDumper<Block>::dumpConstants(PrintStream& out)
{
    if (!block()->constantRegisters().isEmpty()) {
        out.printf("\nConstants:\n");
        size_t i = 0;
        for (const auto& constant : block()->constantRegisters()) {
            const char* sourceCodeRepresentationDescription = nullptr;
            switch (block()->constantsSourceCodeRepresentation()[i]) {
            case SourceCodeRepresentation::Double:
                sourceCodeRepresentationDescription = ": in source as double";
                break;
            case SourceCodeRepresentation::Integer:
                sourceCodeRepresentationDescription = ": in source as integer";
                break;
            case SourceCodeRepresentation::Other:
                sourceCodeRepresentationDescription = "";
                break;
            }
            out.printf("   k%u = %s%s\n", static_cast<unsigned>(i), toCString(constant.get()).data(), sourceCodeRepresentationDescription);
            ++i;
        }
    }
}

template<class Block>
void BytecodeDumper<Block>::dumpRegExps(PrintStream& out)
{
    if (size_t count = block()->numberOfRegExps()) {
        out.printf("\nm_regexps:\n");
        size_t i = 0;
        do {
            out.printf("  re%u = %s\n", static_cast<unsigned>(i), regexpToSourceString(block()->regexp(i)).data());
            ++i;
        } while (i < count);
    }
}

template<class Block>
void BytecodeDumper<Block>::dumpExceptionHandlers(PrintStream& out)
{
    if (unsigned count = block()->numberOfExceptionHandlers()) {
        out.printf("\nException Handlers:\n");
        unsigned i = 0;
        do {
            const auto& handler = block()->exceptionHandler(i);
            out.printf("\t %d: { start: [%4d] end: [%4d] target: [%4d] } %s\n", i + 1, handler.start, handler.end, handler.target, handler.typeName());
            ++i;
        } while (i < count);
    }
}

template<class Block>
void BytecodeDumper<Block>::dumpSwitchJumpTables(PrintStream& out)
{
    if (unsigned count = block()->numberOfSwitchJumpTables()) {
        out.printf("Switch Jump Tables:\n");
        unsigned i = 0;
        do {
            out.printf("  %1d = {\n", i);
            const auto& switchJumpTable = block()->switchJumpTable(i);
            int entry = 0;
            auto end = switchJumpTable.branchOffsets.end();
            for (auto iter = switchJumpTable.branchOffsets.begin(); iter != end; ++iter, ++entry) {
                if (!*iter)
                    continue;
                out.printf("\t\t%4d => %04d\n", entry + switchJumpTable.min, *iter);
            }
            out.printf("      }\n");
            ++i;
        } while (i < count);
    }
}

template<class Block>
void BytecodeDumper<Block>::dumpStringSwitchJumpTables(PrintStream& out)
{
    if (unsigned count = block()->numberOfStringSwitchJumpTables()) {
        out.printf("\nString Switch Jump Tables:\n");
        unsigned i = 0;
        do {
            out.printf("  %1d = {\n", i);
            const auto& stringSwitchJumpTable = block()->stringSwitchJumpTable(i);
            auto end = stringSwitchJumpTable.offsetTable.end();
            for (auto iter = stringSwitchJumpTable.offsetTable.begin(); iter != end; ++iter)
                out.printf("\t\t\"%s\" => %04d\n", iter->key->utf8().data(), iter->value.branchOffset);
            out.printf("      }\n");
            ++i;
        } while (i < count);
    }
}

template<class Block>
void BytecodeDumper<Block>::dumpBlock(Block* block, const typename Block::UnpackedInstructions& instructions, PrintStream& out, const StubInfoMap& stubInfos, const CallLinkInfoMap& callLinkInfos)
{
    VM& vm = *block->vm();
    size_t instructionCount = 0;

    for (size_t i = 0; i < instructions.size(); i += opcodeLengths[vm.interpreter->getOpcodeID(instructions[i])])
        ++instructionCount;

    out.print(*block);
    out.printf(
        ": %lu m_instructions; %lu bytes; %d parameter(s); %d callee register(s); %d variable(s)",
        static_cast<unsigned long>(instructions.size()),
        static_cast<unsigned long>(instructions.size() * sizeof(Instruction)),
        block->numParameters(), block->numCalleeLocals(), block->m_numVars);
    out.print("; scope at ", block->scopeRegister());
    out.printf("\n");

    const auto* begin = instructions.begin();
    const auto* end = instructions.end();
    BytecodeDumper<Block> dumper(block, begin);
    for (const auto* it = begin; it != end; ++it)
        dumper.dumpBytecode(out, begin, it, stubInfos, callLinkInfos);

    dumper.dumpIdentifiers(out);
    dumper.dumpConstants(out);
    dumper.dumpRegExps(out);
    dumper.dumpExceptionHandlers(out);
    dumper.dumpSwitchJumpTables(out);
    dumper.dumpStringSwitchJumpTables(out);

    out.printf("\n");
}

template class BytecodeDumper<UnlinkedCodeBlock>;
template class BytecodeDumper<CodeBlock>;

}
