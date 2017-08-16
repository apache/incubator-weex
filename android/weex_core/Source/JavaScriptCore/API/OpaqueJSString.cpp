/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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
#include "OpaqueJSString.h"

#include "CallFrame.h"
#include "Identifier.h"
#include "IdentifierInlines.h"
#include "JSGlobalObject.h"
#include <wtf/text/StringView.h>

using namespace JSC;

RefPtr<OpaqueJSString> OpaqueJSString::create(const String& string)
{
    if (string.isNull())
        return nullptr;

    return adoptRef(new OpaqueJSString(string));
}

OpaqueJSString::~OpaqueJSString()
{
    // m_characters is put in a local here to avoid an extra atomic load.
    UChar* characters = m_characters;
    if (!characters)
        return;

    if (!m_string.is8Bit() && m_string.characters16() == characters)
        return;

    fastFree(characters);
}

String OpaqueJSString::string() const
{
    // Return a copy of the wrapped string, because the caller may make it an Identifier.
    return m_string.isolatedCopy();
}

Identifier OpaqueJSString::identifier(VM* vm) const
{
    if (m_string.isNull())
        return Identifier();

    if (m_string.isEmpty())
        return Identifier(Identifier::EmptyIdentifier);

    if (m_string.is8Bit())
        return Identifier::fromString(vm, m_string.characters8(), m_string.length());

    return Identifier::fromString(vm, m_string.characters16(), m_string.length());
}

const UChar* OpaqueJSString::characters()
{
    // m_characters is put in a local here to avoid an extra atomic load.
    UChar* characters = m_characters;
    if (characters)
        return characters;

    if (m_string.isNull())
        return nullptr;

    unsigned length = m_string.length();
    UChar* newCharacters = static_cast<UChar*>(fastMalloc(length * sizeof(UChar)));
    StringView(m_string).getCharactersWithUpconvert(newCharacters);

    if (!m_characters.compare_exchange_strong(characters, newCharacters)) {
        fastFree(newCharacters);
        return characters;
    }

    return newCharacters;
}

bool OpaqueJSString::equal(const OpaqueJSString* a, const OpaqueJSString* b)
{
    if (a == b)
        return true;

    if (!a || !b)
        return false;

    return a->m_string == b->m_string;
}
