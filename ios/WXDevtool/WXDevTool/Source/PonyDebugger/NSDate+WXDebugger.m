//
//  NSDate+WXDebugger.m
//  PonyDebugger
//
//  Created by Wen-Hao Lue on 2013-01-30.
//
//

#import "NSDate+WXDebugger.h"

@implementation NSDate (WXDebugger)

+ (NSNumber *)WX_timestamp;
{
    return [NSNumber numberWithDouble:[[NSDate date] timeIntervalSince1970]];
}

@end
