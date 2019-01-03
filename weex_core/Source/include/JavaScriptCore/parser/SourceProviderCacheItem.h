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

#include "ParserTokens.h"
#include <wtf/Vector.h>
#include <wtf/text/UniquedStringImpl.h>
#include <wtf/text/WTFString.h>

namespace JSC {

struct SourceProviderCacheItemCreationParameters {
    unsigned functionNameStart;
    unsigned lastTokenLine;
    unsigned lastTokenStartOffset;
    unsigned lastTokenEndOffset;
    unsigned lastTokenLineStartOffset;
    unsigned endFunctionOffset;
    unsigned parameterCount;
    unsigned functionLength;
    bool needsFullActivation;
    bool usesEval;
    bool strictMode;
    bool needsSuperBinding;
    InnerArrowFunctionCodeFeatures innerArrowFunctionFeatures;
    Vector<UniquedStringImpl*, 8> usedVariables;
    bool isBodyArrowExpression { false };
    JSTokenType tokenType { CLOSEBRACE };
    ConstructorKind constructorKind;
    SuperBinding expectedSuperBinding;
};

#if COMPILER(MSVC)
#pragma warning(push)
#pragma warning(disable: 4200) // Disable "zero-sized array in struct/union" warning
#endif

class SourceProviderCacheItem {
    WTF_MAKE_FAST_ALLOCATED;
public:
    static std::unique_ptr<SourceProviderCacheItem> create(const SourceProviderCacheItemCreationParameters&);
    ~SourceProviderCacheItem();

    JSToken endFunctionToken() const 
    {
        JSToken token;
        token.m_type = isBodyArrowExpression ? tokenType : CLOSEBRACE;
        token.m_data.offset = lastTokenStartOffset;
        token.m_location.startOffset = lastTokenStartOffset;
        token.m_location.endOffset = lastTokenEndOffset;
        token.m_location.line = lastTokenLine;
        token.m_location.lineStartOffset = lastTokenLineStartOffset;
        // token.m_location.sourceOffset is initialized once by the client. So,
        // we do not need to set it here.
        return token;
    }

    unsigned functionNameStart : 31;
    bool needsFullActivation : 1;
    unsigned endFunctionOffset : 31;
    bool usesEval : 1;
    unsigned lastTokenLine : 31;
    bool strictMode : 1;
    unsigned lastTokenStartOffset : 31;
    unsigned lastTokenEndOffset: 31;
    unsigned constructorKind : 2; // ConstructorKind
    unsigned parameterCount : 31;
    unsigned expectedSuperBinding : 1; // SuperBinding
    bool needsSuperBinding: 1;
    unsigned functionLength;
    unsigned lastTokenLineStartOffset;
    unsigned usedVariablesCount;
    InnerArrowFunctionCodeFeatures innerArrowFunctionFeatures;
    bool isBodyArrowExpression;
    JSTokenType tokenType;

    UniquedStringImpl** usedVariables() const { return const_cast<UniquedStringImpl**>(m_variables); }

private:
    SourceProviderCacheItem(const SourceProviderCacheItemCreationParameters&);

    UniquedStringImpl* m_variables[0];
};

inline SourceProviderCacheItem::~SourceProviderCacheItem()
{
    for (unsigned i = 0; i < usedVariablesCount; ++i)
        m_variables[i]->deref();
}

inline std::unique_ptr<SourceProviderCacheItem> SourceProviderCacheItem::create(const SourceProviderCacheItemCreationParameters& parameters)
{
    size_t variableCount = parameters.usedVariables.size();
    size_t objectSize = sizeof(SourceProviderCacheItem) + sizeof(UniquedStringImpl*) * variableCount;
    void* slot = fastMalloc(objectSize);
    return std::unique_ptr<SourceProviderCacheItem>(new (slot) SourceProviderCacheItem(parameters));
}

inline SourceProviderCacheItem::SourceProviderCacheItem(const SourceProviderCacheItemCreationParameters& parameters)
    : functionNameStart(parameters.functionNameStart)
    , needsFullActivation(parameters.needsFullActivation)
    , endFunctionOffset(parameters.endFunctionOffset)
    , usesEval(parameters.usesEval)
    , lastTokenLine(parameters.lastTokenLine)
    , strictMode(parameters.strictMode)
    , lastTokenStartOffset(parameters.lastTokenStartOffset)
    , lastTokenEndOffset(parameters.lastTokenEndOffset)
    , constructorKind(static_cast<unsigned>(parameters.constructorKind))
    , parameterCount(parameters.parameterCount)
    , expectedSuperBinding(static_cast<unsigned>(parameters.expectedSuperBinding))
    , needsSuperBinding(parameters.needsSuperBinding)
    , functionLength(parameters.functionLength)
    , lastTokenLineStartOffset(parameters.lastTokenLineStartOffset)
    , usedVariablesCount(parameters.usedVariables.size())
    , innerArrowFunctionFeatures(parameters.innerArrowFunctionFeatures)
    , isBodyArrowExpression(parameters.isBodyArrowExpression)
    , tokenType(parameters.tokenType)
{
    for (unsigned i = 0; i < usedVariablesCount; ++i) {
        m_variables[i] = parameters.usedVariables[i];
        m_variables[i]->ref();
    }
}

#if COMPILER(MSVC)
#pragma warning(pop)
#endif

} // namespace JSC
