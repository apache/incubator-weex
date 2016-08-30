//
//  WXWebGLDomain.m
//  PonyDebuggerDerivedSources
//
//  Generated on 8/23/12
//
//  Licensed to Square, Inc. under one or more contributor license agreements.
//  See the LICENSE file distributed with this work for the terms under
//  which Square, Inc. licenses this file to you.
//

#import "WXObject.h"
#import "WXWebGLDomain.h"
#import "WXObject.h"
#import "WXWebGLTypes.h"


@interface WXWebGLDomain ()
//Commands

@end

@implementation WXWebGLDomain

@dynamic delegate;

+ (NSString *)domainName;
{
    return @"WebGL";
}



- (void)handleMethodWithName:(NSString *)methodName parameters:(NSDictionary *)params responseCallback:(WXResponseCallback)responseCallback;
{
    if ([methodName isEqualToString:@"enable"] && [self.delegate respondsToSelector:@selector(domain:enableWithCallback:)]) {
        [self.delegate domain:self enableWithCallback:^(id error) {
            responseCallback(nil, error);
        }];
    } else if ([methodName isEqualToString:@"disable"] && [self.delegate respondsToSelector:@selector(domain:disableWithCallback:)]) {
        [self.delegate domain:self disableWithCallback:^(id error) {
            responseCallback(nil, error);
        }];
    } else if ([methodName isEqualToString:@"dropTraceLog"] && [self.delegate respondsToSelector:@selector(domain:dropTraceLogWithTraceLogId:callback:)]) {
        [self.delegate domain:self dropTraceLogWithTraceLogId:[params objectForKey:@"traceLogId"] callback:^(id error) {
            responseCallback(nil, error);
        }];
    } else if ([methodName isEqualToString:@"captureFrame"] && [self.delegate respondsToSelector:@selector(domain:captureFrameWithCallback:)]) {
        [self.delegate domain:self captureFrameWithCallback:^(NSString *traceLogId, id error) {
            NSMutableDictionary *params = [[NSMutableDictionary alloc] initWithCapacity:1];

            if (traceLogId != nil) {
                [params setObject:traceLogId forKey:@"traceLogId"];
            }

            responseCallback(params, error);
        }];
    } else if ([methodName isEqualToString:@"getTraceLog"] && [self.delegate respondsToSelector:@selector(domain:getTraceLogWithTraceLogId:callback:)]) {
        [self.delegate domain:self getTraceLogWithTraceLogId:[params objectForKey:@"traceLogId"] callback:^(WXWebGLTraceLog *traceLog, id error) {
            NSMutableDictionary *params = [[NSMutableDictionary alloc] initWithCapacity:1];

            if (traceLog != nil) {
                [params setObject:traceLog forKey:@"traceLog"];
            }

            responseCallback(params, error);
        }];
    } else if ([methodName isEqualToString:@"replayTraceLog"] && [self.delegate respondsToSelector:@selector(domain:replayTraceLogWithTraceLogId:stepNo:callback:)]) {
        [self.delegate domain:self replayTraceLogWithTraceLogId:[params objectForKey:@"traceLogId"] stepNo:[params objectForKey:@"stepNo"] callback:^(NSString *screenshotDataUrl, id error) {
            NSMutableDictionary *params = [[NSMutableDictionary alloc] initWithCapacity:1];

            if (screenshotDataUrl != nil) {
                [params setObject:screenshotDataUrl forKey:@"screenshotDataUrl"];
            }

            responseCallback(params, error);
        }];
    } else {
        [super handleMethodWithName:methodName parameters:params responseCallback:responseCallback];
    }
}

@end


@implementation WXDebugger (WXWebGLDomain)

- (WXWebGLDomain *)webGLDomain;
{
    return [self domainForName:@"WebGL"];
}

@end
