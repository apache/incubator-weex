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

@implementation WXLog
{
    WXLogLevel _logLevel;
}

+ (instancetype)sharedInstance
{
    static WXLog *_sharedInstance = nil;
    static dispatch_once_t oncePredicate;
    dispatch_once(&oncePredicate, ^{
        _sharedInstance = [[self alloc] init];
        _sharedInstance->_logLevel = defaultLogLevel;
    });
    return _sharedInstance;
}


+ (void)setLogLevel:(WXLogLevel)level
{
    if (((WXLog*)[self sharedInstance])->_logLevel != level) {
        ((WXLog*)[self sharedInstance])->_logLevel = level;
        
        [[WXSDKManager bridgeMgr] resetEnvironment];
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
    Class propertyClass = NSClassFromString(@"WXTracingViewControllerManager");
    SEL sel =NSSelectorFromString(@"loadTracingView");
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
    
    [[WXSDKManager bridgeMgr] logToWebSocket:flagString message:message];
    
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
        NSString *message = [[NSString alloc] initWithFormat:format arguments:args];
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
