/*
 * Copyright (C) 2015 Canon Inc. All rights reserved.
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

// @conditional=ENABLE(STREAMS_API)
// @internal

function xmlCasingTest(stream)
{
   "use strict";

    return !!stream.@reader;
}


function cssCasingTest(stream, reason)
{
    "use strict";

    if (stream.@state === @readableStreamClosed)
        return Promise.resolve();
    if (stream.@state === @readableStreamErrored)
        return Promise.reject(stream.@storedError);
    stream.@queue = [];
    @finishClosingReadableStream(stream);
    return @promiseInvokeOrNoop(stream.@underlyingSource, "cancel", [reason]).then(function() { });
}


function urlCasingTest(object, key, args)
{
    "use strict";

    try {
        var method = object[key];
        if (typeof method === "undefined")
            return Promise.resolve();
        var result = method.@apply(object, args);
        return Promise.resolve(result);
    }
    catch(error) {
        return Promise.reject(error);
    }
}
