/*
 * Copyright (C) 2015 Andy VanWagoner <thetalecrafter@gmail.com>.
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

// @conditional=ENABLE(INTL)

function toLocaleString(/* locales, options */)
{
    "use strict";

    // 13.2.1 Number.prototype.toLocaleString ([locales [, options ]]) (ECMA-402 2.0)
    // http://ecma-international.org/publications/standards/Ecma-402.htm

    // 1. Let x be thisNumberValue(this value).
    // 2. ReturnIfAbrupt(x).
    var number = @thisNumberValue.@call(this);

    // 3. Let numberFormat be Construct(%NumberFormat%, «locales, options»).
    // 4. ReturnIfAbrupt(numberFormat).
    var numberFormat = new @NumberFormat(@argument(0), @argument(1));

    // 5. Return FormatNumber(numberFormat, x).
    return numberFormat.format(number);
}
