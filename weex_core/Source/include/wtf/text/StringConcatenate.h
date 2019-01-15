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
#ifndef StringConcatenate_h
#define StringConcatenate_h

#include <string.h>

#ifndef AtomicString_h
#include <wtf/text/AtomicString.h>
#endif

#ifndef StringView_h
#include <wtf/text/StringView.h>
#endif

// This macro is helpful for testing how many intermediate Strings are created while evaluating an
// expression containing operator+.
#ifndef WTF_STRINGTYPEADAPTER_COPIED_WTF_STRING
#define WTF_STRINGTYPEADAPTER_COPIED_WTF_STRING() ((void)0)
#endif

namespace WTF {

template<typename StringType>
class StringTypeAdapter;

template<>
class StringTypeAdapter<char> {
public:
    StringTypeAdapter<char>(char character)
        : m_character(character)
    {
    }

    unsigned length() { return 1; }
    bool is8Bit() { return true; }

    void writeTo(LChar* destination) const
    {
        *destination = m_character;
    }

    void writeTo(UChar* destination) const
    {
        *destination = m_character;
    }

    String toString() const { return String(&m_character, 1); }

private:
    char m_character;
};

template<>
class StringTypeAdapter<UChar> {
public:
    StringTypeAdapter<UChar>(UChar character)
        : m_character(character)
    {
    }

    unsigned length() const { return 1; }
    bool is8Bit() const { return m_character <= 0xff; }

    void writeTo(LChar* destination) const
    {
        ASSERT(is8Bit());
        *destination = static_cast<LChar>(m_character);
    }

    void writeTo(UChar* destination) const
    {
        *destination = m_character;
    }

    String toString() const { return String(&m_character, 1); }

private:
    UChar m_character;
};

template<>
class StringTypeAdapter<const LChar*> {
public:
    StringTypeAdapter(const LChar* characters)
        : m_characters(characters)
        , m_length(strlen(reinterpret_cast<const char*>(characters)))
    {
    }

    unsigned length() const { return m_length; }
    bool is8Bit() const { return true; }

    void writeTo(LChar* destination) const
    {
        StringView(m_characters, m_length).getCharactersWithUpconvert(destination);
    }

    void writeTo(UChar* destination) const
    {
        StringView(m_characters, m_length).getCharactersWithUpconvert(destination);
    }

    String toString() const { return String(m_characters, m_length); }

private:
    const LChar* m_characters;
    unsigned m_length;
};

template<>
class StringTypeAdapter<const UChar*> {
public:
    StringTypeAdapter(const UChar* characters)
        : m_characters(characters)
    {
        unsigned length = 0;
        while (m_characters[length])
            ++length;

        if (length > std::numeric_limits<unsigned>::max()) // FIXME this is silly https://bugs.webkit.org/show_bug.cgi?id=165790
            CRASH();

        m_length = length;
    }

    unsigned length() const { return m_length; }
    bool is8Bit() const { return false; }

    NO_RETURN_DUE_TO_CRASH void writeTo(LChar*) const
    {
        CRASH(); // FIXME make this a compile-time failure https://bugs.webkit.org/show_bug.cgi?id=165791
    }

    void writeTo(UChar* destination) const
    {
        memcpy(destination, m_characters, m_length * sizeof(UChar));
    }

    String toString() const { return String(m_characters, m_length); }

private:
    const UChar* m_characters;
    unsigned m_length;
};

template<>
class StringTypeAdapter<const char*> : public StringTypeAdapter<const LChar*> {
public:
    StringTypeAdapter(const char* characters)
        : StringTypeAdapter<const LChar*>(reinterpret_cast<const LChar*>(characters))
    {
    }
};

template<>
class StringTypeAdapter<char*> : public StringTypeAdapter<const char*> {
public:
    StringTypeAdapter(const char* characters)
        : StringTypeAdapter<const char*>(characters)
    {
    }
};

template<>
class StringTypeAdapter<ASCIILiteral> : public StringTypeAdapter<const char*> {
public:
    StringTypeAdapter(ASCIILiteral characters)
        : StringTypeAdapter<const char*>(characters)
    {
    }
};

template<>
class StringTypeAdapter<Vector<char>> {
public:
    StringTypeAdapter(const Vector<char>& vector)
        : m_vector(vector)
    {
    }

    size_t length() const { return m_vector.size(); }
    bool is8Bit() const { return true; }

