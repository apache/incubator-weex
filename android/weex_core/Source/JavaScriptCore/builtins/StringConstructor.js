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

function raw(template)
{
    "use strict";

    if (template === null || template === @undefined)
        @throwTypeError("String.raw requires template not be null or undefined");
    var cookedSegments = @Object(template);

    var rawValue = cookedSegments.raw;
    if (rawValue === null || rawValue === @undefined)
        @throwTypeError("String.raw requires template.raw not be null or undefined");
    var rawSegments = @Object(rawValue);

    var numberOfSubstitutions = arguments.length - 1;

    var segmentCount = @toLength(rawSegments.length);

    if (segmentCount <= 0)
        return '';

    var stringElements = '';
    for (var i = 0; ; ++i) {
        var segment = @toString(rawSegments[i]);
        stringElements += segment;

        if ((i + 1) === segmentCount)
            return stringElements;

        if (i < numberOfSubstitutions) {
            var substitutionIndexInArguments = i + 1;
            var next = @toString(arguments[substitutionIndexInArguments]);
            stringElements += next;
        }
    }
}
