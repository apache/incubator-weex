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
#include "DFGSilentRegisterSavePlan.h"
#include "DFGSpeculativeJIT.h"
#include <wtf/FastMalloc.h>

namespace JSC { namespace DFG {

class SlowPathGenerator {
    WTF_MAKE_FAST_ALLOCATED;
public:
    SlowPathGenerator(SpeculativeJIT* jit)
        : m_currentNode(jit->m_currentNode)
        , m_streamIndex(jit->m_stream->size())
        , m_origin(jit->m_origin) 
    {
    }
    virtual ~SlowPathGenerator() { }
    void generate(SpeculativeJIT* jit)
    {
        m_label = jit->m_jit.label();
        jit->m_currentNode = m_currentNode;
        jit->m_outOfLineStreamIndex = m_streamIndex;
        jit->m_origin = m_origin;
        generateInternal(jit);
        jit->m_outOfLineStreamIndex = std::nullopt;
        if (!ASSERT_DISABLED)
            jit->m_jit.abortWithReason(DFGSlowPathGeneratorFellThrough);
    }
    MacroAssembler::Label label() const { return m_label; }
    virtual MacroAssembler::Call call() const
    {
        RELEASE_ASSERT_NOT_REACHED(); // By default slow path generators don't have a call.
        return MacroAssembler::Call();
    }

    const NodeOrigin& origin() const  { return m_origin; }

protected:
    virtual void generateInternal(SpeculativeJIT*) = 0;
    MacroAssembler::Label m_label;
    Node* m_currentNode;
    unsigned m_streamIndex;
    NodeOrigin m_origin;
};

template<typename JumpType>
class JumpingSlowPathGenerator : public SlowPathGenerator {
public:
    JumpingSlowPathGenerator(JumpType from, SpeculativeJIT* jit)
        : SlowPathGenerator(jit)
        , m_from(from)
        , m_to(jit->m_jit.label())
    {
    }
    
protected:
    void linkFrom(SpeculativeJIT* jit)
    {
        m_from.link(&jit->m_jit);
    }
    
    void jumpTo(SpeculativeJIT* jit)
    {
        jit->m_jit.jump().linkTo(m_to, &jit->m_jit);
    }

    JumpType m_from;
    MacroAssembler::Label m_to;
};

enum class ExceptionCheckRequirement {
    CheckNeeded,
    CheckNotNeeded
};

template<typename JumpType, typename FunctionType, typename ResultType>
class CallSlowPathGenerator : public JumpingSlowPathGenerator<JumpType> {
public:
    CallSlowPathGenerator(
        JumpType from, SpeculativeJIT* jit, FunctionType function,
        SpillRegistersMode spillMode, ExceptionCheckRequirement requirement, ResultType result)
        : JumpingSlowPathGenerator<JumpType>(from, jit)
        , m_function(function)
        , m_spillMode(spillMode)
        , m_exceptionCheckRequirement(requirement)
        , m_result(result)
    {
        if (m_spillMode == NeedToSpill)
            jit->silentSpillAllRegistersImpl(false, m_plans, extractResult(result));
    }
    
    MacroAssembler::Call call() const override
    {
        return m_call;
    }
    
protected:
    void setUp(SpeculativeJIT* jit)
    {
        this->linkFrom(jit);
        if (m_spillMode == NeedToSpill) {
            for (unsigned i = 0; i < m_plans.size(); ++i)
                jit->silentSpill(m_plans[i]);
        }
    }
    
    void recordCall(MacroAssembler::Call call)
    {
        m_call = call;
    }
    
    void tearDown(SpeculativeJIT* jit)
    {
        if (m_spillMode == NeedToSpill) {
            GPRReg canTrample = SpeculativeJIT::pickCanTrample(extractResult(m_result));
            for (unsigned i = m_plans.size(); i--;)
                jit->silentFill(m_plans[i], canTrample);
        }
        if (m_exceptionCheckRequirement == ExceptionCheckRequirement::CheckNeeded)
            jit->m_jit.exceptionCheck();
        this->jumpTo(jit);
    }

