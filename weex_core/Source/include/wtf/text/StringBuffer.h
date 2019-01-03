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
#ifndef StringBuffer_h
#define StringBuffer_h

#include <wtf/Assertions.h>
#include <limits>
#include <unicode/utypes.h>

namespace WTF {

template <typename CharType>
class StringBuffer {
    WTF_MAKE_NONCOPYABLE(StringBuffer);
public:
    explicit StringBuffer(unsigned length)
        : m_length(length)
        , m_data(m_length ? static_cast<CharType*>(fastMalloc((Checked<size_t>(m_length) * sizeof(CharType)).unsafeGet())) : nullptr)
    {
    }

    ~StringBuffer()
    {
        fastFree(m_data);
    }

    void shrink(unsigned newLength)
    {
        ASSERT(newLength <= m_length);
        m_length = newLength;
    }

    void resize(unsigned newLength)
    {
        if (newLength > m_length) {
            if (newLength > std::numeric_limits<unsigned>::max() / sizeof(UChar))
                CRASH();
            m_data = static_cast<UChar*>(fastRealloc(m_data, newLength * sizeof(UChar)));
        }
        m_length = newLength;
    }

    unsigned length() const { return m_length; }
    CharType* characters() { return m_data; }

    CharType& operator[](unsigned i) { ASSERT_WITH_SECURITY_IMPLICATION(i < m_length); return m_data[i]; }

    MallocPtr<CharType> release()
    {
        CharType* data = m_data;
        m_data = 0;
        return adoptMallocPtr(data);
    }

private:
    unsigned m_length;
    CharType* m_data;
};

} // namespace WTF

using WTF::StringBuffer;

#endif // StringBuffer_h
