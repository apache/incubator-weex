/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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

#include "AirCode.h"
#include "AirGenerate.h"
#include "AirInstInlines.h"
#include "AllowMacroScratchRegisterUsage.h"
#include "B3Compilation.h"
#include "B3Procedure.h"
#include "CCallHelpers.h"
#include "InitializeThreading.h"
#include "JSCInlines.h"
#include "LinkBuffer.h"
#include "PureNaN.h"
#include "VM.h"
#include <cmath>
#include <map>
#include <string>
#include <wtf/Lock.h>
#include <wtf/NumberOfCores.h>
#include <wtf/Threading.h>

// We don't have a NO_RETURN_DUE_TO_EXIT, nor should we. That's ridiculous.
static bool hiddenTruthBecauseNoReturnIsStupid() { return true; }

static void usage()
{
    dataLog("Usage: testb3 [<filter>]\n");
    if (hiddenTruthBecauseNoReturnIsStupid())
        exit(1);
}

#if ENABLE(B3_JIT)

using namespace JSC;
using namespace JSC::B3::Air;

using JSC::B3::FP;
using JSC::B3::GP;
using JSC::B3::Width;
using JSC::B3::Width8;
using JSC::B3::Width16;
using JSC::B3::Width32;
using JSC::B3::Width64;

namespace {

StaticLock crashLock;

// Nothing fancy for now; we just use the existing WTF assertion machinery.
#define CHECK(x) do {                                                   \
        if (!!(x))                                                      \
            break;                                                      \
        crashLock.lock();                                               \
        WTFReportAssertionFailure(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, #x); \
        CRASH();                                                        \
    } while (false)

VM* vm;

std::unique_ptr<B3::Compilation> compile(B3::Procedure& proc)
{
    prepareForGeneration(proc.code());
    CCallHelpers jit(vm);
    generate(proc.code(), jit);
    LinkBuffer linkBuffer(*vm, jit, nullptr);

    return std::make_unique<B3::Compilation>(
        FINALIZE_CODE(linkBuffer, ("testair compilation")), proc.releaseByproducts());
}

template<typename T, typename... Arguments>
T invoke(const B3::Compilation& code, Arguments... arguments)
{
    T (*function)(Arguments...) = bitwise_cast<T(*)(Arguments...)>(code.code().executableAddress());
    return function(arguments...);
}

template<typename T, typename... Arguments>
T compileAndRun(B3::Procedure& procedure, Arguments... arguments)
{
    return invoke<T>(*compile(procedure), arguments...);
}

void testSimple()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(Move, nullptr, Arg::imm(42), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    CHECK(compileAndRun<int>(proc) == 42);
}

// Use this to put a constant into a register without Air being able to see the constant.
template<typename T>
void loadConstantImpl(BasicBlock* block, T value, B3::Air::Opcode move, Tmp tmp, Tmp scratch)
{
    static StaticLock lock;
    static std::map<T, T*>* map; // I'm not messing with HashMap's problems with integers.

    LockHolder locker(lock);
    if (!map)
        map = new std::map<T, T*>();

    if (!map->count(value))
        (*map)[value] = new T(value);

    T* ptr = (*map)[value];
    block->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(ptr)), scratch);
    block->append(move, nullptr, Arg::addr(scratch), tmp);
}

void loadConstant(BasicBlock* block, intptr_t value, Tmp tmp)
{
    loadConstantImpl<intptr_t>(block, value, Move, tmp, tmp);
}

void loadDoubleConstant(BasicBlock* block, double value, Tmp tmp, Tmp scratch)
{
    loadConstantImpl<double>(block, value, MoveDouble, tmp, scratch);
}

void testShuffleSimpleSwap()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT3), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT2), Arg::widthArg(Width32));

    int32_t things[4];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 2);
    CHECK(things[2] == 4);
    CHECK(things[3] == 3);
}

void testShuffleSimpleShift()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT3), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT4), Arg::widthArg(Width32));

    int32_t things[5];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 2);
    CHECK(things[2] == 3);
    CHECK(things[3] == 3);
    CHECK(things[4] == 4);
}

void testShuffleLongShift()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    loadConstant(root, 5, Tmp(GPRInfo::regT4));
    loadConstant(root, 6, Tmp(GPRInfo::regT5));
    loadConstant(root, 7, Tmp(GPRInfo::regT6));
    loadConstant(root, 8, Tmp(GPRInfo::regT7));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT2), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT3), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT4), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT4), Tmp(GPRInfo::regT5), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT5), Tmp(GPRInfo::regT6), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT6), Tmp(GPRInfo::regT7), Arg::widthArg(Width32));

    int32_t things[8];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT5), Arg::addr(base, 5 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT6), Arg::addr(base, 6 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT7), Arg::addr(base, 7 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 1);
    CHECK(things[2] == 2);
    CHECK(things[3] == 3);
    CHECK(things[4] == 4);
    CHECK(things[5] == 5);
    CHECK(things[6] == 6);
    CHECK(things[7] == 7);
}

void testShuffleLongShiftBackwards()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    loadConstant(root, 5, Tmp(GPRInfo::regT4));
    loadConstant(root, 6, Tmp(GPRInfo::regT5));
    loadConstant(root, 7, Tmp(GPRInfo::regT6));
    loadConstant(root, 8, Tmp(GPRInfo::regT7));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT6), Tmp(GPRInfo::regT7), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT5), Tmp(GPRInfo::regT6), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT4), Tmp(GPRInfo::regT5), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT4), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT3), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT2), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32));

    int32_t things[8];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT5), Arg::addr(base, 5 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT6), Arg::addr(base, 6 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT7), Arg::addr(base, 7 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 1);
    CHECK(things[2] == 2);
    CHECK(things[3] == 3);
    CHECK(things[4] == 4);
    CHECK(things[5] == 5);
    CHECK(things[6] == 6);
    CHECK(things[7] == 7);
}

void testShuffleSimpleRotate()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT2), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT0), Arg::widthArg(Width32));

    int32_t things[4];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 3);
    CHECK(things[1] == 1);
    CHECK(things[2] == 2);
    CHECK(things[3] == 4);
}

void testShuffleSimpleBroadcast()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT2), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT3), Arg::widthArg(Width32));

    int32_t things[4];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 1);
    CHECK(things[2] == 1);
    CHECK(things[3] == 1);
}

