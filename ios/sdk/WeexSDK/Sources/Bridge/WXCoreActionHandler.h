//
//  WXCoreActionHandler.h
//  Pods-WeexDemo
//
//  Created by 邹明 on 2018/6/22.
//

#import <Foundation/Foundation.h>
#import "WXLayoutConstraint.h"

@interface WXCoreActionHandler : NSObject

-(void)setJsVersion:(NSString *)version;

-(void)reportExceptionWithInstance:(NSString *)instanceId function:(NSString *)function exception:(NSString *)exception;

-(int)callNativeWithInstance:(NSString *)instanceId task:(NSString *)task callback:(NSString *)callback;

-(NSInvocation *)callNativeModuleWithInstance:(NSString *)instanceId module:(NSString *)module method:(NSString *)method arguments:(NSArray *)arguments options:(NSDictionary *)options;

-(void)callNativeComponentWithInstance:(NSString *)instanceId ref:(NSString *)ref method:(NSString *)method arguments:(NSArray *)arguments options:(NSDictionary *)options;

-(void)setTimeoutWithCallbackId:(NSString *)callbackId time:(NSString *)time;

-(void)callNativeLog:(NSString *)log;

-(int)callUpdateFinishWithInstance:(NSString *)instanceId task:(NSString *)task callback:(NSString *)callback;

-(int)callRefreshFinishWithInstance:(NSString *)instanceId task:(NSString *)task callback:(NSString *)callbackId;

-(int)callAddEventWithInstance:(NSString *)instanceId ref:(NSString *)ref event:(NSString *)event;

-(int)callRemoveEventWithInstance:(NSString *)instanceId ref:(NSString *)ref event:(NSString *)event;

-(int)callCreatebodyWithInstance:(NSString *)instanceId componentType:(NSString *)componentType ref:(NSString *)ref styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events margins:(WXLayoutMargin *)margins paddings:(WXLayoutPadding *)paddings borderWidth:(WXLayoutBorderWidth *)borders;

-(int)callAddElementWithInstance:(NSString *)instanceId componentType:(NSString *)componentType ref:(NSString *)ref index:(int)index parentRef:(NSString *)parentRef styles:(NSDictionary *)styles attributes:(NSDictionary *)attributes events:(NSArray *)events margins:(WXLayoutMargin *)margins paddings:(WXLayoutPadding *)paddings borderWidth:(WXLayoutBorderWidth *)borders willLayout:(BOOL)willLayout;

-(int)callLayoutWithInstance:(NSString *)instanceId ref:(NSString *)ref top:(float)top bottom:(float)bottom left:(float)left right:(float)right height:(float)height width:(float)width index:(int)index;

-(int)callUpdateStyleWithInstance:(NSString *)instanceId ref:(NSString *)ref styles:(NSDictionary *)style magin:(WXLayoutMargin *)margin padding:(WXLayoutPadding *)padding border:(WXLayoutBorderWidth *)border;

-(int)callUpdateAttrWithInstance:(NSString *)instanceId ref:(NSString *)ref attributes:(NSDictionary *)attributes;

-(int)callCreateFinishWithInstance:(NSString *)instanceId;

-(int)callRemoveElementWithInstance:(NSString *)instanceId ref:(NSString *)ref;

-(int)callMoveElementWithInstance:(NSString *)instanceId ref:(NSString *)ref parentRef:(NSString *)parentRef index:(int)index;

-(int)callAppendTreeCreateFinishWithInstance:(NSString *)instanceId ref:(NSString *)ref;

-(int)callHasTransitionProsWithInstance:(NSString *)instanceId style:(NSDictionary *)style;

@end
