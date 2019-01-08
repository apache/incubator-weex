/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#import "WXLog.h"
#import "WXUtility.h"
#import "WXAssert.h"
#import "WXSDKManager.h"

// Xcode does NOT natively support colors in the Xcode debugging console.
// You'll need to install the XcodeColors plugin to see colors in the Xcode console.
// https://github.com/robbiehanson/XcodeColors
//
// The following is documentation from the XcodeColors project:
//
//
// How to apply color formatting to your log statements:
//
// To set the foreground color:
// Insert the ESCAPE_SEQ into your string, followed by "fg124,12,255;" where r=124, g=12, b=255.
//
// To set the background color:
// Insert the ESCAPE_SEQ into your string, followed by "bg12,24,36;" where r=12, g=24, b=36.
//
// To reset the foreground color (to default value):
// Insert the ESCAPE_SEQ into your string, followed by "fg;"
//
// To reset the background color (to default value):
// Insert the ESCAPE_SEQ into your string, followed by "bg;"
//
// To reset the foreground and background color (to default values) in one operation:
// Insert the ESCAPE_SEQ into your string, followed by ";"

#ifdef DEBUG
static const WXLogLevel defaultLogLevel = WXLogLevelLog;
#else
static const WXLogLevel defaultLogLevel = WXLogLevelWarning;
#endif

static id<WXLogProtocol> _externalLog;

static BOOL _logToWebSocket = NO;

@interface WXSafeLog : NSObject
/**
 * @abstract This is a safer but slower log to resolve DEBUG log crash when there is a retain cycle in the object
 */
+ (NSString *)getLogMessage:(NSString *)format arguments:(va_list)args;

@end

@implementation WXLog
{
    WXLogLevel _logLevel;
}

+ (instancetype)sharedInstance
{
    static WXLog *_sharedInstance = nil;
    static dispatch_once_t oncePredicate;
    dispatch_once(&oncePredicate, ^{
        if (NSClassFromString(@"WXDebugLoggerBridge")) {
            _logToWebSocket = YES;
        }
        _sharedInstance = [[self alloc] init];
        _sharedInstance->_logLevel = defaultLogLevel;
    });
    return _sharedInstance;
}

+ (void)setLogLevel:(WXLogLevel)level
{
    ((WXLog*)[self sharedInstance])->_logLevel = level;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
    Class propertyClass = NSClassFromString(@"WXTracingViewControllerManager");
    SEL sel = NSSelectorFromString(@"loadTracingView");
    if(propertyClass && [propertyClass respondsToSelector:sel]){
        [propertyClass performSelector:sel];
    }
#pragma clang diagnostic pop
}

+ (WXLogLevel)logLevel
{
    return ((WXLog*)[self sharedInstance])->_logLevel;
}

+ (NSString *)logLevelString
{
    NSDictionary *logLevelEnumToString =
    @{
      @(WXLogLevelOff) : @"off",
      @(WXLogLevelError) : @"error",
      @(WXLogLevelWarning) : @"warn",
      @(WXLogLevelInfo) : @"info",
      @(WXLogLevelLog) : @"log",
      @(WXLogLevelDebug) : @"debug",
      @(WXLogLevelAll) : @"debug"
      };
    return [logLevelEnumToString objectForKey:@([self logLevel])];
}

+ (void)setLogLevelString:(NSString *)levelString
{
    NSDictionary *logLevelStringToEnum =
    @{
      @"all" : @(WXLogLevelAll),
      @"error" : @(WXLogLevelError),
      @"warn" : @(WXLogLevelWarning),
      @"info" : @(WXLogLevelInfo),
      @"debug" : @(WXLogLevelDebug),
      @"log" : @(WXLogLevelLog)
    };

    [self setLogLevel:[logLevelStringToEnum[levelString] unsignedIntegerValue]];
}

