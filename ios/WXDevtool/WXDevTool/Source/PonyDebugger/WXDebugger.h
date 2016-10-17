//
//  WXDebugger.h
//  PonyDebugger
//
//  Created by Mike Lewis on 11/5/11.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>
#import <WeexSDK/WXBridgeProtocol.h>

#pragma mark - Preprocessor

// Remote logging definitions. Use preprocessor hackery to make this work nicely.
#ifdef DEBUG
#define WXDevLog(...)                  _WXLog(@"log", ##__VA_ARGS__)
#define WXLogD(...)                 _WXLog(@"debug", ##__VA_ARGS__)
#define WXLogW(...)                 _WXLog(@"warning", ##__VA_ARGS__)
#define WXLogI(...)                 _WXLog(@"info", ##__VA_ARGS__)
#define WXLogE(...)                 _WXLog(@"error", ##__VA_ARGS__)
#define WXLogObjects(...)           _WXLogObjects(@"log", ##__VA_ARGS__)
#define _WXLog(sev, ...)            _WXLogObjectsImpl(sev, @[[NSString stringWithFormat:__VA_ARGS__]]);
#define _WXLogObjects(sev, ...)     _WXLogObjectsImpl(sev, @[__VA_ARGS__]);
#else
#define WXDevLog(...)
#define WXLogD(...)
#define WXLogW(...)
#define WXLogI(...)
#define WXLogE(...)
#define WXLogObjects(...)
#define _WXLog(sev, ...)
#define _WXLogObjects(sev, ...)
#endif



#pragma mark - Definitions

@class SRWebSocket;
@class WXDomainController;
@protocol WXPrettyStringPrinting;


extern void _WXLogObjectsImpl(NSString *severity, NSArray *arguments);


#pragma mark - Public Interface

@interface WXDebugger : NSObject <WXBridgeProtocol>

+ (WXDebugger *)defaultInstance;
+ (id) allocWithZone:(struct _NSZone *)zone;

- (void) coutLogWithLevel:(NSString *)level arguments:(NSArray *)arguments;

- (id)domainForName:(NSString *)name;
- (void)sendEventWithName:(NSString *)string parameters:(id)params;

#pragma mark Connect/Disconnect
- (void)autoConnect;
- (void)autoConnectToBonjourServiceNamed:(NSString*)serviceName;
- (void)connectToURL:(NSURL *)url;
- (BOOL)isConnected;
- (void)disconnect;

#pragma mark Network Debugging
- (void)enableNetworkTrafficDebugging;
- (void)forwardAllNetworkTraffic;
- (void)forwardNetworkTrafficFromDelegateClass:(Class)cls;
+ (void)registerPrettyStringPrinter:(id<WXPrettyStringPrinting>)prettyStringPrinter;
+ (void)unregisterPrettyStringPrinter:(id<WXPrettyStringPrinting>)prettyStringPrinter;

#pragma mark Core Data Debugging
- (void)enableCoreDataDebugging;
- (void)addManagedObjectContext:(NSManagedObjectContext *)context;
- (void)addManagedObjectContext:(NSManagedObjectContext *)context withName:(NSString *)name;
- (void)removeManagedObjectContext:(NSManagedObjectContext *)context;

#pragma mark View Hierarchy Debugging
- (void)enableViewHierarchyDebugging;
- (void)setDisplayedViewAttributeKeyPaths:(NSArray *)keyPaths;

#pragma mark Remote Logging
- (void)enableRemoteLogging;
- (void)clearConsole;

#pragma mark Remote Debugging
- (void)enableRemoteDebugger;
- (void)remoteDebuggertest;

#pragma mark Timeline
- (void)enableTimeline;

#pragma mark CSSStyle
- (void)enableCSSStyle;

#pragma mark openDevTool Debug
- (void)enableDevToolDebug;

#pragma mark - listenning on server
//- (void)serverStartWithHost:(NSString *)host port:(NSUInteger)port;


@end
