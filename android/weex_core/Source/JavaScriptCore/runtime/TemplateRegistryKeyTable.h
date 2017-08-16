/*
 * Copyright (C) 2016 Yusuke Suzuki <utatane.tea@gmail.com>.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
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
