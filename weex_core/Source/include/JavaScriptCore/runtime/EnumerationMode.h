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

namespace JSC {

enum class PropertyNameMode {
    Symbols = 1 << 0,
    Strings = 1 << 1,
    StringsAndSymbols = Symbols | Strings,
};

enum class DontEnumPropertiesMode {
    Include,
    Exclude
};

enum class JSObjectPropertiesMode {
    Include,
    Exclude
};

class EnumerationMode {
public:
    EnumerationMode(DontEnumPropertiesMode dontEnumPropertiesMode = DontEnumPropertiesMode::Exclude, JSObjectPropertiesMode jsObjectPropertiesMode = JSObjectPropertiesMode::Include)
        : m_dontEnumPropertiesMode(dontEnumPropertiesMode)
        , m_jsObjectPropertiesMode(jsObjectPropertiesMode)
    {
    }

    EnumerationMode(const EnumerationMode& mode, JSObjectPropertiesMode jsObjectPropertiesMode)
        : m_dontEnumPropertiesMode(mode.m_dontEnumPropertiesMode)
        , m_jsObjectPropertiesMode(jsObjectPropertiesMode)
    {
    }

    // Add other constructors as needed for convenience

    bool includeDontEnumProperties()
    {
        return m_dontEnumPropertiesMode == DontEnumPropertiesMode::Include;
    }

    bool includeJSObjectProperties()
    {
        return m_jsObjectPropertiesMode == JSObjectPropertiesMode::Include;
    }

private:
    DontEnumPropertiesMode m_dontEnumPropertiesMode;
    JSObjectPropertiesMode m_jsObjectPropertiesMode;
};

} // namespace JSC
