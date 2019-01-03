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

#include "TemplateRegistryKey.h"
#include <wtf/Forward.h>
#include <wtf/HashSet.h>
#include <wtf/Noncopyable.h>

namespace JSC {

class TemplateRegistryKeyTable {
    WTF_MAKE_NONCOPYABLE(TemplateRegistryKeyTable);
public:
    using StringVector = Vector<String, 4>;
    using OptionalStringVector = Vector<std::optional<String>, 4>;

    TemplateRegistryKeyTable() = default;

    Ref<TemplateRegistryKey> createKey(StringVector&& rawStrings, OptionalStringVector&& cookedStrings);

    void unregister(TemplateRegistryKey&);

    ~TemplateRegistryKeyTable();

private:
    struct KeyHash {
        static unsigned hash(const TemplateRegistryKey* key) { return key->hash(); }
        static bool equal(const TemplateRegistryKey* a, const TemplateRegistryKey* b) { return *a == *b; }
        static const bool safeToCompareToEmptyOrDeleted = false;
    };

    HashSet<TemplateRegistryKey*, KeyHash> m_atomicTable;
};

} // namespace JSC
