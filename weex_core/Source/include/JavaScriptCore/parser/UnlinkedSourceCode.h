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
#pragma once

#include "SourceProvider.h"
#include <wtf/RefPtr.h>

namespace JSC {

    class UnlinkedSourceCode {
    public:
        UnlinkedSourceCode()
            : m_provider(0)
            , m_startOffset(0)
            , m_endOffset(0)
        {
        }

        UnlinkedSourceCode(WTF::HashTableDeletedValueType)
            : m_provider(WTF::HashTableDeletedValue)
        {
        }

        UnlinkedSourceCode(Ref<SourceProvider>&& provider)
            : m_provider(WTFMove(provider))
            , m_startOffset(0)
            , m_endOffset(m_provider->source().length())
        {
        }

        UnlinkedSourceCode(Ref<SourceProvider>&& provider, int startOffset, int endOffset)
            : m_provider(WTFMove(provider))
            , m_startOffset(startOffset)
            , m_endOffset(endOffset)
        {
        }

        UnlinkedSourceCode(RefPtr<SourceProvider>&& provider, int startOffset, int endOffset)
            : m_provider(WTFMove(provider))
            , m_startOffset(startOffset)
            , m_endOffset(endOffset)
        {
        }

        UnlinkedSourceCode(const UnlinkedSourceCode& other)
            : m_provider(other.m_provider)
            , m_startOffset(other.m_startOffset)
            , m_endOffset(other.m_endOffset)
        {
        }

        bool isHashTableDeletedValue() const { return m_provider.isHashTableDeletedValue(); }

        unsigned hash() const
        {
            ASSERT(m_provider);
            return m_provider->hash();
        }

        StringView view() const
        {
            if (!m_provider)
                return StringView();
            return m_provider->getRange(m_startOffset, m_endOffset);
        }
        
        CString toUTF8() const;
        
        bool isNull() const { return !m_provider; }
        int startOffset() const { return m_startOffset; }
        int endOffset() const { return m_endOffset; }
        int length() const { return m_endOffset - m_startOffset; }

    protected:
        // FIXME: Make it Ref<SourceProvidier>.
        // https://bugs.webkit.org/show_bug.cgi?id=168325
        RefPtr<SourceProvider> m_provider;
        int m_startOffset;
        int m_endOffset;
    };

} // namespace JSC
