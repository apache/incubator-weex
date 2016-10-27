//
//  WXTimelineDomainController.m
/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */
#import "WXTimelineDomainController.h"

@interface WXTimelineDomainController () <WXTimelineCommandDelegate>

@end

@implementation WXTimelineDomainController

@dynamic domain;

+(WXTimelineDomainController *)defaultInstance {
    static WXTimelineDomainController *defaultInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaultInstance = [[WXTimelineDomainController alloc] init];
    });
    
    return defaultInstance;
}

+(Class)domainClass {
    return [WXTimelineDomain class];
}

#pragma mark - WXTimelineCommandDelegate
// Starts capturing instrumentation events.
// Param maxCallStackDepth: Samples JavaScript stack traces up to <code>maxCallStackDepth</code>, defaults to 5.
- (void)domain:(WXTimelineDomain *)domain startWithMaxCallStackDepth:(NSNumber *)maxCallStackDepth callback:(void (^)(id error))callback {
    
}

// Stops capturing instrumentation events.
- (void)domain:(WXTimelineDomain *)domain stopWithCallback:(void (^)(id error))callback {
    
}

// Starts calculating various DOM statistics and sending them as part of timeline events.
// Param enabled: True to start collecting DOM counters.
- (void)domain:(WXTimelineDomain *)domain setIncludeMemoryDetailsWithEnabled:(NSNumber *)enabled callback:(void (^)(id error))callback {
    
}

// Tells whether timeline agent supports frame instrumentation.
// Callback Param result: True if timeline supports frame instrumentation.
- (void)domain:(WXTimelineDomain *)domain supportsFrameInstrumentationWithCallback:(void (^)(NSNumber *result, id error))callback {
    
}


@end
