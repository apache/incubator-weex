//
//  WXCoreJSHandler.h
//  Pods-WeexDemo
//
//  Created by 邹明 on 2018/6/22.
//

#import "WXBridgeProtocol.h"

@interface WXCoreJSHandler : NSObject <WXBridgeProtocol>
@property (nonatomic, assign) BOOL multiContext;
@end