+ (void)log:(WXLogFlag)flag file:(const char *)fileName line:(NSUInteger)line message:(NSString *)message
{
    NSString *flagString;
    switch (flag) {
        case WXLogFlagError: {
            flagString = @"error";
        }
            break;
        case WXLogFlagWarning:
            flagString = @"warn";
            break;
        case WXLogFlagDebug:
            flagString = @"debug";
            break;
        case WXLogFlagLog:
            flagString = @"log";
            break;
        default:
            flagString = @"info";
            break;
    }

    NSString *logMessage = [NSString stringWithFormat:@"<Weex>[%@]%s:%ld, %@", flagString, fileName, (unsigned long)line, message];

    if ([_externalLog logLevel] & flag) {
        [_externalLog log:flag message:logMessage];
    }

    if (_logToWebSocket) {
        [[WXSDKManager bridgeMgr] logToWebSocket:flagString message:message];
    }

    if ([WXLog logLevel] & flag) {
        NSLog(@"%@", logMessage);
    }
}

+ (void)devLog:(WXLogFlag)flag file:(const char *)fileName line:(NSUInteger)line format:(NSString *)format, ... {
    if ([WXLog logLevel] & flag || [_externalLog logLevel] & flag) {
        if (!format) {
            return;
        }
        NSString *flagString = @"log";
        switch (flag) {
            case WXLogFlagError:
                flagString = @"error";
                break;
            case WXLogFlagWarning:
                flagString = @"warn";
                break;
            case WXLogFlagDebug:
                flagString = @"debug";
                break;
            case WXLogFlagLog:
                flagString = @"log";
                break;
            default:
                flagString = @"info";
                break;
        }

        va_list args;
        va_start(args, format);
		NSString *message = @"";
		if (flag == WXLogFlagDebug) {
			message = [WXSafeLog getLogMessage:format arguments:args];
		} else {
			message = [[NSString alloc] initWithFormat:format arguments:args];
		}
        va_end(args);

        NSArray *messageAry = [NSArray arrayWithObjects:message, nil];
        Class WXLogClass = NSClassFromString(@"WXDebugger");
        if (WXLogClass) {
            SEL selector = NSSelectorFromString(@"coutLogWithLevel:arguments:");
            NSMethodSignature *methodSignature = [WXLogClass instanceMethodSignatureForSelector:selector];
            if (methodSignature == nil) {
                NSString *info = [NSString stringWithFormat:@"%@ not found", NSStringFromSelector(selector)];
                [NSException raise:@"Method invocation appears abnormal" format:info, nil];
            }
            NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:methodSignature];
            [invocation setTarget:[WXLogClass alloc]];
            [invocation setSelector:selector];
            [invocation setArgument:&flagString atIndex:2];
            [invocation setArgument:&messageAry atIndex:3];
            [invocation invoke];
        }

        [self log:flag file:fileName line:line message:message];
    }
}

#pragma mark - External Log

+ (void)registerExternalLog:(id<WXLogProtocol>)externalLog
{
    _externalLog = externalLog;
}

@end

#pragma mark - WXSafeLog

static void dealWithValue(NSMutableString *result, id value, NSMutableSet *outSet);
static NSUInteger getParamCount(NSString *format, NSMutableDictionary *outDict)
{
    static NSMutableSet *possibleTwoSet = nil;
    static NSMutableSet *possibleThreeSet = nil;
    static NSMutableSet *possibleTwo = nil;
    static NSMutableSet *possibleThree = nil;

    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSArray *longArray = @[@"ld"];
        NSArray *longLongArray = @[@"lld"];
        NSDictionary *coolDict = @{@"long":longArray, @"long long":longLongArray};
        possibleTwoSet = [NSMutableSet set];
        possibleThreeSet = [NSMutableSet set];
        possibleTwo = [NSMutableSet set];
        possibleThree = [NSMutableSet set];
        for (NSString * key in coolDict.allKeys) {
            NSArray *array = coolDict[key];
            for (NSString * value in array) {
                if (value.length == 2) {
                    [possibleTwoSet addObject:value];
                    NSString *sub = [value substringWithRange:NSMakeRange(0,1)];
                    if (![possibleTwo containsObject:sub]) {
                        [possibleTwo addObject:sub];
                    }
                } else if (value.length == 3) {
                    [possibleThreeSet addObject:value];
                    NSString *sub = [value substringWithRange:NSMakeRange(0,2)];
                    if (![possibleThree containsObject:sub]) {
                        [possibleThree addObject:sub];
                    }
                }
            }
        }
    });

    NSUInteger paramCount = 0;
    NSUInteger formatLength = [format length];
    NSRange searchRange = NSMakeRange(0, formatLength);
    NSRange paramRange = [format rangeOfString:@"%" options:0 range:searchRange];

    while (paramRange.location != NSNotFound)
    {
        NSString *subString = @" ";
        NSUInteger location = paramRange.location;
        do {
            location ++;
            subString = [format substringWithRange:NSMakeRange(location, 1)];
        } while (([subString compare:@"0"] != NSOrderedAscending && [subString compare:@"9"] != NSOrderedDescending) || [subString compare:@"."] == NSOrderedSame);
        if ([possibleTwo containsObject:subString]) {
            NSString *subString3 = [format substringWithRange:NSMakeRange(location, 2)];
            if ([possibleThree containsObject:subString3]) {
                NSString *subString4 = [format substringWithRange:NSMakeRange(location, 3)];
                if ([possibleThreeSet containsObject:subString4]) {
                    subString = subString4;
                }
            } else {
                NSString *subString2 = [format substringWithRange:NSMakeRange(location, 2)];
                if ([possibleTwoSet containsObject:subString2]) {
                    subString = subString2;
                }
            }
        }
        [outDict setObject:subString forKey:@(paramCount)];
        paramCount++;
        searchRange.location = paramRange.location + 1;
        searchRange.length = formatLength - searchRange.location;

        paramRange = [format rangeOfString:@"%" options:0 range:searchRange];
    }
    return paramCount;
}