void testShuffleBroadcastAllRegs()
{
    B3::Procedure proc;
    Code& code = proc.code();

    const Vector<Reg>& regs = code.regsInPriorityOrder(GP);

    BasicBlock* root = code.addBlock();
    root->append(Move, nullptr, Arg::imm(35), Tmp(GPRInfo::regT0));
    unsigned count = 1;
    for (Reg reg : regs) {
        if (reg != Reg(GPRInfo::regT0))
            loadConstant(root, count++, Tmp(reg));
    }
    Inst& shuffle = root->append(Shuffle, nullptr);
    for (Reg reg : regs) {
        if (reg != Reg(GPRInfo::regT0))
            shuffle.append(Tmp(GPRInfo::regT0), Tmp(reg), Arg::widthArg(Width32));
    }

    StackSlot* slot = code.addStackSlot(sizeof(int32_t) * regs.size(), StackSlotKind::Locked);
    for (unsigned i = 0; i < regs.size(); ++i)
        root->append(Move32, nullptr, Tmp(regs[i]), Arg::stack(slot, i * sizeof(int32_t)));

    Vector<int32_t> things(regs.size(), 666);
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things[0])), base);
    for (unsigned i = 0; i < regs.size(); ++i) {
        root->append(Move32, nullptr, Arg::stack(slot, i * sizeof(int32_t)), Tmp(GPRInfo::regT0));
        root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, i * sizeof(int32_t)));
    }
    
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    CHECK(!compileAndRun<int>(proc));

    for (int32_t thing : things)
        CHECK(thing == 35);
}

void testShuffleTreeShift()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    loadConstant(root, 5, Tmp(GPRInfo::regT4));
    loadConstant(root, 6, Tmp(GPRInfo::regT5));
    loadConstant(root, 7, Tmp(GPRInfo::regT6));
    loadConstant(root, 8, Tmp(GPRInfo::regT7));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT2), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT3), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT4), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT5), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT6), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT7), Arg::widthArg(Width32));

    int32_t things[8];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT5), Arg::addr(base, 5 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT6), Arg::addr(base, 6 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT7), Arg::addr(base, 7 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 1);
    CHECK(things[2] == 1);
    CHECK(things[3] == 2);
    CHECK(things[4] == 2);
    CHECK(things[5] == 3);
    CHECK(things[6] == 3);
    CHECK(things[7] == 4);
}

void testShuffleTreeShiftBackward()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    loadConstant(root, 5, Tmp(GPRInfo::regT4));
    loadConstant(root, 6, Tmp(GPRInfo::regT5));
    loadConstant(root, 7, Tmp(GPRInfo::regT6));
    loadConstant(root, 8, Tmp(GPRInfo::regT7));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT7), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT6), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT5), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT4), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT3), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT2), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32));

    int32_t things[8];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT5), Arg::addr(base, 5 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT6), Arg::addr(base, 6 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT7), Arg::addr(base, 7 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 1);
    CHECK(things[2] == 1);
    CHECK(things[3] == 2);
    CHECK(things[4] == 2);
    CHECK(things[5] == 3);
    CHECK(things[6] == 3);
    CHECK(things[7] == 4);
}

void testShuffleTreeShiftOtherBackward()
{
    // NOTE: This test was my original attempt at TreeShiftBackward but mistakes were made. So, this
    // ends up being just a weird test. But weird tests are useful, so I kept it.
    
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    loadConstant(root, 5, Tmp(GPRInfo::regT4));
    loadConstant(root, 6, Tmp(GPRInfo::regT5));
    loadConstant(root, 7, Tmp(GPRInfo::regT6));
    loadConstant(root, 8, Tmp(GPRInfo::regT7));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT4), Tmp(GPRInfo::regT7), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT5), Tmp(GPRInfo::regT6), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT5), Tmp(GPRInfo::regT5), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT6), Tmp(GPRInfo::regT4), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT6), Tmp(GPRInfo::regT3), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT7), Tmp(GPRInfo::regT2), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT7), Tmp(GPRInfo::regT1), Arg::widthArg(Width32));

    int32_t things[8];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT5), Arg::addr(base, 5 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT6), Arg::addr(base, 6 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT7), Arg::addr(base, 7 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 8);
    CHECK(things[2] == 8);
    CHECK(things[3] == 7);
    CHECK(things[4] == 7);
    CHECK(things[5] == 6);
    CHECK(things[6] == 6);
    CHECK(things[7] == 5);
}

void testShuffleMultipleShifts()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    loadConstant(root, 5, Tmp(GPRInfo::regT4));
    loadConstant(root, 6, Tmp(GPRInfo::regT5));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT3), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT4), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT5), Arg::widthArg(Width32));

    int32_t things[6];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT5), Arg::addr(base, 5 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 1);
    CHECK(things[2] == 3);
    CHECK(things[3] == 3);
    CHECK(things[4] == 3);
    CHECK(things[5] == 1);
}

void testShuffleRotateWithFringe()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    loadConstant(root, 5, Tmp(GPRInfo::regT4));
    loadConstant(root, 6, Tmp(GPRInfo::regT5));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT2), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT0), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT3), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT4), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT5), Arg::widthArg(Width32));

    int32_t things[6];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT5), Arg::addr(base, 5 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 3);
    CHECK(things[1] == 1);
    CHECK(things[2] == 2);
    CHECK(things[3] == 1);
    CHECK(things[4] == 2);
    CHECK(things[5] == 3);
}

void testShuffleRotateWithFringeInWeirdOrder()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    loadConstant(root, 5, Tmp(GPRInfo::regT4));
    loadConstant(root, 6, Tmp(GPRInfo::regT5));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT3), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT4), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT0), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT5), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT2), Arg::widthArg(Width32));

    int32_t things[6];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT5), Arg::addr(base, 5 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 3);
    CHECK(things[1] == 1);
    CHECK(things[2] == 2);
    CHECK(things[3] == 1);
    CHECK(things[4] == 2);
    CHECK(things[5] == 3);
}

void testShuffleRotateWithLongFringe()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    loadConstant(root, 5, Tmp(GPRInfo::regT4));
    loadConstant(root, 6, Tmp(GPRInfo::regT5));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT2), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT0), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT3), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT4), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT4), Tmp(GPRInfo::regT5), Arg::widthArg(Width32));

    int32_t things[6];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT5), Arg::addr(base, 5 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 3);
    CHECK(things[1] == 1);
    CHECK(things[2] == 2);
    CHECK(things[3] == 1);
    CHECK(things[4] == 4);
    CHECK(things[5] == 5);
}

void testShuffleMultipleRotates()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    loadConstant(root, 5, Tmp(GPRInfo::regT4));
    loadConstant(root, 6, Tmp(GPRInfo::regT5));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT2), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT0), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT4), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT4), Tmp(GPRInfo::regT5), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT5), Tmp(GPRInfo::regT3), Arg::widthArg(Width32));

    int32_t things[6];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT5), Arg::addr(base, 5 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 3);
    CHECK(things[1] == 1);
    CHECK(things[2] == 2);
    CHECK(things[3] == 6);
    CHECK(things[4] == 4);
    CHECK(things[5] == 5);
}

