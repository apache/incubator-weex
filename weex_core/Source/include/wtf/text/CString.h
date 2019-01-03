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
#ifndef CString_h
#define CString_h

#include <wtf/HashFunctions.h>
#include <wtf/HashTraits.h>
#include <wtf/Ref.h>
#include <wtf/RefCounted.h>

namespace WTF {

// CStringBuffer is the ref-counted storage class for the characters in a CString.
// The data is implicitly allocated 1 character longer than length(), as it is zero-terminated.
class CStringBuffer : public RefCounted<CStringBuffer> {
public:
    const char* data() { return mutableData(); }
    size_t length() const { return m_length; }

private:
    friend class CString;

    static Ref<CStringBuffer> createUninitialized(size_t length);

    CStringBuffer(size_t length) : m_length(length) { }
    char* mutableData() { return reinterpret_cast_ptr<char*>(this + 1); }

    const size_t m_length;
};

// A container for a null-terminated char array supporting copy-on-write
// assignment.  The contained char array may be null.
class CString {
public:
    CString() { }
    WTF_EXPORT_PRIVATE CString(const char*);
    WTF_EXPORT_PRIVATE CString(const char*, size_t length);
    CString(CStringBuffer* buffer) : m_buffer(buffer) { }
    WTF_EXPORT_PRIVATE static CString newUninitialized(size_t length, char*& characterBuffer);
    CString(HashTableDeletedValueType) : m_buffer(HashTableDeletedValue) { }

    const char* data() const
    {
        return m_buffer ? m_buffer->data() : 0;
    }
    WTF_EXPORT_PRIVATE char* mutableData();
    size_t length() const
    {
        return m_buffer ? m_buffer->length() : 0;
    }

    bool isNull() const { return !m_buffer; }
    bool isSafeToSendToAnotherThread() const;

    CStringBuffer* buffer() const { return m_buffer.get(); }
    
    bool isHashTableDeletedValue() const { return m_buffer.isHashTableDeletedValue(); }
    
    WTF_EXPORT_PRIVATE unsigned hash() const;

private:
    void copyBufferIfNeeded();
    void init(const char*, size_t length);
    RefPtr<CStringBuffer> m_buffer;
};

WTF_EXPORT_PRIVATE bool operator==(const CString& a, const CString& b);
inline bool operator!=(const CString& a, const CString& b) { return !(a == b); }
WTF_EXPORT_PRIVATE bool operator==(const CString& a, const char* b);
inline bool operator!=(const CString& a, const char* b) { return !(a == b); }
WTF_EXPORT_PRIVATE bool operator<(const CString& a, const CString& b);

struct CStringHash {
    static unsigned hash(const CString& string) { return string.hash(); }
    WTF_EXPORT_PRIVATE static bool equal(const CString& a, const CString& b);
    static const bool safeToCompareToEmptyOrDeleted = true;
};

template<typename T> struct DefaultHash;
template<> struct DefaultHash<CString> {
    typedef CStringHash Hash;
};

template<typename T> struct HashTraits;
template<> struct HashTraits<CString> : SimpleClassHashTraits<CString> { };

} // namespace WTF

using WTF::CString;

#endif // CString_h