static NSString *dealWithDictionary(NSDictionary *dict, NSMutableSet *outSet)
{
    NSMutableString *result = [NSMutableString new];
    [result appendString:@" { "];
    int i = 0;
    for (id key in dict.allKeys) {
        if ([key isKindOfClass:[NSDictionary class]] || [key isKindOfClass:[NSArray class]] || [key isKindOfClass:[NSSet class]]) {
            [result appendString:@"(invalid Dictionary key) : ( )"];
        } else {
            [result appendString:[NSString stringWithFormat:@"%@ : ", key]];
            id value = [dict objectForKey:key];
            NSNumber *pointerValue = [NSNumber numberWithLong:(long)value];
            if ([outSet containsObject:pointerValue]) {
                [result appendString:@"(Found Retain Cycle!!!)"];
            } else {
                dealWithValue(result, value, outSet);
            }
        }
        i++;
        if (i < dict.allKeys.count) {
            [result appendString:@" , "];
        }
    }
    [result appendString:@" } "];
    return result;
}

static NSString *dealWithArray(NSArray *array, NSMutableSet *outSet)
{
    NSMutableString *result = [NSMutableString new];
    [result appendString:@" [ "];
    int i = 0;
    for (id value in array) {
        NSNumber *pointerValue = [NSNumber numberWithLong:(long)value];
        if ([outSet containsObject:pointerValue]) {
            [result appendString:@"(Found Retain Cycle!!!)"];
        } else {
            dealWithValue(result, value, outSet);
        }
        i++;
        if (i < array.count) {
            [result appendString:@" , "];
        }
    }
    [result appendString:@" ] "];
    return result;
}

static NSString *dealWithSet(NSSet *set, NSMutableSet *outSet)
{
    NSMutableString *result = [NSMutableString new];
    [result appendString:@" ( "];
    int i = 0;
    for (id value in set) {
        NSNumber *pointerValue = [NSNumber numberWithLong:(long)value];
        if ([outSet containsObject:pointerValue]) {
            [result appendString:@"(Found Retain Cycle!!!)"];
        } else {
            dealWithValue(result, value, outSet);
        }
        i++;
        if (i < set.count) {
            [result appendString:@" , "];
        }
    }
    [result appendString:@" ) "];
    return result;
}

static void dealWithValue(NSMutableString *result, id value, NSMutableSet *outSet)
{
    NSNumber *pointerValue = [NSNumber numberWithLong:(long)value];
    if (pointerValue.longValue > 100000) {
        [outSet addObject:pointerValue];
    }
    if ([value isKindOfClass:[NSArray class]]) {
        NSString *tempString = dealWithArray((NSArray *)value, outSet);
        [result appendString:tempString];
    } else if ([value isKindOfClass:[NSDictionary class]]) {
        [result appendString:dealWithDictionary((NSDictionary *)value, outSet)];
    } else if ([value isKindOfClass:[NSSet class]]) {
        [result appendString:dealWithSet((NSSet *)value, outSet)];
    } else {
        [result appendString:[NSString stringWithFormat:@"%@", value]];
    }
}

