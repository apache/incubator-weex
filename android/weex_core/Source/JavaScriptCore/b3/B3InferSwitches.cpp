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
#include "B3InferSwitches.h"

#if ENABLE(B3_JIT)

#include "B3BasicBlockInlines.h"
#include "B3CaseCollectionInlines.h"
#include "B3InsertionSetInlines.h"
#include "B3PhaseScope.h"
#include "B3ProcedureInlines.h"
#include "B3SwitchValue.h"
#include "B3UseCounts.h"
#include "B3ValueInlines.h"
#include <wtf/ListDump.h>

namespace JSC { namespace B3 {

namespace {

const bool verbose = false;

class InferSwitches {
public:
    InferSwitches(Procedure& proc)
        : m_proc(proc)
        , m_insertionSet(proc)
        , m_useCounts(proc)
    {
    }
    
    bool run()
    {
        if (verbose)
            dataLog("B3 before inferSwitches:\n", m_proc);
        
        bool changed = true;
        bool everChanged = false;
        while (changed) {
            changed = false;
            
            if (verbose)
                dataLog("Performing fixpoint iteration:\n");
            
            for (BasicBlock* block : m_proc)
                changed |= attemptToMergeWithPredecessor(block);

            everChanged |= changed;
        }
        
        if (everChanged) {
            m_proc.resetReachability();
            m_proc.invalidateCFG();
            
            m_proc.deleteOrphans();
            
            if (verbose)
                dataLog("B3 after inferSwitches:\n", m_proc);
            return true;
        }
        
        return false;
    }
    
private:
    bool attemptToMergeWithPredecessor(BasicBlock* block)
    {
        // No point in considering the root block. We also don't consider blocks with multiple
        // predecessors, but we could handle this if we made this code a bit more general and we were
        // not afraid of code bloat.
        if (block->numPredecessors() != 1)
            return false;
        
        SwitchDescription description = describe(block);
        if (verbose)
            dataLog("Description of primary block ", *block, ": ", description, "\n");
        if (!description) {
            if (verbose)
                dataLog("    Bailing because not switch-like.\n");
            return false;
        }
        
        // We know that this block behaves like a switch. But we need to verify that it doesn't also
        // perform any effects or do expensive things. We don't want to create a switch if that will
        // make expensive things execute unconditionally. We're very conservative about how we define
        // "expensive".
        for (Value* value : *block) {
            if (value->isFree())
                continue;
            if (value == description.extra)
                continue;
            if (value == description.branch)
                continue;
            if (verbose)
                dataLog("    Bailing because of ", deepDump(m_proc, value), "\n");
            return false;
        }
        
        BasicBlock* predecessor = block->predecessor(0);
        SwitchDescription predecessorDescription = describe(predecessor);
        if (verbose)
            dataLog("    Description of predecessor block ", *predecessor, ": ", predecessorDescription, "\n");
        if (!predecessorDescription) {
            if (verbose)
                dataLog("    Bailing because not switch-like.\n");
            return false;
        }
        
        // Both us and the predecessor are switch-like, but that doesn't mean that we're compatible.
        // We may be switching on different values!
        if (description.source != predecessorDescription.source) {
            if (verbose)
                dataLog("    Bailing because sources don't match.\n");
            return false;
        }
        
        // We expect that we are the fall-through destination of the predecessor. This is a bit of a
        // goofy condition. If we were not the fall-through destination then our switch is probably
        // just totally redundant and we should be getting rid of it. But we don't handle that here,
        // yet.
        if (predecessorDescription.fallThrough.block() != block) {
            if (verbose)
                dataLog("    Bailing because fall-through of predecessor is not the primary block.\n");
            return false;
        }
        
        // Make sure that there ain't no loops.
        if (description.fallThrough.block() == block
            || description.fallThrough.block() == predecessor) {
            if (verbose)
                dataLog("    Bailing because of fall-through loop.\n");
            return false;
        }
        for (SwitchCase switchCase : description.cases) {
            if (switchCase.targetBlock() == block
                || switchCase.targetBlock() == predecessor) {
                if (verbose)
                    dataLog("    Bailing because of loop in primary cases.\n");
                return false;
            }
        }
        for (SwitchCase switchCase : predecessorDescription.cases) {
            if (switchCase.targetBlock() == block
                || switchCase.targetBlock() == predecessor) {
                if (verbose)
                    dataLog("    Bailing because of loop in predecessor cases.\n");
                return false;
            }
        }
        
        if (verbose)
            dataLog("    Doing it!\n");
        // We're committed to doing the thing.
        
        // Delete the extra value from the predecessor, since that would break downstream inference
        // on the next fixpoint iteration. We would think that this block is too expensive to merge
        // because of the Equal or NotEqual value even though that value is dead! We know it's dead
        // so we kill it ourselves.
        for (Value* value : *predecessor) {
            if (value == predecessorDescription.extra)
                value->replaceWithNopIgnoringType();
        }
        
        // Insert all non-terminal values from our block into our predecessor. We definitely need to
        // do this for constants. We must not do it for the extra value, since that would break
        // downstream inference on the next fixpoint iteration. As a bonus, we don't do it for nops,
        // so that we limit how big blocks get in this phase.
        for (unsigned i = 0; i < block->size() - 1; ++i) {
            Value* value = block->at(i);
            if (value != description.extra && value->opcode() != Nop)
                m_insertionSet.insertValue(predecessor->size() - 1, value);
        }
        m_insertionSet.execute(predecessor);
        block->values().resize(0);
        block->appendNew<Value>(m_proc, Oops, description.branch->origin());
        block->removePredecessor(predecessor);
        
        for (BasicBlock* successorBlock : description.block->successorBlocks())
            successorBlock->replacePredecessor(block, predecessor);

        block->clearSuccessors();
        
        SwitchValue* switchValue = predecessor->replaceLastWithNew<SwitchValue>(
            m_proc, predecessor->last()->origin(), description.source);
        predecessor->clearSuccessors();
        switchValue->setFallThrough(description.fallThrough);
        
        Vector<int64_t> predecessorCases;
        for (SwitchCase switchCase : predecessorDescription.cases) {
            switchValue->appendCase(switchCase);
            predecessorCases.append(switchCase.caseValue());
        }
        std::sort(predecessorCases.begin(), predecessorCases.end());
        auto isPredecessorCase = [&] (int64_t value) -> bool {
            return !!tryBinarySearch<int64_t>(
                predecessorCases, predecessorCases.size(), value,
                [] (int64_t* element) -> int64_t { return *element; });
        };
        
        for (SwitchCase switchCase : description.cases) {
            if (!isPredecessorCase(switchCase.caseValue()))
                switchValue->appendCase(switchCase);
        }
        return true;
    }

