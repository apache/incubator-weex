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

#if ENABLE(DFG_JIT)

#include "DFGCommon.h"
#include "DFGGraph.h"

namespace JSC { namespace DFG {

class Phase {
public:
    Phase(Graph& graph, const char* name)
        : m_graph(graph)
        , m_name(name)
    {
        beginPhase();
    }
    
    ~Phase()
    {
        endPhase();
    }
    
    const char* name() const { return m_name; }
    
    Graph& graph() { return m_graph; }
    
    // Each phase must have a run() method.
    
protected:
    // Things you need to have a DFG compiler phase.
    Graph& m_graph;
    
    VM& vm() { return m_graph.m_vm; }
    CodeBlock* codeBlock() { return m_graph.m_codeBlock; }
    CodeBlock* profiledBlock() { return m_graph.m_profiledBlock; }

    // This runs validation, and uses the graph dump before the phase if possible.
    void validate();
    
    const char* m_name;
    
private:
    // Call these hooks when starting and finishing.
    void beginPhase();
    void endPhase();
    
    CString m_graphDumpBeforePhase;
};

template<typename PhaseType>
bool runAndLog(PhaseType& phase)
{
    double before = 0;

    if (UNLIKELY(Options::reportDFGPhaseTimes()))
        before = monotonicallyIncreasingTimeMS();

    bool result = phase.run();

    if (UNLIKELY(Options::reportDFGPhaseTimes())) {
        double after = monotonicallyIncreasingTimeMS();
        dataLogF("Phase %s took %.4f ms\n", phase.name(), after - before);
    }
    if (result && logCompilationChanges(phase.graph().m_plan.mode))
        dataLogF("Phase %s changed the IR.\n", phase.name());
    return result;
}

template<typename PhaseType>
bool runPhase(Graph& graph)
{
    PhaseType phase(graph);
    return runAndLog(phase);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
