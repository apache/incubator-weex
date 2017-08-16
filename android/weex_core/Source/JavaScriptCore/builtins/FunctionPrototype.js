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

function call(thisArgument)
{
    "use strict";

    let argumentValues = [];
    // Start from 1 to ignore thisArgument
    for (let i = 1; i < arguments.length; i++)
        @putByValDirect(argumentValues, i-1, arguments[i]);

    return this.@apply(thisArgument, argumentValues);
}

function apply(thisValue, argumentValues)
{
    "use strict";

    return this.@apply(thisValue, argumentValues);
}

// FIXME: this should have a different name: https://bugs.webkit.org/show_bug.cgi?id=151363
function symbolHasInstance(value)
{
    "use strict";

    if (typeof this !== "function")
        return false;

    if (@isBoundFunction(this))
        return @hasInstanceBoundFunction(this, value);

    let target = this.prototype;
    return @instanceOf(value, target);
}

function bind(thisValue)
{
    "use strict";

    let target = this;
    if (typeof target !== "function")
        @throwTypeError("|this| is not a function inside Function.prototype.bind");

    let argumentCount = arguments.length;
    let boundArgs = null;
    let numBoundArgs = 0;
    if (argumentCount > 1) {
        numBoundArgs = argumentCount - 1;
        boundArgs = @newArrayWithSize(numBoundArgs);
        for (let i = 0; i < numBoundArgs; i++)
            @putByValDirect(boundArgs, i, arguments[i + 1]);
    }

    let length = 0;
    if (@hasOwnLengthProperty(target)) {
        let lengthValue = target.length;
        if (typeof lengthValue === "number") {
            lengthValue = lengthValue | 0;
            // Note that we only care about positive lengthValues, however, this comparision
            // against numBoundArgs suffices to prove we're not a negative number.
            if (lengthValue > numBoundArgs)
                length = lengthValue - numBoundArgs;
        }
    }

    let name = target.name;
    if (typeof name !== "string")
        name = "";

    return @makeBoundFunction(target, arguments[0], boundArgs, length, name);
}