void testShuffleShiftAndRotate()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    loadConstant(root, 4, Tmp(GPRInfo::regT3));
    loadConstant(root, 5, Tmp(GPRInfo::regT4));
    loadConstant(root, 6, Tmp(GPRInfo::regT5));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT1), Tmp(GPRInfo::regT2), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT0), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT4), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT4), Tmp(GPRInfo::regT5), Arg::widthArg(Width32));

    int32_t things[6];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT5), Arg::addr(base, 5 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 3);
    CHECK(things[1] == 1);
    CHECK(things[2] == 2);
    CHECK(things[3] == 4);
    CHECK(things[4] == 4);
    CHECK(things[5] == 5);
}

void testShuffleShiftAllRegs()
{
    B3::Procedure proc;
    Code& code = proc.code();

    const Vector<Reg>& regs = code.regsInPriorityOrder(GP);

    BasicBlock* root = code.addBlock();
    for (unsigned i = 0; i < regs.size(); ++i)
        loadConstant(root, 35 + i, Tmp(regs[i]));
    Inst& shuffle = root->append(Shuffle, nullptr);
    for (unsigned i = 1; i < regs.size(); ++i)
        shuffle.append(Tmp(regs[i - 1]), Tmp(regs[i]), Arg::widthArg(Width32));

    StackSlot* slot = code.addStackSlot(sizeof(int32_t) * regs.size(), StackSlotKind::Locked);
    for (unsigned i = 0; i < regs.size(); ++i)
        root->append(Move32, nullptr, Tmp(regs[i]), Arg::stack(slot, i * sizeof(int32_t)));

    Vector<int32_t> things(regs.size(), 666);
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things[0])), base);
    for (unsigned i = 0; i < regs.size(); ++i) {
        root->append(Move32, nullptr, Arg::stack(slot, i * sizeof(int32_t)), Tmp(GPRInfo::regT0));
        root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, i * sizeof(int32_t)));
    }
    
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 35);
    for (unsigned i = 1; i < regs.size(); ++i)
        CHECK(things[i] == 35 + static_cast<int32_t>(i) - 1);
}

void testShuffleRotateAllRegs()
{
    B3::Procedure proc;
    Code& code = proc.code();

    const Vector<Reg>& regs = code.regsInPriorityOrder(GP);

    BasicBlock* root = code.addBlock();
    for (unsigned i = 0; i < regs.size(); ++i)
        loadConstant(root, 35 + i, Tmp(regs[i]));
    Inst& shuffle = root->append(Shuffle, nullptr);
    for (unsigned i = 1; i < regs.size(); ++i)
        shuffle.append(Tmp(regs[i - 1]), Tmp(regs[i]), Arg::widthArg(Width32));
    shuffle.append(Tmp(regs.last()), Tmp(regs[0]), Arg::widthArg(Width32));

    StackSlot* slot = code.addStackSlot(sizeof(int32_t) * regs.size(), StackSlotKind::Locked);
    for (unsigned i = 0; i < regs.size(); ++i)
        root->append(Move32, nullptr, Tmp(regs[i]), Arg::stack(slot, i * sizeof(int32_t)));

    Vector<int32_t> things(regs.size(), 666);
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things[0])), base);
    for (unsigned i = 0; i < regs.size(); ++i) {
        root->append(Move32, nullptr, Arg::stack(slot, i * sizeof(int32_t)), Tmp(GPRInfo::regT0));
        root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, i * sizeof(int32_t)));
    }
    
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 35 + static_cast<int32_t>(regs.size()) - 1);
    for (unsigned i = 1; i < regs.size(); ++i)
        CHECK(things[i] == 35 + static_cast<int32_t>(i) - 1);
}

void testShuffleSimpleSwap64()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 10000000000000000ll, Tmp(GPRInfo::regT0));
    loadConstant(root, 20000000000000000ll, Tmp(GPRInfo::regT1));
    loadConstant(root, 30000000000000000ll, Tmp(GPRInfo::regT2));
    loadConstant(root, 40000000000000000ll, Tmp(GPRInfo::regT3));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT3), Arg::widthArg(Width64),
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT2), Arg::widthArg(Width64));

    int64_t things[4];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int64_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 10000000000000000ll);
    CHECK(things[1] == 20000000000000000ll);
    CHECK(things[2] == 40000000000000000ll);
    CHECK(things[3] == 30000000000000000ll);
}

void testShuffleSimpleShift64()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 10000000000000000ll, Tmp(GPRInfo::regT0));
    loadConstant(root, 20000000000000000ll, Tmp(GPRInfo::regT1));
    loadConstant(root, 30000000000000000ll, Tmp(GPRInfo::regT2));
    loadConstant(root, 40000000000000000ll, Tmp(GPRInfo::regT3));
    loadConstant(root, 50000000000000000ll, Tmp(GPRInfo::regT4));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT3), Arg::widthArg(Width64),
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT4), Arg::widthArg(Width64));

    int64_t things[5];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int64_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 10000000000000000ll);
    CHECK(things[1] == 20000000000000000ll);
    CHECK(things[2] == 30000000000000000ll);
    CHECK(things[3] == 30000000000000000ll);
    CHECK(things[4] == 40000000000000000ll);
}

void testShuffleSwapMixedWidth()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 10000000000000000ll, Tmp(GPRInfo::regT0));
    loadConstant(root, 20000000000000000ll, Tmp(GPRInfo::regT1));
    loadConstant(root, 30000000000000000ll, Tmp(GPRInfo::regT2));
    loadConstant(root, 40000000000000000ll, Tmp(GPRInfo::regT3));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT3), Arg::widthArg(Width32),
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT2), Arg::widthArg(Width64));

    int64_t things[4];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int64_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 10000000000000000ll);
    CHECK(things[1] == 20000000000000000ll);
    CHECK(things[2] == 40000000000000000ll);
    CHECK(things[3] == static_cast<uint32_t>(30000000000000000ll));
}

void testShuffleShiftMixedWidth()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadConstant(root, 10000000000000000ll, Tmp(GPRInfo::regT0));
    loadConstant(root, 20000000000000000ll, Tmp(GPRInfo::regT1));
    loadConstant(root, 30000000000000000ll, Tmp(GPRInfo::regT2));
    loadConstant(root, 40000000000000000ll, Tmp(GPRInfo::regT3));
    loadConstant(root, 50000000000000000ll, Tmp(GPRInfo::regT4));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT2), Tmp(GPRInfo::regT3), Arg::widthArg(Width64),
        Tmp(GPRInfo::regT3), Tmp(GPRInfo::regT4), Arg::widthArg(Width32));

    int64_t things[5];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT3), Arg::addr(base, 3 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT4), Arg::addr(base, 4 * sizeof(int64_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 10000000000000000ll);
    CHECK(things[1] == 20000000000000000ll);
    CHECK(things[2] == 30000000000000000ll);
    CHECK(things[3] == 30000000000000000ll);
    CHECK(things[4] == static_cast<uint32_t>(40000000000000000ll));
}

