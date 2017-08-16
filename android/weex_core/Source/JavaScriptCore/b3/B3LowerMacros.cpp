/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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
#include "B3LowerMacros.h"

#if ENABLE(B3_JIT)

#include "AllowMacroScratchRegisterUsage.h"
#include "B3AtomicValue.h"
#include "B3BasicBlockInlines.h"
#include "B3BlockInsertionSet.h"
#include "B3CCallValue.h"
#include "B3CaseCollectionInlines.h"
#include "B3ConstPtrValue.h"
#include "B3FenceValue.h"
#include "B3InsertionSetInlines.h"
#include "B3MemoryValueInlines.h"
#include "B3PatchpointValue.h"
#include "B3PhaseScope.h"
#include "B3ProcedureInlines.h"
#include "B3StackmapGenerationParams.h"
#include "B3SwitchValue.h"
#include "B3UpsilonValue.h"
#include "B3UseCounts.h"
#include "B3ValueInlines.h"
#include "CCallHelpers.h"
#include "LinkBuffer.h"
#include <cmath>
#include <wtf/BitVector.h>

namespace JSC { namespace B3 {

namespace {

class LowerMacros {
public:
    LowerMacros(Procedure& proc)
        : m_proc(proc)
        , m_blockInsertionSet(proc)
        , m_insertionSet(proc)
        , m_useCounts(proc)
    {
    }

