/*
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

@globalPrivate
function advanceStringIndex(string, index, unicode)
{
    // This function implements AdvanceStringIndex described in ES6 21.2.5.2.3.
    "use strict";

    if (!unicode)
        return index + 1;

    if (index + 1 >= string.length)
        return index + 1;

    let first = string.@charCodeAt(index);
    if (first < 0xD800 || first > 0xDBFF)
        return index + 1;

    let second = string.@charCodeAt(index + 1);
    if (second < 0xDC00 || second > 0xDFFF)
        return index + 1;

    return index + 2;
}

@globalPrivate
function regExpExec(regexp, str)
{
    "use strict";

    let exec = regexp.exec;
    let builtinExec = @regExpBuiltinExec;
    if (exec !== builtinExec && typeof exec === "function") {
        let result = exec.@call(regexp, str);
        if (result !== null && !@isObject(result))
            @throwTypeError("The result of a RegExp exec must be null or an object");
        return result;
    }
    return builtinExec.@call(regexp, str);
}

@globalPrivate
function hasObservableSideEffectsForRegExpMatch(regexp) {
    // This is accessed by the RegExpExec internal function.
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

function match(strArg)
{
    "use strict";

    if (!@isObject(this))
        @throwTypeError("RegExp.prototype.@@match requires that |this| be an Object");

    let regexp = this;

    // Check for observable side effects and call the fast path if there aren't any.
    if (!@hasObservableSideEffectsForRegExpMatch(regexp))
        return @regExpMatchFast.@call(regexp, strArg);

    let str = @toString(strArg);

    if (!regexp.global)
        return @regExpExec(regexp, str);
    
    let unicode = regexp.unicode;
    regexp.lastIndex = 0;
    let resultList = [];

    // FIXME: It would be great to implement a solution similar to what we do in
    // RegExpObject::matchGlobal(). It's not clear if this is possible, since this loop has
    // effects. https://bugs.webkit.org/show_bug.cgi?id=158145
    const maximumReasonableMatchSize = 100000000;

    while (true) {
        let result = @regExpExec(regexp, str);
        
        if (result === null) {
            if (resultList.length === 0)
                return null;
            return resultList;
        }

        if (resultList.length > maximumReasonableMatchSize)
            @throwOutOfMemoryError();

        if (!@isObject(result))
            @throwTypeError("RegExp.prototype.@@match call to RegExp.exec didn't return null or an object");

        let resultString = @toString(result[0]);

        if (!resultString.length)
            regexp.lastIndex = @advanceStringIndex(str, regexp.lastIndex, unicode);

        resultList.@push(resultString);
    }
}

function replace(strArg, replace)
{
    "use strict";

    function getSubstitution(matched, str, position, captures, replacement)
    {
        "use strict";

        let matchLength = matched.length;
        let stringLength = str.length;
        let tailPos = position + matchLength;
        let m = captures.length;
        let replacementLength = replacement.length;
        let result = "";
        let lastStart = 0;

        for (let start = 0; start = replacement.indexOf("$", lastStart), start !== -1; lastStart = start) {
            if (start - lastStart > 0)
                result = result + replacement.substring(lastStart, start);
            start++;
            let ch = replacement.charAt(start);
            if (ch === "")
                result = result + "$";
            else {
                switch (ch)
                {
                case "$":
                    result = result + "$";
                    start++;
                    break;
                case "&":
                    result = result + matched;
                    start++;
                    break;
                case "`":
                    if (position > 0)
                        result = result + str.substring(0, position);
                    start++;
                    break;
                case "'":
                    if (tailPos < stringLength)
                        result = result + str.substring(tailPos);
                    start++;
                    break;
                default:
                    let chCode = ch.charCodeAt(0);
                    if (chCode >= 0x30 && chCode <= 0x39) {
                        start++;
                        let n = chCode - 0x30;
                        if (n > m)
                            break;
                        if (start < replacementLength) {
                            let nextChCode = replacement.charCodeAt(start);
                            if (nextChCode >= 0x30 && nextChCode <= 0x39) {
                                let nn = 10 * n + nextChCode - 0x30;
                                if (nn <= m) {
                                    n = nn;
                                    start++;
                                }
                            }
                        }

                        if (n == 0)
                            break;

                        if (captures[n] != @undefined)
                            result = result + captures[n];
                    } else
                        result = result + "$";
                    break;
                }
            }
        }

        return result + replacement.substring(lastStart);
    }

    if (!@isObject(this))
        @throwTypeError("RegExp.prototype.@@replace requires that |this| be an Object");

    let regexp = this;

    let str = @toString(strArg);
    let stringLength = str.length;
    let functionalReplace = typeof replace === 'function';

    if (!functionalReplace)
        replace = @toString(replace);

    let global = regexp.global;
    let unicode = false;

    if (global) {
        unicode = regexp.unicode;
        regexp.lastIndex = 0;
    }

    let resultList = [];
    let result;
    let done = false;
    while (!done) {
        result = @regExpExec(regexp, str);

        if (result === null)
            done = true;
        else {
            resultList.@push(result);
            if (!global)
                done = true;
            else {
                let matchStr = @toString(result[0]);

                if (!matchStr.length)
                    regexp.lastIndex = @advanceStringIndex(str, regexp.lastIndex, unicode);
            }
        }
    }

    let accumulatedResult = "";
    let nextSourcePosition = 0;
    let lastPosition = 0;

    for (result of resultList) {
        let nCaptures = result.length - 1;
        if (nCaptures < 0)
            nCaptures = 0;
        let matched = @toString(result[0]);
        let matchLength = matched.length;
        let position = result.index;
        position = (position > stringLength) ? stringLength : position;
        position = (position < 0) ? 0 : position;

        let captures = [];
        for (let n = 1; n <= nCaptures; n++) {
            let capN = result[n];
            if (capN !== @undefined)
                capN = @toString(capN);
            captures[n] = capN;
        }

        let replacement;

        if (functionalReplace) {
            let replacerArgs = [ matched ].concat(captures.slice(1));
            replacerArgs.@push(position);
            replacerArgs.@push(str);

            let replValue = replace.@apply(@undefined, replacerArgs);
            replacement = @toString(replValue);
        } else
            replacement = getSubstitution(matched, str, position, captures, replace);

        if (position >= nextSourcePosition && position >= lastPosition) {
            accumulatedResult = accumulatedResult + str.substring(nextSourcePosition, position) + replacement;
            nextSourcePosition = position + matchLength;
            lastPosition = position;
        }
    }

    if (nextSourcePosition >= stringLength)
        return  accumulatedResult;

    return accumulatedResult + str.substring(nextSourcePosition);
}

// 21.2.5.9 RegExp.prototype[@@search] (string)
function search(strArg)
{
    "use strict";

    let regexp = this;

    // Check for observable side effects and call the fast path if there aren't any.
    if (@isRegExpObject(regexp) && @tryGetById(regexp, "exec") === @regExpBuiltinExec)
        return @regExpSearchFast.@call(regexp, strArg);

    // 1. Let rx be the this value.
    // 2. If Type(rx) is not Object, throw a TypeError exception.
    if (!@isObject(this))
        @throwTypeError("RegExp.prototype.@@search requires that |this| be an Object");

    // 3. Let S be ? ToString(string).
    let str = @toString(strArg)

    // 4. Let previousLastIndex be ? Get(rx, "lastIndex").
    let previousLastIndex = regexp.lastIndex;
    // 5. Perform ? Set(rx, "lastIndex", 0, true).
    regexp.lastIndex = 0;
    // 6. Let result be ? RegExpExec(rx, S).
    let result = @regExpExec(regexp, str);
    // 7. Perform ? Set(rx, "lastIndex", previousLastIndex, true).
    regexp.lastIndex = previousLastIndex;
    // 8. If result is null, return -1.
    if (result === null)
        return -1;
    // 9. Return ? Get(result, "index").
    return result.index;
}

@globalPrivate
function hasObservableSideEffectsForRegExpSplit(regexp) {
    // This is accessed by the RegExpExec internal function.
    let regexpExec = @tryGetById(regexp, "exec");
    if (regexpExec !== @regExpBuiltinExec)
        return true;
    
    // This is accessed by step 5 below.
    let regexpFlags = @tryGetById(regexp, "flags");
    if (regexpFlags !== @regExpProtoFlagsGetter)
        return true;
    
    // These are accessed by the builtin flags getter.
    let regexpGlobal = @tryGetById(regexp, "global");
    if (regexpGlobal !== @regExpProtoGlobalGetter)
        return true;
    let regexpIgnoreCase = @tryGetById(regexp, "ignoreCase");
    if (regexpIgnoreCase !== @regExpProtoIgnoreCaseGetter)
        return true;
    let regexpMultiline = @tryGetById(regexp, "multiline");
    if (regexpMultiline !== @regExpProtoMultilineGetter)
        return true;
    let regexpSticky = @tryGetById(regexp, "sticky");
    if (regexpSticky !== @regExpProtoStickyGetter)
        return true;
    let regexpUnicode = @tryGetById(regexp, "unicode");
    if (regexpUnicode !== @regExpProtoUnicodeGetter)
        return true;
    
    // This is accessed by the RegExp species constructor.
    let regexpSource = @tryGetById(regexp, "source");
    if (regexpSource !== @regExpProtoSourceGetter)
        return true;
    
    return !@isRegExpObject(regexp);
}

// ES 21.2.5.11 RegExp.prototype[@@split](string, limit)
function split(string, limit)
{
    "use strict";

    // 1. Let rx be the this value.
    // 2. If Type(rx) is not Object, throw a TypeError exception.
    if (!@isObject(this))
        @throwTypeError("RegExp.prototype.@@split requires that |this| be an Object");
    let regexp = this;

    // 3. Let S be ? ToString(string).
    let str = @toString(string);

    // 4. Let C be ? SpeciesConstructor(rx, %RegExp%).
    let speciesConstructor = @speciesConstructor(regexp, @RegExp);

    if (speciesConstructor === @RegExp && !@hasObservableSideEffectsForRegExpSplit(regexp))
        return @regExpSplitFast.@call(regexp, str, limit);

    // 5. Let flags be ? ToString(? Get(rx, "flags")).
    let flags = @toString(regexp.flags);

    // 6. If flags contains "u", let unicodeMatching be true.
    // 7. Else, let unicodeMatching be false.
    let unicodeMatching = @stringIncludesInternal.@call(flags, "u");
    // 8. If flags contains "y", let newFlags be flags.
    // 9. Else, let newFlags be the string that is the concatenation of flags and "y".
    let newFlags = @stringIncludesInternal.@call(flags, "y") ? flags : flags + "y";

    // 10. Let splitter be ? Construct(C, « rx, newFlags »).
    let splitter = new speciesConstructor(regexp, newFlags);

    // We need to check again for RegExp subclasses that will fail the speciesConstructor test
    // but can still use the fast path after we invoke the constructor above.
    if (!@hasObservableSideEffectsForRegExpSplit(splitter))
        return @regExpSplitFast.@call(splitter, str, limit);

    // 11. Let A be ArrayCreate(0).
    // 12. Let lengthA be 0.
    let result = [];

    // 13. If limit is undefined, let lim be 2^32-1; else let lim be ? ToUint32(limit).
    limit = (limit === @undefined) ? 0xffffffff : limit >>> 0;

    // 16. If lim = 0, return A.
    if (!limit)
        return result;

    // 14. [Defered from above] Let size be the number of elements in S.
    let size = str.length;

    // 17. If size = 0, then
    if (!size) {
        // a. Let z be ? RegExpExec(splitter, S).
        let z = @regExpExec(splitter, str);
        // b. If z is not null, return A.
        if (z != null)
            return result;
        // c. Perform ! CreateDataProperty(A, "0", S).
        @putByValDirect(result, 0, str);
        // d. Return A.
        return result;
    }

    // 15. [Defered from above] Let p be 0.
    let position = 0;
    // 18. Let q be p.
    let matchPosition = 0;

    // 19. Repeat, while q < size
    while (matchPosition < size) {
        // a. Perform ? Set(splitter, "lastIndex", q, true).
        splitter.lastIndex = matchPosition;
        // b. Let z be ? RegExpExec(splitter, S).
        let matches = @regExpExec(splitter, str);
        // c. If z is null, let q be AdvanceStringIndex(S, q, unicodeMatching).
        if (matches === null)
            matchPosition = @advanceStringIndex(str, matchPosition, unicodeMatching);
        // d. Else z is not null,
        else {
            // i. Let e be ? ToLength(? Get(splitter, "lastIndex")).
            let endPosition = @toLength(splitter.lastIndex);
            // ii. Let e be min(e, size).
            endPosition = (endPosition <= size) ? endPosition : size;
            // iii. If e = p, let q be AdvanceStringIndex(S, q, unicodeMatching).
            if (endPosition === position)
                matchPosition = @advanceStringIndex(str, matchPosition, unicodeMatching);
            // iv. Else e != p,
            else {
                // 1. Let T be a String value equal to the substring of S consisting of the elements at indices p (inclusive) through q (exclusive).
                let subStr = @stringSubstrInternal.@call(str, position, matchPosition - position);
                // 2. Perform ! CreateDataProperty(A, ! ToString(lengthA), T).
                // 3. Let lengthA be lengthA + 1.
                @putByValDirect(result, result.length, subStr);
                // 4. If lengthA = lim, return A.
                if (result.length == limit)
                    return result;

                // 5. Let p be e.
                position = endPosition;
                // 6. Let numberOfCaptures be ? ToLength(? Get(z, "length")).
                // 7. Let numberOfCaptures be max(numberOfCaptures-1, 0).
                let numberOfCaptures = matches.length > 1 ? matches.length - 1 : 0;

                // 8. Let i be 1.
                let i = 1;
                // 9. Repeat, while i <= numberOfCaptures,
                while (i <= numberOfCaptures) {
                    // a. Let nextCapture be ? Get(z, ! ToString(i)).
                    let nextCapture = matches[i];
                    // b. Perform ! CreateDataProperty(A, ! ToString(lengthA), nextCapture).
                    // d. Let lengthA be lengthA + 1.
                    @putByValDirect(result, result.length, nextCapture);
                    // e. If lengthA = lim, return A.
                    if (result.length == limit)
                        return result;
                    // c. Let i be i + 1.
                    i++;
                }
                // 10. Let q be p.
                matchPosition = position;
            }
        }
    }
    // 20. Let T be a String value equal to the substring of S consisting of the elements at indices p (inclusive) through size (exclusive).
    let remainingStr = @stringSubstrInternal.@call(str, position, size);
    // 21. Perform ! CreateDataProperty(A, ! ToString(lengthA), T).
    @putByValDirect(result, result.length, remainingStr);
    // 22. Return A.
    return result;
}

// ES 21.2.5.13 RegExp.prototype.test(string)
@intrinsic=RegExpTestIntrinsic
function test(strArg)
{
    "use strict";

    let regexp = this;

    // Check for observable side effects and call the fast path if there aren't any.
    if (@isRegExpObject(regexp) && @tryGetById(regexp, "exec") === @regExpBuiltinExec)
        return @regExpTestFast.@call(regexp, strArg);

    // 1. Let R be the this value.
    // 2. If Type(R) is not Object, throw a TypeError exception.
    if (!@isObject(regexp))
        @throwTypeError("RegExp.prototype.test requires that |this| be an Object");

    // 3. Let string be ? ToString(S).
    let str = @toString(strArg);

    // 4. Let match be ? RegExpExec(R, string).
    let match = @regExpExec(regexp, str);

    // 5. If match is not null, return true; else return false.
    if (match !== null)
        return true;
    return false;
}