void testShuffleShiftMemory()
{
    B3::Procedure proc;
    Code& code = proc.code();

    int32_t memory[2];
    memory[0] = 35;
    memory[1] = 36;

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    root->append(Move, nullptr, Arg::immPtr(&memory), Tmp(GPRInfo::regT2));
    root->append(
        Shuffle, nullptr,
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),
        Arg::addr(Tmp(GPRInfo::regT2), 0 * sizeof(int32_t)),
        Arg::addr(Tmp(GPRInfo::regT2), 1 * sizeof(int32_t)), Arg::widthArg(Width32));

    int32_t things[2];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 1);
    CHECK(memory[0] == 35);
    CHECK(memory[1] == 35);
}

void testShuffleShiftMemoryLong()
{
    B3::Procedure proc;
    Code& code = proc.code();

    int32_t memory[2];
    memory[0] = 35;
    memory[1] = 36;

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    loadConstant(root, 3, Tmp(GPRInfo::regT2));
    root->append(Move, nullptr, Arg::immPtr(&memory), Tmp(GPRInfo::regT3));
    root->append(
        Shuffle, nullptr,
        
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),
        
        Tmp(GPRInfo::regT1), Arg::addr(Tmp(GPRInfo::regT3), 0 * sizeof(int32_t)),
        Arg::widthArg(Width32),
        
        Arg::addr(Tmp(GPRInfo::regT3), 0 * sizeof(int32_t)),
        Arg::addr(Tmp(GPRInfo::regT3), 1 * sizeof(int32_t)), Arg::widthArg(Width32),

        Arg::addr(Tmp(GPRInfo::regT3), 1 * sizeof(int32_t)), Tmp(GPRInfo::regT2),
        Arg::widthArg(Width32));

    int32_t things[3];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT2), Arg::addr(base, 2 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 1);
    CHECK(things[2] == 36);
    CHECK(memory[0] == 2);
    CHECK(memory[1] == 35);
}

void testShuffleShiftMemoryAllRegs()
{
    B3::Procedure proc;
    Code& code = proc.code();

    int32_t memory[2];
    memory[0] = 35;
    memory[1] = 36;

    Vector<Reg> regs = code.regsInPriorityOrder(GP);
    regs.removeFirst(Reg(GPRInfo::regT0));

    BasicBlock* root = code.addBlock();
    for (unsigned i = 0; i < regs.size(); ++i)
        loadConstant(root, i + 1, Tmp(regs[i]));
    root->append(Move, nullptr, Arg::immPtr(&memory), Tmp(GPRInfo::regT0));
    Inst& shuffle = root->append(
        Shuffle, nullptr,
        
        Tmp(regs[0]), Arg::addr(Tmp(GPRInfo::regT0), 0 * sizeof(int32_t)),
        Arg::widthArg(Width32),
        
        Arg::addr(Tmp(GPRInfo::regT0), 0 * sizeof(int32_t)),
        Arg::addr(Tmp(GPRInfo::regT0), 1 * sizeof(int32_t)), Arg::widthArg(Width32),

        Arg::addr(Tmp(GPRInfo::regT0), 1 * sizeof(int32_t)), Tmp(regs[1]),
        Arg::widthArg(Width32));

    for (unsigned i = 2; i < regs.size(); ++i)
        shuffle.append(Tmp(regs[i - 1]), Tmp(regs[i]), Arg::widthArg(Width32));

    Vector<int32_t> things(regs.size(), 666);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things[0])), Tmp(GPRInfo::regT0));
    for (unsigned i = 0; i < regs.size(); ++i) {
        root->append(
            Move32, nullptr, Tmp(regs[i]), Arg::addr(Tmp(GPRInfo::regT0), i * sizeof(int32_t)));
    }
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 36);
    for (unsigned i = 2; i < regs.size(); ++i)
        CHECK(things[i] == static_cast<int32_t>(i));
    CHECK(memory[0] == 1);
    CHECK(memory[1] == 35);
}

void testShuffleShiftMemoryAllRegs64()
{
    B3::Procedure proc;
    Code& code = proc.code();

    int64_t memory[2];
    memory[0] = 35000000000000ll;
    memory[1] = 36000000000000ll;

    Vector<Reg> regs = code.regsInPriorityOrder(GP);
    regs.removeFirst(Reg(GPRInfo::regT0));

    BasicBlock* root = code.addBlock();
    for (unsigned i = 0; i < regs.size(); ++i)
        loadConstant(root, (i + 1) * 1000000000000ll, Tmp(regs[i]));
    root->append(Move, nullptr, Arg::immPtr(&memory), Tmp(GPRInfo::regT0));
    Inst& shuffle = root->append(
        Shuffle, nullptr,
        
        Tmp(regs[0]), Arg::addr(Tmp(GPRInfo::regT0), 0 * sizeof(int64_t)),
        Arg::widthArg(Width64),
        
        Arg::addr(Tmp(GPRInfo::regT0), 0 * sizeof(int64_t)),
        Arg::addr(Tmp(GPRInfo::regT0), 1 * sizeof(int64_t)), Arg::widthArg(Width64),

        Arg::addr(Tmp(GPRInfo::regT0), 1 * sizeof(int64_t)), Tmp(regs[1]),
        Arg::widthArg(Width64));

    for (unsigned i = 2; i < regs.size(); ++i)
        shuffle.append(Tmp(regs[i - 1]), Tmp(regs[i]), Arg::widthArg(Width64));

    Vector<int64_t> things(regs.size(), 666);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things[0])), Tmp(GPRInfo::regT0));
    for (unsigned i = 0; i < regs.size(); ++i) {
        root->append(
            Move, nullptr, Tmp(regs[i]), Arg::addr(Tmp(GPRInfo::regT0), i * sizeof(int64_t)));
    }
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1000000000000ll);
    CHECK(things[1] == 36000000000000ll);
    for (unsigned i = 2; i < regs.size(); ++i)
        CHECK(things[i] == static_cast<int64_t>(i) * 1000000000000ll);
    CHECK(memory[0] == 1000000000000ll);
    CHECK(memory[1] == 35000000000000ll);
}