    bool run()
    {
        RELEASE_ASSERT(!m_proc.hasQuirks());
        
        for (BasicBlock* block : m_proc) {
            m_block = block;
            processCurrentBlock();
        }
        m_changed |= m_blockInsertionSet.execute();
        if (m_changed) {
            m_proc.resetReachability();
            m_proc.invalidateCFG();
        }
        
        // This indicates that we've 
        m_proc.setHasQuirks(true);
        
        return m_changed;
    }
    
private:
    void processCurrentBlock()
    {
        for (m_index = 0; m_index < m_block->size(); ++m_index) {
            m_value = m_block->at(m_index);
            m_origin = m_value->origin();
            switch (m_value->opcode()) {
            case Mod: {
                if (m_value->isChill()) {
                    if (isARM64()) {
                        BasicBlock* before = m_blockInsertionSet.splitForward(m_block, m_index, &m_insertionSet);
                        BasicBlock* zeroDenCase = m_blockInsertionSet.insertBefore(m_block);
                        BasicBlock* normalModCase = m_blockInsertionSet.insertBefore(m_block);

                        before->replaceLastWithNew<Value>(m_proc, Branch, m_origin, m_value->child(1));
                        before->setSuccessors(
                            FrequentedBlock(normalModCase, FrequencyClass::Normal),
                            FrequentedBlock(zeroDenCase, FrequencyClass::Rare));

                        Value* divResult = normalModCase->appendNew<Value>(m_proc, chill(Div), m_origin, m_value->child(0), m_value->child(1));
                        Value* multipliedBack = normalModCase->appendNew<Value>(m_proc, Mul, m_origin, divResult, m_value->child(1));
                        Value* result = normalModCase->appendNew<Value>(m_proc, Sub, m_origin, m_value->child(0), multipliedBack);
                        UpsilonValue* normalResult = normalModCase->appendNew<UpsilonValue>(m_proc, m_origin, result);
                        normalModCase->appendNew<Value>(m_proc, Jump, m_origin);
                        normalModCase->setSuccessors(FrequentedBlock(m_block));

                        UpsilonValue* zeroResult = zeroDenCase->appendNew<UpsilonValue>(
                            m_proc, m_origin,
                            zeroDenCase->appendIntConstant(m_proc, m_value, 0));
                        zeroDenCase->appendNew<Value>(m_proc, Jump, m_origin);
                        zeroDenCase->setSuccessors(FrequentedBlock(m_block));

                        Value* phi = m_insertionSet.insert<Value>(m_index, Phi, m_value->type(), m_origin);
                        normalResult->setPhi(phi);
                        zeroResult->setPhi(phi);
                        m_value->replaceWithIdentity(phi);
                        before->updatePredecessorsAfter();
                        m_changed = true;
                    } else
                        makeDivisionChill(Mod);
                    break;
                }
                
                double (*fmodDouble)(double, double) = fmod;
                if (m_value->type() == Double) {
                    Value* functionAddress = m_insertionSet.insert<ConstPtrValue>(m_index, m_origin, fmodDouble);
                    Value* result = m_insertionSet.insert<CCallValue>(m_index, Double, m_origin,
                        Effects::none(),
                        functionAddress,
                        m_value->child(0),
                        m_value->child(1));
                    m_value->replaceWithIdentity(result);
                    m_changed = true;
                } else if (m_value->type() == Float) {
                    Value* numeratorAsDouble = m_insertionSet.insert<Value>(m_index, FloatToDouble, m_origin, m_value->child(0));
                    Value* denominatorAsDouble = m_insertionSet.insert<Value>(m_index, FloatToDouble, m_origin, m_value->child(1));
                    Value* functionAddress = m_insertionSet.insert<ConstPtrValue>(m_index, m_origin, fmodDouble);
                    Value* doubleMod = m_insertionSet.insert<CCallValue>(m_index, Double, m_origin,
                        Effects::none(),
                        functionAddress,
                        numeratorAsDouble,
                        denominatorAsDouble);
                    Value* result = m_insertionSet.insert<Value>(m_index, DoubleToFloat, m_origin, doubleMod);
                    m_value->replaceWithIdentity(result);
                    m_changed = true;
                } else if (isARM64()) {
                    Value* divResult = m_insertionSet.insert<Value>(m_index, chill(Div), m_origin, m_value->child(0), m_value->child(1));
                    Value* multipliedBack = m_insertionSet.insert<Value>(m_index, Mul, m_origin, divResult, m_value->child(1));
                    Value* result = m_insertionSet.insert<Value>(m_index, Sub, m_origin, m_value->child(0), multipliedBack);
                    m_value->replaceWithIdentity(result);
                    m_changed = true;
                }
                break;
            }

            case UMod: {
                if (isARM64()) {
                    Value* divResult = m_insertionSet.insert<Value>(m_index, UDiv, m_origin, m_value->child(0), m_value->child(1));
                    Value* multipliedBack = m_insertionSet.insert<Value>(m_index, Mul, m_origin, divResult, m_value->child(1));
                    Value* result = m_insertionSet.insert<Value>(m_index, Sub, m_origin, m_value->child(0), multipliedBack);
                    m_value->replaceWithIdentity(result);
                    m_changed = true;
                }
                break;
            }

            case Div: {
                if (m_value->isChill())
                    makeDivisionChill(Div);
                break;
            }

            case Switch: {
                SwitchValue* switchValue = m_value->as<SwitchValue>();
                Vector<SwitchCase> cases;
                for (const SwitchCase& switchCase : switchValue->cases(m_block))
                    cases.append(switchCase);
                std::sort(
                    cases.begin(), cases.end(),
                    [] (const SwitchCase& left, const SwitchCase& right) {
                        return left.caseValue() < right.caseValue();
                    });
                FrequentedBlock fallThrough = m_block->fallThrough();
                m_block->values().removeLast();
                recursivelyBuildSwitch(cases, fallThrough, 0, false, cases.size(), m_block);
                m_proc.deleteValue(switchValue);
                m_block->updatePredecessorsAfter();
                m_changed = true;
                break;
            }
                
            case Depend: {
                if (isX86()) {
                    // Create a load-load fence. This codegens to nothing on X86. We use it to tell the
                    // compiler not to block load motion.
                    FenceValue* fence = m_insertionSet.insert<FenceValue>(m_index, m_origin);
                    fence->read = HeapRange();
                    fence->write = HeapRange::top();
                    
                    // Kill the Depend, which should unlock a bunch of code simplification.
                    m_value->replaceWithBottom(m_insertionSet, m_index);
                    
                    m_changed = true;
                }
                break;
            }

            case AtomicWeakCAS:
            case AtomicStrongCAS: {
                AtomicValue* atomic = m_value->as<AtomicValue>();
                Width width = atomic->accessWidth();
                
                if (isCanonicalWidth(width))
                    break;
                
                Value* expectedValue = atomic->child(0);
                
                if (!isX86()) {
                    // On ARM, the load part of the CAS does a load with zero extension. Therefore, we need
                    // to zero-extend the input.
                    Value* maskedExpectedValue = m_insertionSet.insert<Value>(
                        m_index, BitAnd, m_origin, expectedValue,
                        m_insertionSet.insertIntConstant(m_index, expectedValue, mask(width)));
                    
                    atomic->child(0) = maskedExpectedValue;
                }
                
                if (atomic->opcode() == AtomicStrongCAS) {
                    Value* newValue = m_insertionSet.insert<Value>(
                        m_index, signExtendOpcode(width), m_origin,
                        m_insertionSet.insertClone(m_index, atomic));
                    
                    atomic->replaceWithIdentity(newValue);
                }
                
                m_changed = true;
                break;
            }
                
            case AtomicXchgAdd:
            case AtomicXchgAnd:
            case AtomicXchgOr:
            case AtomicXchgSub:
            case AtomicXchgXor:
            case AtomicXchg: {
                // On X86, these may actually return garbage in the high bits. On ARM64, these sorta
                // zero-extend their high bits, except that the high bits might get polluted by high
                // bits in the operand. So, either way, we need to throw a sign-extend on these
                // things.
                
                if (isX86()) {
                    if (m_value->opcode() == AtomicXchgSub && m_useCounts.numUses(m_value)) {
                        // On x86, xchgadd is better than xchgsub if it has any users.
                        m_value->setOpcodeUnsafely(AtomicXchgAdd);
                        m_value->child(0) = m_insertionSet.insert<Value>(
                            m_index, Neg, m_origin, m_value->child(0));
                    }
                    
                    bool exempt = false;
                    switch (m_value->opcode()) {
                    case AtomicXchgAnd:
                    case AtomicXchgOr:
                    case AtomicXchgSub:
                    case AtomicXchgXor:
                        exempt = true;
                        break;
                    default:
                        break;
                    }
                    if (exempt)
                        break;
                }
                
                AtomicValue* atomic = m_value->as<AtomicValue>();
                Width width = atomic->accessWidth();
                
                if (isCanonicalWidth(width))
                    break;
                
                Value* newValue = m_insertionSet.insert<Value>(
                    m_index, signExtendOpcode(width), m_origin,
                    m_insertionSet.insertClone(m_index, atomic));
                
                atomic->replaceWithIdentity(newValue);
                m_changed = true;
                break;
            }
                
            case Load8Z:
            case Load16Z: {
                if (isX86())
                    break;
                
                MemoryValue* memory = m_value->as<MemoryValue>();
                if (!memory->hasFence())
                    break;
                
                // Sub-width load-acq on ARM64 always sign extends.
                Value* newLoad = m_insertionSet.insertClone(m_index, memory);
                newLoad->setOpcodeUnsafely(memory->opcode() == Load8Z ? Load8S : Load16S);
                
                Value* newValue = m_insertionSet.insert<Value>(
                    m_index, BitAnd, m_origin, newLoad,
                    m_insertionSet.insertIntConstant(
                        m_index, m_origin, Int32, mask(memory->accessWidth())));

                m_value->replaceWithIdentity(newValue);
                m_changed = true;
                break;
            }
                
            default:
                break;
            }
        }
        m_insertionSet.execute(m_block);
    }

