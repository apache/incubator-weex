/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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

// Note that the intrisic @typedArrayLength checks that the argument passed is a typed array
// and throws if it is not.


// Typed Arrays have their own species constructor function since they need
// to look up their default constructor, which is expensive. If we used the
// normal speciesConstructor helper we would need to look up the default
// constructor every time.
@globalPrivate
function typedArraySpeciesConstructor(value)
{
    "use strict";
    let constructor = value.constructor;
    if (constructor === @undefined)
        return @typedArrayGetOriginalConstructor(value);

    if (!@isObject(constructor))
        @throwTypeError("|this|.constructor is not an Object or undefined");

    constructor = constructor.@speciesSymbol;
    if (constructor == null)
        return @typedArrayGetOriginalConstructor(value);
    // The lack of an @isConstructor(constructor) check here is not observable because
    // the first thing we will do with the value is attempt to construct the result with it.
    // If any user of this function does not immediately construct the result they need to
    // verify that the result is a constructor.
    return constructor;
}

@globalPrivate
function typedArrayClampArgumentToStartOrEnd(value, length, undefinedValue)
{
    "use strict";

    if (value === @undefined)
        return undefinedValue;

    let int = @toInteger(value);
    if (int < 0) {
        int += length;
        return int < 0 ? 0 : int;
    }
    return int > length ? length : int;
}

function values()
{
    "use strict";
    @typedArrayLength(this);
    return new @createArrayIterator(this, "value", @arrayIteratorValueNext);
}

function keys()
{
    "use strict";
    @typedArrayLength(this);
    return new @createArrayIterator(this, "key", @arrayIteratorKeyNext);
}

function entries()
{
    "use strict";
    @typedArrayLength(this);
    return new @createArrayIterator(this, "key+value", @arrayIteratorKeyValueNext);
}

function every(callback /*, thisArg */)
{
    "use strict";
    var length = @typedArrayLength(this);
    var thisArg = @argument(1);

    if (typeof callback !== "function")
        @throwTypeError("TypedArray.prototype.every callback must be a function");

    for (var i = 0; i < length; i++) {
        if (!callback.@call(thisArg, this[i], i, this))
            return false;
    }

    return true;
}

function fill(value /* [, start [, end]] */)
{
    "use strict";

    let length = @typedArrayLength(this);

    let start = @argument(1);
    let end = @argument(2);

    start = @typedArrayClampArgumentToStartOrEnd(start, length, 0);
    end = @typedArrayClampArgumentToStartOrEnd(end, length, length);

    for (let i = start; i < end; i++)
        this[i] = value;
    return this;
}

function find(callback /* [, thisArg] */)
{
    "use strict";
    var length = @typedArrayLength(this);
    var thisArg = @argument(1);

    if (typeof callback !== "function")
        @throwTypeError("TypedArray.prototype.find callback must be a function");

    for (var i = 0; i < length; i++) {
        let elem = this[i];
        if (callback.@call(thisArg, elem, i, this))
            return elem;
    }
    return @undefined;
}

function findIndex(callback /* [, thisArg] */)
{
    "use strict";
    var length = @typedArrayLength(this);
    var thisArg = @argument(1);

    if (typeof callback !== "function")
        @throwTypeError("TypedArray.prototype.findIndex callback must be a function");

    for (var i = 0; i < length; i++) {
        if (callback.@call(thisArg, this[i], i, this))
            return i;
    }
    return -1;
}

function forEach(callback /* [, thisArg] */)
{
    "use strict";
    var length = @typedArrayLength(this);
    var thisArg = @argument(1);

    if (typeof callback !== "function")
        @throwTypeError("TypedArray.prototype.forEach callback must be a function");

    for (var i = 0; i < length; i++)
        callback.@call(thisArg, this[i], i, this);
}

function some(callback /* [, thisArg] */)
{
    // 22.2.3.24
    "use strict";
    var length = @typedArrayLength(this);
    var thisArg = @argument(1);

    if (typeof callback !== "function")
        @throwTypeError("TypedArray.prototype.some callback must be a function");

    for (var i = 0; i < length; i++) {
        if (callback.@call(thisArg, this[i], i, this))
            return true;
    }

    return false;
}

