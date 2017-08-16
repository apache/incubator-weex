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
#include "B3Procedure.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "B3BasicBlockInlines.h"
#include "B3BasicBlockUtils.h"
#include "B3BlockWorklist.h"
#include "B3CFG.h"
#include "B3DataSection.h"
#include "B3Dominators.h"
#include "B3OpaqueByproducts.h"
#include "B3PhiChildren.h"
#include "B3StackSlot.h"
#include "B3ValueInlines.h"
#include "B3Variable.h"

namespace JSC { namespace B3 {

Procedure::Procedure()
    : m_cfg(new CFG(*this))
    , m_lastPhaseName("initial")
    , m_byproducts(std::make_unique<OpaqueByproducts>())
    , m_code(new Air::Code(*this))
{
}

Procedure::~Procedure()
{
}

void Procedure::printOrigin(PrintStream& out, Origin origin) const
{
    if (m_originPrinter)
        m_originPrinter->run(out, origin);
    else
        out.print(origin);
}

BasicBlock* Procedure::addBlock(double frequency)
{
    std::unique_ptr<BasicBlock> block(new BasicBlock(m_blocks.size(), frequency));
    BasicBlock* result = block.get();
    m_blocks.append(WTFMove(block));
    return result;
}

StackSlot* Procedure::addStackSlot(unsigned byteSize)
{
    return m_stackSlots.addNew(byteSize);
}

Variable* Procedure::addVariable(Type type)
{
    return m_variables.addNew(type); 
}

Value* Procedure::clone(Value* value)
{
    std::unique_ptr<Value> clone(value->cloneImpl());
    clone->m_index = UINT_MAX;
    clone->owner = nullptr;
    return m_values.add(WTFMove(clone));
}

Value* Procedure::addIntConstant(Origin origin, Type type, int64_t value)
{
    switch (type) {
    case Int32:
        return add<Const32Value>(origin, static_cast<int32_t>(value));
    case Int64:
        return add<Const64Value>(origin, value);
    case Double:
        return add<ConstDoubleValue>(origin, static_cast<double>(value));
    case Float:
        return add<ConstFloatValue>(origin, static_cast<float>(value));
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return nullptr;
    }
}

Value* Procedure::addIntConstant(Value* likeValue, int64_t value)
{
    return addIntConstant(likeValue->origin(), likeValue->type(), value);
}

Value* Procedure::addBottom(Origin origin, Type type)
{
    return addIntConstant(origin, type, 0);
}

Value* Procedure::addBottom(Value* value)
{
    return addBottom(value->origin(), value->type());
}

Value* Procedure::addBoolConstant(Origin origin, TriState triState)
{
    int32_t value = 0;
    switch (triState) {
    case FalseTriState:
        value = 0;
        break;
    case TrueTriState:
        value = 1;
        break;
    case MixedTriState:
        return nullptr;
    }

    return addIntConstant(origin, Int32, value);
}

void Procedure::resetValueOwners()
{
    for (BasicBlock* block : *this) {
        for (Value* value : *block)
            value->owner = block;
    }
}

void Procedure::resetReachability()
{
    recomputePredecessors(m_blocks);
    
    // The common case is that this does not find any dead blocks.
    bool foundDead = false;
    for (auto& block : m_blocks) {
        if (isBlockDead(block.get())) {
            foundDead = true;
            break;
        }
    }
    if (!foundDead)
        return;
    
    resetValueOwners();

    for (Value* value : values()) {
        if (UpsilonValue* upsilon = value->as<UpsilonValue>()) {
            if (isBlockDead(upsilon->phi()->owner))
                upsilon->replaceWithNop();
        }
    }
    
    for (auto& block : m_blocks) {
        if (isBlockDead(block.get())) {
            for (Value* value : *block)
                deleteValue(value);
            block = nullptr;
        }
    }
}

void Procedure::invalidateCFG()
{
    m_dominators = nullptr;
}

void Procedure::dump(PrintStream& out) const
{
    IndexSet<Value> valuesInBlocks;
    for (BasicBlock* block : *this) {
        out.print(deepDump(*this, block));
        valuesInBlocks.addAll(*block);
    }
    bool didPrint = false;
    for (Value* value : values()) {
        if (valuesInBlocks.contains(value))
            continue;

        if (!didPrint) {
            dataLog("Orphaned values:\n");
            didPrint = true;
        }
        dataLog("    ", deepDump(*this, value), "\n");
    }
    if (hasQuirks())
        out.print("Has Quirks: True\n");
    if (variables().size()) {
        out.print("Variables:\n");
        for (Variable* variable : variables())
            out.print("    ", deepDump(variable), "\n");
    }
    if (stackSlots().size()) {
        out.print("Stack slots:\n");
        for (StackSlot* slot : stackSlots())
            out.print("    ", pointerDump(slot), ": ", deepDump(slot), "\n");
    }
    if (m_byproducts->count())
        out.print(*m_byproducts);
}

Vector<BasicBlock*> Procedure::blocksInPreOrder()
{
    return B3::blocksInPreOrder(at(0));
}

Vector<BasicBlock*> Procedure::blocksInPostOrder()
{
    return B3::blocksInPostOrder(at(0));
}

void Procedure::deleteStackSlot(StackSlot* stackSlot)
{
    m_stackSlots.remove(stackSlot);
}

void Procedure::deleteVariable(Variable* variable)
{
    m_variables.remove(variable);
}

void Procedure::deleteValue(Value* value)
{
    m_values.remove(value);
}

void Procedure::deleteOrphans()
{
    IndexSet<Value> valuesInBlocks;
    for (BasicBlock* block : *this)
        valuesInBlocks.addAll(*block);

    // Since this method is not on any hot path, we do it conservatively: first a pass to
    // identify the values to be removed, and then a second pass to remove them. This avoids any
    // risk of the value iteration being broken by removals.
    Vector<Value*, 16> toRemove;
    for (Value* value : values()) {
        if (!valuesInBlocks.contains(value))
            toRemove.append(value);
        else if (UpsilonValue* upsilon = value->as<UpsilonValue>()) {
            if (!valuesInBlocks.contains(upsilon->phi()))
                upsilon->replaceWithNop();
        }
    }

    for (Value* value : toRemove)
        deleteValue(value);
}

Dominators& Procedure::dominators()
{
    if (!m_dominators)
        m_dominators = std::make_unique<Dominators>(*this);
    return *m_dominators;
}

void Procedure::addFastConstant(const ValueKey& constant)
{
    RELEASE_ASSERT(constant.isConstant());
    m_fastConstants.add(constant);
}

bool Procedure::isFastConstant(const ValueKey& constant)
{
    if (!constant)
        return false;
    return m_fastConstants.contains(constant);
}

CCallHelpers::Label Procedure::entrypointLabel(unsigned index) const
{
    return m_code->entrypointLabel(index);
}

void* Procedure::addDataSection(size_t size)
{
    if (!size)
        return nullptr;
    std::unique_ptr<DataSection> dataSection = std::make_unique<DataSection>(size);
    void* result = dataSection->data();
    m_byproducts->add(WTFMove(dataSection));
    return result;
}

unsigned Procedure::callArgAreaSizeInBytes() const
{
    return code().callArgAreaSizeInBytes();
}

void Procedure::requestCallArgAreaSizeInBytes(unsigned size)
{
    code().requestCallArgAreaSizeInBytes(size);
}

void Procedure::pinRegister(Reg reg)
{
    code().pinRegister(reg);
}

unsigned Procedure::frameSize() const
{
    return code().frameSize();
}

const RegisterAtOffsetList& Procedure::calleeSaveRegisters() const
{
    return code().calleeSaveRegisters();
}

Value* Procedure::addValueImpl(Value* value)
{
    return m_values.add(std::unique_ptr<Value>(value));
}

void Procedure::setBlockOrderImpl(Vector<BasicBlock*>& blocks)
{
    IndexSet<BasicBlock> blocksSet;
    blocksSet.addAll(blocks);

    for (BasicBlock* block : *this) {
        if (!blocksSet.contains(block))
            blocks.append(block);
    }

    // Place blocks into this's block list by first leaking all of the blocks and then readopting
    // them.
    for (auto& entry : m_blocks)
        entry.release();

    m_blocks.resize(blocks.size());
    for (unsigned i = 0; i < blocks.size(); ++i) {
        BasicBlock* block = blocks[i];
        block->m_index = i;
        m_blocks[i] = std::unique_ptr<BasicBlock>(block);
    }
}

void Procedure::setWasmBoundsCheckGenerator(RefPtr<WasmBoundsCheckGenerator> generator)
{
    code().setWasmBoundsCheckGenerator(generator);
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