    void makeDivisionChill(Opcode nonChillOpcode)
    {
        ASSERT(nonChillOpcode == Div || nonChillOpcode == Mod);

        // ARM supports this instruction natively.
        if (isARM64())
            return;

        // We implement "res = Div<Chill>/Mod<Chill>(num, den)" as follows:
        //
        //     if (den + 1 <=_unsigned 1) {
        //         if (!den) {
        //             res = 0;
        //             goto done;
        //         }
        //         if (num == -2147483648) {
        //             res = isDiv ? num : 0;
        //             goto done;
        //         }
        //     }
        //     res = num (/ or %) dev;
        // done:
        m_changed = true;

        Value* num = m_value->child(0);
        Value* den = m_value->child(1);

        Value* one = m_insertionSet.insertIntConstant(m_index, m_value, 1);
        Value* isDenOK = m_insertionSet.insert<Value>(
            m_index, Above, m_origin,
            m_insertionSet.insert<Value>(m_index, Add, m_origin, den, one),
            one);

        BasicBlock* before = m_blockInsertionSet.splitForward(m_block, m_index, &m_insertionSet);

        BasicBlock* normalDivCase = m_blockInsertionSet.insertBefore(m_block);
        BasicBlock* shadyDenCase = m_blockInsertionSet.insertBefore(m_block);
        BasicBlock* zeroDenCase = m_blockInsertionSet.insertBefore(m_block);
        BasicBlock* neg1DenCase = m_blockInsertionSet.insertBefore(m_block);
        BasicBlock* intMinCase = m_blockInsertionSet.insertBefore(m_block);

        before->replaceLastWithNew<Value>(m_proc, Branch, m_origin, isDenOK);
        before->setSuccessors(
            FrequentedBlock(normalDivCase, FrequencyClass::Normal),
            FrequentedBlock(shadyDenCase, FrequencyClass::Rare));

        UpsilonValue* normalResult = normalDivCase->appendNew<UpsilonValue>(
            m_proc, m_origin,
            normalDivCase->appendNew<Value>(m_proc, nonChillOpcode, m_origin, num, den));
        normalDivCase->appendNew<Value>(m_proc, Jump, m_origin);
        normalDivCase->setSuccessors(FrequentedBlock(m_block));

        shadyDenCase->appendNew<Value>(m_proc, Branch, m_origin, den);
        shadyDenCase->setSuccessors(
            FrequentedBlock(neg1DenCase, FrequencyClass::Normal),
            FrequentedBlock(zeroDenCase, FrequencyClass::Rare));

        UpsilonValue* zeroResult = zeroDenCase->appendNew<UpsilonValue>(
            m_proc, m_origin,
            zeroDenCase->appendIntConstant(m_proc, m_value, 0));
        zeroDenCase->appendNew<Value>(m_proc, Jump, m_origin);
        zeroDenCase->setSuccessors(FrequentedBlock(m_block));

        int64_t badNumeratorConst = 0;
        switch (m_value->type()) {
        case Int32:
            badNumeratorConst = std::numeric_limits<int32_t>::min();
            break;
        case Int64:
            badNumeratorConst = std::numeric_limits<int64_t>::min();
            break;
        default:
            ASSERT_NOT_REACHED();
            badNumeratorConst = 0;
        }

        Value* badNumerator =
            neg1DenCase->appendIntConstant(m_proc, m_value, badNumeratorConst);

        neg1DenCase->appendNew<Value>(
            m_proc, Branch, m_origin,
            neg1DenCase->appendNew<Value>(
                m_proc, Equal, m_origin, num, badNumerator));
        neg1DenCase->setSuccessors(
            FrequentedBlock(intMinCase, FrequencyClass::Rare),
            FrequentedBlock(normalDivCase, FrequencyClass::Normal));

        Value* intMinResult = nonChillOpcode == Div ? badNumerator : intMinCase->appendIntConstant(m_proc, m_value, 0);
        UpsilonValue* intMinResultUpsilon = intMinCase->appendNew<UpsilonValue>(
            m_proc, m_origin, intMinResult);
        intMinCase->appendNew<Value>(m_proc, Jump, m_origin);
        intMinCase->setSuccessors(FrequentedBlock(m_block));

        Value* phi = m_insertionSet.insert<Value>(
            m_index, Phi, m_value->type(), m_origin);
        normalResult->setPhi(phi);
        zeroResult->setPhi(phi);
        intMinResultUpsilon->setPhi(phi);

        m_value->replaceWithIdentity(phi);
        before->updatePredecessorsAfter();
    }

