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

#include "SourceOrigin.h"
#include <wtf/RefCounted.h>
#include <wtf/text/TextPosition.h>
#include <wtf/text/WTFString.h>

namespace JSC {

    enum class SourceProviderSourceType {
        Program,
        Module,
        WebAssembly,
    };

    class SourceProvider : public RefCounted<SourceProvider> {
    public:
        static const intptr_t nullID = 1;
        
        JS_EXPORT_PRIVATE SourceProvider(const SourceOrigin&, const String& url, const TextPosition& startPosition, SourceProviderSourceType);

        JS_EXPORT_PRIVATE virtual ~SourceProvider();

        virtual unsigned hash() const = 0;
        virtual StringView source() const = 0;
        StringView getRange(int start, int end) const
        {
            return source().substring(start, end - start);
        }

        const SourceOrigin& sourceOrigin() const { return m_sourceOrigin; }
        const String& url() const { return m_url; }
        const String& sourceURL() const { return m_sourceURLDirective; }
        const String& sourceMappingURL() const { return m_sourceMappingURLDirective; }

        TextPosition startPosition() const { return m_startPosition; }
        SourceProviderSourceType sourceType() const { return m_sourceType; }

        intptr_t asID()
        {
            if (!m_id)
                getID();
            return m_id;
        }

        bool isValid() const { return m_validated; }
        void setValid() { m_validated = true; }

        void setSourceURLDirective(const String& sourceURL) { m_sourceURLDirective = sourceURL; }
        void setSourceMappingURLDirective(const String& sourceMappingURL) { m_sourceMappingURLDirective = sourceMappingURL; }

    private:
        JS_EXPORT_PRIVATE void getID();

        SourceOrigin m_sourceOrigin;
        String m_url;
        String m_sourceURLDirective;
        String m_sourceMappingURLDirective;
        TextPosition m_startPosition;
        SourceProviderSourceType m_sourceType;
        bool m_validated : 1;
        uintptr_t m_id : sizeof(uintptr_t) * 8 - 1;
    };

    class StringSourceProvider : public SourceProvider {
    public:
        static Ref<StringSourceProvider> create(const String& source, const SourceOrigin& sourceOrigin, const String& url, const TextPosition& startPosition = TextPosition(), SourceProviderSourceType sourceType = SourceProviderSourceType::Program)
        {
            return adoptRef(*new StringSourceProvider(source, sourceOrigin, url, startPosition, sourceType));
        }
        
        unsigned hash() const override
        {
            return m_source.get().hash();
        }

        StringView source() const override
        {
            return m_source.get();
        }

    private:
        StringSourceProvider(const String& source, const SourceOrigin& sourceOrigin, const String& url, const TextPosition& startPosition, SourceProviderSourceType sourceType)
            : SourceProvider(sourceOrigin, url, startPosition, sourceType)
            , m_source(source.isNull() ? *StringImpl::empty() : *source.impl())
        {
        }

        Ref<StringImpl> m_source;
    };
    
#if ENABLE(WEBASSEMBLY)
    class WebAssemblySourceProvider : public SourceProvider {
    public:
        static Ref<WebAssemblySourceProvider> create(const Vector<uint8_t>& data, const SourceOrigin& sourceOrigin, const String& url)
        {
            return adoptRef(*new WebAssemblySourceProvider(data, sourceOrigin, url));
        }

        unsigned hash() const override
        {
            return m_source.impl()->hash();
        }

        StringView source() const override
        {
            return m_source;
        }

        const Vector<uint8_t>& data() const
        {
            return m_data;
        }

    private:
        WebAssemblySourceProvider(const Vector<uint8_t>& data, const SourceOrigin& sourceOrigin, const String& url)
            : SourceProvider(sourceOrigin, url, TextPosition(), SourceProviderSourceType::WebAssembly)
            , m_source("[WebAssembly source]")
            , m_data(data)
        {
        }

        String m_source;
        Vector<uint8_t> m_data;
    };
#endif

} // namespace JSC
