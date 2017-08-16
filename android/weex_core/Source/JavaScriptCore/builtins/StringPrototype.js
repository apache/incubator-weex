/*
 * Copyright (C) 2015 Andy VanWagoner <thetalecrafter@gmail.com>.
 * Copyright (C) 2016 Yusuke Suzuki <utatane.tea@gmail.com>
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

function match(regexp)
{
    "use strict";

    if (this == null)
        @throwTypeError("String.prototype.match requires that |this| not be null or undefined");

    if (regexp != null) {
        var matcher = regexp.@matchSymbol;
        if (matcher != @undefined)
            return matcher.@call(regexp, this);
    }

    let thisString = @toString(this);
    let createdRegExp = @regExpCreate(regexp, @undefined);
    return createdRegExp.@matchSymbol(thisString);
}

@globalPrivate
function repeatSlowPath(string, count)
{
    "use strict";

    // Return an empty string.
    if (count === 0 || string.length === 0)
        return "";

    // Return the original string.
    if (count === 1)
        return string;

    if (string.length * count > @MAX_STRING_LENGTH)
        @throwOutOfMemoryError();

    // Bit operation onto |count| is safe because |count| should be within Int32 range,
    // Repeat log N times to generate the repeated string rope.
    var result = "";
    var operand = string;
    while (true) {
        if (count & 1)
            result += operand;
        count >>= 1;
        if (!count)
            return result;
        operand += operand;
    }
}

@globalPrivate
function repeatCharactersSlowPath(string, count)
{
    "use strict";
    var repeatCount = (count / string.length) | 0;
    var remainingCharacters = count - repeatCount * string.length;
    var result = "";
    var operand = string;
    // Bit operation onto |repeatCount| is safe because |repeatCount| should be within Int32 range,
    // Repeat log N times to generate the repeated string rope.
    while (true) {
        if (repeatCount & 1)
            result += operand;
        repeatCount >>= 1;
        if (!repeatCount)
            break;
        operand += operand;
    }
    if (remainingCharacters)
        result += @stringSubstrInternal.@call(string, 0, remainingCharacters);
    return result;
}


function repeat(count)
{
    "use strict";

    if (this == null)
        @throwTypeError("String.prototype.repeat requires that |this| not be null or undefined");

    var string = @toString(this);
    count = @toInteger(count);

    if (count < 0 || count === @Infinity)
        @throwRangeError("String.prototype.repeat argument must be greater than or equal to 0 and not be Infinity");

    if (string.length === 1)
        return @repeatCharacter(string, count);

    return @repeatSlowPath(string, count);
}

function padStart(maxLength/*, fillString*/)
{
    "use strict";

    if (this == null)
        @throwTypeError("String.prototype.padStart requires that |this| not be null or undefined");

    var string = @toString(this);
    maxLength = @toLength(maxLength);

    var stringLength = string.length;
    if (maxLength <= stringLength)
        return string;

    var filler;
    var fillString = @argument(1);
    if (fillString === @undefined)
        filler = " ";
    else {
        filler = @toString(fillString);
        if (filler === "")
            return string;
    }

    if (maxLength > @MAX_STRING_LENGTH)
        @throwOutOfMemoryError();

    var fillLength = maxLength - stringLength;
    var truncatedStringFiller;

    if (filler.length === 1)
        truncatedStringFiller = @repeatCharacter(filler, fillLength);
    else
        truncatedStringFiller = @repeatCharactersSlowPath(filler, fillLength);
    return truncatedStringFiller + string;
}

function padEnd(maxLength/*, fillString*/)
{
    "use strict";

    if (this == null)
        @throwTypeError("String.prototype.padEnd requires that |this| not be null or undefined");

    var string = @toString(this);
    maxLength = @toLength(maxLength);

    var stringLength = string.length;
    if (maxLength <= stringLength)
        return string;

    var filler;
    var fillString = @argument(1);
    if (fillString === @undefined)
        filler = " ";
    else {
        filler = @toString(fillString);
        if (filler === "")
            return string;
    }

    if (maxLength > @MAX_STRING_LENGTH)
        @throwOutOfMemoryError();

    var fillLength = maxLength - stringLength;
    var truncatedStringFiller;

    if (filler.length === 1)
        truncatedStringFiller = @repeatCharacter(filler, fillLength);
    else
        truncatedStringFiller = @repeatCharactersSlowPath(filler, fillLength);
    return string + truncatedStringFiller;
}

@globalPrivate
function hasObservableSideEffectsForStringReplace(regexp, replacer) {
    if (replacer !== @regExpPrototypeSymbolReplace)
        return true;
    
    let regexpExec = @tryGetById(regexp, "exec");
    if (regexpExec !== @regExpBuiltinExec)
        return true;

    let regexpGlobal = @tryGetById(regexp, "global");
    if (regexpGlobal !== @regExpProtoGlobalGetter)
        return true;

    let regexpUnicode = @tryGetById(regexp, "unicode");
    if (regexpUnicode !== @regExpProtoUnicodeGetter)
        return true;

    return !@isRegExpObject(regexp);
}

@intrinsic=StringPrototypeReplaceIntrinsic
function replace(search, replace)
{
    "use strict";

    if (this == null)
        @throwTypeError("String.prototype.replace requires that |this| not be null or undefined");

    if (search != null) {
        let replacer = search.@replaceSymbol;
        if (replacer !== @undefined) {
            if (!@hasObservableSideEffectsForStringReplace(search, replacer))
                return @toString(this).@replaceUsingRegExp(search, replace);
            return replacer.@call(search, this, replace);
        }
    }

    let thisString = @toString(this);
    let searchString = @toString(search);
    return thisString.@replaceUsingStringSearch(searchString, replace);
}
    
function localeCompare(that/*, locales, options */)
{
    "use strict";

    // 13.1.1 String.prototype.localeCompare (that [, locales [, options ]]) (ECMA-402 2.0)
    // http://ecma-international.org/publications/standards/Ecma-402.htm

    // 1. Let O be RequireObjectCoercible(this value).
    if (this == null)
        @throwTypeError("String.prototype.localeCompare requires that |this| not be null or undefined");

    // 2. Let S be ToString(O).
    // 3. ReturnIfAbrupt(S).
    var thisString = @toString(this);

    // 4. Let That be ToString(that).
    // 5. ReturnIfAbrupt(That).
    var thatString = @toString(that);

    // Avoid creating a collator for defaults.
    var locales = @argument(1);
    var options = @argument(2);
    if (locales === @undefined && options === @undefined)
        return @Collator.prototype.compare(thisString, thatString);

    // 6. Let collator be Construct(%Collator%, «locales, options»).
    // 7. ReturnIfAbrupt(collator).
    var collator = new @Collator(locales, options);

    // 8. Return CompareStrings(collator, S, That).
    return collator.compare(thisString, thatString);
}

function search(regexp)
{
    "use strict";

    if (this == null)
        @throwTypeError("String.prototype.search requires that |this| not be null or undefined");

    if (regexp != null) {
        var searcher = regexp.@searchSymbol;
        if (searcher != @undefined)
            return searcher.@call(regexp, this);
    }

    var thisString = @toString(this);
    var createdRegExp = @regExpCreate(regexp, @undefined);
    return createdRegExp.@searchSymbol(thisString);
}

function split(separator, limit)
{
    "use strict";
    
    if (this == null)
        @throwTypeError("String.prototype.split requires that |this| not be null or undefined");
    
    if (separator != null) {
        var splitter = separator.@splitSymbol;
        if (splitter != @undefined)
            return splitter.@call(separator, this, limit);
    }
    
    return @stringSplitFast.@call(this, separator, limit);
}
