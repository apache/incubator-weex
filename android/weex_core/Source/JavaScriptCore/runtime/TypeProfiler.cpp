/*
 * Copyright (C) 2014 Apple Inc. All Rights Reserved.
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
#include "TypeProfiler.h"

#include "InspectorProtocolObjects.h"
#include "TypeLocation.h"
#include <wtf/text/StringBuilder.h>

namespace JSC {

static const bool verbose = false;

TypeProfiler::TypeProfiler()
    : m_nextUniqueVariableID(1)
{ 
}

void TypeProfiler::logTypesForTypeLocation(TypeLocation* location, VM& vm)
{
    TypeProfilerSearchDescriptor descriptor = location->m_globalVariableID == TypeProfilerReturnStatement ? TypeProfilerSearchDescriptorFunctionReturn : TypeProfilerSearchDescriptorNormal;

    dataLogF("[Start, End]::[%u, %u]\n", location->m_divotStart, location->m_divotEnd);

    if (findLocation(location->m_divotStart, location->m_sourceID, descriptor, vm))
        dataLog("\t\t[Entry IS in System]\n");
    else
        dataLog("\t\t[Entry IS NOT in system]\n");

    dataLog("\t\t", location->m_globalVariableID == TypeProfilerReturnStatement ? "[Return Statement]" : "[Normal Statement]", "\n");

    dataLog("\t\t#Local#\n\t\t", location->m_instructionTypeSet->dumpTypes().replace("\n", "\n\t\t"), "\n");
    if (location->m_globalTypeSet)
        dataLog("\t\t#Global#\n\t\t", location->m_globalTypeSet->dumpTypes().replace("\n", "\n\t\t"), "\n");
}

void TypeProfiler::insertNewLocation(TypeLocation* location)
{
    if (verbose)
        dataLogF("Registering location:: divotStart:%u, divotEnd:%u\n", location->m_divotStart, location->m_divotEnd);

    if (!m_bucketMap.contains(location->m_sourceID)) {
        Vector<TypeLocation*> bucket;
        m_bucketMap.set(location->m_sourceID, bucket);
    }

    Vector<TypeLocation*>& bucket = m_bucketMap.find(location->m_sourceID)->value;
    bucket.append(location);
}

String TypeProfiler::typeInformationForExpressionAtOffset(TypeProfilerSearchDescriptor descriptor, unsigned offset, intptr_t sourceID, VM& vm)
{
    // This returns a JSON string representing an Object with the following properties:
    //     globalTypeSet: 'JSON<TypeSet> | null'
    //     instructionTypeSet: 'JSON<TypeSet>'

    TypeLocation* location = findLocation(offset, sourceID, descriptor, vm);
    ASSERT(location);

    StringBuilder json;  

    json.append('{');

    json.appendLiteral("\"globalTypeSet\":");
    if (location->m_globalTypeSet && location->m_globalVariableID != TypeProfilerNoGlobalIDExists)
        json.append(location->m_globalTypeSet->toJSONString());
    else
        json.appendLiteral("null");
    json.append(',');

    json.appendLiteral("\"instructionTypeSet\":");
    json.append(location->m_instructionTypeSet->toJSONString());
    json.append(',');

    json.appendLiteral("\"isOverflown\":");
    if (location->m_instructionTypeSet->isOverflown() || (location->m_globalTypeSet && location->m_globalTypeSet->isOverflown()))
        json.appendLiteral("true");
    else
        json.appendLiteral("false");

    json.append('}');
    
    return json.toString();
}

TypeLocation* TypeProfiler::findLocation(unsigned divot, intptr_t sourceID, TypeProfilerSearchDescriptor descriptor, VM& vm)
{
    QueryKey queryKey(sourceID, divot, descriptor);
    auto iter = m_queryCache.find(queryKey);
    if (iter != m_queryCache.end())
        return iter->value;

    if (!vm.functionHasExecutedCache()->hasExecutedAtOffset(sourceID, divot))
        return nullptr;

    if (!m_bucketMap.contains(sourceID))
        return nullptr;

    Vector<TypeLocation*>& bucket = m_bucketMap.find(sourceID)->value;
    TypeLocation* bestMatch = nullptr;
    unsigned distance = UINT_MAX; // Because assignments may be nested, make sure we find the closest enclosing assignment to this character offset.
    for (auto* location : bucket) {
        // We found the type location that correlates to the convergence of all return statements in a function.
        // This text offset is the offset of the opening brace in a function declaration.
        if (descriptor == TypeProfilerSearchDescriptorFunctionReturn && location->m_globalVariableID == TypeProfilerReturnStatement && location->m_divotForFunctionOffsetIfReturnStatement == divot)
            return location;

        if (descriptor != TypeProfilerSearchDescriptorFunctionReturn && location->m_globalVariableID != TypeProfilerReturnStatement && location->m_divotStart <= divot && divot <= location->m_divotEnd && location->m_divotEnd - location->m_divotStart <= distance) {
            distance = location->m_divotEnd - location->m_divotStart;
            bestMatch = location;
        }
    }

    if (bestMatch)
        m_queryCache.set(queryKey, bestMatch);
    // FIXME: BestMatch should never be null past this point. This doesn't hold currently because we ignore var assignments when code contains eval/With (VarInjection). 
    // https://bugs.webkit.org/show_bug.cgi?id=135184
    return bestMatch;
}

TypeLocation* TypeProfiler::nextTypeLocation() 
{ 
    return m_typeLocationInfo.add(); 
}

void TypeProfiler::invalidateTypeSetCache()
{
    for (Bag<TypeLocation>::iterator iter = m_typeLocationInfo.begin(); !!iter; ++iter) {
        TypeLocation* location = *iter;
        location->m_instructionTypeSet->invalidateCache();
        if (location->m_globalTypeSet)
            location->m_globalTypeSet->invalidateCache();
    }
}

void TypeProfiler::dumpTypeProfilerData(VM& vm)
{
    for (Bag<TypeLocation>::iterator iter = m_typeLocationInfo.begin(); !!iter; ++iter) {
        TypeLocation* location = *iter;
        logTypesForTypeLocation(location, vm);
    }
}

} // namespace JSC
