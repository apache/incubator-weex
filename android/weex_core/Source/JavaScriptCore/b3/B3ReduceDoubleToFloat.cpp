/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "B3ReduceDoubleToFloat.h"

#if ENABLE(B3_JIT)

#include "B3BasicBlock.h"
#include "B3InsertionSetInlines.h"
#include "B3PhaseScope.h"
#include "B3UseCounts.h"
#include "B3ValueInlines.h"
#include <wtf/IndexSet.h>

namespace JSC { namespace B3 {

namespace {

bool verbose = false;
bool printRemainingConversions = false;

class DoubleToFloatReduction {
public:
    DoubleToFloatReduction(Procedure& procedure)
        : m_procedure(procedure)
    {
    }

    void run()
    {
        if (!findCandidates())
            return;

        findPhisContainingFloat();

        simplify();

        cleanUp();
    }

private:
    // This step find values that are used as Double and cannot be converted to Float..
    // It flows the information backward through Phi-Upsilons.
    bool findCandidates()
    {
        bool foundConversionCandidate = false;
        Vector<Value*, 32> upsilons;

        // First, we find all values that are strictly used as double.
        // Those are values used by something else than DoubleToFloat.
        //
        // We don't know the state of Upsilons until their Phi has been
        // set. We just keep a list of them and update them next.
        for (BasicBlock* block : m_procedure) {
            for (Value* value : *block) {
                value->performSubstitution();

                if (value->opcode() == DoubleToFloat) {
                    foundConversionCandidate = true;

                    Value* child = value->child(0);
                    if (child->opcode() == FloatToDouble) {
                        // We don't really need to simplify this early but it simplifies debugging.
                        value->replaceWithIdentity(child->child(0));
                    }
                    continue;
                }

                if (value->opcode() == FloatToDouble)
                    foundConversionCandidate = true;

                if (value->opcode() == Upsilon) {
                    Value* child = value->child(0);
                    if (child->type() == Double)
                        upsilons.append(value);
                    continue;
                }

                for (Value* child : value->children()) {
                    if (child->type() == Double)
                        m_valuesUsedAsDouble.add(child);
                }
            }
        }

        if (!foundConversionCandidate)
            return false;

        // Now we just need to propagate through Phi-Upsilon.
        // A Upsilon can convert its input to float if its phi is never used as double.
        // If we modify a phi, we need to continue until all the Upsilon-Phi converge.
        bool changedPhiState;
        do {
            changedPhiState = false;
            for (Value* value : upsilons) {
                UpsilonValue* upsilon = value->as<UpsilonValue>();
                Value* phi = upsilon->phi();
                if (!m_valuesUsedAsDouble.contains(phi))
                    continue;

                Value* child = value->child(0);
                bool childChanged = m_valuesUsedAsDouble.add(child);
                if (childChanged && child->opcode() == Phi)
                    changedPhiState = true;
            }
        } while (changedPhiState);

        if (verbose) {
            dataLog("Conversion candidates:\n");
            for (BasicBlock* block : m_procedure) {
                for (Value* value : *block) {
                    if (value->type() == Double && !m_valuesUsedAsDouble.contains(value))
                        dataLog("    ", deepDump(m_procedure, value), "\n");
                }
            }
            dataLog("\n");
        }

        return true;
    }

    // This step finds Phis of type Double that effectively contains Float values.
    // It flows that information forward through Phi-Upsilons.
    void findPhisContainingFloat()
    {
        Vector<Value*, 32> upsilons;

        // The Double value that can be safely turned into a Float are:
        // - FloatToDouble
        // - ConstDouble with a value that converts to Float without losing precision.
        for (BasicBlock* block : m_procedure) {
            for (Value* value : *block) {
                if (value->opcode() != Upsilon)
                    continue;

                Value* child = value->child(0);
                if (child->type() != Double
                    || child->opcode() == FloatToDouble)
                    continue;

                if (child->hasDouble()) {
                    double constValue = child->asDouble();
                    if (isIdentical(static_cast<double>(static_cast<float>(constValue)), constValue))
                        continue;
                }

                if (child->opcode() == Phi) {
                    upsilons.append(value);
                    continue;
                }

                UpsilonValue* upsilon = value->as<UpsilonValue>();
                Value* phi = upsilon->phi();
                m_phisContainingDouble.add(phi);
            }
        }

        // Propagate the flags forward.
        bool changedPhiState;
        do {
            changedPhiState = false;
            for (Value* value : upsilons) {
                Value* child = value->child(0);
                if (m_phisContainingDouble.contains(child)) {
                    UpsilonValue* upsilon = value->as<UpsilonValue>();
                    Value* phi = upsilon->phi();
                    changedPhiState |= m_phisContainingDouble.add(phi);
                }
            }
        } while (changedPhiState);

        if (verbose) {
            dataLog("Phis containing float values:\n");
            for (BasicBlock* block : m_procedure) {
                for (Value* value : *block) {
                    if (value->opcode() == Phi
                        && value->type() == Double
                        && !m_phisContainingDouble.contains(value))
                        dataLog("    ", deepDump(m_procedure, value), "\n");
                }
            }
            dataLog("\n");
        }
    }

