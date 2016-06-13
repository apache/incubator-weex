//
//  PDDebuggerDomainController.h
//  Pods
//
//  Created by yangshengtao on 16/6/2.
//
//

#import "PonyDebugger.h"
#import "PDDebuggerDomain.h"
#import "PDDynamicDebuggerDomain.h"

@interface PDDebuggerDomainController : PDDomainController <PDDebuggerCommandDelegate>

+ (PDDebuggerDomainController *)defaultInstance;

- (void)remoteDebuggerControllerTest;

@property (nonatomic, strong) PDDebuggerDomain *domain;

@end
