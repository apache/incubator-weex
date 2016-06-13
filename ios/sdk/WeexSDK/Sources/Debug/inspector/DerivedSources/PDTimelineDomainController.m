//
//  PDTimelineDomainController.m
//  Pods
//
//  Created by yangshengtao on 16/6/6.
//
//

#import "PDTimelineDomainController.h"

@interface PDTimelineDomainController () <PDTimelineCommandDelegate>

@end

@implementation PDTimelineDomainController

@dynamic domain;

+(PDTimelineDomainController *)defaultInstance {
    static PDTimelineDomainController *defaultInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaultInstance = [[PDTimelineDomainController alloc] init];
    });
    
    return defaultInstance;
}

+(Class)domainClass {
    return [PDTimelineDomain class];
}

#pragma mark - PDTimelineCommandDelegate
// Starts capturing instrumentation events.
// Param maxCallStackDepth: Samples JavaScript stack traces up to <code>maxCallStackDepth</code>, defaults to 5.
- (void)domain:(PDTimelineDomain *)domain startWithMaxCallStackDepth:(NSNumber *)maxCallStackDepth callback:(void (^)(id error))callback {
    
}

// Stops capturing instrumentation events.
- (void)domain:(PDTimelineDomain *)domain stopWithCallback:(void (^)(id error))callback {
    
}

// Starts calculating various DOM statistics and sending them as part of timeline events.
// Param enabled: True to start collecting DOM counters.
- (void)domain:(PDTimelineDomain *)domain setIncludeMemoryDetailsWithEnabled:(NSNumber *)enabled callback:(void (^)(id error))callback {
    
}

// Tells whether timeline agent supports frame instrumentation.
// Callback Param result: True if timeline supports frame instrumentation.
- (void)domain:(PDTimelineDomain *)domain supportsFrameInstrumentationWithCallback:(void (^)(NSNumber *result, id error))callback {
    
}


@end
