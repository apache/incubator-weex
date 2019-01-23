/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#include "DFGDoubleFormatState.h"
#include "DFGVariableAccessData.h"
#include "DFGVariableAccessDataDump.h"
#include "SpeculatedType.h"

namespace JSC { namespace DFG {

class ArgumentPosition {
public:
    ArgumentPosition()
        : m_prediction(SpecNone)
        , m_doubleFormatState(EmptyDoubleFormatState)
        , m_isProfitableToUnbox(false)
        , m_shouldNeverUnbox(false)
    {
    }
    
    void addVariable(VariableAccessData* variable)
    {
        m_variables.append(variable);
        
        // We may set this early. Merging it here saves us time in prediction propagation.
        variable->mergeShouldNeverUnbox(m_shouldNeverUnbox);
    }
    
    VariableAccessData* someVariable() const
    {
        if (m_variables.isEmpty())
            return 0;
        return m_variables[0]->find();
    }
    
    FlushFormat flushFormat() const
    {
        if (VariableAccessData* variable = someVariable())
            return variable->flushFormat();
        return DeadFlush;
    }
    
    bool mergeShouldNeverUnbox(bool shouldNeverUnbox)
    {
        return checkAndSet(m_shouldNeverUnbox, m_shouldNeverUnbox || shouldNeverUnbox);
    }
    
    bool mergeArgumentPredictionAwareness()
    {
        bool changed = false;
        for (unsigned i = 0; i < m_variables.size(); ++i) {
            VariableAccessData* variable = m_variables[i]->find();
            changed |= mergeSpeculation(m_prediction, variable->argumentAwarePrediction());
            changed |= mergeDoubleFormatState(m_doubleFormatState, variable->doubleFormatState());
            changed |= mergeShouldNeverUnbox(variable->shouldNeverUnbox());
        }
        if (!changed)
            return false;
        changed = false;
        for (unsigned i = 0; i < m_variables.size(); ++i) {
            VariableAccessData* variable = m_variables[i]->find();
            changed |= variable->mergeArgumentAwarePrediction(m_prediction);
            changed |= variable->mergeDoubleFormatState(m_doubleFormatState);
            changed |= variable->mergeShouldNeverUnbox(m_shouldNeverUnbox);
        }
        return changed;
    }
    
    bool mergeArgumentUnboxingAwareness()
    {
        bool changed = false;
        for (unsigned i = 0; i < m_variables.size(); ++i) {
            VariableAccessData* variable = m_variables[i]->find();
            changed |= checkAndSet(m_isProfitableToUnbox, m_isProfitableToUnbox || variable->isProfitableToUnbox());
        }
        if (!changed)
            return false;
        changed = false;
        for (unsigned i = 0; i < m_variables.size(); ++i) {
            VariableAccessData* variable = m_variables[i]->find();
            changed |= variable->mergeIsProfitableToUnbox(m_isProfitableToUnbox);
        }
        return changed;
    }
    
    bool shouldUnboxIfPossible() const { return m_isProfitableToUnbox && !m_shouldNeverUnbox; }
    
    SpeculatedType prediction() const { return m_prediction; }
    DoubleFormatState doubleFormatState() const { return m_doubleFormatState; }
    bool shouldUseDoubleFormat() const
    {
        return doubleFormatState() == UsingDoubleFormat && shouldUnboxIfPossible();
    }
    
    void dump(PrintStream& out, Graph* graph)
    {
        for (unsigned i = 0; i < m_variables.size(); ++i) {
            VariableAccessData* variable = m_variables[i]->find();
            VirtualRegister operand = variable->local();

            if (i)
                out.print(" ");

            out.print(operand, "(", VariableAccessDataDump(*graph, variable), ")");
        }
        out.print("\n");
    }
    
private:
    SpeculatedType m_prediction;
    DoubleFormatState m_doubleFormatState;
    bool m_isProfitableToUnbox;
    bool m_shouldNeverUnbox;
    
    Vector<VariableAccessData*, 2> m_variables;
};

} } // namespace JSC::DFG