@implementation WXSafeLog

+ (NSString *)getLogMessage:(NSString *)format arguments:(va_list)args
{
    NSMutableString *mutableFormat = [NSMutableString stringWithString:format];
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    NSUInteger count = getParamCount(format, dict);
    NSMutableArray<NSString *> *replacedDict = [NSMutableArray array];
    for (NSUInteger i = 0; i < count; i++) {
        if ([dict[@(i)] isEqualToString:@"@"]) {
            NSMutableSet *outSet = [NSMutableSet set];
            __unsafe_unretained id obj = va_arg(args, id);
            NSString *output = @"";
            if (obj) {
                if ([obj isKindOfClass:[NSDictionary class]]) {
                    output = dealWithDictionary((NSDictionary *)obj, outSet);
                } else if ([obj isKindOfClass:[NSArray class]]) {
                    output = dealWithArray((NSArray *)obj, outSet);
                } else if ([obj isKindOfClass:[NSSet class]]) {
                    output = dealWithSet((NSSet *)obj, outSet);
                } else {
                    output = [NSString stringWithFormat:@"%@", obj];
                }
            }
            [replacedDict addObject:output];
        } else {
            NSString *logFormat = dict[@(i)];
            if (logFormat.length == 1) {
#define LEN 2
                char tempChar[LEN];
                memset(tempChar, 0, LEN);
                [logFormat getCString:tempChar maxLength:LEN encoding:NSASCIIStringEncoding];
                switch(tempChar[0]) {
                    case '%':
                    case 'c':
//Suppress "Second argument to 'va_arg' is of promotable type 'char'; this va_arg has undefined behavior because arguments will be promoted to 'int'" warning
//                        va_arg(args, char);
                        va_arg(args, int);
                        break;
                    case 'd':
                    case 'D':
                        va_arg(args, int);
                        break;
                    case 'f':
                    case 'F':
                        va_arg(args, double);
                        break;
                    case 'C':
//                        va_arg(args, unichar);
                        va_arg(args, int);
                        break;
                    case 's':
                        va_arg(args, char *);
                        break;
                    case 'S':
                        va_arg(args, unichar *);
                        break;
                    case 'p':
                        va_arg(args, void *);
                        break;
                    default:
                        va_arg(args, void *);
                        break;
                }
            } else {
                static NSDictionary *map;
                static dispatch_once_t onceToken;
                dispatch_once(&onceToken, ^{
                    map = @{
                            @"ld":@"long", \
                            @"lld":@"long long", \
                            };
                });
                NSString *type = map[dict[@(i)]];
                if ([type isEqualToString:@"long"]) {
                    va_arg(args, long);
                } else if ([type isEqualToString:@"long long"]) {
                    va_arg(args, long long);
                }
            }
        }
    }
    NSString *prefix = @"#$~";
    NSString *suffix = @"~$#";
    NSString *replace = [NSString stringWithFormat:@"%@%%p%@", prefix, suffix];
    [mutableFormat replaceOccurrencesOfString:@"%@" withString:replace options:NSLiteralSearch range:NSMakeRange(0, mutableFormat.length)];
    NSMutableString *result = [[NSMutableString alloc] initWithFormat:mutableFormat arguments:args];
    int j = 0;
    NSRange range, range1, range2;
    range1 = [result rangeOfString:prefix options:NSLiteralSearch];
    range2 = [result rangeOfString:suffix options:NSLiteralSearch];
    range = NSMakeRange(range1.location, range2.location+range2.length - range1.location);
    while(range1.length>0 && range2.length>0) {
        [result replaceCharactersInRange:range withString:replacedDict[j++]];
        range1 = [result rangeOfString:prefix options:NSLiteralSearch];
        range2 = [result rangeOfString:suffix options:NSLiteralSearch];
        range = NSMakeRange(range1.location, range2.location+range2.length - range1.location);
    }
    return result;
}

@end
