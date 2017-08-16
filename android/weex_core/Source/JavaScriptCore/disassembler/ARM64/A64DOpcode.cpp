/*
 * Copyright (C) 2012, 2016 Apple Inc. All rights reserved.
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

#if USE(ARM64_DISASSEMBLER)

#include "A64DOpcode.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

namespace JSC { namespace ARM64Disassembler {

A64DOpcode::OpcodeGroup* A64DOpcode::opcodeTable[32];

const char* const A64DOpcode::s_conditionNames[16] = {
    "eq", "ne", "hs", "lo", "mi", "pl", "vs", "vc",
    "hi", "ls", "ge", "lt", "gt", "le", "al", "ne"
};

const char* const A64DOpcode::s_optionName[8] = {
    "uxtb", "uxth", "uxtw", "uxtx", "sxtb", "sxth", "sxtw", "sxtx"
};

const char* const A64DOpcode::s_shiftNames[4] = {
    "lsl", "lsr", "asl", "ror"
};

const char A64DOpcode::s_FPRegisterPrefix[5] = {
    'b', 'h', 's', 'd', 'q'
};

struct OpcodeGroupInitializer {
    unsigned m_opcodeGroupNumber;
    uint32_t m_mask;
    uint32_t m_pattern;
    const char* (*m_format)(A64DOpcode*);
};

#define OPCODE_GROUP_ENTRY(groupIndex, groupClass) \
{ groupIndex, groupClass::mask, groupClass::pattern, groupClass::format }

static OpcodeGroupInitializer opcodeGroupList[] = {
    OPCODE_GROUP_ENTRY(0x08, A64DOpcodeLoadStoreRegisterPair),
    OPCODE_GROUP_ENTRY(0x08, A64DOpcodeLoadStoreExclusive),
    OPCODE_GROUP_ENTRY(0x09, A64DOpcodeLoadStoreRegisterPair),
    OPCODE_GROUP_ENTRY(0x0a, A64DOpcodeLogicalShiftedRegister),
    OPCODE_GROUP_ENTRY(0x0b, A64DOpcodeAddSubtractExtendedRegister),
    OPCODE_GROUP_ENTRY(0x0b, A64DOpcodeAddSubtractShiftedRegister),
    OPCODE_GROUP_ENTRY(0x11, A64DOpcodeAddSubtractImmediate),
    OPCODE_GROUP_ENTRY(0x12, A64DOpcodeMoveWide),
    OPCODE_GROUP_ENTRY(0x12, A64DOpcodeLogicalImmediate),
    OPCODE_GROUP_ENTRY(0x13, A64DOpcodeBitfield),
    OPCODE_GROUP_ENTRY(0x13, A64DOpcodeExtract),
    OPCODE_GROUP_ENTRY(0x14, A64DOpcodeUnconditionalBranchImmediate),
    OPCODE_GROUP_ENTRY(0x14, A64DOpcodeConditionalBranchImmediate),
    OPCODE_GROUP_ENTRY(0x14, A64DOpcodeCompareAndBranchImmediate),
    OPCODE_GROUP_ENTRY(0x14, A64OpcodeExceptionGeneration),
    OPCODE_GROUP_ENTRY(0x15, A64DOpcodeUnconditionalBranchImmediate),
    OPCODE_GROUP_ENTRY(0x15, A64DOpcodeConditionalBranchImmediate),
    OPCODE_GROUP_ENTRY(0x15, A64DOpcodeCompareAndBranchImmediate),
    OPCODE_GROUP_ENTRY(0x15, A64DOpcodeHint),
    OPCODE_GROUP_ENTRY(0x15, A64DOpcodeDmb),
    OPCODE_GROUP_ENTRY(0x16, A64DOpcodeUnconditionalBranchImmediate),
    OPCODE_GROUP_ENTRY(0x16, A64DOpcodeUnconditionalBranchRegister),
    OPCODE_GROUP_ENTRY(0x16, A64DOpcodeTestAndBranchImmediate),
    OPCODE_GROUP_ENTRY(0x17, A64DOpcodeUnconditionalBranchImmediate),
    OPCODE_GROUP_ENTRY(0x17, A64DOpcodeUnconditionalBranchRegister),
    OPCODE_GROUP_ENTRY(0x17, A64DOpcodeTestAndBranchImmediate),
    OPCODE_GROUP_ENTRY(0x18, A64DOpcodeLoadStoreImmediate),
    OPCODE_GROUP_ENTRY(0x18, A64DOpcodeLoadStoreRegisterOffset),
    OPCODE_GROUP_ENTRY(0x19, A64DOpcodeLoadStoreUnsignedImmediate),
    OPCODE_GROUP_ENTRY(0x1a, A64DOpcodeConditionalSelect),
    OPCODE_GROUP_ENTRY(0x1a, A64DOpcodeDataProcessing1Source),
    OPCODE_GROUP_ENTRY(0x1a, A64DOpcodeDataProcessing2Source),
    OPCODE_GROUP_ENTRY(0x1b, A64DOpcodeDataProcessing3Source),
    OPCODE_GROUP_ENTRY(0x1c, A64DOpcodeLoadStoreImmediate),
    OPCODE_GROUP_ENTRY(0x1c, A64DOpcodeLoadStoreRegisterOffset),
    OPCODE_GROUP_ENTRY(0x1d, A64DOpcodeLoadStoreUnsignedImmediate),
    OPCODE_GROUP_ENTRY(0x1e, A64DOpcodeFloatingPointCompare),
    OPCODE_GROUP_ENTRY(0x1e, A64DOpcodeFloatingPointConditionalSelect),
    OPCODE_GROUP_ENTRY(0x1e, A64DOpcodeFloatingPointDataProcessing2Source),
    OPCODE_GROUP_ENTRY(0x1e, A64DOpcodeFloatingPointDataProcessing1Source),
    OPCODE_GROUP_ENTRY(0x1e, A64DOpcodeFloatingFixedPointConversions),
    OPCODE_GROUP_ENTRY(0x1e, A64DOpcodeFloatingPointIntegerConversions),
};

bool A64DOpcode::s_initialized = false;

void A64DOpcode::init()
{
    if (s_initialized)
        return;

    OpcodeGroup* lastGroups[32];

    for (unsigned i = 0; i < 32; i++) {
        opcodeTable[i] = 0;
        lastGroups[i] = 0;
    }

    for (unsigned i = 0; i < sizeof(opcodeGroupList) / sizeof(struct OpcodeGroupInitializer); i++) {
        OpcodeGroup* newOpcodeGroup = new OpcodeGroup(opcodeGroupList[i].m_mask, opcodeGroupList[i].m_pattern, opcodeGroupList[i].m_format);
        uint32_t opcodeGroupNumber = opcodeGroupList[i].m_opcodeGroupNumber;

        if (!opcodeTable[opcodeGroupNumber])
            opcodeTable[opcodeGroupNumber] = newOpcodeGroup;
        else
            lastGroups[opcodeGroupNumber]->setNext(newOpcodeGroup);
        lastGroups[opcodeGroupNumber] = newOpcodeGroup;
    }

    s_initialized = true;
}

void A64DOpcode::setPCAndOpcode(uint32_t* newPC, uint32_t newOpcode)
{
    m_currentPC = newPC;
    m_opcode = newOpcode;
    m_bufferOffset = 0;
    m_formatBuffer[0] = '\0';
}

const char* A64DOpcode::disassemble(uint32_t* currentPC)
{
    setPCAndOpcode(currentPC, *currentPC);

    OpcodeGroup* opGroup = opcodeTable[opcodeGroupNumber(m_opcode)];

    while (opGroup) {
        if (opGroup->matches(m_opcode))
            return opGroup->format(this);
        opGroup = opGroup->next();
    }

    return A64DOpcode::format();
}

void A64DOpcode::bufferPrintf(const char* format, ...)
{
    if (m_bufferOffset >= bufferSize)
        return;

    va_list argList;
    va_start(argList, format);

    m_bufferOffset += vsnprintf(m_formatBuffer + m_bufferOffset, bufferSize - m_bufferOffset, format, argList);

    va_end(argList);
}

const char* A64DOpcode::format()
{
    bufferPrintf("   .long  %08x", m_opcode);
    return m_formatBuffer;
}

void A64DOpcode::appendRegisterName(unsigned registerNumber, bool is64Bit)
{
    if (registerNumber == 29) {
        bufferPrintf(is64Bit ? "fp" : "wfp");
        return;
    }

    if (registerNumber == 30) {
        bufferPrintf(is64Bit ? "lr" : "wlr");
        return;
    }

    bufferPrintf("%c%u", is64Bit ? 'x' : 'w', registerNumber);
}

void A64DOpcode::appendFPRegisterName(unsigned registerNumber, unsigned registerSize)
{
    bufferPrintf("%c%u", FPRegisterPrefix(registerSize), registerNumber);
}

const char* const A64DOpcodeAddSubtract::s_opNames[4] = { "add", "adds", "sub", "subs" };

const char* A64DOpcodeAddSubtractImmediate::format()
{
    if (isCMP())
        appendInstructionName(cmpName());
    else {
        if (isMovSP())
            appendInstructionName("mov");
        else
            appendInstructionName(opName());
        appendSPOrRegisterName(rd(), is64Bit());
        appendSeparator();
    }
    appendSPOrRegisterName(rn(), is64Bit());

    if (!isMovSP()) {
        appendSeparator();
        appendUnsignedImmediate(immed12());
        if (shift()) {
            appendSeparator();
            appendString(shift() == 1 ? "lsl" : "reserved");
        }
    }
    return m_formatBuffer;
}

const char* A64DOpcodeAddSubtractExtendedRegister::format()
{
    if (immediate3() > 4)
        return A64DOpcode::format();

    if (isCMP())
        appendInstructionName(cmpName());
    else {
        appendInstructionName(opName());
        appendSPOrRegisterName(rd(), is64Bit());
        appendSeparator();
    }
    appendSPOrRegisterName(rn(), is64Bit());
    appendSeparator();
    appendZROrRegisterName(rm(), is64Bit() && ((option() & 0x3) == 0x3));
    appendSeparator();
    if (option() == 0x2 && ((rd() == 31) || (rn() == 31)))
        appendString("lsl");
    else
        appendString(optionName());
    if (immediate3()) {
        appendCharacter(' ');
        appendUnsignedImmediate(immediate3());
    }

    return m_formatBuffer;
}

const char* A64DOpcodeAddSubtractShiftedRegister::format()
{
    if (!is64Bit() && immediate6() & 0x20)
        return A64DOpcode::format();

    if (shift() == 0x3)
        return A64DOpcode::format();

    if (isCMP())
        appendInstructionName(cmpName());
    else {
        if (isNeg())
            appendInstructionName(cmpName());
        else
            appendInstructionName(opName());
        appendSPOrRegisterName(rd(), is64Bit());
        appendSeparator();
    }
    if (!isNeg()) {
        appendRegisterName(rn(), is64Bit());
        appendSeparator();
    }
    appendZROrRegisterName(rm(), is64Bit());
    if (immediate6()) {
        appendSeparator();
        appendShiftType(shift());
        appendUnsignedImmediate(immediate6());
    }

    return m_formatBuffer;
}

const char* const A64DOpcodeBitfield::s_opNames[3] = { "sbfm", "bfm", "ubfm" };
const char* const A64DOpcodeBitfield::s_extendPseudoOpNames[3][3] = {
    { "sxtb", "sxth", "sxtw" }, { 0, 0, 0} , { "uxtb", "uxth", "uxtw" } };
const char* const A64DOpcodeBitfield::s_insertOpNames[3] = { "sbfiz", "bfi", "ubfiz" };
const char* const A64DOpcodeBitfield::s_extractOpNames[3] = { "sbfx", "bfxil", "ubfx" };

const char* A64DOpcodeBitfield::format()
{
    if (opc() == 0x3)
        return A64DOpcode::format();

    if (is64Bit() != nBit())
        return A64DOpcode::format();

    if (!is64Bit() && ((immediateR() & 0x20) || (immediateS() & 0x20)))
        return A64DOpcode::format();

    if (!(opc() & 0x1) && !immediateR()) {
        // [un]signed {btye,half-word,word} extend
        bool isSTXType = false;
        if (immediateS() == 7) {
            appendInstructionName(extendPseudoOpNames(0));
            isSTXType = true;
        } else if (immediateS() == 15) {
            appendInstructionName(extendPseudoOpNames(1));
            isSTXType = true;
        } else if (immediateS() == 31 && is64Bit() && !opc()) {
            appendInstructionName(extendPseudoOpNames(2));
            isSTXType = true;
        }

        if (isSTXType) {
            appendRegisterName(rd(), is64Bit());
            appendSeparator();
            appendRegisterName(rn(), false);

            return m_formatBuffer;
        }
    }

    if (!(opc() & 0x1) && ((immediateS() & 0x1f) == 0x1f) && (is64Bit() == (immediateS() >> 5))) {
        // asr/lsr
        appendInstructionName(!opc() ? "asr" : "lsr");

        appendRegisterName(rd(), is64Bit());
        appendSeparator();
        appendRegisterName(rn(), is64Bit());
        appendSeparator();
        appendUnsignedImmediate(immediateR());

        return m_formatBuffer;
    }

    if (opc() == 0x2 && (immediateS() + 1) == immediateR()) {
        // lsl
        appendInstructionName("lsl");
        appendRegisterName(rd(), is64Bit());
        appendSeparator();
        appendRegisterName(rn(), is64Bit());
        appendSeparator();
        appendUnsignedImmediate((is64Bit() ? 64u : 32u) - immediateR());
        
        return m_formatBuffer;
    }
    
    if (immediateS() < immediateR()) {
        if (opc() != 1 || rn() != 0x1f) {
            // bit field insert
            appendInstructionName(insertOpNames());

            appendRegisterName(rd(), is64Bit());
            appendSeparator();
            appendRegisterName(rn(), is64Bit());
            appendSeparator();
            appendUnsignedImmediate((is64Bit() ? 64u : 32u) - immediateR());
            appendSeparator();
            appendUnsignedImmediate(immediateS() + 1);

            return m_formatBuffer;
        }
        
        appendInstructionName(opName());
        appendRegisterName(rd(), is64Bit());
        appendSeparator();
        appendRegisterName(rn(), is64Bit());
        appendSeparator();
        appendUnsignedImmediate(immediateR());
        appendSeparator();
        appendUnsignedImmediate(immediateS());
        
        return m_formatBuffer;
    }
    
    // bit field extract
    appendInstructionName(extractOpNames());

    appendRegisterName(rd(), is64Bit());
    appendSeparator();
    appendRegisterName(rn(), is64Bit());
    appendSeparator();
    appendUnsignedImmediate(immediateR());
    appendSeparator();
    appendUnsignedImmediate(immediateS() - immediateR() + 1);

    return m_formatBuffer;
}

const char* A64DOpcodeCompareAndBranchImmediate::format()
{
    appendInstructionName(opBit() ? "cbnz" : "cbz");
    appendRegisterName(rt(), is64Bit());
    appendSeparator();    
    appendPCRelativeOffset(m_currentPC, static_cast<int32_t>(immediate19()));
    return m_formatBuffer;
}

const char* A64DOpcodeConditionalBranchImmediate::format()
{
    bufferPrintf("   b.%-5.5s", conditionName(condition()));
    appendPCRelativeOffset(m_currentPC, static_cast<int32_t>(immediate19()));
    return m_formatBuffer;
}

const char* const A64DOpcodeConditionalSelect::s_opNames[4] = {
    "csel", "csinc", "csinv", "csneg"
};

const char* A64DOpcodeConditionalSelect::format()
{
    if (sBit())
        return A64DOpcode::format();

    if (op2() & 0x2)
        return A64DOpcode::format();

    if (rn() == rm() && (opNum() == 1 || opNum() == 2)) {
        if (rn() == 31) {
            appendInstructionName((opNum() == 1) ? "cset" : "csetm");
            appendRegisterName(rd(), is64Bit());
        } else {
            appendInstructionName((opNum() == 1) ? "cinc" : "cinv");
            appendRegisterName(rd(), is64Bit());
            appendSeparator();
            appendZROrRegisterName(rn(), is64Bit());
        }
        appendSeparator();
        appendString(conditionName(condition() ^ 0x1));

        return m_formatBuffer;
    }

    appendInstructionName(opName());
    appendRegisterName(rd(), is64Bit());
    appendSeparator();
    appendZROrRegisterName(rn(), is64Bit());
    appendSeparator();
    appendZROrRegisterName(rm(), is64Bit());
    appendSeparator();
    appendString(conditionName(condition()));

    return m_formatBuffer;

}

const char* const A64DOpcodeDataProcessing1Source::s_opNames[8] = {
    "rbit", "rev16", "rev32", "rev", "clz", "cls", 0, 0
};

const char* A64DOpcodeDataProcessing1Source::format()
{
    if (sBit())
        return A64DOpcode::format();

    if (opCode2())
        return A64DOpcode::format();

    if (opCode() & 0x38)
        return A64DOpcode::format();

    if ((opCode() & 0x3e) == 0x6)
        return A64DOpcode::format();

    if (is64Bit() && opCode() == 0x3)
        return A64DOpcode::format();

    if (!is64Bit() && opCode() == 0x2)
        appendInstructionName("rev");
    else
        appendInstructionName(opName());
    appendZROrRegisterName(rd(), is64Bit());
    appendSeparator();
    appendZROrRegisterName(rn(), is64Bit());
    
    return m_formatBuffer;
}

const char* const A64DOpcodeDataProcessing2Source::s_opNames[8] = {
    0, 0, "udiv", "sdiv", "lsl", "lsr", "asr", "ror" // We use the pseudo-op names for the shift/rotate instructions
};

const char* A64DOpcodeDataProcessing2Source::format()
{
    if (sBit())
        return A64DOpcode::format();

    if (!(opCode() & 0x3e))
        return A64DOpcode::format();

    if (opCode() & 0x30)
        return A64DOpcode::format();

    if ((opCode() & 0x34) == 0x4)
        return A64DOpcode::format();

    appendInstructionName(opName());
    appendZROrRegisterName(rd(), is64Bit());
    appendSeparator();
    appendZROrRegisterName(rn(), is64Bit());
    appendSeparator();
    appendZROrRegisterName(rm(), is64Bit());

    return m_formatBuffer;
}

const char* const A64DOpcodeDataProcessing3Source::s_opNames[16] = {
    "madd", "msub", "smaddl", "smsubl", "smulh", 0, 0, 0,
    0, 0, "umaddl", "umsubl", "umulh", 0, 0, 0
};

const char* const A64DOpcodeDataProcessing3Source::s_pseudoOpNames[16] = {
    "mul", "mneg", "smull", "smnegl", "smulh", 0, 0, 0,
    0, 0, "umull", "umnegl", "umulh", 0, 0, 0
};

const char* A64DOpcodeDataProcessing3Source::format()
{
    if (op54())
        return A64DOpcode::format();

    if (opNum() > 12)
        return A64DOpcode::format();

    if (!is64Bit() && opNum() > 1)
        return A64DOpcode::format();

    if (!opName())
        return A64DOpcode::format();

    if ((opNum() & 0x4) && (ra() != 31))
        return A64DOpcode::format();

    appendInstructionName(opName());
    appendZROrRegisterName(rd(), is64Bit());
    appendSeparator();
    bool srcOneAndTwoAre64Bit = is64Bit() & !(opNum() & 0x2);
    appendZROrRegisterName(rn(), srcOneAndTwoAre64Bit);
    appendSeparator();
    appendZROrRegisterName(rm(), srcOneAndTwoAre64Bit);

    if (ra() != 31) {
        appendSeparator();
        appendRegisterName(ra(), is64Bit());
    }

    return m_formatBuffer;
}

const char* A64OpcodeExceptionGeneration::format()
{
    const char* opname = 0;
    if (!op2()) {
        switch (opc()) {
        case 0x0: // SVC, HVC & SMC
            switch (ll()) {
            case 0x1:
                opname = "svc";
                break;
            case 0x2:
                opname = "hvc";
                break;
            case 0x3:
                opname = "smc";
                break;
            }
            break;
        case 0x1: // BRK
            if (!ll())
                opname = "brk";
            break;
        case 0x2: // HLT
            if (!ll())
                opname = "hlt";
            break;
        case 0x5: // DPCS1-3
            switch (ll()) {
            case 0x1:
                opname = "dpcs1";
                break;
            case 0x2:
                opname = "dpcs2";
                break;
            case 0x3:
                opname = "dpcs3";
                break;
            }
            break;
        }
    }

    if (!opname)
        return A64DOpcode::format();

    appendInstructionName(opname);
    appendUnsignedImmediate(immediate16());
    return m_formatBuffer;
}

const char* A64DOpcodeExtract::format()
{
    if (op21() || o0Bit())
        return A64DOpcode::format();

    if (is64Bit() != nBit())
        return A64DOpcode::format();

    if (!is64Bit() && (immediateS() & 0x20))
        return A64DOpcode::format();

    bool isROR = rn() == rm();
    const char* opName = (isROR) ? "ror" : "extr";

    appendInstructionName(opName);
    appendZROrRegisterName(rd(), is64Bit());
    appendSeparator();
    appendZROrRegisterName(rn(), is64Bit());
    if (!isROR) {
        appendSeparator();
        appendZROrRegisterName(rm(), is64Bit());
    }
    appendSeparator();
    appendUnsignedImmediate(immediateS());

    return m_formatBuffer;
}

const char* A64DOpcodeFloatingPointCompare::format()
{
    if (mBit())
        return A64DOpcode::format();

    if (sBit())
        return A64DOpcode::format();

    if (type() & 0x2)
        return A64DOpcode::format();

    if (op())
        return A64DOpcode::format();

    if (opCode2() & 0x7)
        return A64DOpcode::format();

    appendInstructionName(opName());
    unsigned registerSize = type() + 2;
    appendFPRegisterName(rn(), registerSize);
    appendSeparator();
    if (opCode2() & 0x8)
        bufferPrintf("#0.0");
    else
        appendFPRegisterName(rm(), registerSize);
    
    return m_formatBuffer;
}

const char* A64DOpcodeFloatingPointConditionalSelect::format()
{
    if (mBit())
        return A64DOpcode::format();
    
    if (sBit())
        return A64DOpcode::format();
    
    if (type() & 0x2)
        return A64DOpcode::format();

    appendInstructionName(opName());
    unsigned registerSize = type() + 2;
    appendFPRegisterName(rd(), registerSize);
    appendSeparator();
    appendFPRegisterName(rn(), registerSize);
    appendSeparator();
    appendFPRegisterName(rm(), registerSize);
    appendSeparator();
    appendString(conditionName(condition()));
    
    return m_formatBuffer;
}

const char* const A64DOpcodeFloatingPointDataProcessing1Source::s_opNames[16] = {
    "fmov", "fabs", "fneg", "fsqrt", "fcvt", "fcvt", 0, "fcvt",
    "frintn", "frintp", "frintm", "frintz", "frinta", 0, "frintx", "frinti"
};

const char* A64DOpcodeFloatingPointDataProcessing1Source::format()
{
    if (mBit())
        return A64DOpcode::format();

    if (sBit())
        return A64DOpcode::format();

    if (opNum() > 16)
        return A64DOpcode::format();

    switch (type()) {
    case 0:
        if ((opNum() == 0x4) || (opNum() == 0x6) || (opNum() == 0xd))
            return A64DOpcode::format();
        break;
    case 1:
        if ((opNum() == 0x5) || (opNum() == 0x6) || (opNum() == 0xd))
            return A64DOpcode::format();
        break;
    case 2:
        return A64DOpcode::format();
    case 3:
        if ((opNum() < 0x4) || (opNum() > 0x5))
            return A64DOpcode::format();
        break;
    }

    appendInstructionName(opName());
    if ((opNum() >= 0x4) && (opNum() <= 0x7)) {
        unsigned srcRegisterSize = type() ^ 0x2; // 0:s, 1:d & 3:h
        unsigned destRegisterSize = (opNum() & 0x3) ^ 0x2;
        appendFPRegisterName(rd(), destRegisterSize);
        appendSeparator();
        appendFPRegisterName(rn(), srcRegisterSize);
    } else {
        unsigned registerSize = type() + 2;
        appendFPRegisterName(rd(), registerSize);
        appendSeparator();
        appendFPRegisterName(rn(), registerSize);
    }

    return m_formatBuffer;
}

const char* const A64DOpcodeFloatingPointDataProcessing2Source::s_opNames[16] = {
    "fmul", "fdiv", "fadd", "fsub", "fmax", "fmin", "fmaxnm", "fminnm", "fnmul"
};

const char* A64DOpcodeFloatingPointDataProcessing2Source::format()
{
    if (mBit())
        return A64DOpcode::format();

    if (sBit())
        return A64DOpcode::format();

    if (type() & 0x2)
        return A64DOpcode::format();

    if (opNum() > 8)
        return A64DOpcode::format();

    appendInstructionName(opName());
    unsigned registerSize = type() + 2;
    appendFPRegisterName(rd(), registerSize);
    appendSeparator();
    appendFPRegisterName(rn(), registerSize);
    appendSeparator();
    appendFPRegisterName(rm(), registerSize);

    return m_formatBuffer;
}

const char* const A64DOpcodeFloatingFixedPointConversions::s_opNames[4] = {
    "fcvtzs", "fcvtzu", "scvtf", "ucvtf"
};

const char* A64DOpcodeFloatingFixedPointConversions::format()
{
    if (sBit())
        return A64DOpcode::format();

    if (type() & 0x2)
        return A64DOpcode::format();

    if (opcode() & 0x4)
        return A64DOpcode::format();

    if (!(rmode() & 0x1) && !(opcode() & 0x6))
        return A64DOpcode::format();

    if ((rmode() & 0x1) && (opcode() & 0x6) == 0x2)
        return A64DOpcode::format();

    if (!(rmode() & 0x2) && !(opcode() & 0x6))
        return A64DOpcode::format();

    if ((rmode() & 0x2) && (opcode() & 0x6) == 0x2)
        return A64DOpcode::format();

    if (!is64Bit() && scale() >= 32)
        return A64DOpcode::format();

    appendInstructionName(opName());
    unsigned FPRegisterSize = type() + 2;
    bool destIsFP = !rmode();
    
    if (destIsFP) {
        appendFPRegisterName(rd(), FPRegisterSize);
        appendSeparator();
        appendRegisterName(rn(), is64Bit());
    } else {
        appendRegisterName(rd(), is64Bit());
        appendSeparator();
        appendFPRegisterName(rn(), FPRegisterSize);
    }
    appendSeparator();
    appendUnsignedImmediate(64 - scale());
    
    return m_formatBuffer;
}

const char* const A64DOpcodeFloatingPointIntegerConversions::s_opNames[32] = {
    "fcvtns", "fcvtnu", "scvtf", "ucvtf", "fcvtas", "fcvtau", "fmov", "fmov",
    "fcvtps", "fcvtpu", 0, 0, 0, 0, "fmov", "fmov",
    "fcvtms", "fcvtmu", 0, 0, 0, 0, 0, 0,
    "fcvtzs", "fcvtzu", 0, 0, 0, 0, 0, 0
};

const char* A64DOpcodeFloatingPointIntegerConversions::format()
{
    if (sBit())
        return A64DOpcode::format();

    if (type() == 0x3)
        return A64DOpcode::format();

    if (((rmode() & 0x1) || (rmode() & 0x2)) && (((opcode() & 0x6) == 0x2) || ((opcode() & 0x6) == 0x4)))
        return A64DOpcode::format();

    if ((type() == 0x2) && (!(opcode() & 0x4) || ((opcode() & 0x6) == 0x4)))
        return A64DOpcode::format();

    if (!type() && (rmode() & 0x1) && ((opcode() & 0x6) == 0x6))
        return A64DOpcode::format();

    if (is64Bit() && type() == 0x2 && ((opNum() & 0xe) == 0x6))
        return A64DOpcode::format();

    if (!opName())
        return A64DOpcode::format();

    if ((opNum() & 0x1e) == 0xe) {
        // Handle fmov to/from upper half of quad separately
        if (!is64Bit() || (type() != 0x2))
            return A64DOpcode::format();

        appendInstructionName(opName());
        if (opcode() & 0x1) {
            // fmov Vd.D[1], Xn
            bufferPrintf("V%u.D[1]", rd());
            appendSeparator();
            appendZROrRegisterName(rn());
        } else {
            // fmov Xd, Vn.D[1]
            appendZROrRegisterName(rd());
            appendSeparator();
            bufferPrintf("V%u.D[1]", rn());
        }

        return m_formatBuffer;
    }

    appendInstructionName(opName());
    unsigned FPRegisterSize = type() + 2;
    bool destIsFP = ((opNum() == 2) || (opNum() == 3) || (opNum() == 7));

    if (destIsFP) {
        appendFPRegisterName(rd(), FPRegisterSize);
        appendSeparator();
        appendZROrRegisterName(rn(), is64Bit());
    } else {
        appendZROrRegisterName(rd(), is64Bit());
        appendSeparator();
        appendFPRegisterName(rn(), FPRegisterSize);
    }

    return m_formatBuffer;
}

const char* const A64DOpcodeHint::s_opNames[6] = {
    "nop", "yield", "wfe", "wfi", "sev", "sevl"
};

const char* A64DOpcodeHint::format()
{
    appendInstructionName(opName());

    if (immediate7() > 5)
        appendUnsignedImmediate(immediate7());

    return m_formatBuffer;
}

const char* const A64DOpcodeDmb::s_optionNames[16] = {
    0, "oshld", "oshst", "osh", 0, "nshld", "nshst", "nsh",
    0, "ishld", "ishst", "ish", 0, "ld", "st", "sy"
};

const char* A64DOpcodeDmb::format()
{
    appendInstructionName(opName());
    const char* thisOption = option();
    if (thisOption)
        appendString(thisOption);
    else
        appendUnsignedImmediate(crM());

    return m_formatBuffer;
}

const char* const A64DOpcodeLoadStoreExclusive::s_opNames[64] = {
    "stxrb", "stlxrb", 0, 0, "ldxrb", "ldaxrb", 0, 0,
    0, "stlrb", 0, 0, 0, "ldarb", 0, 0,
    "stxrh", "stlxrh", 0, 0, "ldxrh", "ldaxrh", 0, 0,
    0, "stlrh", 0, 0, 0, "ldarh", 0, 0,
    "stxr", "stlxr", "stxp", "stlxp", "ldxr", "ldaxr", "ldxp", "ldaxp",
    0, "stlr", 0, 0, 0, "ldar", 0, 0,
    "stxr", "stlxr", "stxp", "stlxp", "ldxr", "ldaxr", "ldxp", "ldaxp",
    0, "stlr", 0, 0, 0, "ldar", 0, 0
};

const char* A64DOpcodeLoadStoreExclusive::format()
{
    if (o2() && !o1() && !o0())
        return A64DOpcode::format();

    if (o2() && o1())
        return A64DOpcode::format();

    if ((size() < 2) && o1())
        return A64DOpcode::format();

    if (loadBit() && (rs() != 0x1f))
        return A64DOpcode::format();

    if (!isPairOp() && (rt2() != 0x1f))
        return A64DOpcode::format();

    const char* thisOpName = opName();

    if (!thisOpName)
        return A64DOpcode::format();

    appendInstructionName(thisOpName);

    if (!loadBit()) {
        appendZROrRegisterName(rs(), size() == 0x3);
        appendSeparator();
    }

    appendZROrRegisterName(rt(), size() == 0x3);
    appendSeparator();
    if (isPairOp()) {
        appendZROrRegisterName(rt2(), size() == 0x3);
        appendSeparator();
    }
    appendCharacter('[');
    appendSPOrRegisterName(rn());
    appendCharacter(']');

    return m_formatBuffer;
}

// A zero in an entry of the table means the instruction is Unallocated
const char* const A64DOpcodeLoadStore::s_opNames[32] = {
    "strb", "ldrb", "ldrsb", "ldrsb", "str", "ldr", "str", "ldr",
    "strh", "ldrh", "ldrsh", "ldrsh", "str", "ldr", 0, 0,
    "str", "ldr", "ldrsw", 0, "str", "ldr", 0, 0,
    "str", "ldr", 0, 0, "str", "ldr", 0, 0
};

// A zero in an entry of the table means the instruction is Unallocated
const char* const A64DOpcodeLoadStoreImmediate::s_unprivilegedOpNames[32] = {
    "sttrb", "ldtrb", "ldtrsb", "ldtrsb", 0, 0, 0, 0,
    "sttrh", "ldtrh", "ldtrsh", "ldtrsh", 0, 0, 0, 0,
    "sttr", "ldtr", "ldtrsw", 0, 0, 0, 0, 0,
    "sttr", "ldtr", 0, 0, 0, 0, 0, 0
};

// A zero in an entry of the table means the instruction is Unallocated
const char* const A64DOpcodeLoadStoreImmediate::s_unscaledOpNames[32] = {
    "sturb", "ldurb", "ldursb", "ldursb", "stur", "ldur", "stur", "ldur",
    "sturh", "ldurh", "ldursh", "ldursh", "stur", "ldur", 0, 0,
    "stur", "ldur", "ldursw", 0, "stur", "ldur", 0, 0,
    "stur", "ldur", "prfum", 0, "stur", "ldur", 0, 0
};

const char* A64DOpcodeLoadStoreImmediate::format()
{
    const char* thisOpName;

    if (type() & 0x1)
        thisOpName = opName();
    else if (!type())
        thisOpName = unscaledOpName();
    else
        thisOpName = unprivilegedOpName();

    if (!thisOpName)
        return A64DOpcode::format();

    appendInstructionName(thisOpName);
    if (vBit())
        appendFPRegisterName(rt(), size());
    else if (!opc())
        appendZROrRegisterName(rt(), is64BitRT());
    else
        appendRegisterName(rt(), is64BitRT());
    appendSeparator();
    appendCharacter('[');
    appendSPOrRegisterName(rn());

    switch (type()) {
    case 0: // Unscaled Immediate
        if (immediate9()) {
            appendSeparator();
            appendSignedImmediate(immediate9());
        }
        appendCharacter(']');
        break;
    case 1: // Immediate Post-Indexed
        appendCharacter(']');
        if (immediate9()) {
            appendSeparator();
            appendSignedImmediate(immediate9());
        }
        break;
    case 2: // Unprivileged
        if (immediate9()) {
            appendSeparator();
            appendSignedImmediate(immediate9());
        }
        appendCharacter(']');
        break;
    case 3: // Immediate Pre-Indexed
        if (immediate9()) {
            appendSeparator();
            appendSignedImmediate(immediate9());
        }
        appendCharacter(']');
        appendCharacter('!');
        break;
    }

    return m_formatBuffer;
}

const char* A64DOpcodeLoadStoreRegisterOffset::format()
{
    const char* thisOpName = opName();

    if (!thisOpName)
        return A64DOpcode::format();

    if (!(option() & 0x2))
        return A64DOpcode::format();

    appendInstructionName(thisOpName);
    unsigned scale;
    if (vBit()) {
        appendFPRegisterName(rt(), size());
        scale = ((opc() & 2)<<1) | size();
    } else {
        if (!opc())
            appendZROrRegisterName(rt(), is64BitRT());
        else
            appendRegisterName(rt(), is64BitRT());
        scale = size();
    }
    appendSeparator();
    appendCharacter('[');
    appendSPOrRegisterName(rn());
    if (rm() != 31) {
        appendSeparator();
        appendRegisterName(rm(), (option() & 0x3) == 0x3);

        unsigned shift = sBit() ? scale : 0;

        if (option() == 0x3) {
            if (shift) {
                appendSeparator();
                appendString("lsl ");
                appendUnsignedImmediate(shift);
            }
        } else {
            appendSeparator();
            appendString(optionName());
            if (shift)
                appendUnsignedImmediate(shift);
        }
    }

    appendCharacter(']');

    return m_formatBuffer;
}

const char* A64DOpcodeLoadStoreRegisterPair::opName()
{
    if (!vBit() && lBit() && size() == 0x1)
        return "ldpsw";
    if (lBit())
        return "ldp";
    return "stp";
}

const char* A64DOpcodeLoadStoreRegisterPair::format()
{
    const char* thisOpName = opName();
    
    if (size() == 0x3)
        return A64DOpcode::format();

    if ((offsetMode() < 0x1) || (offsetMode() > 0x3))
        return A64DOpcode::format();

    if ((offsetMode() == 0x1) && !vBit() && !lBit())
        return A64DOpcode::format();

    appendInstructionName(thisOpName);
    unsigned offsetShift;
    if (vBit()) {
        appendFPRegisterName(rt(), size());
        appendSeparator();
        appendFPRegisterName(rt2(), size());
        offsetShift = size() + 2;
    } else {
        if (!lBit())
            appendZROrRegisterName(rt(), is64Bit());
        else
            appendRegisterName(rt(), is64Bit());
        appendSeparator();
        if (!lBit())
            appendZROrRegisterName(rt2(), is64Bit());
        else
            appendRegisterName(rt2(), is64Bit());
        offsetShift = (size() >> 1) + 2;
    }

    appendSeparator();
    appendCharacter('[');
    appendSPOrRegisterName(rn());

    int offset = immediate7() << offsetShift;

    if (offsetMode() == 1) {
        appendCharacter(']');
        appendSeparator();
        appendSignedImmediate(offset);
    } else {
        appendSeparator();
        appendSignedImmediate(offset);
        appendCharacter(']');
        if (offsetMode() == 0x3)
            appendCharacter('!');
    }

    return m_formatBuffer;
}

const char* A64DOpcodeLoadStoreUnsignedImmediate::format()
{
    const char* thisOpName = opName();

    if (!thisOpName)
        return A64DOpcode::format();

    appendInstructionName(thisOpName);
    unsigned scale;
    if (vBit()) {
        appendFPRegisterName(rt(), size());
        scale = ((opc() & 2)<<1) | size();
    } else {
        if (!opc())
            appendZROrRegisterName(rt(), is64BitRT());
        else
            appendRegisterName(rt(), is64BitRT());
        scale = size();
    }
    appendSeparator();
    appendCharacter('[');
    appendSPOrRegisterName(rn());

    if (immediate12()) {
        appendSeparator();
        appendUnsignedImmediate(immediate12() << scale);
    }

    appendCharacter(']');

    return m_formatBuffer;
}

// A zero in an entry of the table means the instruction is Unallocated
const char* const A64DOpcodeLogical::s_opNames[8] = {
    "and", "bic", "orr", "orn", "eor", "eon", "ands", "bics"
};

const char* A64DOpcodeLogicalShiftedRegister::format()
{
    if (!is64Bit() && immediate6() & 0x20)
        return A64DOpcode::format();

    if (isTst())
        appendInstructionName("tst");
    else {
        if (isMov())
            appendInstructionName(nBit() ? "mvn" : "mov");
        else
            appendInstructionName(opName(opNumber()));
        appendZROrRegisterName(rd(), is64Bit());
        appendSeparator();
    }

    if (!isMov()) {
        appendZROrRegisterName(rn(), is64Bit());
        appendSeparator();
    }

    appendZROrRegisterName(rm(), is64Bit());
    if (immediate6()) {
        appendSeparator();
        appendShiftType(shift());
        appendUnsignedImmediate(immediate6());
    }

    return m_formatBuffer;
}

static unsigned highestBitSet(unsigned value)
{
    unsigned result = 0;

    while (value >>= 1)
        result++;

    return result;
}

static uint64_t rotateRight(uint64_t value, unsigned width, unsigned shift)
{
    uint64_t result = value;

    if (shift)
        result = (value >> (shift % width)) | (value << (width - shift));

    return result;
}

static uint64_t replicate(uint64_t value, unsigned width)
{
    uint64_t result = 0;

    for (unsigned totalBits = 0; totalBits < 64; totalBits += width)
        result = (result << width) | value;

    return result;
}

const char* A64DOpcodeLogicalImmediate::format()
{
    if (!is64Bit() && nBit())
        return A64DOpcode::format();

    unsigned len = highestBitSet(nBit() << 6 | (immediateS() ^ 0x3f));
    unsigned levels = (1 << len) - 1; // len number of 1 bits starting at LSB

    if ((immediateS() & levels) == levels)
        return A64DOpcode::format();

    unsigned r = immediateR() & levels;
    unsigned s = immediateS() & levels;
    unsigned eSize = 1 << len;
    uint64_t pattern = rotateRight((1ull << (s + 1)) - 1, eSize, r);

    uint64_t immediate = replicate(pattern, eSize);

    if (!is64Bit())
        immediate &= 0xffffffffull;

    if (isTst())
        appendInstructionName("tst");
    else {
        if (isMov())
            appendInstructionName("mov");
        else
            appendInstructionName(opName(opNumber()));
        appendRegisterName(rd(), is64Bit());
        appendSeparator();
    }
    if (!isMov()) {
        appendRegisterName(rn(), is64Bit());
        appendSeparator();
    }
    appendUnsignedImmediate64(immediate);

    return m_formatBuffer;
}

const char* const A64DOpcodeMoveWide::s_opNames[4] = { "movn", 0, "movz", "movk" };

const char* A64DOpcodeMoveWide::format()
{
    if (opc() == 1)
        return A64DOpcode::format();
    if (!is64Bit() && hw() >= 2)
        return A64DOpcode::format();

    if (!opc() && (!immediate16() || !hw()) && (is64Bit() || immediate16() != 0xffff)) {
        // MOV pseudo op for MOVN
        appendInstructionName("mov");
        appendRegisterName(rd(), is64Bit());
        appendSeparator();

        if (is64Bit()) {
            int64_t amount = immediate16() << (hw() * 16);
            amount = ~amount;
            appendSignedImmediate64(amount);
        } else {
            int32_t amount = immediate16() << (hw() * 16);
            amount = ~amount;
            appendSignedImmediate(amount);
        }
    } else {
        appendInstructionName(opName());
        appendRegisterName(rd(), is64Bit());
        appendSeparator();
        appendUnsignedHexImmediate(immediate16());
        if (hw()) {
            appendSeparator();
            appendShiftAmount(hw());
        }
    }

    return m_formatBuffer;
}

const char* A64DOpcodeTestAndBranchImmediate::format()
{
    appendInstructionName(opBit() ? "tbnz" : "tbz");
    appendRegisterName(rt());
    appendSeparator();
    appendUnsignedImmediate(bitNumber());
    appendSeparator();
    appendPCRelativeOffset(m_currentPC, static_cast<int32_t>(immediate14()));
    return m_formatBuffer;
}

const char* A64DOpcodeUnconditionalBranchImmediate::format()
{
    appendInstructionName(op() ? "bl" : "b");
    appendPCRelativeOffset(m_currentPC, static_cast<int32_t>(immediate26()));
    return m_formatBuffer;
}

const char* const A64DOpcodeUnconditionalBranchRegister::s_opNames[8] = { "br", "blr", "ret", "", "eret", "drps", "", "" };

const char* A64DOpcodeUnconditionalBranchRegister::format()
{
    unsigned opcValue = opc();
    if (opcValue == 3 || opcValue > 5)
        return A64DOpcode::format();
    if (((opcValue & 0xe) == 0x4) && rn() != 0x1f)
        return A64DOpcode::format();
    appendInstructionName(opName());
    if (opcValue <= 2)
        appendRegisterName(rn());
    return m_formatBuffer;
}

} } // namespace JSC::ARM64Disassembler

#endif // USE(ARM64_DISASSEMBLER)
