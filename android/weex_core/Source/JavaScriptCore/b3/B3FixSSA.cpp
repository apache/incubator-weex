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
#include "B3FixSSA.h"

#if ENABLE(B3_JIT)

#include "B3BasicBlockInlines.h"
#include "B3BreakCriticalEdges.h"
#include "B3Dominators.h"
#include "B3InsertionSetInlines.h"
#include "B3PhaseScope.h"
#include "B3ProcedureInlines.h"
#include "B3SSACalculator.h"
#include "B3UpsilonValue.h"
#include "B3ValueInlines.h"
#include "B3Variable.h"
#include "B3VariableValue.h"
#include <wtf/CommaPrinter.h>
#include <wtf/IndexSet.h>

namespace JSC { namespace B3 {

namespace {
const bool verbose = false;
} // anonymous namespace

void demoteValues(Procedure& proc, const IndexSet<Value>& values)
{
    HashMap<Value*, Variable*> map;
    HashMap<Value*, Variable*> phiMap;

    // Create stack slots.
    for (Value* value : values.values(proc.values())) {
        map.add(value, proc.addVariable(value->type()));

        if (value->opcode() == Phi)
            phiMap.add(value, proc.addVariable(value->type()));
    }

    if (verbose) {
        dataLog("Demoting values as follows:\n");
        dataLog("   map = ");
        CommaPrinter comma;
        for (auto& entry : map)
            dataLog(comma, *entry.key, "=>", *entry.value);
        dataLog("\n");
        dataLog("   phiMap = ");
        comma = CommaPrinter();
        for (auto& entry : phiMap)
            dataLog(comma, *entry.key, "=>", *entry.value);
        dataLog("\n");
    }

    // Change accesses to the values to accesses to the stack slots.
    InsertionSet insertionSet(proc);
    for (BasicBlock* block : proc) {
        for (unsigned valueIndex = 0; valueIndex < block->size(); ++valueIndex) {
            Value* value = block->at(valueIndex);

            if (value->opcode() == Phi) {
                if (Variable* variable = phiMap.get(value)) {
                    value->replaceWithIdentity(
                        insertionSet.insert<VariableValue>(
                            valueIndex, Get, value->origin(), variable));
                }
            } else {
                for (Value*& child : value->children()) {
                    if (Variable* variable = map.get(child)) {
                        child = insertionSet.insert<VariableValue>(
                            valueIndex, Get, value->origin(), variable);
                    }
                }

                if (UpsilonValue* upsilon = value->as<UpsilonValue>()) {
                    if (Variable* variable = phiMap.get(upsilon->phi())) {
                        insertionSet.insert<VariableValue>(
                            valueIndex, Set, upsilon->origin(), variable, upsilon->child(0));
                        value->replaceWithNop();
                    }
                }
            }

            if (Variable* variable = map.get(value)) {
                insertionSet.insert<VariableValue>(
                    valueIndex + 1, Set, value->origin(), variable, value);
            }
        }
        insertionSet.execute(block);
    }
}

bool fixSSA(Procedure& proc)
{
    PhaseScope phaseScope(proc, "fixSSA");

    // Just for sanity, remove any unused variables first. It's unlikely that this code has any
    // bugs having to do with dead variables, but it would be silly to have to fix such a bug if
    // it did arise.
    IndexSet<Variable> liveVariables;
    for (Value* value : proc.values()) {
        if (VariableValue* variableValue = value->as<VariableValue>())
            liveVariables.add(variableValue->variable());
    }

    for (Variable* variable : proc.variables()) {
        if (!liveVariables.contains(variable))
            proc.deleteVariable(variable);
    }

    if (proc.variables().isEmpty())
        return false;

    // We know that we have variables to optimize, so do that now.
    breakCriticalEdges(proc);

    SSACalculator ssa(proc);

    // Create a SSACalculator::Variable ("calcVar") for every variable.
    Vector<Variable*> calcVarToVariable;
    IndexMap<Variable, SSACalculator::Variable*> variableToCalcVar(proc.variables().size());

    for (Variable* variable : proc.variables()) {
        SSACalculator::Variable* calcVar = ssa.newVariable();
        RELEASE_ASSERT(calcVar->index() == calcVarToVariable.size());
        calcVarToVariable.append(variable);
        variableToCalcVar[variable] = calcVar;
    }

    // Create Defs for all of the stores to the stack variable.
    for (BasicBlock* block : proc) {
        for (Value* value : *block) {
            if (value->opcode() != Set)
                continue;

            Variable* variable = value->as<VariableValue>()->variable();

            if (SSACalculator::Variable* calcVar = variableToCalcVar[variable])
                ssa.newDef(calcVar, block, value->child(0));
        }
    }

    // Decide where Phis are to be inserted. This creates them but does not insert them.
    ssa.computePhis(
        [&] (SSACalculator::Variable* calcVar, BasicBlock* block) -> Value* {
            Variable* variable = calcVarToVariable[calcVar->index()];
            Value* phi = proc.add<Value>(Phi, variable->type(), block->at(0)->origin());
            if (verbose) {
                dataLog(
                    "Adding Phi for ", pointerDump(variable), " at ", *block, ": ",
                    deepDump(proc, phi), "\n");
            }
            return phi;
        });

    // Now perform the conversion.
    InsertionSet insertionSet(proc);
    IndexMap<Variable, Value*> mapping(proc.variables().size());
    for (BasicBlock* block : proc.blocksInPreOrder()) {
        mapping.clear();

        for (unsigned index = calcVarToVariable.size(); index--;) {
            Variable* variable = calcVarToVariable[index];
            SSACalculator::Variable* calcVar = ssa.variable(index);

            SSACalculator::Def* def = ssa.reachingDefAtHead(block, calcVar);
            if (def)
                mapping[variable] = def->value();
        }

        for (SSACalculator::Def* phiDef : ssa.phisForBlock(block)) {
            Variable* variable = calcVarToVariable[phiDef->variable()->index()];

            insertionSet.insertValue(0, phiDef->value());
            mapping[variable] = phiDef->value();
        }

        for (unsigned valueIndex = 0; valueIndex < block->size(); ++valueIndex) {
            Value* value = block->at(valueIndex);
            value->performSubstitution();

            switch (value->opcode()) {
            case Get: {
                VariableValue* variableValue = value->as<VariableValue>();
                Variable* variable = variableValue->variable();

                if (Value* replacement = mapping[variable])
                    value->replaceWithIdentity(replacement);
                else {
                    value->replaceWithIdentity(
                        insertionSet.insertBottom(valueIndex, value));
                }
                break;
            }
                
            case Set: {
                VariableValue* variableValue = value->as<VariableValue>();
                Variable* variable = variableValue->variable();

                mapping[variable] = value->child(0);
                value->replaceWithNop();
                break;
            }

            default:
                break;
            }
        }

        unsigned upsilonInsertionPoint = block->size() - 1;
        Origin upsilonOrigin = block->last()->origin();
        for (BasicBlock* successorBlock : block->successorBlocks()) {
            for (SSACalculator::Def* phiDef : ssa.phisForBlock(successorBlock)) {
                Value* phi = phiDef->value();
                SSACalculator::Variable* calcVar = phiDef->variable();
                Variable* variable = calcVarToVariable[calcVar->index()];

                Value* mappedValue = mapping[variable];
                if (verbose) {
                    dataLog(
                        "Mapped value for ", *variable, " with successor Phi ", *phi,
                        " at end of ", *block, ": ", pointerDump(mappedValue), "\n");
                }
                
                if (!mappedValue)
                    mappedValue = insertionSet.insertBottom(upsilonInsertionPoint, phi);
                
                insertionSet.insert<UpsilonValue>(
                    upsilonInsertionPoint, upsilonOrigin, mappedValue, phi);
            }
        }

        insertionSet.execute(block);
    }

    if (verbose) {
        dataLog("B3 after SSA conversion:\n");
        dataLog(proc);
    }

    return true;
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

