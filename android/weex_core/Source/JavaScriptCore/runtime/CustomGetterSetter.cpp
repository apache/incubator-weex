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

#include "config.h"
#include "CustomGetterSetter.h"

#include "JSCInlines.h"
#include <wtf/Assertions.h>

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(CustomGetterSetter);

const ClassInfo CustomGetterSetter::s_info = { "CustomGetterSetter", 0, 0, CREATE_METHOD_TABLE(CustomGetterSetter) };

bool callCustomSetter(ExecState* exec, CustomGetterSetter::CustomSetter setter, bool isAccessor, JSValue thisValue, JSValue value)
{
    ASSERT(setter);
    bool result = setter(exec, JSValue::encode(thisValue), JSValue::encode(value));
    // Always return true if there is a setter and it is observed as an accessor to users.
    if (isAccessor)
        return true;
    return result;
}

bool callCustomSetter(ExecState* exec, JSValue customGetterSetter, bool isAccessor, JSObject* base, JSValue thisValue, JSValue value)
{
    CustomGetterSetter::CustomSetter setter = jsCast<CustomGetterSetter*>(customGetterSetter)->setter();
    // Return false since there is no setter.
    if (!setter)
        return false;
    // FIXME: Remove this differences in custom values and custom accessors.
    // https://bugs.webkit.org/show_bug.cgi?id=158014
    if (!isAccessor)
        thisValue = base;
    return callCustomSetter(exec, setter, isAccessor, thisValue, value);
}

} // namespace JSC
