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
#ifndef OpaqueJSString_h
#define OpaqueJSString_h

#include <atomic>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/text/WTFString.h>

namespace JSC {
    class Identifier;
    class VM;
}

struct OpaqueJSString : public ThreadSafeRefCounted<OpaqueJSString> {
    static Ref<OpaqueJSString> create()
    {
        return adoptRef(*new OpaqueJSString);
    }

    static Ref<OpaqueJSString> create(const LChar* characters, unsigned length)
    {
        return adoptRef(*new OpaqueJSString(characters, length));
    }

    static Ref<OpaqueJSString> create(const UChar* characters, unsigned length)
    {
        return adoptRef(*new OpaqueJSString(characters, length));
    }

    JS_EXPORT_PRIVATE static RefPtr<OpaqueJSString> create(const String&);

    JS_EXPORT_PRIVATE ~OpaqueJSString();

    bool is8Bit() { return m_string.is8Bit(); }
    const LChar* characters8() { return m_string.characters8(); }
    const UChar* characters16() { return m_string.characters16(); }
    unsigned length() { return m_string.length(); }

    const UChar* characters();

    JS_EXPORT_PRIVATE String string() const;
    JSC::Identifier identifier(JSC::VM*) const;

    static bool equal(const OpaqueJSString*, const OpaqueJSString*);

private:
    friend class WTF::ThreadSafeRefCounted<OpaqueJSString>;

    OpaqueJSString()
        : m_characters(nullptr)
    {
    }

    OpaqueJSString(const String& string)
        : m_string(string.isolatedCopy())
        , m_characters(m_string.impl() && m_string.is8Bit() ? nullptr : const_cast<UChar*>(m_string.characters16()))
    {
    }

    OpaqueJSString(const LChar* characters, unsigned length)
        : m_string(characters, length)
        , m_characters(nullptr)
    {
    }

    OpaqueJSString(const UChar* characters, unsigned length)
        : m_string(characters, length)
        , m_characters(m_string.impl() && m_string.is8Bit() ? nullptr : const_cast<UChar*>(m_string.characters16()))
    {
    }

    String m_string;

    // This will be initialized on demand when characters() is called if the string needs up-conversion.
    std::atomic<UChar*> m_characters;
};

#endif
