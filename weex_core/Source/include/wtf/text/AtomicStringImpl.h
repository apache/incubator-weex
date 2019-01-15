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
#ifndef AtomicStringImpl_h
#define AtomicStringImpl_h

#include <wtf/text/UniquedStringImpl.h>

namespace WTF {

class AtomicStringTable;

class AtomicStringImpl : public UniquedStringImpl {
public:
    static RefPtr<AtomicStringImpl> lookUp(LChar* characters, unsigned length)
    {
        return lookUpInternal(characters, length);
    }
    static RefPtr<AtomicStringImpl> lookUp(UChar* characters, unsigned length)
    {
        return lookUpInternal(characters, length);
    }
    static RefPtr<AtomicStringImpl> lookUp(StringImpl* string)
    {
        if (!string || string->isAtomic())
            return static_cast<AtomicStringImpl*>(string);
        return lookUpSlowCase(*string);
    }

    static void remove(AtomicStringImpl*);

    WTF_EXPORT_STRING_API static RefPtr<AtomicStringImpl> add(const LChar*);
    ALWAYS_INLINE static RefPtr<AtomicStringImpl> add(const char* s) { return add(reinterpret_cast<const LChar*>(s)); };
    WTF_EXPORT_STRING_API static RefPtr<AtomicStringImpl> add(const LChar*, unsigned length);
    WTF_EXPORT_STRING_API static RefPtr<AtomicStringImpl> add(const UChar*, unsigned length);
    ALWAYS_INLINE static RefPtr<AtomicStringImpl> add(const char* s, unsigned length) { return add(reinterpret_cast<const LChar*>(s), length); };
    WTF_EXPORT_STRING_API static Ref<AtomicStringImpl> add(const UChar*, unsigned length, unsigned existingHash);
    WTF_EXPORT_STRING_API static RefPtr<AtomicStringImpl> add(const UChar*);
    WTF_EXPORT_STRING_API static RefPtr<AtomicStringImpl> add(StringImpl*, unsigned offset, unsigned length);
    ALWAYS_INLINE static RefPtr<AtomicStringImpl> add(StringImpl* string)
    {
        if (!string)
            return static_cast<AtomicStringImpl*>(string);
        return add(*string);
    }
    WTF_EXPORT_STRING_API static Ref<AtomicStringImpl> addLiteral(const char* characters, unsigned length);

    // Returns null if the input data contains an invalid UTF-8 sequence.
    WTF_EXPORT_STRING_API static RefPtr<AtomicStringImpl> addUTF8(const char* start, const char* end);
#if USE(CF)
    WTF_EXPORT_STRING_API static RefPtr<AtomicStringImpl> add(CFStringRef);
#endif

    template<typename StringTableProvider>
    ALWAYS_INLINE static RefPtr<AtomicStringImpl> addWithStringTableProvider(StringTableProvider& stringTableProvider, StringImpl* string)
    {
        if (!string)
            return nullptr;
        return add(*stringTableProvider.atomicStringTable(), *string);
    }

#if !ASSERT_DISABLED
    WTF_EXPORT_STRING_API static bool isInAtomicStringTable(StringImpl*);
#endif

private:
    AtomicStringImpl() = delete;

    ALWAYS_INLINE static Ref<AtomicStringImpl> add(StringImpl& string)
    {
        if (string.isAtomic()) {
            ASSERT_WITH_MESSAGE(!string.length() || isInAtomicStringTable(&string), "The atomic string comes from an other thread!");
            return static_cast<AtomicStringImpl&>(string);
        }
        return addSlowCase(string);
    }

    ALWAYS_INLINE static Ref<AtomicStringImpl> add(AtomicStringTable& stringTable, StringImpl& string)
    {
        if (string.isAtomic()) {
            ASSERT_WITH_MESSAGE(!string.length() || isInAtomicStringTable(&string), "The atomic string comes from an other thread!");
            return static_cast<AtomicStringImpl&>(string);
        }
        return addSlowCase(stringTable, string);
    }

    WTF_EXPORT_STRING_API static Ref<AtomicStringImpl> addSlowCase(StringImpl&);
    WTF_EXPORT_STRING_API static Ref<AtomicStringImpl> addSlowCase(AtomicStringTable&, StringImpl&);

    WTF_EXPORT_STRING_API static RefPtr<AtomicStringImpl> lookUpSlowCase(StringImpl&);

    WTF_EXPORT_STRING_API static RefPtr<AtomicStringImpl> lookUpInternal(const LChar*, unsigned length);
    WTF_EXPORT_STRING_API static RefPtr<AtomicStringImpl> lookUpInternal(const UChar*, unsigned length);
};

#if !ASSERT_DISABLED
// AtomicStringImpls created from StaticStringImpl will ASSERT
// in the generic ValueCheck<T>::checkConsistency
// as they are not allocated by fastMalloc.
// We don't currently have any way to detect that case
// so we ignore the consistency check for all AtomicStringImpls*.
template<> struct
ValueCheck<AtomicStringImpl*> {
    static void checkConsistency(const AtomicStringImpl*) { }
};

template<> struct
ValueCheck<const AtomicStringImpl*> {
    static void checkConsistency(const AtomicStringImpl*) { }
};
#endif

}

using WTF::AtomicStringImpl;

#endif
