//
//  WXPageDomainController.m
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 8/6/12.
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXPageDomainController.h"
#import "WXRuntimeDomainController.h"
#import "WXPageDomain.h"
#import "WXPageTypes.h"


@interface WXPageDomainController () <WXPageCommandDelegate>
@end


@implementation WXPageDomainController

@dynamic domain;

#pragma mark - Statics

+ (WXPageDomainController *)defaultInstance;
{
    static WXPageDomainController *defaultInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaultInstance = [[WXPageDomainController alloc] init];
    });
    
    return defaultInstance;
}

+ (Class)domainClass;
{
    return [WXPageDomain class];
}


#pragma mark - WXPageCommandDelegate
- (void)domain:(WXPageDomain *)domain enableWithCallback:(void (^)(id error))callback {
    callback(nil);
//    [self.domain workerRegistrationUpdated];
//    [self.domain workerVersionUpdated];
}

- (void)domain:(WXPageDomain *)domain canOverrideDeviceMetricsWithCallback:(void (^)(NSNumber *result, id error))callback {
    callback([NSNumber numberWithBool:YES],nil);
}

- (void)domain:(WXPageDomain *)domain canOverrideGeolocationWithCallback:(void (^)(NSNumber *result, id error))callback {
    callback([NSNumber numberWithBool:YES],nil);
}

- (void)domain:(WXPageDomain *)domain canOverrideDeviceOrientationWithCallback:(void (^)(NSNumber *result, id error))callback {
    callback([NSNumber numberWithBool:YES],nil);
}

- (void)domain:(WXPageDomain *)domain getResourceTreeWithCallback:(void (^)(WXPageFrameResourceTree *, id))callback;
{
    NSString *localPath = [NSString stringWithFormat:@"%@/bundlejs/index.js",[NSBundle mainBundle].bundlePath];
    
    WXPageFrame *frame = [[WXPageFrame alloc] init];
    frame.identifier = @"3888.3";
    frame.name = @"Root";
    frame.securityOrigin = [NSBundle mainBundle].bundlePath;//[[NSBundle mainBundle] bundleIdentifier];
    frame.url = localPath;//localPath;
    frame.loaderId = @"11111";
    frame.mimeType = @"application/javascript";
    
    WXFrameResource *resource = [[WXFrameResource alloc] init];
    resource.url = @"index.js";
    resource.mimeType = @"application/javascript";
    resource.type = @"Script";
    
    WXPageFrameResourceTree *resourceTree = [[WXPageFrameResourceTree alloc] init];
    resourceTree.frame = frame;
    resourceTree.resources = @[resource];
    
    callback(resourceTree, nil);
}

- (void)domain:(WXPageDomain *)domain reloadWithIgnoreCache:(NSNumber *)ignoreCache scriptToEvaluateOnLoad:(NSString *)scriptToEvaluateOnLoad callback:(void (^)(id))callback;
{
    callback(nil);
}

@end
