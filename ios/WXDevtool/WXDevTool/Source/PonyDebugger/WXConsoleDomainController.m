//
//  WXConsoleDomainController.m
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 2013-01-30.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXConsoleDomainController.h"
#import "WXRuntimeDomainController.h"
#import "WXDefinitions.h"
#import "WXRuntimeTypes.h"
#import "WXConsoleDomain.h"
#import "WXConsoleTypes.h"
#import "NSObject+WXRuntimePropertyDescriptor.h"


@interface WXConsoleDomainController () <WXConsoleCommandDelegate>

// Keep track of any keys coming in from the logging functions to release later.
@property (nonatomic, strong) NSMutableSet *loggedObjectKeys;

@end


@implementation WXConsoleDomainController

@dynamic domain;

#pragma mark - Statics

+ (WXConsoleDomainController *)defaultInstance;
{
    static WXConsoleDomainController *defaultInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaultInstance = [[WXConsoleDomainController alloc] init];
    });
    
    return defaultInstance;
}

+ (Class)domainClass;
{
    return [WXConsoleDomain class];
}

#pragma mark - WXConsoleCommandDelegate

// Clears console messages collected in the browser.
- (void)domain:(WXConsoleDomain *)domain clearMessagesWithCallback:(void (^)(id error))callback;
{
    callback(nil);

    // Clearing the console will release all references logged at the current time.
    [[WXRuntimeDomainController defaultInstance] clearObjectReferencesByKey:self.loggedObjectKeys.allObjects];
    [self.loggedObjectKeys removeAllObjects];
}


#pragma mark - Public Methods

- (void)logWithArguments:(NSArray *)args severity:(NSString *)severity
{
    // Construct the message by creating the runtime objects for each argument provided.
    NSMutableString *text = [[NSMutableString alloc] init];
    NSMutableArray *parameters = [[NSMutableArray alloc] init];
    for (NSObject *object in args) {
        WXRuntimeRemoteObject *remoteObject = [NSObject WX_remoteObjectRepresentationForObject:object];
        if ([remoteObject.subtype isEqualToString:@"array"]) {
            remoteObject.subtype = nil;
        }
        [text appendFormat:@"%@ ", object];
        [parameters addObject:remoteObject];

        if (remoteObject.objectId) {
            [self.loggedObjectKeys addObject:remoteObject.objectId];
        }
    }

    [text deleteCharactersInRange:NSMakeRange(text.length - 1, 1)];

    WXConsoleConsoleMessage *consoleMessage = [[WXConsoleConsoleMessage alloc] init];
    NSArray *severityOptions = @[@"debug", @"log", @"warning", @"info", @"error"];
    if ([severityOptions containsObject:severity]) {
        consoleMessage.level = severity;
    } else {
        consoleMessage.level = @"log";
    }

    consoleMessage.source = @"console-api";
    consoleMessage.stackTrace = [[NSArray alloc] init];
    consoleMessage.parameters = parameters;
    consoleMessage.repeatCount = [NSNumber numberWithInteger:1];
    consoleMessage.text = text;

    [self.domain messageAddedWithMessage:consoleMessage];
}

- (void)clear;
{
    [self.domain messagesCleared];

    // Clearing the console will release all references logged at the current time.
    [[WXRuntimeDomainController defaultInstance] clearObjectReferencesByKey:self.loggedObjectKeys.allObjects];
    [self.loggedObjectKeys removeAllObjects];
}

@end
