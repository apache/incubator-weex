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

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

#define WXLogLevel WeexLogLevel

typedef NS_ENUM(NSInteger, WXLogFlag) {
    WXLogFlagError      = 1 << 0,
    WXLogFlagWarning    = 1 << 1,
    WXLogFlagInfo       = 1 << 2,
    WXLogFlagLog        = 1 << 3,
    WXLogFlagDebug      = 1 << 4
};

/**
 *  Use Log levels to filter logs.
 */
typedef NS_ENUM(NSUInteger, WeexLogLevel){
    /**
     *  No logs
     */
    WXLogLevelOff       = 0,
    
    /**
     *  Error only
     */
    WXLogLevelError     = WXLogFlagError,
    
    /**
     *  Error and warning
     */
    WXLogLevelWarning   = WXLogLevelError | WXLogFlagWarning,
    
    /**
     *  Error, warning and info
     */
    WXLogLevelInfo      = WXLogLevelWarning | WXLogFlagInfo,
    
    /**
     *  Log, warning info
     */
    WXLogLevelLog       = WXLogFlagLog | WXLogLevelInfo,
    
    /**
     *  Error, warning, info and debug logs
     */
    WXLogLevelDebug     = WXLogLevelLog | WXLogFlagDebug,
    
    /**
     *  All
     */
    WXLogLevelAll       = NSUIntegerMax
};

/**
 *  External log protocol, which is used to output the log to the external.
 */
@protocol WXLogProtocol <NSObject>

@required

/**
 * External log level.
 */
- (WXLogLevel)logLevel;

- (void)log:(WXLogFlag)flag message:(NSString *)message;

@end

@interface WXLog : NSObject

+ (WXLogLevel)logLevel;

+ (void)setLogLevel:(WXLogLevel)level;

+ (NSString *)logLevelString;

+ (void)setLogLevelString:(NSString *)levelString;

//+ (void)log:(WXLogFlag)flag file:(const char *)fileName line:(NSUInteger)line format:(NSString *)format, ... NS_FORMAT_FUNCTION(4,5);

+ (void)log:(WXLogFlag)flag file:(const char *)fileName line:(NSUInteger)line message:(NSString *)message;

+ (void)devLog:(WXLogFlag)flag file:(const char *)fileName line:(NSUInteger)line format:(NSString *)format, ... NS_FORMAT_FUNCTION(4,5);

+ (void)registerExternalLog:(id<WXLogProtocol>)externalLog;

+ (id<WXLogProtocol>)getCurrentExternalLog;

@end

#define WX_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

 
#define WX_LOG(flag, fmt, ...)          \
do {                                    \
    [WXLog devLog:flag                     \
             file:WX_FILENAME              \
             line:__LINE__                 \
           format:(fmt), ## __VA_ARGS__];  \
} while(0)

extern void _WXLogObjectsImpl(NSString *severity, NSArray *arguments);

#define WXLog(format,...)               WX_LOG(WXLogFlagLog, format, ##__VA_ARGS__)
#define WXLogDebug(format, ...)         WX_LOG(WXLogFlagDebug, format, ##__VA_ARGS__)
#define WXLogInfo(format, ...)          WX_LOG(WXLogFlagInfo, format, ##__VA_ARGS__)
#define WXLogWarning(format, ...)       WX_LOG(WXLogFlagWarning, format ,##__VA_ARGS__)
#define WXLogError(format, ...)         WX_LOG(WXLogFlagError, format, ##__VA_ARGS__)

NS_ASSUME_NONNULL_END
