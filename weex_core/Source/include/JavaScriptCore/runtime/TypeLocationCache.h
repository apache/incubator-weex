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
