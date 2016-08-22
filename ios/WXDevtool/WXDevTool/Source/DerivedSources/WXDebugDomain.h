/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "PDDynamicDebuggerDomain.h"
#import "PDObject.h"
#import "PDDebugger.h"
#import "WXDevtool.h"

@protocol WXDebugCommandDelegate;

@interface WXDebugDomain : PDDynamicDebuggerDomain

@property (nonatomic, assign) id <WXDebugCommandDelegate, PDCommandDelegate>delegate;

@end

@protocol WXDebugCommandDelegate <PDCommandDelegate>
@optional
- (void)domain:(WXDebugDomain *)domain sendLogLevel:(NSString *)level WithCallback:(void (^)(id error))callback;

@end