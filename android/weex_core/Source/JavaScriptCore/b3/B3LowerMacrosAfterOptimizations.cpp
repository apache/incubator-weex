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
#include "B3LowerMacrosAfterOptimizations.h"

#if ENABLE(B3_JIT)

#include "B3BasicBlockInlines.h"
#include "B3BlockInsertionSet.h"
#include "B3CCallValue.h"
#include "B3ConstDoubleValue.h"
#include "B3ConstFloatValue.h"
#include "B3ConstPtrValue.h"
#include "B3InsertionSetInlines.h"
#include "B3PhaseScope.h"

namespace JSC { namespace B3 {

namespace {

class LowerMacros {
public:
    LowerMacros(Procedure& proc)
        : m_proc(proc)
        , m_blockInsertionSet(proc)
        , m_insertionSet(proc)
    {
    }

    bool run()
    {
        for (BasicBlock* block : m_proc) {
            m_block = block;
            processCurrentBlock();
        }
        m_changed |= m_blockInsertionSet.execute();
        if (m_changed) {
            m_proc.resetReachability();
            m_proc.invalidateCFG();
        }
        return m_changed;
    }
    
private:
    void processCurrentBlock()
    {
        for (m_index = 0; m_index < m_block->size(); ++m_index) {
            m_value = m_block->at(m_index);
            m_origin = m_value->origin();
            switch (m_value->opcode()) {
            case Abs: {
                // ARM supports this instruction natively.
                if (isARM64())
                    break;

                Value* mask = nullptr;
                if (m_value->type() == Double)
                    mask = m_insertionSet.insert<ConstDoubleValue>(m_index, m_origin, bitwise_cast<double>(~(1ll << 63)));
                else if (m_value->type() == Float)
                    mask = m_insertionSet.insert<ConstFloatValue>(m_index, m_origin, bitwise_cast<float>(~(1 << 31)));
                else
                    RELEASE_ASSERT_NOT_REACHED();
                Value* result = m_insertionSet.insert<Value>(m_index, BitAnd, m_origin, m_value->child(0), mask);
                m_value->replaceWithIdentity(result);
                break;
            }
            case Ceil: {
                if (MacroAssembler::supportsFloatingPointRounding())
                    break;

                Value* functionAddress = nullptr;
                if (m_value->type() == Double) {
                    double (*ceilDouble)(double) = ceil;
                    functionAddress = m_insertionSet.insert<ConstPtrValue>(m_index, m_origin, ceilDouble);
                } else if (m_value->type() == Float)
                    functionAddress = m_insertionSet.insert<ConstPtrValue>(m_index, m_origin, ceilf);
                else
                    RELEASE_ASSERT_NOT_REACHED();

                Value* result = m_insertionSet.insert<CCallValue>(m_index,
                    m_value->type(),
                    m_origin,
                    Effects::none(),
                    functionAddress,
                    m_value->child(0));
                m_value->replaceWithIdentity(result);
                break;
            }
            case Floor: {
                if (MacroAssembler::supportsFloatingPointRounding())
                    break;

                Value* functionAddress = nullptr;
                if (m_value->type() == Double) {
                    double (*floorDouble)(double) = floor;
                    functionAddress = m_insertionSet.insert<ConstPtrValue>(m_index, m_origin, floorDouble);
                } else if (m_value->type() == Float)
                    functionAddress = m_insertionSet.insert<ConstPtrValue>(m_index, m_origin, floorf);
                else
                    RELEASE_ASSERT_NOT_REACHED();

                Value* result = m_insertionSet.insert<CCallValue>(m_index,
                    m_value->type(),
                    m_origin,
                    Effects::none(),
                    functionAddress,
                    m_value->child(0));
                m_value->replaceWithIdentity(result);
                break;
            }
            case Neg: {
                if (!isFloat(m_value->type()))
                    break;
                
                // X86 is odd in that it requires this.
                if (!isX86())
                    break;

                Value* mask = nullptr;
                if (m_value->type() == Double)
                    mask = m_insertionSet.insert<ConstDoubleValue>(m_index, m_origin, -0.0);
                else {
                    RELEASE_ASSERT(m_value->type() == Float);
                    mask = m_insertionSet.insert<ConstFloatValue>(m_index, m_origin, -0.0f);
                }

                Value* result = m_insertionSet.insert<Value>(
                    m_index, BitXor, m_origin, m_value->child(0), mask);
                m_value->replaceWithIdentity(result);
                break;
            }

            case RotL: {
                // ARM64 doesn't have a rotate left.
                if (isARM64()) {
                    Value* newShift = m_insertionSet.insert<Value>(m_index, Neg, m_value->origin(), m_value->child(1));
                    Value* rotate = m_insertionSet.insert<Value>(m_index, RotR, m_value->origin(), m_value->child(0), newShift);
                    m_value->replaceWithIdentity(rotate);
                    break;
                }
                break;
            }
                
            default:
                break;
            }
        }
        m_insertionSet.execute(m_block);
    }
    
    Procedure& m_proc;
    BlockInsertionSet m_blockInsertionSet;
    InsertionSet m_insertionSet;
    BasicBlock* m_block;
    unsigned m_index;
    Value* m_value;
    Origin m_origin;
    bool m_changed { false };
};

bool lowerMacrosImpl(Procedure& proc)
{
    LowerMacros lowerMacros(proc);
    return lowerMacros.run();
}

} // anonymous namespace

bool lowerMacrosAfterOptimizations(Procedure& proc)
{
    PhaseScope phaseScope(proc, "lowerMacrosAfterOptimizations");
    bool result = lowerMacrosImpl(proc);
    if (shouldValidateIR())
        RELEASE_ASSERT(!lowerMacrosImpl(proc));
    return result;
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

