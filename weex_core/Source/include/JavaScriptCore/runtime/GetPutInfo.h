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

#include <wtf/text/UniquedStringImpl.h>

namespace JSC {

class Structure;
class WatchpointSet;
class JSLexicalEnvironment;

enum ResolveMode {
    ThrowIfNotFound,
    DoNotThrowIfNotFound
};

enum ResolveType {
    // Lexical scope guaranteed a certain type of variable access.
    GlobalProperty,
    GlobalVar,
    GlobalLexicalVar,
    ClosureVar,
    LocalClosureVar,
    ModuleVar,

    // Ditto, but at least one intervening scope used non-strict eval, which
    // can inject an intercepting var delcaration at runtime.
    GlobalPropertyWithVarInjectionChecks,
    GlobalVarWithVarInjectionChecks,
    GlobalLexicalVarWithVarInjectionChecks,
    ClosureVarWithVarInjectionChecks,

    // We haven't found which scope this belongs to, and we also
    // haven't ruled out the possibility of it being cached. Ideally,
    // we want to transition this to GlobalVar/GlobalLexicalVar/GlobalProperty <with/without injection>
    UnresolvedProperty,
    UnresolvedPropertyWithVarInjectionChecks,

    // Lexical scope didn't prove anything -- probably because of a 'with' scope.
    Dynamic
};

enum class InitializationMode : unsigned {
    Initialization,      // "let x = 20;"
    ConstInitialization, // "const x = 20;"
    NotInitialization    // "x = 20;"
};

ALWAYS_INLINE const char* resolveModeName(ResolveMode resolveMode)
{
    static const char* const names[] = {
        "ThrowIfNotFound",
        "DoNotThrowIfNotFound"
    };
    return names[resolveMode];
}

ALWAYS_INLINE const char* resolveTypeName(ResolveType type)
{
    static const char* const names[] = {
        "GlobalProperty",
        "GlobalVar",
        "GlobalLexicalVar",
        "ClosureVar",
        "LocalClosureVar",
        "ModuleVar",
        "GlobalPropertyWithVarInjectionChecks",
        "GlobalVarWithVarInjectionChecks",
        "GlobalLexicalVarWithVarInjectionChecks",
        "ClosureVarWithVarInjectionChecks",
        "UnresolvedProperty",
        "UnresolvedPropertyWithVarInjectionChecks",
        "Dynamic"
    };
    return names[type];
}

ALWAYS_INLINE const char* initializationModeName(InitializationMode initializationMode)
{
    static const char* const names[] = {
        "Initialization",
        "ConstInitialization",
        "NotInitialization"
    };
    return names[static_cast<unsigned>(initializationMode)];
}

ALWAYS_INLINE bool isInitialization(InitializationMode initializationMode)
{
    switch (initializationMode) {
    case InitializationMode::Initialization:
    case InitializationMode::ConstInitialization:
        return true;
    case InitializationMode::NotInitialization:
        return false;
    }
    ASSERT_NOT_REACHED();
    return false;
}

ALWAYS_INLINE ResolveType makeType(ResolveType type, bool needsVarInjectionChecks)
{
    if (!needsVarInjectionChecks)
        return type;

    switch (type) {
    case GlobalProperty:
        return GlobalPropertyWithVarInjectionChecks;
    case GlobalVar:
        return GlobalVarWithVarInjectionChecks;
    case GlobalLexicalVar:
        return GlobalLexicalVarWithVarInjectionChecks;
    case ClosureVar:
    case LocalClosureVar:
        return ClosureVarWithVarInjectionChecks;
    case UnresolvedProperty:
        return UnresolvedPropertyWithVarInjectionChecks;
    case ModuleVar:
    case GlobalPropertyWithVarInjectionChecks:
    case GlobalVarWithVarInjectionChecks:
    case GlobalLexicalVarWithVarInjectionChecks:
    case ClosureVarWithVarInjectionChecks:
    case UnresolvedPropertyWithVarInjectionChecks:
    case Dynamic:
        return type;
    }

    RELEASE_ASSERT_NOT_REACHED();
    return type;
}

ALWAYS_INLINE bool needsVarInjectionChecks(ResolveType type)
{
    switch (type) {
    case GlobalProperty:
    case GlobalVar:
    case GlobalLexicalVar:
    case ClosureVar:
    case LocalClosureVar:
    case ModuleVar:
    case UnresolvedProperty:
        return false;
    case GlobalPropertyWithVarInjectionChecks:
    case GlobalVarWithVarInjectionChecks:
    case GlobalLexicalVarWithVarInjectionChecks:
    case ClosureVarWithVarInjectionChecks:
    case UnresolvedPropertyWithVarInjectionChecks:
    case Dynamic:
        return true;
    default:
        RELEASE_ASSERT_NOT_REACHED();
        return true;
    }
}

struct ResolveOp {
    ResolveOp(ResolveType type, size_t depth, Structure* structure, JSLexicalEnvironment* lexicalEnvironment, WatchpointSet* watchpointSet, uintptr_t operand, UniquedStringImpl* importedName = nullptr)
        : type(type)
        , depth(depth)
        , structure(structure)
        , lexicalEnvironment(lexicalEnvironment)
        , watchpointSet(watchpointSet)
        , operand(operand)
        , importedName(importedName)
    {
    }

    ResolveType type;
    size_t depth;
    Structure* structure;
    JSLexicalEnvironment* lexicalEnvironment;
    WatchpointSet* watchpointSet;
    uintptr_t operand;
    RefPtr<UniquedStringImpl> importedName;
};

class GetPutInfo {
    typedef unsigned Operand;
public:
    // Give each field 10 bits for simplicity.
    static_assert(sizeof(Operand) * 8 > 30, "Not enough bits for GetPutInfo");
    static const unsigned modeShift = 20;
    static const unsigned initializationShift = 10;
    static const unsigned typeBits = (1 << initializationShift) - 1;
    static const unsigned initializationBits = ((1 << modeShift) - 1) & ~typeBits;
    static const unsigned modeBits = ((1 << 30) - 1) & ~initializationBits & ~typeBits;
    static_assert((modeBits & initializationBits & typeBits) == 0x0, "There should be no intersection between ResolveMode ResolveType and InitializationMode");

    GetPutInfo(ResolveMode resolveMode, ResolveType resolveType, InitializationMode initializationMode)
        : m_operand((resolveMode << modeShift) | (static_cast<unsigned>(initializationMode) << initializationShift) | resolveType)
    {
    }

    explicit GetPutInfo(unsigned operand)
        : m_operand(operand)
    {
    }

    ResolveType resolveType() const { return static_cast<ResolveType>(m_operand & typeBits); }
    InitializationMode initializationMode() const { return static_cast<InitializationMode>((m_operand & initializationBits) >> initializationShift); }
    ResolveMode resolveMode() const { return static_cast<ResolveMode>((m_operand & modeBits) >> modeShift); }
    unsigned operand() { return m_operand; }

private:
    Operand m_operand;
};

enum GetOrPut { Get, Put };

} // namespace JSC