    FunctionType m_function;
    SpillRegistersMode m_spillMode;
    ExceptionCheckRequirement m_exceptionCheckRequirement;
    ResultType m_result;
    MacroAssembler::Call m_call;
    Vector<SilentRegisterSavePlan, 2> m_plans;
};

template<typename JumpType, typename FunctionType, typename ResultType, typename... Arguments>
class CallResultAndArgumentsSlowPathGenerator
    : public CallSlowPathGenerator<JumpType, FunctionType, ResultType> {
public:
    CallResultAndArgumentsSlowPathGenerator(
        JumpType from, SpeculativeJIT* jit, FunctionType function,
        SpillRegistersMode spillMode, ExceptionCheckRequirement requirement, ResultType result, Arguments... arguments)
        : CallSlowPathGenerator<JumpType, FunctionType, ResultType>(
            from, jit, function, spillMode, requirement, result)
        , m_arguments(std::forward<Arguments>(arguments)...)
    {
    }

protected:
    template<size_t... ArgumentsIndex>
    void unpackAndGenerate(SpeculativeJIT* jit, std::index_sequence<ArgumentsIndex...>)
    {
        this->setUp(jit);
        this->recordCall(jit->callOperation(this->m_function, extractResult(this->m_result), std::get<ArgumentsIndex>(m_arguments)...));
        this->tearDown(jit);
    }

    void generateInternal(SpeculativeJIT* jit) override
    {
        unpackAndGenerate(jit, std::make_index_sequence<std::tuple_size<std::tuple<Arguments...>>::value>());
    }

    std::tuple<Arguments...> m_arguments;
};

template<typename JumpType, typename FunctionType, typename ResultType, typename... Arguments>
inline std::unique_ptr<SlowPathGenerator> slowPathCall(
    JumpType from, SpeculativeJIT* jit, FunctionType function,
    SpillRegistersMode spillMode, ExceptionCheckRequirement requirement,
    ResultType result, Arguments... arguments)
{
    return std::make_unique<CallResultAndArgumentsSlowPathGenerator<JumpType, FunctionType, ResultType, Arguments...>>(
        from, jit, function, spillMode, requirement, result, arguments...);
}

template<typename JumpType, typename FunctionType, typename ResultType, typename... Arguments>
inline std::unique_ptr<SlowPathGenerator> slowPathCall(
    JumpType from, SpeculativeJIT* jit, FunctionType function,
    ResultType result, Arguments... arguments)
{
    return slowPathCall(
        from, jit, function, NeedToSpill, ExceptionCheckRequirement::CheckNeeded, result, arguments...);
}

template<typename JumpType, typename DestinationType, typename SourceType, unsigned numberOfAssignments>
class AssigningSlowPathGenerator : public JumpingSlowPathGenerator<JumpType> {
public:
    AssigningSlowPathGenerator(
        JumpType from, SpeculativeJIT* jit,
        DestinationType destination[numberOfAssignments],
        SourceType source[numberOfAssignments])
        : JumpingSlowPathGenerator<JumpType>(from, jit)
    {
        for (unsigned i = numberOfAssignments; i--;) {
            m_destination[i] = destination[i];
            m_source[i] = source[i];
        }
    }

protected:
    void generateInternal(SpeculativeJIT* jit) override
    {
        this->linkFrom(jit);
        for (unsigned i = numberOfAssignments; i--;)
            jit->m_jit.move(m_source[i], m_destination[i]);
        this->jumpTo(jit);
    }

private:
    DestinationType m_destination[numberOfAssignments];
    SourceType m_source[numberOfAssignments];
};

template<typename JumpType, typename DestinationType, typename SourceType, unsigned numberOfAssignments>
inline std::unique_ptr<SlowPathGenerator> slowPathMove(
    JumpType from, SpeculativeJIT* jit, SourceType source[numberOfAssignments], DestinationType destination[numberOfAssignments])
{
    return std::make_unique<AssigningSlowPathGenerator<JumpType, DestinationType, SourceType, numberOfAssignments>>(
        from, jit, destination, source);
}

template<typename JumpType, typename DestinationType, typename SourceType>
inline std::unique_ptr<SlowPathGenerator> slowPathMove(
    JumpType from, SpeculativeJIT* jit, SourceType source, DestinationType destination)
{
    SourceType sourceArray[1] = { source };
    DestinationType destinationArray[1] = { destination };
    return std::make_unique<AssigningSlowPathGenerator<JumpType, DestinationType, SourceType, 1>>(
        from, jit, destinationArray, sourceArray);
}

template<typename JumpType, typename DestinationType, typename SourceType>
inline std::unique_ptr<SlowPathGenerator> slowPathMove(
    JumpType from, SpeculativeJIT* jit, SourceType source1, DestinationType destination1, SourceType source2, DestinationType destination2)
{
    SourceType sourceArray[2] = { source1, source2 };
    DestinationType destinationArray[2] = { destination1, destination2 };
    return std::make_unique<AssigningSlowPathGenerator<JumpType, DestinationType, SourceType, 2>>(
        from, jit, destinationArray, sourceArray);
}

} } // namespace JSC::DFG

#endif // ENABLD(DFG_JIT)
