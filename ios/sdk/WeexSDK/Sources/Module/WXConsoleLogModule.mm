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

#import "WXConsoleLogModule.h"
#import "WXLog.h"
#include "base/log_defines.h"

#if !TARGET_OS_WATCH
#import <asl.h>
#endif

#if !__has_feature(objc_arc)
#error This file must be compiled with ARC. Use -fobjc-arc flag (or convert project to ARC).
#endif

@interface WXConsoleLogHandler : NSObject<WXLogProtocol>

@property (nonatomic, assign) WXLogLevel logLevel;

+ (WXConsoleLogHandler *)sharedInstance;

- (WXLogLevel)logLevel;

- (void)log:(WXLogFlag)flag message:(NSString *)message;

@end

@implementation WXConsoleLogHandler

+ (WXConsoleLogHandler *)sharedInstance
{
    static dispatch_once_t onceToken;
    static WXConsoleLogHandler* instance = nil;
    dispatch_once(&onceToken, ^{
        instance = [[WXConsoleLogHandler alloc] init];
    });
    return instance;
}

- (instancetype)init
{
    if (self = [super init]) {
        _logLevel = WXLogLevelAll;
    }
    return self;
}

- (WXLogLevel)logLevel
{
    return _logLevel;
}

- (void)log:(WXLogFlag)flag message:(NSString *)message
{
#if !TARGET_OS_WATCH
    dispatch_async(dispatch_get_main_queue(), ^{
        asl_log_message(ASL_LEVEL_NOTICE, "%s", [message UTF8String]);
    });
#else
    printf("\n%s\n", [message UTF8String]);
#endif
}

@end

@implementation WXConsoleLogModule

WX_EXPORT_METHOD(@selector(switchLogLevel:callback:))

- (void)switchLogLevel:(NSString*)logLevel callback:(WXKeepAliveCallback)callback {
    static id<WXLogProtocol> OriginalLogger = nil;
    
    // When first invoke WXConsoleLogModule we record original logger
    if (OriginalLogger == nil) {
        OriginalLogger = [WXLog getCurrentExternalLog];
    }
    
    if ([logLevel isEqualToString:@"off"]) {
        weex::base::LogImplement::getLog()->setPrintLevel(WeexCore::LogLevel::Off);
        [WXLog registerExternalLog:OriginalLogger];
    }
    else if ([logLevel isEqualToString:@"error"]) {
        weex::base::LogImplement::getLog()->setPrintLevel(WeexCore::LogLevel::Error);
        [WXConsoleLogHandler sharedInstance].logLevel = WXLogLevelError;
        [WXLog registerExternalLog:[WXConsoleLogHandler sharedInstance]];
    }
    else if ([logLevel isEqualToString:@"warning"]) {
        weex::base::LogImplement::getLog()->setPrintLevel(WeexCore::LogLevel::Warn);
        [WXConsoleLogHandler sharedInstance].logLevel = WXLogLevelWarning;
        [WXLog registerExternalLog:[WXConsoleLogHandler sharedInstance]];
    }
    else if ([logLevel isEqualToString:@"info"]) {
        weex::base::LogImplement::getLog()->setPrintLevel(WeexCore::LogLevel::Info);
        [WXConsoleLogHandler sharedInstance].logLevel = WXLogLevelInfo;
        [WXLog registerExternalLog:[WXConsoleLogHandler sharedInstance]];
    }
    else if ([logLevel isEqualToString:@"debug"]) {
        weex::base::LogImplement::getLog()->setPrintLevel(WeexCore::LogLevel::Debug);
        [WXConsoleLogHandler sharedInstance].logLevel = WXLogLevelDebug;
        [WXLog registerExternalLog:[WXConsoleLogHandler sharedInstance]];
    }
    
    if (callback) {
        callback(@{}, NO);
    }
}

@end
