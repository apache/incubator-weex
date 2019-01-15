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
#include <wtf/RefPtr.h>

namespace JSC {

class ModuleScopeData : public RefCounted<ModuleScopeData> {
WTF_MAKE_NONCOPYABLE(ModuleScopeData);
WTF_MAKE_FAST_ALLOCATED;
public:
    typedef HashMap<RefPtr<UniquedStringImpl>, Vector<RefPtr<UniquedStringImpl>>, IdentifierRepHash, HashTraits<RefPtr<UniquedStringImpl>>> IdentifierAliasMap;

    static Ref<ModuleScopeData> create() { return adoptRef(*new ModuleScopeData); }

    const IdentifierAliasMap& exportedBindings() const { return m_exportedBindings; }

    bool exportName(const Identifier& exportedName)
    {
        return m_exportedNames.add(exportedName.impl()).isNewEntry;
    }

    void exportBinding(const Identifier& localName, const Identifier& exportedName)
    {
        m_exportedBindings.add(localName.impl(), Vector<RefPtr<UniquedStringImpl>>()).iterator->value.append(exportedName.impl());
    }

    void exportBinding(const Identifier& localName)
    {
        exportBinding(localName, localName);
    }

private:
    ModuleScopeData() = default;

    IdentifierSet m_exportedNames { };
    IdentifierAliasMap m_exportedBindings { };
};

} // namespace