int64_t combineHiLo(int64_t high, int64_t low)
{
    union {
        int64_t value;
        int32_t halves[2];
    } u;
    u.value = high;
    u.halves[0] = static_cast<int32_t>(low);
    return u.value;
}

void testShuffleShiftMemoryAllRegsMixedWidth()
{
    B3::Procedure proc;
    Code& code = proc.code();

    int64_t memory[2];
    memory[0] = 35000000000000ll;
    memory[1] = 36000000000000ll;

    Vector<Reg> regs = code.regsInPriorityOrder(GP);
    regs.removeFirst(Reg(GPRInfo::regT0));

    BasicBlock* root = code.addBlock();
    for (unsigned i = 0; i < regs.size(); ++i)
        loadConstant(root, (i + 1) * 1000000000000ll, Tmp(regs[i]));
    root->append(Move, nullptr, Arg::immPtr(&memory), Tmp(GPRInfo::regT0));
    Inst& shuffle = root->append(
        Shuffle, nullptr,
        
        Tmp(regs[0]), Arg::addr(Tmp(GPRInfo::regT0), 0 * sizeof(int64_t)),
        Arg::widthArg(Width32),
        
        Arg::addr(Tmp(GPRInfo::regT0), 0 * sizeof(int64_t)),
        Arg::addr(Tmp(GPRInfo::regT0), 1 * sizeof(int64_t)), Arg::widthArg(Width64),

        Arg::addr(Tmp(GPRInfo::regT0), 1 * sizeof(int64_t)), Tmp(regs[1]),
        Arg::widthArg(Width32));

    for (unsigned i = 2; i < regs.size(); ++i) {
        shuffle.append(
            Tmp(regs[i - 1]), Tmp(regs[i]),
            (i & 1) ? Arg::widthArg(Width32) : Arg::widthArg(Width64));
    }

    Vector<int64_t> things(regs.size(), 666);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things[0])), Tmp(GPRInfo::regT0));
    for (unsigned i = 0; i < regs.size(); ++i) {
        root->append(
            Move, nullptr, Tmp(regs[i]), Arg::addr(Tmp(GPRInfo::regT0), i * sizeof(int64_t)));
    }
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1000000000000ll);
    CHECK(things[1] == static_cast<uint32_t>(36000000000000ll));
    for (unsigned i = 2; i < regs.size(); ++i) {
        int64_t value = static_cast<int64_t>(i) * 1000000000000ll;
        CHECK(things[i] == ((i & 1) ? static_cast<uint32_t>(value) : value));
    }
    CHECK(memory[0] == combineHiLo(35000000000000ll, 1000000000000ll));
    CHECK(memory[1] == 35000000000000ll);
}

void testShuffleRotateMemory()
{
    B3::Procedure proc;
    Code& code = proc.code();

    int32_t memory[2];
    memory[0] = 35;
    memory[1] = 36;

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1, Tmp(GPRInfo::regT0));
    loadConstant(root, 2, Tmp(GPRInfo::regT1));
    root->append(Move, nullptr, Arg::immPtr(&memory), Tmp(GPRInfo::regT2));
    root->append(
        Shuffle, nullptr,
        
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),

        Tmp(GPRInfo::regT1), Arg::addr(Tmp(GPRInfo::regT2), 0 * sizeof(int32_t)),
        Arg::widthArg(Width32),
        
        Arg::addr(Tmp(GPRInfo::regT2), 0 * sizeof(int32_t)),
        Arg::addr(Tmp(GPRInfo::regT2), 1 * sizeof(int32_t)), Arg::widthArg(Width32),

        Arg::addr(Tmp(GPRInfo::regT2), 1 * sizeof(int32_t)), Tmp(GPRInfo::regT0),
        Arg::widthArg(Width32));

    int32_t things[2];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move32, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int32_t)));
    root->append(Move32, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int32_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 36);
    CHECK(things[1] == 1);
    CHECK(memory[0] == 2);
    CHECK(memory[1] == 35);
}

void testShuffleRotateMemory64()
{
    B3::Procedure proc;
    Code& code = proc.code();

    int64_t memory[2];
    memory[0] = 35000000000000ll;
    memory[1] = 36000000000000ll;

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1000000000000ll, Tmp(GPRInfo::regT0));
    loadConstant(root, 2000000000000ll, Tmp(GPRInfo::regT1));
    root->append(Move, nullptr, Arg::immPtr(&memory), Tmp(GPRInfo::regT2));
    root->append(
        Shuffle, nullptr,
        
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width64),

        Tmp(GPRInfo::regT1), Arg::addr(Tmp(GPRInfo::regT2), 0 * sizeof(int64_t)),
        Arg::widthArg(Width64),
        
        Arg::addr(Tmp(GPRInfo::regT2), 0 * sizeof(int64_t)),
        Arg::addr(Tmp(GPRInfo::regT2), 1 * sizeof(int64_t)), Arg::widthArg(Width64),

        Arg::addr(Tmp(GPRInfo::regT2), 1 * sizeof(int64_t)), Tmp(GPRInfo::regT0),
        Arg::widthArg(Width64));

    int64_t things[2];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int64_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 36000000000000ll);
    CHECK(things[1] == 1000000000000ll);
    CHECK(memory[0] == 2000000000000ll);
    CHECK(memory[1] == 35000000000000ll);
}

