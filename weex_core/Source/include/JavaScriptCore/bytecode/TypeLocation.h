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

#include "TypeSet.h"

namespace JSC {

enum TypeProfilerGlobalIDFlags {
    TypeProfilerNeedsUniqueIDGeneration = -1,
    TypeProfilerNoGlobalIDExists = -2,
    TypeProfilerReturnStatement = -3
};

typedef intptr_t GlobalVariableID;

class TypeLocation {
public:
    TypeLocation()
        : m_lastSeenType(TypeNothing)
        , m_divotForFunctionOffsetIfReturnStatement(UINT_MAX)
        , m_instructionTypeSet(TypeSet::create())
        , m_globalTypeSet(nullptr)
    {
    }

    GlobalVariableID m_globalVariableID;
    RuntimeType m_lastSeenType;
    intptr_t m_sourceID;
    unsigned m_divotStart;
    unsigned m_divotEnd;
    unsigned m_divotForFunctionOffsetIfReturnStatement;
    RefPtr<TypeSet> m_instructionTypeSet;
    RefPtr<TypeSet> m_globalTypeSet;
};

} // namespace JSC
