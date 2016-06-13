//
//  PDTimelineDomainController.h
//  Pods
//
//  Created by yangshengtao on 16/6/6.
//
//

#import "PonyDebugger.h"
#import "PDTimelineDomain.h"

@interface PDTimelineDomainController : PDDomainController

+(PDTimelineDomainController *)defaultInstance;

@property (nonatomic, strong) PDTimelineDomain *domain;

@end
