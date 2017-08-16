/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

// https://tc39.github.io/ecma262/#sec-reflect.apply
function apply(target, thisArgument, argumentsList)
{
    "use strict";

    if (typeof target !== "function")
        @throwTypeError("Reflect.apply requires the first argument be a function");

    if (!@isObject(argumentsList))
        @throwTypeError("Reflect.apply requires the third argument be an object");

    return target.@apply(thisArgument, argumentsList);
}

// https://tc39.github.io/ecma262/#sec-reflect.deleteproperty
function deleteProperty(target, propertyKey)
{
    // Intentionally keep the code the sloppy mode to suppress the TypeError
    // raised by the delete operator under the strict mode.

    if (!@isObject(target))
        @throwTypeError("Reflect.deleteProperty requires the first argument be an object");

    return delete target[propertyKey];
}

// https://tc39.github.io/ecma262/#sec-reflect.has
function has(target, propertyKey)
{
    "use strict";

    if (!@isObject(target))
        @throwTypeError("Reflect.has requires the first argument be an object");

    return propertyKey in target;
}
