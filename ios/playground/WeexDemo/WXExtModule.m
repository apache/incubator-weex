//
//  WXExtModule.m
//  WeexDemo
//
//  Created by admin on 2017/8/8.
//  Copyright © 2017年 taobao. All rights reserved.
//

#import "WXExtModule.h"

@implementation WXExtModule

WX_EXPORT_METHOD(@selector(generateCover:))

- (void)generateCover:(WXModuleCallback)callback
{
#if !TARGET_IPHONE_SIMULATOR
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    setenv("GCOV_PREFIX", [documentsDirectory cStringUsingEncoding:NSUTF8StringEncoding], 1);
    setenv("GCOV_PREFIX_STRIP", "6", 1);
#endif
    extern void __gcov_flush(void);
    __gcov_flush();
    
    if (callback) {
        NSDictionary * result = @{@"ok": @true};
        callback(result);
    }
}

@end
