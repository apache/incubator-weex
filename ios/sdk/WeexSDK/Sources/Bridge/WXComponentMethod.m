//
//  WXComponentMethod.m
//  WeexSDK
//
//  Created by yinfeng on 2016/12/16.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import "WXComponentMethod.h"
#import "WXComponentFactory.h"
#import "WXComponentManager.h"
#import "WXSDKInstance.h"
#import "WXLog.h"
#import "WXUtility.h"

@implementation WXComponentMethod
{
    NSString *_componentName;
    NSString *_componentRef;
}

- (instancetype)initWithComponentName:(NSString *)componentName
                         componentRef:(NSString *)ref
                           methodName:(NSString *)methodName
                            arguments:(NSArray *)arguments
                             instance:(WXSDKInstance *)instance
{
    if (self = [super initWithMethodName:methodName arguments:arguments instance:instance]) {
        _componentName = componentName;
        _componentRef = ref;
    }
    
    return self;
}

- (void)invoke
{
    WXPerformBlockOnComponentThread(^{
        WXComponent *component = [self.instance componentForRef:_componentRef];
        if (!component) {
            WXLogError(@"component not found for ref:%@, type:%@", _componentRef, _componentName);
        }
        SEL selector = [WXComponentFactory methodWithComponentName:_componentName withMethod:self.methodName];
        NSInvocation * invocation = [self invocationWithTarget:component selector:selector];
        WXPerformBlockOnMainThread(^{
            [invocation invoke];
        });
    });
    
    
}

@end