void testShuffleRotateMemoryMixedWidth()
{
    B3::Procedure proc;
    Code& code = proc.code();

    int64_t memory[2];
    memory[0] = 35000000000000ll;
    memory[1] = 36000000000000ll;

    BasicBlock* root = code.addBlock();
    loadConstant(root, 1000000000000ll, Tmp(GPRInfo::regT0));
    loadConstant(root, 2000000000000ll, Tmp(GPRInfo::regT1));
    root->append(Move, nullptr, Arg::immPtr(&memory), Tmp(GPRInfo::regT2));
    root->append(
        Shuffle, nullptr,
        
        Tmp(GPRInfo::regT0), Tmp(GPRInfo::regT1), Arg::widthArg(Width32),

        Tmp(GPRInfo::regT1), Arg::addr(Tmp(GPRInfo::regT2), 0 * sizeof(int64_t)),
        Arg::widthArg(Width64),
        
        Arg::addr(Tmp(GPRInfo::regT2), 0 * sizeof(int64_t)),
        Arg::addr(Tmp(GPRInfo::regT2), 1 * sizeof(int64_t)), Arg::widthArg(Width32),

        Arg::addr(Tmp(GPRInfo::regT2), 1 * sizeof(int64_t)), Tmp(GPRInfo::regT0),
        Arg::widthArg(Width64));

    int64_t things[2];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(Move, nullptr, Tmp(GPRInfo::regT0), Arg::addr(base, 0 * sizeof(int64_t)));
    root->append(Move, nullptr, Tmp(GPRInfo::regT1), Arg::addr(base, 1 * sizeof(int64_t)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 36000000000000ll);
    CHECK(things[1] == static_cast<uint32_t>(1000000000000ll));
    CHECK(memory[0] == 2000000000000ll);
    CHECK(memory[1] == combineHiLo(36000000000000ll, 35000000000000ll));
}

void testShuffleRotateMemoryAllRegs64()
{
    B3::Procedure proc;
    Code& code = proc.code();

    int64_t memory[2];
    memory[0] = 35000000000000ll;
    memory[1] = 36000000000000ll;

    Vector<Reg> regs = code.regsInPriorityOrder(GP);
    regs.removeFirst(Reg(GPRInfo::regT0));

    BasicBlock* root = code.addBlock();
    for (unsigned i = 0; i < regs.size(); ++i)
        loadConstant(root, (i + 1) * 1000000000000ll, Tmp(regs[i]));
    root->append(Move, nullptr, Arg::immPtr(&memory), Tmp(GPRInfo::regT0));
    Inst& shuffle = root->append(
        Shuffle, nullptr,
        
        Tmp(regs[0]), Arg::addr(Tmp(GPRInfo::regT0), 0 * sizeof(int64_t)),
        Arg::widthArg(Width64),
        
        Arg::addr(Tmp(GPRInfo::regT0), 0 * sizeof(int64_t)),
        Arg::addr(Tmp(GPRInfo::regT0), 1 * sizeof(int64_t)), Arg::widthArg(Width64),

        Arg::addr(Tmp(GPRInfo::regT0), 1 * sizeof(int64_t)), Tmp(regs[1]),
        Arg::widthArg(Width64),

        regs.last(), regs[0], Arg::widthArg(Width64));

    for (unsigned i = 2; i < regs.size(); ++i)
        shuffle.append(Tmp(regs[i - 1]), Tmp(regs[i]), Arg::widthArg(Width64));

    Vector<int64_t> things(regs.size(), 666);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things[0])), Tmp(GPRInfo::regT0));
    for (unsigned i = 0; i < regs.size(); ++i) {
        root->append(
            Move, nullptr, Tmp(regs[i]), Arg::addr(Tmp(GPRInfo::regT0), i * sizeof(int64_t)));
    }
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == static_cast<int64_t>(regs.size()) * 1000000000000ll);
    CHECK(things[1] == 36000000000000ll);
    for (unsigned i = 2; i < regs.size(); ++i)
        CHECK(things[i] == static_cast<int64_t>(i) * 1000000000000ll);
    CHECK(memory[0] == 1000000000000ll);
    CHECK(memory[1] == 35000000000000ll);
}

void testShuffleRotateMemoryAllRegsMixedWidth()
{
    B3::Procedure proc;
    Code& code = proc.code();

    int64_t memory[2];
    memory[0] = 35000000000000ll;
    memory[1] = 36000000000000ll;

    Vector<Reg> regs = code.regsInPriorityOrder(GP);
    regs.removeFirst(Reg(GPRInfo::regT0));

    BasicBlock* root = code.addBlock();
    for (unsigned i = 0; i < regs.size(); ++i)
        loadConstant(root, (i + 1) * 1000000000000ll, Tmp(regs[i]));
    root->append(Move, nullptr, Arg::immPtr(&memory), Tmp(GPRInfo::regT0));
    Inst& shuffle = root->append(
        Shuffle, nullptr,
        
        Tmp(regs[0]), Arg::addr(Tmp(GPRInfo::regT0), 0 * sizeof(int64_t)),
        Arg::widthArg(Width32),
        
        Arg::addr(Tmp(GPRInfo::regT0), 0 * sizeof(int64_t)),
        Arg::addr(Tmp(GPRInfo::regT0), 1 * sizeof(int64_t)), Arg::widthArg(Width64),

        Arg::addr(Tmp(GPRInfo::regT0), 1 * sizeof(int64_t)), Tmp(regs[1]),
        Arg::widthArg(Width32),

        regs.last(), regs[0], Arg::widthArg(Width32));

    for (unsigned i = 2; i < regs.size(); ++i)
        shuffle.append(Tmp(regs[i - 1]), Tmp(regs[i]), Arg::widthArg(Width64));

    Vector<int64_t> things(regs.size(), 666);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things[0])), Tmp(GPRInfo::regT0));
    for (unsigned i = 0; i < regs.size(); ++i) {
        root->append(
            Move, nullptr, Tmp(regs[i]), Arg::addr(Tmp(GPRInfo::regT0), i * sizeof(int64_t)));
    }
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == static_cast<uint32_t>(static_cast<int64_t>(regs.size()) * 1000000000000ll));
    CHECK(things[1] == static_cast<uint32_t>(36000000000000ll));
    for (unsigned i = 2; i < regs.size(); ++i)
        CHECK(things[i] == static_cast<int64_t>(i) * 1000000000000ll);
    CHECK(memory[0] == combineHiLo(35000000000000ll, 1000000000000ll));
    CHECK(memory[1] == 35000000000000ll);
}

void testShuffleSwapDouble()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadDoubleConstant(root, 1, Tmp(FPRInfo::fpRegT0), Tmp(GPRInfo::regT0));
    loadDoubleConstant(root, 2, Tmp(FPRInfo::fpRegT1), Tmp(GPRInfo::regT0));
    loadDoubleConstant(root, 3, Tmp(FPRInfo::fpRegT2), Tmp(GPRInfo::regT0));
    loadDoubleConstant(root, 4, Tmp(FPRInfo::fpRegT3), Tmp(GPRInfo::regT0));
    root->append(
        Shuffle, nullptr,
        Tmp(FPRInfo::fpRegT2), Tmp(FPRInfo::fpRegT3), Arg::widthArg(Width64),
        Tmp(FPRInfo::fpRegT3), Tmp(FPRInfo::fpRegT2), Arg::widthArg(Width64));

    double things[4];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::fpRegT0), Arg::addr(base, 0 * sizeof(double)));
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::fpRegT1), Arg::addr(base, 1 * sizeof(double)));
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::fpRegT2), Arg::addr(base, 2 * sizeof(double)));
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::fpRegT3), Arg::addr(base, 3 * sizeof(double)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 2);
    CHECK(things[2] == 4);
    CHECK(things[3] == 3);
}

