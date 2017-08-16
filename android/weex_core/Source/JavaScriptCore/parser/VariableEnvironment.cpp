/*
 * Copyright (C) 2015 Apple Inc. All Rights Reserved.
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
#include "VariableEnvironment.h"
#include <wtf/text/UniquedStringImpl.h>

namespace JSC {

void VariableEnvironment::markVariableAsCapturedIfDefined(const RefPtr<UniquedStringImpl>& identifier)
{
    auto findResult = m_map.find(identifier);
    if (findResult != m_map.end())
        findResult->value.setIsCaptured();
}

void VariableEnvironment::markVariableAsCaptured(const RefPtr<UniquedStringImpl>& identifier)
{
    auto findResult = m_map.find(identifier);
    RELEASE_ASSERT(findResult != m_map.end());
    findResult->value.setIsCaptured();
}

void VariableEnvironment::markAllVariablesAsCaptured()
{
    if (m_isEverythingCaptured)
        return;

    m_isEverythingCaptured = true; // For fast queries.
    // We must mark every entry as captured for when we iterate through m_map and entry.isCaptured() is called.
    for (auto& value : m_map.values())
        value.setIsCaptured();
}

bool VariableEnvironment::hasCapturedVariables() const
{
    if (m_isEverythingCaptured)
        return size() > 0;
    for (auto& value : m_map.values()) {
        if (value.isCaptured())
            return true;
    }
    return false;
}

bool VariableEnvironment::captures(UniquedStringImpl* identifier) const
{
    if (m_isEverythingCaptured)
        return true;

    auto findResult = m_map.find(identifier);
    if (findResult == m_map.end())
        return false;
    return findResult->value.isCaptured();
}

void VariableEnvironment::swap(VariableEnvironment& other)
{
    m_map.swap(other.m_map);
    m_isEverythingCaptured = other.m_isEverythingCaptured;
}

void VariableEnvironment::markVariableAsImported(const RefPtr<UniquedStringImpl>& identifier)
{
    auto findResult = m_map.find(identifier);
    RELEASE_ASSERT(findResult != m_map.end());
    findResult->value.setIsImported();
}

void VariableEnvironment::markVariableAsExported(const RefPtr<UniquedStringImpl>& identifier)
{
    auto findResult = m_map.find(identifier);
    RELEASE_ASSERT(findResult != m_map.end());
    findResult->value.setIsExported();
}

} // namespace JSC
