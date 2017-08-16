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

#pragma once

#include "TypeLocation.h"
#include <unordered_map>
#include <wtf/HashMethod.h>

namespace JSC {

class VM;

class TypeLocationCache {
public:
    struct LocationKey {
        LocationKey() {}
        bool operator==(const LocationKey& other) const 
        {
            return m_globalVariableID == other.m_globalVariableID
                && m_sourceID == other.m_sourceID
                && m_start == other.m_start
                && m_end == other.m_end;
        }

        unsigned hash() const
        {
            return m_globalVariableID + m_sourceID + m_start + m_end;
        }

        GlobalVariableID m_globalVariableID;
        intptr_t m_sourceID;
        unsigned m_start;
        unsigned m_end;
    };

    std::pair<TypeLocation*, bool> getTypeLocation(GlobalVariableID, intptr_t, unsigned start, unsigned end, RefPtr<TypeSet>&&, VM*);
private:     
    typedef std::unordered_map<LocationKey, TypeLocation*, HashMethod<LocationKey>> LocationMap;
    LocationMap m_locationMap;
};

} // namespace JSC
