/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TypeCastsCF_h
#define TypeCastsCF_h

#include <CoreFoundation/CoreFoundation.h>
#include <wtf/Assertions.h>

namespace WTF {

template <typename> struct CFTypeTrait;

#define DECLARE_CF_TYPE_TRAIT(ClassName) \
template <> \
struct CFTypeTrait<ClassName##Ref> { \
    static inline CFTypeID typeID() { return ClassName##GetTypeID(); } \
};

DECLARE_CF_TYPE_TRAIT(CFArray);
DECLARE_CF_TYPE_TRAIT(CFBoolean);
DECLARE_CF_TYPE_TRAIT(CFData);
DECLARE_CF_TYPE_TRAIT(CFDictionary);
DECLARE_CF_TYPE_TRAIT(CFNumber);
DECLARE_CF_TYPE_TRAIT(CFString);

#undef DECLARE_CF_TYPE_TRAIT

template<typename T> T dynamic_cf_cast(CFTypeRef object)
{
    if (!object)
        return nullptr;

    if (CFGetTypeID(object) != CFTypeTrait<T>::typeID())
        return nullptr;

    return static_cast<T>(object);
}

template<typename T> T checked_cf_cast(CFTypeRef object)
{
    auto result = dynamic_cf_cast<T>(object);
    ASSERT_WITH_SECURITY_IMPLICATION(result);

    return result;
}

} // namespace WTF

using WTF::checked_cf_cast;
using WTF::dynamic_cf_cast;

#endif // TypeCastsCF_h
