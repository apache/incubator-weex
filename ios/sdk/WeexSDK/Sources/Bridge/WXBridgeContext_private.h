//
//  WXBridgeContext_private.h
//  Pods
//
//  Created by shenmo on 2018/7/3.
//

#import "WXBridgeContext.h"

@interface WXBridgeContext (Private)

- (NSInteger)invokeNative:(NSString *)instanceId tasks:(NSArray *)tasks callback:(NSString __unused*)callback;

@end