function sort(comparator)
{
    // 22.2.3.25
    "use strict";

    function min(a, b)
    {
        return a < b ? a : b;
    }

    function merge(dst, src, srcIndex, srcEnd, width, comparator)
    {
        var left = srcIndex;
        var leftEnd = min(left + width, srcEnd);
        var right = leftEnd;
        var rightEnd = min(right + width, srcEnd);

        for (var dstIndex = left; dstIndex < rightEnd; ++dstIndex) {
            if (right < rightEnd) {
                if (left >= leftEnd || comparator(src[right], src[left]) < 0) {
                    dst[dstIndex] = src[right++];
                    continue;
                }
            }

            dst[dstIndex] = src[left++];
        }
    }

    function mergeSort(array, valueCount, comparator)
    {
        var buffer = [ ];
        buffer.length = valueCount;

        var dst = buffer;
        var src = array;

        for (var width = 1; width < valueCount; width *= 2) {
            for (var srcIndex = 0; srcIndex < valueCount; srcIndex += 2 * width)
                merge(dst, src, srcIndex, valueCount, width, comparator);

            var tmp = src;
            src = dst;
            dst = tmp;
        }

        if (src != array) {
            for(var i = 0; i < valueCount; i++)
                array[i] = src[i];
        }
    }

    var length = @typedArrayLength(this);

    if (length < 2)
        return;

    if (typeof comparator == "function")
        mergeSort(this, length, comparator);
    else
        @typedArraySort(this);
    
    return this;
}

function subarray(begin, end)
{
    "use strict";

    if (!@isTypedArrayView(this))
        @throwTypeError("|this| should be a typed array view");

    let start = @toInteger(begin);
    let finish;
    if (end !== @undefined)
        finish = @toInteger(end);

    let constructor = @typedArraySpeciesConstructor(this);

    return @typedArraySubarrayCreate.@call(this, start, finish, constructor);
}

function reduce(callback /* [, initialValue] */)
{
    // 22.2.3.19
    "use strict";

    var length = @typedArrayLength(this);

    if (typeof callback !== "function")
        @throwTypeError("TypedArray.prototype.reduce callback must be a function");

    var argumentCount = @argumentCount();
    if (length === 0 && argumentCount < 2)
        @throwTypeError("TypedArray.prototype.reduce of empty array with no initial value");

    var accumulator, k = 0;
    if (argumentCount > 1)
        accumulator = @argument(1);
    else
        accumulator = this[k++];

    for (; k < length; k++)
        accumulator = callback.@call(@undefined, accumulator, this[k], k, this);

    return accumulator;
}

function reduceRight(callback /* [, initialValue] */)
{
    // 22.2.3.20
    "use strict";

    var length = @typedArrayLength(this);

    if (typeof callback !== "function")
        @throwTypeError("TypedArray.prototype.reduceRight callback must be a function");

    var argumentCount = @argumentCount();
    if (length === 0 && argumentCount < 2)
        @throwTypeError("TypedArray.prototype.reduceRight of empty array with no initial value");

    var accumulator, k = length - 1;
    if (argumentCount > 1)
        accumulator = @argument(1);
    else
        accumulator = this[k--];

    for (; k >= 0; k--)
        accumulator = callback.@call(@undefined, accumulator, this[k], k, this);

    return accumulator;
}

function map(callback /*, thisArg */)
{
    // 22.2.3.18
    "use strict";

    var length = @typedArrayLength(this);

    if (typeof callback !== "function")
        @throwTypeError("TypedArray.prototype.map callback must be a function");

    var thisArg = @argument(1);

    // Do species construction
    var constructor = this.constructor;
    var result;
    if (constructor === @undefined)
        result = new (@typedArrayGetOriginalConstructor(this))(length);
    else {
        var speciesConstructor = @Object(constructor).@speciesSymbol;
        if (speciesConstructor === null || speciesConstructor === @undefined)
            result = new (@typedArrayGetOriginalConstructor(this))(length);
        else {
            result = new speciesConstructor(length);
            // typedArrayLength throws if it doesn't get a view.
            @typedArrayLength(result);
        }
    }

    for (var i = 0; i < length; i++) {
        var mappedValue = callback.@call(thisArg, this[i], i, this);
        result[i] = mappedValue;
    }
    return result;
}

function filter(callback /*, thisArg */)
{
    "use strict";

    var length = @typedArrayLength(this);

    if (typeof callback !== "function")
        @throwTypeError("TypedArray.prototype.filter callback must be a function");

    var thisArg = @argument(1);
    var kept = [];

    for (var i = 0; i < length; i++) {
        var value = this[i];
        if (callback.@call(thisArg, value, i, this))
            kept.@push(value);
    }

    var constructor = this.constructor;
    var result;
    var resultLength = kept.length;
    if (constructor === @undefined)
        result = new (@typedArrayGetOriginalConstructor(this))(resultLength);
    else {
        var speciesConstructor = @Object(constructor).@speciesSymbol;
        if (speciesConstructor === null || speciesConstructor === @undefined)
            result = new (@typedArrayGetOriginalConstructor(this))(resultLength);
        else {
            result = new speciesConstructor(resultLength);
            // typedArrayLength throws if it doesn't get a view.
            @typedArrayLength(result);
        }
    }

    for (var i = 0; i < kept.length; i++)
        result[i] = kept[i];

    return result;
}

function toLocaleString()
{
    "use strict";

    var length = @typedArrayLength(this);

    if (length == 0)
        return "";

    var string = this[0].toLocaleString();
    for (var i = 1; i < length; i++)
        string += "," + this[i].toLocaleString();

    return string;
}
