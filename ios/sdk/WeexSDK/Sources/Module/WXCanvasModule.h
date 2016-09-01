//
//  WXCanvasModule.h
//  WeexSDK
//
//  Created by yuankong on 16/6/28.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WXModuleProtocol.h"

@interface WXCanvasModule : NSObject <WXModuleProtocol>

- (UIImage *) getImage:(NSString *)imageURL;

@end
