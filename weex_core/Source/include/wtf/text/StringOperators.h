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
#ifndef StringOperators_h
#define StringOperators_h

namespace WTF {

template<typename StringType1, typename StringType2>
class StringAppend {
public:
    StringAppend(StringType1 string1, StringType2 string2)
        : m_string1(string1)
        , m_string2(string2)
    {
    }

    operator String() const
    {
        String result = tryMakeString(m_string1, m_string2);
        if (!result)
            CRASH();
        return result;
    }

    operator AtomicString() const
    {
        return operator String();
    }

    bool is8Bit()
    {
        StringTypeAdapter<StringType1> adapter1(m_string1);
        StringTypeAdapter<StringType2> adapter2(m_string2);
        return adapter1.is8Bit() && adapter2.is8Bit();
    }

    void writeTo(LChar* destination)
    {
        ASSERT(is8Bit());
        StringTypeAdapter<StringType1> adapter1(m_string1);
        StringTypeAdapter<StringType2> adapter2(m_string2);
        adapter1.writeTo(destination);
        adapter2.writeTo(destination + adapter1.length());
    }

    void writeTo(UChar* destination)
    {
        StringTypeAdapter<StringType1> adapter1(m_string1);
        StringTypeAdapter<StringType2> adapter2(m_string2);
        adapter1.writeTo(destination);
        adapter2.writeTo(destination + adapter1.length());
    }

    unsigned length()
    {
        StringTypeAdapter<StringType1> adapter1(m_string1);
        StringTypeAdapter<StringType2> adapter2(m_string2);
        return adapter1.length() + adapter2.length();
    }    

private:
    StringType1 m_string1;
    StringType2 m_string2;
};

template<typename StringType1, typename StringType2>
class StringTypeAdapter<StringAppend<StringType1, StringType2>> {
public:
    StringTypeAdapter<StringAppend<StringType1, StringType2>>(StringAppend<StringType1, StringType2>& buffer)
        : m_buffer(buffer)
    {
    }

    unsigned length() { return m_buffer.length(); }

    bool is8Bit() { return m_buffer.is8Bit(); }

    void writeTo(LChar* destination) { m_buffer.writeTo(destination); }
    void writeTo(UChar* destination) { m_buffer.writeTo(destination); }

    String toString() const { return m_buffer; }

private:
    StringAppend<StringType1, StringType2>& m_buffer;
};

inline StringAppend<const char*, String> operator+(const char* string1, const String& string2)
{
    return StringAppend<const char*, String>(string1, string2);
}

inline StringAppend<const char*, AtomicString> operator+(const char* string1, const AtomicString& string2)
{
    return StringAppend<const char*, AtomicString>(string1, string2);
}

template<typename U, typename V>
inline StringAppend<const char*, StringAppend<U, V>> operator+(const char* string1, const StringAppend<U, V>& string2)
{
    return StringAppend<const char*, StringAppend<U, V>>(string1, string2);
}

inline StringAppend<const UChar*, String> operator+(const UChar* string1, const String& string2)
{
    return StringAppend<const UChar*, String>(string1, string2);
}

inline StringAppend<const UChar*, AtomicString> operator+(const UChar* string1, const AtomicString& string2)
{
    return StringAppend<const UChar*, AtomicString>(string1, string2);
}

template<typename U, typename V>
inline StringAppend<const UChar*, StringAppend<U, V>> operator+(const UChar* string1, const StringAppend<U, V>& string2)
{
    return StringAppend<const UChar*, StringAppend<U, V>>(string1, string2);
}

inline StringAppend<ASCIILiteral, String> operator+(const ASCIILiteral& string1, const String& string2)
{
    return StringAppend<ASCIILiteral, String>(string1, string2);
}

inline StringAppend<ASCIILiteral, AtomicString> operator+(const ASCIILiteral& string1, const AtomicString& string2)
{
    return StringAppend<ASCIILiteral, AtomicString>(string1, string2);
}

template<typename U, typename V>
inline StringAppend<ASCIILiteral, StringAppend<U, V>> operator+(const ASCIILiteral& string1, const StringAppend<U, V>& string2)
{
    return StringAppend<ASCIILiteral, StringAppend<U, V>>(string1, string2);
}

template<typename T>
StringAppend<String, T> operator+(const String& string1, T string2)
{
    return StringAppend<String, T>(string1, string2);
}

template<typename U, typename V, typename W>
StringAppend<StringAppend<U, V>, W> operator+(const StringAppend<U, V>& string1, W string2)
{
    return StringAppend<StringAppend<U, V>, W>(string1, string2);
}

} // namespace WTF

#endif // StringOperators_h
