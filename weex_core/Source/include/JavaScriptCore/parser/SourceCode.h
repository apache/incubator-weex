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

#include "UnlinkedSourceCode.h"

namespace JSC {

    class SourceCode : public UnlinkedSourceCode {
    public:
        SourceCode()
            : UnlinkedSourceCode()
            , m_firstLine(OrdinalNumber::beforeFirst())
            , m_startColumn(OrdinalNumber::beforeFirst())
        {
        }

        SourceCode(Ref<SourceProvider>&& provider)
            : UnlinkedSourceCode(WTFMove(provider))
        {
        }

        SourceCode(Ref<SourceProvider>&& provider, int firstLine, int startColumn)
            : UnlinkedSourceCode(WTFMove(provider))
            , m_firstLine(OrdinalNumber::fromOneBasedInt(std::max(firstLine, 1)))
            , m_startColumn(OrdinalNumber::fromOneBasedInt(std::max(startColumn, 1)))
        {
        }

        SourceCode(Ref<SourceProvider>&& provider, int startOffset, int endOffset, int firstLine, int startColumn)
            : UnlinkedSourceCode(WTFMove(provider), startOffset, endOffset)
            , m_firstLine(OrdinalNumber::fromOneBasedInt(std::max(firstLine, 1)))
            , m_startColumn(OrdinalNumber::fromOneBasedInt(std::max(startColumn, 1)))
        {
        }

        SourceCode(RefPtr<SourceProvider>&& provider, int startOffset, int endOffset, int firstLine, int startColumn)
            : UnlinkedSourceCode(WTFMove(provider), startOffset, endOffset)
            , m_firstLine(OrdinalNumber::fromOneBasedInt(std::max(firstLine, 1)))
            , m_startColumn(OrdinalNumber::fromOneBasedInt(std::max(startColumn, 1)))
        {
        }

        OrdinalNumber firstLine() const { return m_firstLine; }
        OrdinalNumber startColumn() const { return m_startColumn; }

        intptr_t providerID() const
        {
            if (!m_provider)
                return SourceProvider::nullID;
            return m_provider->asID();
        }

        SourceProvider* provider() const { return m_provider.get(); }

        SourceCode subExpression(unsigned openBrace, unsigned closeBrace, int firstLine, int startColumn);

    private:
        OrdinalNumber m_firstLine;
        OrdinalNumber m_startColumn;
    };

    inline SourceCode makeSource(const String& source, const SourceOrigin& sourceOrigin, const String& url = String(), const TextPosition& startPosition = TextPosition(), SourceProviderSourceType sourceType = SourceProviderSourceType::Program)
    {
        return SourceCode(StringSourceProvider::create(source, sourceOrigin, url, startPosition, sourceType), startPosition.m_line.oneBasedInt(), startPosition.m_column.oneBasedInt());
    }
    
    inline SourceCode SourceCode::subExpression(unsigned openBrace, unsigned closeBrace, int firstLine, int startColumn)
    {
        startColumn += 1; // Convert to base 1.
        return SourceCode(RefPtr<SourceProvider> { provider() }, openBrace, closeBrace + 1, firstLine, startColumn);
    }

} // namespace JSC