    bool canBeTransformedToFloat(Value* value)
    {
        if (value->opcode() == FloatToDouble)
            return true;

        if (value->hasDouble())
            return true; // Double constant truncated to float.

        if (value->opcode() == Phi) {
            return value->type() == Float
                || (value->type() == Double && !m_phisContainingDouble.contains(value));
        }
        return false;
    }

    Value* transformToFloat(Value* value, unsigned valueIndex, InsertionSet& insertionSet)
    {
        ASSERT(canBeTransformedToFloat(value));
        if (value->opcode() == FloatToDouble)
            return value->child(0);

        if (value->hasDouble())
            return insertionSet.insert<ConstFloatValue>(valueIndex, value->origin(), static_cast<float>(value->asDouble()));

        if (value->opcode() == Phi) {
            ASSERT(value->type() == Double || value->type() == Float);
            if (value->type() == Double)
                convertPhi(value);
            return value;
        }
        RELEASE_ASSERT_NOT_REACHED();
        return nullptr;
    }

    void convertPhi(Value* phi)
    {
        ASSERT(phi->opcode() == Phi);
        ASSERT(phi->type() == Double);
        phi->setType(Float);
        m_convertedPhis.add(phi);
    }

    bool attemptTwoOperandsSimplify(Value* candidate, unsigned candidateIndex, InsertionSet& insertionSet)
    {
        Value* left = candidate->child(0);
        Value* right = candidate->child(1);
        if (!canBeTransformedToFloat(left) || !canBeTransformedToFloat(right))
            return false;

        m_convertedValue.add(candidate);
        candidate->child(0) = transformToFloat(left, candidateIndex, insertionSet);
        candidate->child(1) = transformToFloat(right, candidateIndex, insertionSet);
        return true;
    }

    // Simplify Double operations into Float operations.
    void simplify()
    {
        Vector<Value*, 32> upsilonReferencingDoublePhi;

        InsertionSet insertionSet(m_procedure);
        for (BasicBlock* block : m_procedure) {
            for (unsigned index = 0; index < block->size(); ++index) {
                Value* value = block->at(index);

                switch (value->opcode()) {
                case Equal:
                case NotEqual:
                case LessThan:
                case GreaterThan:
                case LessEqual:
                case GreaterEqual:
                case EqualOrUnordered:
                    attemptTwoOperandsSimplify(value, index, insertionSet);
                    continue;
                case Upsilon: {
                    Value* child = value->child(0);
                    if (child->opcode() == Phi && child->type() == Double)
                        upsilonReferencingDoublePhi.append(value);
                    continue;
                }
                default:
                    break;
                }

                if (m_valuesUsedAsDouble.contains(value))
                    continue;

                switch (value->opcode()) {
                case Add:
                case Sub:
                case Mul:
                case Div:
                    if (attemptTwoOperandsSimplify(value, index, insertionSet))
                        value->setType(Float);
                    break;
                case Abs:
                case Ceil:
                case Floor:
                case Neg:
                case Sqrt: {
                    Value* child = value->child(0);
                    if (canBeTransformedToFloat(child)) {
                        value->child(0) = transformToFloat(child, index, insertionSet);
                        value->setType(Float);
                        m_convertedValue.add(value);
                    }
                    break;
                }
                case IToD: {
                    Value* iToF = insertionSet.insert<Value>(index, IToF, value->origin(), value->child(0));
                    value->setType(Float);
                    value->replaceWithIdentity(iToF);
                    m_convertedValue.add(value);
                    break;
                }
                case FloatToDouble:
                    // This happens if we round twice.
                    // Typically, this is indirect through Phi-Upsilons.
                    // The Upsilon rounds and the Phi rounds.
                    value->setType(Float);
                    value->replaceWithIdentity(value->child(0));
                    m_convertedValue.add(value);
                    break;
                case Phi:
                    // If a Phi is always converted to Float, we always make it into a float Phi-Upsilon.
                    // This is a simplistic view of things. Ideally we should keep type that will minimize
                    // the amount of conversion in the loop.
                    if (value->type() == Double)
                        convertPhi(value);
                    break;
                default:
                    break;
                }
            }
            insertionSet.execute(block);
        }

        if (!upsilonReferencingDoublePhi.isEmpty()) {
            // If a Phi contains Float values typed as Double, but is not used as Float
            // by a non-trivial operation, we did not convert it.
            //
            // We fix that now by converting the remaining phis that contains
            // float but where not converted to float.
            bool changedPhi;
            do {
                changedPhi = false;

                for (Value* value : upsilonReferencingDoublePhi) {
                    UpsilonValue* upsilon = value->as<UpsilonValue>();
                    Value* child = value->child(0);
                    Value* phi = upsilon->phi();
                    if (phi->type() == Float && child->type() == Double
                        && !m_phisContainingDouble.contains(child)) {
                        convertPhi(child);
                        changedPhi = true;
                    }
                }

            } while (changedPhi);
        }
    }

