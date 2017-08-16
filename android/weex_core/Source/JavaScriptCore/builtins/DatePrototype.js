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

    function toDateTimeOptionsAnyAll(opts)
    {
        // ToDateTimeOptions(options, "any", "all")
        // http://www.ecma-international.org/ecma-402/2.0/#sec-InitializeDateTimeFormat

        var options;
        if (opts === @undefined)
            options = null;
        else if (opts === null)
            @throwTypeError("null is not an object");
        else
            options = @Object(opts);

        // Check original instead of descendant to reduce lookups up the prototype chain.
        var needsDefaults = !options || (
            options.weekday === @undefined &&
            options.year === @undefined &&
            options.month === @undefined &&
            options.day === @undefined &&
            options.hour === @undefined &&
            options.minute === @undefined &&
            options.second === @undefined
        );

        // Only create descendant if it will have own properties.
        if (needsDefaults) {
            options = @Object.@create(options);
            options.year = "numeric";
            options.month = "numeric";
            options.day = "numeric";
            options.hour = "numeric";
            options.minute = "numeric";
            options.second = "numeric";
        }

        // 9. Return options.
        return options;
    }

    // 13.3.1 Date.prototype.toLocaleString ([locales [, options ]]) (ECMA-402 2.0)
    // http://www.ecma-international.org/ecma-402/2.0/#sec-Date.prototype.toLocaleString

    var value = @thisTimeValue.@call(this);
    if (@isNaN(value))
        return "Invalid Date";

    var options = toDateTimeOptionsAnyAll(@argument(1));
    var locales = @argument(0);

    var dateFormat = new @DateTimeFormat(locales, options);
    return dateFormat.format(value);
}

function toLocaleDateString(/* locales, options */)
{
    "use strict";

    function toDateTimeOptionsDateDate(opts)
    {
        // ToDateTimeOptions(options, "date", "date")
        // http://www.ecma-international.org/ecma-402/2.0/#sec-InitializeDateTimeFormat

        var options;
        if (opts === @undefined)
            options = null;
        else if (opts === null)
            @throwTypeError("null is not an object");
        else
            options = @Object(opts);

        // Check original instead of descendant to reduce lookups up the prototype chain.
        var needsDefaults = !options || (
            options.weekday === @undefined &&
            options.year === @undefined &&
            options.month === @undefined &&
            options.day === @undefined
        );

        // Only create descendant if it will have own properties.
        if (needsDefaults) {
            options = @Object.@create(options);
            options.year = "numeric";
            options.month = "numeric";
            options.day = "numeric";
        }

        return options;
    }

    // 13.3.2 Date.prototype.toLocaleDateString ([locales [, options ]]) (ECMA-402 2.0)
    // http://www.ecma-international.org/ecma-402/2.0/#sec-Date.prototype.toLocaleDateString

    var value = @thisTimeValue.@call(this);
    if (@isNaN(value))
        return "Invalid Date";

    var options = toDateTimeOptionsDateDate(@argument(1));
    var locales = @argument(0);

    var dateFormat = new @DateTimeFormat(locales, options);
    return dateFormat.format(value);
}

function toLocaleTimeString(/* locales, options */)
{
    "use strict";

    function toDateTimeOptionsTimeTime(opts)
    {
        // ToDateTimeOptions(options, "time", "time")
        // http://www.ecma-international.org/ecma-402/2.0/#sec-InitializeDateTimeFormat

        var options;
        if (opts === @undefined)
            options = null;
        else if (opts === null)
            @throwTypeError("null is not an object");
        else
            options = @Object(opts);

        // Check original instead of descendant to reduce lookups up the prototype chain.
        var needsDefaults = !options || (
            options.hour === @undefined &&
            options.minute === @undefined &&
            options.second === @undefined
        );

        // Only create descendant if it will have own properties.
        if (needsDefaults) {
            options = @Object.@create(options);
            options.hour = "numeric";
            options.minute = "numeric";
            options.second = "numeric";
        }

        return options;
    }

    // 13.3.3 Date.prototype.toLocaleTimeString ([locales [, options ]]) (ECMA-402 2.0)
    // http://www.ecma-international.org/ecma-402/2.0/#sec-Date.prototype.toLocaleTimeString

    var value = @thisTimeValue.@call(this);
    if (@isNaN(value))
        return "Invalid Date";

    var options = toDateTimeOptionsTimeTime(@argument(1));
    var locales = @argument(0);

    var dateFormat = new @DateTimeFormat(locales, options);
    return dateFormat.format(value);
}