void testShuffleShiftDouble()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    loadDoubleConstant(root, 1, Tmp(FPRInfo::fpRegT0), Tmp(GPRInfo::regT0));
    loadDoubleConstant(root, 2, Tmp(FPRInfo::fpRegT1), Tmp(GPRInfo::regT0));
    loadDoubleConstant(root, 3, Tmp(FPRInfo::fpRegT2), Tmp(GPRInfo::regT0));
    loadDoubleConstant(root, 4, Tmp(FPRInfo::fpRegT3), Tmp(GPRInfo::regT0));
    root->append(
        Shuffle, nullptr,
        Tmp(FPRInfo::fpRegT2), Tmp(FPRInfo::fpRegT3), Arg::widthArg(Width64));

    double things[4];
    Tmp base = code.newTmp(GP);
    root->append(Move, nullptr, Arg::bigImm(bitwise_cast<intptr_t>(&things)), base);
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::fpRegT0), Arg::addr(base, 0 * sizeof(double)));
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::fpRegT1), Arg::addr(base, 1 * sizeof(double)));
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::fpRegT2), Arg::addr(base, 2 * sizeof(double)));
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::fpRegT3), Arg::addr(base, 3 * sizeof(double)));
    root->append(Move, nullptr, Arg::imm(0), Tmp(GPRInfo::returnValueGPR));
    root->append(Ret32, nullptr, Tmp(GPRInfo::returnValueGPR));

    memset(things, 0, sizeof(things));
    
    CHECK(!compileAndRun<int>(proc));

    CHECK(things[0] == 1);
    CHECK(things[1] == 2);
    CHECK(things[2] == 3);
    CHECK(things[3] == 3);
}

#if CPU(X86) || CPU(X86_64)
void testX86VMULSD()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(MulDouble, nullptr, Tmp(FPRInfo::argumentFPR0), Tmp(FPRInfo::argumentFPR1), Tmp(FPRInfo::argumentFPR2));
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::argumentFPR2), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    CHECK(compileAndRun<double>(proc, 2.4, 4.2, pureNaN()) == 2.4 * 4.2);
}

void testX86VMULSDDestRex()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(MulDouble, nullptr, Tmp(FPRInfo::argumentFPR0), Tmp(FPRInfo::argumentFPR1), Tmp(X86Registers::xmm15));
    root->append(MoveDouble, nullptr, Tmp(X86Registers::xmm15), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    CHECK(compileAndRun<double>(proc, 2.4, 4.2, pureNaN()) == 2.4 * 4.2);
}

void testX86VMULSDOp1DestRex()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::argumentFPR0), Tmp(X86Registers::xmm14));
    root->append(MulDouble, nullptr, Tmp(X86Registers::xmm14), Tmp(FPRInfo::argumentFPR1), Tmp(X86Registers::xmm15));
    root->append(MoveDouble, nullptr, Tmp(X86Registers::xmm15), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    CHECK(compileAndRun<double>(proc, 2.4, 4.2, pureNaN()) == 2.4 * 4.2);
}

void testX86VMULSDOp2DestRex()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::argumentFPR1), Tmp(X86Registers::xmm14));
    root->append(MulDouble, nullptr, Tmp(FPRInfo::argumentFPR0), Tmp(X86Registers::xmm14), Tmp(X86Registers::xmm15));
    root->append(MoveDouble, nullptr, Tmp(X86Registers::xmm15), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    CHECK(compileAndRun<double>(proc, 2.4, 4.2, pureNaN()) == 2.4 * 4.2);
}

void testX86VMULSDOpsDestRex()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::argumentFPR0), Tmp(X86Registers::xmm14));
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::argumentFPR1), Tmp(X86Registers::xmm13));
    root->append(MulDouble, nullptr, Tmp(X86Registers::xmm14), Tmp(X86Registers::xmm13), Tmp(X86Registers::xmm15));
    root->append(MoveDouble, nullptr, Tmp(X86Registers::xmm15), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    CHECK(compileAndRun<double>(proc, 2.4, 4.2, pureNaN()) == 2.4 * 4.2);
}

void testX86VMULSDAddr()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(MulDouble, nullptr, Tmp(FPRInfo::argumentFPR0), Arg::addr(Tmp(GPRInfo::argumentGPR0), - 16), Tmp(FPRInfo::argumentFPR2));
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::argumentFPR2), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    double secondArg = 4.2;
    CHECK(compileAndRun<double>(proc, 2.4, &secondArg + 2, pureNaN()) == 2.4 * 4.2);
}

void testX86VMULSDAddrOpRexAddr()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(Move, nullptr, Tmp(GPRInfo::argumentGPR0), Tmp(X86Registers::r13));
    root->append(MulDouble, nullptr, Tmp(FPRInfo::argumentFPR0), Arg::addr(Tmp(X86Registers::r13), - 16), Tmp(FPRInfo::argumentFPR2));
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::argumentFPR2), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    double secondArg = 4.2;
    CHECK(compileAndRun<double>(proc, 2.4, &secondArg + 2, pureNaN()) == 2.4 * 4.2);
}

void testX86VMULSDDestRexAddr()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(MulDouble, nullptr, Tmp(FPRInfo::argumentFPR0), Arg::addr(Tmp(GPRInfo::argumentGPR0), 16), Tmp(X86Registers::xmm15));
    root->append(MoveDouble, nullptr, Tmp(X86Registers::xmm15), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    double secondArg = 4.2;
    CHECK(compileAndRun<double>(proc, 2.4, &secondArg - 2, pureNaN()) == 2.4 * 4.2);
}

void testX86VMULSDRegOpDestRexAddr()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(MoveDouble, nullptr, Tmp(FPRInfo::argumentFPR0), Tmp(X86Registers::xmm14));
    root->append(MulDouble, nullptr, Arg::addr(Tmp(GPRInfo::argumentGPR0)), Tmp(X86Registers::xmm14), Tmp(X86Registers::xmm15));
    root->append(MoveDouble, nullptr, Tmp(X86Registers::xmm15), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    double secondArg = 4.2;
    CHECK(compileAndRun<double>(proc, 2.4, &secondArg, pureNaN()) == 2.4 * 4.2);
}

void testX86VMULSDAddrOpDestRexAddr()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(Move, nullptr, Tmp(GPRInfo::argumentGPR0), Tmp(X86Registers::r13));
    root->append(MulDouble, nullptr, Tmp(FPRInfo::argumentFPR0), Arg::addr(Tmp(X86Registers::r13), 8), Tmp(X86Registers::xmm15));
    root->append(MoveDouble, nullptr, Tmp(X86Registers::xmm15), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    double secondArg = 4.2;
    CHECK(compileAndRun<double>(proc, 2.4, &secondArg - 1, pureNaN()) == 2.4 * 4.2);
}

