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

#include "ParserModes.h"
#include "UnlinkedSourceCode.h"
#include <wtf/HashTraits.h>

namespace JSC {

enum class SourceCodeType { EvalType, ProgramType, FunctionType, ModuleType };
enum class TypeProfilerEnabled { No, Yes };
enum class ControlFlowProfilerEnabled { No, Yes };

class SourceCodeFlags {
public:
    SourceCodeFlags() = default;

    SourceCodeFlags(
        SourceCodeType codeType, JSParserStrictMode strictMode, JSParserScriptMode scriptMode, 
        DerivedContextType derivedContextType, EvalContextType evalContextType, bool isArrowFunctionContext,
        DebuggerMode debuggerMode, TypeProfilerEnabled typeProfilerEnabled, ControlFlowProfilerEnabled controlFlowProfilerEnabled)
        : m_flags(
            (static_cast<unsigned>(debuggerMode) << 8) |
            (static_cast<unsigned>(typeProfilerEnabled) << 7) |
            (static_cast<unsigned>(controlFlowProfilerEnabled) << 6) |
            (static_cast<unsigned>(scriptMode) << 5) |
            (static_cast<unsigned>(isArrowFunctionContext) << 4) |
            (static_cast<unsigned>(evalContextType) << 3) |
            (static_cast<unsigned>(derivedContextType) << 2) |
            (static_cast<unsigned>(codeType) << 1) |
            (static_cast<unsigned>(strictMode))
        )
    {
    }

    inline bool operator==(const SourceCodeFlags& rhs) const
    {
        return m_flags == rhs.m_flags;
    }

    unsigned bits() { return m_flags; }

private:
    unsigned m_flags { 0 };
};

class SourceCodeKey {
public:
    SourceCodeKey()
    {
    }

    SourceCodeKey(
        const UnlinkedSourceCode& sourceCode, const String& name, SourceCodeType codeType, JSParserStrictMode strictMode, 
        JSParserScriptMode scriptMode, DerivedContextType derivedContextType, EvalContextType evalContextType, bool isArrowFunctionContext,
        DebuggerMode debuggerMode, TypeProfilerEnabled typeProfilerEnabled, ControlFlowProfilerEnabled controlFlowProfilerEnabled)
            : m_sourceCode(sourceCode)
            , m_name(name)
            , m_flags(codeType, strictMode, scriptMode, derivedContextType, evalContextType, isArrowFunctionContext, debuggerMode, typeProfilerEnabled, controlFlowProfilerEnabled)
            , m_hash(sourceCode.hash() ^ m_flags.bits())
    {
    }

    SourceCodeKey(WTF::HashTableDeletedValueType)
        : m_sourceCode(WTF::HashTableDeletedValue)
    {
    }

    bool isHashTableDeletedValue() const { return m_sourceCode.isHashTableDeletedValue(); }

    unsigned hash() const { return m_hash; }

    size_t length() const { return m_sourceCode.length(); }

    bool isNull() const { return m_sourceCode.isNull(); }

    // To save memory, we compute our string on demand. It's expected that source
    // providers cache their strings to make this efficient.
    StringView string() const { return m_sourceCode.view(); }

    bool operator==(const SourceCodeKey& other) const
    {
        return m_hash == other.m_hash
            && length() == other.length()
            && m_flags == other.m_flags
            && m_name == other.m_name
            && string() == other.string();
    }

    struct Hash {
        static unsigned hash(const SourceCodeKey& key) { return key.hash(); }
        static bool equal(const SourceCodeKey& a, const SourceCodeKey& b) { return a == b; }
        static const bool safeToCompareToEmptyOrDeleted = false;
    };

    struct HashTraits : SimpleClassHashTraits<SourceCodeKey> {
        static const bool hasIsEmptyValueFunction = true;
        static bool isEmptyValue(const SourceCodeKey& key) { return key.isNull(); }
    };

private:
    UnlinkedSourceCode m_sourceCode;
    String m_name;
    SourceCodeFlags m_flags;
    unsigned m_hash;
};

} // namespace JSC
