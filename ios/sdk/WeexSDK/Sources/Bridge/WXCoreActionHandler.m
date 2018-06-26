//
//  WXCoreActionHandler.m
//  Pods-WeexDemo
//
//  Created by 邹明 on 2018/6/22.
//

#import "WXCoreActionHandler.h"

@implementation WXCoreActionHandler

-(void)setJsVersion:(NSString *)version{
    
}

-(void)reportExceptionWithInstance:(NSString *)instanceId function:(NSString *)function exception:(NSString *)exception{
    
}

-(int)callNativeWithInstance:(NSString *)instanceId task:(NSString *)task callback:(NSString *)callback{
    return 0;
}

-(NSInvocation *)callNativeModuleWithInstance:(NSString *)instanceId module:(NSString *)module method:(NSString *)method arguments:(NSArray *)arguments options:(NSDictionary *)options{
    return NULL;
}

-(void)callNativeComponentWithInstance:(NSString *)instanceId ref:(NSString *)ref method:(NSString *)method arguments:(NSArray *)arguments options:(NSDictionary *)options{
    
}

-(void)setTimeoutWithCallbackId:(NSString *)callbackId time:(NSString *)time{
    
}

-(void)callNativeLog:(NSString *)log{
    
}

-(int)callUpdateFinishWithInstance:(NSString *)instanceId task:(NSString *)task callback:(NSString *)callback{
    return 0;
}

-(int)callRefreshFinishWithInstance:(NSString *)instanceId task:(NSString *)task callback:(NSString *)callbackId{
    return 0;
}

-(int)callAddEventWithInstance:(NSString *)instanceId ref:(NSString *)ref event:(NSString *)event{
    return 0;
}

-(int)callRemoveEventWithInstance:(NSString *)instanceId ref:(NSString *)ref event:(NSString *)event{
    return 0;
}

-(int)callCreatebodyWithInstance:(NSString *)instanceId componentType:(NSString *)componentType ref:(NSString *)ref styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events margins:(WXLayoutMargin *)margins paddings:(WXLayoutPadding *)paddings borderWidth:(WXLayoutBorderWidth *)borders{
    return 0;
}

-(int)callAddElementWithInstance:(NSString *)instanceId componentType:(NSString *)componentType ref:(NSString *)ref index:(int)index parentRef:(NSString *)parentRef styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events margins:(WXLayoutMargin *)margins paddings:(WXLayoutPadding *)paddings borderWidth:(WXLayoutBorderWidth *)borders willLayout:(BOOL)willLayout{
    return 0;
}

-(int)callLayoutWithInstance:(NSString *)instanceId ref:(NSString *)ref top:(float)top bottom:(float)bottom left:(float)left right:(float)right height:(float)height width:(float)width index:(int)index{
    return 0;
}

-(int)callUpdateStyleWithInstance:(NSString *)instanceId ref:(NSString *)ref styles:(NSDictionary *)style magin:(WXLayoutMargin *)margin padding:(WXLayoutPadding *)padding border:(WXLayoutBorderWidth *)border{
    return 0;
}

-(int)callUpdateAttrWithInstance:(NSString *)instanceId ref:(NSString *)ref attributes:(NSDictionary *)attributes{
    return 0;
}

-(int)callCreateFinishWithInstance:(NSString *)instanceId{
    return 0;
}

-(int)callRemoveElementWithInstance:(NSString *)instanceId ref:(NSString *)ref{
    return 0;
}

-(int)callMoveElementWithInstance:(NSString *)instanceId ref:(NSString *)ref parentRef:(NSString *)parentRef index:(int)index{
    return 0;
}

-(int)callAppendTreeCreateFinishWithInstance:(NSString *)instanceId ref:(NSString *)ref{
    return 0;
}

-(int)callHasTransitionProsWithInstance:(NSString *)instanceId style:(NSDictionary *)style{
    return 0;
}

@end
