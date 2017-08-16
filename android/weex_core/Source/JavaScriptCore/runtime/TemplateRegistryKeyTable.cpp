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

#include "config.h"
#include "TemplateRegistryKeyTable.h"

#include "TemplateRegistryKey.h"

namespace JSC {

struct TemplateRegistryKeyTranslator {
    static unsigned hash(TemplateRegistryKey* generator) { return generator->hash(); }
    static inline bool equal(TemplateRegistryKey* key, TemplateRegistryKey* generator) { return *key == *generator; }
    static void translate(TemplateRegistryKey*& location, TemplateRegistryKey* generator, unsigned) { location = generator; }
};

TemplateRegistryKeyTable::~TemplateRegistryKeyTable()
{
    for (auto& key : m_atomicTable)
        key->m_table = nullptr;
}

Ref<TemplateRegistryKey> TemplateRegistryKeyTable::createKey(TemplateRegistryKey::StringVector&& rawStrings, TemplateRegistryKey::OptionalStringVector&& cookedStrings)
{
    auto key = TemplateRegistryKey::create(WTFMove(rawStrings), WTFMove(cookedStrings));
    auto addResult = m_atomicTable.add<TemplateRegistryKeyTranslator>(key.ptr());
    if (addResult.isNewEntry)
        (*addResult.iterator)->m_table = this;

    return **addResult.iterator;
}

void TemplateRegistryKeyTable::unregister(TemplateRegistryKey& key)
{
    ASSERT(key.m_table == this);
    auto iterator = m_atomicTable.find(&key);
    ASSERT_WITH_MESSAGE(iterator != m_atomicTable.end(), "The TemplateRegistryKey being removed is registered in the other TemplateRegistryKeyTable.");
    m_atomicTable.remove(iterator);

}

} // namespace JSC
