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
#include "TypeLocationCache.h"

#include "TypeProfiler.h"
#include "VM.h"

namespace JSC {

std::pair<TypeLocation*, bool> TypeLocationCache::getTypeLocation(GlobalVariableID globalVariableID, intptr_t sourceID, unsigned start, unsigned end, RefPtr<TypeSet>&& globalTypeSet, VM* vm)
{
    LocationKey key;
    key.m_globalVariableID = globalVariableID;
    key.m_sourceID = sourceID;
    key.m_start = start;
    key.m_end = end;

    bool isNewLocation = false;
    if (m_locationMap.find(key) == m_locationMap.end()) {
        ASSERT(vm->typeProfiler());
        TypeLocation* location = vm->typeProfiler()->nextTypeLocation();
        location->m_globalVariableID = globalVariableID;
        location->m_sourceID = sourceID;
        location->m_divotStart = start;
        location->m_divotEnd = end;
        location->m_globalTypeSet = WTFMove(globalTypeSet);

        m_locationMap[key] = location;
        isNewLocation = true;
    }

    TypeLocation* location = m_locationMap.find(key)->second;
    return std::pair<TypeLocation*, bool>(location, isNewLocation);
}

} // namespace JSC
