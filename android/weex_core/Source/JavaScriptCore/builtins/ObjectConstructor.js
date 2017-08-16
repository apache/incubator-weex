/*
 * Copyright (C) 2016 Oleksandr Skachkov <gskachkov@gmail.com>.
 * Copyright (C) 2015 Jordan Harband. All rights reserved.
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

@globalPrivate
function enumerableOwnProperties(object, kind)
{
    "use strict";

    const obj = @Object(object);
    const ownKeys = @Reflect.@ownKeys(obj);
    const properties = [];
    for (let i = 0, keysLength = ownKeys.length; i < keysLength; ++i) {
        let nextKey = ownKeys[i];
        if (typeof nextKey === 'string') {
            let descriptor = @Reflect.@getOwnPropertyDescriptor(obj, nextKey);
            if (descriptor !== @undefined && descriptor.enumerable) {
                if (kind === @iterationKindValue)
                    properties.@push(obj[nextKey]);
                else if (kind === @iterationKindKeyValue)
                    properties.@push([nextKey, obj[nextKey]]);
            }
        }
    }
    
    return properties;
}

function values(object)
{
    "use strict";
    
    if (object == null)
        @throwTypeError("Object.values requires that input parameter not be null or undefined");

    return @enumerableOwnProperties(object, @iterationKindValue);
}

function entries(object)
{
    "use strict";
    
    if (object == null)
        @throwTypeError("Object.entries requires that input parameter not be null or undefined");
    
    return @enumerableOwnProperties(object, @iterationKindKeyValue);
}

function assign(target/*[*/, /*...*/sources/*] */)
{
    "use strict";

    if (target == null)
        @throwTypeError("Object.assign requires that input parameter not be null or undefined");

    let objTarget = @Object(target);
    for (let s = 1, argumentsLength = arguments.length; s < argumentsLength; ++s) {
        let nextSource = arguments[s];
        if (nextSource != null) {
            let from = @Object(nextSource);
            let keys = @Reflect.@ownKeys(from);
            for (let i = 0, keysLength = keys.length; i < keysLength; ++i) {
                let nextKey = keys[i];
                let descriptor = @Reflect.@getOwnPropertyDescriptor(from, nextKey);
                if (descriptor !== @undefined && descriptor.enumerable)
                    objTarget[nextKey] = from[nextKey];
            }
        }
    }
    return objTarget;
}
