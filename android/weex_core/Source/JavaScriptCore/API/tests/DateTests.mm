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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "DateTests.h"
#import <Foundation/Foundation.h>

#if JSC_OBJC_API_ENABLED

extern "C" void checkResult(NSString *description, bool passed);

@interface DateTests : NSObject
+ (void) NSDateToJSDateTest;
+ (void) JSDateToNSDateTest;
+ (void) roundTripThroughJSDateTest;
+ (void) roundTripThroughObjCDateTest;
@end

static unsigned unitFlags = NSCalendarUnitSecond | NSCalendarUnitMinute | NSCalendarUnitHour | NSCalendarUnitDay | NSCalendarUnitMonth | NSCalendarUnitYear;

@implementation DateTests
+ (void) NSDateToJSDateTest
{
    JSContext *context = [[JSContext alloc] init];
    NSDate *now = [NSDate dateWithTimeIntervalSinceNow:0];
    NSDateComponents *components = [[NSCalendar currentCalendar] components:unitFlags fromDate:now];
    JSValue *jsNow = [JSValue valueWithObject:now inContext:context];
    int year = [[jsNow invokeMethod:@"getFullYear" withArguments:@[]] toInt32];
    // Months are 0-indexed for JavaScript Dates.
    int month = [[jsNow invokeMethod:@"getMonth" withArguments:@[]] toInt32] + 1;
    int day = [[jsNow invokeMethod:@"getDate" withArguments:@[]] toInt32];
    int hour = [[jsNow invokeMethod:@"getHours" withArguments:@[]] toInt32];
    int minute = [[jsNow invokeMethod:@"getMinutes" withArguments:@[]] toInt32];
    int second = [[jsNow invokeMethod:@"getSeconds" withArguments:@[]] toInt32];

    checkResult(@"NSDate to JS Date", year == [components year]
        && month == [components month]
        && day == [components day]
        && hour == [components hour]
        && minute == [components minute]
        && second == [components second]);
}

+ (void) JSDateToNSDateTest
{
    JSContext *context = [[JSContext alloc] init];
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
    [formatter setDateFormat:@"MMMM dd',' yyyy hh:mm:ss"];
    NSDate *februaryFourth2014 = [formatter dateFromString:@"February 4, 2014 11:40:03"];
    NSDateComponents *components = [[NSCalendar currentCalendar] components:unitFlags fromDate:februaryFourth2014];
    // Months are 0-indexed for JavaScript Dates.
    JSValue *jsDate = [context[@"Date"] constructWithArguments:@[@2014, @1, @4, @11, @40, @3]];
    
    int year = [[jsDate invokeMethod:@"getFullYear" withArguments:@[]] toInt32];
    int month = [[jsDate invokeMethod:@"getMonth" withArguments:@[]] toInt32] + 1;
    int day = [[jsDate invokeMethod:@"getDate" withArguments:@[]] toInt32];
    int hour = [[jsDate invokeMethod:@"getHours" withArguments:@[]] toInt32];
    int minute = [[jsDate invokeMethod:@"getMinutes" withArguments:@[]] toInt32];
    int second = [[jsDate invokeMethod:@"getSeconds" withArguments:@[]] toInt32];

    checkResult(@"JS Date to NSDate", year == [components year]
        && month == [components month]
        && day == [components day]
        && hour == [components hour]
        && minute == [components minute]
        && second == [components second]);
}

+ (void) roundTripThroughJSDateTest
{
    JSContext *context = [[JSContext alloc] init];
    [context evaluateScript:@"function jsReturnDate(date) { return date; }"];
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
    [formatter setDateFormat:@"MMMM dd',' yyyy hh:mm:ss"];
    NSDate *februaryFourth2014 = [formatter dateFromString:@"February 4, 2014 11:40:03"];
    NSDateComponents *components = [[NSCalendar currentCalendar] components:unitFlags fromDate:februaryFourth2014];
    
    JSValue *roundTripThroughJS = [context[@"jsReturnDate"] callWithArguments:@[februaryFourth2014]];
    int year = [[roundTripThroughJS invokeMethod:@"getFullYear" withArguments:@[]] toInt32];
    // Months are 0-indexed for JavaScript Dates.
    int month = [[roundTripThroughJS invokeMethod:@"getMonth" withArguments:@[]] toInt32] + 1;
    int day = [[roundTripThroughJS invokeMethod:@"getDate" withArguments:@[]] toInt32];
    int hour = [[roundTripThroughJS invokeMethod:@"getHours" withArguments:@[]] toInt32];
    int minute = [[roundTripThroughJS invokeMethod:@"getMinutes" withArguments:@[]] toInt32];
    int second = [[roundTripThroughJS invokeMethod:@"getSeconds" withArguments:@[]] toInt32];

    checkResult(@"JS date round trip", year == [components year]
        && month == [components month]
        && day == [components day]
        && hour == [components hour]
        && minute == [components minute]
        && second == [components second]);
}

+ (void) roundTripThroughObjCDateTest
{
    JSContext *context = [[JSContext alloc] init];
    context[@"objcReturnDate"] = ^(NSDate *date) {
        return date;
    };
    [context evaluateScript:@"function test() {\
        var date = new Date(2014, 1, 4, 11, 40, 3); \
        var result = objcReturnDate(date); \
        return date.getYear() === result.getYear() \
            && date.getMonth() === result.getMonth() \
            && date.getDate() === result.getDate() \
            && date.getHours() === result.getHours() \
            && date.getMinutes() === result.getMinutes() \
            && date.getSeconds() === result.getSeconds() \
            && date.getMilliseconds() === result.getMilliseconds();\
    }"];
    
    checkResult(@"ObjC date round trip", [[context[@"test"] callWithArguments:@[]] toBool]);
}

@end

void runDateTests()
{
    @autoreleasepool {
        [DateTests NSDateToJSDateTest];
        [DateTests JSDateToNSDateTest];
        [DateTests roundTripThroughJSDateTest];
        [DateTests roundTripThroughObjCDateTest];
    }
}

#endif // JSC_OBJC_API_ENABLED