    void recursivelyBuildSwitch(
        const Vector<SwitchCase>& cases, FrequentedBlock fallThrough, unsigned start, bool hardStart,
        unsigned end, BasicBlock* before)
    {
        Value* child = m_value->child(0);
        Type type = child->type();
        
        // It's a good idea to use a table-based switch in some cases: the number of cases has to be
        // large enough and they have to be dense enough. This could probably be improved a lot. For
        // example, we could still use a jump table in cases where the inputs are sparse so long as we
        // shift off the uninteresting bits. On the other hand, it's not clear that this would
        // actually be any better than what we have done here and it's not clear that it would be
        // better than a binary switch.
        const unsigned minCasesForTable = 7;
        const unsigned densityLimit = 4;
        if (end - start >= minCasesForTable) {
            int64_t firstValue = cases[start].caseValue();
            int64_t lastValue = cases[end - 1].caseValue();
            if ((lastValue - firstValue + 1) / (end - start) < densityLimit) {
                BasicBlock* switchBlock = m_blockInsertionSet.insertAfter(m_block);
                Value* index = before->appendNew<Value>(
                    m_proc, Sub, m_origin, child,
                    before->appendIntConstant(m_proc, m_origin, type, firstValue));
                before->appendNew<Value>(
                    m_proc, Branch, m_origin,
                    before->appendNew<Value>(
                        m_proc, Above, m_origin, index,
                        before->appendIntConstant(m_proc, m_origin, type, lastValue - firstValue)));
                before->setSuccessors(fallThrough, FrequentedBlock(switchBlock));
                
                size_t tableSize = lastValue - firstValue + 1;
                
                if (index->type() != pointerType() && index->type() == Int32)
                    index = switchBlock->appendNew<Value>(m_proc, ZExt32, m_origin, index);
                
                PatchpointValue* patchpoint =
                    switchBlock->appendNew<PatchpointValue>(m_proc, Void, m_origin);

                // Even though this loads from the jump table, the jump table is immutable. For the
                // purpose of alias analysis, reading something immutable is like reading nothing.
                patchpoint->effects = Effects();
                patchpoint->effects.terminal = true;
                
                patchpoint->appendSomeRegister(index);
                patchpoint->numGPScratchRegisters++;
                // Technically, we don't have to clobber macro registers on X86_64. This is probably
                // OK though.
                patchpoint->clobber(RegisterSet::macroScratchRegisters());
                
                BitVector handledIndices;
                for (unsigned i = start; i < end; ++i) {
                    FrequentedBlock block = cases[i].target();
                    int64_t value = cases[i].caseValue();
                    switchBlock->appendSuccessor(block);
                    size_t index = value - firstValue;
                    ASSERT(!handledIndices.get(index));
                    handledIndices.set(index);
                }
                
                bool hasUnhandledIndex = false;
                for (unsigned i = 0; i < tableSize; ++i) {
                    if (!handledIndices.get(i)) {
                        hasUnhandledIndex = true;
                        break;
                    }
                }
                
                if (hasUnhandledIndex)
                    switchBlock->appendSuccessor(fallThrough);

                patchpoint->setGenerator(
                    [=] (CCallHelpers& jit, const StackmapGenerationParams& params) {
                        AllowMacroScratchRegisterUsage allowScratch(jit);
                        
                        MacroAssemblerCodePtr* jumpTable = static_cast<MacroAssemblerCodePtr*>(
                            params.proc().addDataSection(sizeof(MacroAssemblerCodePtr) * tableSize));
                        
                        GPRReg index = params[0].gpr();
                        GPRReg scratch = params.gpScratch(0);
                        
                        jit.move(CCallHelpers::TrustedImmPtr(jumpTable), scratch);
                        jit.jump(CCallHelpers::BaseIndex(scratch, index, CCallHelpers::timesPtr()));
                        
                        // These labels are guaranteed to be populated before either late paths or
                        // link tasks run.
                        Vector<Box<CCallHelpers::Label>> labels = params.successorLabels();
                        
                        jit.addLinkTask(
                            [=] (LinkBuffer& linkBuffer) {
                                if (hasUnhandledIndex) {
                                    MacroAssemblerCodePtr fallThrough =
                                        linkBuffer.locationOf(*labels.last());
                                    for (unsigned i = tableSize; i--;)
                                        jumpTable[i] = fallThrough;
                                }
                                
                                unsigned labelIndex = 0;
                                for (unsigned tableIndex : handledIndices) {
                                    jumpTable[tableIndex] =
                                        linkBuffer.locationOf(*labels[labelIndex++]);
                                }
                            });
                    });
                return;
            }
        }
        
        // See comments in jit/BinarySwitch.cpp for a justification of this algorithm. The only
        // thing we do differently is that we don't use randomness.

        const unsigned leafThreshold = 3;

        unsigned size = end - start;

        if (size <= leafThreshold) {
            bool allConsecutive = false;

            if ((hardStart || (start && cases[start - 1].caseValue() == cases[start].caseValue() - 1))
                && end < cases.size()
                && cases[end - 1].caseValue() == cases[end].caseValue() - 1) {
                allConsecutive = true;
                for (unsigned i = 0; i < size - 1; ++i) {
                    if (cases[start + i].caseValue() + 1 != cases[start + i + 1].caseValue()) {
                        allConsecutive = false;
                        break;
                    }
                }
            }

            unsigned limit = allConsecutive ? size - 1 : size;
            
            for (unsigned i = 0; i < limit; ++i) {
                BasicBlock* nextCheck = m_blockInsertionSet.insertAfter(m_block);
                before->appendNew<Value>(
                    m_proc, Branch, m_origin,
                    before->appendNew<Value>(
                        m_proc, Equal, m_origin, child,
                        before->appendIntConstant(
                            m_proc, m_origin, type,
                            cases[start + i].caseValue())));
                before->setSuccessors(cases[start + i].target(), FrequentedBlock(nextCheck));

                before = nextCheck;
            }

            before->appendNew<Value>(m_proc, Jump, m_origin);
            if (allConsecutive)
                before->setSuccessors(cases[end - 1].target());
            else
                before->setSuccessors(fallThrough);
            return;
        }

        unsigned medianIndex = (start + end) / 2;

        BasicBlock* left = m_blockInsertionSet.insertAfter(m_block);
        BasicBlock* right = m_blockInsertionSet.insertAfter(m_block);

        before->appendNew<Value>(
            m_proc, Branch, m_origin,
            before->appendNew<Value>(
                m_proc, LessThan, m_origin, child,
                before->appendIntConstant(
                    m_proc, m_origin, type,
                    cases[medianIndex].caseValue())));
        before->setSuccessors(FrequentedBlock(left), FrequentedBlock(right));

        recursivelyBuildSwitch(cases, fallThrough, start, hardStart, medianIndex, left);
        recursivelyBuildSwitch(cases, fallThrough, medianIndex, true, end, right);
    }
    
    Procedure& m_proc;
    BlockInsertionSet m_blockInsertionSet;
    InsertionSet m_insertionSet;
    UseCounts m_useCounts;
    BasicBlock* m_block;
    unsigned m_index;
    Value* m_value;
    Origin m_origin;
    bool m_changed { false };
};

} // anonymous namespace

bool lowerMacros(Procedure& proc)
{
    LowerMacros lowerMacros(proc);
    return lowerMacros.run();
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

