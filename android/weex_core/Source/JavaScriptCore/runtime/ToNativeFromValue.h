/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#pragma once

#include "JSCJSValue.h"

namespace JSC {

template<typename Adaptor>
typename Adaptor::Type toNativeFromValue(JSValue value)
{
    if (value.isInt32())
        return Adaptor::toNativeFromInt32(value.asInt32());
    return Adaptor::toNativeFromDouble(value.asDouble());
}

template<typename Adaptor>
typename Adaptor::Type toNativeFromValue(ExecState* exec, JSValue value)
{
    if (value.isInt32())
        return Adaptor::toNativeFromInt32(value.asInt32());
    if (value.isNumber())
        return Adaptor::toNativeFromDouble(value.asDouble());
    return Adaptor::toNativeFromDouble(value.toNumber(exec));
}

template<typename Adaptor>
std::optional<typename Adaptor::Type> toNativeFromValueWithoutCoercion(JSValue value)
{
    if (!value.isNumber())
        return std::nullopt;
    if (value.isInt32())
        return Adaptor::toNativeFromInt32WithoutCoercion(value.asInt32());
    return Adaptor::toNativeFromDoubleWithoutCoercion(value.asDouble());
}

} // namespace JSC
