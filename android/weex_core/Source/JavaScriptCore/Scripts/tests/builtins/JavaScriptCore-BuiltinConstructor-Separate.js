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

function of(/* items... */)
{
    "use strict";

    var length = arguments.length;
    // TODO: Need isConstructor(this) instead of typeof "function" check.
    var array = typeof this === 'function' ? new this(length) : new @Array(length);
    for (var k = 0; k < length; ++k)
        @putByValDirect(array, k, arguments[k]);
    array.length = length;
    return array;
}

function from(items /*, mapFn, thisArg */)
{
    "use strict";

    var thisObj = this;

    var mapFn = arguments.length > 1 ? arguments[1] : undefined;

    var thisArg;

    if (mapFn !== undefined) {
        if (typeof mapFn !== "function")
            throw new @TypeError("Array.from requires that the second argument, when provided, be a function");

        if (arguments.length > 2)
            thisArg = arguments[2];
    }

    if (items == null)
        throw new @TypeError("Array.from requires an array-like object - not null or undefined");

    var iteratorMethod = items[@symbolIterator];
    if (iteratorMethod != null) {
        if (typeof iteratorMethod !== "function")
            throw new @TypeError("Array.from requires that the property of the first argument, items[Symbol.iterator], when exists, be a function");

        // TODO: Need isConstructor(thisObj) instead of typeof "function" check.
        var result = (typeof thisObj === "function") ? @Object(new thisObj()) : [];

        var k = 0;
        var iterator = iteratorMethod.@call(items);

        // Since for-of loop once more looks up the @@iterator property of a given iterable,
        // it could be observable if the user defines a getter for @@iterator.
        // To avoid this situation, we define a wrapper object that @@iterator just returns a given iterator.
        var wrapper = {
            [@symbolIterator]() {
                return iterator;
            }
        };

        for (var value of wrapper) {
            if (mapFn)
                @putByValDirect(result, k, thisArg === undefined ? mapFn(value, k) : mapFn.@call(thisArg, value, k));
            else
                @putByValDirect(result, k, value);
            k += 1;
        }

        result.length = k;
        return result;
    }

    var arrayLike = @Object(items);
    var arrayLikeLength = @toLength(arrayLike.length);

    // TODO: Need isConstructor(thisObj) instead of typeof "function" check.
    var result = (typeof thisObj === "function") ? @Object(new thisObj(arrayLikeLength)) : new @Array(arrayLikeLength);

    var k = 0;
    while (k < arrayLikeLength) {
        var value = arrayLike[k];
        if (mapFn)
            @putByValDirect(result, k, thisArg === undefined ? mapFn(value, k) : mapFn.@call(thisArg, value, k));
        else
            @putByValDirect(result, k, value);
        k += 1;
    }

    result.length = arrayLikeLength;
    return result;
}