    void writeTo(LChar* destination) const
    {
        StringView(reinterpret_cast<const LChar*>(m_vector.data()), m_vector.size()).getCharactersWithUpconvert(destination);
    }

    void writeTo(UChar* destination) const
    {
        StringView(reinterpret_cast<const LChar*>(m_vector.data()), m_vector.size()).getCharactersWithUpconvert(destination);
    }

    String toString() const { return String(m_vector.data(), m_vector.size()); }

private:
    const Vector<char>& m_vector;
};

template<>
class StringTypeAdapter<String> {
public:
    StringTypeAdapter<String>(const String& string)
        : m_string(string)
    {
    }

    unsigned length() const { return m_string.length(); }
    bool is8Bit() const { return m_string.isNull() || m_string.is8Bit(); }

    void writeTo(LChar* destination) const
    {
        StringView(m_string).getCharactersWithUpconvert(destination);
        WTF_STRINGTYPEADAPTER_COPIED_WTF_STRING();
    }

    void writeTo(UChar* destination) const
    {
        StringView(m_string).getCharactersWithUpconvert(destination);
        WTF_STRINGTYPEADAPTER_COPIED_WTF_STRING();
    }

    String toString() const { return m_string; }

private:
    const String& m_string;
};

template<>
class StringTypeAdapter<AtomicString> : public StringTypeAdapter<String> {
public:
    StringTypeAdapter(const AtomicString& string)
        : StringTypeAdapter<String>(string.string())
    {
    }
};

inline void sumWithOverflow(bool& overflow, unsigned& total, unsigned addend)
{
    unsigned oldTotal = total;
    total = oldTotal + addend;
    if (total < oldTotal)
        overflow = true;
}

template<typename... Unsigned>
inline void sumWithOverflow(bool& overflow, unsigned& total, unsigned addend, Unsigned ...addends)
{
    unsigned oldTotal = total;
    total = oldTotal + addend;
    if (total < oldTotal)
        overflow = true;
    sumWithOverflow(overflow, total, addends...);
}

template<typename Adapter>
inline bool are8Bit(Adapter adapter)
{
    return adapter.is8Bit();
}

template<typename Adapter, typename... Adapters>
inline bool are8Bit(Adapter adapter, Adapters ...adapters)
{
    return adapter.is8Bit() && are8Bit(adapters...);
}

template<typename ResultType, typename Adapter>
inline void makeStringAccumulator(ResultType* result, Adapter adapter)
{
    adapter.writeTo(result);
}

template<typename ResultType, typename Adapter, typename... Adapters>
inline void makeStringAccumulator(ResultType* result, Adapter adapter, Adapters ...adapters)
{
    adapter.writeTo(result);
    makeStringAccumulator(result + adapter.length(), adapters...);
}

template<typename StringTypeAdapter, typename... StringTypeAdapters>
String tryMakeStringFromAdapters(StringTypeAdapter adapter, StringTypeAdapters ...adapters)
{
    bool overflow = false;
    unsigned length = adapter.length();
    sumWithOverflow(overflow, length, adapters.length()...);
    if (overflow)
        return String();

    if (are8Bit(adapter, adapters...)) {
        LChar* buffer;
        RefPtr<StringImpl> resultImpl = StringImpl::tryCreateUninitialized(length, buffer);
        if (!resultImpl)
            return String();

        makeStringAccumulator(buffer, adapter, adapters...);

        return WTFMove(resultImpl);
    }

    UChar* buffer;
    RefPtr<StringImpl> resultImpl = StringImpl::tryCreateUninitialized(length, buffer);
    if (!resultImpl)
        return String();

    makeStringAccumulator(buffer, adapter, adapters...);

    return WTFMove(resultImpl);
}

template<typename... StringTypes>
String tryMakeString(StringTypes ...strings)
{
    return tryMakeStringFromAdapters(StringTypeAdapter<StringTypes>(strings)...);
}

// Convenience only.
template<typename StringType>
String makeString(StringType string)
{
    return String(string);
}

template<typename... StringTypes>
String makeString(StringTypes... strings)
{
    String result = tryMakeString(strings...);
    if (!result)
        CRASH();
    return result;
}

} // namespace WTF

using WTF::makeString;
using WTF::tryMakeString;

#include <wtf/text/StringOperators.h>
#endif
