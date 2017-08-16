/*
 * Copyright (C) 2011, 2016 Apple Inc. All rights reserved.
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
#include "LLIntData.h"

#include "ArithProfile.h"
#include "BytecodeConventions.h"
#include "CodeBlock.h"
#include "CodeType.h"
#include "InitializeThreading.h"
#include "Instruction.h"
#include "JSScope.h"
#include "LLIntCLoop.h"
#include "LLIntCommon.h"
#include "MaxFrameExtentForSlowPathCall.h"
#include "Opcode.h"
#include "PropertyOffset.h"
#include "ShadowChicken.h"
#include "WriteBarrier.h"
#include <string>
#include <wtf/NeverDestroyed.h>

#define STATIC_ASSERT(cond) static_assert(cond, "LLInt assumes " #cond)

namespace JSC { namespace LLInt {

Instruction* Data::s_exceptionInstructions = 0;
Opcode Data::s_opcodeMap[numOpcodeIDs] = { };
OpcodeStatsArray* Data::s_opcodeStatsArray = nullptr;

#if ENABLE(JIT)
extern "C" void llint_entry(void*);
#endif

void initialize()
{
    Data::s_exceptionInstructions = new Instruction[maxOpcodeLength + 1];

#if !ENABLE(JIT)
    CLoop::initialize();

#else // ENABLE(JIT)
    llint_entry(&Data::s_opcodeMap);

    for (int i = 0; i < maxOpcodeLength + 1; ++i)
        Data::s_exceptionInstructions[i].u.pointer =
            LLInt::getCodePtr(llint_throw_from_slow_path_trampoline);
#endif // ENABLE(JIT)

#if ENABLE(LLINT_STATS)
    Data::ensureStats();
#endif
}

#if COMPILER(CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif
void Data::performAssertions(VM& vm)
{
    UNUSED_PARAM(vm);
    
    // Assertions to match LowLevelInterpreter.asm.  If you change any of this code, be
    // prepared to change LowLevelInterpreter.asm as well!!

#if USE(JSVALUE64)
    const ptrdiff_t PtrSize = 8;
    const ptrdiff_t CallFrameHeaderSlots = 5;
#else // USE(JSVALUE64) // i.e. 32-bit version
    const ptrdiff_t PtrSize = 4;
    const ptrdiff_t CallFrameHeaderSlots = 4;
#endif
    const ptrdiff_t SlotSize = 8;

    STATIC_ASSERT(sizeof(void*) == PtrSize);
    STATIC_ASSERT(sizeof(Register) == SlotSize);
    STATIC_ASSERT(CallFrame::headerSizeInRegisters == CallFrameHeaderSlots);

    ASSERT(!CallFrame::callerFrameOffset());
    STATIC_ASSERT(CallerFrameAndPC::sizeInRegisters == (PtrSize * 2) / SlotSize);
    ASSERT(CallFrame::returnPCOffset() == CallFrame::callerFrameOffset() + PtrSize);
    ASSERT(CallFrameSlot::codeBlock * sizeof(Register) == CallFrame::returnPCOffset() + PtrSize);
    STATIC_ASSERT(CallFrameSlot::callee * sizeof(Register) == CallFrameSlot::codeBlock * sizeof(Register) + SlotSize);
    STATIC_ASSERT(CallFrameSlot::argumentCount * sizeof(Register) == CallFrameSlot::callee * sizeof(Register) + SlotSize);
    STATIC_ASSERT(CallFrameSlot::thisArgument * sizeof(Register) == CallFrameSlot::argumentCount * sizeof(Register) + SlotSize);
    STATIC_ASSERT(CallFrame::headerSizeInRegisters == CallFrameSlot::thisArgument);

    ASSERT(CallFrame::argumentOffsetIncludingThis(0) == CallFrameSlot::thisArgument);

#if CPU(BIG_ENDIAN)
    ASSERT(OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.tag) == 0);
    ASSERT(OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.payload) == 4);
#else
    ASSERT(OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.tag) == 4);
    ASSERT(OBJECT_OFFSETOF(EncodedValueDescriptor, asBits.payload) == 0);
#endif
#if USE(JSVALUE32_64)
    STATIC_ASSERT(JSValue::Int32Tag == static_cast<unsigned>(-1));
    STATIC_ASSERT(JSValue::BooleanTag == static_cast<unsigned>(-2));
    STATIC_ASSERT(JSValue::NullTag == static_cast<unsigned>(-3));
    STATIC_ASSERT(JSValue::UndefinedTag == static_cast<unsigned>(-4));
    STATIC_ASSERT(JSValue::CellTag == static_cast<unsigned>(-5));
    STATIC_ASSERT(JSValue::EmptyValueTag == static_cast<unsigned>(-6));
    STATIC_ASSERT(JSValue::DeletedValueTag == static_cast<unsigned>(-7));
    STATIC_ASSERT(JSValue::LowestTag == static_cast<unsigned>(-7));
#else
    STATIC_ASSERT(TagBitTypeOther == 0x2);
    STATIC_ASSERT(TagBitBool == 0x4);
    STATIC_ASSERT(TagBitUndefined == 0x8);
    STATIC_ASSERT(ValueEmpty == 0x0);
    STATIC_ASSERT(ValueFalse == (TagBitTypeOther | TagBitBool));
    STATIC_ASSERT(ValueTrue == (TagBitTypeOther | TagBitBool | 1));
    STATIC_ASSERT(ValueUndefined == (TagBitTypeOther | TagBitUndefined));
    STATIC_ASSERT(ValueNull == TagBitTypeOther);
#endif
#if (CPU(X86_64) && !OS(WINDOWS)) || CPU(ARM64) || !ENABLE(JIT)
    STATIC_ASSERT(!maxFrameExtentForSlowPathCall);
#elif CPU(ARM)
    STATIC_ASSERT(maxFrameExtentForSlowPathCall == 24);
#elif CPU(X86) || CPU(MIPS)
    STATIC_ASSERT(maxFrameExtentForSlowPathCall == 40);
#elif CPU(X86_64) && OS(WINDOWS)
    STATIC_ASSERT(maxFrameExtentForSlowPathCall == 64);
#endif

#if !ENABLE(JIT) || USE(JSVALUE32_64)
    ASSERT(!CodeBlock::llintBaselineCalleeSaveSpaceAsVirtualRegisters());
#elif (CPU(X86_64) && !OS(WINDOWS))  || CPU(ARM64)
    ASSERT(CodeBlock::llintBaselineCalleeSaveSpaceAsVirtualRegisters() == 3);
#elif (CPU(X86_64) && OS(WINDOWS))
    ASSERT(CodeBlock::llintBaselineCalleeSaveSpaceAsVirtualRegisters() == 3);
#endif
    
    STATIC_ASSERT(StringType == 6);
    STATIC_ASSERT(SymbolType == 7);
    STATIC_ASSERT(ObjectType == 23);
    STATIC_ASSERT(FinalObjectType == 24);
    STATIC_ASSERT(JSFunctionType == 26);
    STATIC_ASSERT(ArrayType == 34);
    STATIC_ASSERT(DerivedArrayType == 35);
    STATIC_ASSERT(ProxyObjectType == 53);
    STATIC_ASSERT(Int8ArrayType == 36);
    STATIC_ASSERT(Int16ArrayType == 37);
    STATIC_ASSERT(Int32ArrayType == 38);
    STATIC_ASSERT(Uint8ArrayType == 39);
    STATIC_ASSERT(Uint8ClampedArrayType == 40);
    STATIC_ASSERT(Uint16ArrayType == 41);
    STATIC_ASSERT(Uint32ArrayType == 42);
    STATIC_ASSERT(Float32ArrayType == 43);
    STATIC_ASSERT(Float64ArrayType == 44);
    STATIC_ASSERT(MasqueradesAsUndefined == 1);
    STATIC_ASSERT(ImplementsDefaultHasInstance == 2);
    STATIC_ASSERT(FirstConstantRegisterIndex == 0x40000000);
    STATIC_ASSERT(GlobalCode == 0);
    STATIC_ASSERT(EvalCode == 1);
    STATIC_ASSERT(FunctionCode == 2);
    STATIC_ASSERT(ModuleCode == 3);
    
    STATIC_ASSERT(IsArray == 0x01);
    STATIC_ASSERT(IndexingShapeMask == 0x0E);
    STATIC_ASSERT(NoIndexingShape == 0x00);
    STATIC_ASSERT(Int32Shape == 0x04);
    STATIC_ASSERT(DoubleShape == 0x06);
    STATIC_ASSERT(ContiguousShape == 0x08);
    STATIC_ASSERT(ArrayStorageShape == 0x0A);
    STATIC_ASSERT(SlowPutArrayStorageShape == 0x0C);

    ASSERT(!(reinterpret_cast<ptrdiff_t>((reinterpret_cast<WriteBarrier<JSCell>*>(0x4000)->slot())) - 0x4000));
    static_assert(PutByIdPrimaryTypeMask == 0x6, "LLInt assumes PutByIdPrimaryTypeMask is == 0x6");
    static_assert(PutByIdPrimaryTypeSecondary == 0x0, "LLInt assumes PutByIdPrimaryTypeSecondary is == 0x0");
    static_assert(PutByIdPrimaryTypeObjectWithStructure == 0x2, "LLInt assumes PutByIdPrimaryTypeObjectWithStructure is == 0x2");
    static_assert(PutByIdPrimaryTypeObjectWithStructureOrOther == 0x4, "LLInt assumes PutByIdPrimaryTypeObjectWithStructureOrOther is == 0x4");
    static_assert(PutByIdSecondaryTypeMask == -0x8, "LLInt assumes PutByIdSecondaryTypeMask is == -0x8");
    static_assert(PutByIdSecondaryTypeBottom == 0x0, "LLInt assumes PutByIdSecondaryTypeBottom is == 0x0");
    static_assert(PutByIdSecondaryTypeBoolean == 0x8, "LLInt assumes PutByIdSecondaryTypeBoolean is == 0x8");
    static_assert(PutByIdSecondaryTypeOther == 0x10, "LLInt assumes PutByIdSecondaryTypeOther is == 0x10");
    static_assert(PutByIdSecondaryTypeInt32 == 0x18, "LLInt assumes PutByIdSecondaryTypeInt32 is == 0x18");
    static_assert(PutByIdSecondaryTypeNumber == 0x20, "LLInt assumes PutByIdSecondaryTypeNumber is == 0x20");
    static_assert(PutByIdSecondaryTypeString == 0x28, "LLInt assumes PutByIdSecondaryTypeString is == 0x28");
    static_assert(PutByIdSecondaryTypeSymbol == 0x30, "LLInt assumes PutByIdSecondaryTypeSymbol is == 0x30");
    static_assert(PutByIdSecondaryTypeObject == 0x38, "LLInt assumes PutByIdSecondaryTypeObject is == 0x38");
    static_assert(PutByIdSecondaryTypeObjectOrOther == 0x40, "LLInt assumes PutByIdSecondaryTypeObjectOrOther is == 0x40");
    static_assert(PutByIdSecondaryTypeTop == 0x48, "LLInt assumes PutByIdSecondaryTypeTop is == 0x48");

    static_assert(GlobalProperty == 0, "LLInt assumes GlobalProperty ResultType is == 0");
    static_assert(GlobalVar == 1, "LLInt assumes GlobalVar ResultType is == 1");
    static_assert(GlobalLexicalVar == 2, "LLInt assumes GlobalLexicalVar ResultType is == 2");
    static_assert(ClosureVar == 3, "LLInt assumes ClosureVar ResultType is == 3");
    static_assert(LocalClosureVar == 4, "LLInt assumes LocalClosureVar ResultType is == 4");
    static_assert(ModuleVar == 5, "LLInt assumes ModuleVar ResultType is == 5");
    static_assert(GlobalPropertyWithVarInjectionChecks == 6, "LLInt assumes GlobalPropertyWithVarInjectionChecks ResultType is == 6");
    static_assert(GlobalVarWithVarInjectionChecks == 7, "LLInt assumes GlobalVarWithVarInjectionChecks ResultType is == 7");
    static_assert(GlobalLexicalVarWithVarInjectionChecks == 8, "LLInt assumes GlobalLexicalVarWithVarInjectionChecks ResultType is == 8");
    static_assert(ClosureVarWithVarInjectionChecks == 9, "LLInt assumes ClosureVarWithVarInjectionChecks ResultType is == 9");

    static_assert(static_cast<unsigned>(InitializationMode::NotInitialization) == 2, "LLInt assumes that InitializationMode::NotInitialization is 0");
    
    STATIC_ASSERT(GetPutInfo::typeBits == 0x3ff);
    STATIC_ASSERT(GetPutInfo::initializationShift == 10);
    STATIC_ASSERT(GetPutInfo::initializationBits == 0xffc00);

    STATIC_ASSERT(MarkedBlock::blockSize == 16 * 1024);
    STATIC_ASSERT(blackThreshold == 0);

    ASSERT(bitwise_cast<uintptr_t>(ShadowChicken::Packet::tailMarker()) == static_cast<uintptr_t>(0x7a11));

    // FIXME: make these assertions less horrible.
#if !ASSERT_DISABLED
    Vector<int> testVector;
    testVector.resize(42);
    ASSERT(bitwise_cast<uint32_t*>(&testVector)[sizeof(void*)/sizeof(uint32_t) + 1] == 42);
    ASSERT(bitwise_cast<int**>(&testVector)[0] == testVector.begin());
#endif

    ASSERT(StringImpl::s_hashFlag8BitBuffer == 8);

    {
        uint32_t bits = 0x120000;
        UNUSED_PARAM(bits);
        ArithProfile arithProfile;
        arithProfile.lhsSawInt32();
        arithProfile.rhsSawInt32();
        ASSERT(arithProfile.bits() == bits);
        ASSERT(ArithProfile::fromInt(bits).lhsObservedType().isOnlyInt32());
        ASSERT(ArithProfile::fromInt(bits).rhsObservedType().isOnlyInt32());
    }
    {
        uint32_t bits = 0x220000;
        UNUSED_PARAM(bits);
        ArithProfile arithProfile;
        arithProfile.lhsSawNumber();
        arithProfile.rhsSawInt32();
        ASSERT(arithProfile.bits() == bits);
        ASSERT(ArithProfile::fromInt(bits).lhsObservedType().isOnlyNumber());
        ASSERT(ArithProfile::fromInt(bits).rhsObservedType().isOnlyInt32());
    }
    {
        uint32_t bits = 0x240000;
        UNUSED_PARAM(bits);
        ArithProfile arithProfile;
        arithProfile.lhsSawNumber();
        arithProfile.rhsSawNumber();
        ASSERT(arithProfile.bits() == bits);
        ASSERT(ArithProfile::fromInt(bits).lhsObservedType().isOnlyNumber());
        ASSERT(ArithProfile::fromInt(bits).rhsObservedType().isOnlyNumber());
    }
    {
        uint32_t bits = 0x140000;
        UNUSED_PARAM(bits);
        ArithProfile arithProfile;
        arithProfile.lhsSawInt32();
        arithProfile.rhsSawNumber();
        ASSERT(arithProfile.bits() == bits);
        ASSERT(ArithProfile::fromInt(bits).lhsObservedType().isOnlyInt32());
        ASSERT(ArithProfile::fromInt(bits).rhsObservedType().isOnlyNumber());
    }
}
#if COMPILER(CLANG)
#pragma clang diagnostic pop
#endif

void Data::finalizeStats()
{
#if ENABLE(LLINT_STATS)
    if (!Options::reportLLIntStats())
        return;
    
    if (Options::llintStatsFile())
        saveStats();
    
    dumpStats();
#endif
}

#if ENABLE(LLINT_STATS)
static const bool verboseStats = false;

static bool compareStats(const OpcodeStats& a, const OpcodeStats& b)
{
    if (a.count > b.count)
        return true;
    if (a.count < b.count)
        return false;
    return a.slowPathCount > b.slowPathCount;
}

void Data::dumpStats()
{
    ASSERT(Options::reportLLIntStats());
    auto statsCopy = *s_opcodeStatsArray;
    std::sort(statsCopy.begin(), statsCopy.end(), compareStats);

    dataLog("Opcode stats:\n");
    unsigned i = 0;
    for (auto& stats : statsCopy) {
        if (stats.count || stats.slowPathCount)
            dataLog("   [", i++, "]: fast:", stats.count, " slow:", stats.slowPathCount, " ", opcodeNames[stats.id], "\n");
    }
}

void Data::ensureStats()
{
    static std::once_flag initializeOptionsOnceFlag;
    std::call_once(initializeOptionsOnceFlag, [] {
        s_opcodeStatsArray = new OpcodeStatsArray();
        resetStats();
    });
}

void Data::loadStats()
{
    static NeverDestroyed<std::string> installedStatsFile;
    if (!Options::llintStatsFile() || !installedStatsFile.get().compare(Options::llintStatsFile()))
        return;

    Options::reportLLIntStats() = true; // Force stats collection.
    installedStatsFile.get() = Options::llintStatsFile();

    ensureStats();

    const char* filename = Options::llintStatsFile();
    FILE* file = fopen(filename, "r");
    if (!file) {
        dataLogF("Failed to open file %s. Did you add the file-read-write-data entitlement to WebProcess.sb?\n", filename);
        return;
    }

    resetStats();

    OpcodeStats loaded;
    unsigned index;
    char opcodeName[100];
    while (fscanf(file, "[%u]: fast:%zu slow:%zu id:%u %s\n", &index, &loaded.count, &loaded.slowPathCount, &loaded.id, opcodeName) != EOF) {
        if (verboseStats)
            dataLogF("loaded [%u]: fast %zu slow %zu id:%u %s\n", index, loaded.count, loaded.slowPathCount, loaded.id, opcodeName);

        OpcodeStats& stats = opcodeStats(loaded.id);
        stats.count = loaded.count;
        stats.slowPathCount = loaded.slowPathCount;
    }

    if (verboseStats) {
        dataLogF("After loading from %s, ", filename);
        dumpStats();
    }

    int result = fclose(file);
    if (result)
        dataLogF("Failed to close file %s: %s\n", filename, strerror(errno));
}

void Data::resetStats()
{
    unsigned i = 0;
    for (auto& stats : *s_opcodeStatsArray) {
        stats.id = static_cast<OpcodeID>(i++);
        stats.count = 0;
        stats.slowPathCount = 0;
    }
}

void Data::saveStats()
{
    ASSERT(Options::reportLLIntStats() && Options::llintStatsFile());
    const char* filename = Options::llintStatsFile();

    FILE* file = fopen(filename, "w");
    if (!file) {
        dataLogF("Failed to open file %s. Did you add the file-read-write-data entitlement to WebProcess.sb?\n", filename);
        return;
    }

    auto statsCopy = *s_opcodeStatsArray;
    std::sort(statsCopy.begin(), statsCopy.end(), compareStats);

    int index = 0;
    for (auto& stats : statsCopy) {
        if (!stats.count && !stats.slowPathCount)
            break; // stats are sorted. If we encountered 0 counts, then there are no more non-zero counts.

        if (verboseStats)
            dataLogF("saved [%u]: fast:%zu slow:%zu id:%u %s\n", index, stats.count, stats.slowPathCount, stats.id, opcodeNames[stats.id]);

        fprintf(file, "[%u]: fast:%zu slow:%zu id:%u %s\n", index, stats.count, stats.slowPathCount, stats.id, opcodeNames[stats.id]);
        index++;
    }

    int result = fclose(file);
    if (result)
        dataLogF("Failed to close file %s: %s\n", filename, strerror(errno));
}
#endif

} } // namespace JSC::LLInt
