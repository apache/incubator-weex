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

#include "ExceptionHelpers.h"
#include "JSString.h"
#include <wtf/Vector.h>

namespace JSC {

// FIXME: Should move the last few callers over from this to WTF::StringBuilder.
class JSStringBuilder {
public:
    JSStringBuilder()
        : m_okay(true)
        , m_is8Bit(true)
    {
    }

    void append(LChar character)
    {
        if (m_is8Bit) {
            m_okay &= buffer8.tryAppend(&character, 1);
            return;
        }
        UChar upconvertedCharacter = character;
        m_okay &= buffer16.tryAppend(&upconvertedCharacter, 1);
    }

    void append(UChar character)
    {
        if (m_is8Bit) {
            if (character < 0x100) {
                LChar narrowedCharacter = character;
                m_okay &= buffer8.tryAppend(&narrowedCharacter, 1);
                return;
            }
            upConvert();
        }
        m_okay &= buffer16.tryAppend(&character, 1);
    }

    void append(const char* str)
    {
        append(reinterpret_cast<const LChar*>(str), strlen(str));
    }

    JSValue build(ExecState* exec)
    {
        VM& vm = exec->vm();
        auto scope = DECLARE_THROW_SCOPE(vm);
        if (!m_okay)
            return throwOutOfMemoryError(exec, scope);
        if (m_is8Bit) {
            buffer8.shrinkToFit();
            if (!buffer8.data())
                return throwOutOfMemoryError(exec, scope);
            return jsString(exec, String::adopt(WTFMove(buffer8)));
        }
        buffer16.shrinkToFit();
        if (!buffer16.data())
            return throwOutOfMemoryError(exec, scope);
        return jsString(exec, String::adopt(WTFMove(buffer16)));
    }

private:
    void append(const LChar* characters, size_t length)
    {
        if (m_is8Bit) {
            m_okay &= buffer8.tryAppend(characters, length);
            return;
        }
        // FIXME: There must be a more efficient way of doing this.
        m_okay &= buffer16.tryReserveCapacity(buffer16.size() + length);
        for (size_t i = 0; i < length; i++) {
            UChar upconvertedCharacter = characters[i];
            m_okay &= buffer16.tryAppend(&upconvertedCharacter, 1);
        }
    }

    void upConvert()
    {
        ASSERT(m_is8Bit);
        size_t len = buffer8.size();

        for (size_t i = 0; i < len; i++)
            buffer16.append(buffer8[i]);

        buffer8.clear();
        m_is8Bit = false;
    }

    Vector<LChar, 64, UnsafeVectorOverflow> buffer8;
    Vector<UChar, 64, UnsafeVectorOverflow> buffer16;
    bool m_okay;
    bool m_is8Bit;
};

template<typename StringType>
inline JSValue jsMakeNontrivialString(ExecState* exec, StringType&& string)
{
    return jsNontrivialString(exec, std::forward<StringType>(string));
}

template<typename StringType, typename... StringTypes>
inline JSValue jsMakeNontrivialString(ExecState* exec, const StringType& string, const StringTypes&... strings)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    String result = tryMakeString(string, strings...);
    if (UNLIKELY(!result || !JSString::isValidLength(result.length())))
        return throwOutOfMemoryError(exec, scope);
    return jsNontrivialString(exec, WTFMove(result));
}

} // namespace JSC