void testX86VMULSDBaseNeedsRex()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(Move, nullptr, Tmp(GPRInfo::argumentGPR0), Tmp(X86Registers::r13));
    root->append(MulDouble, nullptr, Arg::index(Tmp(X86Registers::r13), Tmp(GPRInfo::argumentGPR1)), Tmp(FPRInfo::argumentFPR0), Tmp(X86Registers::xmm0));
    root->append(MoveDouble, nullptr, Tmp(X86Registers::xmm0), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    double secondArg = 4.2;
    uint64_t index = 8;
    CHECK(compileAndRun<double>(proc, 2.4, &secondArg - 1, index, pureNaN()) == 2.4 * 4.2);
}

void testX86VMULSDIndexNeedsRex()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(Move, nullptr, Tmp(GPRInfo::argumentGPR1), Tmp(X86Registers::r13));
    root->append(MulDouble, nullptr, Arg::index(Tmp(GPRInfo::argumentGPR0), Tmp(X86Registers::r13)), Tmp(FPRInfo::argumentFPR0), Tmp(X86Registers::xmm0));
    root->append(MoveDouble, nullptr, Tmp(X86Registers::xmm0), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    double secondArg = 4.2;
    uint64_t index = - 8;
    CHECK(compileAndRun<double>(proc, 2.4, &secondArg + 1, index, pureNaN()) == 2.4 * 4.2);
}

void testX86VMULSDBaseIndexNeedRex()
{
    B3::Procedure proc;
    Code& code = proc.code();

    BasicBlock* root = code.addBlock();
    root->append(Move, nullptr, Tmp(GPRInfo::argumentGPR0), Tmp(X86Registers::r12));
    root->append(Move, nullptr, Tmp(GPRInfo::argumentGPR1), Tmp(X86Registers::r13));
    root->append(MulDouble, nullptr, Arg::index(Tmp(X86Registers::r12), Tmp(X86Registers::r13)), Tmp(FPRInfo::argumentFPR0), Tmp(X86Registers::xmm0));
    root->append(MoveDouble, nullptr, Tmp(X86Registers::xmm0), Tmp(FPRInfo::returnValueFPR));
    root->append(RetDouble, nullptr, Tmp(FPRInfo::returnValueFPR));

    double secondArg = 4.2;
    uint64_t index = 16;
    CHECK(compileAndRun<double>(proc, 2.4, &secondArg - 2, index, pureNaN()) == 2.4 * 4.2);
}

#endif

#define RUN(test) do {                          \
        if (!shouldRun(#test))                  \
            break;                              \
        tasks.append(                           \
            createSharedTask<void()>(           \
                [&] () {                        \
                    dataLog(#test "...\n");     \
                    test;                       \
                    dataLog(#test ": OK!\n");   \
                }));                            \
    } while (false);

void run(const char* filter)
{
    JSC::initializeThreading();
    vm = &VM::create(LargeHeap).leakRef();

    Deque<RefPtr<SharedTask<void()>>> tasks;

    auto shouldRun = [&] (const char* testName) -> bool {
        return !filter || !!strcasestr(testName, filter);
    };

    RUN(testSimple());
    
    RUN(testShuffleSimpleSwap());
    RUN(testShuffleSimpleShift());
    RUN(testShuffleLongShift());
    RUN(testShuffleLongShiftBackwards());
    RUN(testShuffleSimpleRotate());
    RUN(testShuffleSimpleBroadcast());
    RUN(testShuffleBroadcastAllRegs());
    RUN(testShuffleTreeShift());
    RUN(testShuffleTreeShiftBackward());
    RUN(testShuffleTreeShiftOtherBackward());
    RUN(testShuffleMultipleShifts());
    RUN(testShuffleRotateWithFringe());
    RUN(testShuffleRotateWithFringeInWeirdOrder());
    RUN(testShuffleRotateWithLongFringe());
    RUN(testShuffleMultipleRotates());
    RUN(testShuffleShiftAndRotate());
    RUN(testShuffleShiftAllRegs());
    RUN(testShuffleRotateAllRegs());
    RUN(testShuffleSimpleSwap64());
    RUN(testShuffleSimpleShift64());
    RUN(testShuffleSwapMixedWidth());
    RUN(testShuffleShiftMixedWidth());
    RUN(testShuffleShiftMemory());
    RUN(testShuffleShiftMemoryLong());
    RUN(testShuffleShiftMemoryAllRegs());
    RUN(testShuffleShiftMemoryAllRegs64());
    RUN(testShuffleShiftMemoryAllRegsMixedWidth());
    RUN(testShuffleRotateMemory());
    RUN(testShuffleRotateMemory64());
    RUN(testShuffleRotateMemoryMixedWidth());
    RUN(testShuffleRotateMemoryAllRegs64());
    RUN(testShuffleRotateMemoryAllRegsMixedWidth());
    RUN(testShuffleSwapDouble());
    RUN(testShuffleShiftDouble());

#if CPU(X86) || CPU(X86_64)
    RUN(testX86VMULSD());
    RUN(testX86VMULSDDestRex());
    RUN(testX86VMULSDOp1DestRex());
    RUN(testX86VMULSDOp2DestRex());
    RUN(testX86VMULSDOpsDestRex());

    RUN(testX86VMULSDAddr());
    RUN(testX86VMULSDAddrOpRexAddr());
    RUN(testX86VMULSDDestRexAddr());
    RUN(testX86VMULSDRegOpDestRexAddr());
    RUN(testX86VMULSDAddrOpDestRexAddr());

    RUN(testX86VMULSDBaseNeedsRex());
    RUN(testX86VMULSDIndexNeedsRex());
    RUN(testX86VMULSDBaseIndexNeedRex());
#endif

    if (tasks.isEmpty())
        usage();

    Lock lock;

    Vector<ThreadIdentifier> threads;
    for (unsigned i = filter ? 1 : WTF::numberOfProcessorCores(); i--;) {
        threads.append(
            createThread(
                "testb3 thread",
                [&] () {
                    for (;;) {
                        RefPtr<SharedTask<void()>> task;
                        {
                            LockHolder locker(lock);
                            if (tasks.isEmpty())
                                return;
                            task = tasks.takeFirst();
                        }

                        task->run();
                    }
                }));
    }

    for (ThreadIdentifier thread : threads)
        waitForThreadCompletion(thread);
    crashLock.lock();
}

} // anonymois namespace

#else // ENABLE(B3_JIT)

static void run(const char*)
{
    dataLog("B3 JIT is not enabled.\n");
}

#endif // ENABLE(B3_JIT)

int main(int argc, char** argv)
{
    const char* filter = nullptr;
    switch (argc) {
    case 1:
        break;
    case 2:
        filter = argv[1];
        break;
    default:
        usage();
        break;
    }
    
    run(filter);
    return 0;
}
