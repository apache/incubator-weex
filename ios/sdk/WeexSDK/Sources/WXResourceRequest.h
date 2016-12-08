/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>

@interface WXResourceRequest : NSMutableURLRequest

@property (nonatomic, strong) NSString *referrer;
@property (nonatomic, strong) NSString *userAgent;

+ (instancetype)requestWithURL:(NSURL *)url
                      referrer:(NSString *)referrer
                   cachePolicy:(NSURLRequestCachePolicy)cachePolicy;

@end
