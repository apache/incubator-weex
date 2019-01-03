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

namespace JSC { namespace DFG {

enum DoubleFormatState {
    EmptyDoubleFormatState, // bottom
    UsingDoubleFormat,
    NotUsingDoubleFormat,
    CantUseDoubleFormat // top
};

inline DoubleFormatState mergeDoubleFormatStates(DoubleFormatState a, DoubleFormatState b)
{
    switch (a) {
    case EmptyDoubleFormatState:
        return b;
    case UsingDoubleFormat:
        switch (b) {
        case EmptyDoubleFormatState:
        case UsingDoubleFormat:
            return UsingDoubleFormat;
        case NotUsingDoubleFormat:
        case CantUseDoubleFormat:
            return CantUseDoubleFormat;
        }
    case NotUsingDoubleFormat:
        switch (b) {
        case EmptyDoubleFormatState:
        case NotUsingDoubleFormat:
            return NotUsingDoubleFormat;
        case UsingDoubleFormat:
        case CantUseDoubleFormat:
            return CantUseDoubleFormat;
        }
    case CantUseDoubleFormat:
        return CantUseDoubleFormat;
    }
    RELEASE_ASSERT_NOT_REACHED();
    return CantUseDoubleFormat;
}

inline bool mergeDoubleFormatState(DoubleFormatState& dest, DoubleFormatState src)
{
    DoubleFormatState newState = mergeDoubleFormatStates(dest, src);
    if (newState == dest)
        return false;
    dest = newState;
    return true;
}

inline const char* doubleFormatStateToString(DoubleFormatState state)
{
    switch (state) {
    case EmptyDoubleFormatState:
        return "Empty";
    case UsingDoubleFormat:
        return "DoubleFormat";
    case NotUsingDoubleFormat:
        return "ValueFormat";
    case CantUseDoubleFormat:
        return "ForceValue";
    }
    RELEASE_ASSERT_NOT_REACHED();
    return 0;
}

} } // namespace JSC::DFG
