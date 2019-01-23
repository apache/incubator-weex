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
#ifndef Base64_h
#define Base64_h

#include <wtf/Vector.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

namespace WTF {

enum Base64EncodePolicy {
    Base64DoNotInsertLFs,
    Base64InsertLFs,
    Base64URLPolicy // No padding, no LFs.
};

enum Base64DecodeOptions {
    Base64Default = 0,
    Base64ValidatePadding = 1 << 0,
    Base64IgnoreSpacesAndNewLines = 1 << 1,
};

class SignedOrUnsignedCharVectorAdapter {
public:
    SignedOrUnsignedCharVectorAdapter(Vector<char>& vector)
        : m_isSigned(true)
    {
        m_vector.c = &vector;
    }
    SignedOrUnsignedCharVectorAdapter(Vector<uint8_t>& vector)
        : m_isSigned(false)
    {
        m_vector.u = &vector;
    }

    uint8_t* data()
    {
        if (m_isSigned)
            return reinterpret_cast<uint8_t*>(m_vector.c->data());
        return m_vector.u->data();
    }
    
    size_t size() const
    {
        if (m_isSigned)
            return m_vector.c->size();
        return m_vector.u->size();
    }
    
    void clear()
    {
        if (m_isSigned) {
            m_vector.c->clear();
            return;
        }
        m_vector.u->clear();
    }
    
    void grow(size_t size)
    {
        if (m_isSigned) {
            m_vector.c->grow(size);
            return;
        }
        m_vector.u->grow(size);
    }
    
    void shrink(size_t size)
    {
        if (m_isSigned) {
            m_vector.c->shrink(size);
            return;
        }
        m_vector.u->shrink(size);
    }
    
    uint8_t& operator[](size_t position) { return data()[position]; }

private:
    bool m_isSigned;
    union {
        Vector<char>* c;
        Vector<uint8_t>* u;
    } m_vector;
};

class ConstSignedOrUnsignedCharVectorAdapter {
public:
    ConstSignedOrUnsignedCharVectorAdapter(const Vector<char>& vector)
        : m_isSigned(false)
    {
        m_vector.c = &vector;
    }
    ConstSignedOrUnsignedCharVectorAdapter(const Vector<uint8_t>& vector)
        : m_isSigned(true)
    {
        m_vector.u = &vector;
    }

    const uint8_t* data() const
    {
        if (m_isSigned)
            return reinterpret_cast<const uint8_t*>(m_vector.c->data());
        return m_vector.u->data();
    }
    size_t size() const
    {
        if (m_isSigned)
            return m_vector.c->size();
        return m_vector.u->size();
    }

private:
    bool m_isSigned;
    union {
        const Vector<char>* c;
        const Vector<uint8_t>* u;
    } m_vector;
};

WTF_EXPORT_PRIVATE void base64Encode(const void*, unsigned, Vector<char>&, Base64EncodePolicy = Base64DoNotInsertLFs);
void base64Encode(ConstSignedOrUnsignedCharVectorAdapter, Vector<char>&, Base64EncodePolicy = Base64DoNotInsertLFs);
void base64Encode(const CString&, Vector<char>&, Base64EncodePolicy = Base64DoNotInsertLFs);
String base64Encode(const void*, unsigned, Base64EncodePolicy = Base64DoNotInsertLFs);
String base64Encode(ConstSignedOrUnsignedCharVectorAdapter, Base64EncodePolicy = Base64DoNotInsertLFs);
String base64Encode(const CString&, Base64EncodePolicy = Base64DoNotInsertLFs);

bool base64Decode(const String&, SignedOrUnsignedCharVectorAdapter, unsigned options = Base64Default);
WTF_EXPORT_PRIVATE bool base64Decode(const Vector<char>&, SignedOrUnsignedCharVectorAdapter, unsigned options = Base64Default);
WTF_EXPORT_PRIVATE bool base64Decode(const char*, unsigned, SignedOrUnsignedCharVectorAdapter, unsigned options = Base64Default);

inline void base64Encode(ConstSignedOrUnsignedCharVectorAdapter in, Vector<char>& out, Base64EncodePolicy policy)
{
    base64Encode(in.data(), in.size(), out, policy);
}

inline void base64Encode(const CString& in, Vector<char>& out, Base64EncodePolicy policy)
{
    base64Encode(in.data(), in.length(), out, policy);
}

inline String base64Encode(ConstSignedOrUnsignedCharVectorAdapter in, Base64EncodePolicy policy)
{
    return base64Encode(in.data(), in.size(), policy);
}

inline String base64Encode(const CString& in, Base64EncodePolicy policy)
{
    return base64Encode(in.data(), in.length(), policy);
}

// ======================================================================================
// All the same functions modified for base64url, as defined in RFC 4648.
// This format uses '-' and '_' instead of '+' and '/' respectively.
// ======================================================================================

WTF_EXPORT_PRIVATE void base64URLEncode(const void*, unsigned, Vector<char>&);
void base64URLEncode(ConstSignedOrUnsignedCharVectorAdapter, Vector<char>&);
void base64URLEncode(const CString&, Vector<char>&);

WTF_EXPORT_PRIVATE String base64URLEncode(const void*, unsigned);
String base64URLEncode(ConstSignedOrUnsignedCharVectorAdapter);
String base64URLEncode(const CString&);

WTF_EXPORT_PRIVATE bool base64URLDecode(const String&, SignedOrUnsignedCharVectorAdapter);
WTF_EXPORT_PRIVATE bool base64URLDecode(const Vector<char>&, SignedOrUnsignedCharVectorAdapter);
WTF_EXPORT_PRIVATE bool base64URLDecode(const char*, unsigned, SignedOrUnsignedCharVectorAdapter);

inline void base64URLEncode(ConstSignedOrUnsignedCharVectorAdapter in, Vector<char>& out)
{
    base64URLEncode(in.data(), in.size(), out);
}

inline void base64URLEncode(const CString& in, Vector<char>& out)
{
    base64URLEncode(in.data(), in.length(), out);
}

inline String base64URLEncode(ConstSignedOrUnsignedCharVectorAdapter in)
{
    return base64URLEncode(in.data(), in.size());
}

inline String base64URLEncode(const CString& in)
{
    return base64URLEncode(in.data(), in.length());
}

} // namespace WTF

using WTF::Base64EncodePolicy;
using WTF::Base64DoNotInsertLFs;
using WTF::Base64InsertLFs;
using WTF::Base64ValidatePadding;
using WTF::Base64IgnoreSpacesAndNewLines;
using WTF::base64Encode;
using WTF::base64Decode;
using WTF::base64URLDecode;

#endif // Base64_h