    struct SwitchDescription {
        SwitchDescription()
        {
        }
        
        explicit operator bool() { return !!block; }
        
        void dump(PrintStream& out) const
        {
            out.print(
                "{block = ", pointerDump(block),
                ", branch = ", pointerDump(branch),
                ", extra = ", pointerDump(extra),
                ", source = ", pointerDump(source),
                ", cases = ", listDump(cases),
                ", fallThrough = ", fallThrough, "}");
        }

        BasicBlock* block { nullptr };
        Value* branch { nullptr };
        Value* extra { nullptr }; // This is the Equal or NotEqual value, if applicable.
        Value* source { nullptr };
        Vector<SwitchCase, 1> cases;
        FrequentedBlock fallThrough;
    };
    
    SwitchDescription describe(BasicBlock* block)
    {
        SwitchDescription result;
        result.block = block;
        result.branch = block->last();
        
        switch (result.branch->opcode()) {
        case Branch: {
            Value* predicate = result.branch->child(0);
            FrequentedBlock taken = result.block->taken();
            FrequentedBlock notTaken = result.block->notTaken();
            bool handled = false;
            // NOTE: This uses UseCounts that we computed before any transformation. This is fine
            // because although we may have mutated the IR, we would not have added any new
            // predicates.
            if (predicate->numChildren() == 2
                && predicate->child(1)->hasInt()
                && m_useCounts.numUses(predicate) == 1) {
                switch (predicate->opcode()) {
                case Equal:
                    result.source = predicate->child(0);
                    result.extra = predicate;
                    result.cases.append(SwitchCase(predicate->child(1)->asInt(), taken));
                    result.fallThrough = notTaken;
                    handled = true;
                    break;
                case NotEqual:
                    result.source = predicate->child(0);
                    result.extra = predicate;
                    result.cases.append(SwitchCase(predicate->child(1)->asInt(), notTaken));
                    result.fallThrough = taken;
                    handled = true;
                    break;
                default:
                    break;
                }
            }
            if (handled)
                break;
            result.source = predicate;
            result.cases.append(SwitchCase(0, notTaken));
            result.fallThrough = taken;
            break;
        }
            
        case Switch: {
            SwitchValue* switchValue = result.branch->as<SwitchValue>();
            result.source = switchValue->child(0);
            for (SwitchCase switchCase : switchValue->cases(result.block))
                result.cases.append(switchCase);
            result.fallThrough = result.block->fallThrough();
            break;
        }
            
        default:
            result.block = nullptr;
            result.branch = nullptr;
            break;
        }
        
        return result;
    }
    
    Procedure& m_proc;
    InsertionSet m_insertionSet;
    UseCounts m_useCounts;
};

} // anonymous namespace

bool inferSwitches(Procedure& proc)
{
    PhaseScope phaseScope(proc, "inferSwitches");
    InferSwitches inferSwitches(proc);
    return inferSwitches.run();
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

