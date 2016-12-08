/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXResourceRequest.h"

NSString * const kHTTPHeaderNameUserAgent = @"User-Agent";
NSString * const kHTTPHeaderNameReferrer = @"Referer"; // The misspelling referer originated in the original proposal by computer "scientist" Phillip Hallam-Baker to incorporate the field into the HTTP specification. ╮(╯_╰)╭

@implementation WXResourceRequest

+ (instancetype)requestWithURL:(NSURL *)url referrer:(NSString *)referrer cachePolicy:(NSURLRequestCachePolicy)cachePolicy
{
    return [[self alloc] initWithURL:url referrer:referrer cachePolicy:cachePolicy];
}

- (instancetype)initWithURL:(NSURL *)url referrer:(NSString *)referrer cachePolicy:(NSURLRequestCachePolicy)cachePolicy
{
    if (self = [super initWithURL:url]) {
        self.cachePolicy = cachePolicy;
        [self setValue:referrer forHTTPHeaderField:kHTTPHeaderNameReferrer];
    }
    
    return self;
}

- (NSString *)referrer
{
    return [self valueForKey:kHTTPHeaderNameReferrer];
}

- (void)setReferrer:(NSString *)referrer
{
    [self setValue:referrer forKey:kHTTPHeaderNameReferrer];
}

- (NSString *)userAgent
{
    return [self valueForKey:kHTTPHeaderNameUserAgent];
}

- (void)setUserAgent:(NSString *)userAgent
{
    [self setValue:userAgent forKey:kHTTPHeaderNameUserAgent];
}

@end
