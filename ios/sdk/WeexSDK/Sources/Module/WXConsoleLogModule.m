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
    // We use printf to output console log
    printf("\n%s\n", [message UTF8String]);
}

@end

@implementation WXConsoleLogModule

WX_EXPORT_METHOD(@selector(switchLogLevel:))

- (void)switchLogLevel:(NSString*)logLevel {
    static id<WXLogProtocol> OriginalLogger = nil;
    
    // When first invoke WXConsoleLogModule we record original logger
    if (OriginalLogger == nil) {
        OriginalLogger = [WXLog getCurrentExternalLog];
    }
    
    if ([logLevel isEqualToString:@"off"]) {
        [WXLog registerExternalLog:OriginalLogger];
    }
    else if ([logLevel isEqualToString:@"error"]) {
        [WXConsoleLogHandler sharedInstance].logLevel = WXLogLevelError;
        [WXLog registerExternalLog:[WXConsoleLogHandler sharedInstance]];
    }
    else if ([logLevel isEqualToString:@"warning"]) {
        [WXConsoleLogHandler sharedInstance].logLevel = WXLogLevelWarning;
        [WXLog registerExternalLog:[WXConsoleLogHandler sharedInstance]];
    }
    else if ([logLevel isEqualToString:@"info"]) {
        [WXConsoleLogHandler sharedInstance].logLevel = WXLogLevelInfo;
        [WXLog registerExternalLog:[WXConsoleLogHandler sharedInstance]];
    }
    else if ([logLevel isEqualToString:@"debug"]) {
        [WXConsoleLogHandler sharedInstance].logLevel = WXLogLevelDebug;
        [WXLog registerExternalLog:[WXConsoleLogHandler sharedInstance]];
    }
}

@end
