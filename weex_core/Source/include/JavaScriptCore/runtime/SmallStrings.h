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

#include "TypeofType.h"
#include "WriteBarrier.h"
#include <wtf/Noncopyable.h>

#define JSC_COMMON_STRINGS_EACH_NAME(macro) \
    macro(default) \
    macro(boolean) \
    macro(false) \
    macro(function) \
    macro(number) \
    macro(null) \
    macro(object) \
    macro(undefined) \
    macro(string) \
    macro(symbol) \
    macro(true)

namespace WTF {
class StringImpl;
}

namespace JSC {

class VM;
class JSString;
class SmallStringsStorage;
class SlotVisitor;

static const unsigned maxSingleCharacterString = 0xFF;

class SmallStrings {
    WTF_MAKE_NONCOPYABLE(SmallStrings);
public:
    SmallStrings();
    ~SmallStrings();

    JSString* emptyString()
    {
        return m_emptyString;
    }

    JSString* singleCharacterString(unsigned char character)
    {
        return m_singleCharacterStrings[character];
    }

    JS_EXPORT_PRIVATE WTF::StringImpl& singleCharacterStringRep(unsigned char character);

    JSString** singleCharacterStrings() { return &m_singleCharacterStrings[0]; }

    void initializeCommonStrings(VM&);
    void visitStrongReferences(SlotVisitor&);

#define JSC_COMMON_STRINGS_ACCESSOR_DEFINITION(name) \
    JSString* name##String() const                   \
    {                                                \
        return m_##name;                             \
    }
    JSC_COMMON_STRINGS_EACH_NAME(JSC_COMMON_STRINGS_ACCESSOR_DEFINITION)
#undef JSC_COMMON_STRINGS_ACCESSOR_DEFINITION
    
    JSString* typeString(TypeofType type) const
    {
        switch (type) {
        case TypeofType::Undefined:
            return undefinedString();
        case TypeofType::Boolean:
            return booleanString();
        case TypeofType::Number:
            return numberString();
        case TypeofType::String:
            return stringString();
        case TypeofType::Symbol:
            return symbolString();
        case TypeofType::Object:
            return objectString();
        case TypeofType::Function:
            return functionString();
        }
        
        RELEASE_ASSERT_NOT_REACHED();
        return nullptr;
    }

    JSString* objectStringStart() const { return m_objectStringStart; }
    JSString* nullObjectString() const { return m_nullObjectString; }
    JSString* undefinedObjectString() const { return m_undefinedObjectString; }

    bool needsToBeVisited(CollectionScope scope) const
    {
        if (scope == CollectionScope::Full)
            return true;
        return m_needsToBeVisited;
    }

private:
    static const unsigned singleCharacterStringCount = maxSingleCharacterString + 1;

    void createEmptyString(VM*);
    void createSingleCharacterString(VM*, unsigned char);

    void initialize(VM*, JSString*&, const char* value);

    JSString* m_emptyString;
#define JSC_COMMON_STRINGS_ATTRIBUTE_DECLARATION(name) JSString* m_##name;
    JSC_COMMON_STRINGS_EACH_NAME(JSC_COMMON_STRINGS_ATTRIBUTE_DECLARATION)
#undef JSC_COMMON_STRINGS_ATTRIBUTE_DECLARATION
    JSString* m_objectStringStart;
    JSString* m_nullObjectString;
    JSString* m_undefinedObjectString;
    JSString* m_singleCharacterStrings[singleCharacterStringCount];
    std::unique_ptr<SmallStringsStorage> m_storage;
    bool m_needsToBeVisited;
};

} // namespace JSC
