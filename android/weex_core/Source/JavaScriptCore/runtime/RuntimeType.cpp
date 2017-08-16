/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
 * Copyright (C) Saam Barati <saambarati1@gmail.com>. All rights reserved.
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


#include "config.h"
#include "RuntimeType.h"

#include "JSCInlines.h"

namespace JSC {

RuntimeType runtimeTypeForValue(JSValue value)
{
    if (UNLIKELY(!value))
        return TypeNothing;

    if (value.isUndefined())
        return TypeUndefined;
    if (value.isNull())
        return TypeNull;
    if (value.isAnyInt())
        return TypeAnyInt;
    if (value.isNumber())
        return TypeNumber;
    if (value.isString())
        return TypeString;
    if (value.isBoolean())
        return TypeBoolean;
    if (value.isObject())
        return TypeObject;
    if (value.isFunction())
        return TypeFunction;
    if (value.isSymbol())
        return TypeSymbol;

    return TypeNothing;
}

String runtimeTypeAsString(RuntimeType type)
{
    if (type == TypeUndefined)
        return ASCIILiteral("Undefined");
    if (type == TypeNull)
        return ASCIILiteral("Null");
    if (type == TypeAnyInt)
        return ASCIILiteral("Integer");
    if (type == TypeNumber)
        return ASCIILiteral("Number");
    if (type == TypeString)
        return ASCIILiteral("String");
    if (type == TypeObject)
        return ASCIILiteral("Object");
    if (type == TypeBoolean)
        return ASCIILiteral("Boolean");
    if (type == TypeFunction)
        return ASCIILiteral("Function");
    if (type == TypeNothing)
        return ASCIILiteral("(Nothing)");

    RELEASE_ASSERT_NOT_REACHED();
    return emptyString();
}

} // namespace JSC
