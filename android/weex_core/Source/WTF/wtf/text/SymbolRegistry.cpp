/*
 * Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>.
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
#include "SymbolRegistry.h"

namespace WTF {

SymbolRegistry::~SymbolRegistry()
{
    for (auto& key : m_table)
        static_cast<SymbolImpl&>(*key.impl()).symbolRegistry() = nullptr;
}

Ref<SymbolImpl> SymbolRegistry::symbolForKey(const String& rep)
{
    auto addResult = m_table.add(SymbolRegistryKey(rep.impl()));
    if (!addResult.isNewEntry)
        return *static_cast<SymbolImpl*>(addResult.iterator->impl());

    auto symbol = SymbolImpl::create(*rep.impl());
    symbol->symbolRegistry() = this;
    *addResult.iterator = SymbolRegistryKey(&symbol.get());
    return symbol;
}

String SymbolRegistry::keyForSymbol(SymbolImpl& uid)
{
    ASSERT(uid.symbolRegistry() == this);
    return uid.extractFoldedString();
}

void SymbolRegistry::remove(SymbolImpl& uid)
{
    ASSERT(uid.symbolRegistry() == this);
    auto iterator = m_table.find(SymbolRegistryKey(&uid));
    ASSERT_WITH_MESSAGE(iterator != m_table.end(), "The string being removed is registered in the string table of an other thread!");
    m_table.remove(iterator);
}

}
