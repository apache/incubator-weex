//
//  PDProfilerDomainController.m
//  Pods
//
//  Created by yangshengtao on 16/6/6.
//
//

#import "PDProfilerDomainController.h"

@implementation PDProfilerDomainController

+(PDProfilerDomainController *)defaultInstace {
    static PDProfilerDomainController *defaultInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaultInstance = [[PDProfilerDomainController alloc] init];
    });
    
    return defaultInstance;
}

@end
