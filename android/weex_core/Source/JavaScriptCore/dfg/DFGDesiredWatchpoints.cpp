/*
 * Copyright (C) 2013-2016 Apple Inc. All rights reserved.
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
#include "DFGDesiredWatchpoints.h"

#if ENABLE(DFG_JIT)

#include "ArrayBufferNeuteringWatchpoint.h"
#include "CodeBlock.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

void ArrayBufferViewWatchpointAdaptor::add(
    CodeBlock* codeBlock, JSArrayBufferView* view, CommonData& common)
{
    Watchpoint* watchpoint = common.watchpoints.add(codeBlock);
    ArrayBufferNeuteringWatchpoint* neuteringWatchpoint =
        ArrayBufferNeuteringWatchpoint::create(*codeBlock->vm());
    neuteringWatchpoint->set()->add(watchpoint);
    codeBlock->addConstant(neuteringWatchpoint);
    // FIXME: We don't need to set this watchpoint at all for shared buffers.
    // https://bugs.webkit.org/show_bug.cgi?id=164108
    codeBlock->vm()->heap.addReference(neuteringWatchpoint, view->possiblySharedBuffer());
}

void InferredValueAdaptor::add(
    CodeBlock* codeBlock, InferredValue* inferredValue, CommonData& common)
{
    codeBlock->addConstant(inferredValue); // For common users, it doesn't really matter if it's weak or not. If references to it go away, we go away, too.
    inferredValue->add(common.watchpoints.add(codeBlock));
}

void AdaptiveStructureWatchpointAdaptor::add(
    CodeBlock* codeBlock, const ObjectPropertyCondition& key, CommonData& common)
{
    switch (key.kind()) {
    case PropertyCondition::Equivalence:
        common.adaptiveInferredPropertyValueWatchpoints.add(key, codeBlock)->install();
        break;
    default:
        common.adaptiveStructureWatchpoints.add(key, codeBlock)->install();
        break;
    }
}

void InferredTypeAdaptor::add(CodeBlock* codeBlock, const DesiredInferredType& key, CommonData& common)
{
    key.add(common.watchpoints.add(codeBlock));
}

DesiredWatchpoints::DesiredWatchpoints() { }
DesiredWatchpoints::~DesiredWatchpoints() { }

void DesiredWatchpoints::addLazily(WatchpointSet* set)
{
    m_sets.addLazily(set);
}

void DesiredWatchpoints::addLazily(InlineWatchpointSet& set)
{
    m_inlineSets.addLazily(&set);
}

void DesiredWatchpoints::addLazily(InferredValue* inferredValue)
{
    m_inferredValues.addLazily(inferredValue);
}

void DesiredWatchpoints::addLazily(JSArrayBufferView* view)
{
    m_bufferViews.addLazily(view);
}

void DesiredWatchpoints::addLazily(const ObjectPropertyCondition& key)
{
    m_adaptiveStructureSets.addLazily(key);
}

void DesiredWatchpoints::addLazily(const DesiredInferredType& key)
{
    m_inferredTypes.addLazily(key);
}

bool DesiredWatchpoints::consider(Structure* structure)
{
    if (!structure->dfgShouldWatch())
        return false;
    addLazily(structure->transitionWatchpointSet());
    return true;
}

void DesiredWatchpoints::reallyAdd(CodeBlock* codeBlock, CommonData& commonData)
{
    m_sets.reallyAdd(codeBlock, commonData);
    m_inlineSets.reallyAdd(codeBlock, commonData);
    m_inferredValues.reallyAdd(codeBlock, commonData);
    m_bufferViews.reallyAdd(codeBlock, commonData);
    m_adaptiveStructureSets.reallyAdd(codeBlock, commonData);
    m_inferredTypes.reallyAdd(codeBlock, commonData);
}

bool DesiredWatchpoints::areStillValid() const
{
    return m_sets.areStillValid()
        && m_inlineSets.areStillValid()
        && m_inferredValues.areStillValid()
        && m_bufferViews.areStillValid()
        && m_adaptiveStructureSets.areStillValid()
        && m_inferredTypes.areStillValid();
}

void DesiredWatchpoints::dumpInContext(PrintStream& out, DumpContext* context) const
{
    out.print("Desired watchpoints:\n");
    out.print("    Watchpoint sets: ", inContext(m_sets, context), "\n");
    out.print("    Inline watchpoint sets: ", inContext(m_inlineSets, context), "\n");
    out.print("    Inferred values: ", inContext(m_inferredValues, context), "\n");
    out.print("    Buffer views: ", inContext(m_bufferViews, context), "\n");
    out.print("    Object property conditions: ", inContext(m_adaptiveStructureSets, context), "\n");
    out.print("    Inferred types: ", inContext(m_inferredTypes, context), "\n");
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

