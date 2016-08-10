//
//  WXPolyfillSet.h
//  WeexSDK
//
//  Created by yinfeng on 16/8/10.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <JavaScriptCore/JavaScriptCore.h>

@protocol WXPolyfillSetJSExports <JSExport>

+ (instancetype)create;

- (BOOL)has:(id)value;

- (NSUInteger)size;

- (void)add:(id)value;

- (BOOL)delete:(id)value;

- (void)clear;

@end

@interface WXPolyfillSet : NSObject <WXPolyfillSetJSExports>

@end

