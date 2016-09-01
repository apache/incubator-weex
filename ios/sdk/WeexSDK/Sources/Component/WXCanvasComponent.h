//
//  WXCanvasComponent.h
//  WeexSDK
//
//  Created by yuankong on 16/6/22.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import <WeexSDK/WeexSDK.h>
#import "WXCanvasModule.h"
#import <GLKit/GLKit.h>

@interface WXCanvasComponent : WXComponent <GLKViewDelegate>

- (void) addDrawActions:(NSArray *)actions canvasModule:(WXCanvasModule*)canvasModule;

@end
