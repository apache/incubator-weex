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

#include "Identifier.h"
#include <wtf/HashMap.h>

namespace JSC {

struct VariableEnvironmentEntry {
public:
    ALWAYS_INLINE bool isCaptured() const { return m_bits & IsCaptured; }
    ALWAYS_INLINE bool isConst() const { return m_bits & IsConst; }
    ALWAYS_INLINE bool isVar() const { return m_bits & IsVar; }
    ALWAYS_INLINE bool isLet() const { return m_bits & IsLet; }
    ALWAYS_INLINE bool isExported() const { return m_bits & IsExported; }
    ALWAYS_INLINE bool isImported() const { return m_bits & IsImported; }
    ALWAYS_INLINE bool isImportedNamespace() const { return m_bits & IsImportedNamespace; }
    ALWAYS_INLINE bool isFunction() const { return m_bits & IsFunction; }
    ALWAYS_INLINE bool isParameter() const { return m_bits & IsParameter; }
    ALWAYS_INLINE bool isSloppyModeHoistingCandidate() const { return m_bits & IsSloppyModeHoistingCandidate; }

    ALWAYS_INLINE void setIsCaptured() { m_bits |= IsCaptured; }
    ALWAYS_INLINE void setIsConst() { m_bits |= IsConst; }
    ALWAYS_INLINE void setIsVar() { m_bits |= IsVar; }
    ALWAYS_INLINE void setIsLet() { m_bits |= IsLet; }
    ALWAYS_INLINE void setIsExported() { m_bits |= IsExported; }
    ALWAYS_INLINE void setIsImported() { m_bits |= IsImported; }
    ALWAYS_INLINE void setIsImportedNamespace() { m_bits |= IsImportedNamespace; }
    ALWAYS_INLINE void setIsFunction() { m_bits |= IsFunction; }
    ALWAYS_INLINE void setIsParameter() { m_bits |= IsParameter; }
    ALWAYS_INLINE void setIsSloppyModeHoistingCandidate() { m_bits |= IsSloppyModeHoistingCandidate; }

    ALWAYS_INLINE void clearIsVar() { m_bits &= ~IsVar; }

private:
    enum Traits : uint16_t {
        IsCaptured = 1 << 0,
        IsConst = 1 << 1,
        IsVar = 1 << 2,
        IsLet = 1 << 3,
        IsExported = 1 << 4,
        IsImported = 1 << 5,
        IsImportedNamespace = 1 << 6,
        IsFunction = 1 << 7,
        IsParameter = 1 << 8,
        IsSloppyModeHoistingCandidate = 1 << 9
    };
    uint16_t m_bits { 0 };
};

struct VariableEnvironmentEntryHashTraits : HashTraits<VariableEnvironmentEntry> {
    static const bool needsDestruction = false;
};

class VariableEnvironment {
private:
    typedef HashMap<RefPtr<UniquedStringImpl>, VariableEnvironmentEntry, IdentifierRepHash, HashTraits<RefPtr<UniquedStringImpl>>, VariableEnvironmentEntryHashTraits> Map;
public:
    VariableEnvironment()
    { }
    VariableEnvironment(VariableEnvironment&& other)
        : m_map(WTFMove(other.m_map))
        , m_isEverythingCaptured(other.m_isEverythingCaptured)
    { }
    VariableEnvironment(const VariableEnvironment&) = default;
    VariableEnvironment& operator=(const VariableEnvironment&) = default;

    ALWAYS_INLINE Map::iterator begin() { return m_map.begin(); }
    ALWAYS_INLINE Map::iterator end() { return m_map.end(); }
    ALWAYS_INLINE Map::const_iterator begin() const { return m_map.begin(); }
    ALWAYS_INLINE Map::const_iterator end() const { return m_map.end(); }
    ALWAYS_INLINE Map::AddResult add(const RefPtr<UniquedStringImpl>& identifier) { return m_map.add(identifier, VariableEnvironmentEntry()); }
    ALWAYS_INLINE Map::AddResult add(const Identifier& identifier) { return add(identifier.impl()); }
    ALWAYS_INLINE unsigned size() const { return m_map.size(); }
    ALWAYS_INLINE bool contains(const RefPtr<UniquedStringImpl>& identifier) const { return m_map.contains(identifier); }
    ALWAYS_INLINE bool remove(const RefPtr<UniquedStringImpl>& identifier) { return m_map.remove(identifier); }
    ALWAYS_INLINE Map::iterator find(const RefPtr<UniquedStringImpl>& identifier) { return m_map.find(identifier); }
    ALWAYS_INLINE Map::const_iterator find(const RefPtr<UniquedStringImpl>& identifier) const { return m_map.find(identifier); }
    void swap(VariableEnvironment& other);
    void markVariableAsCapturedIfDefined(const RefPtr<UniquedStringImpl>& identifier);
    void markVariableAsCaptured(const RefPtr<UniquedStringImpl>& identifier);
    void markAllVariablesAsCaptured();
    bool hasCapturedVariables() const;
    bool captures(UniquedStringImpl* identifier) const;
    void markVariableAsImported(const RefPtr<UniquedStringImpl>& identifier);
    void markVariableAsExported(const RefPtr<UniquedStringImpl>& identifier);

private:
    Map m_map;
    bool m_isEverythingCaptured { false };
};

} // namespace JSC
