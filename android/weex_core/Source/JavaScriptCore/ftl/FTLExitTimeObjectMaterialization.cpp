/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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
#include "FTLExitTimeObjectMaterialization.h"

#if ENABLE(FTL_JIT)

#include "DFGGraph.h"

namespace JSC { namespace FTL {

using namespace JSC::DFG;

ExitTimeObjectMaterialization::ExitTimeObjectMaterialization(NodeType type, CodeOrigin codeOrigin)
    : m_type(type)
    , m_origin(codeOrigin)
{
}

ExitTimeObjectMaterialization::~ExitTimeObjectMaterialization()
{
}

void ExitTimeObjectMaterialization::add(
    PromotedLocationDescriptor location, const ExitValue& value)
{
    m_properties.append(ExitPropertyValue(location, value));
}

ExitValue ExitTimeObjectMaterialization::get(PromotedLocationDescriptor location) const
{
    for (ExitPropertyValue value : m_properties) {
        if (value.location() == location)
            return value.value();
    }
    return ExitValue();
}

void ExitTimeObjectMaterialization::accountForLocalsOffset(int offset)
{
    for (ExitPropertyValue& property : m_properties)
        property = property.withLocalsOffset(offset);
}

void ExitTimeObjectMaterialization::dump(PrintStream& out) const
{
    out.print(RawPointer(this), ":", Graph::opName(m_type), "(", listDump(m_properties), ")");
}

void ExitTimeObjectMaterialization::validateReferences(const TrackedReferences& trackedReferences) const
{
    for (ExitPropertyValue value : m_properties)
        value.validateReferences(trackedReferences);
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)

