/*
 * Copyright (C) 2015 Yusuke Suzuki <utatane.tea@gmail.com>.
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

function next()
{
    "use strict";

    if (this == null)
        @throwTypeError("%StringIteratorPrototype%.next requires that |this| not be null or undefined");

    var position = this.@stringIteratorNextIndex;
    if (position === @undefined)
        @throwTypeError("%StringIteratorPrototype%.next requires that |this| be a String Iterator instance");

    var done = true;
    var value = @undefined;

    var string = this.@iteratedString;
    if (string !== @undefined) {
        var length = string.length >>> 0;
        if (position >= length) {
            this.@iteratedString = @undefined;
        } else {
            done = false;

            var first = string.@charCodeAt(position);
            if (first < 0xD800 || first > 0xDBFF || position + 1 === length)
                value = string[position];
            else {
                var second = string.@charCodeAt(position + 1);
                if (second < 0xDC00 || second > 0xDFFF)
                    value = string[position];
                else
                    value = string[position] + string[position + 1];
            }

            this.@stringIteratorNextIndex = position + value.length;
        }
    }

    return {done, value};
}
