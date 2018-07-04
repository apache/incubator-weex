//
//  WXBridgeManager_private.h
//  Pods
//
//  Created by shenmo on 2018/7/3.
//

#import "WXBridgeManager.h"
#import "WXBridgeContext.h"

@interface WXBridgeManager ()

@property (nonatomic, strong) WXBridgeContext *bridgeCtx;
@property (nonatomic, assign) BOOL stopRunning;
@property (nonatomic, strong) NSMutableArray *instanceIdStack;

+ (instancetype)sharedManager;

+ (NSThread*)jsThread;

@end