    // We are in an inconsistent state where we have
    // DoubleToFloat nodes over values producing float and Phis that are
    // float for Upsilons that are Double.
    //
    // This steps puts us back in a consistent state.
    void cleanUp()
    {
        InsertionSet insertionSet(m_procedure);

        for (BasicBlock* block : m_procedure) {
            for (unsigned index = 0; index < block->size(); ++index) {
                Value* value = block->at(index);
                if (value->opcode() == DoubleToFloat && value->child(0)->type() == Float) {
                    value->replaceWithIdentity(value->child(0));
                    continue;
                }

                if (value->opcode() == Upsilon) {
                    UpsilonValue* upsilon = value->as<UpsilonValue>();
                    Value* child = value->child(0);
                    Value* phi = upsilon->phi();

                    if (phi->type() == Float) {
                        if (child->type() == Double) {
                            Value* newChild = nullptr;
                            if (child->opcode() == FloatToDouble)
                                newChild = child->child(0);
                            else if (child->hasDouble())
                                newChild = insertionSet.insert<ConstFloatValue>(index, child->origin(), static_cast<float>(child->asDouble()));
                            else
                                newChild = insertionSet.insert<Value>(index, DoubleToFloat, upsilon->origin(), child);
                            upsilon->child(0) = newChild;
                        }
                        continue;
                    }
                }

                if (!m_convertedValue.contains(value)) {
                    // Phis can be converted from Double to Float if the value they contain
                    // is not more precise than a Float.
                    // If the value is needed as Double, it has to be converted back.
                    for (Value*& child : value->children()) {
                        if (m_convertedPhis.contains(child))
                            child = insertionSet.insert<Value>(index, FloatToDouble, value->origin(), child);
                    }
                }
            }
            insertionSet.execute(block);
        }
    }

    Procedure& m_procedure;

    // Set of all the Double values that are actually used as Double.
    // Converting any of them to Float would lose precision.
    IndexSet<Value> m_valuesUsedAsDouble;

    // Set of all the Phi of type Double that really contains a Double.
    // Any Double Phi not in the set can be converted to Float without losing precision.
    IndexSet<Value> m_phisContainingDouble;

    // Any value that was converted from producing a Double to producing a Float.
    // This set does not include Phi-Upsilons.
    IndexSet<Value> m_convertedValue;

    // Any value that previously produced Double and now produce Float.
    IndexSet<Value> m_convertedPhis;
};

void printGraphIfConverting(Procedure& procedure)
{
    if (!printRemainingConversions)
        return;

    UseCounts useCount(procedure);

    Vector<Value*> doubleToFloat;
    Vector<Value*> floatToDouble;

    for (BasicBlock* block : procedure) {
        for (Value* value : *block) {
            if (!useCount.numUses(value))
                continue;

            if (value->opcode() == DoubleToFloat)
                doubleToFloat.append(value);
            if (value->opcode() == FloatToDouble)
                floatToDouble.append(value);
        }
    }

    if (doubleToFloat.isEmpty() && floatToDouble.isEmpty())
        return;

    dataLog("Procedure with Float-Double conversion:\n", procedure, "\n");
    dataLog("Converting nodes:\n");
    for (Value* value : doubleToFloat)
        dataLog("    ", deepDump(procedure, value), "\n");
    for (Value* value : floatToDouble)
        dataLog("    ", deepDump(procedure, value), "\n");

}

} // anonymous namespace.

void reduceDoubleToFloat(Procedure& procedure)
{
    PhaseScope phaseScope(procedure, "reduceDoubleToFloat");

    if (verbose)
        dataLog("Before DoubleToFloatReduction:\n", procedure, "\n");

    DoubleToFloatReduction doubleToFloatReduction(procedure);
    doubleToFloatReduction.run();

    if (verbose)
        dataLog("After DoubleToFloatReduction:\n", procedure, "\n");

    printGraphIfConverting(procedure);
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

