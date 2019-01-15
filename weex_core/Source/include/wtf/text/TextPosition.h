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

#include "OrdinalNumber.h"

namespace WTF {

// TextPosition structure specifies coordinates within an text resource. It is used mostly
// for saving script source position.
class TextPosition {
public:
    TextPosition(OrdinalNumber line, OrdinalNumber column)
        : m_line(line)
        , m_column(column)
    {
    }

    TextPosition() { }
    bool operator==(const TextPosition& other) { return m_line == other.m_line && m_column == other.m_column; }
    bool operator!=(const TextPosition& other) { return !((*this) == other); }

    // A value with line value less than a minimum; used as an impossible position.
    static TextPosition belowRangePosition() { return TextPosition(OrdinalNumber::beforeFirst(), OrdinalNumber::beforeFirst()); }

    OrdinalNumber m_line;
    OrdinalNumber m_column;
};

}

using WTF::TextPosition;
