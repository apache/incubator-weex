//
//  JSValue+Weex.h
//  WeexSDK
//
//  Created by yinfeng on 2016/12/15.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import <JavaScriptCore/JavaScriptCore.h>

@interface JSValue (Weex)

+ (JSValue *)wx_valueWithReturnValueFromInvocation:(NSInvocation *)invocation inContext:(JSContext *)context;

@end
